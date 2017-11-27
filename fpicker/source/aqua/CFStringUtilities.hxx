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
#define _CFSTRINGUTILITIES_HXX_

#include <premac.h>
#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

enum InfoType {
    FULLPATH,
    FILENAME,
    PATHWITHOUTLASTCOMPONENT
};

rtl::OUString CFStringToOUString(const CFStringRef sOrig);
rtl::OUString FSRefToOUString(FSRef fsRef, InfoType info = FULLPATH);
rtl::OUString CFURLRefToOUString(CFURLRef aUrlRef, InfoType info);
CFStringRef CFStringCreateWithOUString(const rtl::OUString& aString);

//debug utils
#define PARAMFILLER "\n                    "

inline void DBG_PRINT_ENTRY() {
    OSL_TRACE(">>> %s", __func__);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname) {
    OSL_TRACE(">>> %s::%s", classname, methodname);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1) {
    OSL_TRACE(">>> %s::%s%s%s = %s", classname, methodname, PARAMFILLER, param1, value1);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const rtl::OUString& value1) {
    OSL_TRACE(">>> %s::%s%s%s = %s", classname, methodname, PARAMFILLER, param1, OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr());
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const CFStringRef value1) {
    OSL_TRACE(">>> %s::%s%s%s =", classname, methodname, PARAMFILLER, param1);
#if OSL_DEBUG_LEVEL > 1
    CFShow(value1);
#endif
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const NSString* value1) {
    OSL_TRACE(">>> %s::%s%s%s =", classname, methodname, PARAMFILLER, param1);
#if OSL_DEBUG_LEVEL > 1
    NSLog( const_cast<NSString*>(value1));
#endif
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1) {
    OSL_TRACE(">>> %s::%s%s%s = %d", classname, methodname, PARAMFILLER, param1, value1);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1, const char* param2, const int value2) {
    OSL_TRACE(">>> %s::%s%s%s = %s%s%s = %d", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1, const char* param2, const char* value2) {
    OSL_TRACE(">>> %s::%s%s%s = %s%s%s = %s", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const int value2) {
    OSL_TRACE(">>> %s::%s%s%s = %d%s%s = %d", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const char* value2) {
    OSL_TRACE(">>> %s::%s%s%s = %d%s%s = %s", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const CFStringRef value2) {
    OSL_TRACE(">>> %s::%s%s%s = %d%s%s =", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
#if OSL_DEBUG_LEVEL > 1
    CFShow(value2);
#endif
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const NSString* value2) {
    OSL_TRACE(">>> %s::%s%s%s = %d%s%s =", classname, methodname, PARAMFILLER, param1, value1, PARAMFILLER, param2, value2);
#if OSL_DEBUG_LEVEL > 1
    NSLog( const_cast<NSString*>(value2));
#endif
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const rtl::OUString& value1, const char* param2, const rtl::OUString& value2) {
    OSL_TRACE(">>> %s::%s%s%s = %s%s%s = %s", classname, methodname, PARAMFILLER, param1, OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr(), PARAMFILLER, param2, OUStringToOString(value2, RTL_TEXTENCODING_UTF8).getStr());
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const rtl::OUString& value1, const char* param2, const int value2) {
    OSL_TRACE(">>> %s::%s%s%s = %s%s%s = %d", classname, methodname, PARAMFILLER, param1, OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr(), PARAMFILLER, param2, value2);
}

//exit method debugs

inline void DBG_PRINT_EXIT() {
    OSL_TRACE("<<< %s", __func__);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname) {
    OSL_TRACE("<<< %s::%s", classname, methodname);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const char* retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = %s", classname, methodname, PARAMFILLER, retVal);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, int retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = %d", classname, methodname, PARAMFILLER, retVal);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const CFStringRef retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = ", classname, methodname, PARAMFILLER);
#if OSL_DEBUG_LEVEL > 1
    CFShow(retVal);
#endif
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const NSString* retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = ", classname, methodname, PARAMFILLER);
#if OSL_DEBUG_LEVEL > 1
    NSLog( const_cast<NSString*>(retVal));
#endif
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const rtl::OUString& retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = %s", classname, methodname, PARAMFILLER, OUStringToOString(retVal, RTL_TEXTENCODING_UTF8).getStr());
}

#endif //_CFSTRINGUTILITIES_HXX_
