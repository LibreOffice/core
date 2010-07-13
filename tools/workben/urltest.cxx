/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <tools/inetmime.hxx>
#include <tools/urlobj.hxx>
#include "com/sun/star/util/XStringWidth.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "cppuhelper/implbase1.hxx"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <stdio.h>

using namespace com::sun;

std::ostream & operator <<(std::ostream & out, rtl::OUString const & value) {
    out << rtl::OUStringToOString(value, RTL_TEXTENCODING_ASCII_US).getStr();
    return out;
}

namespace {

template< typename T1, typename T2 > bool assertEqual(
    rtl::OUString const & message, T1 const & expected, T2 const & actual)
{
    bool success = expected == actual;
    if (!success) {
        std::cout
            << "FAILED " << message << ": " << expected << " != " << actual
            << '\n';
    }
    return success;
}

}

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
    String aTest;
    for (std::size_t i = 0; i < nSize; ++i)
    {
        if (pTest[i].m_pBase)
        {
            aBase.SetURL(pTest[i].m_pBase);
        }
        if (aBase.HasError())
        {
            printf(" BAD BASE %s\n",
                   pTest[i].m_pBase ? pTest[i].m_pBase : "");
            bSuccess = false;
            continue;
        }
        INetURLObject aAbs;
        aBase.GetNewAbsURL(pTest[i].m_pRel, &aAbs);
        ByteString aTheAbs(String(aAbs.GetMainURL(INetURLObject::NO_DECODE)),
                           RTL_TEXTENCODING_ISO_8859_1);
        if (!(aTheAbs.Equals(pTest[i].m_pAbs)
              || pTest[i].m_pAlt && aTheAbs.Equals(pTest[i].m_pAlt)))
        {
            printf(" BAD GetNewAbsURL %s -> %s (%s)\n", pTest[i].m_pRel,
                   aTheAbs.GetBuffer(), pTest[i].m_pAbs);
            bSuccess = false;
        }
        aTheAbs = ByteString(
            String(
                INetURLObject::GetAbsURL(
                    aBase.GetMainURL(INetURLObject::NO_DECODE),
                    UniString(pTest[i].m_pRel, RTL_TEXTENCODING_ISO_8859_1))),
            RTL_TEXTENCODING_ISO_8859_1);
        if (!(aTheAbs.Equals(pTest[i].m_pAbs)
              || pTest[i].m_pAlt && aTheAbs.Equals(pTest[i].m_pAlt)))
        {
            printf(" BAD GetAbsURL %s -> %s (%s)\n", pTest[i].m_pRel,
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
                      equalsAscii(pTest[i].m_pUrl)
            && aUrl2.GetMainURL(INetURLObject::NO_DECODE).
                         equalsAscii(pTest[i].m_pUrl))
            printf("  ok %s %s -> %s\n",
                   ByteString(aPath, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
                   toString(pTest[i].m_eStyle), pTest[i].m_pUrl);
        else
        {
            String aTestA = aUrl1.GetMainURL(INetURLObject::NO_DECODE);
            String aTestB = aUrl2.GetMainURL(INetURLObject::NO_DECODE);

            printf(" BAD %s %s -> %s, %s (%s)\n",
                   ByteString(aPath, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
                   toString(pTest[i].m_eStyle),
                   ByteString(aTestA, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
                   ByteString(aTestB, RTL_TEXTENCODING_ISO_8859_1).GetBuffer(),
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
    sal_Int32 nMax = aObj.GetMainURL(INetURLObject::NO_DECODE).getLength() + 10;
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

bool test_getSegmentCount(
    char const * url, bool ignoreFinalSlash, sal_Int32 result)
{
    return
        assertEqual(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test_getSegmentCount")),
            result,
            INetURLObject(rtl::OUString::createFromAscii(url)).getSegmentCount(
                ignoreFinalSlash));
}

bool test_insertName(
    char const * url, char const * name, bool appendFinalSlash, sal_Int32 index,
    bool ignoreFinalSlash, bool success, char const * result)
{
    INetURLObject tmp(rtl::OUString::createFromAscii(url));
    return
        assertEqual(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test_insertName")),
            success,
            tmp.insertName(
                rtl::OUString::createFromAscii(name), appendFinalSlash, index,
                ignoreFinalSlash)) &
        assertEqual(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test_insertName")),
            rtl::OUString::createFromAscii(result),
            tmp.GetMainURL(INetURLObject::NO_DECODE));
}

bool test_removeSegment(
    char const * url, sal_Int32 index, bool ignoreFinalSlash, bool success,
    char const * result)
{
    INetURLObject tmp(rtl::OUString::createFromAscii(url));
    return
        assertEqual(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test_removeSegment")),
            success, tmp.removeSegment(index, ignoreFinalSlash)) &
        assertEqual(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test_removeSegment")),
            rtl::OUString::createFromAscii(result),
            tmp.GetMainURL(INetURLObject::NO_DECODE));
}

}

int
#if defined WNT
__cdecl
#endif // WNT
main()
{
    bool bSuccess = true;

    if (true)
    {
        // The data for this test is taken from the files
        // <http://www.ics.uci.edu/~fielding/url/testN.html> with N = 1,
        // ..., 3, as of August 28, 2000:
        static RelToAbsTest const aTest[]
            = { //{ "http://a/b/c/d;p?q", "g:h", "g:h", 0 },
                { "http://a/b/c/d;p?q", "g", "http://a/b/c/g", 0 },
                { 0, "./g", "http://a/b/c/g", 0 },
                { 0, "g/", "http://a/b/c/g/", 0 },
                { 0, "/g", "http://a/g", 0 },
                { 0, "//g", "http://g", "http://g/" },
                //{ 0, "?y", "http://a/b/c/d;p?y", 0 },
                { 0, "g?y", "http://a/b/c/g?y", 0 },
                //{ 0, "#s", "http://a/b/c/d;p?q#s", 0 },
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
                //{ 0, "", "http://a/b/c/d;p?q", 0 },
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
                //{ 0, "http:", "http:", 0 },
                { "http://a/b/c/d;p?q=1/2", "g", "http://a/b/c/g", 0 },
                { 0, "./g", "http://a/b/c/g", 0 },
                { 0, "g/", "http://a/b/c/g/", 0 },
                { 0, "/g", "http://a/g", 0 },
                { 0, "//g", "http://g", "http://g/" },
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
                { 0, "../../g", "http://a/b/g", 0 },
                { "file:///", "generic:", "file:///generic:", 0 },
                { 0, "generic:#fragment", "file:///generic:#fragment", 0 },
                { 0, "generic:something", "generic:something", 0 },
                { 0, "c:/foo/bar", "file:///c:/foo/bar", 0 },
                { 0, "c:\\foo\\bar", "file:///c:%5Cfoo%5Cbar", 0 } };
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
        struct Test { char const * in; char const * out; };
        static Test const aTest[]
            = { { "vnd.sun.star.help://", "vnd.sun.star.help:///" },
                { "vnd.sun.star.help:///", 0 },
                { "vnd.sun.star.help://swriter",
                  "vnd.sun.star.help://swriter/" },
                { "vnd.sun.star.help://swriter/", 0 },
                { "vnd.sun.star.help://swriter/12345", 0 },
                { "vnd.sun.star.help://swriter/1234X", 0 },
                { "vnd.sun.star.help://swriter/?a=b?c=d", 0 },
                { "vnd.sun.star.help://swriter/12345?a=b?c=d", 0 },
                { "vnd.sun.star.help://swriter/12345???", 0 },
                { "vnd.sun.star.help://swriter/#xxx", 0 },
                { "vnd.sun.star.help://swriter/12345#xxx", 0 },
                { "vnd.sun.star.help://swriter/1234X#xxx", 0 },
                { "vnd.sun.star.help://swriter/?a=b?c=d#xxx", 0 },
                { "vnd.sun.star.help://swriter/12345?a=b?c=d#xxx", 0 },
                { "vnd.sun.star.help://swriter/12345???#xxx", 0 },
                { "vnd.sun.star.help://swriter/start", 0 },
                { "vnd.sun.star.help://swriter/s/t/a/r/t", 0 },
                { "vnd.sun.star.help://swriter/a%2Fb%3Fc%2534d/e?f", 0 },
                { "vnd.sun.star.help://swriter?foo",
                  "vnd.sun.star.help://swriter/?foo" },
                { "vnd.sun.star.help://swriter/?foo", 0 } };
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i].in);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i].in);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         equalsAscii(
                             aTest[i].out == 0 ? aTest[i].in : aTest[i].out)
                             != sal_True)
            {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s\n",
                       aTest[i].in,
                       ByteString(sTest, RTL_TEXTENCODING_ASCII_US).
                       GetBuffer());
            }
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
                "vnd.sun.star.generic:///x/y/z",
                "vnd.sun.star.generic://host:34/x/y/z"
                /*TODO "wfs://",*/
                /*TODO "wfs://LocalHost",*/
                /*TODO "wfs:///c|/xyz/",*/
                /*TODO "wfs://xxx/yyy?zzz",*/
                /*TODO "wfs:///x/y/z"*/ };
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else
            {
                if (aUrl.GetProtocol() != INET_PROT_GENERIC) {
                    printf("BAD PROTOCOL %i -> %i\n",
                           aUrl.GetProtocol(),
                           INET_PROT_GENERIC);
                }
                if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         equalsAscii(aTest[i]) != sal_True)
                {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer());
                }
            }
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
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         equalsAscii(aTest[i]) != sal_True)
            {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer());
            }
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
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i]);
            if (aUrl.HasError())
                printf("BAD %s\n", aTest[i]);
            else if (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                         equalsAscii(aTest[i]) != sal_True)
            {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s\n",
                       aTest[i],
                       ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer());
            }
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
        {
            bool bWasAbsolute;
            if (!rtl::OUString(INetURLObject(rtl::OUString(
                                                 RTL_CONSTASCII_USTRINGPARAM(
                                                     "file:///"))).
                                   smartRel2Abs(
                                           rtl::OUString(
                                               RTL_CONSTASCII_USTRINGPARAM(
                                                   "generic:")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM("file:///generic:"))
                || bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"generic:\")\n");
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
                                                   "generic:#fragment")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM(
                             "file:///generic:#fragment"))
                || bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"generic:#fragment\")\n");
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
                                                   "generic:something")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM("generic:something"))
                || !bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"generic:something\")\n");
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
                                                   "\\\\unc_host\\path")),
                                           bWasAbsolute).
                                       GetMainURL(INetURLObject::NO_DECODE)).
                     equalsAsciiL(
                         RTL_CONSTASCII_STRINGPARAM("file://unc_host/path"))
                || !bWasAbsolute)
            {
                printf("BAD smartRel2Abs(\"\\\\unc_host\\path\")\n");
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
        abbreviate(INetURLObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "http://foo/aa/bb//cc/d/eee////ff/ggggggg/hhhhhh/iii/j/"
            "kkkkkkkkkkkkk/ll/mm/nn/oo/p"))));
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
                { "news: @def", "news:%20@def" },
                { "vnd.sun.star.tdoc:", 0 },
                { "vnd.sun.star.tdoc:a/b/c", 0 },
                { "vnd.sun.star.tdoc:/", "vnd.sun.star.tdoc:/" },
                { "vnd.sun.star.tdoc:/a;b/", "vnd.sun.star.tdoc:/a%3Bb/" },
                { "vnd.sun.star.tdoc:/a?b", "vnd.sun.star.tdoc:/a%3Fb" },
                { "http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/x",
                  "http://[fedc:ba98:7654:3210:fedc:ba98:7654:3210]:80/x" },
                { "http://[1080:0:0:0:8:800:200C:417A]/index.html",
                  "http://[1080:0:0:0:8:800:200c:417a]/index.html" },
                { "http://[3ffe:2a00:100:7031::1]",
                  "http://[3ffe:2a00:100:7031::1]/" },
                { "http://[1080::8:800:200c:417a]/foo",
                  "http://[1080::8:800:200c:417a]/foo" },
                { "http://[::192.9.5.5]/ipng", "http://[::192.9.5.5]/ipng" },
                { "http://[:::192.9.5.5]/ipng", "http://[:::192.9.5.5]/ipng" },
                { "http://[::FFFF:129.144.52.38]:80/index.html",
                  "http://[::ffff:129.144.52.38]:80/index.html" },
                { "http://[2010:836B:4179::836B:4179]",
                  "http://[2010:836b:4179::836b:4179]/" },
                { "http://[::1]", "http://[::1]/" },
                { "http://[0:0:0:0:0:0:13.1.68.3]/",
                  "http://[0:0:0:0:0:0:13.1.68.3]/" },
                { "http://[0:0:0:0:0:FFFF:129.144.52.38]/",
                  "http://[0:0:0:0:0:ffff:129.144.52.38]/" },
                { "smb://", "smb:///" },
                { "smb://foo", "smb://foo/" },
                { "smb://x;foo:bar@baz.xyz:12345/ab?cd",
                  "smb://x;foo:bar@baz.xyz:12345/ab?cd" } };
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl(aTest[i].m_pInput);
            if (aTest[i].m_pOutput == 0
                ? !aUrl.HasError()
                : (aUrl.HasError()
                   || (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                           equalsAscii(aTest[i].m_pOutput)
                       != sal_True)))
            {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s != %s\n",
                       aTest[i].m_pInput,
                       aUrl.HasError() ? "<none>"
                       : ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].m_pOutput == 0 ? "<none>" : aTest[i].m_pOutput);
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
            = { { "file://d:\\dir1\\file1", "file:///d:/dir1/file1" },
                { "http://as@alaska:8000/test/test.sxw", 0 } };
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
        {
            INetURLObject aUrl = INetURLObject(
                String(aTest[i].m_pInput, RTL_TEXTENCODING_UTF8),
                INET_PROT_HTTP);
            if (aTest[i].m_pOutput == 0
                ? !aUrl.HasError()
                : (aUrl.HasError()
                   || (aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI).
                           equalsAscii(aTest[i].m_pOutput)
                       != sal_True)))
            {
                String sTest(aUrl.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD %s -> %s != %s\n",
                       aTest[i].m_pInput,
                       aUrl.HasError() ? "<none>"
                       : ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].m_pOutput == 0 ? "<none>" : aTest[i].m_pOutput);
            }
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
            printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c%%3Fabc/def%%3F>:"
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
                printf("BAD <vnd.sun.star.pkg://foo.bar/a/b/c%%3Fabc/def%%3F>:"
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
        for (std::size_t i = 0; i < sizeof aTest / sizeof aTest[0]; ++i)
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
                                equalsAscii(aTest[i].pUri)
                            != sal_True)))
            {
                String sTest(aUri.GetMainURL(INetURLObject::DECODE_TO_IURI));
                printf("BAD ConcatData(%d, ..., %s) -> %s != %s\n",
                       static_cast< int >(aTest[i].eScheme), aTest[i].pPath,
                       aUri.HasError() ? "<none>"
                       : ByteString(sTest, RTL_TEXTENCODING_ASCII_US).GetBuffer(),
                       aTest[i].pUri == 0 ? "<none>" : aTest[i].pUri);
            }
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

    if (true) { // #112130#
        INetURLObject url1(rtl::OUString::createFromAscii(".uno:abc%3Fdef"));
        if (url1.GetProtocol() != INET_PROT_UNO) {
            printf("BAD .uno:abc%%3Fdef\n");
            bSuccess = false;
        }
        if (!rtl::OUString(url1.GetURLPath(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("abc%3Fdef"))) {
            printf(
                "BAD GetURLPath(.uno:abc%%3Fdef): %s\n",
                rtl::OUStringToOString(
                    url1.GetURLPath(INetURLObject::NO_DECODE),
                    osl_getThreadTextEncoding()).getStr());
            bSuccess = false;
        }
        if (url1.HasParam()) {
            printf("BAD HasParam(.uno:abc%%3Fdef)\n");
            bSuccess = false;
        }
        INetURLObject url2(rtl::OUString::createFromAscii(".uno:abc?def?ghi"));
        if (url2.GetProtocol() != INET_PROT_UNO) {
            printf("BAD .uno:abc?def?ghi\n");
            bSuccess = false;
        }
        if (!rtl::OUString(url2.GetURLPath(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("abc"))) {
            printf(
                "BAD GetURLPath(.uno:abc?def?ghi): %s\n",
                rtl::OUStringToOString(
                    url2.GetURLPath(INetURLObject::NO_DECODE),
                    osl_getThreadTextEncoding()).getStr());
            bSuccess = false;
        }
        if (!url2.HasParam()) {
            printf("BAD HasParam(.uno:abc?def?ghi)\n");
            bSuccess = false;
        }
        if (!rtl::OUString(url2.GetParam(INetURLObject::NO_DECODE)).
                equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("def?ghi"))) {
            printf(
                "BAD GetURLPath(.uno:abc?def?ghi): %s\n",
                rtl::OUStringToOString(
                    url2.GetParam(INetURLObject::NO_DECODE),
                    osl_getThreadTextEncoding()).getStr());
            bSuccess = false;
        }
    }

    if (true) { // #116269#
        rtl::OUString url;
        INetURLObject urlobj;

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("A-b.3:/%2f?x#y"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, INET_PROT_GENERIC, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a-b.3:/%2F?x#y")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));
        bSuccess &= assertEqual(url, false, urlobj.HasUserData());
        bSuccess &= assertEqual(url, false, urlobj.hasPassword());
        bSuccess &= assertEqual(url, false, urlobj.HasPort());
        bSuccess &= assertEqual(
            url, rtl::OUString(), rtl::OUString(urlobj.GetHost()));
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/%2F?x")),
            rtl::OUString(urlobj.GetURLPath()));
        bSuccess &= assertEqual(url, false, urlobj.HasParam());
        bSuccess &= assertEqual(url, true, urlobj.HasMark());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("y")),
            rtl::OUString(urlobj.GetMark()));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, true, urlobj.HasError());

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:#"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, true, urlobj.HasError());

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:/"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, INET_PROT_GENERIC, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, url,
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".foo:/"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, true, urlobj.HasError());

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("C:\\bla"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, true, urlobj.HasError());

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("C:\\bla"));
        urlobj = INetURLObject(url, INET_PROT_FILE);
        bSuccess &= assertEqual(url, INET_PROT_FILE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///C:/bla")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LPR:\\bla"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(url, INET_PROT_GENERIC, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("lpr:%5Cbla")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter"));
        urlobj = INetURLObject(url);
        bSuccess &= assertEqual(
            url, INET_PROT_PRIV_SOFFICE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, url,
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("A-b.3:/%2f?x#y"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_GENERIC, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a-b.3:/%2F?x#y")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_CID, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cid:foo:")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:#"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_CID, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cid:foo:#")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foo:/"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_GENERIC, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, url,
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".foo:/"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_CID, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cid:.foo:/")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("C:\\bla"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(url, INET_PROT_FILE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///C:/bla")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        url = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter"));
        urlobj = INetURLObject(url, INET_PROT_CID);
        bSuccess &= assertEqual(
            url, INET_PROT_PRIV_SOFFICE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url, url,
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));

        // #i80134#:
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\\\foobar\\%20#"));
        urlobj = INetURLObject(url, INET_PROT_FILE);
        bSuccess &= assertEqual(url, INET_PROT_FILE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("file://foobar/%2520%23")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\\\foo_bar\\%20#"));
        urlobj = INetURLObject(url, INET_PROT_FILE);
        bSuccess &= assertEqual(url, INET_PROT_FILE, urlobj.GetProtocol());
        bSuccess &= assertEqual(
            url,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("file://foo_bar/%2520%23")),
            rtl::OUString(urlobj.GetMainURL(INetURLObject::NO_DECODE)));
    }

    if (true) { // #i53184#
        rtl::OUString url(RTL_CONSTASCII_USTRINGPARAM("file://comp_name/path"));
        bSuccess &= assertEqual(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("#i53184# smart INET_PROT_FILE")),
            INetURLObject(url, INET_PROT_FILE).GetMainURL(
                INetURLObject::NO_DECODE),
            url);
        bSuccess &= assertEqual(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("#i53184# strict")),
            INetURLObject(url).GetMainURL(INetURLObject::NO_DECODE), url);
    }

    if (true) {
        rtl::OUString path;
        path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/a/b/c"));
        bSuccess &= assertEqual(
            path,
            rtl::OUString(
                INetURLObject(path, INetURLObject::FSYS_DETECT).GetMainURL(
                    INetURLObject::NO_DECODE)),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///a/b/c")));
        path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a\\b\\c"));
        bSuccess &= assertEqual(
            path,
            rtl::OUString(
                INetURLObject(path, INetURLObject::FSYS_DETECT).GetMainURL(
                    INetURLObject::NO_DECODE)),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///a/b/c")));
        path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a:b:c"));
        bSuccess &= assertEqual(
            path, INetURLObject(path, INetURLObject::FSYS_DETECT).HasError(),
            true);
        bSuccess &= assertEqual(
            path,
            rtl::OUString(
                INetURLObject(
                    path,
                    INetURLObject::FSysStyle(
                        INetURLObject::FSYS_DETECT | INetURLObject::FSYS_MAC)).
                GetMainURL(INetURLObject::NO_DECODE)),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///a/b/c")));
        rtl::OUString url;
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/a/b/c"));
        bSuccess &= assertEqual(
            url,
            rtl::OUString(
                INetURLObject(url, INET_PROT_HTTP).GetMainURL(
                    INetURLObject::NO_DECODE)),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///a/b/c")));
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a:\\b\\c"));
        bSuccess &= assertEqual(
            url,
            rtl::OUString(
                INetURLObject(url, INET_PROT_HTTP).GetMainURL(
                    INetURLObject::NO_DECODE)),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///a:/b/c")));
        url = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a:b:c"));
        bSuccess &= assertEqual(
            url, INetURLObject(url, INET_PROT_HTTP).HasError(), true);
        bSuccess &= assertEqual(
            url,
            (INetURLObject(
                url, INET_PROT_HTTP, INetURLObject::WAS_ENCODED,
                RTL_TEXTENCODING_UTF8,
                INetURLObject::FSysStyle(
                    INetURLObject::FSYS_DETECT | INetURLObject::FSYS_MAC)).
             HasError()),
            true);
    }

    bSuccess &= test_getSegmentCount("mailto:a@b", false, 0);
    bSuccess &= test_getSegmentCount("vnd.sun.star.expand:$PREFIX", false, 1);
    bSuccess &= test_getSegmentCount("vnd.sun.star.expand:$PREFIX", true, 1);
    bSuccess &= test_getSegmentCount("vnd.sun.star.expand:$PREFIX/", false, 2);
    bSuccess &= test_getSegmentCount("vnd.sun.star.expand:$PREFIX/", true, 1);
    bSuccess &= test_getSegmentCount(
        "vnd.sun.star.expand:$PREFIX/foo", false, 2);
    bSuccess &= test_getSegmentCount(
        "vnd.sun.star.expand:$PREFIX/foo", true, 2);
    bSuccess &= test_getSegmentCount("file:///", false, 1);
    bSuccess &= test_getSegmentCount("file:///", true, 0);
    bSuccess &= test_getSegmentCount("file:///foo", false, 1);
    bSuccess &= test_getSegmentCount("file:///foo", true, 1);

    bSuccess &= test_insertName(
        "mailto:a@b", "foo", false, 0, false, false, "mailto:a@b");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false, 0, false, true,
        "vnd.sun.star.expand:%2Ffoo/$PREFIX");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false, 0, true, true,
        "vnd.sun.star.expand:%2Ffoo/$PREFIX");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true, 0, false, true,
        "vnd.sun.star.expand:%2Ffoo/$PREFIX");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true, 0, true, true,
        "vnd.sun.star.expand:%2Ffoo/$PREFIX");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false, 1, false, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false, 1, true, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true, 1, false, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true, 1, true, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false,
        INetURLObject::LAST_SEGMENT, false, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", false,
        INetURLObject::LAST_SEGMENT, true, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true,
        INetURLObject::LAST_SEGMENT, false, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX", "foo", true,
        INetURLObject::LAST_SEGMENT, true, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", false,
        1, false, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", false,
        1, true, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", true,
        1, false, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", true,
        1, true, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", false,
        INetURLObject::LAST_SEGMENT, false, true,
        "vnd.sun.star.expand:$PREFIX//foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", false,
        INetURLObject::LAST_SEGMENT, true, true,
        "vnd.sun.star.expand:$PREFIX/foo");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", true,
        INetURLObject::LAST_SEGMENT, false, true,
        "vnd.sun.star.expand:$PREFIX//foo/");
    bSuccess &= test_insertName(
        "vnd.sun.star.expand:$PREFIX/", "foo", true,
        INetURLObject::LAST_SEGMENT, true, true,
        "vnd.sun.star.expand:$PREFIX/foo/");
    bSuccess &= test_insertName(
        "file:///", "foo", false, 0, false, true, "file:///foo/");
    bSuccess &= test_insertName(
        "file:///", "foo", false, 0, true, true, "file:///foo");
    bSuccess &= test_insertName(
        "file:///", "foo", true, 0, false, true, "file:///foo/");
    bSuccess &= test_insertName(
        "file:///", "foo", true, 0, true, true, "file:///foo/");
    bSuccess &= test_insertName(
        "file:///bar", "foo", false, 0, false, true, "file:///foo/bar");
    bSuccess &= test_insertName(
        "file:///bar", "foo", false, 0, true, true, "file:///foo/bar");
    bSuccess &= test_insertName(
        "file:///bar", "foo", true, 0, false, true, "file:///foo/bar");
    bSuccess &= test_insertName(
        "file:///bar", "foo", true, 0, true, true, "file:///foo/bar");

    bSuccess &= test_removeSegment("mailto:a@b", 0, false, false, "mailto:a@b");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 0, false, false,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 0, true, true,
        "vnd.sun.star.expand:%2F");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 1, false, false,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 1, true, false,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 2, false, false,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", 2, true, false,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", INetURLObject::LAST_SEGMENT, false,
        false, "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX", INetURLObject::LAST_SEGMENT, true, true,
        "vnd.sun.star.expand:%2F");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 0, false, true,
        "vnd.sun.star.expand:%2F");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 0, true, true,
        "vnd.sun.star.expand:%2F");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 1, false, true,
        "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 1, true, true,
        "vnd.sun.star.expand:$PREFIX/");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 2, false, false,
        "vnd.sun.star.expand:$PREFIX/");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", 2, true, false,
        "vnd.sun.star.expand:$PREFIX/");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", INetURLObject::LAST_SEGMENT, false,
        true, "vnd.sun.star.expand:$PREFIX");
    bSuccess &= test_removeSegment(
        "vnd.sun.star.expand:$PREFIX/", INetURLObject::LAST_SEGMENT, true,
        true, "vnd.sun.star.expand:%2F");
    bSuccess &= test_removeSegment("file:///", 0, false, true, "file:///");
    bSuccess &= test_removeSegment("file:///", 0, true, true, "file:///");
    bSuccess &= test_removeSegment("file:///", 1, false, false, "file:///");
    bSuccess &= test_removeSegment("file:///", 1, true, false, "file:///");
    bSuccess &= test_removeSegment("file:///", 2, false, false, "file:///");
    bSuccess &= test_removeSegment("file:///", 2, true, false, "file:///");
    bSuccess &= test_removeSegment(
        "file:///", INetURLObject::LAST_SEGMENT, false, true, "file:///");
    bSuccess &= test_removeSegment(
        "file:///", INetURLObject::LAST_SEGMENT, true, false, "file:///");
    bSuccess &= test_removeSegment("file:///foo", 0, false, true, "file:///");
    bSuccess &= test_removeSegment("file:///foo", 0, true, true, "file:///");
    bSuccess &= test_removeSegment(
        "file:///foo", 1, false, false, "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo", 1, true, false, "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo", 2, false, false, "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo", 2, true, false, "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo", INetURLObject::LAST_SEGMENT, false, true, "file:///");
    bSuccess &= test_removeSegment(
        "file:///foo", INetURLObject::LAST_SEGMENT, true, true, "file:///");
    bSuccess &= test_removeSegment("file:///foo/", 0, false, true, "file:///");
    bSuccess &= test_removeSegment("file:///foo/", 0, true, true, "file:///");
    bSuccess &= test_removeSegment(
        "file:///foo/", 1, false, true, "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo/", 1, true, true, "file:///foo/");
    bSuccess &= test_removeSegment(
        "file:///foo/", 2, false, false, "file:///foo/");
    bSuccess &= test_removeSegment(
        "file:///foo/", 2, true, false, "file:///foo/");
    bSuccess &= test_removeSegment(
        "file:///foo/", INetURLObject::LAST_SEGMENT, false, true,
        "file:///foo");
    bSuccess &= test_removeSegment(
        "file:///foo/", INetURLObject::LAST_SEGMENT, true, true, "file:///");

    return bSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}
