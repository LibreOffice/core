/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CFSTRINGUTILITIES_HXX_
#define _CFSTRINGUTILITIES_HXX_

#include <premac.h>
#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include "sal/log.hxx"

enum InfoType {
    FULLPATH,
    FILENAME,
    PATHWITHOUTLASTCOMPONENT
};

OUString CFStringToOUString(const CFStringRef sOrig);
OUString FSRefToOUString(FSRef fsRef, InfoType info = FULLPATH);
OUString CFURLRefToOUString(CFURLRef aUrlRef, InfoType info);
CFStringRef CFStringCreateWithOUString(const OUString& aString);

//debug utils
#define PARAMFILLER "\n                    "

inline void DBG_PRINT_ENTRY() {
    SAL_INFO("fpicker.aqua",">>> "<< __func__);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const OUString& value1) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr());
}

#if OSL_DEBUG_LEVEL > 1
inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const CFStringRef value1)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname<< PARAMFILLER << param1 <<" =");
    CFShow(value1);

}
#else
inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const CFStringRef /* value1 */)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " =");
}
#endif

#if OSL_DEBUG_LEVEL > 1
inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const NSString* value1)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " =");
    NSLog(const_cast<NSString*>(value1));
}
#else
inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const NSString* /* value1 */)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " =");
}
#endif

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1, const char* param2, const int value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 <<" = " << value1 << PARAMFILLER << param2 << " = " << value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const char* value1, const char* param2, const char* value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1 << PARAMFILLER << param2 << " = " << value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const int value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1 << PARAMFILLER << param2 << " = " << value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const char* value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1 << PARAMFILLER << param2 << " = " << value2);
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const CFStringRef value2)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1 << PARAMFILLER << param2 << " =" << value2);
#if OSL_DEBUG_LEVEL > 1
    CFShow(value2);
#endif
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const int value1, const char* param2, const NSString* value2)
{
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << value1 << PARAMFILLER << param2 << " =" << value2);
#if OSL_DEBUG_LEVEL > 1
    NSLog( const_cast<NSString*>(value2));
#endif
}


inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const OUString& value1, const char* param2, const OUString& value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname <<  PARAMFILLER << param1 << " = " << OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr() << PARAMFILLER << param2 << " = " << OUStringToOString(value2, RTL_TEXTENCODING_UTF8).getStr());
}

inline void DBG_PRINT_ENTRY(const char * classname, const char * methodname, const char* param1, const OUString& value1, const char* param2, const int value2) {
    SAL_INFO("fpicker.aqua",">>> " << classname << "::" << methodname << PARAMFILLER << param1 << " = " << OUStringToOString(value1, RTL_TEXTENCODING_UTF8).getStr() << PARAMFILLER << param2 <<" = " << value2);
}

//exit method debugs

inline void DBG_PRINT_EXIT() {
    SAL_INFO("fpicker.aqua","<<< " << __func__);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname) {
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const char* retVal) {
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = " << retVal);
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, int retVal) {
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = " << retVal);
}

#if OSL_DEBUG_LEVEL > 1
inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const CFStringRef retVal)
{
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = ");
    CFShow(retVal);
}
#else
inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const CFStringRef /* retVal */)
{
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = ");
}
#endif

#if OSL_DEBUG_LEVEL > 1
inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const NSString* retVal)
{
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = ");
    NSLog(const_cast<NSString*>(retVal));
}
#else
inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const NSString* /* retVal */ )
{
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = ");
}
#endif

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const OUString& retVal) {
    SAL_INFO("fpicker.aqua","<<< " << classname << "::" << methodname << PARAMFILLER << "returnValue = " << OUStringToOString(retVal, RTL_TEXTENCODING_UTF8).getStr());
}

#endif //_CFSTRINGUTILITIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
