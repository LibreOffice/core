/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdexcept>

#include <sal/types.h>

#include <rtl/ustring.hxx>

#include <ToxLinkProcessor.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

using namespace sw;

class ToxLinkProcessorTest : public CppUnit::TestFixture
{
    void ExceptionIsThrownIfTooManyLinksAreClosed();
    void AddingAndClosingTwoLinksResultsInTwoClosedLinks();
    void LinkIsCreatedCorrectly();
    void LinkSequenceIsPreserved();

    CPPUNIT_TEST_SUITE(ToxLinkProcessorTest);
    CPPUNIT_TEST(ExceptionIsThrownIfTooManyLinksAreClosed);
    CPPUNIT_TEST(AddingAndClosingTwoLinksResultsInTwoClosedLinks);
    CPPUNIT_TEST(LinkIsCreatedCorrectly);
    CPPUNIT_TEST(LinkSequenceIsPreserved);
    CPPUNIT_TEST_SUITE_END();
public:
    static const OUString STYLE_NAME_1;
    static const OUString STYLE_NAME_2;
    static const sal_uInt16 POOL_ID_1;
    static const sal_uInt16 POOL_ID_2;
    static const OUString URL_1;
    static const OUString URL_2;
};

const OUString ToxLinkProcessorTest::STYLE_NAME_1 = "anyStyle1";
const OUString ToxLinkProcessorTest::STYLE_NAME_2 = "anyStyle2";
const OUString ToxLinkProcessorTest::URL_1 = "anyUrl1";
const OUString ToxLinkProcessorTest::URL_2 = "anyUrl2";
const sal_uInt16 ToxLinkProcessorTest::POOL_ID_1 = 42;
const sal_uInt16 ToxLinkProcessorTest::POOL_ID_2 = 43;

void
ToxLinkProcessorTest::ExceptionIsThrownIfTooManyLinksAreClosed()
{
    ToxLinkProcessor sut;
    sut.StartNewLink(0, STYLE_NAME_1);
    sut.CloseLink(1, URL_1);
    CPPUNIT_ASSERT_THROW(sut.CloseLink(1, URL_1), std::runtime_error);
}

void
ToxLinkProcessorTest::AddingAndClosingTwoLinksResultsInTwoClosedLinks()
{
    ToxLinkProcessor sut;
    sut.StartNewLink(0, STYLE_NAME_1);
    sut.StartNewLink(0, STYLE_NAME_2);
    sut.CloseLink(1, URL_1);
    sut.CloseLink(1, URL_2);
    CPPUNIT_ASSERT_EQUAL(2u, static_cast<unsigned>(sut.mClosedLinks.size()));
    CPPUNIT_ASSERT_MESSAGE("no links are open", sut.mStartedLinks.empty());
}

class ToxLinkProcessorWithOverriddenObtainPoolId : public ToxLinkProcessor {
public:
    virtual sal_uInt16
    ObtainPoolId(const OUString& characterStyle) const SAL_OVERRIDE {
        if (characterStyle == ToxLinkProcessorTest::STYLE_NAME_1) {
            return ToxLinkProcessorTest::POOL_ID_1;
        }
        if (characterStyle == ToxLinkProcessorTest::STYLE_NAME_2) {
            return ToxLinkProcessorTest::POOL_ID_2;
        }
        return 0;
    }
};

void
ToxLinkProcessorTest::LinkIsCreatedCorrectly()
{
    // obtainpoolid needs to be overridden to check what we are
    ToxLinkProcessorWithOverriddenObtainPoolId sut;

    sut.StartNewLink(0, STYLE_NAME_1);
    sut.CloseLink(1, URL_1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link", STYLE_NAME_1, sut.mClosedLinks.at(0).mINetFmt.GetVisitedFmt());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link", URL_1, sut.mClosedLinks.at(0).mINetFmt.GetValue());
}

void
ToxLinkProcessorTest::LinkSequenceIsPreserved()
{

    // obtainpoolid needs to be overridden to check what we are
    ToxLinkProcessorWithOverriddenObtainPoolId sut;

    sut.StartNewLink(0, STYLE_NAME_1);
    sut.StartNewLink(0, STYLE_NAME_2);
    sut.CloseLink(1, URL_2);
    sut.CloseLink(1, URL_1);

    // check first closed element
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link",
            STYLE_NAME_2, sut.mClosedLinks.at(0).mINetFmt.GetVisitedFmt());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pool id is stored correctly in link",
            POOL_ID_2, sut.mClosedLinks.at(0).mINetFmt.GetINetFmtId());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link",
            URL_2, sut.mClosedLinks.at(0).mINetFmt.GetValue());
    // check second closed element
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Style is stored correctly in link",
            STYLE_NAME_1, sut.mClosedLinks.at(1).mINetFmt.GetVisitedFmt());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Pool id is stored correctly in link",
            POOL_ID_1, sut.mClosedLinks.at(1).mINetFmt.GetINetFmtId());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Url is stored correctly in link",
            URL_1, sut.mClosedLinks.at(1).mINetFmt.GetValue());
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(ToxLinkProcessorTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
