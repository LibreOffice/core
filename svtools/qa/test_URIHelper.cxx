/*************************************************************************
 *
 *  $RCSfile: test_URIHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2002-07-19 15:13:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/bootstrap.hxx"
#include "rtl/string.h"
#include "rtl/tres.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "tools/solar.h"
#include "unotools/charclass.hxx"

#include "urihelper.hxx"

extern "C" sal_Bool SAL_CALL test_URIHelper_FindFirstURLInText(
    rtl_TestResult * pTestResult)
{
    // This test needs an XMultiServiceFactory, so it needs an adequate
    // environment to be able to create one...

    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > xFactory;
    try
    {
        xFactory = com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >(
                cppu::defaultBootstrap_InitialComponentContext()->
                getServiceManager(),
                com::sun::star::uno::UNO_QUERY);
    }
    catch (com::sun::star::uno::Exception &)
    {
        pTestResult->pFuncs->state_(pTestResult, false,
                                    "test_URIHelper_FindFirstURLInText",
                                    "create XMultiServiceFactory", false);
        return false;
    }
    CharClass aClass(xFactory,
                     com::sun::star::lang::Locale(
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en")),
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US")),
                         rtl::OUString()));

    bool bReturn = true;

    {
        rtl::OUString aText(
            RTL_CONSTASCII_USTRINGPARAM("abc.def.ghi,www.xxx.yyy..."));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aText.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aText, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aResult.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                 "http://www.xxx.yyy/"))
            && nBegin == 12 && nEnd == 23;
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            "\"abc.def.ghi,www.xxx.yyy...\" -> http://www.xxx.yyy/", false);
        bReturn = bReturn && bSuccess;
    }

    {
        rtl::OUString aText(
            RTL_CONSTASCII_USTRINGPARAM("abc.def.ghi,wwww.xxx.yyy..."));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aText.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aText, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aResult.getLength() == 0
            && nBegin == static_cast< xub_StrLen >(aText.getLength())
            && nEnd == static_cast< xub_StrLen >(aText.getLength());
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            "\"abc.def.ghi,wwww.xxx.yyy...\" -> none", false);
        bReturn = bReturn && bSuccess;
    }

    {
        rtl::OUString aText(
            RTL_CONSTASCII_USTRINGPARAM("abc.def.ghi,wWW.xxx.yyy..."));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aText.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aText, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aResult.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                 "http://wWW.xxx.yyy/"))
            && nBegin == 12 && nEnd == 23;
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            "\"abc.def.ghi,wWW.xxx.yyy...\" -> http://wWW.xxx.yyy/", false);
        bReturn = bReturn && bSuccess;
    }

    {
        rtl::OUString aText(
            RTL_CONSTASCII_USTRINGPARAM("abc.def.ghi,ftp.xxx.yyy/zzz..."));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aText.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aText, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aResult.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                 "ftp://ftp.xxx.yyy/zzz"))
            && nBegin == 12 && nEnd == 27;
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            "\"abc.def.ghi,ftp.xxx.yyy/zzz...\" -> ftp://ftp.xxx.yyy/zzz",
            false);
        bReturn = bReturn && bSuccess;
    }

    {
        rtl::OUString aText(
            RTL_CONSTASCII_USTRINGPARAM("abc.def.ghi,Ftp.xxx.yyy/zzz..."));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aText.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aText, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aResult.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                 "ftp://Ftp.xxx.yyy/zzz"))
            && nBegin == 12 && nEnd == 27;
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            "\"abc.def.ghi,Ftp.xxx.yyy/zzz...\" -> ftp://Ftp.xxx.yyy/zzz",
            false);
        bReturn = bReturn && bSuccess;
    }

    return bReturn;
}
