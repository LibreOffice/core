/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/types.h>
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
#include "com/sun/star/uri/XVndSunStarExpandUrlReference.hpp"
#include "com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarScriptUrlReference.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include <sal/macros.h>

#include <cstdlib>

namespace {

#define TEST_ASSERT_EQUAL(token1, token2, token3, expected, actual) \
    CPPUNIT_ASSERT_MESSAGE( \
        createTestAssertEqualMessage( \
            token1, token2, token3, #expected, #actual, expected, actual). \
            getStr(), \
        (actual) == (expected))

template< typename T > void append(
    OUStringBuffer & buffer, T const & value)
{
    buffer.append(value);
}

template<> void append(OUStringBuffer & buffer, bool const & value) {
    buffer.append(static_cast< sal_Bool >(value));
}

template<> void append(OUStringBuffer & buffer, std::size_t const & value)
{
    buffer.append(static_cast< sal_Int32 >(value));
}

template<> void append(OUStringBuffer & buffer, char const * const & value)
{
    buffer.appendAscii(value);
}

template< typename T1, typename T2, typename T3, typename T4 >
OString createTestAssertEqualMessage(
    char const * token1, T1 const & token2, T2 const & token3,
    char const * expectedExpr, char const * actualExpr, T3 const & expected,
    T4 const & actual)
{
    OUStringBuffer buf;
    buf.appendAscii(token1);
    buf.append('|');
    append(buf, token2);
    buf.append('|');
    append(buf, token3);
    buf.append(": TEST_ASSERT_EQUAL(");
    buf.appendAscii(expectedExpr);
    buf.append(", ");
    buf.appendAscii(actualExpr);
    buf.append("): <");
    append(buf, expected);
    buf.append("> != <");
    append(buf, actual);
    buf.append('>');
    return OUStringToOString(
        buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
}

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();

    virtual void tearDown();

    void testParse();

    void testMakeAbsolute();

    void testMakeRelative();

    void testVndSunStarExpand();

    void testVndSunStarScript();

    void testTranslator();

    void testPkgUrlFactory();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testParse);
    CPPUNIT_TEST(testMakeAbsolute);
    CPPUNIT_TEST(testMakeRelative);
    CPPUNIT_TEST(testVndSunStarExpand);
    CPPUNIT_TEST(testVndSunStarScript);
    CPPUNIT_TEST(testTranslator);
    CPPUNIT_TEST(testPkgUrlFactory);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference< css::uno::XComponentContext > m_context;
    css::uno::Reference< css::uri::XUriReferenceFactory > m_uriFactory;
};

void Test::setUp() {
    m_context = cppu::defaultBootstrap_InitialComponentContext();
    m_uriFactory = css::uri::UriReferenceFactory::create(m_context);
}

void Test::tearDown() {
    m_uriFactory.clear();
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
        { "scheme:
          "", false, 0, "", "", "", "", "", 0, 0 },
        { "scheme:
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "scheme:
          "
        { "scheme:
          "
        { "scheme:#", 0, 0, false, 0,
          0, false, 0, 0, 0, 0, 0, 0, 0, 0 },
        { "scheme:?", "scheme", "?", false, 0,
          "?", false, 0, "", "", "", "", "", 0, 0 },
        { "/", 0, "/", true, 0,
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "
          "", false, 0, "", "", "", "", "", 0, 0 },
        { "
          "/", false, 1, "", "", "", "", "", 0, 0 },
        { "
          "
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].uriReference)));
        CPPUNIT_ASSERT(uriRef.is() == (data[i].schemeSpecificPart != 0));
        if (uriRef.is()) {
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].uriReference),
                uriRef->getUriReference());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].scheme != 0, uriRef->isAbsolute());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(
                    data[i].scheme == 0 ? "" : data[i].scheme),
                uriRef->getScheme());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].schemeSpecificPart),
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
                OUString::createFromAscii(
                    data[i].authority == 0 ? "" : data[i].authority),
                uriRef->getAuthority());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].path),
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
                OUString(""), uriRef->getPathSegment(-1));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].pathSegment0),
                uriRef->getPathSegment(0));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].pathSegment1),
                uriRef->getPathSegment(1));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].pathSegment2),
                uriRef->getPathSegment(2));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].pathSegment3),
                uriRef->getPathSegment(3));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(data[i].pathSegment4),
                uriRef->getPathSegment(4));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString(), uriRef->getPathSegment(5));
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].query != 0, uriRef->hasQuery());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(
                    data[i].query == 0 ? "" : data[i].query),
                uriRef->getQuery());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                data[i].fragment != 0, uriRef->hasFragment());
            TEST_ASSERT_EQUAL(
                "testParse", i, data[i].uriReference,
                OUString::createFromAscii(
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
        
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "g:h" },
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, 0 },
        { "http:
          css::uri::RelativeUriExcessParentSegments_RETAIN, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_REMOVE, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, 0 },
        { "http:
          css::uri::RelativeUriExcessParentSegments_RETAIN,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_REMOVE, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http:
          css::uri::RelativeUriExcessParentSegments_ERROR,
          "http:
        { "http.:
          css::uri::RelativeUriExcessParentSegments_ERROR, "http.:g" },

        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:

        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:

        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:

        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:

        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
        { "scheme:
          css::uri::RelativeUriExcessParentSegments_ERROR, "scheme:
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > baseUriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].baseUriReference)));
        CPPUNIT_ASSERT(baseUriRef.is());
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].uriReference)));
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
                OUString::createFromAscii(data[i].absolute),
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
        { "scheme1:
          "scheme2:
        { "scheme:
          "scheme:a/b/c?q#s", 0 },
        { "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
        { "scheme:
          "scheme:
        { "scheme:
          "scheme:
        { "scheme:
        { "scheme:
        { "scheme:
          0 },
        { "scheme:
          0 },
        { "scheme:
          0 },
        { "scheme:
        { "scheme:
          0 },
        { "scheme:
          0 },
        { "scheme:
        { "scheme:
          "../../../b", 0 },
        { "scheme:
          0 },
        { "scheme:
          0 },
        { "scheme:
        { "scheme:
        { "scheme:
        { "scheme:
        { "scheme:
        { "scheme:
          0 },
        { "scheme:
        { "scheme:
        { "scheme:
          0 },
        { "scheme:
          "../../d/.e/.",
          "scheme:
        { "scheme:
          "../%2E%2E/d/.e/%2E", "scheme:
        { "scheme:
          "
        { "scheme:
          "..
        { "scheme:
          0 },
        { "scheme:
          "../c/d", 0 } };
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > baseUriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].baseUriReference)));
        CPPUNIT_ASSERT(baseUriRef.is());
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].uriReference)));
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
                OUString::createFromAscii(data[i].relative),
                relative->getUriReference());
            css::uno::Reference< css::uri::XUriReference > absolute(
                m_uriFactory->makeAbsolute(
                    baseUriRef, relative, true,
                    css::uri::RelativeUriExcessParentSegments_ERROR));
            CPPUNIT_ASSERT(absolute.is());
            TEST_ASSERT_EQUAL(
                "testMakeRelative", i, data[i].uriReference,
                OUString::createFromAscii(
                    data[i].absolute == 0
                    ? data[i].uriReference : data[i].absolute),
                absolute->getUriReference());
        }
    }
}

void Test::testVndSunStarExpand() {
    struct Data {
        char const * uriReference;
        char const * expanded;
    };
    Data data[] = {
        { "vnd.sun.star.expand:", "" }, 
        { "vnd.sun.star.expand:/", "/" }, 
        { "vnd.sun.star.expand:%80", 0 },
        { "vnd.sun.star.expand:%5C$%5C%24%5C%5C", "$$\\" } };
    css::uno::Reference< css::util::XMacroExpander > expander(
        m_context->getValueByName(
              OUString( "/singletons/com.sun.star.util.theMacroExpander")),
        css::uno::UNO_QUERY_THROW);
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].uriReference)));
        TEST_ASSERT_EQUAL(
            "testVndSunStarExpand", i, data[i].uriReference,
            data[i].expanded != 0, uriRef.is());
        if (uriRef.is()) {
            css::uno::Reference< css::uri::XVndSunStarExpandUrlReference >
                expandUrl(uriRef, css::uno::UNO_QUERY_THROW);
            TEST_ASSERT_EQUAL(
                "testVndSunStarExpand", i, data[i].uriReference,
                OUString::createFromAscii(data[i].expanded),
                expandUrl->expand(expander));
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
        const bool   normalized;
        Parameter parameters[parameterCount];
    };
    Data data[] = {
        { "vnd.sun.star.script:", 0, false, {} },
        { "vnd.sun.star.script:/", 0, false, {} },
        { "vnd.sun.star.script:/abc/def?ghi=jkl&mno=pqr", 0, false, {} },
        { "vnd.sun.star.script:abc%3fdef/ghi", "abc?def/ghi", false, {} },
        { "vnd.sun.star.script:name?a", 0, false, {} },
        { "vnd.sun.star.script:name?a=", "name", true, { { "a", "" }, { "A", 0 } } },
        { "vnd.sun.star.script:name?a=&", 0, true, {} },
        { "vnd.sun.star.script:name?key1=&%26=%3D&key1=hello", "name", true,
          { { "key1", "" }, { "key2", 0 }, { "&", "=" } } } };
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > uriRef(
            m_uriFactory->parse(
                OUString::createFromAscii(data[i].uriReference)));
        TEST_ASSERT_EQUAL(
            "testVndSunStarScript", i, data[i].uriReference, data[i].name != 0,
            uriRef.is());
        if (uriRef.is()) {
            css::uno::Reference< css::uri::XVndSunStarScriptUrlReference >
                scriptUrl(uriRef, css::uno::UNO_QUERY_THROW);
            TEST_ASSERT_EQUAL(
                "testVndSunStarScript", i, data[i].uriReference,
                OUString::createFromAscii(data[i].uriReference),
                scriptUrl->getUriReference());
            TEST_ASSERT_EQUAL(
                "testVndSunStarScript", i, data[i].uriReference,
                OUString::createFromAscii(data[i].name),
                scriptUrl->getName());
            OUString originalReference(uriRef->getUriReference());
            for (std::size_t j = 0; j < parameterCount; ++j) {
                if (data[i].parameters[j].key != 0) {
                    TEST_ASSERT_EQUAL(
                        "testVndSunStarScript",
                        static_cast< double >(i)
                        + static_cast< double >(j) / 10.0,
                        data[i].uriReference,
                        data[i].parameters[j].value != 0,
                        scriptUrl->hasParameter(
                            OUString::createFromAscii(
                                data[i].parameters[j].key)));
                    TEST_ASSERT_EQUAL(
                        "testVndSunStarScript",
                        static_cast< double >(i)
                        + static_cast< double >(j) / 10.0,
                        data[i].uriReference,
                        OUString::createFromAscii(
                            data[i].parameters[j].value),
                        scriptUrl->getParameter(
                            OUString::createFromAscii(
                                data[i].parameters[j].key)));

                    
                    
                    if ( data[i].normalized ) {
                        if ( scriptUrl->hasParameter(OUString::createFromAscii(
                            data[i].parameters[j].key)) ) {
                            scriptUrl->setParameter(
                                OUString::createFromAscii(
                                    data[i].parameters[j].key),
                                scriptUrl->getParameter(
                                    OUString::createFromAscii(
                                        data[i].parameters[j].key)));
                            TEST_ASSERT_EQUAL(
                                "testVndSunStarScript",
                                static_cast< double >(i)
                                + static_cast< double >(j) / 10.0,
                                OUString("setParameter"),
                                originalReference,
                                uriRef->getUriReference());
                        }
                    }
                }
            }
            if ( data[i].normalized ) {
                scriptUrl->setName(scriptUrl->getName());
                TEST_ASSERT_EQUAL(
                    "testVndSunStarScript",
                    i,
                    OUString("setName"),
                    originalReference,
                    uriRef->getUriReference());
            }
        }
    }

    css::uno::Reference< css::uri::XUriReference > uriRef(
        m_uriFactory->parse(
            OUString( "vnd.sun.star.script:Hello?location=Library.Module")),
        css::uno::UNO_SET_THROW);
    css::uno::Reference< css::uri::XVndSunStarScriptUrlReference >
        scriptUrl(uriRef, css::uno::UNO_QUERY_THROW);

    scriptUrl->setParameter(
        OUString("location"),
        OUString("foo"));
    TEST_ASSERT_EQUAL(
        "testVndSunStarScript", (sal_Int32)10, (sal_Int32)1,
        uriRef->getUriReference(),
        OUString("vnd.sun.star.script:Hello?location=foo"));

    scriptUrl->setParameter(
        OUString("language"),
        OUString("StarBasic"));
    TEST_ASSERT_EQUAL(
        "testVndSunStarScript", (sal_Int32)10, (sal_Int32)2,
        uriRef->getUriReference(),
        OUString("vnd.sun.star.script:Hello?location=foo&language=StarBasic"));


    bool caughtExpected = false;
    try {
        scriptUrl->setName(OUString());
    }
    catch( const css::lang::IllegalArgumentException& ) {
        caughtExpected = true;
    }
    TEST_ASSERT_EQUAL(
        "testVndSunStarScript",
        OUString("illegal arguments"),
        OUString("name"),
        caughtExpected,
        true);

    caughtExpected = false;
    try {
        scriptUrl->setParameter(
            OUString(),
            OUString("non-empty"));
    }
    catch( const css::lang::IllegalArgumentException& ) {
        caughtExpected = true;
    }
    TEST_ASSERT_EQUAL(
        "testVndSunStarScript",
        OUString("illegal arguments"),
        OUString("parameter"),
        caughtExpected,
        true);
}

void Test::testTranslator() {
    struct Data {
        char const * externalUriReference;
        char const * internalUriReference;
        bool toInternal;
    };
    Data data[] = {
        { "", "", true },
        { "#fragment", "#fragment", true },
        { "segment/segment?query#fragment", "segment/segment?query#fragment",
          true },
        { "/segment/segment?query#fragment", "/segment/segment?query#fragment",
          true },
        { "
          "
        { "foo:bar#fragment", "foo:bar#fragment", true },
        { "file:
        { "file:
        { "file:
        { "file:
          "file:
        { "file:
        { "file:
    css::uno::Reference< css::uri::XExternalUriReferenceTranslator >
        translator(css::uri::ExternalUriReferenceTranslator::create(m_context));
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        if (data[i].toInternal) {
            TEST_ASSERT_EQUAL(
                "testTranslator, translateToInternal", i,
                data[i].externalUriReference,
                OUString::createFromAscii(data[i].internalUriReference),
                translator->translateToInternal(
                    OUString::createFromAscii(
                        data[i].externalUriReference)));
        }
        TEST_ASSERT_EQUAL(
            "testTranslator, translateToExternal", i,
            data[i].internalUriReference,
            OUString::createFromAscii(data[i].externalUriReference),
            translator->translateToExternal(
                OUString::createFromAscii(data[i].internalUriReference)));
    }
}

void Test::testPkgUrlFactory() {
    struct Data {
        char const * authority;
        char const * result;
    };
    Data data[] = {
        { "a/b/c", 0 },
        { "file:
        { "file:
          "vnd.sun.star.pkg:
    css::uno::Reference< css::uri::XVndSunStarPkgUrlReferenceFactory > factory(
        css::uri::VndSunStarPkgUrlReferenceFactory::create(m_context));
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(data); ++i) {
        css::uno::Reference< css::uri::XUriReference > url(
            factory->createVndSunStarPkgUrlReference(
                m_uriFactory->parse(
                    OUString::createFromAscii(data[i].authority))));
        TEST_ASSERT_EQUAL(
            "testVndSunStarPkgFactory", i, data[i].authority,
            data[i].result != 0, static_cast< bool >(url.is()));
        if (data[i].result != 0) {
            TEST_ASSERT_EQUAL(
                "testVndSunStarPkgFactory", i, data[i].authority,
                OUString::createFromAscii(data[i].result),
                url->getUriReference());
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
