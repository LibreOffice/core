/*************************************************************************
 *
 *  $RCSfile: test_uriproc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:45:12 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uri/ExternalUriReferenceTranslator.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/VndSunStarPkgUrlReferenceFactory.hpp"
#include "com/sun/star/uri/XExternalUriReferenceTranslator.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarScriptUrlReference.hpp"
#include "cppuhelper/servicefactory.hxx"
#include "cppunit/simpleheader.hxx"
#include "osl/diagnose.h"
#include "osl/thread.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <cstdlib>

namespace css = com::sun::star;

namespace {

#define TEST_ASSERT_EQUAL(token1, token2, token3, expected, actual) \
    CPPUNIT_ASSERT_MESSAGE( \
        createTestAssertEqualMessage( \
            token1, token2, token3, #expected, #actual, expected, actual). \
            getStr(), \
        (actual) == (expected))

template< typename T > void append(
    rtl::OUStringBuffer & buffer, T const & value)
{
    buffer.append(value);
}

template<> void append(rtl::OUStringBuffer & buffer, bool const & value) {
    buffer.append(static_cast< sal_Bool >(value));
}

template<> void append(rtl::OUStringBuffer & buffer, std::size_t const & value)
{
    buffer.append(static_cast< sal_Int32 >(value));
}

template<> void append(rtl::OUStringBuffer & buffer, char const * const & value)
{
    buffer.appendAscii(value);
}

template< typename T1, typename T2, typename T3, typename T4 >
rtl::OString createTestAssertEqualMessage(
    char const * token1, T1 const & token2, T2 const & token3,
    char const * expectedExpr, char const * actualExpr, T3 const & expected,
    T4 const & actual)
{
    rtl::OUStringBuffer buf;
    buf.appendAscii(token1);
    buf.append(static_cast< sal_Unicode >('|'));
    append(buf, token2);
    buf.append(static_cast< sal_Unicode >('|'));
    append(buf, token3);
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(": TEST_ASSERT_EQUAL("));
    buf.appendAscii(expectedExpr);
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    buf.appendAscii(actualExpr);
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("): <"));
    append(buf, expected);
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("> != <"));
    append(buf, actual);
    buf.append(static_cast< sal_Unicode >('>'));
    return rtl::OUStringToOString(
        buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();

    virtual void tearDown();

    void testParse();

    void testMakeAbsolute();

    void testMakeRelative();

    void testVndSunStarScript();

    void testTranslator();

    void testPkgUrlFactory();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testMakeAbsolute);
    CPPUNIT_TEST(testMakeRelative);
    CPPUNIT_TEST(testVndSunStarScript);
    CPPUNIT_TEST(testTranslator);
    CPPUNIT_TEST(testPkgUrlFactory);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference< css::uno::XComponentContext > m_context;
    css::uno::Reference< css::uri::XUriReferenceFactory > m_uriFactory;
};

void Test::setUp() {
    char const * registry = getForwardString();
    css::uno::Reference< css::lang::XMultiServiceFactory > factory(
        cppu::createRegistryServiceFactory(
            rtl::OUString(
                registry, rtl_str_getLength(registry),
                osl_getThreadTextEncoding())));
        //TODO: check for string conversion failure
    css::uno::Reference< css::beans::XPropertySet >(
        factory, css::uno::UNO_QUERY_THROW)->getPropertyValue(
            rtl::OUString::createFromAscii("DefaultContext")) >>= m_context;
    m_uriFactory = css::uri::UriReferenceFactory::create(m_context);
}

void Test::tearDown() {
    css::uno::Reference< css::lang::XComponent >(
        m_context, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testParse() {
    struct Data {
        char const * uriReference;
        char const * scheme;
        char const * schemeSpecificPart;
        bool isHierarchical;
        char const * authority;
        char const * path;
        bool hasRelativePath;
        sal_Int32 pathSegmentCount;
        char const * pathSegment0;
        char const * pathSegment1;
        char const * pathSegment2;
        char const * pathSegment3;
        char const * pathSegment4;
        char const * query;
        char const * fragment;
    };
    Data data[] = {
        { "", 0, "", true, 0,
          "", true, 0, "", "", "", "", "", 0, 0 },
        { "scheme:", 0, 0, false, 0,
          0, false, 0, 0, 0, 0, 0, 0, 0, 0 },
        { "scheme:/", "scheme", "/", true, 0,
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "scheme://", "scheme", "//", true, "",
          "", false, 0, "", "", "", "", "", 0, 0 },
        { "scheme:///", "scheme", "///", true, "",
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "scheme:////", "scheme", "////", true, "",
          "//", false, 2, "", "", "", "", "", 0, 0 },
        { "scheme:////", "scheme", "////", true, "",
          "//", false, 2, "", "", "", "", "", 0, 0 },
        { "scheme:#", 0, 0, false, 0,
          0, false, 0, 0, 0, 0, 0, 0, 0, 0 },
        { "scheme:?", "scheme", "?", false, 0,
          "?", false, 0, "", "", "", "", "", 0, 0 },
        { "/", 0, "/", true, 0,
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "//", 0, "//", true, "",
          "", false, 0, "", "", "", "", "", 0, 0 },
        { "///", 0, "///", true, "",
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "////", 0, "////", true, "",
          "//", false, 2, "", "", "", "", "", 0, 0 } };
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].uriReference)));
        CPPUNIT_ASSERT(uriRef.is() == (data[i].schemeSpecificPart != 0));
        if (uriRef.is()) {
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].uriReference),
                uriRef->getUriReference());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].scheme != 0, uriRef->isAbsolute());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(
                    data[i].scheme == 0 ? "" : data[i].scheme),
                uriRef->getScheme());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].schemeSpecificPart),
                uriRef->getSchemeSpecificPart());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].isHierarchical,
                static_cast< bool >(uriRef->isHierarchical()));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].authority != 0, uriRef->hasAuthority());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(
                    data[i].authority == 0 ? "" : data[i].authority),
                uriRef->getAuthority());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].path),
                uriRef->getPath());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].hasRelativePath,
                static_cast< bool >(uriRef->hasRelativePath()));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].pathSegmentCount, uriRef->getPathSegmentCount());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(""), uriRef->getPathSegment(-1));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].pathSegment0),
                uriRef->getPathSegment(0));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].pathSegment1),
                uriRef->getPathSegment(1));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].pathSegment2),
                uriRef->getPathSegment(2));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].pathSegment3),
                uriRef->getPathSegment(3));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].pathSegment4),
                uriRef->getPathSegment(4));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(""), uriRef->getPathSegment(5));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].query != 0, uriRef->hasQuery());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(
                    data[i].query == 0 ? "" : data[i].query),
                uriRef->getQuery());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].fragment != 0, uriRef->hasFragment());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                rtl::OUString::createFromAscii(
                    data[i].fragment == 0 ? "" : data[i].fragment),
                uriRef->getFragment());
        }
    }
}

void Test::testMakeAbsolute() {
    struct Data {
        char const * baseUriReference;
        char const * uriReference;
        bool processSpecialBaseSegments;
        css::uri::RelativeUriExcessParentSegments excessParentSegments;
        char const * absolute;
    };
    Data data[] = {
        // The following tests are taken from RFC 2396, Appendix C:
        { "http://a/b/c/d;p?q", "g:h", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "g:h" },
        { "http://a/b/c/d;p?q", "g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/g" },
        { "http://a/b/c/d;p?q", "./g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/g" },
        { "http://a/b/c/d;p?q", "g/", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/g/" },
        { "http://a/b/c/d;p?q", "//g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://g" },
        { "http://a/b/c/d;p?q", "?y", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/?y" },
        { "http://a/b/c/d;p?q", "g?y", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g?y" },
        { "http://a/b/c/d;p?q", "#s", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/d;p?q#s" },
        { "http://a/b/c/d;p?q", "g#s", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g#s" },
        { "http://a/b/c/d;p?q", "g?y#s", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g?y#s" },
        { "http://a/b/c/d;p?q", ";x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/;x" },
        { "http://a/b/c/d;p?q", "g;x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g;x" },
        { "http://a/b/c/d;p?q", "g;x?y#s", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g;x?y#s" },
        { "http://a/b/c/d;p?q", ".", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/" },
        { "http://a/b/c/d;p?q", "./", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/" },
        { "http://a/b/c/d;p?q", "..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/" },
        { "http://a/b/c/d;p?q", "../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/" },
        { "http://a/b/c/d;p?q", "../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/g" },
        { "http://a/b/c/d;p?q", "../..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/" },
        { "http://a/b/c/d;p?q", "../../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/" },
        { "http://a/b/c/d;p?q", "../../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/g" },
        { "http://a/b/c/d;p?q", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/d;p?q" },
        { "http://a/b/c/d;p?q", "../../../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, 0 },
        { "http://a/b/c/d;p?q", "../../../g", true,
          css::uri::RelativeUriExcessParentSegments_RETAIN, "http://a/../g" },
        { "http://a/b/c/d;p?q", "../../../g", true,
          css::uri::RelativeUriExcessParentSegments_REMOVE, "http://a/g" },
        { "http://a/b/c/d;p?q", "../../../../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, 0 },
        { "http://a/b/c/d;p?q", "../../../../g", true,
          css::uri::RelativeUriExcessParentSegments_RETAIN,
          "http://a/../../g" },
        { "http://a/b/c/d;p?q", "../../../../g", true,
          css::uri::RelativeUriExcessParentSegments_REMOVE, "http://a/g" },
        { "http://a/b/c/d;p?q", "/./g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/./g" },
        { "http://a/b/c/d;p?q", "/../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/../g" },
        { "http://a/b/c/d;p?q", "g.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/g." },
        { "http://a/b/c/d;p?q", ".g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/.g" },
        { "http://a/b/c/d;p?q", "g..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g.." },
        { "http://a/b/c/d;p?q", "..g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/..g" },
        { "http://a/b/c/d;p?q", "./../g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/g" },
        { "http://a/b/c/d;p?q", "./g/.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/g/" },
        { "http://a/b/c/d;p?q", "g/./h", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g/h" },
        { "http://a/b/c/d;p?q", "g/../h", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/h" },
        { "http://a/b/c/d;p?q", "g;x=1/./y", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g;x=1/y" },
        { "http://a/b/c/d;p?q", "g;x=1/../y", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http://a/b/c/y" },
        { "http://a/b/c/d;p?q", "g?y/./x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g?y/./x" },
        { "http://a/b/c/d;p?q", "g?y/../x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g?y/../x" },
        { "http://a/b/c/d;p?q", "g#s/./x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g#s/./x" },
        { "http://a/b/c/d;p?q", "g#s/../x", true,
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http://a/b/c/g#s/../x" },
        { "http.://a/b/c/d;p?q", "http.:g", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "http.:g" },

        { "scheme://a", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", ".", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "././x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "././x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "./././x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },

        { "scheme://a/", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", ".", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "././x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "././x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/", "./././x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },

        { "scheme://a/b", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b" },
        { "scheme://a/b", ".", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "././x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "././x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },
        { "scheme://a/b", "./././x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/" },

        { "scheme://a/b/", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", ".", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/../", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./././x/..", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./x/../.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./x/.././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "././x/.././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "././x/../././", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },
        { "scheme://a/b/", "./././x/../././.", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a/b/" },

        { "scheme://a#s", "", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a" },
        { "scheme://a", "?q", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a?q" },
        { "scheme://a#s", "?q", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a?q" },
        { "scheme://a", "#s", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a#s" },
        { "scheme://a#s1", "#s2", true,
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme://a#s2" } };
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        css::uno::Reference< css::uri::XUriReference > baseUriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].baseUriReference)));
        CPPUNIT_ASSERT(baseUriRef.is());
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].uriReference)));
        CPPUNIT_ASSERT(uriRef.is());
        css::uno::Reference< css::uri::XUriReference > absolute(
            m_uriFactory->makeAbsolute(
                baseUriRef, uriRef, data[i].processSpecialBaseSegments,
                data[i].excessParentSegments));
        TEST_ASSERT_EQUAL(
            "testMakeAbsolute", i, data[i].uriReference,
            data[i].absolute != 0, absolute.is());
        if (absolute.is()) {
            TEST_ASSERT_EQUAL(
                "testMakeAbsolute", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].absolute),
                absolute->getUriReference());
        }
    }
}

void Test::testMakeRelative() {
    struct Data {
        char const * baseUriReference;
        char const * uriReference;
        bool preferAuthorityOverRelativePath;
        bool preferAbsoluteOverRelativePath;
        bool encodeRetainedSpecialSegments;
        char const * relative;
        char const * absolute;
    };
    Data data[] = {
        { "scheme1://a/b/c", "scheme2://a/b/c?q#s", true, true, false,
          "scheme2://a/b/c?q#s", 0 },
        { "scheme://a/b/c", "scheme:a/b/c?q#s", true, true, false,
          "scheme:a/b/c?q#s", 0 },
        { "scheme://a/b/c", "", true, true, false, "", "scheme://a/b/c" },
        { "scheme://a/b/c", "//d/e/f", true, true, false, "//d/e/f",
          "scheme://d/e/f" },
        { "scheme://a/b/c", "./e?q#s", true, true, false, "./e?q#s",
          "scheme://a/b/e?q#s" },
        { "scheme://a/b", "scheme://a?q", true, true, false, "/?q",
          "scheme://a/?q" },
        { "scheme://a/b", "scheme://a?q", true, false, false, "?q",
          "scheme://a/?q" },
        { "scheme://a", "scheme://a?q", true, true, false, "?q", 0 },
        { "scheme://a/", "scheme://a?q", true, true, false, "?q",
          "scheme://a/?q" },
        { "scheme://a", "scheme://a/?q", true, true, false, "?q",
          "scheme://a?q" },
        { "scheme://a/", "scheme://a/?q", true, true, false, "?q",
          "scheme://a/?q" },
        { "scheme://a?q", "scheme://a?q", true, true, false, "", 0 },
        { "scheme://a/?q", "scheme://a?q", true, true, false, "",
          "scheme://a/?q" },
        { "scheme://a?q", "scheme://a/?q", true, true, false, "",
          "scheme://a?q" },
        { "scheme://a/?q", "scheme://a/?q", true, true, false, "", 0 },
        { "scheme://a/b/c/d", "scheme://a//", true, true, false, "//a//", 0 },
        { "scheme://a/b/c/d", "scheme://a//", false, true, false, "../..//",
          0 },
        { "scheme://a/b/c/d", "scheme://a//", true, false, false, "../..//",
          0 },
        { "scheme://a/b/c/d", "scheme://a//", false, false, false, "../..//",
          0 },
        { "scheme://a/b/c/d", "scheme://a/e", true, true, false, "/e", 0 },
        { "scheme://a/b/c/d", "scheme://a/e", true, false, false, "../../e",
          0 },
        { "scheme://a/b/c/d/e", "scheme://a/b/f", true, true, false, "../../f",
          0 },
        { "scheme://a/b/c/d/e", "scheme://a/b", true, true, false, "/b", 0 },
        { "scheme://a/b/c/d/e", "scheme://a/b", true, false, false,
          "../../../b", 0 },
        { "scheme://a/b/c/d/e", "scheme://a/b/", true, true, false, "../..",
          0 },
        { "scheme://a/b/c/d/e", "scheme://a/b/c", true, true, false, "../../c",
          0 },
        { "scheme://a/b/c/d/e", "scheme://a/b/c/", true, true, false, "..", 0 },
        { "scheme://a/b/", "scheme://a/b/c/d", true, true, false, "c/d", 0 },
        { "scheme://a/b/", "scheme://a/b/c/d/", true, true, false, "c/d/", 0 },
        { "scheme://a/b/c", "scheme://a/b//", true, true, false, ".//", 0 },
        { "scheme://a/b/c", "scheme://a/b//d", true, true, false, ".//d", 0 },
        { "scheme://a/b/c", "scheme://a/b//d//", true, true, false, ".//d//",
          0 },
        { "scheme://a/b/c", "scheme://a/b/d+:", true, true, false, "./d+:", 0 },
        { "scheme://a/b/c", "scheme://a/b/+d:", true, true, false, "+d:", 0 },
        { "scheme://a/b/c", "scheme://a/b/d#e:f", true, true, false, "d#e:f",
          0 },
        { "scheme://a/b/c/", "scheme://a/b/../d/.e/.", true, true, false,
          "../../d/.e/.",
          "scheme://a/d/.e/" },
        { "scheme://a/b/c/", "scheme://a/b/../d/.e/.", true, true, true,
          "../%2E%2E/d/.e/%2E", "scheme://a/b/%2E%2E/d/.e/%2E" },
        { "scheme://auth/a/b", "scheme://auth//c/d", true, true, false,
          "//auth//c/d", 0 },
        { "scheme://auth/a/b", "scheme://auth//c/d", false, true, false,
          "..//c/d", 0 },
        { "scheme://auth/a/b", "scheme://auth/c/d", true, true, false, "/c/d",
          0 },
        { "scheme://auth/a/b", "scheme://auth/c/d", true, false, false,
          "../c/d", 0 } };
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        css::uno::Reference< css::uri::XUriReference > baseUriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].baseUriReference)));
        CPPUNIT_ASSERT(baseUriRef.is());
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].uriReference)));
        CPPUNIT_ASSERT(uriRef.is());
        css::uno::Reference< css::uri::XUriReference > relative(
            m_uriFactory->makeRelative(
                baseUriRef, uriRef, data[i].preferAuthorityOverRelativePath,
                data[i].preferAbsoluteOverRelativePath,
                data[i].encodeRetainedSpecialSegments));
        TEST_ASSERT_EQUAL(
            "testMakeRelative", i, data[i].uriReference,
            data[i].relative != 0, relative.is());
        if (relative.is()) {
            TEST_ASSERT_EQUAL(
                "testMakeRelative", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].relative),
                relative->getUriReference());
            css::uno::Reference< css::uri::XUriReference > absolute(
                m_uriFactory->makeAbsolute(
                    baseUriRef, relative, true,
                    css::uri::RelativeUriExcessParentSegments_ERROR));
            CPPUNIT_ASSERT(absolute.is());
            TEST_ASSERT_EQUAL(
                "testMakeRelative", i, data[i].uriReference,
                rtl::OUString::createFromAscii(
                    data[i].absolute == 0
                    ? data[i].uriReference : data[i].absolute),
                absolute->getUriReference());
        }
    }
}

void Test::testVndSunStarScript() {
    struct Parameter {
        char const * key;
        char const * value;
    };
    std::size_t const parameterCount = 3;
    struct Data {
        char const * uriReference;
        char const * name;
        Parameter parameters[parameterCount];
    };
    Data data[] = {
        { "vnd.sun.star.script:", 0 },
        { "vnd.sun.star.script:/", 0 },
        { "vnd.sun.star.script:/abc/def?ghi=jkl&mno=pqr", 0 },
        { "vnd.sun.star.script:abc%3fdef/ghi", "abc?def/ghi" },
        { "vnd.sun.star.script:name?a", 0 },
        { "vnd.sun.star.script:name?a=", "name", { { "a", "" }, { "A", 0 } } },
        { "vnd.sun.star.script:name?a=&", 0 },
        { "vnd.sun.star.script:name?key1=&%26=%3D&key1=hello", "name",
          { { "key1", "" }, { "key2", 0 }, { "&", "=" } } } };
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                rtl::OUString::createFromAscii(data[i].uriReference)));
        TEST_ASSERT_EQUAL(
            "testVndSunStarScript", i, data[i].uriReference, data[i].name != 0,
            uriRef.is());
        if (uriRef.is()) {
            css::uno::Reference< css::uri::XVndSunStarScriptUrlReference >
                scriptUrl(uriRef, css::uno::UNO_QUERY_THROW);
            TEST_ASSERT_EQUAL(
                "testVndSunStarScript", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].uriReference),
                scriptUrl->getUriReference());
            TEST_ASSERT_EQUAL(
                "testVndSunStarScript", i, data[i].uriReference,
                rtl::OUString::createFromAscii(data[i].name),
                scriptUrl->getName());
            for (std::size_t j = 0; j < parameterCount; ++j) {
                if (data[i].parameters[j].key != 0) {
                    TEST_ASSERT_EQUAL(
                        "testVndSunStarScript",
                        static_cast< double >(i)
                        + static_cast< double >(j) / 10.0,
                        data[i].uriReference,
                        data[i].parameters[j].value != 0,
                        scriptUrl->hasParameter(
                            rtl::OUString::createFromAscii(
                                data[i].parameters[j].key)));
                    TEST_ASSERT_EQUAL(
                        "testVndSunStarScript",
                        static_cast< double >(i)
                        + static_cast< double >(j) / 10.0,
                        data[i].uriReference,
                        rtl::OUString::createFromAscii(
                            data[i].parameters[j].value),
                        scriptUrl->getParameter(
                            rtl::OUString::createFromAscii(
                                data[i].parameters[j].key)));
                }
            }
        }
    }
}

void Test::testTranslator() {
    struct Data {
        char const * externalUriReference;
        char const * internalUriReference;
    };
    Data data[] = {
        { "", "" },
        { "#fragment", "#fragment" },
        { "segment/segment?query#fragment", "segment/segment?query#fragment" },
        { "/segment/segment?query#fragment",
          "/segment/segment?query#fragment" },
        { "//authority/segment?query#fragment",
          "//authority/segment?query#fragment" },
        { "foo:bar#fragment", "foo:bar#fragment" },
        { "file:///abc/def", "file:///abc/def" },
        { 0, "file:///abc/%FEef" },
        { 0, "file:///abc/%80%80ef" },
        { 0, "file:///abc/%ED%A0%80%ED%B0%80ef" } };
    css::uno::Reference< css::uri::XExternalUriReferenceTranslator >
        translator(css::uri::ExternalUriReferenceTranslator::create(m_context));
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        if (data[i].externalUriReference != 0) {
            TEST_ASSERT_EQUAL(
                "testTranslator, translateToInternal", i,
                data[i].externalUriReference,
                (data[i].internalUriReference == 0
                 ? rtl::OUString()
                 : rtl::OUString::createFromAscii(
                     data[i].internalUriReference)),
                translator->translateToInternal(
                    rtl::OUString::createFromAscii(
                        data[i].externalUriReference)));
        }
        if (data[i].internalUriReference != 0) {
            TEST_ASSERT_EQUAL(
                "testTranslator, translateToExternal", i,
                data[i].internalUriReference,
                (data[i].externalUriReference == 0
                 ? rtl::OUString()
                 : rtl::OUString::createFromAscii(
                     data[i].externalUriReference)),
                translator->translateToExternal(
                    rtl::OUString::createFromAscii(
                        data[i].internalUriReference)));
        }
    }
}

void Test::testPkgUrlFactory() {
    struct Data {
        char const * authority;
        char const * result;
    };
    Data data[] = {
        { "a/b/c", 0 },
        { "file:///#foo", 0 },
        { "file:///a%25b%2fc/d~e&f@g?h",
          "vnd.sun.star.pkg://file:%2F%2F%2Fa%2525b%252fc%2Fd~e&f@g%3Fh" } };
    css::uno::Reference< css::uri::XVndSunStarPkgUrlReferenceFactory > factory(
        css::uri::VndSunStarPkgUrlReferenceFactory::create(m_context));
    for (std::size_t i = 0; i < sizeof data / sizeof data[0]; ++i) {
        css::uno::Reference< css::uri::XUriReference > url(
            factory->createVndSunStarPkgUrlReference(
                m_uriFactory->parse(
                    rtl::OUString::createFromAscii(data[i].authority))));
        TEST_ASSERT_EQUAL(
            "testVndSunStarPkgFactory", i, data[i].authority,
            data[i].result != 0, static_cast< bool >(url.is()));
        if (data[i].result != 0) {
            TEST_ASSERT_EQUAL(
                "testVndSunStarPkgFactory", i, data[i].authority,
                rtl::OUString::createFromAscii(data[i].result),
                url->getUriReference());
        }
    }
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
