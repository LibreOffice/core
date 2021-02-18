/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <rtl/ustring.hxx>

#include <ToxLinkProcessor.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <test/bootstrapfixture.hxx>

#include <swdll.hxx>

using namespace sw;

class ToxLinkProcessorTest : public test::BootstrapFixture
{
    void NoExceptionIsThrownIfTooManyLinksAreClosed();
    void AddingAndClosingTwoOverlappingLinksResultsInOneClosedLink();
    void LinkIsCreatedCorrectly();
    void LinkSequenceIsPreserved();

    CPPUNIT_TEST_SUITE(ToxLinkProcessorTest);
    CPPUNIT_TEST(NoExceptionIsThrownIfTooManyLinksAreClosed);
    CPPUNIT_TEST(AddingAndClosingTwoOverlappingLinksResultsInOneClosedLink);
    CPPUNIT_TEST(LinkIsCreatedCorrectly);
    CPPUNIT_TEST(LinkSequenceIsPreserved);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp() override {
        BootstrapFixture::setUp();
        SwGlobals::ensure();
    }

    static constexpr OUStringLiteral STYLE_NAME_1 = u"anyStyle1";
    static constexpr OUStringLiteral STYLE_NAME_2 = u"anyStyle2";
    static const sal_uInt16 POOL_ID_1;
    static const sal_uInt16 POOL_ID_2;
    static constexpr OUStringLiteral URL_1 = u"#anyUrl1";
    static constexpr OUStringLiteral URL_2 = u"#anyUrl2";
};

const sal_uInt16 ToxLinkProcessorTest::POOL_ID_1 = 42;
const sal_uInt16 ToxLinkProcessorTest::POOL_ID_2 = 43;

void
ToxLinkProcessorTest::NoExceptionIsThrownIfTooManyLinksAreClosed()
{
    ToxLinkProcessor sut;
    sut.StartNewLink(0, STYLE_NAME_1);
    sut.CloseLink(1, URL_1, /*bRelative=*/true);
    // fdo#85872 actually it turns out the UI does something like this
    // so an exception must not be thrown!
    // should not succeed either (for backward compatibility)
    sut.CloseLink(2, URL_1, /*bRelative=*/true);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned>(sut.m_ClosedLinks.size()));
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned>(sut.m_ClosedLinks.at(0)->mEndTextPos));
    CPPUNIT_ASSERT_MESSAGE("no links are open", !sut.m_pStartedLink);
}

void
ToxLinkProcessorTest::AddingAndClosingTwoOverlappingLinksResultsInOneClosedLink()
{
    ToxLinkProcessor sut;
    sut.StartNewLink(0, STYLE_NAME_1);
    sut.StartNewLink(0, STYLE_NAME_2);
    sut.CloseLink(1, URL_1, /*bRelative=*/true);
    // this should not cause an error, and should not succeed either
    // (for backward compatibility)
    sut.CloseLink(1, URL_2, /*bRelative=*/true);
    CPPUNIT_ASSERT_EQUAL(1u, static_cast<unsigned>(sut.m_ClosedLinks.size()));
    CPPUNIT_ASSERT_MESSAGE("no links are open", !sut.m_pStartedLink);
    // backward compatibility: the last start is closed by the first end
    CPPUNIT_ASSERT_EQUAL(OUString(STYLE_NAME_2), sut.m_ClosedLinks[0]->mINetFormat.GetINetFormat());
    CPPUNIT_ASSERT_EQUAL(OUString(URL_1), sut.m_ClosedLinks[0]->mINetFormat.GetValue());
}

namespace {

class ToxLinkProcessorWithOverriddenObtainPoolId : public ToxLinkProcessor {
public:
    virtual sal_uInt16
    ObtainPoolId(const OUString& characterStyle) const override {
        if (characterStyle == ToxLinkProcessorTest::STYLE_NAME_1) {
            return ToxLinkProcessorTest::POOL_ID_1;
        }
        if (characterStyle == ToxLinkProcessorTest::STYLE_NAME_2) {
            return ToxLinkProcessorTest::POOL_ID_2;
        }
        return 0;
    }
};

}

void
ToxLinkProcessorTest::LinkIsCreatedCorrectly()
{
    // obtainpoolid needs to be overridden to check what we are
    ToxLinkProcessorWithOverriddenObtainPoolId sut;

    sut.StartNewLink(0, STYLE_NAME_1);
    sut.CloseLink(1, URL_1, /*bRelative=*/true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link", OUString(STYLE_NAME_1), sut.m_ClosedLinks.at(0)->mINetFormat.GetVisitedFormat());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link", OUString(URL_1), sut.m_ClosedLinks.at(0)->mINetFormat.GetValue());
}

void
ToxLinkProcessorTest::LinkSequenceIsPreserved()
{

    // obtainpoolid needs to be overridden to check what we are
    ToxLinkProcessorWithOverriddenObtainPoolId sut;

    sut.StartNewLink(0, STYLE_NAME_2);
    sut.CloseLink(1, URL_2, /*bRelative=*/true);
    sut.StartNewLink(1, STYLE_NAME_1);
    sut.CloseLink(2, URL_1, /*bRelative=*/true);

    // check first closed element
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link",
            OUString(STYLE_NAME_2), sut.m_ClosedLinks.at(0)->mINetFormat.GetVisitedFormat());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pool id is stored correctly in link",
            POOL_ID_2, sut.m_ClosedLinks.at(0)->mINetFormat.GetINetFormatId());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link",
            OUString(URL_2), sut.m_ClosedLinks.at(0)->mINetFormat.GetValue());
    // check second closed element
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link",
            OUString(STYLE_NAME_1), sut.m_ClosedLinks.at(1)->mINetFormat.GetVisitedFormat());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pool id is stored correctly in link",
            POOL_ID_1, sut.m_ClosedLinks.at(1)->mINetFormat.GetINetFormatId());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link",
            OUString(URL_1), sut.m_ClosedLinks.at(1)->mINetFormat.GetValue());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxLinkProcessorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
