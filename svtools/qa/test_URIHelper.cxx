/*************************************************************************
 *
 *  $RCSfile: test_URIHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2002-08-22 14:33:34 $
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
#include "rtl/strbuf.hxx"
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

    struct Test
    {
        char const * sInput;
        char const * sResult;
        xub_StrLen nBegin;
        xub_StrLen nEnd;
    };
    static Test const aTests[]
        = {{ "...ftp://bla.bla.bla/blubber/...",
             "ftp://bla.bla.bla/blubber/", 3, 29 },
           { "..\\ftp://bla.bla.bla/blubber/...", 0, 0, 0 },
           { "..\\ftp:\\\\bla.bla.bla\\blubber/...",
             "file://bla.bla.bla/blubber%2F", 7, 29 },
           { "http://sun.com", "http://sun.com/", 0, 14 },
           { "http://sun.com/", "http://sun.com/", 0, 15 },
           { "http://www.xerox.com@www.pcworld.com/go/3990332.htm", 0, 0, 0 },
           { "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm",
             "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm", 0, 50 },
           { "Version.1.2.3", 0, 0, 0 },
           { "Version:1.2.3", 0, 0, 0 },
           { "a.b.c", 0, 0, 0 },
           { "file:///a|...", "file:///a:", 0, 10 },
           { "file:///a||...", "file:///a%7C%7C", 0, 11 },
           { "file:///a|/bc#...", "file:///a:/bc", 0, 13 },
           { "file:///a|/bc#de...", "file:///a:/bc#de", 0, 16 },
           { "abc.def.ghi,ftp.xxx.yyy/zzz...",
             "ftp://ftp.xxx.yyy/zzz", 12, 27 },
           { "abc.def.ghi,Ftp.xxx.yyy/zzz...",
             "ftp://Ftp.xxx.yyy/zzz", 12, 27 },
           { "abc.def.ghi,www.xxx.yyy...", "http://www.xxx.yyy/", 12, 23 },
           { "abc.def.ghi,wwww.xxx.yyy...", 0, 0, 0 },
           { "abc.def.ghi,wWW.xxx.yyy...", "http://wWW.xxx.yyy/", 12, 23 },
           { "Bla {mailto.me@abc.def.g.h.i}...",
             "mailto:%7Bmailto.me@abc.def.g.h.i", 4, 28 },
           { "abc@def@ghi", 0, 0, 0 },
           { "lala@sun.com", "mailto:lala@sun.com", 0, 12 },
           { "1lala@sun.com", "mailto:1lala@sun.com", 0, 13 },
           { "aaa_bbb@xxx.yy", "mailto:aaa_bbb@xxx.yy", 0, 14 },
           { "{a:\\bla/bla/bla...}", "file:///a:/bla/bla/bla", 1, 15 },
           { "#b:/c/d#e#f#", "file:///b:/c/d", 1, 7 },
           { "a:/", "file:///a:/", 0, 3 } };

    bool bReturn = true;
    for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
    {
        rtl::OUString aInput(rtl::OUString::createFromAscii(aTests[i].sInput));
        xub_StrLen nBegin = 0;
        xub_StrLen nEnd = static_cast< xub_StrLen >(aInput.getLength());
        rtl::OUString aResult(URIHelper::FindFirstURLInText(aInput, nBegin,
                                                            nEnd, aClass));
        bool bSuccess = aTests[i].sResult == 0
            ? (aResult.getLength() == 0
               && nBegin == aInput.getLength() && nEnd == aInput.getLength())
            : (aResult.equalsAscii(aTests[i].sResult)
               && nBegin == aTests[i].nBegin && nEnd == aTests[i].nEnd);
        rtl::OStringBuffer aBuffer;
        aBuffer.append('"');
        aBuffer.append(aTests[i].sInput);
        aBuffer.append(RTL_CONSTASCII_STRINGPARAM("\" -> "));
        aBuffer.append(aTests[i].sResult == 0 ? "none" : aTests[i].sResult);
        aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" ("));
        aBuffer.append(static_cast< sal_Int32 >(aTests[i].nBegin));
        aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
        aBuffer.append(static_cast< sal_Int32 >(aTests[i].nEnd));
        aBuffer.append(')');
        if (!bSuccess)
        {
            aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" != "));
            aBuffer.append(rtl::OUStringToOString(aResult,
                                                  RTL_TEXTENCODING_UTF8));
            aBuffer.append(RTL_CONSTASCII_STRINGPARAM(" ("));
            aBuffer.append(static_cast< sal_Int32 >(nBegin));
            aBuffer.append(RTL_CONSTASCII_STRINGPARAM(", "));
            aBuffer.append(static_cast< sal_Int32 >(nEnd));
            aBuffer.append(')');
        }
        pTestResult->pFuncs->state_(
            pTestResult, bSuccess, "test_URIHelper_FindFirstURLInText",
            aBuffer.getStr(), false);
        bReturn = bReturn && bSuccess;
    }
    return bReturn;
}
