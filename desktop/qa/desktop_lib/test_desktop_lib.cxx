/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/lokhelper.hxx>
#include <test/unoapi_test.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/propertysequence.hxx>
#include <svl/srchitem.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include "../../inc/lib/init.hxx"

using namespace com::sun::star;
using namespace desktop;

class DesktopLOKTest : public UnoApiTest
{
public:
    DesktopLOKTest() : UnoApiTest("/desktop/qa/data/")
    {
    }

    virtual ~DesktopLOKTest()
    {
    }

    virtual void setUp() SAL_OVERRIDE
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    };

    virtual void tearDown() SAL_OVERRIDE
    {
        closeDoc();
        UnoApiTest::tearDown();
    };

    LibLODocument_Impl* loadDoc(const char* pName, LibreOfficeKitDocumentType eType = LOK_DOCTYPE_TEXT);
    void closeDoc();
    static void callback(int nType, const char* pPayload, void* pData);
    void callbackImpl(int nType, const char* pPayload);

    void testGetStyles();
    void testGetFonts();
    void testCreateView();
    void testGetFilterTypes();
    void testGetPartPageRectangles();
    void testSearchCalc();

    CPPUNIT_TEST_SUITE(DesktopLOKTest);
    CPPUNIT_TEST(testGetStyles);
    CPPUNIT_TEST(testGetFonts);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST(testGetFilterTypes);
    CPPUNIT_TEST(testGetPartPageRectangles);
    CPPUNIT_TEST(testSearchCalc);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference<lang::XComponent> mxComponent;
    OString m_aTextSelection;
};

LibLODocument_Impl* DesktopLOKTest::loadDoc(const char* pName, LibreOfficeKitDocumentType eType)
{
    OUString aFileURL;
    createFileURL(OUString::createFromAscii(pName), aFileURL);
    OUString aService;
    switch (eType)
    {
    case LOK_DOCTYPE_TEXT:
        aService = "com.sun.star.text.TextDocument";
        break;
    case LOK_DOCTYPE_SPREADSHEET:
        aService = "com.sun.star.sheet.SpreadsheetDocument";
        break;
    default:
        CPPUNIT_ASSERT(false);
        break;
    }
    mxComponent = loadFromDesktop(aFileURL, aService);
    if (!mxComponent.is())
    {
        CPPUNIT_ASSERT(false);
    }
    return new LibLODocument_Impl(mxComponent);
}

void DesktopLOKTest::closeDoc()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }
}

void DesktopLOKTest::callback(int nType, const char* pPayload, void* pData)
{
    static_cast<DesktopLOKTest*>(pData)->callbackImpl(nType, pPayload);
}

void DesktopLOKTest::callbackImpl(int nType, const char* pPayload)
{
    switch (nType)
    {
    case LOK_CALLBACK_TEXT_SELECTION:
    {
        m_aTextSelection = pPayload;
    }
    break;
    }
}

void DesktopLOKTest::testGetStyles()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:StyleApply");
    std::stringstream aStream(pJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( aTree.size() > 0 );
    CPPUNIT_ASSERT( aTree.get_child("commandName").get_value<std::string>() == ".uno:StyleApply" );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const std::pair<std::string, boost::property_tree::ptree>& rPair : aValues)
    {
        CPPUNIT_ASSERT( rPair.second.size() > 0);
        if (rPair.first != "CharacterStyles" &&
            rPair.first != "ParagraphStyles" &&
            rPair.first != "FrameStyles" &&
            rPair.first != "PageStyles" &&
            rPair.first != "NumberingStyles" &&
            rPair.first != "CellStyles" &&
            rPair.first != "ShapeStyles")
        {
            CPPUNIT_FAIL("Unknown style family: " + rPair.first);
        }
    }
    closeDoc();
}

void DesktopLOKTest::testGetFonts()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:CharFontName");
    std::stringstream aStream(pJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( aTree.size() > 0 );
    CPPUNIT_ASSERT( aTree.get_child("commandName").get_value<std::string>() == ".uno:CharFontName" );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const std::pair<std::string, boost::property_tree::ptree>& rPair : aValues)
    {
        // check that we have font sizes available for each font
        CPPUNIT_ASSERT( rPair.second.size() > 0);
    }
    closeDoc();
}

void DesktopLOKTest::testCreateView()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getViews(pDocument));

    int nId = pDocument->m_pDocumentClass->createView(pDocument);
    CPPUNIT_ASSERT_EQUAL(2, pDocument->m_pDocumentClass->getViews(pDocument));

    // Make sure the created view is the active one, then switch to the old
    // one.
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getView(pDocument));
    pDocument->m_pDocumentClass->setView(pDocument, 0);
    CPPUNIT_ASSERT_EQUAL(0, pDocument->m_pDocumentClass->getView(pDocument));

    pDocument->m_pDocumentClass->destroyView(pDocument, nId);
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getViews(pDocument));
    closeDoc();
}

void DesktopLOKTest::testGetPartPageRectangles()
{
    // Test that we get as many page rectangles as expected: blank document is
    // one page.
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    char* pRectangles = pDocument->pClass->getPartPageRectangles(pDocument);
    OUString sRectangles = OUString::fromUtf8(pRectangles);

    std::vector<OUString> aRectangles;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aRectangle = sRectangles.getToken(0, ';', nIndex);
        if (!aRectangle.isEmpty())
            aRectangles.push_back(aRectangle);
    }
    while (nIndex >= 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aRectangles.size());

    free(pRectangles);
    closeDoc();
}

void DesktopLOKTest::testGetFilterTypes()
{
    LibLibreOffice_Impl aOffice;
    char* pJSON = aOffice.m_pOfficeClass->getFilterTypes(&aOffice);

    std::stringstream aStream(pJSON);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);

    CPPUNIT_ASSERT(aTree.size() > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("application/vnd.oasis.opendocument.text"), aTree.get_child("writer8").get_child("MediaType").get_value<std::string>());
    free(pJSON);
}

void DesktopLOKTest::testSearchCalc()
{
    LibLibreOffice_Impl aOffice;
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("search.ods");
    pDocument->pClass->initializeForRendering(pDocument);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("foo"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);

    std::vector<OString> aSelections;
    sal_Int32 nIndex = 0;
    do
    {
        OString aToken = m_aTextSelection.getToken(0, ';', nIndex);
        aSelections.push_back(aToken);
    } while (nIndex >= 0);
    // This was 1, find-all only found one match.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aSelections.size());

    closeDoc();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLOKTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
