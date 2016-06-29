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
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
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
#include <vcl/svapp.hxx>

#include <lib/init.hxx>

using namespace com::sun::star;
using namespace desktop;

class DesktopLOKTest : public UnoApiTest
{
public:
    DesktopLOKTest() : UnoApiTest("/desktop/qa/data/"),
    m_nSelectionBeforeSearchResult(0),
    m_nSelectionAfterSearchResult(0),
    m_bModified(false)
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
    void testSearchAllNotificationsCalc();
    void testPaintTile();
    void testSaveAs();
    void testSaveAsCalc();
    void testPasteWriter();
    void testPasteWriterJPEG();
    void testRowColumnHeaders();
    void testHiddenRowHeaders();
    void testCellCursor();
    void testCommandResult();
    void testWriterComments();
    void testModifiedStatus();
    void testSheetOperations();
    void testSheetSelections();
    void testContextMenuCalc();
    void testContextMenuWriter();
    void testContextMenuImpress();
    void testNotificationCompression();

    CPPUNIT_TEST_SUITE(DesktopLOKTest);
    CPPUNIT_TEST(testModifiedStatus);
    CPPUNIT_TEST(testGetStyles);
    CPPUNIT_TEST(testGetFonts);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST(testGetFilterTypes);
    CPPUNIT_TEST(testGetPartPageRectangles);
    CPPUNIT_TEST(testSearchCalc);
    CPPUNIT_TEST(testSearchAllNotificationsCalc);
    CPPUNIT_TEST(testPaintTile);
    CPPUNIT_TEST(testSaveAs);
    CPPUNIT_TEST(testSaveAsCalc);
    CPPUNIT_TEST(testPasteWriter);
    CPPUNIT_TEST(testPasteWriterJPEG);
    CPPUNIT_TEST(testRowColumnHeaders);
    CPPUNIT_TEST(testHiddenRowHeaders);
    CPPUNIT_TEST(testCellCursor);
    CPPUNIT_TEST(testCommandResult);
    CPPUNIT_TEST(testWriterComments);
    CPPUNIT_TEST(testSheetOperations);
    CPPUNIT_TEST(testSheetSelections);
    CPPUNIT_TEST(testContextMenuCalc);
    CPPUNIT_TEST(testContextMenuWriter);
    CPPUNIT_TEST(testContextMenuImpress);
    CPPUNIT_TEST(testNotificationCompression);
    CPPUNIT_TEST_SUITE_END();

    uno::Reference<lang::XComponent> mxComponent;
    OString m_aTextSelection;
    std::vector<OString> m_aSearchResultSelection;
    std::vector<int> m_aSearchResultPart;
    int m_nSelectionBeforeSearchResult;
    int m_nSelectionAfterSearchResult;

    // for testCommandResult
    osl::Condition m_aCommandResultCondition;
    OString m_aCommandResult;

    // for testModifiedStatus
    osl::Condition m_aStateChangedCondition;
    bool m_bModified;

    // for testContextMenu{Calc, Writer}
    osl::Condition m_aContextMenuCondition;
    boost::property_tree::ptree m_aContextMenuResult;
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
    case LOK_DOCTYPE_PRESENTATION:
        aService = "com.sun.star.presentation.PresentationDocument";
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
        if (m_aSearchResultSelection.empty())
            ++m_nSelectionBeforeSearchResult;
        else
            ++m_nSelectionAfterSearchResult;
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
    case LOK_CALLBACK_STATE_CHANGED:
    {
        OString aPayload(pPayload);
        OString aPrefix(".uno:ModifiedStatus=");
        if (aPayload.startsWith(aPrefix))
        {
            m_bModified = aPayload.copy(aPrefix.getLength()).toBoolean();
            m_aStateChangedCondition.set();
        }
    }
    break;
    case LOK_CALLBACK_CONTEXT_MENU:
    {
        m_aContextMenuResult.clear();
        std::stringstream aStream(pPayload);
        boost::property_tree::read_json(aStream, m_aContextMenuResult);
        m_aContextMenuCondition.set();
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
        if( rPair.first != "ClearStyle")
        {
            CPPUNIT_ASSERT( rPair.second.size() > 0);
        }
        if (rPair.first != "CharacterStyles" &&
            rPair.first != "ParagraphStyles" &&
            rPair.first != "FrameStyles" &&
            rPair.first != "PageStyles" &&
            rPair.first != "NumberingStyles" &&
            rPair.first != "CellStyles" &&
            rPair.first != "ShapeStyles" &&
            rPair.first != "Commands")
        {
            CPPUNIT_FAIL("Unknown style family: " + rPair.first);
        }
    }
}

void DesktopLOKTest::testGetFonts()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_presentation.odp");
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
    free(pJSON);
    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testCreateView()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getViews(pDocument));

    int nId0 = pDocument->m_pDocumentClass->getView(pDocument);
    int nId1 = pDocument->m_pDocumentClass->createView(pDocument);
    CPPUNIT_ASSERT_EQUAL(2, pDocument->m_pDocumentClass->getViews(pDocument));

    // Make sure the created view is the active one, then switch to the old
    // one.
    CPPUNIT_ASSERT_EQUAL(nId1, pDocument->m_pDocumentClass->getView(pDocument));
    pDocument->m_pDocumentClass->setView(pDocument, nId0);
    CPPUNIT_ASSERT_EQUAL(nId0, pDocument->m_pDocumentClass->getView(pDocument));

    pDocument->m_pDocumentClass->destroyView(pDocument, nId1);
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
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("foo"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));

    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

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

void DesktopLOKTest::testSearchAllNotificationsCalc()
{
    LibLibreOffice_Impl aOffice;
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("search.ods");
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", uno::makeAny(OUString("foo"))},
        {"SearchItem.Backward", uno::makeAny(false)},
        {"SearchItem.Command", uno::makeAny(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    comphelper::dispatchCommand(".uno:ExecuteSearch", aPropertyValues);
    Scheduler::ProcessEventsToIdle();

    // This was 1, make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, m_nSelectionBeforeSearchResult);
    // But we do get the selection afterwards.
    CPPUNIT_ASSERT(m_nSelectionAfterSearchResult > 0);

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
    CPPUNIT_ASSERT(pDocument->pClass->saveAs(pDocument, aTempFile.GetURL().toUtf8().getStr(), "png", nullptr));
}

void DesktopLOKTest::testSaveAsCalc()
{
    LibLODocument_Impl* pDocument = loadDoc("search.ods");
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    CPPUNIT_ASSERT(pDocument->pClass->saveAs(pDocument, aTempFile.GetURL().toUtf8().getStr(), "png", nullptr));
}

void DesktopLOKTest::testPasteWriter()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    OString aText("hello");

    CPPUNIT_ASSERT(pDocument->pClass->paste(pDocument, "text/plain;charset=utf-8", aText.getStr(), aText.getLength()));

    pDocument->pClass->postUnoCommand(pDocument, ".uno:SelectAll", nullptr, false);
    char* pText = pDocument->pClass->getTextSelection(pDocument, "text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(OString("hello"), OString(pText));
    free(pText);

    // textt/plain should be rejected.
    CPPUNIT_ASSERT(!pDocument->pClass->paste(pDocument, "textt/plain;charset=utf-8", aText.getStr(), aText.getLength()));
    // Writer is expected to support text/html.
    CPPUNIT_ASSERT(pDocument->pClass->paste(pDocument, "text/html", aText.getStr(), aText.getLength()));

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testPasteWriterJPEG()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");

    OUString aFileURL;
    createFileURL(OUString::createFromAscii("paste.jpg"), aFileURL);
    std::ifstream aImageStream(aFileURL.toUtf8().copy(strlen("file://")).getStr());
    std::vector<char> aImageContents((std::istreambuf_iterator<char>(aImageStream)), std::istreambuf_iterator<char>());

    CPPUNIT_ASSERT(pDocument->pClass->paste(pDocument, "image/jpeg", aImageContents.data(), aImageContents.size()));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 0, JPEG was not handled as a format for clipboard paste.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // This was text::TextContentAnchorType_AT_PARAGRAPH.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, xShape->getPropertyValue("AnchorType").get<text::TextContentAnchorType>());

    // Delete the pasted picture, and paste again with a custom anchor type.
    uno::Reference<lang::XComponent>(xShape, uno::UNO_QUERY)->dispose();
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AnchorType", uno::makeAny(static_cast<sal_uInt16>(text::TextContentAnchorType_AT_CHARACTER))},
    }));
    comphelper::dispatchCommand(".uno:Paste", aPropertyValues);
    xShape.set(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // This was text::TextContentAnchorType_AS_CHARACTER, AnchorType argument was ignored.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, xShape->getPropertyValue("AnchorType").get<text::TextContentAnchorType>());

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testRowColumnHeaders()
{
    /*
     * Payload example:
     *
     * {
     *     "rows": [
     *         {
     *             "size": "254.987250637468",
     *             "text": "1"
     *         },
     *         {
     *             "size": "509.974501274936",
     *             "text": "2"
     *         }
     *     ],
     *     "columns": [
     *         {
     *             "size": "1274.93625318734",
     *             "text": "A"
     *         },
     *         {
     *             "size": "2549.87250637468",
     *             "text": "B"
     *         }
     *     ]
     * }
     *
     * "size" defines the bottom/right boundary of a row/column in twips (size between 0 and boundary)
     * "text" has the header label in UTF-8
     */
    LibLODocument_Impl* pDocument = loadDoc("search.ods");

    pDocument->pClass->initializeForRendering(pDocument, nullptr);

    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewRowColumnHeaders");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        CPPUNIT_ASSERT(nSize > 0);
        OString aText(rValue.second.get<std::string>("text").c_str());
        if (!nPrevious)
            // This failed, as the first item did not contain the text of the first row.
            CPPUNIT_ASSERT_EQUAL(OString("1"), aText);
        else
        {
            // Make sure that size is absolute: the first two items have the same relative size.
            CPPUNIT_ASSERT(nPrevious < nSize);
            break;
        }
        nPrevious = nSize;
    }

    nPrevious = 0;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("columns"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        CPPUNIT_ASSERT(nSize > 0);
        OString aText(rValue.second.get<std::string>("text").c_str());
        if (!nPrevious)
            CPPUNIT_ASSERT_EQUAL(OString("A"), aText);
        else
        {
            CPPUNIT_ASSERT(nPrevious < nSize);
            break;
        }
        nPrevious = nSize;
    }
}

void DesktopLOKTest::testHiddenRowHeaders()
{
    LibLODocument_Impl* pDocument = loadDoc("hidden-row.ods");

    pDocument->pClass->initializeForRendering(pDocument, nullptr);

    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewRowColumnHeaders");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    bool bFirst = true;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        CPPUNIT_ASSERT(nSize > 0);

        if (bFirst)
            bFirst = false;
        else
        {
            // nSize was 509, nPrevious was 254, i.e. hidden row wasn't reported as 0 height.
            CPPUNIT_ASSERT_EQUAL(nPrevious, nSize);
            break;
        }

        nPrevious = nSize;
    }
}

void DesktopLOKTest::testCellCursor()
{
    LibLODocument_Impl* pDocument = loadDoc("search.ods");

    boost::property_tree::ptree aTree;

    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:CellCursor?tileWidth=1&tileHeight=1&outputWidth=1&outputHeight=1");

    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);

    OString aRectangle(aTree.get<std::string>("commandValues").c_str());
    CPPUNIT_ASSERT_EQUAL(aRectangle, OString("0, 0, 1278, 254"));
}

void DesktopLOKTest::testCommandResult()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");

    // the postUnoCommand() is supposed to be async, let's test it safely
    // [no idea if it is async in reality - most probably we are operating
    // under some solar mutex or something anyway ;-) - but...]
    TimeValue aTimeValue = { 2 , 0 }; // 2 seconds max

    // nothing is triggered when we have no callback yet, we just time out on
    // the condition var.
    m_aCommandResultCondition.reset();
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Bold", nullptr, true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(&aTimeValue);

    CPPUNIT_ASSERT(m_aCommandResult.isEmpty());

    // but we get some real values when the callback is set up
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    m_aCommandResultCondition.reset();
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Bold", nullptr, true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(&aTimeValue);

    boost::property_tree::ptree aTree;
    std::stringstream aStream(m_aCommandResult.getStr());
    boost::property_tree::read_json(aStream, aTree);

    CPPUNIT_ASSERT_EQUAL(aTree.get_child("commandName").get_value<std::string>(), std::string(".uno:Bold"));
    CPPUNIT_ASSERT_EQUAL(aTree.get_child("success").get_value<bool>(), true);
}

void DesktopLOKTest::testWriterComments()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);
    uno::Reference<awt::XReschedule> xToolkit(com::sun::star::awt::Toolkit::create(comphelper::getProcessComponentContext()), uno::UNO_QUERY);

    // Insert a comment at the beginning of the document and wait till the main
    // loop grabs the focus, so characters end up in the annotation window.
    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aCommandResultCondition.reset();
    pDocument->pClass->postUnoCommand(pDocument, ".uno:InsertAnnotation", nullptr, true);
    Scheduler::ProcessEventsToIdle();

    m_aCommandResultCondition.wait(&aTimeValue);
    CPPUNIT_ASSERT(!m_aCommandResult.isEmpty());
    xToolkit->reschedule();

    // Test that we have a comment.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnumeration = xParagraphEnumerationAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphEnumeration->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextPortionEnumeration = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xTextPortionEnumeration->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), xTextPortion->getPropertyValue("TextPortionType").get<OUString>());

    // Type "test" and finish editing.
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 'e', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 's', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 0, com::sun::star::awt::Key::ESCAPE);

    // Test that the typed characters ended up in the right window.
    auto xTextField = xTextPortion->getPropertyValue("TextField").get< uno::Reference<beans::XPropertySet> >();
    // This was empty, typed characters ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("test"), xTextField->getPropertyValue("Content").get<OUString>());

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testModifiedStatus()
{
    LibLibreOffice_Impl aOffice;
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    // Type "t" and check that the document was set as modified
    m_bModified = false;
    m_aStateChangedCondition.reset();
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    Scheduler::ProcessEventsToIdle();
    TimeValue aTimeValue = { 2 , 0 }; // 2 seconds max
    m_aStateChangedCondition.wait(&aTimeValue);
    Scheduler::ProcessEventsToIdle();
    Scheduler::ProcessEventsToIdle();

    // This was false, there was no callback about the modified status change.
    CPPUNIT_ASSERT(m_bModified);

    // Perform SaveAs with "TakeOwnership" option set, and check that the
    // modification state was reset
    m_aStateChangedCondition.reset();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    CPPUNIT_ASSERT(pDocument->pClass->saveAs(pDocument, aTempFile.GetURL().toUtf8().getStr(), "odt", "TakeOwnership"));
    Scheduler::ProcessEventsToIdle();
    m_aStateChangedCondition.wait(&aTimeValue);
    Scheduler::ProcessEventsToIdle();
    Scheduler::ProcessEventsToIdle();

    // There was no callback about the modified status change.
    CPPUNIT_ASSERT(!m_bModified);

    // Modify the document again
    m_aStateChangedCondition.reset();
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    Scheduler::ProcessEventsToIdle();
    m_aStateChangedCondition.wait(&aTimeValue);
    Scheduler::ProcessEventsToIdle();
    Scheduler::ProcessEventsToIdle();

    // There was no callback about the modified status change.
    CPPUNIT_ASSERT(m_bModified);

    /*
    // TODO: move this to a test where LOK is fully bootstrapped, so that we can
    // get back the notification about ".uno:Save" too
    // Now perform a normal "Save", and check the modified state was reset
    // again
    m_aStateChangedCondition.reset();
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Save", nullptr, false);
    m_aStateChangedCondition.wait(&aTimeValue);
    Scheduler::ProcessEventsToIdle();

    // There was no callback about the modified status change.
    CPPUNIT_ASSERT(!m_bModified);
    */

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testSheetOperations()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("sheets.ods");

    // insert the last sheet
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Insert",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"LastSheet\" }, \"Index\": { \"type\": \"long\", \"value\": 0 } }", false);

    // insert the first sheet
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Insert",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"FirstSheet\" }, \"Index\": { \"type\": \"long\", \"value\": 1 } }", false);

    // rename the \"Sheet1\" (2nd now) to \"Renamed\"
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Name",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"Renamed\" }, \"Index\": { \"type\": \"long\", \"value\": 2 } }", false);

    // delete the \"Sheet2\" (3rd)
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Remove",
          "{ \"Index\": { \"type\": \"long\", \"value\": 3 } }", false);

    CPPUNIT_ASSERT_EQUAL(pDocument->pClass->getParts(pDocument), 6);

    std::vector<OString> pExpected = { "FirstSheet", "Renamed", "Sheet3", "Sheet4", "Sheet5", "LastSheet" };
    for (int i = 0; i < 6; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(pExpected[i], OString(pDocument->pClass->getPartName(pDocument, i)));
    }

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testSheetSelections()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("sheets.ods", LOK_DOCTYPE_SPREADSHEET);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    /*
     * Check if selection data is correct
     */
    // Values in twips
    int row5 = 1150;
    int col1 = 1100;
    int col2 = 2200;
    int col3 = 3300;
    int col4 = 4400;
    int col5 = 5500;

    // Select row 5 from column 1 through column 5
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      col1, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEMOVE,
                                      col2, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEMOVE,
                                      col3, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEMOVE,
                                      col4, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEMOVE,
                                      col5, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      col5, row5,
                                      1, 1, 0);

    // Copy the contents and check if matches expected data
    {
        char* pUsedMimeType = nullptr;
        char* pCopiedContent = pDocument->pClass->getTextSelection(pDocument, nullptr, &pUsedMimeType);
        std::vector<int> pExpected = {5, 6, 7, 8, 9};
        std::istringstream iss(pCopiedContent);
        for (uint i = 0; i < pExpected.size(); i++)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(pExpected[i], std::stoi(token));
        }

        free(pUsedMimeType);
        free(pCopiedContent);
    }

    /*
     * Check if clicking inside the selection deselects the whole selection
     */
    int row10 = 2400;
    // Select starting from row5, col1 to row10, col5
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      col1, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEMOVE,
                                      col5, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      col5, row10,
                                      1, 1, 0);

    // Click at row5, col4
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      col4, row5,
                                      1, 1, 0);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONUP,
                                      col4, row5,
                                      1, 1, 0);

    // Selected text should get deselected and copying should give us
    // content of only one cell, now
    {
        char* pUsedMimeType  = nullptr;
        char* pCopiedContent = pDocument->pClass->getTextSelection(pDocument, nullptr, &pUsedMimeType);
        std::vector<int> pExpected = { 8 };
        std::istringstream iss(pCopiedContent);
        for (uint i = 0; i < pExpected.size(); i++)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(pExpected[i], std::stoi(token));
        }

        free(pUsedMimeType);
        free(pCopiedContent);
    }

    comphelper::LibreOfficeKit::setActive(false);
}

namespace {

    void verifyContextMenuStructure(boost::property_tree::ptree& aRoot)
    {
        for (const auto& aItemPair: aRoot)
        {
            // This is an array, so no key
            CPPUNIT_ASSERT_EQUAL(std::string(aItemPair.first.data()), std::string(""));

            boost::property_tree::ptree aItemValue = aItemPair.second;
            boost::optional<boost::property_tree::ptree&> aText = aItemValue.get_child_optional("text");
            boost::optional<boost::property_tree::ptree&> aType = aItemValue.get_child_optional("type");
            boost::optional<boost::property_tree::ptree&> aCommand = aItemValue.get_child_optional("command");
            boost::optional<boost::property_tree::ptree&> aSubmenu = aItemValue.get_child_optional("menu");
            boost::optional<boost::property_tree::ptree&> aEnabled = aItemValue.get_child_optional("enabled");
            boost::optional<boost::property_tree::ptree&> aChecktype = aItemValue.get_child_optional("checktype");
            boost::optional<boost::property_tree::ptree&> aChecked = aItemValue.get_child_optional("checked");

            // type is omnipresent
            CPPUNIT_ASSERT( aType );

            // seperator doesn't have any other attribs
            if ( aType.get().data() == "separator" )
            {
                CPPUNIT_ASSERT( !aText && !aCommand && !aSubmenu && !aEnabled && !aChecktype && !aChecked );
            }
            else if ( aType.get().data() == "command" )
            {
                CPPUNIT_ASSERT( aCommand && aText );
            }
            else if ( aType.get().data() == "menu")
            {
                CPPUNIT_ASSERT( aSubmenu && aText );
                verifyContextMenuStructure( aSubmenu.get() );
            }

            if ( aChecktype )
            {
                CPPUNIT_ASSERT( aChecktype.get().data() == "radio" ||
                                aChecktype.get().data() == "checkmark" ||
                                aChecktype.get().data() == "auto" );

                CPPUNIT_ASSERT( aChecked &&
                                ( aChecked.get().data() == "true" || aChecked.get().data() == "false" ) );
            }
        }

    }

} // end anonymous namespace

void DesktopLOKTest::testContextMenuCalc()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("sheets.ods", LOK_DOCTYPE_SPREADSHEET);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    // Values in twips
    int row5 = 1150;
    int col1 = 1100;

    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      col1, row5,
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aContextMenuCondition.wait(&aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testContextMenuWriter()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt", LOK_DOCTYPE_TEXT);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    Point aRandomPoint(1150, 1100);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      aRandomPoint.X(), aRandomPoint.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aContextMenuCondition.wait(&aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testContextMenuImpress()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_presentation.odp", LOK_DOCTYPE_PRESENTATION);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    Point aRandomPoint(1150, 1100);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      aRandomPoint.X(), aRandomPoint.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aContextMenuCondition.wait(&aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    comphelper::LibreOfficeKit::setActive(false);
}

static void callbackCompressionTest(const int type, const char* payload, void* data)
{
    std::vector<std::tuple<int, std::string>>* notifs = static_cast<std::vector<std::tuple<int, std::string>>*>(data);
    notifs->emplace_back(type, std::string(payload ? payload : "(nil)"));
}

void DesktopLOKTest::testNotificationCompression()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    std::vector<std::tuple<int, std::string>> notifs;
    std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

    handler->queue(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, ""); // 0
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15 25 15 10"); // Superseeded.
    handler->queue(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, ""); // Should be dropped.
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15 25 15 10"); // Superseeded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15 25 15 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, ""); // Superseeded.
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ""); // 2
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ".uno:Bold"); // 3
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ""); // 4
    handler->queue(LOK_CALLBACK_MOUSE_POINTER, "text"); // 5
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15 25 15 10"); // 6
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15 25 15 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_MOUSE_POINTER, "text"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15 25 15 10"); // Superseeded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15 25 15 10"); // Superseeded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15 25 15 10"); // Superseedd.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15 25 15 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15 25 15 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, ""); // 7
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15 25 15 10"); // 8
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15 25 15 10"); // 9
    handler->queue(LOK_CALLBACK_CELL_CURSOR, "15 25 15 10"); // 10
    handler->queue(LOK_CALLBACK_CURSOR_VISIBLE, ""); // 11
    handler->queue(LOK_CALLBACK_CELL_CURSOR, "15 25 15 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_CELL_FORMULA, "blah"); // 12
    handler->queue(LOK_CALLBACK_SET_PART, "1"); // 13
    handler->queue(LOK_CALLBACK_CURSOR_VISIBLE, ""); // Should be dropped.
    handler->queue(LOK_CALLBACK_CELL_FORMULA, "blah"); // Should be dropped.
    handler->queue(LOK_CALLBACK_SET_PART, "1"); // Should be dropped.

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(13), notifs.size());

    size_t i = 0;
    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Bold"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_MOUSE_POINTER, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("text"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15 25 15 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION_START, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15 25 15 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION_END, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15 25 15 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CELL_CURSOR, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15 25 15 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CURSOR_VISIBLE, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CELL_FORMULA, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("blah"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_SET_PART, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), std::get<1>(notifs[i++]));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLOKTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
