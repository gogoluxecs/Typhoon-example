////////////////////////////////////////////////////////////////////////////////
//
//  TYPHOON FRAMEWORK
//  Copyright 2013, Jasper Blues & Contributors
//  All Rights Reserved.
//
//  NOTICE: The authors permit you to use, modify, and distribute this file
//  in accordance with the terms of the license agreement accompanying it.
//
////////////////////////////////////////////////////////////////////////////////


#import <Foundation/Foundation.h>

#import "TyphoonMethod.h"

@class TyphoonDefinition;
@class TyphoonRuntimeArguments;

/**
* @ingroup Definition
* Describes the lifecycle of a Typhoon component.

* <strong>TyphoonScopeObjectGraph</strong>
* (default) This scope is essential (and unique to Typhoon) for mobile and desktop applications. When a component is resolved, any
* dependencies with the object-graph will be treated as shared instances during resolution. Once resolution is complete they are not
* retained by the TyphoonComponentFactory. This allows instantiating an entire object graph for a use-case (say for a ViewController), and
* then discarding it when that use-case has completed, therefore making efficient use of memory.
*
* <strong>TyphoonScopePrototype</strong>
* Indicates that a new instance should always be created by Typhoon, whenever this component is obtained from an assembly or referenced by
* another component.
*
* <strong>TyphoonScopeSingleton</strong>
Indicates that Typhoon should retain the instance that exists for as long as the TyphoonComponentFactory exists.
*
* <strong>TyphoonScopeWeakSingleton</strong>
Indicates that a shared instance should be created as long as necessary. When your application's classes stop referencing this component it
will be deallocated until needed again.
*
*/
typedef NS_ENUM(NSInteger, TyphoonScope)
{
    TyphoonScopeObjectGraph,
    TyphoonScopePrototype,
    TyphoonScopeSingleton,
    TyphoonScopeLazySingleton,
    TyphoonScopeWeakSingleton,
};


typedef NS_OPTIONS(NSInteger, TyphoonAutoInjectVisibility)
{
    TyphoonAutoInjectVisibilityNone = 0,
    TyphoonAutoInjectVisibilityByClass = 1 << 0,
    TyphoonAutoInjectVisibilityByProtocol = 1 << 1,
};

const static TyphoonAutoInjectVisibility TyphoonAutoInjectVisibilityDefault = TyphoonAutoInjectVisibilityByClass | TyphoonAutoInjectVisibilityByProtocol;

typedef void(^TyphoonDefinitionBlock)(TyphoonDefinition *definition);

/**
* @ingroup Definition
*/
@interface TyphoonDefinition : NSObject <NSCopying>
{
    Class _type;
    NSString *_key;
    TyphoonMethod *_initializer;
    NSMutableSet *_injectedProperties;
    NSMutableSet *_injectedMethods;
    TyphoonScope _scope;
    TyphoonDefinition *_factory;
    TyphoonDefinition *_parent;
}

@property(nonatomic, readonly) Class type;

/**
* A custom callback method that is invoked before property injection occurs. Use this method as an alternative to
* TyphoonPropertyInjectionDelegate if the component being instantiated is a 3rd party library, or if a direct dependency on Typhoon is not
* desired.
*
* @see TyphoonPropertyInjectionDelegate
*/
@property(nonatomic) SEL beforeInjections;

/**
* A custom callback method that is invoked after property injection occurs. Use this method as an alternative to
* TyphoonPropertyInjectionDelegate if the component being instantiated is a 3rd party library, or if a direct dependency on Typhoon is not
* desired.
*
* @see TyphoonPropertyInjectionDelegate
*/
@property(nonatomic) SEL afterInjections;

/**
* The scope of the component, default being TyphoonScopeObjectGraph.
*/
@property(nonatomic) TyphoonScope scope;

/**
* Specifies visibility for for AutoInjection.
*
* AutoInjection performs when using method:
* - (void)injectProperty:(SEL)withSelector;
* or when using:
* InjectedClass or InjectedProtocol marco
*/
@property(nonatomic) TyphoonAutoInjectVisibility autoInjectionVisibility;

/**
* A component that will produce an instance (with or without parameters) of this component. For example:
*
@code

- (id)sqliteManager
{
    return [TyphoonDefinition withClass:[MySqliteManager class] configuration:^(TyphoonDefinition* definition)
    {
        [definition useInitializer:@selector(initWithDatabaseName:) parameters:^(TyphoonMethod* initializer)
        {
            [initializer injectParameterWith:@"database.sqlite"];
        }];
        definition.scope = TyphoonScopeSingleton;
    }];
}

- (id)databaseQueue
{
    return [TyphoonDefinition withClass:[FMDatabaseQueue class] configuration:^(TyphoonDefinition* definition)
    {
        [definition useInitializer:@selector(queue)];
        definition.factory = [self sqliteManager];
    }];
}

@endcode
*
* @note If the factory method takes arguments, these are provided in the initializer block, just like a regular initializer method.
*
* @see injectProperty:withDefinition:selector: An alternative short-hand approach for no-args instances.
* @see injectProperty:withDefinition:keyPath: An alternative short-hand approach for no-args instances.
* @see TyphoonFactoryProvider - For creating factories where the configuration arguments are not known until runtime.
*
*
*/
@property(nonatomic, strong) id factory;

/**
* A parent component. When parent is defined the initializer and/or properties from a definition are inherited, unless overridden. Example:
*
@code

- (id)signUpClient
{
    return [TyphoonDefinition withClass:[SignUpClientDefaultImpl class] configuration:^(TyphoonDefinition* definition)
    {
        definition.parent = [self abstractClient];
    }];
}

- (id)abstractClient
{
    return [TyphoonDefinition withClass:[ClientBase class] configuration:^(TyphoonDefinition* definition)
    {
        [definition injectProperty:@selector(serviceUrl) with:TyphoonConfig(@"service.url"];
        [definition injectProperty:@selector(networkMonitor) with:[self internetMonitor]];
        [definition injectProperty:@selector(allowInvalidSSLCertificates) with:@(YES)];
    }];
}

@endcode
*
* @see abstract
*
*/
@property(nonatomic, strong) id parent;

/**
* If set, designates that a component can not be instantiated directly.
*
* @see parent
*/
@property(nonatomic) BOOL abstract;


/* ====================================================================================================================================== */
#pragma mark Factory methods

+ (id)withClass:(Class)clazz;

+ (id)withClass:(Class)clazz configuration:(TyphoonDefinitionBlock)injections;

+ (id)withFactory:(id)definition selector:(SEL)selector;

+ (id)withFactory:(id)definition selector:(SEL)selector parameters:(void (^)(TyphoonMethod *factoryMethod))parametersBlock;

/* ====================================================================================================================================== */
#pragma mark Injection

/**
* Injects property with a component from the container that matches the type (class or protocol) of the property.
*/
- (void)injectProperty:(SEL)withSelector;

/**
 * Injects property for gives selector with injection, where injection can be
 * - obtained from Injection* functions
 * - another definition
 * - assembly or collaboration assembly reference (TyphoonComponentFactory will be injected)
 * - object instance
 */
- (void)injectProperty:(SEL)selector with:(id)injection;

/** 
 * Injects method specified by selector with parameters.
 * @see TyphoonMethod documentation for information about parameters
 */
- (void)injectMethod:(SEL)selector parameters:(void (^)(TyphoonMethod *method))parametersBlock;

/**
 * Injects initializer specified by selector and parameters. 
 * Initializer allow you to create object with special selector and params. Without this injection,
 * object will be created by 'alloc-init' calls
 */
- (void)useInitializer:(SEL)selector parameters:(void (^)(TyphoonMethod *initializer))parametersBlock;

/**
* Convenience method to use a no-args initializer.
*
*/
- (void)useInitializer:(SEL)selector;

#pragma mark Making injections from definition

/**
 * Returns injection which can be used for example in injectProperty:with: method
 * This method will injects result of selector invocation
 * @param selector selector to invoke on resolved definition
 */
- (id)property:(SEL)selector;

/**
 * Returns injection which can be used for example in injectProperty:with: method
 * This method will injects valueForKeyPath: with given keyPath
 * @param keyPath path used as argument while calling valueForKeyPath: on resolved definition
 */
- (id)keyPath:(NSString *)keyPath;

@end

@interface TyphoonDefinition(Unavailable)

@property(nonatomic, assign, getter = isLazy) BOOL lazy __attribute((unavailable("Use TyphoonScopeLazySingleton instead")));

+ (id)withClass:(Class)clazz factory:(id)definition selector:(SEL)selector __attribute((unavailable("Use withFactory:selector: method instead")));
+ (instancetype)configDefinitionWithResource:(id)resource __attribute__((unavailable("Use configDefinitionWithName instead")));
+ (instancetype)configDefinitionWithResources:(NSArray *)array __attribute__((unavailable("Use configDefinitionWithName instead")));

@end