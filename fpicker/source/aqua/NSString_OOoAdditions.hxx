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



#ifndef _NSSTRING_OOOADDITIONS_HXX_
#define _NSSTRING_OOOADDITIONS_HXX_

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#include <rtl/ustring.hxx>
// #include <sal/types.h>

//for Cocoa types
@interface NSString (OOoAdditions)
+ (id) stringWithOUString:(const rtl::OUString&)ouString;
- (id) initWithOUString:(const rtl::OUString&)ouString;
- (rtl::OUString) OUString;
@end

#endif // _NSSTRING_OOOADDITIONS_HXX_
