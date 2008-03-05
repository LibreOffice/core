/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CFStringUtilities.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:34:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CFSTRINGUTILITIES_HXX_
#define _CFSTRINGUTILITIES_HXX_

#include <premac.h>
#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
    NSLog(value1);
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
    NSLog(value2);
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
    NSLog(retVal);
#endif
}

inline void DBG_PRINT_EXIT(const char * classname, const char * methodname, const rtl::OUString& retVal) {
    OSL_TRACE("<<< %s::%s%sreturnValue = %s", classname, methodname, PARAMFILLER, OUStringToOString(retVal, RTL_TEXTENCODING_UTF8).getStr());
}

#endif //_CFSTRINGUTILITIES_HXX_
