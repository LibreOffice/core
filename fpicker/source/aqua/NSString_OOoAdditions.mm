/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#include "NSString_OOoAdditions.hxx"

#define CLASS_NAME "NSString"

@implementation NSString (OOoAdditions) 

+ (id) stringWithOUString:(const rtl::OUString&)ouString
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "ouString", ouString);

    NSString *string = [[NSString alloc] initWithOUString:ouString];

    DBG_PRINT_EXIT(CLASS_NAME, __func__, string);
    return [string autorelease];
}

- (id) initWithOUString:(const rtl::OUString&)ouString
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "ouString", ouString);
    if ((self = [super init])) {
        self = [self initWithCharacters:ouString.getStr() length:ouString.getLength()];
        
        DBG_PRINT_EXIT(CLASS_NAME, __func__, self);
        
        return self;
    }
    
    DBG_PRINT_EXIT(CLASS_NAME, __func__, self);
    return nil;
}

- (rtl::OUString) OUString
{
    unsigned int nFileNameLength = [self length];

    UniChar unichars[nFileNameLength+1];

    //'close' the string buffer correctly
    unichars[nFileNameLength] = '\0';

    [self getCharacters:unichars];

    return rtl::OUString(unichars);
}

@end
