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
#import "TyphoonComponentFactory.h"

@interface TyphoonComponentFactory (TyphoonDefinitionRegisterer)

- (TyphoonDefinition*)definitionForKey:(NSString*)key;

- (id)objectForDefinition:(TyphoonDefinition*)definition;

- (void)addDefinitionToRegistry:(TyphoonDefinition*)definition;

- (void)addComponentPostProcessor:(id<TyphoonComponentFactoryPostProcessor>)postProcessor;

@end