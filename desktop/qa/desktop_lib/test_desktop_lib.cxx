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
#include <rtl/uri.hxx>
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
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <unotools/datetime.hxx>
#include <comphelper/string.hxx>
#include <comphelper/scopeguard.hxx>
#include <cairo.h>
#include <ostream>
#include <vcl/scheduler.hxx>

#include <lib/init.hxx>

using namespace com::sun::star;
using namespace desktop;

namespace com { namespace sun { namespace star { namespace text {
std::ostream& operator<<(std::ostream& os, css::text::TextContentAnchorType const & n)
{
    os << (sal_Int32)n;
    return os;
}
} } } };

class DesktopLOKTest : public UnoApiTest
{
public:
    DesktopLOKTest() : UnoApiTest("/desktop/qa/data/"),
    m_nSelectionBeforeSearchResult(0),
    m_nSelectionAfterSearchResult(0),
    m_bModified(false),
    m_nTrackChanges(0)
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
        SfxApplication::GetOrCreate();
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
    void testUndoWriter();
    void testRowColumnHeaders();
    void testHiddenRowHeaders();
    void testCellCursor();
    void testCommandResult();
    void testWriterComments();
    void testSheetOperations();
    void testSheetSelections();
    void testContextMenuCalc();
    void testContextMenuWriter();
    void testContextMenuImpress();
    void testNotificationCompression();
    void testTileInvalidationCompression();
    void testPartInInvalidation();
    void testRedlineWriter();
    void testTrackChanges();
    void testRedlineCalc();
    void testPaintPartTile();
    void testWriterCommentInsertCursor();
    void testGetFontSubset();
    void testCommentsWriter();
    void testCommentsCalc();
    void testCommentsImpress();
    void testCommentsCallbacksWriter();
    void testRunMacro();
    void testExtractParameter();

    CPPUNIT_TEST_SUITE(DesktopLOKTest);
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
    CPPUNIT_TEST(testUndoWriter);
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
    CPPUNIT_TEST(testTileInvalidationCompression);
    CPPUNIT_TEST(testPartInInvalidation);
    CPPUNIT_TEST(testRedlineWriter);
    CPPUNIT_TEST(testTrackChanges);
    CPPUNIT_TEST(testRedlineCalc);
    CPPUNIT_TEST(testPaintPartTile);
    CPPUNIT_TEST(testWriterCommentInsertCursor);
    CPPUNIT_TEST(testGetFontSubset);
    CPPUNIT_TEST(testCommentsWriter);
    CPPUNIT_TEST(testCommentsCalc);
    CPPUNIT_TEST(testCommentsImpress);
    CPPUNIT_TEST(testCommentsCallbacksWriter);
    CPPUNIT_TEST(testRunMacro);
    CPPUNIT_TEST(testExtractParameter);
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
    int m_nTrackChanges;

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
        else if (aPayload.startsWith(".uno:TrackChanges=") && aPayload.endsWith("=true"))
            ++m_nTrackChanges;
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
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:StyleApply"), aTree.get_child("commandName").get_value<std::string>()  );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const auto& rPair : aValues)
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
            rPair.first != "TableStyles" &&
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
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:CharFontName"), aTree.get_child("commandName").get_value<std::string>() );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    for (const auto& rPair : aValues)
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
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getViewsCount(pDocument));

    int nId0 = pDocument->m_pDocumentClass->getView(pDocument);
    int nId1 = pDocument->m_pDocumentClass->createView(pDocument);
    CPPUNIT_ASSERT_EQUAL(2, pDocument->m_pDocumentClass->getViewsCount(pDocument));

    // Test getViewIds().
    std::vector<int> aViewIds(2);
    CPPUNIT_ASSERT(pDocument->m_pDocumentClass->getViewIds(pDocument, aViewIds.data(), aViewIds.size()));
    CPPUNIT_ASSERT_EQUAL(nId0, aViewIds[0]);
    CPPUNIT_ASSERT_EQUAL(nId1, aViewIds[1]);

    // Make sure the created view is the active one, then switch to the old
    // one.
    CPPUNIT_ASSERT_EQUAL(nId1, pDocument->m_pDocumentClass->getView(pDocument));
    pDocument->m_pDocumentClass->setView(pDocument, nId0);
    CPPUNIT_ASSERT_EQUAL(nId0, pDocument->m_pDocumentClass->getView(pDocument));

    pDocument->m_pDocumentClass->destroyView(pDocument, nId1);
    CPPUNIT_ASSERT_EQUAL(1, pDocument->m_pDocumentClass->getViewsCount(pDocument));
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
    sal_Int32 nStride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, nCanvasWidth);
    std::vector<unsigned char> aBuffer(nStride * nCanvasHeight);
    int nTilePosX = 0;
    int nTilePosY = 0;
    int nTileWidth = 1000;
    int nTileHeight = 3000;

    // This used to crash: paintTile() implementation did not handle
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
    createFileURL("paste.jpg", aFileURL);
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

void DesktopLOKTest::testUndoWriter()
{
    // Load a Writer document and press a key.
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 't', 0);

    // Get undo info.
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:Undo");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one undo action.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("actions").size());

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

    long nWidth = 0;
    long nHeight = 0;
    pDocument->m_pDocumentClass->getDocumentSize(pDocument, &nWidth, &nHeight);
    long nX = rtl::math::round(nWidth / 4.0);
    long nY = rtl::math::round(nHeight / 4.0);
    nWidth = rtl::math::round(nWidth / 2.0);
    nHeight = rtl::math::round(nHeight / 2.0);

    std::stringstream aPayload;
    aPayload << ".uno:ViewRowColumnHeaders?x=" << nX << "&y=" << nY << "&width=" << nWidth << "&height=" << nHeight;

    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, aPayload.str().c_str());
    std::stringstream aStream(pJSON);
    free(pJSON);

    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    bool bFirstHeader = true;
    bool bNotEnoughHeaders = true;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        OString aText(rValue.second.get<std::string>("text").c_str());

        if (bFirstHeader)
        {
            CPPUNIT_ASSERT(nSize <= nY);
            CPPUNIT_ASSERT_EQUAL(OString("11"), aText);
            bFirstHeader = false;
        }
        else
        {
            CPPUNIT_ASSERT(nSize > 0);
            CPPUNIT_ASSERT(nPrevious < nSize);
            if (nSize > nY + nHeight)
            {
                bNotEnoughHeaders = false;
                break;
            }
        }
        nPrevious = nSize;
    }
    CPPUNIT_ASSERT(!bNotEnoughHeaders);

    nPrevious = 0;
    bFirstHeader = true;
    bNotEnoughHeaders = true;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("columns"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();
        OString aText(rValue.second.get<std::string>("text").c_str());
        if (bFirstHeader)
        {
            CPPUNIT_ASSERT(nSize <= nX);
            CPPUNIT_ASSERT_EQUAL(OString("4"), aText);
            bFirstHeader = false;
        }
        else
        {
            CPPUNIT_ASSERT(nSize > 0);
            CPPUNIT_ASSERT(nPrevious < nSize);
            if (nSize > nX + nWidth)
            {
                bNotEnoughHeaders = false;
                break;
            }
        }
        nPrevious = nSize;
    }
    CPPUNIT_ASSERT(!bNotEnoughHeaders);
}

void DesktopLOKTest::testHiddenRowHeaders()
{
    LibLODocument_Impl* pDocument = loadDoc("hidden-row.ods");

    pDocument->pClass->initializeForRendering(pDocument, nullptr);

    long nX = 0;
    long nY = 0;
    long nWidth = 0;
    long nHeight = 0;
    pDocument->m_pDocumentClass->getDocumentSize(pDocument, &nWidth, &nHeight);

    std::stringstream aPayload;
    aPayload << ".uno:ViewRowColumnHeaders?x=" << nX << "&y=" << nY << "&width=" << nWidth << "&height=" << nHeight;

    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, aPayload.str().c_str());
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    sal_Int32 nIndex = 0;
    for (boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = OString(rValue.second.get<std::string>("size").c_str()).toInt32();

        if (nIndex++ == 2)
        {
            // nSize was 510, nPrevious was 255, i.e. hidden row wasn't reported as 0 height.
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
    CPPUNIT_ASSERT_EQUAL(OString("0, 0, 1279, 255"), aRectangle);
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
    m_aCommandResultCondition.wait(aTimeValue);

    CPPUNIT_ASSERT(m_aCommandResult.isEmpty());

    // but we get some real values when the callback is set up
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    m_aCommandResultCondition.reset();
    pDocument->pClass->postUnoCommand(pDocument, ".uno:Bold", nullptr, true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);

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
    m_aCommandResultCondition.wait(aTimeValue);
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

void DesktopLOKTest::testTrackChanges()
{
    // Load a document and create two views.
    LibLibreOffice_Impl aOffice;
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);
    pDocument->pClass->createView(pDocument);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);
    Scheduler::ProcessEventsToIdle();

    // Enable trak changes and assert that both views get notified.
    m_nTrackChanges = 0;
    pDocument->pClass->postUnoCommand(pDocument, ".uno:TrackChanges", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    // This was 1, only the active view was notified.
    CPPUNIT_ASSERT_EQUAL(2, m_nTrackChanges);

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

    std::vector<OString> aExpected = { "FirstSheet", "Renamed", "Sheet3", "Sheet4", "Sheet5", "LastSheet" };
    for (int i = 0; i < 6; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(aExpected[i], OString(pDocument->pClass->getPartName(pDocument, i)));
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
        std::vector<long> aExpected = {5, 6, 7, 8, 9};
        std::istringstream iss(pCopiedContent);
        for (size_t i = 0; i < aExpected.size(); i++)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], strtol(token.c_str(), nullptr, 10));
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
        std::vector<long> aExpected = { 8 };
        std::istringstream iss(pCopiedContent);
        for (size_t i = 0; i < aExpected.size(); i++)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], strtol(token.c_str(), nullptr, 10));
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

            // separator doesn't have any other attribs
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

    boost::optional<boost::property_tree::ptree>
    getContextMenuItem(boost::property_tree::ptree& aMenu, std::string const & unoSelector)
    {
        boost::optional<boost::property_tree::ptree> aMenuItem;
        for (const auto& aItemPair: aMenu)
        {
            boost::property_tree::ptree aItemValue = aItemPair.second;

            boost::optional<boost::property_tree::ptree&> aCommand = aItemValue.get_child_optional("command");
            if (aCommand && aCommand.get().data() == unoSelector )
            {
                aMenuItem = aItemValue;
                break;
            }
        }

        return aMenuItem;
    }

} // end anonymous namespace

void DesktopLOKTest::testContextMenuCalc()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("sheet_with_image.ods", LOK_DOCTYPE_SPREADSHEET);
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    // Values in twips
    Point aPointOnImage(1150, 1100);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      aPointOnImage.X(), aPointOnImage.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aContextMenuCondition.wait(aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    // tests for calc specific context menu
    // Cut is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Cut");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("true"));
    }

    // Copy is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Copy");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("true"));
    }

    // Paste is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Paste");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("true"));
    }

    // Remove hyperlink is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:DeleteShapeHyperlink");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // open hyperlink is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:OpenHyperlinkOnCursor");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // checkbutton tests
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:AnchorMenu");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aSubmenu = aMenuItem.get().get_child_optional("menu");
        CPPUNIT_ASSERT(aSubmenu);

        boost::optional<boost::property_tree::ptree> aMenuItemToPage = getContextMenuItem(aSubmenu.get(), ".uno:SetAnchorToPage");
        CPPUNIT_ASSERT(aMenuItemToPage);

        boost::optional<boost::property_tree::ptree> aMenuItemToCell = getContextMenuItem(aSubmenu.get(), ".uno:SetAnchorToCell");
        CPPUNIT_ASSERT(aMenuItemToCell);

        // these are radio buttons
        boost::optional<boost::property_tree::ptree&> aChecktypeToPage = aMenuItemToPage.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeToPage);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktypeToPage.get().data()), std::string("radio"));

        boost::optional<boost::property_tree::ptree&> aChecktypeToCell = aMenuItemToCell.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeToCell);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktypeToCell.get().data()), std::string("radio"));

        // ToPage is checked
        boost::optional<boost::property_tree::ptree&> aCheckedToPage = aMenuItemToPage.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedToPage);
        CPPUNIT_ASSERT_EQUAL(std::string(aCheckedToPage.get().data()), std::string("true"));

        // ToCell is unchecked
        boost::optional<boost::property_tree::ptree&> aCheckedToCell = aMenuItemToCell.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedToCell);
        CPPUNIT_ASSERT_EQUAL(std::string(aCheckedToCell.get().data()), std::string("false"));
    }

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testContextMenuWriter()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    pDocument->pClass->initializeForRendering(pDocument, nullptr);
    pDocument->pClass->registerCallback(pDocument, &DesktopLOKTest::callback, this);

    Point aRandomPoint(1150, 1100);
    pDocument->pClass->postMouseEvent(pDocument,
                                      LOK_MOUSEEVENT_MOUSEBUTTONDOWN,
                                      aRandomPoint.X(), aRandomPoint.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue aTimeValue = {2 , 0}; // 2 seconds max
    m_aContextMenuCondition.wait(aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    // tests for writer specific context menu
   // Cut is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Cut");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // Copy is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Copy");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // Paste is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Paste");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("true"));
    }

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
    m_aContextMenuCondition.wait(aTimeValue);

    CPPUNIT_ASSERT( !m_aContextMenuResult.empty() );
    boost::optional<boost::property_tree::ptree&> aMenu = m_aContextMenuResult.get_child_optional("menu");
    CPPUNIT_ASSERT( aMenu );
    verifyContextMenuStructure( aMenu.get() );

    // tests for impress specific context menu
    // Cut is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Cut");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // Copy is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Copy");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // Paste is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Paste");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("true"));
    }

    // SaveBackground is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:SaveBackground");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(std::string(aEnabled.get().data()), std::string("false"));
    }

    // checkbutton tests
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:ShowRuler");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aChecktype = aMenuItem.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktype);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktype.get().data()), std::string("checkmark"));

        boost::optional<boost::property_tree::ptree&> aChecked = aMenuItem.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aChecked);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecked.get().data()), std::string("false"));
    }

    // Checkbutton tests inside SnapLines submenu
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:SnapLinesMenu");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aSubmenu = aMenuItem.get().get_child_optional("menu");
        CPPUNIT_ASSERT(aSubmenu);

        boost::optional<boost::property_tree::ptree> aMenuItemHelpVis = getContextMenuItem(aSubmenu.get(), ".uno:HelplinesVisible");
        CPPUNIT_ASSERT(aMenuItemHelpVis);

        boost::optional<boost::property_tree::ptree> aMenuItemHelpUse = getContextMenuItem(aSubmenu.get(), ".uno:HelplinesUse");
        CPPUNIT_ASSERT(aMenuItemHelpUse);

        boost::optional<boost::property_tree::ptree> aMenuItemHelpFront = getContextMenuItem(aSubmenu.get(), ".uno:HelplinesFront");
        CPPUNIT_ASSERT(aMenuItemHelpFront);

        // these are checkmarks
        boost::optional<boost::property_tree::ptree&> aChecktypeHelpVis = aMenuItemHelpVis.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeHelpVis);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktypeHelpVis.get().data()), std::string("checkmark"));

        boost::optional<boost::property_tree::ptree&> aChecktypeHelpUse = aMenuItemHelpUse.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeHelpUse);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktypeHelpUse.get().data()), std::string("checkmark"));

        boost::optional<boost::property_tree::ptree&> aChecktypeHelpFront = aMenuItemHelpFront.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeHelpFront);
        CPPUNIT_ASSERT_EQUAL(std::string(aChecktypeHelpFront.get().data()), std::string("checkmark"));

        // HelplineVisible is unchecked
        boost::optional<boost::property_tree::ptree&> aCheckedHelpVis = aMenuItemHelpVis.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedHelpVis);
        CPPUNIT_ASSERT_EQUAL(std::string(aCheckedHelpVis.get().data()), std::string("false"));

        // HelplineUse is checked
        boost::optional<boost::property_tree::ptree&> aCheckedHelpUse = aMenuItemHelpUse.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedHelpUse);
        CPPUNIT_ASSERT_EQUAL(std::string(aCheckedHelpUse.get().data()), std::string("true"));

        // HelplineFront is checked
        boost::optional<boost::property_tree::ptree&> aCheckedHelpFront = aMenuItemHelpFront.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedHelpFront);
        CPPUNIT_ASSERT_EQUAL(std::string(aCheckedHelpFront.get().data()), std::string("true"));
    }

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
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15, 25, 15, 10"); // Superseded.
    handler->queue(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, ""); // Should be dropped.
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15, 25, 15, 10"); // 1
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, ""); // Superseded.
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ""); // 2
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ".uno:Bold"); // 3
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ""); // 4
    handler->queue(LOK_CALLBACK_MOUSE_POINTER, "text"); // 5
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_MOUSE_POINTER, "text"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15, 25, 15, 10"); // Superseded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15, 25, 15, 10"); // Superseded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, "15, 25, 15, 10"); // Superseded.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_TEXT_SELECTION, ""); // 7
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_START, "15, 25, 15, 10"); // 8
    handler->queue(LOK_CALLBACK_TEXT_SELECTION_END, "15, 25, 15, 10"); // 9
    handler->queue(LOK_CALLBACK_CELL_CURSOR, "15, 25, 15, 10"); // 10
    handler->queue(LOK_CALLBACK_CURSOR_VISIBLE, ""); // 11
    handler->queue(LOK_CALLBACK_CELL_CURSOR, "15, 25, 15, 10"); // Should be dropped.
    handler->queue(LOK_CALLBACK_CELL_FORMULA, "blah"); // 12
    handler->queue(LOK_CALLBACK_SET_PART, "1"); // 13
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ".uno:AssignLayout=20"); // Superseded
    handler->queue(LOK_CALLBACK_CURSOR_VISIBLE, ""); // Should be dropped.
    handler->queue(LOK_CALLBACK_CELL_FORMULA, "blah"); // Should be dropped.
    handler->queue(LOK_CALLBACK_SET_PART, "1"); // Should be dropped.
    handler->queue(LOK_CALLBACK_STATE_CHANGED, ".uno:AssignLayout=1"); // 14

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), notifs.size());

    size_t i = 0;
    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Bold"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_MOUSE_POINTER, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("text"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION_START, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_TEXT_SELECTION_END, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CELL_CURSOR, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CURSOR_VISIBLE, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_CELL_FORMULA, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("blah"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_SET_PART, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_STATE_CHANGED, (int)std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:AssignLayout=1"), std::get<1>(notifs[i++]));
}

void DesktopLOKTest::testTileInvalidationCompression()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");

    comphelper::LibreOfficeKit::setPartInInvalidation(true);
    comphelper::ScopeGuard aGuard([]()
    {
        comphelper::LibreOfficeKit::setPartInInvalidation(false);
    });

    // Single part merging
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "-100, -50, 500, 650, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, -32767, -32767, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "100, 100, 200, 200, 0");

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 400, 600, 0"), std::get<1>(notifs[i++]));
    }

    // Part Number
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 200, 200, 1"); // Different part
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 0, 0, 2"); // Invalid
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "-121, -121, 200, 200, 0"); // Inside first
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, -32767, -32767, 1"); // Invalid

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 1"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 239, 239, 0"), std::get<1>(notifs[i++]));
    }

    // All Parts
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 0"); // 0
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 200, 200, 1"); // 1: Different part
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 0, 0, -1"); // Invalid
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "-121, -121, 200, 200, -1"); // 0: All parts
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, -32767, -32767, -1"); // Invalid
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "-100, -100, 1200, 1200, -1"); // 0: All parts
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 3"); // Overlapped
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "1000, 1000, 1239, 1239, 2"); // 1: Unique region

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 1100, 1100, -1"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("1000, 1000, 1239, 1239, 2"), std::get<1>(notifs[i++]));
    }

    // All Parts (partial)
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 200, 200, 0"); // 0
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 100, 100, 1"); // 1: Different part
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 0, 0, -1"); // Invalid
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "150, 150, 50, 50, -1"); // 2: All-parts
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, -32767, -32767, -1"); // Invalid
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "150, 150, 40, 40, 3"); // Overlapped w/ 2
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 200, 200, 4"); // 3: Unique
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "1000, 1000, 1239, 1239, 1"); // 4: Unique

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 100, 100, 1"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("150, 150, 50, 50, -1"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 4"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("1000, 1000, 1239, 1239, 1"), std::get<1>(notifs[i++]));
    }

    // Merge with "EMPTY"
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 239, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "EMPTY, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, 239, 240, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "-121, -121, 300, 300, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "0, 0, -32767, -32767, 0");

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 1000000000, 1000000000, 0"), std::get<1>(notifs[i++]));
    }
}

void DesktopLOKTest::testPartInInvalidation()
{
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    // No part in invalidation: merge.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "10, 10, 20, 10");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "20, 10, 20, 10");

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        CPPUNIT_ASSERT_EQUAL((int)LOK_CALLBACK_INVALIDATE_TILES, (int)std::get<0>(notifs[0]));
        CPPUNIT_ASSERT_EQUAL(std::string("10, 10, 30, 10"), std::get<1>(notifs[0]));
    }
    // No part in invalidation: don't merge.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "10, 10, 20, 10");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "40, 10, 20, 10");

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());
    }

    // Part in invalidation, intersection and parts match -> merge.
    {
        comphelper::LibreOfficeKit::setPartInInvalidation(true);
        comphelper::ScopeGuard aGuard([]()
        {
            comphelper::LibreOfficeKit::setPartInInvalidation(false);
        });

        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "10, 10, 20, 10, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "20, 10, 20, 10, 0");

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());
    }
    // Part in invalidation, intersection and parts don't match -> don't merge.
    {
        comphelper::LibreOfficeKit::setPartInInvalidation(true);
        comphelper::ScopeGuard aGuard([]()
        {
            comphelper::LibreOfficeKit::setPartInInvalidation(false);
        });

        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));

        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "10, 10, 20, 10, 0");
        handler->queue(LOK_CALLBACK_INVALIDATE_TILES, "20, 10, 20, 10, 1");

        Scheduler::ProcessEventsToIdle();

        // This failed as RectangleAndPart::Create() always assumed no part in
        // payload, so this was merged -> it was 1.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());
    }
}

void DesktopLOKTest::testRedlineWriter()
{
    // Load a Writer document, enable change recording and press a key.
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 't', 0);

    // Get redline info.
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:AcceptTrackedChanges");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("redlines").size());

    for (boost::property_tree::ptree::value_type& rRedline : aTree.get_child("redlines"))
        // This failed with boost::property_tree::ptree_bad_path, as there were no description field.
        CPPUNIT_ASSERT_EQUAL(std::string("Insert “t”"), rRedline.second.get<std::string>("description"));

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testRedlineCalc()
{
    // Load a Writer document, enable change recording and press a key.
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("sheets.ods");
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("RecordChanges", uno::makeAny(true));
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 't', 0);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 0, KEY_RETURN);
    pDocument->pClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 0, KEY_RETURN);

    // Get redline info.
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:AcceptTrackedChanges");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("redlines").size());

    for (boost::property_tree::ptree::value_type& rRedline : aTree.get_child("redlines"))
        // This failed with boost::property_tree::ptree_bad_path, as there were no description field.
        CPPUNIT_ASSERT_EQUAL(std::string("Cell B4 changed from '5' to 't'"), rRedline.second.get<std::string>("description"));

    comphelper::LibreOfficeKit::setActive(false);
}

class ViewCallback
{
public:
    bool m_bTilesInvalidated;
    tools::Rectangle m_aOwnCursor;
    boost::property_tree::ptree m_aCommentCallbackResult;

    ViewCallback()
        : m_bTilesInvalidated(false)
    {
    }

    static void callback(int nType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(nType, pPayload);
    }

    void callbackImpl(int nType, const char* pPayload)
    {
        OString aPayload(pPayload);
        switch (nType)
        {
        case LOK_CALLBACK_INVALIDATE_TILES:
        {
            m_bTilesInvalidated = true;
        }
        break;
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        {
            uno::Sequence<OUString> aSeq = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aPayload));
            if (OString("EMPTY") == pPayload)
                return;
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aOwnCursor.setX(aSeq[0].toInt32());
            m_aOwnCursor.setY(aSeq[1].toInt32());
            m_aOwnCursor.setWidth(aSeq[2].toInt32());
            m_aOwnCursor.setHeight(aSeq[3].toInt32());
        }
        break;
        case LOK_CALLBACK_COMMENT:
        {
            m_aCommentCallbackResult.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aCommentCallbackResult);
            m_aCommentCallbackResult = m_aCommentCallbackResult.get_child("comment");
        }
        break;
        }
    }
};

void DesktopLOKTest::testPaintPartTile()
{
    // Load an impress doc of 2 slides.
    comphelper::LibreOfficeKit::setActive();
    ViewCallback aView1;
    ViewCallback aView2;
    std::unique_ptr<LibLODocument_Impl> xDocument(loadDoc("2slides.odp"));
    LibLODocument_Impl* pDocument = xDocument.get();
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView1);
    int nView1 = pDocument->m_pDocumentClass->getView(pDocument);

    // Create a second view.
    pDocument->m_pDocumentClass->createView(pDocument);
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView2);

    // Go to the second slide in the second view.
    pDocument->m_pDocumentClass->setPart(pDocument, 1);

    // Switch back to the first view and start typing.
    pDocument->m_pDocumentClass->setView(pDocument, nView1);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 0, awt::Key::TAB);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 0, awt::Key::TAB);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 'x', 0);

    // Call paintPartTile() to paint the second part (in whichever view it finds suitable for this).
    unsigned char pPixels[256 * 256 * 4];
    pDocument->m_pDocumentClass->paintPartTile(pDocument, pPixels, 1, 256, 256, 0, 0, 256, 256);

    // Type again.
    Scheduler::ProcessEventsToIdle();
    aView1.m_bTilesInvalidated = false;
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // This failed: paintPartTile() (as a side-effect) ended the text edit of
    // the first view, so there were no invalidations.
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);

    Scheduler::ProcessEventsToIdle();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testWriterCommentInsertCursor()
{
    // Load a document and type a character into the body text of the second view.
    comphelper::LibreOfficeKit::setActive();
    ViewCallback aView1;
    ViewCallback aView2;
    std::unique_ptr<LibLODocument_Impl> xDocument(loadDoc("blank_text.odt"));
    LibLODocument_Impl* pDocument = xDocument.get();
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView1);
    pDocument->m_pDocumentClass->createView(pDocument);
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView2);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 'x', 0);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    tools::Rectangle aBodyCursor = aView2.m_aOwnCursor;

    // Now insert a comment and make sure that the comment's cursor is shown,
    // not the body text's one.
    aView1.m_aOwnCursor.SetEmpty();
    const int nCtrlAltC = KEY_MOD1 + KEY_MOD2 + 512 + 'c' - 'a';
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYINPUT, 'c', nCtrlAltC);
    pDocument->m_pDocumentClass->postKeyEvent(pDocument, LOK_KEYEVENT_KEYUP, 'c', nCtrlAltC);
    Scheduler::ProcessEventsToIdle();
    // Wait for SfxBindings to actually update the state, which updated the
    // cursor as well.
    osl::Thread::wait(std::chrono::seconds(1));
    Scheduler::ProcessEventsToIdle();
    // This failed: the body cursor was shown right after inserting a comment.
    CPPUNIT_ASSERT(aView2.m_aOwnCursor.getX() > aBodyCursor.getX());
    // This failed, the first view's cursor also jumped when the second view
    // inserted the comment.
    CPPUNIT_ASSERT(aView1.m_aOwnCursor.IsEmpty());

    Scheduler::ProcessEventsToIdle();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testGetFontSubset()
{
    comphelper::LibreOfficeKit::setActive();
    LibLODocument_Impl* pDocument = loadDoc("blank_text.odt");
    OUString aFontName = rtl::Uri::encode(
        OUString("Liberation Sans"),
        rtl_UriCharClassRelSegment,
        rtl_UriEncodeKeepEscapes,
        RTL_TEXTENCODING_UTF8
    );
    OUString aUnoFontSubset(".uno:FontSubset&name=");
    OString aCommand = OUStringToOString(aUnoFontSubset + aFontName, RTL_TEXTENCODING_UTF8);
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, aCommand.getStr());
    std::stringstream aStream(pJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( aTree.size() > 0 );
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:FontSubset"), aTree.get_child("commandName").get_value<std::string>() );
    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( aValues.size() > 0 );
    free(pJSON);
    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testCommentsWriter()
{
    comphelper::LibreOfficeKit::setActive();
    // Disable tiled rendering for comments
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    LibLODocument_Impl* pDocument = loadDoc("comments.odt");
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, nullptr);
    long nWidth, nHeight;
    pDocument->m_pDocumentClass->getDocumentSize(pDocument, &nWidth, &nHeight);

    // Document width alongwith without sidebar comes to be < 13000
    CPPUNIT_ASSERT( nWidth < 13000 );

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewAnnotations");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // There are 3 comments in the document already
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), aTree.get_child("comments").size());

    int nComment2Id = 0;
    // Check if all comment fields have valid data
    for (const auto& rComment : aTree.get_child("comments"))
    {
        CPPUNIT_ASSERT(rComment.second.get<int>("id") > 0);
        CPPUNIT_ASSERT(!rComment.second.get<std::string>("author").empty());
        CPPUNIT_ASSERT(!rComment.second.get<std::string>("text").empty());
        // Has a valid iso 8601 date time string
        css::util::DateTime aDateTime;
        OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime").c_str());
        CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));

        // This comment has a marked text range
        if (rComment.second.get<std::string>("text") == "Comment 2")
        {
            CPPUNIT_ASSERT(!rComment.second.get<std::string>("textRange").empty());
            nComment2Id = rComment.second.get<int>("id");
        }
        // This is a reply comment
        else if (rComment.second.get<std::string>("text") == "Reply to Comment 2")
        {
            CPPUNIT_ASSERT_EQUAL(nComment2Id, rComment.second.get<int>("parent"));
        }
    }

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
    comphelper::LibreOfficeKit::setActive(false);
}


void DesktopLOKTest::testCommentsCalc()
{
    comphelper::LibreOfficeKit::setActive();
    // Disable tiled rendering for comments
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    LibLODocument_Impl* pDocument = loadDoc("sheets.ods");
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, nullptr);

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewAnnotations");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // There are 2 comments in the document already
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTree.get_child("comments").size());

    // Check if all comment fields have valid data
    int nIdx = 0;
    for (const auto& rComment : aTree.get_child("comments"))
    {
        switch(nIdx)
        {
            case 0:
            {
                CPPUNIT_ASSERT_EQUAL(std::string("Sheet5.G15"), rComment.second.get<std::string>("id"));
                CPPUNIT_ASSERT_EQUAL(std::string("Comment1"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("7650, 3570, 1274, 254"), rComment.second.get<std::string>("cellPos"));
            }
            break;
            case 1:
            {
                CPPUNIT_ASSERT_EQUAL(std::string("Sheet5.H18"), rComment.second.get<std::string>("id"));
                CPPUNIT_ASSERT_EQUAL(std::string("Comment2"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("8925, 4335, 1274, 254"), rComment.second.get<std::string>("cellPos"));
            }
            break;
        }

        ++nIdx;
    }

    // We checked all the comments
    CPPUNIT_ASSERT_EQUAL(2, nIdx);

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
    comphelper::LibreOfficeKit::setActive(false);
}


void DesktopLOKTest::testCommentsImpress()
{
    comphelper::LibreOfficeKit::setActive();
    // Disable tiled rendering for comments
    comphelper::LibreOfficeKit::setTiledAnnotations(false);

    LibLODocument_Impl* pDocument = loadDoc("blank_presentation.odp");
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, nullptr);

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewAnnotations");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // There are 2 comments in the document already
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), aTree.get_child("comments").size());

    // Check if all comment fields have valid data
    int nIdx = 0;
    for (const auto& rComment : aTree.get_child("comments"))
    {
        switch(nIdx)
        {
            case 0:
            {
                CPPUNIT_ASSERT(rComment.second.get<int>("id") > 0);
                CPPUNIT_ASSERT_EQUAL(std::string("This is comment1"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("LOK User1"), rComment.second.get<std::string>("author"));
                css::util::DateTime aDateTime;
                OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime").c_str());
                CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));
            }
            break;
            case 1:
            {
                CPPUNIT_ASSERT(rComment.second.get<int>("id") > 0);
                CPPUNIT_ASSERT_EQUAL(std::string("This is comment2"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("LOK User2"), rComment.second.get<std::string>("author"));
                css::util::DateTime aDateTime;
                OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime").c_str());
                CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));
            }
            break;
        }

        ++nIdx;
    }

    // We checked all the comments
    CPPUNIT_ASSERT_EQUAL(2, nIdx);

    comphelper::LibreOfficeKit::setTiledAnnotations(true);
    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testCommentsCallbacksWriter()
{
    comphelper::LibreOfficeKit::setActive();
    // Comments callback are emitted only if tiled annotations are off
    comphelper::LibreOfficeKit::setTiledAnnotations(false);
    ViewCallback aView1;
    ViewCallback aView2;
    LibLODocument_Impl* pDocument = loadDoc("comments.odt");
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView1);
    pDocument->m_pDocumentClass->createView(pDocument);
    pDocument->m_pDocumentClass->initializeForRendering(pDocument, "{}");
    pDocument->m_pDocumentClass->registerCallback(pDocument, &ViewCallback::callback, &aView2);

    // Add a new comment
    OString aCommandArgs("{ \"Text\": { \"type\": \"string\", \"value\": \"Additional comment\" }, \"Author\": { \"type\": \"string\", \"value\": \"LOK User1\" } }");
    pDocument->pClass->postUnoCommand(pDocument, ".uno:InsertAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    int nCommentId1 = aView1.m_aCommentCallbackResult.get<int>("id");

    // Reply to a comment just added
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId1) + "\" }, \"Text\": { \"type\": \"string\", \"value\": \"Reply comment\" } }";
    pDocument->pClass->postUnoCommand(pDocument, ".uno:ReplyComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action and linked to its parent comment
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("Reply comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("Reply comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));
    int nCommentId2 = aView1.m_aCommentCallbackResult.get<int>("id");

    // Edit the previously added comment
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId2) + "\" }, \"Text\": { \"type\": \"string\", \"value\": \"Edited comment\" } }";
    pDocument->pClass->postUnoCommand(pDocument, ".uno:EditAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    // parent is unchanged still
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("Edited comment"), aView2.m_aCommentCallbackResult.get<std::string>("text"));

    // Delete the reply comment just added
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\":  \"" + OString::number(nCommentId2) + "\" } }";
    pDocument->pClass->postUnoCommand(pDocument, ".uno:DeleteComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Remove' action
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId2, aView1.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(nCommentId2, aView2.m_aCommentCallbackResult.get<int>("id"));

    // Reply to nCommentId1 again
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId1) + "\" }, \"Text\": { \"type\": \"string\", \"value\": \"Reply comment again\" } }";
    pDocument->pClass->postUnoCommand(pDocument, ".uno:ReplyComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a LOK_CALLBACK_COMMENT callback with comment 'Add' action and linked to its parent comment
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parent"));
    CPPUNIT_ASSERT_EQUAL(std::string("Reply comment again"), aView1.m_aCommentCallbackResult.get<std::string>("text"));
    CPPUNIT_ASSERT_EQUAL(std::string("Reply comment again"), aView2.m_aCommentCallbackResult.get<std::string>("text"));

    // .uno:ViewAnnotations returns total of 5 comments
    boost::property_tree::ptree aTree;
    char* pJSON = pDocument->m_pDocumentClass->getCommandValues(pDocument, ".uno:ViewAnnotations");
    std::stringstream aStream(pJSON);
    free(pJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), aTree.get_child("comments").size());

    comphelper::LibreOfficeKit::setActive(false);
}

void DesktopLOKTest::testRunMacro()
{
    LibLibreOffice_Impl aOffice;
    bool bGoodMacro, bNonExistentMacro;

    // Tools macros come pre-installed in system share/basic folder,
    bGoodMacro = aOffice.m_pOfficeClass->runMacro(&aOffice, OString("macro:///Tools.Debug.ActivateReadOnlyFlag()").getStr());
    CPPUNIT_ASSERT(bGoodMacro);

    bNonExistentMacro = aOffice.m_pOfficeClass->runMacro(&aOffice, OString("macro:///I.Am.Not(There)").getStr());
    CPPUNIT_ASSERT(!bNonExistentMacro);
}

void DesktopLOKTest::testExtractParameter()
{
    OUString aOptions("Language=de-DE");
    OUString aValue = extractParameter(aOptions, "Language");
    CPPUNIT_ASSERT_EQUAL(OUString("de-DE"), aValue);
    CPPUNIT_ASSERT_EQUAL(OUString(), aOptions);

    aOptions = "Language=en-US,Something";
    aValue = extractParameter(aOptions, "Language");
    CPPUNIT_ASSERT_EQUAL(OUString("en-US"), aValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Something"), aOptions);

    aOptions = "SomethingElse,Language=cs-CZ";
    aValue = extractParameter(aOptions, "Language");
    CPPUNIT_ASSERT_EQUAL(OUString("cs-CZ"), aValue);
    CPPUNIT_ASSERT_EQUAL(OUString("SomethingElse"), aOptions);

    aOptions = "Something1,Language=hu-HU,Something2";
    aValue = extractParameter(aOptions, "Language");
    CPPUNIT_ASSERT_EQUAL(OUString("hu-HU"), aValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Something1,Something2"), aOptions);

    aOptions = "Something1,Something2=blah,Something3";
    aValue = extractParameter(aOptions, "Language");
    CPPUNIT_ASSERT_EQUAL(OUString(), aValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Something1,Something2=blah,Something3"), aOptions);
}

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLOKTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
