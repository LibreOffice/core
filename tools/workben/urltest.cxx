/*************************************************************************
 *
 *  $RCSfile: urltest.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:39:57 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef TOOLS_INETMIME_HXX
#include "inetmime.hxx"
#endif
#ifndef _URLOBJ_HXX
#include "urlobj.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_XSTRINGWIDTH_HPP_
#include "com/sun/star/util/XStringWidth.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include "cppuhelper/implbase1.hxx"
#endif
#ifndef _RTL_STRING_H_
#include "rtl/string.h"
#endif
#ifndef _RTL_STRING_HXX_
#include "rtl/string.hxx"
#endif
#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif
#ifndef _RTL_USTRING_H_
#include "rtl/ustring.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#include <cstddef>
#include <cstdlib>
#include <stdio.h>

using namespace com::sun;

//============================================================================
//
//  testRelToAbs
//
//============================================================================

struct RelToAbsTest
{
    sal_Char const * m_pBase;
    sal_Char const * m_pRel;
    sal_Char const * m_pAbs;
    sal_Char const * m_pAlt;
};

//============================================================================
bool testRelToAbs(RelToAbsTest const * pTest, std::size_t nSize)
{
    bool bSuccess = true;
    INetURLObject aBase;
    for (std::size_t i = 0; i < nSize; ++i)
    {
        if (pTest[i].m_pBase)
        {
            aBase.SetURL(pTest[i].m_pBase);
            if (!aBase.HasError())
                printf("Base %s\n",
                       ByteString(aBase.GetMainURL(INetURLObject::NO_DECODE),
                                  RTL_TEXTENCODING_ISO_8859_1).
                           GetBuffer());
        }
        if (aBase.HasError())
        {
            printf(" BAD BASE %s\n",
                   pTest[i].m_pBase ? pTest[i].m_pBase : "");
            continue;
        }
        INetURLObject aAbs;
        aBase.GetNewAbsURL(pTest[i].m_pRel, &aAbs);
        ByteString aTheAbs(aAbs.GetMainURL(INetURLObject::NO_DECODE),
                           RTL_TEXTENCODING_ISO_8859_1);
        if (aTheAbs.Equals(pTest[i].m_pAbs)
            || pTest[i].m_pAlt && aTheAbs.Equals(pTest[i].m_pAlt))
            printf("  ok %s -> %s\n", pTest[i].m_pRel, aTheAbs.GetBuffer());
        else
        {
            printf(" BAD %s -> %s (%s)\n", pTest[i].m_pRel,
                   aTheAbs.GetBuffer(), pTest[i].m_pAbs);
            bSuccess = false;
        }
    }
    printf("\n");
    return bSuccess;
}

//============================================================================
//
//  testSetFSys
//
//============================================================================

struct SetFSysTest
{
    sal_Char const * m_pPath;
    INetURLObject::FSysStyle m_eStyle;
    sal_Char const * m_pUrl;
};

//============================================================================
inline sal_Char const * toString(INetURLObject::FSysStyle eStyle)
{
    static sal_Char aBuffer[5];
    int i = 0;
    if (eStyle & INetURLObject::FSYS_VOS)
        aBuffer[i++] = 'V';
    if (eStyle & INetURLObject::FSYS_UNX)
        aBuffer[i++] = 'U';
    if (eStyle & INetURLObject::FSYS_DOS)
        aBuffer[i++] = 'D';
    if (eStyle & INetURLObject::FSYS_MAC)
        aBuffer[i++] = 'M';
    if (i == 0)
        aBuffer[i++] = '-';
    aBuffer[i] = '\0';
    return aBuffer;
}

//============================================================================
bool testSetFSys(SetFSysTest const * pTest, std::size_t nSize)
{
    bool bSuccess = true;
    String aPath;
    for (std::size_t i = 0; i < nSize; ++i)
    {
        if (pTest[i].m_pPath)
            aPath = String::CreateFromAscii(pTest[i].m_pPath);
        if (aPath.Len() == 0)
        {
            printf(" NO PATH\n");
            continue;
        }
        INetURLObject aUrl1(aPath, pTest[i].m_eStyle);
        INetURLObject aUrl2;
        aUrl2.setFSysPath(aPath, pTest[i].m_eStyle);
        if (aUrl1.GetMainURL(INetURLObject::NO_DECODE).
                      EqualsAscii(pTest[i].m_pUrl)
            && aUrl2.GetMainURL(INetURLObject::NO_DECODE).
                         EqualsAscii(pTest[i].m_pUrl))
            printf("  ok %s %s -> %s\n",
                   ByteString(aPath, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
                   toString(pTest[i].m_eStyle), pTest[i].m_pUrl);
        else
        {
            printf(" BAD %s %s -> %s, %s (%s)\n",
                   ByteString(aPath, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
                   toString(pTest[i].m_eStyle),
                   ByteString(aUrl1.GetMainURL(INetURLObject::NO_DECODE),
                              RTL_TEXTENCODING_ISO_8859_1).
                       GetBuffer(),
                   ByteString(aUrl2.GetMainURL(INetURLObject::NO_DECODE),
                              RTL_TEXTENCODING_ISO_8859_1).
                       GetBuffer(),
                   pTest[i].m_pUrl);
            bSuccess = false;
        }
    }
    printf("\n");
    return bSuccess;
}

//============================================================================
//
//  main
//
//============================================================================

namespace {

class StringWidth: public cppu::WeakImplHelper1< star::util::XStringWidth >
{
public:
    virtual sal_Int32 SAL_CALL queryStringWidth(rtl::OUString const & rString)
        throw (star::uno::RuntimeException)
    {
        return rString.getLength();
    }
};

void abbreviate(INetURLObject aObj)
{
    star::uno::Reference< star::util::XStringWidth > xWidth(new StringWidth);
    sal_Int32 nMax = aObj.GetMainURL(INetURLObject::NO_DECODE).Len() + 10;
    for (sal_Int32 i = -10; i <= nMax; ++i)
    {
        rtl::OString
            aAbbreviated(rtl::OUStringToOString(
                             aObj.getAbbreviated(xWidth,
                                                 i,
                                                 INetURLObject::NO_DECODE),
                             RTL_TEXTENCODING_UTF8));
        printf(
            "%4ld: <%s", static_cast< long int >(i), aAbbreviated.getStr());
        for (sal_Int32 j = aAbbreviated.getLength(); j < i; ++j)
            printf(" ");
        printf(">\n");
    }
}

}

int
#if defined WNT
__cdecl
#endif // WNT
main()
{
    bool bSuccess = true;

    if (false)
    {
        // The data for this test is taken from the files
        // <http://www.ics.uci.edu/~fielding/url/testN.html> with N = 1,
        // ..., 3, as of August 28, 2000:
        static RelToAbsTest const aTest[]
            = { { "http://a/b/c/d;p?q", "g:h", "g:h", 0 },
                { 0, "g", "http://a/b/c/g", 0 },
                { 0, "./g", "http://a/b/c/g", 0 },
                { 0, "g/", "http://a/b/c/g/", 0 },
                { 0, "/g", "http://a/g", 0 },
                { 0, "//g", "http://g", 0 },
                { 0, "?y", "http://a/b/c/d;p?y", 0 },
                { 0, "g?y", "http://a/b/c/g?y", 0 },
                { 0, "#s", "http://a/b/c/d;p?q#s", 0 },
                { 0, "g#s", "http://a/b/c/g#s", 0 },
                { 0, "g?y#s", "http://a/b/c/g?y#s", 0 },
                { 0, ";x", "http://a/b/c/;x", 0 },
                { 0, "g;x", "http://a/b/c/g;x", 0 },
                { 0, "g;x?y#s", "http://a/b/c/g;x?y#s", 0 },
                { 0, ".", "http://a/b/c/", 0 },
                { 0, "./", "http://a/b/c/", 0 },
                { 0, "..", "http://a/b/", 0 },
                { 0, "../", "http://a/b/", 0 },
                { 0, "../g", "http://a/b/g", 0 },
                { 0, "../..", "http://a/", 0 },
                { 0, "../../", "http://a/", 0 },
                { 0, "../../g", "http://a/g", 0 },
                { 0, "", "http://a/b/c/d;p?q", 0 },
                { 0, "../../../g", "http://a/../g", "http://a/g" },
                { 0, "../../../../g", "http://a/../../g", "http://a/g" },
                { 0, "/./g", "http://a/./g", 0 },
                { 0, "/../g", "http://a/../g", 0 },
                { 0, "g.", "http://a/b/c/g.", 0 },
                { 0, ".g", "http://a/b/c/.g", 0 },
                { 0, "g..", "http://a/b/c/g..", 0 },
                { 0, "..g", "http://a/b/c/..g", 0 },
                { 0, "./../g", "http://a/b/g", 0 },
                { 0, "./g/.", "http://a/b/c/g/", 0 },
                { 0, "g/./h", "http://a/b/c/g/h", 0 },
                { 0, "g/../h", "http://a/b/c/h", 0 },
                { 0, "g;x=1/./y", "http://a/b/c/g;x=1/y", 0 },
                { 0, "g;x=1/../y", "http://a/b/c/y", 0 },
                { 0, "g?y/./x", "http://a/b/c/g?y/./x", 0 },
                { 0, "g?y/../x", "http://a/b/c/g?y/../x", 0 },
                { 0, "g#s/./x", "http://a/b/c/g#s/./x", 0 },
                { 0, "g#s/../x", "http://a/b/c/g#s/../x", 0 },
                { 0, "http:g", "http:g", "http://a/b/c/g" },
                { 0, "http:", "http:", 0 },
                { "http://a/b/c/d;p?q=1/2", "g", "http://a/b/c/g", 0 },
                { 0, "./g", "http://a/b/c/g", 0 },
                { 0, "g/", "http://a/b/c/g/", 0 },
                { 0, "/g", "http://a/g", 0 },
                { 0, "//g", "http://g", 0 },
                { 0, "g?y", "http://a/b/c/g?y", 0 },
                { 0, "g?y/./x", "http://a/b/c/g?y/./x", 0 },
                { 0, "g?y/../x", "http://a/b/c/g?y/../x", 0 },
                { 0, "g#s", "http://a/b/c/g#s", 0 },
                { 0, "g#s/./x", "http://a/b/c/g#s/./x", 0 },
                { 0, "g#s/../x", "http://a/b/c/g#s/../x", 0 },
                { 0, "./", "http://a/b/c/", 0 },
                { 0, "../", "http://a/b/", 0 },
                { 0, "../g", "http://a/b/g", 0 },
                { 0, "../../", "http://a/", 0 },
                { 0, "../../g", "http://a/g", 0 },
                { "http://a/b/c/d;p=1/2?q", "g", "http://a/b/c/d;p=1/g", 0 },
                { 0, "./g", "http://a/b/c/d;p=1/g", 0 },
                { 0, "g/", "http://a/b/c/d;p=1/g/", 0 },
                { 0, "g?y", "http://a/b/c/d;p=1/g?y", 0 },
                { 0, ";x", "http://a/b/c/d;p=1/;x", 0 },
                { 0, "g;x", "http://a/b/c/d;p=1/g;x", 0 },
                { 0, "g;x=1/./y", "http://a/b/c/d;p=1/g;x=1/y", 0 },
                { 0, "g;x=1/../y", "http://a/b/c/d;p=1/y", 0 },
                { 0, "./", "http://a/b/c/d;p=1/", 0 },
                { 0, "../", "http://a/b/c/", 0 },
                { 0, "../g", "http://a/b/c/g", 0 },
                { 0, "../../", "http://a/b/", 0 },
                { 0, "../../g", "http://a/b/g", 0 } };
        if (!testRelToAbs(aTest, sizeof aTest / sizeof (RelToAbsTest)))
            bSuccess = false;
    }

    if (false)
    {
        static SetFSysTest const aTest[]
            = { { "//.", INetURLObject::FSysStyle(0), "" },
                { 0, INetURLObject::FSysStyle(1), "file:///" },
                { 0, INetURLObject::FSysStyle(2), "file:////." },
                { 0, INetURLObject::FSysStyle(3), "file:///" },
                { 0, INetURLObject::FSysStyle(4), "file:///%2F%2F." },
                { 0, INetURLObject::FSysStyle(5), "file:///" },
                { 0, INetURLObject::FSysStyle(6), "file:////." },
                { 0, INetURLObject::FSysStyle(7), "file:///" },
                { 0, INetURLObject::FSysStyle(8), "file:///%2F%2F." },
                { 0, INetURLObject::FSysStyle(9), "file:///" },
                { 0, INetURLObject::FSysStyle(10), "file:////." },
                { 0, INetURLObject::FSysStyle(11), "file:///" },
                { 0, INetURLObject::FSysStyle(12), "file:///%2F%2F." },
                { 0, INetURLObject::FSysStyle(13), "file:///" },
                { 0, INetURLObject::FSysStyle(14), "file:////." },
                { 0, INetURLObject::FSysStyle(15), "file:///" },
                { "//./", INetURLObject::FSysStyle(0), "" },
                { 0, INetURLObject::FSysStyle(1), "file:///" },
                { 0, INetURLObject::FSysStyle(2), "file:////./" },
                { 0, INetURLObject::FSysStyle(3), "file:///" },
                { 0, INetURLObject::FSysStyle(4), "file:///%2F%2F.%2F" },
                { 0, INetURLObject::FSysStyle(5), "file:///" },
                { 0, INetURLObject::FSysStyle(6), "file:////./" },
                { 0, INetURLObject::FSysStyle(7), "file:///" },
                { 0, INetURLObject::FSysStyle(8), "file:///%2F%2F.%2F" },
                { 0, INetURLObject::FSysStyle(9), "file:///" },
                { 0, INetURLObject::FSysStyle(10), "file:////./" },
                { 0, INetURLObject::FSysStyle(11), "file:///" },
                { 0, INetURLObject::FSysStyle(12), "file:///%2F%2F.%2F" },
                { 0, INetURLObject::FSysStyle(13), "file:///" },
                { 0, INetURLObject::FSysStyle(14), "file:////./" },
                { 0, INetURLObject::FSysStyle(15), "file:///" },
                { "//./a/b\\c:d", INetURLObject::FSysStyle(0), "" },
                { 0, INetURLObject::FSysStyle(1), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(2), "file:////./a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(3), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(4), "file:///%2F%2F.%2Fa%2Fb/c:d" },
                { 0, INetURLObject::FSysStyle(5), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(6), "file:////./a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(7), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(8), "file:///%2F%2F.%2Fa%2Fb%5Cc/d" },
                { 0, INetURLObject::FSysStyle(9), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(10), "file:////./a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(11), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(12), "file:///%2F%2F.%2Fa%2Fb/c:d" },
                { 0, INetURLObject::FSysStyle(13), "file:///a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(14), "file:////./a/b%5Cc:d" },
                { 0, INetURLObject::FSysStyle(15), "file:///a/b%5Cc:d" } };
        if (!testSetFSys(aTest, sizeof aTest / sizeof (SetFSysTest)))
            bSuccess = false;
    }

/*
    if (false)
    {
        bool bAbs = false;
        INetURLObject aUrl1(INetURLObject().smartRel2Abs(L"/export/home/mba/Office/user/Basic/soffice.sbl", bAbs));

        INetURLObject aUrl2a(L"/export/home/mba/Office/user/Basic/soffice.sbl", INET_PROT_FILE);

        INetURLObject aUrl2b(L"file:///export/home/mba/Office/user/Basic/soffice.sbl", INET_PROT_FILE);

        INetURLObject aUrl3a(L"/export/home/mba/Office/user/Basic/soffice.sbl", INetURLObject::FSYS_DETECT);

        INetURLObject aUrl3b(L"file:///export/home/mba/Office/user/Basic/soffice.sbl", INetURLObject::FSYS_DETECT);
    }
*/

    if (true)
    {
        INetURLObject aUrl1("http://host:1234/xy/~zw?xxx=yyy");
        if (aUrl1.HasError())
        {
            printf("BAD http\n");
            bSuccess = false;
        }
        INetURLObject aUrl2("vnd.sun.star.webdav://host:1234/xy/~zw?xxx=yyy");
        if (aUrl2.HasError())
        {
            printf("BAD vnd.sun.star.webdav\n");
            bSuccess = false;
        }
    }

    if (true)
    {
        static sal_Char const * const aTest[]
            = { /*TODO "vnd.sun.star.help://",*/
                /*TODO "vnd.sun.star.help://swriter",*/
                "vnd.sun.star.help://swriter/",
                "vnd.sun.star.help://swriter/12345",
                "vnd.sun.star.help://swriter/1234X",
                "vnd.sun.star.help://swriter/?a=b?c=d",
                "vnd.sun.star.help://swriter/12345?a=b?c=d",
                "vnd.sun.star.help://swriter/12345???",
                "vnd.sun.star.help://swriter/#xxx",
                "vnd.sun.star.help://swriter/12345#xxx",
                "vnd.sun.star.help://swriter/1234X#xxx",
                "vnd.sun.star.help://swriter/?a=b?c=d#xxx",
                "vnd.sun.star.help://swriter/12345?a=b?c=d#xxx",
                "vnd.sun.star.help://swriter/12345???#xxx",
                "vnd.sun.star.help://swriter/start" };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         CompareToAscii(aTest[i]) != 0)
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(aUrl.GetMainURL(
                                      INetURLObject::DECODE_TO_IURI),
                                  RTL_TEXTENCODING_ASCII_US).
                           GetBuffer());
        }
    }

    if (true)
    {
        static sal_Char const * const aTest[]
            = { /*TODO "vnd.sun.star.wfs://",*/
                /*TODO "vnd.sun.star.wfs://LocalHost",*/
                /*TODO "vnd.sun.star.wfs:///c|/xyz/",*/
                /*TODO "vnd.sun.star.wfs://xxx/yyy?zzz",*/
                "vnd.sun.star.wfs:///x/y/z",
                /*TODO "wfs://",*/
                /*TODO "wfs://LocalHost",*/
                /*TODO "wfs:///c|/xyz/",*/
                /*TODO "wfs://xxx/yyy?zzz",*/
                /*TODO "wfs:///x/y/z"*/ };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         CompareToAscii(aTest[i]) != 0)
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(aUrl.GetMainURL(
                                      INetURLObject::DECODE_TO_IURI),
                                  RTL_TEXTENCODING_ASCII_US).
                           GetBuffer());
        }
    }

    if (true)
    {
        static sal_Char const * const aTest[]
            = { /*TODO "vnd.sun.star.pkg:",*/
                /*TODO "vnd.sun.star.pkg:/",*/
                /*TODO "vnd.sun.star.pkg://abc",*/
                /*TODO "vnd.sun.star.pkg://file:%2F%2F%2Fa:%2Fb%20c",*/
                "vnd.sun.star.pkg://file:%2F%2F%2Fa:%2Fb%20c/",
                "vnd.sun.star.pkg://file:%2F%2F%2Fa:%2Fb%20c/xx",
                /*TODO "vnd.sun.star.pkg://file:%2F%2F%2Fa:%2Fb%20c/xx;yy",*/
                "vnd.sun.star.pkg://file:%2F%2F%2Fa:%2Fb%20c/xx//yy" };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         CompareToAscii(aTest[i]) != 0)
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(aUrl.GetMainURL(
                                      INetURLObject::DECODE_TO_IURI),
                                  RTL_TEXTENCODING_ASCII_US).
                           GetBuffer());
        }
    }

    if (true)
    {
        static sal_Char const * const aTest[]
            = { /*TODO "vnd.sun.star.cmd:",*/
                /*TODO "vnd.sun.star.cmd:/",*/
                "vnd.sun.star.cmd:logout",
                "vnd.sun.star.cmd:log/out",
                /*TODO "vnd.sun.star.cmd:[logout]",*/
                "vnd.sun.star.cmd:log[out]" };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         CompareToAscii(aTest[i]) != 0)
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(aUrl.GetMainURL(
                                      INetURLObject::DECODE_TO_IURI),
                                  RTL_TEXTENCODING_ASCII_US).
                           GetBuffer());
        }
    }

    if (true)
    {
        rtl::OUString
            aParameters(rtl::OUString::createFromAscii("; CharSet=UTF-8  ; Blubber=Blob"));
        sal_Unicode const * pBegin = aParameters.getStr();
        sal_Unicode const * pEnd = pBegin + aParameters.getLength();
        INetContentTypeParameterList aList;
        if (INetMIME::scanParameters(pBegin, pEnd, &aList) == pEnd)
        {
            ULONG nCount = aList.Count();
            for (ULONG i = 0; i < nCount; ++i)
            {
                INetContentTypeParameter const * p = aList.GetObject(i);
                if (p)
                {
/*
                    printf("attribute: '%s'\n charset: '%s'\n language: '%s'\n value: '%s'\n converted: %s\n",
                           p->m_sAttribute.GetBuffer(),
                           p->m_sCharset.GetBuffer(),
                           p->m_sLanguage.GetBuffer(),
                           rtl::OUStringToOString(p->m_sValue,RTL_TEXTENCODING_UTF8).getStr(),
                           p->m_bConverted ? "true" : "false");
*/
                }
                else
                    printf("BAD INetContentTypeParameter\n");
            }
        }
        else
        {
            printf("BAD INetMIME::scanParameters()\n");
            bSuccess = false;
        }
    }

    if (true)
    {
        {
            INetURLObject aObj;
            aObj.setFSysPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a:")),
                             INetURLObject::FSYS_DETECT);
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:")))
            {
                printf("BAD setFSysPath(\"a:\")\n");
                bSuccess = false;
            }
        }
        {
            INetURLObject aObj;
            aObj.setFSysPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "a:/")),
                             INetURLObject::FSYS_DETECT);
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:/")))
            {
                printf("BAD setFSysPath(\"a:/\")\n");
                bSuccess = false;
            }
        }
        {
            INetURLObject aObj;
            aObj.setFSysPath(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "a:\\")),
                             INetURLObject::FSYS_DETECT);
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:/")))
            {
                printf("BAD setFSysPath(\"a:\\\")\n");
                bSuccess = false;
            }
        }

        if (!rtl::OUString(INetURLObject("file:///a:").
                               getFSysPath(INetURLObject::FSYS_DETECT)).
                 equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("a:")))
        {
            printf("BAD getFSysPath(\"file:///a:\")\n");
            bSuccess = false;
        }
        if (!rtl::OUString(INetURLObject("file:///a:/").
                               getFSysPath(INetURLObject::FSYS_DETECT)).
                 equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("a:\\")))
        {
            printf("BAD getFSysPath(\"file:///a:/\")\n");
            bSuccess = false;
        }

        {
            bool bWasAbsolute;
            if (!rtl::OUString(INetURLObject(rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "file:///"))).
                                   smartRel2Abs(
                                           rtl::OUString(
                                               RTL_CONSTASCII_USTRINGPARAM(
                                                   "a:")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:"))
                || !bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"a:\")\n");
                bSuccess = false;
            }
        }
        {
            bool bWasAbsolute;
            if (!rtl::OUString(INetURLObject(rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "file:///"))).
                                   smartRel2Abs(
                                           rtl::OUString(
                                               RTL_CONSTASCII_USTRINGPARAM(
                                                   "a:/")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:/"))
                || !bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"a:/\")\n");
                bSuccess = false;
            }
        }
        {
            bool bWasAbsolute;
            if (!rtl::OUString(INetURLObject(rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "file:///"))).
                                   smartRel2Abs(
                                           rtl::OUString(
                                               RTL_CONSTASCII_USTRINGPARAM(
                                                   "a:\\")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("file:///a:/"))
                || !bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"a:\\\")\n");
                bSuccess = false;
            }
        }
    }

    if (true)
    {
/*TODO
        {
            INetURLObject aObj(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("http://xxx/yyy?abc/def~")));
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("http://xxx/yyy?abc%2Fdef%7E")))
            {
                printf("BAD http query 1\n");
                bSuccess = false;
            }
        }
*/
        {
            INetURLObject aObj(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("https://xxx/yyy?abc/def~")));
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("https://xxx/yyy?abc/def~")))
            {
                printf("BAD https query 1\n");
                bSuccess = false;
            }
        }
/*TODO
        {
            INetURLObject aObj(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("http://xxx/yyy")));
            aObj.SetParam("abc/def~");
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("http://xxx/yyy?abc%2Fdef%7E")))
            {
                printf("BAD http query 2\n");
                bSuccess = false;
            }
        }
*/
        {
            INetURLObject aObj(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("https://xxx/yyy")));
            aObj.SetParam("abc/def~");
            if (!rtl::OUString(aObj.GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("https://xxx/yyy?abc/def~")))
            {
                printf("BAD https query 2\n");
                bSuccess = false;
            }
        }
    }

    if (true)
    {
        if (INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier:"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 1\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier://"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 2\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier:///"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 3\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier:///abc"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 4\n");
            bSuccess = false;
        }
        if (INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier://abc"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 5\n");
            bSuccess = false;
        }
        if (INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.hier://abc/def"))).HasError())
        {
            printf("BAD vnd.sun.star.hier test 6\n");
            bSuccess = false;
        }
    }

    if (true)
    {
        if (INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                            "vnd.sun.star.script:"))).
                HasError())
        {
            printf("BAD vnd.sun.star.script:\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                             "vnd.sun.star.script:abc"))).
                 HasError())
        {
            printf("BAD vnd.sun.star.script:abc\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                             "vnd.sun.star.script:a=b,c"))).
                 HasError())
        {
            printf("BAD vnd.sun.star.script:a=b,c\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(
                               RTL_CONSTASCII_USTRINGPARAM(
                                   "vnd.sun.star.script:a=b,c=d,"))).
                 HasError())
        {
            printf("BAD vnd.sun.star.script:a=b,c=d,\n");
            bSuccess = false;
        }
        if (!INetURLObject(rtl::OUString(
                               RTL_CONSTASCII_USTRINGPARAM(
                                   "vnd.sun.star.script:a=b,,c=d"))).
                 HasError())
        {
            printf("BAD vnd.sun.star.script:a=b,,c=d\n");
            bSuccess = false;
        }
        {
            INetURLObject
                aObj(rtl::OUString(
                         RTL_CONSTASCII_USTRINGPARAM(
                             "vnd.sun.star.script:A%3dB=,b%C3%A4h=b%c3%b6h,"
                                 "a=3,a=4,x=%3D%2C")));
            {
                bool bFound
                    = aObj.getParameter(rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "a=")),
                                        0);
                if (bFound)
                {
                    printf("BAD a=\n");
                    bSuccess = false;
                }
            }
            {
                UniString aValue;
                bool bFound
                    = aObj.getParameter(rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "a=b")),
                                        &aValue);
                if (!bFound
                    || !rtl::OUString(aValue).
                            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("")))
                {
                    printf("BAD a=b\n");
                    bSuccess = false;
                }
            }
            {
                UniString aValue;
                bool bFound
                    = aObj.getParameter(rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "A=B")),
                                        &aValue);
                if (!bFound
                    || !rtl::OUString(aValue).
                            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("")))
                {
                    printf("BAD A=B\n");
                    bSuccess = false;
                }
            }
            {
                sal_Unicode const aKey[] = { 'b', 0xE4, 'h' };
                sal_Unicode const aGoodValue[] = { 'b', 0xF6, 'h' };
                UniString aValue;
                bool bFound = aObj.getParameter(rtl::OUString(aKey, 3),
                                                &aValue);
                if (!bFound
                    || rtl::OUString(aValue) != rtl::OUString(aGoodValue, 3))
                {
                    printf("BAD b\\u00E4h\n");
                    bSuccess = false;
                }
            }
            {
                UniString aValue;
                bool bFound
                    = aObj.getParameter(rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "a")),
                                        &aValue);
                if (!bFound
                    || !rtl::OUString(aValue).
                            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("3")))
                {
                    printf("BAD a\n");
                    bSuccess = false;
                }
            }
            {
                UniString aValue;
                bool bFound
                    = aObj.getParameter(rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "x")),
                                        &aValue);
                if (!bFound
                    || !rtl::OUString(aValue).
                            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("=,")))
                {
                    printf("BAD a\n");
                    bSuccess = false;
                }
            }
        }
    }

    if (false)
    {
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///a"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///a/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///ab/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///abc/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///a/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///ab/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///abc/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///abcdef/d"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///abcdef/de"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file:///abcdef/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/a"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/a/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/ab/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/abc/def/"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/a/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/ab/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/abc/def"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/abcdef/d"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/abcdef/de"))));
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "file://some.host/abcdef/def"))));
    }

    if (true)
    {
        {
            rtl::OUString
                aBase(RTL_CONSTASCII_USTRINGPARAM("file:///a:/b/c"));
            rtl::OUString aAbs(RTL_CONSTASCII_USTRINGPARAM("file:///a:/d/e"));
            rtl::OUString aRel(INetURLObject::GetRelURL(aBase, aAbs));
            if (!aRel.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("../d/e")))
            {
                printf("BAD GetRelURL(%s, %s) = %s\n",
                       rtl::OUStringToOString(aBase, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aAbs, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aRel, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
        {
            rtl::OUString
                aBase(RTL_CONSTASCII_USTRINGPARAM("file:///a:/b/c"));
            rtl::OUString aAbs(RTL_CONSTASCII_USTRINGPARAM("file:///d:/e/f"));
            rtl::OUString aRel(INetURLObject::GetRelURL(aBase, aAbs));
            if (aRel != aAbs)
            {
                printf("BAD GetRelURL(%s, %s) = %s\n",
                       rtl::OUStringToOString(aBase, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aAbs, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aRel, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
        {
            rtl::OUString
                aBase(RTL_CONSTASCII_USTRINGPARAM("file:///a:/b/c"));
            rtl::OUString aAbs(RTL_CONSTASCII_USTRINGPARAM("file:///d/e/f"));
            rtl::OUString aRel(INetURLObject::GetRelURL(aBase, aAbs));
            if (!aRel.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("../../d/e/f")))
            {
                printf("BAD GetRelURL(%s, %s) = %s\n",
                       rtl::OUStringToOString(aBase, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aAbs, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aRel, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
        {
            rtl::OUString
                aBase(RTL_CONSTASCII_USTRINGPARAM("file:///a:/b/c"));
            rtl::OUString aAbs(RTL_CONSTASCII_USTRINGPARAM("file:///d:/e/f"));
            rtl::OUString
                aRel(INetURLObject::GetRelURL(aBase,
                                              aAbs,
                                              INetURLObject::WAS_ENCODED,
                                              INetURLObject::DECODE_TO_IURI,
                                              RTL_TEXTENCODING_UTF8,
                                              INetURLObject::FSYS_UNX));
            if (!aRel.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                       "../../d:/e/f")))
            {
                printf("BAD GetRelURL(%s, %s) = %s\n",
                       rtl::OUStringToOString(aBase, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aAbs, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aRel, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
/*TODO
        {
            rtl::OUString
                aBase(RTL_CONSTASCII_USTRINGPARAM("file:///test.html"));
            rtl::OUString
                aAbs(RTL_CONSTASCII_USTRINGPARAM("/images/myimage.gif"));
            rtl::OUString aRel(INetURLObject::GetRelURL(aBase, aAbs));
            if (aRel != aAbs)
            {
                printf("BAD GetRelURL(%s, %s) = %s\n",
                       rtl::OUStringToOString(aBase, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aAbs, RTL_TEXTENCODING_UTF8).
                           getStr(),
                       rtl::OUStringToOString(aRel, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
*/
    }

    if (true)
    {
        INetURLObject aUrl(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                             "file://host/dir/file")));
        rtl::OUString aPath;
        aPath = aUrl.getFSysPath(INetURLObject::FSYS_DETECT);
        if (!aPath.
                 equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("//host/dir/file")))
        {
            printf("BAD getFSysPath(VOS|UNX|DOS|MAC) = %s\n",
                   rtl::OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).
                       getStr());
            bSuccess = false;
        }
        aPath = aUrl.getFSysPath(INetURLObject::FSysStyle(
                                     INetURLObject::FSYS_UNX
                                         | INetURLObject::FSYS_DOS
                                         | INetURLObject::FSYS_MAC));
        if (!aPath.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                    "\\\\host\\dir\\file")))
        {
            printf("BAD getFSysPath(UNX|DOS|MAC) = %s\n",
                   rtl::OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).
                       getStr());
            bSuccess = false;
        }
        aPath = aUrl.getFSysPath(INetURLObject::FSysStyle(
                                     INetURLObject::FSYS_UNX
                                         | INetURLObject::FSYS_MAC));
        if (aPath.getLength() != 0)
        {
            printf("BAD getFSysPath(UNX|MAC) = %s\n",
                   rtl::OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).
                       getStr());
            bSuccess = false;
        }
    }

    if (true)
    {
        {
            INetURLObject aUrl1(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.odma:")));
            rtl::OUString aUrl2(aUrl1.GetMainURL(
                                    INetURLObject::DECODE_TO_IURI));
            if (!aUrl2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.odma:/")))
            {
                printf("BAD vnd.sun.star.odma: != %s\n",
                       rtl::OUStringToOString(aUrl2, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
        {
            INetURLObject aUrl1(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.odma:/")));
            rtl::OUString aUrl2(aUrl1.GetMainURL(
                                    INetURLObject::DECODE_TO_IURI));
            if (!aUrl2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.odma:/")))
            {
                printf("BAD vnd.sun.star.odma:/ != %s\n",
                       rtl::OUStringToOString(aUrl2, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
        {
            INetURLObject aUrl1(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.odma:/bla/bla")));
            rtl::OUString aUrl2(aUrl1.GetMainURL(
                                    INetURLObject::DECODE_TO_IURI));
            if (!aUrl2.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.odma:/bla%2Fbla")))
            {
                printf("BAD vnd.sun.star.odma:/bla/bla != %s\n",
                       rtl::OUStringToOString(aUrl2, RTL_TEXTENCODING_UTF8).
                           getStr());
                bSuccess = false;
            }
        }
    }

    if (true)
    {
        struct Test
        {
            char const * m_pInput;
            char const * m_pOutput;
        };
        static Test const aTest[]
            = { { "file:///abc", "file:///abc" },
                { "file://localhost/abc", "file:///abc" },
                { "file://LocalHost/abc", "file:///abc" },
                { "file://LOCALHOST/abc", "file:///abc" },
                { "file://127.0.0.1/abc", "file://127.0.0.1/abc" },
                { "file://xxx.yyy-zzz/abc", "file://xxx.yyy-zzz/abc" },
                { "file://xxx_yyy/abc", "file://xxx_yyy/abc" },
                { "file://!%23$%&'()-.@^_{}~/abc",
                  "file://!%23$%25&'()-.@%5E_%7B%7D~/abc" },
                { "file://d:\\dir1\\file1", 0 },
                { "http://as@alaska:8000/test/test.sxw", 0 },
                { "telnet:", 0 },
                { "telnet://", 0 },
                { "telnet://ab:cd@ef:", "telnet://ab:cd@ef:/" },
                { "telnet://ab:cd@ef:123", "telnet://ab:cd@ef:123/" },
                { "TELNET://abc.def.ghi/", "telnet://abc.def.ghi/" },
                { "telnet://abc.def.ghi/jkl", 0 },
                { "telnet://abc.def.ghi?jkl", 0 },
                { "telnet://abc.def.ghi/?jkl", 0 },
                { "file:", 0 },
                { "file:/", "file:///" },
                { "file:/abc", "file:///abc" },
                { "file:/abc/def", "file:///abc/def" },
                { "file:/localhost", "file:///localhost" },
                { "file://", "file:///" },
                { "file:///", "file:///" },
                { "http:", 0 },
                { "http:/abc", 0 },
                { "news:", 0 },
                { "news:*", "news:*" },
                { "news:**", 0 },
                { "news:%2A", 0 },
                { "news:a", "news:a" },
                { "news:A", "news:A" },
                { "news:+-._", 0 },
                { "news:A0+-._", "news:A0+-._" },
                { "news:0", 0 },
                { "news:AB,", 0 },
                { "news:abc@def", "news:abc@def" },
                { "news:abc@def:33", 0 },
                { "news:abc@123.456.789.0", "news:abc@123.456.789.0" },
                { "news:abc@def.", "news:abc@def." },
                { "news:abc@def.ghi", "news:abc@def.ghi" },
                { "news:abc@def.-ghi", 0 },
                { "news:abc@def.ghi@", 0 },
                { "news:%21%22%23@def", "news:%21%22%23@def" },
                { "news:!%22%23@def", "news:!%22%23@def" },
                { "news: @def", "news:%20@def" } };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i].m_pInput);
            if (aTest[i].m_pOutput == 0
                ? !aUrl.HasError()
                : (aUrl.HasError()
                   || (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                           CompareToAscii(aTest[i].m_pOutput)
                       != 0)))
                printf("BAD %s -> %s != %s\n",
                       aTest[i].m_pInput,
                       aUrl.HasError() ? "<none>"
                       : ByteString(aUrl.GetMainURL(
                                        INetURLObject::DECODE_TO_IURI),
                                    RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].m_pOutput == 0 ? "<none>" : aTest[i].m_pOutput);
        }
    }

    if (true)
    {
        struct Test
        {
            char const * m_pInput;
            char const * m_pOutput;
        };
        static Test const aTest[]
            = { { "file://d:\\dir1\\file1", "file:///d:/dir1/file1" },
                { "http://as@alaska:8000/test/test.sxw", 0 } };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(String(aTest[i].m_pInput, RTL_TEXTENCODING_UTF8),
                               INET_PROT_HTTP);
            if (aTest[i].m_pOutput == 0
                ? !aUrl.HasError()
                : (aUrl.HasError()
                   || (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                           CompareToAscii(aTest[i].m_pOutput)
                       != 0)))
                printf("BAD %s -> %s != %s\n",
                       aTest[i].m_pInput,
                       aUrl.HasError() ? "<none>"
                       : ByteString(aUrl.GetMainURL(
                                        INetURLObject::DECODE_TO_IURI),
                                    RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].m_pOutput == 0 ? "<none>" : aTest[i].m_pOutput);
        }
    }

    if (true)
    {
        INetURLObject aUrl;
        rtl::OUString aUser;
        aUrl = INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "ftp://test")));
        aUser = aUrl.GetUser();
        if (aUser.getLength() != 0)
            printf(
                "BAD <ftp://test> user: \"%s\" != \"\"",
                rtl::OUStringToOString(aUser, RTL_TEXTENCODING_UTF8).getStr());
        aUrl = INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "ftp://user@test")));
        aUser = aUrl.GetUser();
        if (!aUser.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("user")))
            printf(
                "BAD <ftp://user@test> user: \"%s\" != \"user\"",
                rtl::OUStringToOString(aUser, RTL_TEXTENCODING_UTF8).getStr());
    }

    if (true)
    {
        INetURLObject aUrl;

        aUrl = INetURLObject("vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?");
        if (aUrl.GetProtocol() != INET_PROT_VND_SUN_STAR_PKG)
            printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?>:"
                   " scheme = %d\n",
                   static_cast< int >(aUrl.GetProtocol()));
        else
        {
            if (!rtl::OUString(aUrl.GetMainURL(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                 "vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?")))
                printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?>:"
                       " URL = %s\n",
                       rtl::OUStringToOString(
                           aUrl.GetMainURL(INetURLObject::NO_DECODE),
                           RTL_TEXTENCODING_UTF8).getStr());
            if (!rtl::OUString(aUrl.GetParam(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("abc/def?")))
                printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?>:"
                       " query = %s\n",
                       rtl::OUStringToOString(
                           aUrl.GetParam(INetURLObject::NO_DECODE),
                           RTL_TEXTENCODING_UTF8).getStr());
        }

        aUrl = INetURLObject("vnd.sun.star.pkg://foo.bar/a/b/c%3Fabc/def%3F");
        if (aUrl.GetProtocol() != INET_PROT_VND_SUN_STAR_PKG)
            printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c%3Fabc/def%3F>:"
                   " scheme = %d\n",
                   static_cast< int >(aUrl.GetProtocol()));
        else
        {
            if (!rtl::OUString(aUrl.GetMainURL(INetURLObject::NO_DECODE)).
                equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "vnd.sun.star.pkg://foo.bar/a/b/c%3Fabc/def%3F")))
                printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c?abc/def?>:"
                       " URL = %s\n",
                       rtl::OUStringToOString(
                           aUrl.GetMainURL(INetURLObject::NO_DECODE),
                           RTL_TEXTENCODING_UTF8).getStr());
            if (!rtl::OUString(aUrl.GetParam(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("")))
                printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c%3Fabc/def%3F>:"
                       " query = %s\n",
                       rtl::OUStringToOString(
                           aUrl.GetParam(INetURLObject::NO_DECODE),
                           RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    if (true)
    {
        struct Test
        {
            INetProtocol eScheme;
            char const * pPath;
            char const * pUri;
        };
        static Test const aTest[]
            = { { INET_PROT_FILE, "", "file:///" },
                { INET_PROT_FILE, "/", "file:///" },
                { INET_PROT_FILE, "abc", 0 },
                { INET_PROT_FILE, "/abc/", "file:///abc/" },
                { INET_PROT_NEWS, "", 0 },
                { INET_PROT_NEWS, "*", "news:*" },
                { INET_PROT_NEWS, "**", 0 },
                { INET_PROT_NEWS, "%2A", 0 },
                { INET_PROT_NEWS, "a", "news:a" },
                { INET_PROT_NEWS, "A", "news:A" },
                { INET_PROT_NEWS, "+-._", 0 },
                { INET_PROT_NEWS, "A0+-._", "news:A0+-._" },
                { INET_PROT_NEWS, "0", 0 },
                { INET_PROT_NEWS, "AB,", 0 },
                { INET_PROT_NEWS, "abc@def", "news:abc@def" },
                { INET_PROT_NEWS, "abc@def:33", 0 },
                { INET_PROT_NEWS, "abc@123.456.789.0",
                  "news:abc@123.456.789.0" },
                { INET_PROT_NEWS, "abc@def.", "news:abc@def." },
                { INET_PROT_NEWS, "abc@def.ghi", "news:abc@def.ghi" },
                { INET_PROT_NEWS, "abc@def.-ghi", 0 },
                { INET_PROT_NEWS, "abc@def.ghi@", 0 },
                { INET_PROT_NEWS, "!\"#@def", "news:!%22%23@def" },
                { INET_PROT_NEWS, " @def", "news:%20@def" } };
        for (int i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUri;
            bool bOk = aUri.ConcatData(aTest[i].eScheme, String(), String(),
                                       String(), 0,
                                       String(aTest[i].pPath,
                                              RTL_TEXTENCODING_ASCII_US),
                                       INetURLObject::ENCODE_ALL);
            if (bOk == aUri.HasError())
                printf(
                    "BAD ConcatData(%d, ..., %s) = %d, HasError() = %d\n",
                    static_cast< int >(aTest[i].eScheme), aTest[i].pPath,
                    static_cast< int >(bOk),
                    static_cast< int >(aUri.HasError()));
            else if (aTest[i].pUri == 0
                     ? !aUri.HasError()
                     : (aUri.HasError()
                        || (aUri.GetMainURL(INetURLObject::DECODE_TO_IURI).
                                CompareToAscii(aTest[i].pUri)
                            != 0)))
                printf("BAD ConcatData(%d, ..., %s) -> %s != %s\n",
                       static_cast< int >(aTest[i].eScheme), aTest[i].pPath,
                       aUri.HasError() ? "<none>"
                       : ByteString(aUri.GetMainURL(
                                        INetURLObject::DECODE_TO_IURI),
                                    RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].pUri == 0 ? "<none>" : aTest[i].pUri);
        }
    }

    if (true)
    {
        // #i13760#

        // Test for unrelated URLs.
        const rtl::OUString aBaseURL(RTL_CONSTASCII_USTRINGPARAM(
                                "http://www.openoffice.org"));
        rtl::OUString aRelURL (RTL_CONSTASCII_USTRINGPARAM(
                                "http://www.sun.com"));

        rtl::OUString aRelURLToTest(
            INetURLObject::GetRelURL(aBaseURL, aRelURL));

        if (INetURLObject(aRelURLToTest) != INetURLObject(aRelURL))
            printf("BAD GetRelURL(%s, %s), ret = %s\n",
                       ByteString(aBaseURL.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       ByteString(aRelURL.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       ByteString(aRelURLToTest.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer());

        // Test for related URLs.
        aRelURL = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "http://www.openoffice.org/api/test.html"));
        aRelURLToTest = rtl::OUString(
            INetURLObject::GetRelURL(aBaseURL, aRelURL));

        if (!aRelURLToTest.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("api/test.html")))
            printf("BAD GetRelURL(%s, %s), ret = %s\n",
                       ByteString(aBaseURL.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       ByteString(aRelURL.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       ByteString(aRelURLToTest.getStr(),
                                  RTL_TEXTENCODING_ASCII_US).GetBuffer());
    }

    return bSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
