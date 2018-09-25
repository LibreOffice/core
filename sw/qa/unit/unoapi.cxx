/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>

#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

class UnoApiTest : public SwModelTestBase
{
public:
    UnoApiTest()
        : SwModelTestBase("sw/qa/unit/data/", "writer8")
    {
    }
};

/**
 * Macro to declare a new test with preloaded file
 * (similar to DECLARE_SW_ROUNDTRIP_TEST)
 */
#define DECLARE_UNOAPI_TEST_FILE(TestName, filename)                                               \
    class TestName : public UnoApiTest                                                             \
    {                                                                                              \
    protected:                                                                                     \
        virtual OUString getTestName() override { return OUString(#TestName); }                    \
                                                                                                   \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(loadAndTest);                                                                 \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void loadAndTest()                                                                         \
        {                                                                                          \
            load(mpTestDocumentPath, filename);                                                    \
            runTest();                                                                             \
        }                                                                                          \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

/**
 * Macro to declare a new test without loading any files
 */
#define DECLARE_UNOAPI_TEST(TestName)                                                              \
    class TestName : public CppUnit::TestFixture                                                   \
    {                                                                                              \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(runTest);                                                                     \
        CPPUNIT_TEST_SUITE_END();                                                                  \
        void runTest();                                                                            \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::runTest()

static bool ensureAutoTextExistsByTitle(const Reference<XAutoTextGroup>& autoTextGroup,
                                        const OUString& autoTextName)
{
    uno::Sequence<OUString> aTitles(autoTextGroup->getTitles());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

static bool ensureAutoTextExistsByName(const Reference<XAutoTextGroup>& autoTextGroup,
                                       const OUString& autoTextName)
{
    uno::Sequence<OUString> aTitles(autoTextGroup->getElementNames());
    for (const auto& rTitle : aTitles)
    {
        if (rTitle == autoTextName)
            return true;
    }
    return false;
}

DECLARE_UNOAPI_TEST_FILE(testXAutoTextGroup, "xautotextgroup.odt")
{
    Reference<XAutoTextContainer> xAutoTextContainer
        = AutoTextContainer::create(comphelper::getProcessComponentContext());

    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    const OUString sGroupName = "TestGroup*1";
    const OUString sTextName = "TEST";
    const OUString sTextNameNew = "TESTRENAMED";
    const OUString sTextTitle = "Test Auto Text";
    const OUString sTextTitleNew = "Test Auto Text Renamed";

    // Create new temporary group
    Reference<XAutoTextGroup> xAutoTextGroup(xAutoTextContainer->insertNewByName(sGroupName),
                                             uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("AutoTextGroup was not found!", xAutoTextGroup.is());

    // Insert new element and ensure it exists
    Reference<XAutoTextEntry> xAutoTextEntry
        = xAutoTextGroup->insertNewByName(sTextName, sTextTitle, xTextRange);
    CPPUNIT_ASSERT_MESSAGE("AutoText was not inserted!", xAutoTextEntry.is());
    CPPUNIT_ASSERT_MESSAGE("Can't find newly created AutoText by title!",
                           ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitle));
    CPPUNIT_ASSERT_MESSAGE("Can't find newly created AutoText by name!",
                           ensureAutoTextExistsByName(xAutoTextGroup, sTextName));

    // Insert once again the same should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on insertion of same AutoText",
                                 xAutoTextGroup->insertNewByName(sTextName, sTextTitle, xTextRange),
                                 container::ElementExistException);

    // Rename it & ensure everything is ok
    xAutoTextGroup->renameByName(sTextName, sTextNameNew, sTextTitleNew);
    CPPUNIT_ASSERT_MESSAGE("Can't find renamed AutoText by title!",
                           ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitleNew));
    CPPUNIT_ASSERT_MESSAGE("Can't find renamed AutoText by name!",
                           ensureAutoTextExistsByName(xAutoTextGroup, sTextNameNew));
    // Not found by old names
    CPPUNIT_ASSERT_MESSAGE("Found AutoText by old title!",
                           !ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitle));
    CPPUNIT_ASSERT_MESSAGE("Found AutoText by old name!",
                           !ensureAutoTextExistsByName(xAutoTextGroup, sTextName));

    // Rename not existing should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE(
        "We expect an exception on renaming not-exising AutoText",
        xAutoTextGroup->renameByName(sTextName, sTextNameNew, sTextTitleNew),
        container::ElementExistException);

    // Remove it and ensure it does not exist
    xAutoTextGroup->removeByName(sTextNameNew);
    CPPUNIT_ASSERT_MESSAGE("AutoText was not removed!",
                           !ensureAutoTextExistsByTitle(xAutoTextGroup, sTextTitleNew));
    CPPUNIT_ASSERT_MESSAGE("AutoText was not removed!",
                           !ensureAutoTextExistsByName(xAutoTextGroup, sTextNameNew));

    // Remove non-existing element should throw an exception
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on removing not-existing AutoText",
                                 xAutoTextGroup->removeByName(sTextName),
                                 container::NoSuchElementException);

    // Remove our temporary group
    xAutoTextContainer->removeByName(sGroupName);
}

DECLARE_UNOAPI_TEST(testXURI)
{
    Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());

    // createKnown()
    Reference<rdf::XURI> xURIcreateKnown(rdf::URI::createKnown(xContext, rdf::URIs::ODF_PREFIX),
                                         UNO_SET_THROW);
    CPPUNIT_ASSERT(xURIcreateKnown.is());
    CPPUNIT_ASSERT_EQUAL(OUString("http://docs.oasis-open.org/ns/office/1.2/meta/odf#"),
                         xURIcreateKnown->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("prefix"), xURIcreateKnown->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://docs.oasis-open.org/ns/office/1.2/meta/odf#prefix"),
                         xURIcreateKnown->getStringValue());

    // createKnown() with invalid constant
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid constant",
                                 rdf::URI::createKnown(xContext, 12345),
                                 lang::IllegalArgumentException);

    // create()
    Reference<rdf::XURI> xURIcreate(rdf::URI::create(xContext, "http://example.com/url#somedata"),
                                    UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreate->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"), xURIcreate->getStringValue());

    // create() without local name splitted with "/"
    Reference<rdf::XURI> xURIcreate2(rdf::URI::create(xContext, "http://example.com/url"),
                                     UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/"), xURIcreate2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("url"), xURIcreate2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url"), xURIcreate2->getStringValue());

    // create() without prefix
    Reference<rdf::XURI> xURIcreate3(rdf::URI::create(xContext, "#somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("#"), xURIcreate3->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreate3->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("#somedata"), xURIcreate3->getStringValue());

    // create() with invalid URI
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::create(xContext, "some junk and not URI"),
                                 lang::IllegalArgumentException);

    // createNS()
    Reference<rdf::XURI> xURIcreateNS(
        rdf::URI::createNS(xContext, "http://example.com/url#", "somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#"), xURIcreateNS->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("somedata"), xURIcreateNS->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/url#somedata"),
                         xURIcreateNS->getStringValue());

    // TODO: What's going on here? Is such usecase valid?
    Reference<rdf::XURI> xURIcreateNS2(
        rdf::URI::createNS(xContext, "http://example.com/url", "somedata"), UNO_SET_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/"), xURIcreateNS2->getNamespace());
    CPPUNIT_ASSERT_EQUAL(OUString("urlsomedata"), xURIcreateNS2->getLocalName());
    CPPUNIT_ASSERT_EQUAL(OUString("http://example.com/urlsomedata"),
                         xURIcreateNS2->getStringValue());

    // createNS() some invalid cases
    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, "bla", "bla"),
                                 lang::IllegalArgumentException);

    CPPUNIT_ASSERT_THROW_MESSAGE("We expect an exception on invalid URI",
                                 rdf::URI::createNS(xContext, OUString(), OUString()),
                                 lang::IllegalArgumentException);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
