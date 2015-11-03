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
#include <osl/conditn.hxx>
#include <svl/srchitem.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <unotools/tempfile.hxx>

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

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    };

    virtual void tearDown() override
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
    void testPaintTile();
    void testSaveAs();
    void testSaveAsCalc();
    void testPasteWriter();
    void testRowColumnHeaders();
    void testCommandResult();

    CPPUNIT_TEST_SUITE(DesktopLOKTest);
    CPPUNIT_TEST(testGetStyles);
    CPPUNIT_TEST(testGetFonts);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST(testGetFilterTypes);
    CPPUNIT_TEST(testGetPartPageRectangles);
    CPPUNIT_TEST(testSearchCalc);
    CPPUNIT_TEST(testPaintTile);
    CPPUNIT_TEST(testSaveAs);
    CPPUNIT_TEST(testSaveAsCalc);
    CPPUNIT_TEST(testPasteWriter);
    CPPUNIT_TEST(testRowColumnHeaders);
    CPPUNIT_TEST(testCommandResult);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference<lang::XComponent> mxComponent;
    OString m_aTextSelection;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;

    // for testCommandResult
    osl::Condition m_aCommandResultCondition;
    OString m_aCommandResult;
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
    case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
    {
        m_aSearchResultSelection.clear();
        boost::property_tree::ptree aTree;
        std::stringstream aStream(pPayload);
        boost::property_tree::read_json(aStream, aTree);
        for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("searchResultSelection"))
        {
            m_aSearchResultSelection.push_back(rValue.second.get<std::string>("rectangles").c_str());
            m_aSearchResultPart.push_back(std::atoi(rValue.second.get<std::string>("part").c_str()));
        }
    }
    break;
    case LOK_CALLBACK_UNO_COMMAND_RESULT:
    {
        m_aCommandResult = pPayload;
        m_aCommandResultCondition.set();
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
    // Make sure that we get exactly as many rectangle lists as matches.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), m_aSearchResultSelection.size());
    // Result is on the first sheet.
    CPPUNIT_ASSERT_EQUAL(0, m_aSearchResultPart[0]);

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testPaintTile()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    int nCanvasWidth = 100;
    int nCanvasHeight = 300;
    std::vector<unsigned char> aBuffer(nCanvasWidth * nCanvasHeight * 4);
    int nTilePosX = 0;
    int nTilePosY = 0;
    int nTileWidth = 1000;
    int nTileHeight = 3000;

    // This used to crash: painTile() implementation did not handle
    // nCanvasWidth != nCanvasHeight correctly, as usually both are just always
    // 256.
    pDocument->pClass->paintTile(pDocument, aBuffer.data(), nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    // This crashed in OutputDevice::DrawDeviceAlphaBitmap().
    nCanvasWidth = 200;
    nCanvasHeight = 200;
    nTileWidth = 4000;
    nTileHeight = 4000;
    aBuffer.resize(nCanvasWidth * nCanvasHeight * 4);
    pDocument->pClass->paintTile(pDocument, aBuffer.data(), nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

void DesktopLOKTest::testSaveAs()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    CPPUNIT_ASSERT(pDocument->pClass->saveAs(pDocument, aTempFile.GetURL().toUtf8().getStr(), "png", 0));
}

void DesktopLOKTest::testSaveAsCalc()
{
    LibLODocument_Impl* pDocument = loadDoc("search.ods");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    CPPUNIT_ASSERT(pDocument->pClass->saveAs(pDocument, aTempFile.GetURL().toUtf8().getStr(), "png", 0));
}

void DesktopLOKTest::testPasteWriter()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    OString aText("hello");

    CPPUNIT_ASSERT(pDocument->pClass->paste(pDocument, "text/plain;charset=utf-8", aText.getStr(), aText.getLength()));

    pDocument->pClass->postUnoCommand(pDocument, ".uno:SelectAll", 0, false);
    char* pText = pDocument->pClass->getTextSelection(pDocument, "text/plain;charset=utf-8", 0);
    CPPUNIT_ASSERT_EQUAL(OString("hello"), OString(pText));
    free(pText);

    // textt/plain should be rejected.
    CPPUNIT_ASSERT(!pDocument->pClass->paste(pDocument, "textt/plain;charset=utf-8", aText.getStr(), aText.getLength()));
    // Writer is expected to support text/html.
    CPPUNIT_ASSERT(pDocument->pClass->paste(pDocument, "text/html", aText.getStr(), aText.getLength()));

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testRowColumnHeaders()
{
    LibLODocument_Impl* pDocument = loadDoc("search.ods");
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewRowColumnHeaders");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        CPPUNIT_ASSERT(nSize > 0);
        OString aText(rValue.second.get<std::string>("text").c_str());
        // This failed, as the first item did not contain the text of the first row.
        CPPUNIT_ASSERT_EQUAL(OString("1"), aText);
        break;
    }

    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("columns"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        CPPUNIT_ASSERT(nSize > 0);
        OString aText(rValue.second.get<std::string>("text").c_str());
        CPPUNIT_ASSERT_EQUAL(OString("A"), aText);
        break;
    }
}

void DesktopLOKTest::testCommandResult()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    // the postUnoCommand() is supposed to be async, let's test it safely
    // [no idea if it is async in reality - most probably we are operating
    // under some solar mutex or something anyway ;-) - but...]
    m_aCommandResultCondition.reset();

    pDocument->pClass->postUnoCommand(pDocument, ".uno:Bold", 0, true);

    TimeValue aTimeValue = { 2 , 0 }; // 2 seconds max
    m_aCommandResultCondition.wait(aTimeValue);

    boost::property_tree::ptree aTree;
    std::stringstream aStream(m_aCommandResult.getStr());
    boost::property_tree::read_json(aStream, aTree);

    CPPUNIT_ASSERT_EQUAL(aTree.get_child("commandName").get_value<std::string>(), std::string(".uno:Bold"));
    CPPUNIT_ASSERT_EQUAL(aTree.get_child("success").get_value<bool>(), true);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLOKTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
