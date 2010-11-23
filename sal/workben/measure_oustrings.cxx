/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Caolán McNamara <caolanm:redhat.com> ]
 * Portions created by Caolán McNamara are Copyright (C) 2010 * Red Hat, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include <sal/main.h>
#include <rtl/ustring.hxx>
#include <osl/time.h>

#include <iostream>

#if defined __GXX_EXPERIMENTAL_CXX0X__
#   define HAVE_CXX_Ox 1
#endif

#ifdef HAVE_CXX_Ox
#  define SAL_DECLARE_UTF16(str) u ## str
#elif (defined(__SUNPRO_CC) && __SUNPRO_CC >= 0x550)
#  define SAL_DECLARE_UTF16(str) U ## str
#elif __SIZEOF_WCHAR_T__ == 2
#  define SAL_DECLARE_UTF16(str) L ## str
#endif

#ifdef SAL_DECLARE_UTF16
#  define RTL_CONSTASCII_USTRINGPARAM_WIDE(str) \
     reinterpret_cast<const sal_Unicode*>(SAL_DECLARE_UTF16(str)), SAL_N_ELEMENTS(str)
#endif

SAL_IMPLEMENT_MAIN()
{
    //get my cpu fan up to speed :-)
    for (int i = 0; i < 10000000; ++i)
    {
        rtl::OUString sFoo(rtl::OUString::createFromAscii("X"));
        rtl::OUString sBar(RTL_CONSTASCII_USTRINGPARAM("X"));
#ifdef SAL_DECLARE_UTF16
        rtl::OUString sBoo(RTL_CONSTASCII_USTRINGPARAM_WIDE("X"));
#endif
        rtl::OUString sBaz(static_cast<sal_Unicode>('X'));
        rtl::OUString sNone;
    }

    std::cout << "--Empty Strings--" << std::endl;

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sFoo;
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString() " << nEndTime - nStartTime << "ms" << std::endl;
    }

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sFoo(rtl::OUString::createFromAscii(""));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString::createFromAscii() " << nEndTime - nStartTime << "ms" << std::endl;
    }

    std::cout << "--Single Chars--" << std::endl;

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sFoo(rtl::OUString::createFromAscii("X"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString::createFromAscii(\"X\") " << nEndTime - nStartTime << "ms" << std::endl;
    }

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sBar(RTL_CONSTASCII_USTRINGPARAM("X"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\"X\")) " << nEndTime - nStartTime << "ms" << std::endl;
    }

#ifdef SAL_DECLARE_UTF16
    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sBar(RTL_CONSTASCII_USTRINGPARAM_WIDE("X"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString(RTL_CONSTASCII_USTRINGPARAM_WIDE(\"X\")) " << nEndTime - nStartTime<< "ms" << std::endl;
    }
#endif

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sBaz(static_cast<sal_Unicode>('X'));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString(static_cast<sal_Unicode>('X') " << nEndTime - nStartTime << "ms" << std::endl;
    }

    std::cout << "--MultiChar Strings--" << std::endl;
    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sFoo(rtl::OUString::createFromAscii("XXXXXXXXXXXXXXX"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString::createFromAscii(\"XXXXXXXX\") " << nEndTime - nStartTime << "ms" << std::endl;
    }

    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sBar(RTL_CONSTASCII_USTRINGPARAM("XXXXXXXXXXXXXXX"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\"XXXXXXXX\")) " << nEndTime - nStartTime << "ms" << std::endl;
    }

#ifdef SAL_DECLARE_UTF16
    {
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    for (int i = 0; i < 100000000; ++i)
        rtl::OUString sBar(RTL_CONSTASCII_USTRINGPARAM_WIDE("XXXXXXXXXXXXXXX"));
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << "rtl::OUString(RTL_CONSTASCII_USTRINGPARAM_WIDE(\"XXXXXXXX\")) " << nEndTime - nStartTime << "ms" << std::endl;
    }
#endif

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
