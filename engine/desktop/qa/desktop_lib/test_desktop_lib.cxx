/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_crypto.h>
#include <config_oox.h>
#include <cstdlib>
#include <memory>
#include <string_view>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/window.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/math.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/kit/helper.hxx>
#include <test/unoapi_test.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertysequence.hxx>
#include <osl/conditn.hxx>
#include <svl/srchitem.hxx>
#include <COKit/COKit.hxx>
#include <unotools/tempfile.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <unotools/datetime.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <comphelper/string.hxx>
#include <comphelper/scopeguard.hxx>
#include <cairo.h>
#include <config_fonts.h>
#include <config_mpl.h>
#include <tools/json_writer.hxx>
#include <o3tl/unit_conversion.hxx>
#include <o3tl/string_view.hxx>

#include <lib/init.hxx>
#include <svx/svxids.hrc>

#include <cppunit/TestAssert.h>
#include <vcl/BitmapTools.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <svtools/colorcfg.hxx>
#include <sal/types.h>
#include <test/kitcallback.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace com::sun::star;
using namespace desktop;

static COKitDocumentType getDocumentTypeFromName(std::string_view name)
{
    CPPUNIT_ASSERT_MESSAGE("Document name must include extension.", name.size() > 4);

    const auto it = name.rfind('.');
    CPPUNIT_ASSERT_MESSAGE("Document name must include extension.", it != std::string::npos);
    const std::string_view ext = name.substr(it);

    if (ext == ".ods")
        return COKitDocumentType::SPREADSHEET;

    if (ext == ".odp")
        return COKitDocumentType::PRESENTATION;

    return COKitDocumentType::TEXT;
}

class DesktopKitTest : public UnoApiTest
{
public:
    DesktopKitTest() : UnoApiTest(u"/desktop/qa/data/"_ustr),
    m_nSelectionBeforeSearchResult(0),
    m_nSelectionAfterSearchResult(0),
    m_bModified(false),
    m_nTrackChanges(0)
    {
    }
    ~DesktopKitTest();

    void readFileIntoByteVector(
        std::u16string_view sFilename, std::vector<sal_uInt8> & rByteVector);

    virtual void setUp() override
    {
        comphelper::COKit::setActive(true);

        UnoApiTest::setUp();
    }

    virtual void tearDown() override
    {
        closeDoc();

        // documents are already closed, no need to call UnoApiTest::tearDown
        test::BootstrapFixture::tearDown();

        comphelper::COKit::setActive(false);
    }

    std::unique_ptr<COKitDocumentImpl>
    loadDocImpl(const char* pName, COKitDocumentType eType);

private:
    std::unique_ptr<COKitDocumentImpl>
    loadDocImpl(const char* pName);

public:
    std::unique_ptr<COKitDocumentImpl>
    loadDocUrlImpl(const OUString& rFileURL, COKitDocumentType eType,
                   const cpo::uno::Sequence<beans::PropertyValue>& rExtraArgs = {});

    COKitDocumentImpl* loadDocUrl(const OUString& rFileURL, COKitDocumentType eType);
    COKitDocumentImpl* loadDoc(const char* pName, COKitDocumentType eType);
    COKitDocumentImpl* loadDoc(const char* pName)
    {
        return loadDoc(pName, getDocumentTypeFromName(pName));
    }

    void closeDoc(std::unique_ptr<COKitDocumentImpl>& loDocument);
    void closeDoc() { closeDoc(m_pDocument); }
    static void callback(COKitCallbackType eType, const char* pPayload, void* pData);
    void callbackImpl(COKitCallbackType eType, const char* pPayload);

    void testGetStyles();
    void testGetFonts();
    void testCreateView();
    void testGetPartPageRectangles();
    void testSearchCalc();
    void testPropertySettingOnFormulaBar();
    void testSearchTermReset();
    void testWriterShapePosSizeDialog();
    void testFormulaBarAcceptButton();
    void testSearchAllNotificationsCalc();
    void testPaintTile();
    void testSaveAs();
    void testSaveFailedReportsReason();
    void testExportDirectToPdfDottedName();
    void testSaveAsJsonOptions();
    void testSaveAsCalc();
    void testPasteWriter();
    void testPasteWriterJPEG();
    void testClipboardMarkdownFlavor();
    void testUndoWriter();
    void testRowColumnHeaders();
    void testHiddenRowHeaders();
    void testCellCursor();
    void testCommandResult();
    void testWriterComments();
    void testCommentAuthorFromSession();
    void testCommentAuthorAnonymous();
    void testSheetOperations();
    void testSheetSelections();
    void testSheetDragDrop();
    void testContextMenuCalc();
    void testContextMenuWriter();
    void testNotificationCompression();
    void testVectorDeltaPushCoalescing();
    void testTileInvalidationCompression();
    void testPartInInvalidation();
    void testBinaryCallback();
    void testOmitInvalidate();
    void test2ViewsOmitInvalidate();
    void testPaintTileOmitInvalidate();
    void testCreateViewOmitInvalidate();
    void testInput();
    void testRedlineWriter();
    void testRedlineCalc();
    void testPaintPartTile();
    void testPaintPartTileHidesGridOnOtherPart();
    void testPaintPartTileHidesGridOnActivePartPreview();
    void testPaintPartTileDifferentSchemes();
#if HAVE_MORE_FONTS
    void testGetFontSubset();
#endif
    void testCommentsWriter();
    void testCommentsCalc();
    void testCommentsImpress();
    void testCommentsImpressCrossDocument();
    void testDocSizeChangedCrossDocument();
    void testViewSelectionCrossDocument();
    void testAuthorFieldUpdateCrossDocument();
    void testCommentsCallbacksWriter();
    void testCommentsAddEditDeleteDraw();
    void testCommentsInReadOnlyMode();
    void testRedlinesInReadOnlyMode();
    void testCalcValidityDropdown();
    void testCalcValidityDropdownInReadonlyMode();
    void testRunMacro();
    void testExtractParameter();
    void testGetSignatureState_NonSigned();
    void testGetSignatureState_Signed();
#if 0 // broken with system nss on RHEL 7
    void testInsertCertificate_DER_ODT();
    void testInsertCertificate_PEM_ODT();
    void testInsertCertificate_PEM_DOCX();
#endif
#if !MPL_HAVE_SUBSET
    void testSignDocument_PEM_PDF();
#endif
    void testTextSelectionHandles();
    void testComplexSelection();
    void testSpellcheckerMultiView();
    void testDialogPaste();
    void testCalcSaveAs();
    void testControlState();
    void testMetricField();
    void testMultiDocuments();
    void testJumpCursor();
    void testRenderSearchResult_WriterNode();
    void testRenderSearchResult_CommonNode();
    void testNoDuplicateTableSelection();
    void testMultiViewTableSelection();
    void testColorPaletteCallback();

    CPPUNIT_TEST_SUITE(DesktopKitTest);
    CPPUNIT_TEST(testGetStyles);
    CPPUNIT_TEST(testGetFonts);
    CPPUNIT_TEST(testCreateView);
    CPPUNIT_TEST(testGetPartPageRectangles);
    CPPUNIT_TEST(testSearchCalc);
    CPPUNIT_TEST(testPropertySettingOnFormulaBar);
    CPPUNIT_TEST(testSearchTermReset);
    CPPUNIT_TEST(testWriterShapePosSizeDialog);
    CPPUNIT_TEST(testFormulaBarAcceptButton);
    CPPUNIT_TEST(testSearchAllNotificationsCalc);
    CPPUNIT_TEST(testPaintTile);
    CPPUNIT_TEST(testSaveAs);
    CPPUNIT_TEST(testSaveFailedReportsReason);
    CPPUNIT_TEST(testExportDirectToPdfDottedName);
    CPPUNIT_TEST(testSaveAsJsonOptions);
    CPPUNIT_TEST(testSaveAsCalc);
    CPPUNIT_TEST(testPasteWriter);
    CPPUNIT_TEST(testPasteWriterJPEG);
    CPPUNIT_TEST(testClipboardMarkdownFlavor);
    CPPUNIT_TEST(testUndoWriter);
    CPPUNIT_TEST(testRowColumnHeaders);
    CPPUNIT_TEST(testHiddenRowHeaders);
    CPPUNIT_TEST(testCellCursor);
    CPPUNIT_TEST(testCommandResult);
    CPPUNIT_TEST(testWriterComments);
    CPPUNIT_TEST(testCommentAuthorFromSession);
    CPPUNIT_TEST(testCommentAuthorAnonymous);
    CPPUNIT_TEST(testSheetOperations);
    CPPUNIT_TEST(testSheetSelections);
    CPPUNIT_TEST(testSheetDragDrop);
    CPPUNIT_TEST(testContextMenuCalc);
    CPPUNIT_TEST(testContextMenuWriter);
    CPPUNIT_TEST(testNotificationCompression);
    CPPUNIT_TEST(testVectorDeltaPushCoalescing);
    CPPUNIT_TEST(testTileInvalidationCompression);
    CPPUNIT_TEST(testPartInInvalidation);
    CPPUNIT_TEST(testBinaryCallback);
    CPPUNIT_TEST(testOmitInvalidate);
    CPPUNIT_TEST(test2ViewsOmitInvalidate);
    CPPUNIT_TEST(testPaintTileOmitInvalidate);
    CPPUNIT_TEST(testCreateViewOmitInvalidate);
    CPPUNIT_TEST(testInput);
    CPPUNIT_TEST(testRedlineWriter);
    CPPUNIT_TEST(testRedlineCalc);
    CPPUNIT_TEST(testPaintPartTile);
    CPPUNIT_TEST(testPaintPartTileHidesGridOnOtherPart);
    CPPUNIT_TEST(testPaintPartTileHidesGridOnActivePartPreview);
    CPPUNIT_TEST(testPaintPartTileDifferentSchemes);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testGetFontSubset);
#endif
    CPPUNIT_TEST(testCommentsWriter);
    CPPUNIT_TEST(testCommentsCalc);
    CPPUNIT_TEST(testCommentsImpress);
    CPPUNIT_TEST(testCommentsImpressCrossDocument);
    CPPUNIT_TEST(testDocSizeChangedCrossDocument);
    CPPUNIT_TEST(testViewSelectionCrossDocument);
    CPPUNIT_TEST(testAuthorFieldUpdateCrossDocument);
    CPPUNIT_TEST(testCommentsCallbacksWriter);
    CPPUNIT_TEST(testCommentsAddEditDeleteDraw);
    CPPUNIT_TEST(testCommentsInReadOnlyMode);
    CPPUNIT_TEST(testRedlinesInReadOnlyMode);
    CPPUNIT_TEST(testCalcValidityDropdown);
    CPPUNIT_TEST(testCalcValidityDropdownInReadonlyMode);
    CPPUNIT_TEST(testRunMacro);
    CPPUNIT_TEST(testExtractParameter);
    CPPUNIT_TEST(testGetSignatureState_Signed);
    CPPUNIT_TEST(testGetSignatureState_NonSigned);
#if !MPL_HAVE_SUBSET
#if 0 // broken with system nss on RHEL 7
    CPPUNIT_TEST(testInsertCertificate_DER_ODT);
    CPPUNIT_TEST(testInsertCertificate_PEM_ODT);
    CPPUNIT_TEST(testInsertCertificate_PEM_DOCX);
#endif
    CPPUNIT_TEST(testSignDocument_PEM_PDF);
#endif
    CPPUNIT_TEST(testTextSelectionHandles);
    CPPUNIT_TEST(testComplexSelection);
    CPPUNIT_TEST(testSpellcheckerMultiView);
    CPPUNIT_TEST(testDialogPaste);
    CPPUNIT_TEST(testCalcSaveAs);
    CPPUNIT_TEST(testControlState);
    CPPUNIT_TEST(testMetricField);
    CPPUNIT_TEST(testMultiDocuments);
    CPPUNIT_TEST(testJumpCursor);
    CPPUNIT_TEST(testRenderSearchResult_WriterNode);
    CPPUNIT_TEST(testRenderSearchResult_CommonNode);
    CPPUNIT_TEST(testNoDuplicateTableSelection);
    CPPUNIT_TEST(testMultiViewTableSelection);
    CPPUNIT_TEST(testColorPaletteCallback);
    CPPUNIT_TEST_SUITE_END();

    OString m_aTextSelection;
    OString m_aTextSelectionStart;
    OString m_aTextSelectionEnd;
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

    std::unique_ptr<COKitDocumentImpl> m_pDocument;
};

DesktopKitTest::~DesktopKitTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

static Control* GetFocusControl(vcl::Window const * pParent)
{
    sal_uInt16 nChildren = pParent->GetChildCount();
    for (sal_uInt16 nChild = 0; nChild < nChildren; ++nChild)
    {
        vcl::Window* pChild = pParent->GetChild( nChild );
        Control* pCtrl = dynamic_cast<Control*>(pChild);
        if (pCtrl && pCtrl->HasControlFocus())
            return pCtrl;

        Control* pSubCtrl = GetFocusControl( pChild );
        if (pSubCtrl)
            return pSubCtrl;
    }
    return nullptr;
}

std::unique_ptr<COKitDocumentImpl>
DesktopKitTest::loadDocUrlImpl(const OUString& rFileURL, COKitDocumentType eType,
                               const cpo::uno::Sequence<beans::PropertyValue>& rExtraArgs)
{
    OUString aService;
    switch (eType)
    {
    case COKitDocumentType::TEXT:
        aService = u"com.sun.star.text.TextDocument"_ustr;
        break;
    case COKitDocumentType::SPREADSHEET:
        aService = u"com.sun.star.sheet.SpreadsheetDocument"_ustr;
        break;
    case COKitDocumentType::PRESENTATION:
        aService = u"com.sun.star.presentation.PresentationDocument"_ustr;
        break;
    default:
        CPPUNIT_ASSERT(false);
        break;
    }

    static int nDocumentIdCounter = 0;
    comphelper::COKit::setDocId(ViewShellDocId(nDocumentIdCounter));
    mxComponent = loadFromDesktop(rFileURL, aService, rExtraArgs);

    std::unique_ptr<COKitDocumentImpl> pDocument(new COKitDocumentImpl(mxComponent, nDocumentIdCounter));
    ++nDocumentIdCounter;

    return pDocument;
}

std::unique_ptr<COKitDocumentImpl>
DesktopKitTest::loadDocImpl(const char* pName, COKitDocumentType eType)
{
    OUString aFileURL = createFileURL(OUString::createFromAscii(pName));
    return loadDocUrlImpl(aFileURL, eType);
}

std::unique_ptr<COKitDocumentImpl>
DesktopKitTest::loadDocImpl(const char* pName)
{
    return loadDocImpl(pName, getDocumentTypeFromName(pName));
}

COKitDocumentImpl* DesktopKitTest::loadDocUrl(const OUString& rFileURL, COKitDocumentType eType)
{
    m_pDocument = loadDocUrlImpl(rFileURL, eType);
    return m_pDocument.get();
}

COKitDocumentImpl* DesktopKitTest::loadDoc(const char* pName, COKitDocumentType eType)
{
    m_pDocument = loadDocImpl(pName, eType);
    return m_pDocument.get();
}

void DesktopKitTest::closeDoc(std::unique_ptr<COKitDocumentImpl>& pDocument)
{
    if (pDocument)
    {
        pDocument->registerCallback(nullptr, nullptr);
        pDocument.reset();
    }

    if (mxComponent.is())
    {
        css::uno::Reference<util::XCloseable> xCloseable(mxComponent, css::uno::UNO_QUERY_THROW);
        xCloseable->close(false);
        mxComponent.clear();
    }
}

void DesktopKitTest::callback(COKitCallbackType eType, const char* pPayload, void* pData)
{
    static_cast<DesktopKitTest*>(pData)->callbackImpl(eType, pPayload);
}

void DesktopKitTest::callbackImpl(COKitCallbackType eType, const char* pPayload)
{
    switch (eType)
    {
    case COKitCallbackType::TEXT_SELECTION:
    {
        m_aTextSelection = OString(pPayload);
        if (m_aSearchResultSelection.empty())
            ++m_nSelectionBeforeSearchResult;
        else
            ++m_nSelectionAfterSearchResult;
    }
    break;
    case COKitCallbackType::TEXT_SELECTION_START:
        m_aTextSelectionStart = OString(pPayload);
    break;
    case COKitCallbackType::TEXT_SELECTION_END:
        m_aTextSelectionEnd = OString(pPayload);
    break;
    case COKitCallbackType::SEARCH_RESULT_SELECTION:
    {
        m_aSearchResultSelection.clear();
        boost::property_tree::ptree aTree;
        std::stringstream aStream(pPayload);
        boost::property_tree::read_json(aStream, aTree);
        for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("searchResultSelection"))
        {
            m_aSearchResultSelection.emplace_back(rValue.second.get<std::string>("rectangles").c_str());
            m_aSearchResultPart.push_back(std::atoi(rValue.second.get<std::string>("part").c_str()));
        }
    }
    break;
    case COKitCallbackType::UNO_COMMAND_RESULT:
    {
        m_aCommandResult = OString(pPayload);
        m_aCommandResultCondition.set();
    }
    break;
    case COKitCallbackType::STATE_CHANGED:
    {
        OString aPayload(pPayload);
        OString aPrefix(".uno:ModifiedStatus="_ostr);
        if (aPayload.startsWith(aPrefix))
        {
            m_bModified = aPayload.copy(aPrefix.getLength()).toBoolean();
            m_aStateChangedCondition.set();
        }
        else if (aPayload.startsWith(".uno:TrackChanges=") && aPayload.endsWith("=true"))
            ++m_nTrackChanges;
    }
    break;
    case COKitCallbackType::CONTEXT_MENU:
    {
        m_aContextMenuResult.clear();
        std::stringstream aStream(pPayload);
        boost::property_tree::read_json(aStream, m_aContextMenuResult);
        m_aContextMenuCondition.set();
    }
    break;
    default:
    break;
    }
}

void DesktopKitTest::testGetStyles()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:StyleApply");
    std::stringstream aStream(aJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( !aTree.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:StyleApply"), aTree.get_child("commandName").get_value<std::string>()  );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( !aValues.empty() );
    for (const auto& rPair : aValues)
    {
        if( rPair.first != "ClearStyle")
        {
            CPPUNIT_ASSERT( !rPair.second.empty());
        }
        if (rPair.first != "CharacterStyles" &&
            rPair.first != "ParagraphStyles" &&
            rPair.first != "FrameStyles" &&
            rPair.first != "PageStyles" &&
            rPair.first != "NumberingStyles" &&
            rPair.first != "CellStyles" &&
            rPair.first != "ShapeStyles" &&
            rPair.first != "TableStyles" &&
            rPair.first != "HeaderFooter" &&
            rPair.first != "Commands")
        {
            CPPUNIT_FAIL("Unknown style family: " + rPair.first);
        }
    }
}

void DesktopKitTest::testGetFonts()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_presentation.odp");
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:CharFontName");
    std::stringstream aStream(aJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( !aTree.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:CharFontName"), aTree.get_child("commandName").get_value<std::string>() );

    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( !aValues.empty() );
    for (const auto& rPair : aValues)
    {
        // check that we have font sizes available for each font
        CPPUNIT_ASSERT( !rPair.second.empty());
    }
}

void DesktopKitTest::testCreateView()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT_EQUAL(1, pDocument->getViewsCount());

    int nId0 = pDocument->getView();
    int nId1 = pDocument->createView();
    CPPUNIT_ASSERT_EQUAL(2, pDocument->getViewsCount());

    // Test getViewIds().
    std::vector<int> aViewIds(2);
    CPPUNIT_ASSERT(pDocument->getViewIds(aViewIds.data(), aViewIds.size()));
    // The expectation is that the most recently used shell is at the start
    CPPUNIT_ASSERT_EQUAL(nId1, aViewIds[0]);
    CPPUNIT_ASSERT_EQUAL(nId0, aViewIds[1]);

    // Make sure the created view is the active one, then switch to the old
    // one.
    CPPUNIT_ASSERT_EQUAL(nId1, pDocument->getView());
    pDocument->setView(nId0);
    CPPUNIT_ASSERT_EQUAL(nId0, pDocument->getView());

    pDocument->destroyView(nId1);
    CPPUNIT_ASSERT_EQUAL(1, pDocument->getViewsCount());
}

void DesktopKitTest::testGetPartPageRectangles()
{
    // Test that we get as many page rectangles as expected: blank document is
    // one page.
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    std::string pRectangles = pDocument->getWriterPageRectangles();
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
}

void DesktopKitTest::testSearchCalc()
{
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("search.ods");
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", cpo::uno::Any(u"foo"_ustr)},
        {"SearchItem.Backward", cpo::uno::Any(false)},
        {"SearchItem.Command", cpo::uno::Any(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);

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
}

void DesktopKitTest::testSearchAllNotificationsCalc()
{
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("search.ods");
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"SearchItem.SearchString", cpo::uno::Any(u"foo"_ustr)},
        {"SearchItem.Backward", cpo::uno::Any(false)},
        {"SearchItem.Command", cpo::uno::Any(static_cast<sal_uInt16>(SvxSearchCmd::FIND_ALL))},
    }));
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);

    // This was 1, make sure that we get no notifications about selection changes during search.
    CPPUNIT_ASSERT_EQUAL(0, m_nSelectionBeforeSearchResult);
    // But we do get the selection afterwards.
    CPPUNIT_ASSERT(m_nSelectionAfterSearchResult > 0);
}

void DesktopKitTest::testPaintTile()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
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
    pDocument->paintTile(aBuffer.data(), nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    // This crashed in OutputDevice::DrawDeviceAlphaBitmap().
    nCanvasWidth = 200;
    nCanvasHeight = 200;
    nTileWidth = 4000;
    nTileHeight = 4000;
    aBuffer.resize(nCanvasWidth * nCanvasHeight * 4);
    pDocument->paintTile(aBuffer.data(), nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

void DesktopKitTest::testSaveAs()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "png", nullptr));
}

void DesktopKitTest::testSaveFailedReportsReason()
{
    // Load the document editable, then drop the medium to read-only while leaving the document
    // itself editable. In the kit a .uno:Save against a read-only medium fails with an
    // input/output error and no dialog, which stands in for any store that fails while the
    // document is loaded.
    m_pDocument = loadDocUrlImpl(createFileURL(u"blank_text.odt"), COKitDocumentType::TEXT);
    COKitDocumentImpl* pDocument = m_pDocument.get();
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(pDocument->mxComponent);
    CPPUNIT_ASSERT(pShell);
    pShell->SetReadOnly();

    TimeValue aTimeValue = { 5, 0 };
    m_aCommandResultCondition.reset();
    // A save argument makes the dispatch report the store's own true/false result rather than
    // masking a failed save as done, matching how the kit posts a save with its own arguments.
    pDocument->postUnoCommand(".uno:Save",
                                      "{\"NoFileSync\":{\"type\":\"boolean\",\"value\":false}}", true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);

    boost::property_tree::ptree aTree;
    std::stringstream aStream((std::string(m_aCommandResult)));
    boost::property_tree::read_json(aStream, aTree);

    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Save"),
                         aTree.get_child("commandName").get_value<std::string>());
    // The save failed, so the reason rides the result as a string rather than the void a failure
    // would otherwise leave. coolwsd turns that string into the dialog's technical detail.
    CPPUNIT_ASSERT_EQUAL(false, aTree.get_child("success").get_value<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("string"),
                         aTree.get_child("result.type").get_value<std::string>());
    CPPUNIT_ASSERT(!aTree.get_child("result.value").get_value<std::string>().empty());
}

void DesktopKitTest::testExportDirectToPdfDottedName()
{
    // Regression test: a document whose base name contains a dot (here a
    // "26.04"-style version number) must keep its full name when exported to
    // PDF. The COKit export path used to re-apply the extension with
    // INetURLObject::SetExtension(), which regards everything after the last
    // dot as the extension. For "CODE 26.04 Release Blog" that overwrote
    // " Release Blog" after the dot in "26.04", suggesting "CODE 26.pdf".

    // Host a document under a temp directory using the problematic name.
    const OUString aTempDir = utl::CreateTempURL(nullptr, /*bDirectory*/ true);
    INetURLObject aDocObj(aTempDir);
    aDocObj.insertName(u"CODE 26.04 Release Blog.odt", false, INetURLObject::LAST_SEGMENT,
                       INetURLObject::EncodeMechanism::All);
    const OUString aDocUrl = aDocObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                         osl::File::copy(createFileURL(u"blank_text.odt"), aDocUrl));

    // Capture the URI the engine suggests to the file-save dialog callback,
    // then abort the actual save by handing back an empty result.
    OUString aSuggestedUri;
    comphelper::COKit::setFileSaveDialogCallback(
        [&aSuggestedUri](const char* pSuggestedUri, char* pResult, size_t nResultLen)
        {
            aSuggestedUri = OUString::fromUtf8(pSuggestedUri);
            if (nResultLen)
                pResult[0] = '\0';
        });
    comphelper::ScopeGuard aResetCallback(
        []() { comphelper::COKit::setFileSaveDialogCallback({}); });

    COKitDocumentImpl* pDocument = loadDocUrl(aDocUrl, COKitDocumentType::TEXT);
    pDocument->postUnoCommand(".uno:ExportDirectToPDF", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE("file-save dialog callback was not invoked", !aSuggestedUri.isEmpty());
    const INetURLObject aSuggested(aSuggestedUri);
    CPPUNIT_ASSERT_EQUAL(u"CODE 26.04 Release Blog.pdf"_ustr,
                         aSuggested.GetLastName(INetURLObject::DecodeMechanism::WithCharset));

    osl::File::remove(aDocUrl);
    osl::Directory::remove(aTempDir);
}

void DesktopKitTest::testSaveAsJsonOptions()
{
    // Given a document with 3 pages:
    COKitDocumentImpl* pDocument = loadDoc("3page.odg");

    // When exporting that document to PDF, skipping the first page:
    OString aOptions("{\"PageRange\":{\"type\":\"string\",\"value\":\"2-\"}}"_ostr);
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "pdf", aOptions.getStr()));

    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    // Then make sure the resulting PDF has 2 pages:
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = parsePDFExport();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. FilterOptions was ignored.
    CPPUNIT_ASSERT_EQUAL(2, pPdfDocument->getPageCount());
}

void DesktopKitTest::testSaveAsCalc()
{
    COKitDocumentImpl* pDocument = loadDoc("search.ods");
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "png", nullptr));
}

void DesktopKitTest::testPasteWriter()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    OString aText("hello"_ostr);

    CPPUNIT_ASSERT(pDocument->paste("text/plain;charset=utf-8", aText.getStr(), aText.getLength()));

    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    std::string aSelectedText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL("hello"_ostr, OString(aSelectedText));

    // textt/plain should be rejected.
    CPPUNIT_ASSERT(!pDocument->paste("textt/plain;charset=utf-8", aText.getStr(), aText.getLength()));
    // Writer is expected to support text/html.
    CPPUNIT_ASSERT(pDocument->paste("text/html", aText.getStr(), aText.getLength()));

    // Overwrite doc contents with a HTML paste.
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    OString aComment("foo <!-- bar --> baz"_ostr);
    CPPUNIT_ASSERT(pDocument->paste("text/html", aComment.getStr(), aComment.getLength()));

    // Check if we have a comment.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnumeration = xParagraphEnumerationAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphEnumeration->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextPortionEnumeration = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xTextPortionEnumeration->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, xTextPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>());
    // Without the accompanying fix in place, this test would have failed, as we had a comment
    // between "foo" and "baz".
    CPPUNIT_ASSERT(!xTextPortionEnumeration->hasMoreElements());

    // Overwrite the doc contents with an explicitly plain text paste.
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    OString aPlainText("foo _bar_ baz"_ostr);
    CPPUNIT_ASSERT(pDocument->paste("text/plain", aPlainText.getStr(),
                                            aPlainText.getLength()));

    // Check if '_bar_' was pasted as-is.
    xParagraphEnumeration = xParagraphEnumerationAccess->createEnumeration();
    xParagraph.set(xParagraphEnumeration->nextElement(), uno::UNO_QUERY);
    xTextPortionEnumeration = xParagraph->createEnumeration();
    uno::Reference<text::XTextRange> xTextPortionRange(xTextPortionEnumeration->nextElement(),
                                                       uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: foo _bar_ baz
    // - Actual  : foo
    // i.e. the text/plain input was parsed as markdown, while that should not happen when
    // specifying the text/plain mimetype explicitly (and not text/markdown).
    CPPUNIT_ASSERT_EQUAL(u"foo _bar_ baz"_ustr, xTextPortionRange->getString());
}

void DesktopKitTest::testPasteWriterJPEG()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    OUString aFileURL = createFileURL(u"paste.jpg");
    SvFileStream aImageStream(aFileURL, StreamMode::READ);
    std::vector<char> aImageContents(aImageStream.remainingSize());
    aImageStream.ReadBytes(aImageContents.data(), aImageContents.size());

    CPPUNIT_ASSERT(pDocument->paste("image/jpeg", aImageContents.data(), aImageContents.size()));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 0, JPEG was not handled as a format for clipboard paste.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // This was text::TextContentAnchorType_AT_PARAGRAPH.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, xShape->getPropertyValue(u"AnchorType"_ustr).get<text::TextContentAnchorType>());

    // Delete the pasted picture, and paste again with a custom anchor type.
    uno::Reference<lang::XComponent>(xShape, uno::UNO_QUERY_THROW)->dispose();
    cpo::uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AnchorType", cpo::uno::Any(static_cast<sal_uInt16>(text::TextContentAnchorType_AT_CHARACTER))},
    }));
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, aPropertyValues);
    xShape.set(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // This was text::TextContentAnchorType_AS_CHARACTER, AnchorType argument was ignored.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, xShape->getPropertyValue(u"AnchorType"_ustr).get<text::TextContentAnchorType>());
}

void DesktopKitTest::testClipboardMarkdownFlavor()
{
    // Given text/plain and markdown data on the kit clipboard:
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    OString aText("foo _bar_ baz"_ostr);
    const char* pInMimeTypes[] = { "text/plain;charset=utf-8" };
    const size_t pInSizes[] = { static_cast<size_t>(aText.getLength()) };
    const char* pInStreams[] = { aText.getStr() };
    CPPUNIT_ASSERT(pDocument->setClipboard(1, pInMimeTypes, pInSizes, pInStreams));

    // When getting the clipboard content:
    size_t nOutCount = 0;
    char** pOutMimeTypes = nullptr;
    size_t* pOutSizes = nullptr;
    char** pOutStreams = nullptr;
    CPPUNIT_ASSERT(pDocument->getClipboard(nullptr, &nOutCount, &pOutMimeTypes, &pOutSizes,
                                           &pOutStreams));

    // Then make sure the plain text data is also advertised as markdown:
    bool bHasPlain = false;
    bool bHasMarkdown = false;
    OString aMarkdownContent;
    for (size_t i = 0; i < nOutCount; ++i)
    {
        OString aMime(pOutMimeTypes[i]);
        if (aMime == "text/plain;charset=utf-8")
            bHasPlain = true;
        else if (aMime == "text/markdown")
        {
            bHasMarkdown = true;
            aMarkdownContent = OString(pOutStreams[i], pOutSizes[i]);
        }
        free(pOutMimeTypes[i]);
        free(pOutStreams[i]);
    }
    free(pOutMimeTypes);
    free(pOutStreams);
    free(pOutSizes);
    CPPUNIT_ASSERT(bHasPlain);
    // Without the accompanying fix in place, this test would have failed, there was no markdown
    // advertised when listing available formats.
    CPPUNIT_ASSERT(bHasMarkdown);
    CPPUNIT_ASSERT_EQUAL(aText, aMarkdownContent);
}

void DesktopKitTest::testUndoWriter()
{
    // Load a Writer document and press a key.
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 't', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 't', 0);
    Scheduler::ProcessEventsToIdle();
    // Get undo info.
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:Undo");
    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one undo action.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("actions").size());
}

void DesktopKitTest::testRowColumnHeaders()
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
    COKitDocumentImpl* pDocument = loadDoc("search.ods");

    pDocument->initializeForRendering(nullptr);

    long nWidth = 0;
    long nHeight = 0;
    pDocument->getDocumentSize(&nWidth, &nHeight);
    tools::Long nX = rtl::math::round(nWidth / 4.0);
    tools::Long nY = rtl::math::round(nHeight / 4.0);
    nWidth = rtl::math::round(nWidth / 2.0);
    nHeight = rtl::math::round(nHeight / 2.0);

    std::stringstream aPayload;
    aPayload << ".uno:ViewRowColumnHeaders?x=" << nX << "&y=" << nY << "&width=" << nWidth << "&height=" << nHeight;

    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(aPayload.str().c_str());
    std::stringstream aStream(aJSON);

    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    bool bFirstHeader = true;
    bool bNotEnoughHeaders = true;
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = o3tl::toInt32(rValue.second.get<std::string>("size"));
        nSize = o3tl::convert(nSize, o3tl::Length::px, o3tl::Length::twip);
        OString aText(rValue.second.get<std::string>("text"));

        if (bFirstHeader)
        {
            CPPUNIT_ASSERT(nSize <= nY);
            CPPUNIT_ASSERT_EQUAL("10"_ostr, aText);
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
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("columns"))
    {
        sal_Int32 nSize = o3tl::toInt32(rValue.second.get<std::string>("size"));
        nSize = o3tl::convert(nSize, o3tl::Length::px, o3tl::Length::twip);
        OString aText(rValue.second.get<std::string>("text"));
        if (bFirstHeader)
        {
            CPPUNIT_ASSERT(nSize <= nX);
            CPPUNIT_ASSERT_EQUAL("3"_ostr, aText);
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

void DesktopKitTest::testHiddenRowHeaders()
{
    COKitDocumentImpl* pDocument = loadDoc("hidden-row.ods");

    pDocument->initializeForRendering(nullptr);

    tools::Long const nX = 0;
    tools::Long const nY = 0;
    long nWidth = 0;
    long nHeight = 0;
    pDocument->getDocumentSize(&nWidth, &nHeight);

    std::stringstream aPayload;
    aPayload << ".uno:ViewRowColumnHeaders?x=" << nX << "&y=" << nY << "&width=" << nWidth << "&height=" << nHeight;

    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(aPayload.str().c_str());
    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);
    sal_Int32 nPrevious = 0;
    sal_Int32 nIndex = 0;
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("rows"))
    {
        sal_Int32 nSize = o3tl::toInt32(rValue.second.get<std::string>("size"));

        if (nIndex++ == 2)
        {
            // nSize was 510, nPrevious was 255, i.e. hidden row wasn't reported as 0 height.
            CPPUNIT_ASSERT_EQUAL(nPrevious, nSize);
            break;
        }
        nPrevious = nSize;
    }
}

void DesktopKitTest::testCellCursor()
{
    COKitDocumentImpl* pDocument = loadDoc("search.ods");

    boost::property_tree::ptree aTree;

    std::string aJSON = pDocument->getCommandValues(".uno:CellCursor?tileWidth=1&tileHeight=1&outputWidth=1&outputHeight=1");

    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());

    boost::property_tree::read_json(aStream, aTree);

    OString aRectangle(aTree.get<std::string>("commandValues"));
    // cell cursor geometry + col + row
    CPPUNIT_ASSERT_EQUAL("0, 0, 1274, 254, 0, 0"_ostr, aRectangle);
}

void DesktopKitTest::testCommandResult()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    // the postUnoCommand() is supposed to be async, let's test it safely
    // [no idea if it is async in reality - most probably we are operating
    // under some solar mutex or something anyway ;-) - but...]
    TimeValue aTimeValue = { 2 , 0 }; // 2 seconds max

    // nothing is triggered when we have no callback yet, we just time out on
    // the condition var.
    m_aCommandResultCondition.reset();
    pDocument->postUnoCommand(".uno:Bold", nullptr, true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);

    CPPUNIT_ASSERT(m_aCommandResult.isEmpty());

    // but we get some real values when the callback is set up
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    m_aCommandResultCondition.reset();
    pDocument->postUnoCommand(".uno:Bold", nullptr, true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);

    boost::property_tree::ptree aTree;
    std::stringstream aStream((std::string(m_aCommandResult)));
    boost::property_tree::read_json(aStream, aTree);

    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Bold"), aTree.get_child("commandName").get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(true, aTree.get_child("success").get_value<bool>());
}

void DesktopKitTest::testWriterComments()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->registerCallback(&DesktopKitTest::callback, this);
    uno::Reference<awt::XReschedule> xToolkit = css::awt::Toolkit::create(comphelper::getProcessComponentContext());

    // Insert a comment at the beginning of the document and wait till the main
    // loop grabs the focus, so characters end up in the annotation window.
    TimeValue const aTimeValue = {2 , 0}; // 2 seconds max
    m_aCommandResultCondition.reset();
    pDocument->postUnoCommand(".uno:InsertAnnotation", R"({"Author":{"type":"string","value":"LocalUser#0"},"Html":{"type":"string","value":"test"}})", true);
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
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, xTextPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>());

    // Test that the typed characters ended up in the right window.
    auto xTextField = xTextPortion->getPropertyValue(u"TextField"_ustr).get< uno::Reference<beans::XPropertySet> >();
    // This was empty, typed characters ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, xTextField->getPropertyValue(u"Content"_ustr).get<OUString>());
}

void DesktopKitTest::testCommentAuthorFromSession()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    // The server provides "Jane Doe" as the author for this view.
    pDocument->initializeForRendering(R"({".uno:Author":{"type":"string","value":"Jane Doe"}})");
    pDocument->registerCallback(&DesktopKitTest::callback, this);
    uno::Reference<awt::XReschedule> xToolkit = css::awt::Toolkit::create(comphelper::getProcessComponentContext());

    TimeValue const aTimeValue = {2 , 0}; // 2 seconds max
    m_aCommandResultCondition.reset();
    // The InsertAnnotation command carries a different author than the session.
    pDocument->postUnoCommand(".uno:InsertAnnotation",
        R"({"Author":{"type":"string","value":"Other User"},"Html":{"type":"string","value":"test"}})", true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);
    CPPUNIT_ASSERT(!m_aCommandResult.isEmpty());
    xToolkit->reschedule();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnumeration = xParagraphEnumerationAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphEnumeration->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextPortionEnumeration = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xTextPortionEnumeration->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, xTextPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>());
    auto xTextField = xTextPortion->getPropertyValue(u"TextField"_ustr).get< uno::Reference<beans::XPropertySet> >();
    // The comment is attributed to the server-provided session identity, not the name the command carried.
    CPPUNIT_ASSERT_EQUAL(u"Jane Doe"_ustr, xTextField->getPropertyValue(u"Author"_ustr).get<OUString>());
}

void DesktopKitTest::testCommentAuthorAnonymous()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    // An anonymous session sends no .uno:Author, so the view has no server-provided author.
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);
    uno::Reference<awt::XReschedule> xToolkit = css::awt::Toolkit::create(comphelper::getProcessComponentContext());

    TimeValue const aTimeValue = {2 , 0}; // 2 seconds max
    m_aCommandResultCondition.reset();
    pDocument->postUnoCommand(".uno:InsertAnnotation",
        R"({"Author":{"type":"string","value":"Other User"},"Html":{"type":"string","value":"test"}})", true);
    Scheduler::ProcessEventsToIdle();
    m_aCommandResultCondition.wait(aTimeValue);
    CPPUNIT_ASSERT(!m_aCommandResult.isEmpty());
    xToolkit->reschedule();

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnumeration = xParagraphEnumerationAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphEnumeration->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextPortionEnumeration = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xTextPortionEnumeration->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, xTextPortion->getPropertyValue(u"TextPortionType"_ustr).get<OUString>());
    auto xTextField = xTextPortion->getPropertyValue(u"TextField"_ustr).get< uno::Reference<beans::XPropertySet> >();
    // With no server-provided identity the comment gets a blank author: not the
    // name the command carried, and not the shared module author either.
    CPPUNIT_ASSERT_EQUAL(OUString(), xTextField->getPropertyValue(u"Author"_ustr).get<OUString>());
}

void DesktopKitTest::testSheetOperations()
{
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods");

    // insert the last sheet
    pDocument->postUnoCommand(".uno:Insert",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"LastSheet\" }, \"Index\": { \"type\": \"long\", \"value\": 0 } }", false);

    // insert the first sheet
    pDocument->postUnoCommand(".uno:Insert",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"FirstSheet\" }, \"Index\": { \"type\": \"long\", \"value\": 1 } }", false);

    // rename the \"Sheet1\" (2nd now) to \"Renamed\"
    pDocument->postUnoCommand(".uno:Name",
          "{ \"Name\": { \"type\": \"string\", \"value\": \"Renamed\" }, \"Index\": { \"type\": \"long\", \"value\": 2 } }", false);

    // delete the \"Sheet2\" (3rd)
    pDocument->postUnoCommand(".uno:Remove",
          "{ \"Index\": { \"type\": \"long\", \"value\": 3 } }", false);

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(6, pDocument->getParts());

    std::vector<OString> aExpected = { "FirstSheet"_ostr, "Renamed"_ostr, "Sheet3"_ostr, "Sheet4"_ostr, "Sheet5"_ostr, "LastSheet"_ostr };
    for (int i = 0; i < 6; ++i)
    {
        char* pPartName = pDocument->getPartName(i);
        CPPUNIT_ASSERT_EQUAL(aExpected[i], OString(pPartName));
        free(pPartName);
    }
}

void DesktopKitTest::testSheetSelections()
{
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods", COKitDocumentType::SPREADSHEET);
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    /*
     * Check if selection data is correct
     */
    // Values in twips
    int row5 = 1150;
    int col1 = 1100;
    int const col2 = 2200;
    int const col3 = 3300;
    int col4 = 4400;
    int col5 = 5500;

    // Select row 5 from column 1 through column 5
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      col1, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col2, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col3, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col4, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col5, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP,
                                      col5, row5,
                                      1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Copy the contents and check if matches expected data
    {
        std::string aUsedMimeType;
        std::string aCopiedContent = pDocument->getTextSelection("", &aUsedMimeType);
        std::vector<long> aExpected = {5, 6, 7, 8, 9};
        std::istringstream iss(aCopiedContent);
        for (const long nIndex : aExpected)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(nIndex, strtol(token.c_str(), nullptr, 10));
        }
    }

    /*
     * Check if clicking inside the selection deselects the whole selection
     */

    // Click at row5, col4
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      col4, row5,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP,
                                      col4, row5,
                                      1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Selected text should get deselected and copying should give us
    // content of only one cell, now
    {
        std::string aCopiedContent = pDocument->getTextSelection("", nullptr);
        std::vector<long> aExpected = { 8 };
        std::istringstream iss(aCopiedContent);
        for (const long nIndex : aExpected)
        {
            std::string token;
            iss >> token;
            CPPUNIT_ASSERT_EQUAL(nIndex, strtol(token.c_str(), nullptr, 10));
        }
    }
}

void DesktopKitTest::testSheetDragDrop()
{
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods", COKitDocumentType::SPREADSHEET);
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    int row01 = 100;
    int col01 = 1100;
    int col02 = 2200;
    int col03 = 3300;
    int col05 = 5500;
    int col07 = 5700;

    // Select row 01 from column 01 through column 05
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      col01, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col02, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col05, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP,
                                      col05, row01,
                                      1, 1, 0);

    Scheduler::ProcessEventsToIdle();
    {
        SfxViewShell* pViewShell = SfxViewShell::Current();
        SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();

        OUString sValue;
        cpo::uno::Any aValue;
        css::util::URL aURL;
        std::unique_ptr<SfxPoolItem> pState;

        aURL.Protocol = u".uno:"_ustr;
        aURL.Complete = u".uno:Address"_ustr;
        aURL.Path = u"Address"_ustr;
        aURL.Main = u".uno:Address"_ustr;

        rViewFrame.GetBindings().QueryState(rViewFrame.GetBindings().QuerySlotId(aURL), pState);
        pState->QueryValue(aValue);
        aValue >>= sValue;
        CPPUNIT_ASSERT_EQUAL(u"Sheet5.A1:E1"_ustr, sValue);
    }

    // Check selection content
    {
        std::string aSelContent = pDocument->getTextSelection("", nullptr);
        std::vector<long> aExpected = {1, 2, 3, 4, 5};
        std::istringstream aContent(aSelContent);
        std::string token;
        for (const long nIndex : aExpected)
        {
            aContent >> token;
            CPPUNIT_ASSERT_EQUAL(nIndex, strtol(token.c_str(), nullptr, 10));
        }
    }

    // drag and drop
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      col01, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col02, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::MOVE,
                                      col03, row01,
                                      1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP,
                                      col07, row01,
                                      1, 1, 0);

    Scheduler::ProcessEventsToIdle();
    {
        SfxViewShell* pViewShell = SfxViewShell::Current();
        SfxViewFrame& rViewFrame = pViewShell->GetViewFrame();

        OUString sValue;
        cpo::uno::Any aValue;
        css::util::URL aURL;
        std::unique_ptr<SfxPoolItem> pState;

        aURL.Protocol = u".uno:"_ustr;
        aURL.Complete = u".uno:Address"_ustr;
        aURL.Path = u"Address"_ustr;
        aURL.Main = u".uno:Address"_ustr;

        rViewFrame.GetBindings().QueryState(rViewFrame.GetBindings().QuerySlotId(aURL), pState);
        pState->QueryValue(aValue);
        aValue >>= sValue;
        CPPUNIT_ASSERT_EQUAL(u"Sheet5.D1:H1"_ustr, sValue);
    }

    // Check selection content
    {
        std::string aSelContent = pDocument->getTextSelection("", nullptr);
        std::vector<long> aExpected = {1, 2, 3, 4, 5};
        std::istringstream aContent(aSelContent);
        std::string token;
        for (const long nIndex : aExpected)
        {
            aContent >> token;
            CPPUNIT_ASSERT_EQUAL(nIndex, strtol(token.c_str(), nullptr, 10));
        }
    }
}

namespace {

    void verifyContextMenuStructure(boost::property_tree::ptree& aRoot)
    {
        for (const auto& aItemPair: aRoot)
        {
            // This is an array, so no key
            CPPUNIT_ASSERT_EQUAL(aItemPair.first, std::string(""));

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
                CPPUNIT_ASSERT( !aText );
                CPPUNIT_ASSERT( !aCommand );
                CPPUNIT_ASSERT( !aSubmenu );
                CPPUNIT_ASSERT( !aEnabled );
                CPPUNIT_ASSERT( !aChecktype );
                CPPUNIT_ASSERT( !aChecked );
            }
            else if ( aType.get().data() == "command" )
            {
                CPPUNIT_ASSERT( aCommand );
                CPPUNIT_ASSERT( aText );
            }
            else if ( aType.get().data() == "menu")
            {
                CPPUNIT_ASSERT( aSubmenu );
                CPPUNIT_ASSERT( aText );
                verifyContextMenuStructure( aSubmenu.get() );
            }

            if ( aChecktype )
            {
                CPPUNIT_ASSERT( aChecktype.get().data() == "radio" ||
                                aChecktype.get().data() == "checkmark" ||
                                aChecktype.get().data() == "auto" );

                CPPUNIT_ASSERT( aChecked );
                CPPUNIT_ASSERT( aChecked.get().data() == "true" || aChecked.get().data() == "false" );
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

void DesktopKitTest::testContextMenuCalc()
{
    COKitDocumentImpl* pDocument = loadDoc("sheet_with_image.ods", COKitDocumentType::SPREADSHEET);
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    // Values in twips
    Point aPointOnImage(1150, 1100);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      aPointOnImage.X(), aPointOnImage.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue const aTimeValue = {2 , 0}; // 2 seconds max
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
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("true"));
    }

    // Copy is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Copy");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("true"));
    }

    // Paste is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Paste");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("true"));
    }

    // Remove hyperlink is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:RemoveHyperlink");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("false"));
    }

    // open hyperlink is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:OpenHyperlinkOnCursor");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("false"));
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
        CPPUNIT_ASSERT_EQUAL(aChecktypeToPage.get().data(), std::string("radio"));

        boost::optional<boost::property_tree::ptree&> aChecktypeToCell = aMenuItemToCell.get().get_child_optional("checktype");
        CPPUNIT_ASSERT(aChecktypeToCell);
        CPPUNIT_ASSERT_EQUAL(aChecktypeToCell.get().data(), std::string("radio"));

        // ToPage is checked
        boost::optional<boost::property_tree::ptree&> aCheckedToPage = aMenuItemToPage.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedToPage);
        CPPUNIT_ASSERT_EQUAL(aCheckedToPage.get().data(), std::string("true"));

        // ToCell is unchecked
        boost::optional<boost::property_tree::ptree&> aCheckedToCell = aMenuItemToCell.get().get_child_optional("checked");
        CPPUNIT_ASSERT(aCheckedToCell);
        CPPUNIT_ASSERT_EQUAL(aCheckedToCell.get().data(), std::string("false"));
    }
}

void DesktopKitTest::testContextMenuWriter()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    Point aRandomPoint(1150, 1100);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN,
                                      aRandomPoint.X(), aRandomPoint.Y(),
                                      1, 4, 0);
    Scheduler::ProcessEventsToIdle();

    TimeValue const aTimeValue = {2 , 0}; // 2 seconds max
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
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("false"));
    }

    // Copy is disabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Copy");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("false"));
    }

    // Paste is enabled
    {
        boost::optional<boost::property_tree::ptree> aMenuItem = getContextMenuItem(aMenu.get(), ".uno:Paste");
        CPPUNIT_ASSERT(aMenuItem);

        boost::optional<boost::property_tree::ptree&> aEnabled = aMenuItem.get().get_child_optional("enabled");
        CPPUNIT_ASSERT(aEnabled);
        CPPUNIT_ASSERT_EQUAL(aEnabled.get().data(), std::string("true"));
    }
}

static void callbackCompressionTest(COKitCallbackType eType, const char* payload, void* data)
{
    std::vector<std::tuple<int, std::string>>* notifs = static_cast<std::vector<std::tuple<int, std::string>>*>(data);
    notifs->emplace_back(static_cast<int>(eType), std::string(payload ? payload : "(nil)"));
}

void DesktopKitTest::testNotificationCompression()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    std::vector<std::tuple<int, std::string>> notifs;
    std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
    handler->setViewId(KitHelper::getCurrentView());

    handler->queue(COKitCallbackType::INVALIDATE_VISIBLE_CURSOR, ""_ostr); // 0
    handler->queue(COKitCallbackType::TEXT_SELECTION, "15, 25, 15, 10"_ostr); // Superseded.
    handler->queue(COKitCallbackType::INVALIDATE_VISIBLE_CURSOR, ""_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::INVALIDATE_TILES, "15, 25, 15, 10, 0, 0"_ostr); // 1
    handler->queue(COKitCallbackType::TEXT_SELECTION, "15, 25, 15, 10"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::TEXT_SELECTION, ""_ostr); // Superseded.
    handler->queue(COKitCallbackType::STATE_CHANGED, ""_ostr); // 2
    handler->queue(COKitCallbackType::STATE_CHANGED, ".uno:Bold"_ostr); // 3
    handler->queue(COKitCallbackType::STATE_CHANGED, ""_ostr); // 4
    handler->queue(COKitCallbackType::MOUSE_POINTER, "text"_ostr); // 5
    handler->queue(COKitCallbackType::INVALIDATE_TILES, "15, 25, 15, 10, 0, 0"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::INVALIDATE_TILES, "15, 25, 15, 10, 0, 0"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::MOUSE_POINTER, "text"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::TEXT_SELECTION_START, "15, 25, 15, 10"_ostr); // Superseded.
    handler->queue(COKitCallbackType::TEXT_SELECTION_END, "15, 25, 15, 10"_ostr); // Superseded.
    handler->queue(COKitCallbackType::TEXT_SELECTION, "15, 25, 15, 10"_ostr); // Superseded.
    handler->queue(COKitCallbackType::TEXT_SELECTION_START, "15, 25, 15, 10"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::TEXT_SELECTION_END, "15, 25, 15, 10"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::TEXT_SELECTION, ""_ostr); // 7
    handler->queue(COKitCallbackType::TEXT_SELECTION_START, "15, 25, 15, 10"_ostr); // 8
    handler->queue(COKitCallbackType::TEXT_SELECTION_END, "15, 25, 15, 10"_ostr); // 9
    handler->queue(COKitCallbackType::CELL_CURSOR, "15, 25, 15, 10"_ostr); // 10
    handler->queue(COKitCallbackType::CURSOR_VISIBLE, ""_ostr); // 11
    handler->queue(COKitCallbackType::CELL_CURSOR, "15, 25, 15, 10"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::CELL_FORMULA, "blah"_ostr); // 12
    handler->queue(COKitCallbackType::SET_PART, "1"_ostr); // 13
    handler->queue(COKitCallbackType::STATE_CHANGED, ".uno:AssignLayout=20"_ostr); // Superseded
    handler->queue(COKitCallbackType::CURSOR_VISIBLE, ""_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::CELL_FORMULA, "blah"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::SET_PART, "1"_ostr); // Should be dropped.
    handler->queue(COKitCallbackType::STATE_CHANGED, ".uno:AssignLayout=1"_ostr); // 14

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(14), notifs.size());

    size_t i = 0;
    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_VISIBLE_CURSOR), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10, 0, 0"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::STATE_CHANGED), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::STATE_CHANGED), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:Bold"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::STATE_CHANGED), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::MOUSE_POINTER), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("text"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::TEXT_SELECTION), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::TEXT_SELECTION_START), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::TEXT_SELECTION_END), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::CELL_CURSOR), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("15, 25, 15, 10"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::CURSOR_VISIBLE), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(""), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::CELL_FORMULA), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("blah"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::SET_PART), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string("1"), std::get<1>(notifs[i++]));

    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::STATE_CHANGED), std::get<0>(notifs[i]));
    CPPUNIT_ASSERT_EQUAL(std::string(".uno:AssignLayout=1"), std::get<1>(notifs[i++]));
}

void DesktopKitTest::testVectorDeltaPushCoalescing()
{
    // Repeated part changes produce a single pushed vector
    // primitives delta when the queue flushes, delivered after
    // the queued messages.
    COKitDocumentImpl* pDocument = loadDoc("blank_presentation.odp");
    std::vector<std::tuple<int, std::string>> aNotificationList;
    std::unique_ptr<CallbackFlushHandler> handler(
        new CallbackFlushHandler(pDocument, callbackCompressionTest, &aNotificationList));
    handler->setViewId(KitHelper::getCurrentView());
    handler->setVectorRendering();

    tools::Rectangle aRectangle1(Point(10, 10), Size(20, 10));
    handler->viewInvalidateTilesCallback(&aRectangle1, 0, 0);
    handler->viewVectorPartChanged(0);
    handler->viewVectorPartChanged(0);

    Scheduler::ProcessEventsToIdle();

    size_t nDeltaCount = 0;
    for (const auto& rNotification : aNotificationList)
    {
        if (std::get<0>(rNotification) == static_cast<int>(COKitCallbackType::VECTOR_PRIMITIVES_DELTA))
            ++nDeltaCount;
    }
    CPPUNIT_ASSERT_EQUAL(size_t(1), nDeltaCount);

    CPPUNIT_ASSERT(!aNotificationList.empty());
    CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::VECTOR_PRIMITIVES_DELTA), std::get<0>(aNotificationList.back()));
    CPPUNIT_ASSERT(std::get<1>(aNotificationList.back()).find("vectorprimitivesdelta") != std::string::npos);
}

void DesktopKitTest::testTileInvalidationCompression()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    // Single part merging
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "-100, -50, 500, 650, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, -32767, -32767, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "100, 100, 200, 200, 0, 0"_ostr);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 400, 600, 0, 0"), std::get<1>(notifs[i++]));
    }

    // Part Number
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 200, 200, 1, 0"_ostr); // Different part
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 0, 0, 2, 0"_ostr); // Invalid
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "-121, -121, 200, 200, 0, 0"_ostr); // Inside first
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, -32767, -32767, 1, 0"_ostr); // Invalid

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 1, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 239, 239, 0, 0"), std::get<1>(notifs[i++]));
    }

    // All Parts
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 0, 0"_ostr); // 0
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 200, 200, 1, 0"_ostr); // 1: Different part
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 0, 0, -1, 0"_ostr); // Invalid
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "-121, -121, 200, 200, -1, 0"_ostr); // 0: All parts
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, -32767, -32767, -1, 0"_ostr); // Invalid
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "-100, -100, 1200, 1200, -1, 0"_ostr); // 0: All parts
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 3, 0"_ostr); // Overlapped
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "1000, 1000, 1239, 1239, 2, 0"_ostr); // 1: Unique region

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 1100, 1100, -1, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("1000, 1000, 1239, 1239, 2, 0"), std::get<1>(notifs[i++]));
    }

    // All Parts (partial)
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 200, 200, 0, 0"_ostr); // 0
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 100, 100, 1, 0"_ostr); // 1: Different part
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 0, 0, -1, 0"_ostr); // Invalid
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "150, 150, 50, 50, -1, 0"_ostr); // 2: All-parts
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, -32767, -32767, -1, 0"_ostr); // Invalid
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "150, 150, 40, 40, 3, 0"_ostr); // Overlapped w/ 2
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 200, 200, 4, 0"_ostr); // 3: Unique
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "1000, 1000, 1239, 1239, 1, 0"_ostr); // 4: Unique

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 0, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 100, 100, 1, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("150, 150, 50, 50, -1, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 200, 200, 4, 0"), std::get<1>(notifs[i++]));

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("1000, 1000, 1239, 1239, 1, 0"), std::get<1>(notifs[i++]));
    }

    // Merge with "EMPTY"
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 239, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "EMPTY, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, 239, 240, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "-121, -121, 300, 300, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "0, 0, -32767, -32767, 0, 0"_ostr);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        size_t i = 0;
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[i]));
        CPPUNIT_ASSERT_EQUAL(std::string("EMPTY, 0, 0"), std::get<1>(notifs[i++]));
    }
}

void DesktopKitTest::testPartInInvalidation()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    // Intersection and parts match -> merge.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "10, 10, 20, 10, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "20, 10, 20, 10, 0, 0"_ostr);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());

        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[0]));
        CPPUNIT_ASSERT_EQUAL(std::string("10, 10, 30, 10, 0, 0"), std::get<1>(notifs[0]));
    }
    // No intersection -> don't merge.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "10, 10, 20, 10, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "40, 10, 20, 10, 0, 0"_ostr);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());
    }
    // Intersection and parts don't match -> don't merge.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackCompressionTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, "10, 10, 20, 10, 0, 0"_ostr);
        handler->queue(COKitCallbackType::INVALIDATE_TILES, "20, 10, 20, 10, 1, 0"_ostr);

        Scheduler::ProcessEventsToIdle();

        // This failed as RectangleAndPart::Create() always assumed no part in
        // payload, so this was merged -> it was 1.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), notifs.size());
    }
}

static void callbackBinaryCallbackTest(COKitCallbackType eType, const char* payload, void* data)
{
    std::vector<std::tuple<int, std::string>>* notifs = static_cast<std::vector<std::tuple<int, std::string>>*>(data);
    notifs->emplace_back(static_cast<int>(eType), std::string(payload ? payload : "(nil)"));
}

void DesktopKitTest::testBinaryCallback()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    const tools::Rectangle rect1(Point(10,15),Size(20,25));
    // -1 means all parts, so the invalidation carries the part it was given.
    const int nAllParts = -1;
    const std::string rect1String(std::string(rect1.toString()) + ", -1, 0");
    // Verify that using queue() and viewInvalidateTilesCallback() has the same result.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->queue(COKitCallbackType::INVALIDATE_TILES, OString(rect1String));

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[0]));
        CPPUNIT_ASSERT_EQUAL(rect1String, std::get<1>(notifs[0]));
    }
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->tilePainted(nAllParts, /*nMode=*/0, rect1);
        handler->viewInvalidateTilesCallback(&rect1, nAllParts, 0);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[0]));
        CPPUNIT_ASSERT_EQUAL(rect1String, std::get<1>(notifs[0]));
    }
    // Verify that the "EMPTY" invalidation gets converted properly.
    {
        std::vector<std::tuple<int, std::string>> notifs;
        std::unique_ptr<CallbackFlushHandler> handler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &notifs));
        handler->setViewId(KitHelper::getCurrentView());

        handler->tilePainted(nAllParts, /*nMode=*/0, rect1);
        handler->viewInvalidateTilesCallback(nullptr, nAllParts, 0);

        Scheduler::ProcessEventsToIdle();

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), notifs.size());
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(notifs[0]));
        CPPUNIT_ASSERT_EQUAL(std::string("EMPTY, -1, 0"), std::get<1>(notifs[0]));
    }
}

void DesktopKitTest::testOmitInvalidate()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    tools::Rectangle aRectangle{Point(0, 0), Size(10, 10)};

    {
        // Given a clean state:
        std::vector<std::tuple<int, std::string>> aCallbacks;
        std::unique_ptr<CallbackFlushHandler> pHandler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &aCallbacks));
        pHandler->setViewId(0);

        // When emitting just an invalidation:
        pHandler->viewInvalidateTilesCallback(&aRectangle, /*nPart=*/0, /*nMode=*/0);

        // Then make sure that's filtered out:
        Scheduler::ProcessEventsToIdle();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 0
        // - Actual  : 1
        // i.e. invalidation was emitted when we haven't rendered any tiles yet.
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aCallbacks.size());
    }

    {
        // Given a clean state:
        std::vector<std::tuple<int, std::string>> aCallbacks;
        std::unique_ptr<CallbackFlushHandler> pHandler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &aCallbacks));
        pHandler->setViewId(0);

        // When emitting an invalidation outside the painted area:
        pHandler->tilePainted(/*nPart=*/0, /*nMode=*/0, aRectangle);
        tools::Rectangle aElsewhere{Point(20, 20), Size(10, 10)};
        pHandler->viewInvalidateTilesCallback(&aElsewhere, /*nPart=*/0, /*nMode=*/0);

        // Then make sure that's filtered out:
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), aCallbacks.size());
    }

    {
        // Given a clean state:
        std::vector<std::tuple<int, std::string>> aCallbacks;
        std::unique_ptr<CallbackFlushHandler> pHandler(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &aCallbacks));
        pHandler->setViewId(0);

        // When emitting an invalidation partly outside the painted area:
        pHandler->tilePainted(/*nPart=*/0, /*nMode=*/0, aRectangle);
        tools::Rectangle aLarger{Point(0, 0), Size(20, 10)};
        pHandler->viewInvalidateTilesCallback(&aLarger, /*nPart=*/0, /*nMode=*/0);

        // Then make sure that's cropped:
        Scheduler::ProcessEventsToIdle();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aCallbacks.size());
        CPPUNIT_ASSERT_EQUAL(int(COKitCallbackType::INVALIDATE_TILES), std::get<0>(aCallbacks[0]));
        // x, y, w, h, part, mode; so this is cropped.
        CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 9, 9, 0, 0"), std::get<1>(aCallbacks[0]));
    }
}

void DesktopKitTest::test2ViewsOmitInvalidate()
{
    // Given a document with 2 views:
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    std::vector<std::tuple<int, std::string>> aCallbacks1;
    std::shared_ptr<CallbackFlushHandler> pHandler1(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &aCallbacks1));
    pHandler1->setViewId(0);
    pDocument->mpCallbackFlushHandlers[0] = pHandler1;
    std::vector<std::tuple<int, std::string>> aCallbacks2;
    std::shared_ptr<CallbackFlushHandler> pHandler2(new CallbackFlushHandler(pDocument, callbackBinaryCallbackTest, &aCallbacks2));
    pHandler2->setViewId(1);
    pDocument->mpCallbackFlushHandlers[1] = pHandler2;

    // When painting a tile for a larger area, and then 2 invalidates: the first view gets a smaller
    // invalidate, the second view gets a larger invalidate:
    tools::Rectangle aPaint{Point(0, 0), Size(20, 10)};
    pDocument->updateViewsForPaintedTile(/*nOrigViewId=*/0, /*nPart=*/0, /*nMode=*/0, aPaint);
    tools::Rectangle aSmaller{Point(0, 0), Size(10, 10)};
    pHandler1->viewInvalidateTilesCallback(&aSmaller, /*nPart=*/0, /*nMode=*/0);
    tools::Rectangle aLarger{Point(0, 0), Size(20, 10)};
    pHandler2->viewInvalidateTilesCallback(&aLarger, /*nPart=*/0, /*nMode=*/0);

    // Then make sure this larger invalidate for the 2nd view is not lost:
    Scheduler::ProcessEventsToIdle();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the 2nd view's (larger) invalidate was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aCallbacks2.size());
    CPPUNIT_ASSERT_EQUAL(std::string("0, 0, 19, 9, 0, 0"), std::get<1>(aCallbacks2[0]));
}

void DesktopKitTest::testInput()
{
    // Load a Writer document, enable change recording and press a key.
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    Scheduler::ProcessEventsToIdle(); // Get focus & other bits setup.

    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, "far");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, "far");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, " ");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, " ");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, "beyond");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, "beyond");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, " ");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, " ");
    // Mis-spelled ...
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, "kovely");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, "kovely");
    // Remove it again
    pDocument->removeTextContext(0, 6, 0);
    // Replace it with lovely
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, "lovely");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, "lovely");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, " ");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, " ");

    // get the text ...
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    std::string aText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL("far beyond lovely "_ostr, OString(aText));
}

void DesktopKitTest::testRedlineWriter()
{
    // Load a Writer document, enable change recording and press a key.
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue(u"RecordChanges"_ustr, cpo::uno::Any(true));
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 't', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 't', 0);
    Scheduler::ProcessEventsToIdle();

    // Get redline info.
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:AcceptTrackedChanges");
    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("redlines").size());

    for (const boost::property_tree::ptree::value_type& rRedline : aTree.get_child("redlines"))
        // This failed with boost::property_tree::ptree_bad_path, as there were no description field.
        CPPUNIT_ASSERT_EQUAL(std::string("Insert \xE2\x80\x9Ct\xE2\x80\x9D"), rRedline.second.get<std::string>("description"));
            // U+201C LEFT DOUBLE QUOTATION MARK, U+201D RIGHT DOUBLE QUOTATION
            // MARK
}

void DesktopKitTest::testRedlineCalc()
{
    // Load a Writer document, enable change recording and press a key.
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods");
    uno::Reference<beans::XPropertySet> xPropertySet(mxComponent, uno::UNO_QUERY);
    xPropertySet->setPropertyValue(u"RecordChanges"_ustr, cpo::uno::Any(true));
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 't', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 't', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RETURN);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Get redline info.
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:AcceptTrackedChanges");
    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    // Make sure that pressing a key creates exactly one redline.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aTree.get_child("redlines").size());

    for (const boost::property_tree::ptree::value_type& rRedline : aTree.get_child("redlines"))
        // This failed with boost::property_tree::ptree_bad_path, as there were no description field.
        CPPUNIT_ASSERT_EQUAL(std::string("Cell B4 changed from '5' to 't'"), rRedline.second.get<std::string>("description"));
}

namespace {

struct RedlineInfo
{
    std::string action;
    std::string index;
    std::string author;
    std::string type;
    std::string comment;
    std::string description;
    std::string dateTime;
};

std::vector<RedlineInfo> getRedlineInfo(const boost::property_tree::ptree& redlineNode)
{
    std::vector<RedlineInfo> result;
    result.reserve(redlineNode.size());
    for (const auto& redline : redlineNode)
    {
        result.emplace_back();
        result.back().index = redline.second.get<std::string>("index");
        result.back().author = redline.second.get<std::string>("author");
        result.back().type = redline.second.get<std::string>("type");
        result.back().comment = redline.second.get<std::string>("comment");
        result.back().description = redline.second.get<std::string>("description");
        result.back().dateTime = redline.second.get<std::string>("dateTime");
        if (auto oAction = redline.second.get_optional<std::string>("action"))
            result.back().action = *oAction;
    }

    return result;
}

std::vector<RedlineInfo> getRedlineInfo(COKitDocumentImpl* pDocument)
{
    std::string json
        = pDocument->getCommandValues(".uno:AcceptTrackedChanges");
    std::stringstream stream(json);
    CPPUNIT_ASSERT(!stream.str().empty());
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(stream, tree);
    return getRedlineInfo(tree.get_child("redlines"));
}

class ViewCallback
{
    COKitDocumentImpl* mpDocument;
    int mnView;
public:
    OString m_aCellFormula;
    boost::property_tree::ptree m_JSONDialog;
    int m_nTableSelectionCount;
    int m_nColorPaletteCallbackCount = 0;
    bool m_bEmptyTableSelection;
    bool m_bTilesInvalidated;
    bool m_bZeroCursor;
    bool m_stateBold;
    tools::Rectangle m_aOwnCursor;
    boost::property_tree::ptree m_aCommentCallbackResult;
    int m_nComments = 0;
    int m_nDocSizeChanged = 0;
    int m_nTextViewSelection = 0;
    boost::property_tree::ptree m_aColorPaletteCallbackResult;
    RedlineInfo m_aLastRedlineInfo;
    std::string m_searchTerm;
    int m_findReplaceDialogId;
    unsigned long long m_posSizeDialogId = 0;

    ViewCallback(COKitDocumentImpl* pDocument)
        : mpDocument(pDocument),
          m_nTableSelectionCount(0),
          m_bEmptyTableSelection(false),
          m_bTilesInvalidated(false),
          m_bZeroCursor(false),
          m_stateBold(false)
    {
        mnView = KitHelper::getCurrentView();
        mpDocument->registerCallback(&ViewCallback::callback, this);
    }

    ~ViewCallback()
    {
        mpDocument->setView(mnView);
        mpDocument->registerCallback(nullptr, nullptr);
    }

    static void callback(COKitCallbackType eType, const char* pPayload, void* pData)
    {
        static_cast<ViewCallback*>(pData)->callbackImpl(eType, pPayload);
    }

    void callbackImpl(COKitCallbackType eType, const char* pPayload)
    {
        OString aPayload(pPayload);
        switch (eType)
        {
        case COKitCallbackType::INVALIDATE_TILES:
        {
            m_bTilesInvalidated = true;
        }
        break;
        case COKitCallbackType::INVALIDATE_VISIBLE_CURSOR:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree aTree;
            boost::property_tree::read_json(aStream, aTree);
            OString aRectangle(aTree.get_child("rectangle").get_value<std::string>());

            if (aRectangle == "EMPTY")
                return;
            cpo::uno::Sequence<OUString> aSeq
                = comphelper::string::convertCommaSeparated(OUString::fromUtf8(aRectangle));
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), aSeq.getLength());
            m_aOwnCursor.SetLeft(aSeq[0].toInt32());
            m_aOwnCursor.SetTop(aSeq[1].toInt32());
            m_aOwnCursor.setWidth(aSeq[2].toInt32());
            m_aOwnCursor.setHeight(aSeq[3].toInt32());

            if (m_aOwnCursor.Left() == 0 && m_aOwnCursor.Top() == 0)
                m_bZeroCursor = true;
        }
        break;
        case COKitCallbackType::COMMENT:
        {
            ++m_nComments;
            m_aCommentCallbackResult.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aCommentCallbackResult);
            m_aCommentCallbackResult = m_aCommentCallbackResult.get_child("comment");
        }
        break;
        case COKitCallbackType::DOCUMENT_SIZE_CHANGED:
        {
            ++m_nDocSizeChanged;
        }
        break;
        case COKitCallbackType::TEXT_VIEW_SELECTION:
        {
            ++m_nTextViewSelection;
        }
        break;
        case COKitCallbackType::CELL_FORMULA:
        {
            m_aCellFormula = aPayload;
        }
        break;
        case COKitCallbackType::TABLE_SELECTED:
        {
            m_bEmptyTableSelection = (std::string(pPayload).compare("{ }") == 0);
            ++m_nTableSelectionCount;
        }
        break;
        case COKitCallbackType::COLOR_PALETTES:
        {
            m_aColorPaletteCallbackResult.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_aColorPaletteCallbackResult);
            ++m_nColorPaletteCallbackCount;
        }
        break;
        case COKitCallbackType::WINDOW:
        {
            m_JSONDialog.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_JSONDialog);

            if (m_JSONDialog.find("title") != m_JSONDialog.not_found() && m_JSONDialog.get_child("title").get_value<std::string>() == "Find and Replace")
            {
                m_findReplaceDialogId = std::atoi(m_JSONDialog.get_child("id").get_value<std::string>().c_str());
                // Set search term to something random and make sure it is read from incoming JSON (COKitCallbackType::JSDIALOG).
                m_searchTerm = "something random";
            }
        }
        break;
        case COKitCallbackType::JSDIALOG:
        {
            m_JSONDialog.clear();
            std::stringstream aStream(pPayload);
            boost::property_tree::read_json(aStream, m_JSONDialog);

            if (m_JSONDialog.find("jsontype") != m_JSONDialog.not_found() && m_JSONDialog.get_child("jsontype").get_value<std::string>() == "dialog")
            {
                if (m_JSONDialog.find("title") != m_JSONDialog.not_found()
                    && m_JSONDialog.get_child("title").get_value<std::string>() == "Position and Size")
                {
                    // The full dialog dump can contain two top level "id" keys: the
                    // widget id string and the numeric kit window id. Take the numeric one.
                    for (const auto& rPair : m_JSONDialog)
                    {
                        if (rPair.first != "id")
                            continue;
                        const std::string sId = rPair.second.get_value<std::string>();
                        if (unsigned long long nId = std::strtoull(sId.c_str(), nullptr, 10))
                            m_posSizeDialogId = nId;
                    }
                }
                if (m_JSONDialog.find("data") != m_JSONDialog.not_found())
                {
                    if (m_JSONDialog.get_child("data").find("control_id") != m_JSONDialog.get_child("data").not_found())
                    {
                        if (m_JSONDialog.get_child("data").get_child("control_id").get_value<std::string>() == "searchterm")
                        {
                            if (m_JSONDialog.get_child("data").find("text") != m_JSONDialog.get_child("data").not_found())
                            {
                                m_searchTerm = m_JSONDialog.get_child("data").get_child("text").get_value<std::string>();
                            }
                        }
                    }
                }
            }
        }
        break;
        case COKitCallbackType::REDLINE_TABLE_SIZE_CHANGED:
        case COKitCallbackType::REDLINE_TABLE_ENTRY_MODIFIED:
        {
            std::stringstream aStream(pPayload);
            boost::property_tree::ptree tree;
            boost::property_tree::read_json(aStream, tree);
            auto redlines = getRedlineInfo(tree);
            CPPUNIT_ASSERT_EQUAL(size_t(1), redlines.size());
            m_aLastRedlineInfo = redlines[0];
        }
        break;
        case COKitCallbackType::STATE_CHANGED:
        {
            if (aPayload.startsWith(".uno:Bold="))
            {
                m_stateBold = aPayload.copy(".uno:Bold="_ostr.getLength()).toBoolean();
            }
        }
        break;
        default:
        break;
        }
    }
};

}

void DesktopKitTest::testPaintPartTile()
{
    // Load an impress doc of 2 slides.
//    ViewCallback aView1;
//    ViewCallback aView2;
    COKitDocumentImpl* pDocument = loadDoc("2slides.odp");
    pDocument->initializeForRendering("{}");
//    pDocument->registerCallback(&ViewCallback::callback, &aView1);
    int nView1 = pDocument->getView();

    // Create a second view.
    pDocument->createView();
    pDocument->initializeForRendering("{}");
//    pDocument->registerCallback(&ViewCallback::callback, &aView2);

    // Go to the second slide in the second view. The boundary names a slide by
    // its part number, the page's stable unique id.
    const int nSecondSlide
        = static_cast<int>(pDocument->getPartUniqueId(1, 0));
    pDocument->setPart(nSecondSlide);

    // Switch back to the first view and start typing.
    pDocument->setView(nView1);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, awt::Key::TAB);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, awt::Key::TAB);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    // Call paintPartTile() to paint the second part (in whichever view it finds suitable for this).
    unsigned char pPixels[256 * 256 * 4];
    pDocument->paintPartTile(pPixels, nSecondSlide, 0, 256, 256, 0, 0, 256, 256);

    // Type again.
    Scheduler::ProcessEventsToIdle();
//    aView1.m_bTilesInvalidated = false;
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();
    // This failed: paintPartTile() (as a side-effect) ended the text edit of
    // the first view, so there were no invalidations.
    //CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
}

void DesktopKitTest::testPaintTileOmitInvalidate()
{
    // Given a painted tile:
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    ViewCallback aView(pDocument);
    const int nCanvasWidth = 256;
    const int nCanvasHeight = 256;
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aPixels;
    pDocument->paintTile(aPixels.data(), nCanvasWidth, nCanvasHeight, 0, 0, 3840, 3840);
    Scheduler::ProcessEventsToIdle();
    aView.m_bTilesInvalidated = false;

    // When pressing a key:
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    Scheduler::ProcessEventsToIdle();

    // Then make sure we get an invalidation:
    CPPUNIT_ASSERT(aView.m_bTilesInvalidated);
}

void DesktopKitTest::testCreateViewOmitInvalidate()
{
    // Given a document with 2 views: view 1 renders sheet One, then view 2 gets created and finally
    // view 1 switches to sheet Two:
    COKitDocumentImpl* pDocument = loadDoc("create-view-omit-invalidate.ods");
    pDocument->initializeForRendering(nullptr);
    ViewCallback aView1(pDocument);
    int nView1 = pDocument->getView();
    const int nCanvasWidth = 256;
    const int nCanvasHeight = 256;
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aPixels;
    pDocument->paintTile(aPixels.data(), nCanvasWidth, nCanvasHeight, 0, 0, 3840, 3840);
    pDocument->createView();
    pDocument->initializeForRendering(nullptr);
    ViewCallback aView2(pDocument);
    pDocument->setView(nView1);
    pDocument->setPart(1);
    Scheduler::ProcessEventsToIdle();
    aView1.m_bTilesInvalidated = false;
    aView2.m_bTilesInvalidated = false;

    // When pressing a key in view 1, on sheet Two:
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'x', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 'x', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RETURN);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RETURN);
    Scheduler::ProcessEventsToIdle();

    // Then make sure that both views are invalidated:
    CPPUNIT_ASSERT(aView1.m_bTilesInvalidated);
    // Without the accompanying fix in place, this test would have failed, the 2nd view was not
    // invalidated when it was created after a paintTile().
    CPPUNIT_ASSERT(aView2.m_bTilesInvalidated);
}

void DesktopKitTest::testPaintPartTileHidesGridOnOtherPart()
{
    // Load an impress doc of 2 slides, with a single view on the first one.
    COKitDocumentImpl* pDocument = loadDoc("2slides.odp");
    pDocument->initializeForRendering("{}");

    // The boundary names a slide by its part number, the page's stable unique id.
    const int nSecondSlide = static_cast<int>(pDocument->getPartUniqueId(1, 0));

    constexpr int nCanvasWidth = 256;
    constexpr int nCanvasHeight = 256;
    // this is BGRA format data
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aWithoutGrid;
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aWithGridOnEditedPart;

    // There is no other view already sitting on the second slide, so this
    // has to temporarily borrow the only view to paint a page it is not
    // editing. This is a preview request, the way the slide panel asks for
    // a thumbnail of a slide other than the one being edited.
    pDocument->paintPartTile(aWithoutGrid.data(), nSecondSlide, 0, nCanvasWidth, nCanvasHeight, 0,
                              0, 3840, 3840, /*bIsPreview=*/true);

    // Turn the grid on, as if the user enabled it while editing the first slide.
    dispatchCommand(mxComponent, u".uno:GridVisible"_ustr, cpo::uno::Sequence<beans::PropertyValue>());

    pDocument->paintPartTile(aWithGridOnEditedPart.data(), nSecondSlide, 0, nCanvasWidth,
                              nCanvasHeight, 0, 0, 3840, 3840, /*bIsPreview=*/true);

    // The grid is an editing aid for the page being edited, not part of a
    // page's own content, so the tile of the second slide must come out the
    // same whether or not the grid is shown on the first slide.
    CPPUNIT_ASSERT(operator==(aWithoutGrid, aWithGridOnEditedPart));
}

void DesktopKitTest::testPaintPartTileHidesGridOnActivePartPreview()
{
    // Load an impress doc, keep the single view on the (active) first slide.
    COKitDocumentImpl* pDocument = loadDoc("2slides.odp");
    pDocument->initializeForRendering("{}");

    const int nFirstSlide = static_cast<int>(pDocument->getPartUniqueId(0, 0));

    constexpr int nCanvasWidth = 256;
    constexpr int nCanvasHeight = 256;
    // this is BGRA format data
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aWithoutGrid;
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aViewportWithGrid;
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aPreviewWithGrid;

    // Baseline: grid off, a tile of the (only) active slide.
    pDocument->paintPartTile(aWithoutGrid.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight, 0,
                              0, 3840, 3840);

    dispatchCommand(mxComponent, u".uno:GridVisible"_ustr, cpo::uno::Sequence<beans::PropertyValue>());

    // The editing viewport's own tile of the active slide still shows the
    // grid the user turned on: this is not a preview.
    pDocument->paintPartTile(aViewportWithGrid.data(), nFirstSlide, 0, nCanvasWidth,
                              nCanvasHeight, 0, 0, 3840, 3840, /*bIsPreview=*/false);
    CPPUNIT_ASSERT(!(aWithoutGrid == aViewportWithGrid));

    // A preview of that same active slide must not carry the grid along,
    // even though the tile is otherwise identical to the viewport's own.
    pDocument->paintPartTile(aPreviewWithGrid.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight,
                              0, 0, 3840, 3840, /*bIsPreview=*/true);
    CPPUNIT_ASSERT(operator==(aWithoutGrid, aPreviewWithGrid));
}

void DesktopKitTest::testPaintPartTileDifferentSchemes()
{
    Color aDarkColor(0x1c, 0x1c, 0x1c);

    // Add a minimal dark scheme
    {
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = aDarkColor;
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"Dark"_ustr);
    }

    // Add a minimal light scheme
    {
        svtools::EditableColorConfig aColorConfig;
        svtools::ColorConfigValue aValue;
        aValue.bIsVisible = true;
        aValue.nColor = COL_WHITE;
        aColorConfig.SetColorValue(svtools::DOCCOLOR, aValue);
        aColorConfig.AddScheme(u"Light"_ustr);
    }

    // This view will default to light scheme
    COKitDocumentImpl* pDocument = loadDoc("2slides.odp");
    pDocument->initializeForRendering("{}");
    int nView1 = pDocument->getView();

    // Create a second view
    pDocument->createView();
    pDocument->initializeForRendering("{}");

    // Go to the second slide in the second view. The boundary names a slide by
    // its part number, the page's stable unique id.
    const int nFirstSlide
        = static_cast<int>(pDocument->getPartUniqueId(0, 0));
    const int nSecondSlide
        = static_cast<int>(pDocument->getPartUniqueId(1, 0));
    pDocument->setPart(nSecondSlide);

    // Set to dark scheme
    {
        cpo::uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence(
            {
                { "NewTheme", cpo::uno::Any(u"Dark"_ustr) },
            }
        );
        dispatchCommand(mxComponent, u".uno:ChangeTheme"_ustr, aPropertyValues);
    }

    constexpr int nCanvasWidth = 256;
    constexpr int nCanvasHeight = 256;

    // Just a random pixel in the middle of the canvas
    constexpr int nPixelX = 128;
    constexpr int nPixelY = 128 * nCanvasWidth;

    // this is BGRA format data
    std::array<sal_uInt8, nCanvasWidth * nCanvasHeight * 4> aPixels;

    // Both parts should be painted with dark scheme
    pDocument->paintPartTile(aPixels.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight, 0, 0, nCanvasWidth, nCanvasHeight);
    Color aPixel(aPixels[nPixelX + nPixelY + 0], aPixels[nPixelX + nPixelY + 1], aPixels[nPixelX + nPixelY + 2]);
    CPPUNIT_ASSERT_EQUAL(aDarkColor, aPixel);

    pDocument->paintPartTile(aPixels.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight, 0, 0, nCanvasWidth, nCanvasHeight);
    aPixel = Color(aPixels[nPixelX + nPixelY + 0], aPixels[nPixelX + nPixelY + 1], aPixels[nPixelX + nPixelY + 2]);
    CPPUNIT_ASSERT_EQUAL(aDarkColor, aPixel);

    // Switch back to first view
    pDocument->setView(nView1);

    // Both parts should be painted with light scheme
    pDocument->paintPartTile(aPixels.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight, 0, 0, nCanvasWidth, nCanvasHeight);
    aPixel = Color(aPixels[nPixelX + nPixelY + 0], aPixels[nPixelX + nPixelY + 1], aPixels[nPixelX + nPixelY + 2]);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aPixel);

    pDocument->paintPartTile(aPixels.data(), nFirstSlide, 0, nCanvasWidth, nCanvasHeight, 0, 0, nCanvasWidth, nCanvasHeight);
    aPixel = Color(aPixels[nPixelX + nPixelY + 0], aPixels[nPixelX + nPixelY + 1], aPixels[nPixelX + nPixelY + 2]);
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, aPixel);
}

#if HAVE_MORE_FONTS
#include <rtl/uri.hxx>
void DesktopKitTest::testGetFontSubset()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    OUString aFontName = rtl::Uri::encode(
        u"Liberation Sans"_ustr,
        rtl_UriCharClassRelSegment,
        rtl_UriEncodeKeepEscapes,
        RTL_TEXTENCODING_UTF8
    );
    OString aCommand = ".uno:FontSubset&name=" + OUStringToOString(aFontName, RTL_TEXTENCODING_UTF8);
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(aCommand.getStr());
    std::stringstream aStream(aJSON);
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT( !aTree.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string(".uno:FontSubset"), aTree.get_child("commandName").get_value<std::string>() );
    boost::property_tree::ptree aValues = aTree.get_child("commandValues");
    CPPUNIT_ASSERT( !aValues.empty() );
}
#endif

void DesktopKitTest::testCommentsWriter()
{
    COKitDocumentImpl* pDocument = loadDoc("comments.odt");
    pDocument->initializeForRendering(nullptr);
    long nWidth, nHeight;
    pDocument->getDocumentSize(&nWidth, &nHeight);

    // Document width alongwith without sidebar comes to be < 13000
    CPPUNIT_ASSERT( nWidth < 13000 );

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:ViewAnnotations");
    std::stringstream aStream(aJSON);
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
        CPPUNIT_ASSERT(!rComment.second.get<std::string>("html").empty());
        // Has a valid iso 8601 date time string
        css::util::DateTime aDateTime;
        OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime"));
        CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));

        // This comment has a marked text range
        if (rComment.second.get<std::string>("html") == "<div>Comment 2</div>")
        {
            CPPUNIT_ASSERT(!rComment.second.get<std::string>("textRange").empty());
            nComment2Id = rComment.second.get<int>("id");
        }
        // This is a reply comment
        else if (rComment.second.get<std::string>("html") == "<div>Reply to Comment 2</div>")
        {
            CPPUNIT_ASSERT_EQUAL(nComment2Id, rComment.second.get<int>("parentId"));
        }
    }
}


void DesktopKitTest::testCommentsCalc()
{
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods");
    pDocument->initializeForRendering(nullptr);

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:ViewAnnotations");
    std::stringstream aStream(aJSON);
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
                CPPUNIT_ASSERT_EQUAL(std::string("4"), rComment.second.get<std::string>("tab"));
                CPPUNIT_ASSERT_EQUAL(std::string("Comment1"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("6 14 6 14"), rComment.second.get<std::string>("cellRange"));
            }
            break;
            case 1:
            {
                CPPUNIT_ASSERT_EQUAL(std::string("4"), rComment.second.get<std::string>("tab"));
                CPPUNIT_ASSERT_EQUAL(std::string("Comment2"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("7 17 7 17"), rComment.second.get<std::string>("cellRange"));
            }
            break;
        }

        ++nIdx;
    }

    // We checked all the comments
    CPPUNIT_ASSERT_EQUAL(2, nIdx);
}


void DesktopKitTest::testCommentsImpress()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_presentation.odp");
    pDocument->initializeForRendering(nullptr);

    // Can we get all the comments using .uno:ViewAnnotations command ?
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:ViewAnnotations");
    std::stringstream aStream(aJSON);
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
                CPPUNIT_ASSERT_EQUAL(std::string("Kit User1"), rComment.second.get<std::string>("author"));
                css::util::DateTime aDateTime;
                OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime"));
                CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));
            }
            break;
            case 1:
            {
                CPPUNIT_ASSERT(rComment.second.get<int>("id") > 0);
                CPPUNIT_ASSERT_EQUAL(std::string("This is comment2"), rComment.second.get<std::string>("text"));
                CPPUNIT_ASSERT_EQUAL(std::string("Kit User2"), rComment.second.get<std::string>("author"));
                css::util::DateTime aDateTime;
                OUString aDateTimeString = OUString::createFromAscii(rComment.second.get<std::string>("dateTime"));
                CPPUNIT_ASSERT(utl::ISO8601parseDateTime(aDateTimeString, aDateTime));
            }
            break;
        }

        ++nIdx;
    }

    // We checked all the comments
    CPPUNIT_ASSERT_EQUAL(2, nIdx);
}

void DesktopKitTest::testCommentsImpressCrossDocument()
{
    // Two presentations open in the same process.
    std::unique_ptr<COKitDocumentImpl> pDocument1 = loadDocImpl("blank_presentation.odp");
    pDocument1->initializeForRendering("{}");
    int nView1 = pDocument1->getView();
    ViewCallback aView1(pDocument1.get());

    std::unique_ptr<COKitDocumentImpl> pDocument2 = loadDocImpl("2slides.odp");
    pDocument2->initializeForRendering("{}");
    ViewCallback aView2(pDocument2.get());

    // Add a comment to the first presentation.
    pDocument1->setView(nView1);
    OString aCommandArgs("{ \"Text\": { \"type\": \"string\", \"value\": \"Comment in doc1\" }, \"Author\": { \"type\": \"string\", \"value\": \"Kit User1\" } }"_ostr);
    pDocument1->postUnoCommand(".uno:InsertAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // The comment is delivered to the presentation that owns it.
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Comment in doc1"), aView1.m_aCommentCallbackResult.get<std::string>("text"));

    // The other presentation never sees the comment.
    CPPUNIT_ASSERT_EQUAL(0, aView2.m_nComments);
}

void DesktopKitTest::testDocSizeChangedCrossDocument()
{
    // Two spreadsheets open in one process, both showing the first sheet.
    // The two files differ so that the desktop keeps two separate views
    // instead of reusing one for the same file.
    std::unique_ptr<COKitDocumentImpl> pDocument1 = loadDocImpl("empty.ods");
    pDocument1->initializeForRendering("{}");
    int nView1 = pDocument1->getView();
    ViewCallback aView1(pDocument1.get());

    std::unique_ptr<COKitDocumentImpl> pDocument2 = loadDocImpl("search.ods");
    pDocument2->initializeForRendering("{}");
    ViewCallback aView2(pDocument2.get());

    Scheduler::ProcessEventsToIdle();
    aView1.m_nDocSizeChanged = 0;
    aView2.m_nDocSizeChanged = 0;

    // Change a column width in the first spreadsheet. This repaints the column
    // headers, so the first spreadsheet is told its size changed. Both
    // spreadsheets show the same sheet number, so a broadcast that matches only
    // on the sheet without checking the owning document would reach the second
    // spreadsheet too.
    pDocument1->setView(nView1);
    pDocument1->postUnoCommand(".uno:ColumnWidth",
        "{ \"ColumnWidth\": { \"type\": \"unsigned short\", \"value\": \"4000\" },"
        " \"Column\": { \"type\": \"unsigned short\", \"value\": \"3\" } }",
        false);
    Scheduler::ProcessEventsToIdle();

    // The spreadsheet whose column changed is told its size changed; the other
    // never is.
    CPPUNIT_ASSERT(aView1.m_nDocSizeChanged > 0);
    CPPUNIT_ASSERT_EQUAL(0, aView2.m_nDocSizeChanged);
}

void DesktopKitTest::testViewSelectionCrossDocument()
{
    // Two spreadsheets open in one process, both showing the first sheet.
    // The two files differ so that the desktop keeps two separate views
    // instead of reusing one for the same file.
    std::unique_ptr<COKitDocumentImpl> pDocument1 = loadDocImpl("empty.ods");
    pDocument1->initializeForRendering("{}");
    int nView1 = pDocument1->getView();
    ViewCallback aView1(pDocument1.get());

    std::unique_ptr<COKitDocumentImpl> pDocument2 = loadDocImpl("search.ods");
    pDocument2->initializeForRendering("{}");
    ViewCallback aView2(pDocument2.get());

    Scheduler::ProcessEventsToIdle();
    aView2.m_nTextViewSelection = 0;

    // Select a cell range in the first spreadsheet.
    pDocument1->setView(nView1);
    pDocument1->postUnoCommand(".uno:GoToCell",
        "{ \"ToPoint\": { \"type\": \"string\", \"value\": \"B2:D5\" } }",
        false);
    Scheduler::ProcessEventsToIdle();

    // The other spreadsheet never receives the first one's view selection.
    CPPUNIT_ASSERT_EQUAL(0, aView2.m_nTextViewSelection);
}

namespace
{
/// Returns the rendered text of the first field found in a Writer document's first paragraph.
OUString getAuthorFieldText(COKitDocumentImpl* pDocument)
{
    uno::Reference<text::XTextDocument> xTextDocument(pDocument->mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(
        xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnumeration
        = xParagraphEnumerationAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphEnumeration->nextElement(),
                                                              uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xTextPortionEnumeration
        = xParagraph->createEnumeration();
    while (xTextPortionEnumeration->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortionEnumeration->nextElement(),
                                                          uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        if (aPortionType == "TextField")
        {
            uno::Reference<text::XTextField> xTextField(
                xTextPortion->getPropertyValue(u"TextField"_ustr), uno::UNO_QUERY);
            return xTextField->getPresentation(false);
        }
    }

    CPPUNIT_FAIL("no field found in the first paragraph");
}
}

void DesktopKitTest::testAuthorFieldUpdateCrossDocument()
{
    // Two Writer documents open in one process, each with an author field.
    // The two files differ so that the desktop keeps two separate views
    // instead of reusing one for the same file.
    std::unique_ptr<COKitDocumentImpl> pDocument1 = loadDocImpl("author-field-1.fodt");
    pDocument1->initializeForRendering("{}");

    std::unique_ptr<COKitDocumentImpl> pDocument2 = loadDocImpl("author-field-2.fodt");
    pDocument2->initializeForRendering("{}");

    // Record the second document's author field text before touching the first
    // document, so a later change to it would show up as a difference here.
    OUString sDocument2AuthorBefore = getAuthorFieldText(pDocument2.get());

    // Change the author on the first document while the second stays open.
    // Both documents' views are alive at this point, matching a process that
    // hosts several documents at once.
    pDocument1->initializeForRendering("{\".uno:Author\":{\"type\":\"string\",\"value\":\"New Author\"}}");

    // The first document's field picks up the new author...
    CPPUNIT_ASSERT_EQUAL(u"New Author"_ustr, getAuthorFieldText(pDocument1.get()));
    // ...and the second document's field is untouched.
    CPPUNIT_ASSERT_EQUAL(sDocument2AuthorBefore, getAuthorFieldText(pDocument2.get()));
}

void DesktopKitTest::testCommentsCallbacksWriter()
{
    COKitDocumentImpl* pDocument = loadDoc("comments.odt");
    pDocument->initializeForRendering("{}");
    ViewCallback aView1(pDocument);
    pDocument->createView();
    pDocument->initializeForRendering("{}");
    ViewCallback aView2(pDocument);

    // Add a new comment
    OString aCommandArgs("{ \"Text\": { \"type\": \"string\", \"value\": \"Additional comment\" }, \"Author\": { \"type\": \"string\", \"value\": \"Kit User1\" } }"_ostr);
    pDocument->postUnoCommand(".uno:InsertAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Add' action
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    int nCommentId1 = aView1.m_aCommentCallbackResult.get<int>("id");

    // Reply to a comment just added
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId1) + "\" }, \"Text\": { \"type\": \"string\", \"value\": \"Reply comment\" } }";
    pDocument->postUnoCommand(".uno:ReplyComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Add' action and linked to its parent comment
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment</div>"), aView1.m_aCommentCallbackResult.get<std::string>("html"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment</div>"), aView2.m_aCommentCallbackResult.get<std::string>("html"));
    int nCommentId2 = aView1.m_aCommentCallbackResult.get<int>("id");

    // Edit the previously added comment
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId2) + "\" }, \"Text\": { \"type\": \"string\", \"value\": \"Edited comment\" } }";
    pDocument->postUnoCommand(".uno:EditAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    // parent is unchanged still
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Edited comment</div>"), aView1.m_aCommentCallbackResult.get<std::string>("html"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Edited comment</div>"), aView2.m_aCommentCallbackResult.get<std::string>("html"));

    // Delete the reply comment just added
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\":  \"" + OString::number(nCommentId2) + "\" } }";
    pDocument->postUnoCommand(".uno:DeleteComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Remove' action
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId2, aView1.m_aCommentCallbackResult.get<int>("id"));
    CPPUNIT_ASSERT_EQUAL(nCommentId2, aView2.m_aCommentCallbackResult.get<int>("id"));

    // Reply to nCommentId1 again
    aCommandArgs = "{ \"Id\": { \"type\": \"string\", \"value\": \"" + OString::number(nCommentId1) + "\" }, \"Html\": { \"type\": \"string\", \"value\": \"Reply comment again\" } }";
    pDocument->postUnoCommand(".uno:ReplyComment", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Add' action and linked to its parent comment
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView2.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView2.m_aCommentCallbackResult.get<int>("parentId"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment again</div>"), aView1.m_aCommentCallbackResult.get<std::string>("html"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment again</div>"), aView2.m_aCommentCallbackResult.get<std::string>("html"));

    // Ensure that an undo and redo restores the html contents
    aView1.m_aCommentCallbackResult.clear();
    aView2.m_aCommentCallbackResult.clear();
    pDocument->postUnoCommand(".uno:Undo", "", false);
    pDocument->postUnoCommand(".uno:Redo", "", false);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment again</div>"), aView1.m_aCommentCallbackResult.get<std::string>("html"));
    CPPUNIT_ASSERT_EQUAL(std::string("<div>Reply comment again</div>"), aView2.m_aCommentCallbackResult.get<std::string>("html"));

    // .uno:ViewAnnotations returns total of 5 comments
    boost::property_tree::ptree aTree;
    std::string aJSON = pDocument->getCommandValues(".uno:ViewAnnotations");
    std::stringstream aStream(aJSON);
    CPPUNIT_ASSERT(!aStream.str().empty());
    boost::property_tree::read_json(aStream, aTree);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(5), aTree.get_child("comments").size());
}

namespace
{

void addParameter(tools::JsonWriter& rJson, const char* sName, std::string_view eType, std::string_view value)
{
    auto testNode = rJson.startNode(sName);
    rJson.put("type", eType);
    rJson.put("value", value);
}

}

void DesktopKitTest::testCommentsAddEditDeleteDraw()
{
    COKitDocumentImpl* pDocument = loadDoc("BlankDrawDocument.odg");
    pDocument->initializeForRendering("{}");
    ViewCallback aView1(pDocument);

    // Add a new comment
    OString aCommandArgs;
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Text", "string", "Comment");
        addParameter(aJson, "Author", "string", "Kit User1");
        aCommandArgs = aJson.finishAndGetAsOString();
    }

    pDocument->postUnoCommand(".uno:InsertAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Add' action
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    int nCommentId1 = aView1.m_aCommentCallbackResult.get<int>("id");

    // Edit the previously added comment
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Id", "string", OString::number(nCommentId1));
        addParameter(aJson, "Text", "string", "Edited comment");
        aCommandArgs = aJson.finishAndGetAsOString();
    }

    pDocument->postUnoCommand(".uno:EditAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("id"));

    // Delete Comment
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Id", "string", OString::number(nCommentId1));
        aCommandArgs = aJson.finishAndGetAsOString();
    }
    pDocument->postUnoCommand(".uno:DeleteAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Remove' action
    CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView1.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId1, aView1.m_aCommentCallbackResult.get<int>("id"));
}

void DesktopKitTest::testCommentsInReadOnlyMode()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    int viewId = pDocument->createView();
    pDocument->setView(viewId);

    pDocument->initializeForRendering("{\".uno:Author\":{\"type\":\"string\",\"value\":\"Kit User1\"}}");

    KitHelper::setViewReadOnly(viewId, true);
    KitHelper::setAllowChangeComments(viewId, true);

    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);

    // Add a new comment
    OString aCommandArgs;
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Text", "string", "Comment");
        addParameter(aJson, "Author", "string", "Kit User1");
        aCommandArgs = aJson.finishAndGetAsOString();
    }

    pDocument->postUnoCommand(".uno:InsertAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Add' action
    CPPUNIT_ASSERT_EQUAL(std::string("Add"), aView.m_aCommentCallbackResult.get<std::string>("action"));
    int nCommentId = aView.m_aCommentCallbackResult.get<int>("id");

    // Edit the previously added comment
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Id", "string", OString::number(nCommentId));
        addParameter(aJson, "Text", "string", "Edited comment");
        aCommandArgs = aJson.finishAndGetAsOString();
    }

    pDocument->postUnoCommand(".uno:EditAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // We received a COKitCallbackType::COMMENT callback with comment 'Modify' action
    CPPUNIT_ASSERT_EQUAL(std::string("Modify"), aView.m_aCommentCallbackResult.get<std::string>("action"));
    CPPUNIT_ASSERT_EQUAL(nCommentId, aView.m_aCommentCallbackResult.get<int>("id"));

    // Delete Comment
    {
        tools::JsonWriter aJson;
        addParameter(aJson, "Id", "string", OString::number(nCommentId));
        aCommandArgs = aJson.finishAndGetAsOString();
    }
    pDocument->postUnoCommand(".uno:DeleteAnnotation", aCommandArgs.getStr(), false);
    Scheduler::ProcessEventsToIdle();

    // Result is not sent for delete operation for some reason. But it is sent when debugging with online.
    // TODO: Enable below 2 checks.

    // We received a COKitCallbackType::COMMENT callback with comment 'Remove' action
    //CPPUNIT_ASSERT_EQUAL(std::string("Remove"), aView.m_aCommentCallbackResult.get<std::string>("action"));
    //CPPUNIT_ASSERT_EQUAL(nCommentId, aView.m_aCommentCallbackResult.get<int>("id"));
}

void DesktopKitTest::testRedlinesInReadOnlyMode()
{
    // In AllowManageRedlines mode, it must be possible to perform redline editing commands,
    // even in read-only mode.

    using namespace std::string_literals;

    COKitDocumentImpl* pDocument = loadDoc("three-changes.fodt");

    int viewId = pDocument->createView();
    pDocument->setView(viewId);
    pDocument->initializeForRendering("{}");
    ViewCallback aCallback(pDocument);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(size_t(3), getRedlineInfo(pDocument).size());

    // Activate read-only mode
    KitHelper::setViewReadOnly(viewId, true);

    // Go to the 1st tracked change: "Delete “Donec”"
    pDocument->postUnoCommand(".uno:NextTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();

    // Check that redline management commands don't work in pure read-only
    // Try to reject current redline
    pDocument->postUnoCommand(".uno:RejectTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();
    // Nothing happened
    CPPUNIT_ASSERT_EQUAL(size_t(3), getRedlineInfo(pDocument).size());
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.action);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.author);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.type);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.comment);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.description);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.dateTime);

    // Activate the AllowManageRedlines mode
    KitHelper::setAllowManageRedlines(viewId, true);

    // Try to reject current redline
    pDocument->postUnoCommand(".uno:RejectTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();
    // One change gone; it is recorded "Remove"d in aCallback.m_aLastRedlineInfo
    CPPUNIT_ASSERT_EQUAL(size_t(2), getRedlineInfo(pDocument).size());
    CPPUNIT_ASSERT_EQUAL("Remove"s, aCallback.m_aLastRedlineInfo.action);
    CPPUNIT_ASSERT_EQUAL("Mike"s, aCallback.m_aLastRedlineInfo.author);
    CPPUNIT_ASSERT_EQUAL("Delete"s, aCallback.m_aLastRedlineInfo.type);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.comment);
    CPPUNIT_ASSERT_EQUAL("Delete “Donec”"s, aCallback.m_aLastRedlineInfo.description);
    CPPUNIT_ASSERT_EQUAL("2025-06-16T14:08:27"s, aCallback.m_aLastRedlineInfo.dateTime);

    // Go to the 2nd tracked change: "Attributes changed"
    pDocument->postUnoCommand(".uno:NextTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();

    // Comment on it
    pDocument->postUnoCommand(".uno:CommentChangeTracking",
                                      R"({"Text":{"type":"string","value":"Some comment"}})",
                                      false);
    Scheduler::ProcessEventsToIdle();
    // One change got a comment; it is recorded "Modify"ed in aCallback.m_aLastRedlineInfo
    CPPUNIT_ASSERT_EQUAL(size_t(2), getRedlineInfo(pDocument).size());
    CPPUNIT_ASSERT_EQUAL("Modify"s, aCallback.m_aLastRedlineInfo.action);
    CPPUNIT_ASSERT_EQUAL("Mike"s, aCallback.m_aLastRedlineInfo.author);
    CPPUNIT_ASSERT_EQUAL("Format"s, aCallback.m_aLastRedlineInfo.type);
    CPPUNIT_ASSERT_EQUAL("Some comment"s, aCallback.m_aLastRedlineInfo.comment);
    CPPUNIT_ASSERT_EQUAL("Attributes changed"s, aCallback.m_aLastRedlineInfo.description);
    CPPUNIT_ASSERT_EQUAL("2025-06-17T12:41:00"s, aCallback.m_aLastRedlineInfo.dateTime);

    // Go to the 3rd tracked change: "Insert “ Sapienti sat.”"
    pDocument->postUnoCommand(".uno:NextTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();

    // Accept it
    pDocument->postUnoCommand(".uno:AcceptTrackedChange", {}, false);
    Scheduler::ProcessEventsToIdle();
    // One change gone; it is recorded "Remove"d in aCallback.m_aLastRedlineInfo
    CPPUNIT_ASSERT_EQUAL(size_t(1), getRedlineInfo(pDocument).size());
    CPPUNIT_ASSERT_EQUAL("Remove"s, aCallback.m_aLastRedlineInfo.action);
    CPPUNIT_ASSERT_EQUAL("Mike"s, aCallback.m_aLastRedlineInfo.author);
    CPPUNIT_ASSERT_EQUAL("Insert"s, aCallback.m_aLastRedlineInfo.type);
    CPPUNIT_ASSERT_EQUAL(""s, aCallback.m_aLastRedlineInfo.comment);
    CPPUNIT_ASSERT_EQUAL("Insert “ Sapienti sat.”"s, aCallback.m_aLastRedlineInfo.description);
    CPPUNIT_ASSERT_EQUAL("2025-06-17T12:41:19"s, aCallback.m_aLastRedlineInfo.dateTime);

    // Make sure that another (unrelated to redline management) editing command is not working
    pDocument->postUnoCommand(".uno:InsertAnnotation",
                                      R"({"Text":{"type":"string","value":"Comment"}})",
                                      false);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(aCallback.m_aCommentCallbackResult.empty());

    // Check that the same command would succeed in AllowChangeComments mode
    KitHelper::setAllowChangeComments(viewId, true);
    pDocument->postUnoCommand(".uno:InsertAnnotation",
                                      R"({"Text":{"type":"string","value":"Comment"}})",
                                      false);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT(!aCallback.m_aCommentCallbackResult.empty());
}

void DesktopKitTest::testCalcValidityDropdown()
{
    COKitDocumentImpl* pDocument = loadDoc("validity.ods");
    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    // Select row 1 from column 1.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Open dropdown.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1380, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1380, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Select some value from dropdown.
    pDocument->sendDialogEvent(aView.m_JSONDialog.get_child("id").get_value<int>(), "{\"id\":\"list\", \"cmd\": \"select\", \"data\": \"3\", \"type\": \"treeview\"}");
    Scheduler::ProcessEventsToIdle();

    // Activate the selected value.
    pDocument->sendDialogEvent(aView.m_JSONDialog.get_child("id").get_value<int>(), "{\"id\":\"list\", \"cmd\": \"activate\", \"data\": \"3\", \"type\": \"treeview\"}");
    Scheduler::ProcessEventsToIdle();

    // Check the content of the current cell. The selected value of the dropdown was 1. It should be 4 now.
    std::string aCellContent = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL("4"_ostr, OString(aCellContent));
}

void DesktopKitTest::testCalcValidityDropdownInReadonlyMode()
{
    COKitDocumentImpl* pDocument = loadDoc("validity.ods");
    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    int viewId = pDocument->getView();
    KitHelper::setViewReadOnly(viewId, true);
    Scheduler::ProcessEventsToIdle();
    aView.m_JSONDialog.clear();

    // Select row 1 from column 1.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Attempt to open dropdown.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1380, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1380, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Dropdown should not open in readonly mode.
    CPPUNIT_ASSERT_EQUAL(true, aView.m_JSONDialog.empty());
}

void DesktopKitTest::testPropertySettingOnFormulaBar()
{
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("formulabar.ods");
    Scheduler::ProcessEventsToIdle();

    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    // Go to A1. There are 2 words in the cell.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1000, 150, 1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Set the focus to formulabar.
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"grab_focus\", \"data\": \"null\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Select the first word.
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"textselection\", \"data\": \"0;3;0;0\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Set bold property for the selected word.
    pDocument->postUnoCommand(".uno:Bold", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(true, aView.m_stateBold);

    // Select the second word. Without the fix, this selection removes the "bold" attribute.
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"textselection\", \"data\": \"4;9;0;0\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Select the first word again.
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"textselection\", \"data\": \"0;3;0;0\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Unset bold property for the selected word.
    pDocument->postUnoCommand(".uno:Bold", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(false, aView.m_stateBold); // This line doesn't pass without the fix in this commit.
}

void DesktopKitTest::testSearchTermReset()
{
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("empty.ods");
    Scheduler::ProcessEventsToIdle();

    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    pDocument->postUnoCommand(".uno:SearchDialog", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // Send "something" as current search string (searchterm).
    pDocument->sendDialogEvent(aView.m_findReplaceDialogId, "{\"id\":\"searchterm\", \"cmd\": \"change\", \"data\": \"something\", \"type\": \"combobox\"}");
    Scheduler::ProcessEventsToIdle();

    // Press search button.
    pDocument->sendDialogEvent(aView.m_findReplaceDialogId, "{\"id\":\"search\", \"cmd\": \"click\", \"data\": \"undefined\", \"type\": \"pushbutton\"}");
    Scheduler::ProcessEventsToIdle();

    // Close the dialog.
    pDocument->sendDialogEvent(aView.m_findReplaceDialogId, "{\"id\":\"__DIALOG__\", \"cmd\": \"close\", \"data\": \"null\", \"type\": \"dialog\"}");
    Scheduler::ProcessEventsToIdle();

    // Reopen search dialog.
    pDocument->postUnoCommand(".uno:SearchDialog", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // We should have got the "searchterm" again. It should be empty. Below line doesn't pass without the changes in this commit.
    CPPUNIT_ASSERT_EQUAL(std::string(""), aView.m_searchTerm);
}

void DesktopKitTest::testWriterShapePosSizeDialog()
{
    // cool#12182: the Position and Size dialog has to resize a selected Writer shape.
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    Scheduler::ProcessEventsToIdle();

    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    // Insert a rectangle. It is selected after insertion.
    pDocument->postUnoCommand(".uno:BasicShapes.rectangle", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDrawPage->getCount());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    const awt::Size aSizeBefore = xShape->getSize();

    pDocument->postUnoCommand(".uno:TransformDialog", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // The dialog has arrived as a jsdialog.
    CPPUNIT_ASSERT(aView.m_posSizeDialogId != 0);

    // Enter a new width on the Position and Size tab and confirm with OK.
    pDocument->sendDialogEvent(aView.m_posSizeDialogId,
        "{\"id\":\"width\", \"cmd\": \"change\", \"data\": \"2\", \"type\": \"spinfield\"}");
    Scheduler::ProcessEventsToIdle();

    pDocument->sendDialogEvent(aView.m_posSizeDialogId,
        "{\"id\":\"ok\", \"cmd\": \"click\", \"data\": \"1\", \"type\": \"pushbutton\"}");
    Scheduler::ProcessEventsToIdle();

    const awt::Size aSizeAfter = xShape->getSize();
    CPPUNIT_ASSERT_MESSAGE("entered width was not applied to the shape",
                           aSizeBefore.Width != aSizeAfter.Width);
}

void DesktopKitTest::testFormulaBarAcceptButton()
{
    COKitImpl aOffice;
    COKitDocumentImpl* pDocument = loadDoc("empty.ods");
    Scheduler::ProcessEventsToIdle();

    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    ViewCallback aView(pDocument);
    Scheduler::ProcessEventsToIdle();

    // Go to A1.
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONDOWN, 1000, 150, 1, 1, 0);
    pDocument->postMouseEvent(COKitMouseEventType::BUTTONUP, 1000, 150, 1, 1, 0);
    Scheduler::ProcessEventsToIdle();

    // Set the focus to formulabar.
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"grab_focus\", \"data\": \"null\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Set selection (nothing selected).
    pDocument->sendDialogEvent(0, "{\"id\":\"sc_input_window\", \"cmd\": \"textselection\", \"data\": \"0;0;0;0\", \"type\": \"drawingarea\"}");
    Scheduler::ProcessEventsToIdle();

    // Set text.
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT, "H");
    pDocument->postWindowExtTextInputEvent(0, COKitExtTextInputType::TEXTINPUT_END, "H");
    Scheduler::ProcessEventsToIdle();

    aView.m_JSONDialog.clear();
    pDocument->postUnoCommand(".uno:AcceptFormula", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    // Client should have receive a JSDialog event for formulabar by now.

    // These lines don't pass without the fix in this commit.
    CPPUNIT_ASSERT_EQUAL(std::string("formulabar"), aView.m_JSONDialog.get_child("jsontype").get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("setText"), aView.m_JSONDialog.get_child("data").get_child("action_type").get_value<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("sc_input_window"), aView.m_JSONDialog.get_child("data").get_child("control_id").get_value<std::string>());
}

void DesktopKitTest::testRunMacro()
{
    COKitImpl aOffice;
    bool bNonExistentMacro;

    // Verify that running a non-existent macro returns false.
    // Note: testing a successful macro run requires either a system-level
    // Basic library (removed with the wizards module) or a document loaded
    // with MacroExecutionMode::ALWAYS_EXECUTE_NO_WARN which this test
    // framework doesn't easily support.
    bNonExistentMacro = aOffice.runMacro("macro:///I.Am.Not(There)");
    CPPUNIT_ASSERT(!bNonExistentMacro);
}

void DesktopKitTest::testExtractParameter()
{
    OUString aOptions(u"Language=de-DE"_ustr);
    OUString aValue = extractParameter(aOptions, u"Language");
    CPPUNIT_ASSERT_EQUAL(u"de-DE"_ustr, aValue);
    CPPUNIT_ASSERT_EQUAL(OUString(), aOptions);

    aOptions = u"Language=en-US,Something"_ustr;
    aValue = extractParameter(aOptions, u"Language");
    CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, aValue);
    CPPUNIT_ASSERT_EQUAL(u"Something"_ustr, aOptions);

    aOptions = u"SomethingElse,Language=cs-CZ"_ustr;
    aValue = extractParameter(aOptions, u"Language");
    CPPUNIT_ASSERT_EQUAL(u"cs-CZ"_ustr, aValue);
    CPPUNIT_ASSERT_EQUAL(u"SomethingElse"_ustr, aOptions);

    aOptions = u"Something1,Language=hu-HU,Something2"_ustr;
    aValue = extractParameter(aOptions, u"Language");
    CPPUNIT_ASSERT_EQUAL(u"hu-HU"_ustr, aValue);
    CPPUNIT_ASSERT_EQUAL(u"Something1,Something2"_ustr, aOptions);

    aOptions = u"Something1,Something2=blah,Something3"_ustr;
    aValue = extractParameter(aOptions, u"Language");
    CPPUNIT_ASSERT_EQUAL(OUString(), aValue);
    CPPUNIT_ASSERT_EQUAL(u"Something1,Something2=blah,Something3"_ustr, aOptions);
}

void DesktopKitTest::readFileIntoByteVector(std::u16string_view sFilename, std::vector<unsigned char> & rByteVector)
{
    rByteVector.clear();
    OUString aURL = createFileURL(sFilename);
    SvFileStream aStream(aURL, StreamMode::READ);
    rByteVector.resize(aStream.remainingSize());
    aStream.ReadBytes(rByteVector.data(), aStream.remainingSize());
}

void DesktopKitTest::testGetSignatureState_Signed()
{
    COKitDocumentImpl* pDocument = loadDoc("signed.odt");
    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    int nState = pDocument->getSignatureState();
    if (nState == 1)
    {
        // Already SignatureState::OK, then can't test the effect of trusting new CAs.
        return;
    }

    CPPUNIT_ASSERT_EQUAL(int(4), nState);

    // Adding a certificate needs XCertificateCreator, which currently only the NSS security
    // environment implements
#if USE_CRYPTO_NSS
    std::vector<unsigned char> aCertificate;
    {
        readFileIntoByteVector(u"rootCA.der", aCertificate);
        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"intermediateRootCA.der", aCertificate);
        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    nState = pDocument->getSignatureState();
    CPPUNIT_ASSERT_EQUAL(int(1), nState);
#endif
}

void DesktopKitTest::testGetSignatureState_NonSigned()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    int nState = pDocument->getSignatureState();
    CPPUNIT_ASSERT_EQUAL(int(0), nState);
}

#if 0 // broken with system nss on RHEL 7
void DesktopKitTest::testInsertCertificate_DER_ODT()
{
    // Load the document, save it into a temp file and load that file again
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "odt", nullptr));
    closeDoc();

    pDocument = loadDocUrl(maTempFile.GetURL(), COKitDocumentType::TEXT);

    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    std::vector<unsigned char> aCertificate;
    std::vector<unsigned char> aPrivateKey;

    {
        readFileIntoByteVector(u"rootCA.der", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"intermediateRootCA.der", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"certificate.der", aCertificate);
        readFileIntoByteVector(u"certificatePrivateKey.der", aPrivateKey);

        bool bResult = pDocument->insertCertificate(aCertificate.data(), int(aCertificate.size()),
                            aPrivateKey.data(), int(aPrivateKey.size()));
        CPPUNIT_ASSERT(bResult);
    }

    int nState = pDocument->getSignatureState();
    CPPUNIT_ASSERT_EQUAL(int(1), nState);
}


void DesktopKitTest::testInsertCertificate_PEM_ODT()
{
    // Load the document, save it into a temp file and load that file again
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "odt", nullptr));
    closeDoc();

    pDocument = loadDocUrl(maTempFile.GetURL(), COKitDocumentType::TEXT);

    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    std::vector<unsigned char> aCertificate;
    std::vector<unsigned char> aPrivateKey;

    {
        readFileIntoByteVector(u"test-cert-chain-1.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-2.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-3.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-signing.pem", aCertificate);
        readFileIntoByteVector(u"test-PK-signing.pem", aPrivateKey);

        bool bResult = pDocument->insertCertificate(aCertificate.data(), int(aCertificate.size()),
                            aPrivateKey.data(), int(aPrivateKey.size()));
        CPPUNIT_ASSERT(bResult);
    }

    int nState = pDocument->getSignatureState();
    CPPUNIT_ASSERT_EQUAL(int(1), nState);
}

void DesktopKitTest::testInsertCertificate_PEM_DOCX()
{
    // Load the document, save it into a temp file and load that file again
    COKitDocumentImpl* pDocument = loadDoc("blank_text.docx");
    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "docx", nullptr));
    closeDoc();

    pDocument = loadDocUrl(maTempFile.GetURL(), COKitDocumentType::TEXT);

    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    std::vector<unsigned char> aCertificate;
    std::vector<unsigned char> aPrivateKey;

    {
        readFileIntoByteVector(u"test-cert-chain-1.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-2.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-3.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-signing.pem", aCertificate);
        readFileIntoByteVector(u"test-PK-signing.pem", aPrivateKey);

        bool bResult = pDocument->insertCertificate(aCertificate.data(), int(aCertificate.size()),
                            aPrivateKey.data(), int(aPrivateKey.size()));
        CPPUNIT_ASSERT(bResult);
    }

    int nState = pDocument->getSignatureState();
    CPPUNIT_ASSERT_EQUAL(int(5), nState);
}
#endif

#if !MPL_HAVE_SUBSET
void DesktopKitTest::testSignDocument_PEM_PDF()
{
    // Load the document, save it into a temp file and load that file again
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");

    Scheduler::ProcessEventsToIdle();
    pDocument->initializeForRendering("{}");
    Scheduler::ProcessEventsToIdle();

    std::vector<unsigned char> aCertificate;
    std::vector<unsigned char> aPrivateKey;

    {
        readFileIntoByteVector(u"test-cert-chain-1.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-2.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    {
        readFileIntoByteVector(u"test-cert-chain-3.pem", aCertificate);

        bool bResult = pDocument->addCertificate(aCertificate.data(), int(aCertificate.size()));
        CPPUNIT_ASSERT(bResult);
    }

    CPPUNIT_ASSERT(pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "pdf", nullptr));

    closeDoc();

    Scheduler::ProcessEventsToIdle();

    readFileIntoByteVector(u"test-cert-signing.pem", aCertificate);
    readFileIntoByteVector(u"test-PK-signing.pem", aPrivateKey);

    COKitImpl aOffice;
    bool bResult = aOffice.signDocument(maTempFile.GetURL().toUtf8().getStr(),
                                         aCertificate.data(), int(aCertificate.size()),
                                         aPrivateKey.data(), int(aPrivateKey.size()));

    CPPUNIT_ASSERT(bResult);
}
#endif

void DesktopKitTest::testTextSelectionHandles()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    OString aText("hello"_ostr);
    CPPUNIT_ASSERT(pDocument->paste("text/plain;charset=utf-8", aText.getStr(), aText.getLength()));

    // select the inserted text
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    std::string aSelText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(aText, OString(aSelText));
    CPPUNIT_ASSERT_EQUAL("1418, 1418, 0, 275"_ostr, m_aTextSelectionStart);
    CPPUNIT_ASSERT_EQUAL("1897, 1418, 0, 275"_ostr, m_aTextSelectionEnd);

    // deselect & check
    m_aTextSelectionStart = ""_ostr;
    m_aTextSelectionEnd = ""_ostr;
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();
    aSelText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(""_ostr, OString(aSelText));
    CPPUNIT_ASSERT_EQUAL(OString(), m_aTextSelectionStart);
    CPPUNIT_ASSERT_EQUAL(OString(), m_aTextSelectionEnd);

    // select again; the positions of the selection handles have to be sent
    // again
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();
    aSelText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(aText, OString(aSelText));
    CPPUNIT_ASSERT_EQUAL("1418, 1418, 0, 275"_ostr, m_aTextSelectionStart);
    CPPUNIT_ASSERT_EQUAL("1897, 1418, 0, 275"_ostr, m_aTextSelectionEnd);
}

void DesktopKitTest::testDialogPaste()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->postUnoCommand(".uno:HyperlinkDialog", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    SfxViewShell* pViewShell = SfxViewShell::Current();
    pViewShell->GetViewFrame().GetBindings().Update();

    VclPtr<vcl::Window> pWindow(Application::GetActiveTopWindow());
    CPPUNIT_ASSERT(pWindow);

    pDocument->postWindow(pWindow->GetKitWindowId(), COKitWindowAction::PASTE,
            "{ \"MimeType\" : { \"type\" : \"string\", \"value\" : \"text/plain;charset=utf-8\" }, \"Data\" : { \"type\" : \"[]byte\", \"value\" : \"www.softwarelibre.org.bo\" } }");
    Scheduler::ProcessEventsToIdle();

    Control* pCtrlFocused = GetFocusControl(pWindow.get());
    CPPUNIT_ASSERT(pCtrlFocused);
    CPPUNIT_ASSERT_EQUAL(WindowType::COMBOBOX, pCtrlFocused->GetType());
    CPPUNIT_ASSERT_EQUAL(u"www.softwarelibre.org.bo"_ustr, pCtrlFocused->GetText());

    static_cast<SystemWindow*>(pWindow.get())->Close();
    Scheduler::ProcessEventsToIdle();
}

void DesktopKitTest::testComplexSelection()
{
    // Start with a blank text file and add contents.
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    static constexpr OString aText("hello world"_ostr);

    // Certainly not complex.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::NONE),
                         static_cast<int>(pDocument->getSelectionType()));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::NONE),
                         static_cast<int>(pDocument->getSelectionTypeAndText("", nullptr, nullptr)));

    // Paste text.
    CPPUNIT_ASSERT(pDocument->paste("text/plain;charset=utf-8", aText.getStr(), aText.getLength()));

    // No selection.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::NONE),
                         static_cast<int>(pDocument->getSelectionType()));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::NONE),
                         static_cast<int>(pDocument->getSelectionTypeAndText("", nullptr, nullptr)));

    // Paste an image.
    OUString aFileURL = createFileURL(u"paste.jpg");
    SvFileStream aImageStream(aFileURL, StreamMode::READ);
    std::vector<char> aImageContents(aImageStream.remainingSize());
    aImageStream.ReadBytes(aImageContents.data(), aImageContents.size());
    CPPUNIT_ASSERT(pDocument->paste("image/jpeg", aImageContents.data(), aImageContents.size()));

    // Now select-all.
    pDocument->postUnoCommand(".uno:SelectAll", nullptr, false);
    Scheduler::ProcessEventsToIdle();

    // Export as plain text, we should get only the text part "hello".
    std::string aSelText = pDocument->getTextSelection("text/plain;charset=utf-8", nullptr);
    CPPUNIT_ASSERT_EQUAL(aText, OString(aSelText));

    // Export as rtf, we should also get the image.
    aSelText = pDocument->getTextSelection("text/rtf", nullptr);
    CPPUNIT_ASSERT(aSelText.find(aText.getStr()) != std::string::npos); // Must have the text.
    CPPUNIT_ASSERT(aSelText.find("pict{") != std::string::npos); // Must have the image as well.

    // Export as html, we should also get the image.
    aSelText = pDocument->getTextSelection("text/html", nullptr);
    CPPUNIT_ASSERT(aSelText.find(aText.getStr()) != std::string::npos); // Must have the text.
    CPPUNIT_ASSERT(aSelText.find("<img") != std::string::npos); // Must have the image as well.

    // We expect this to be complex.
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::COMPLEX),
                         static_cast<int>(pDocument->getSelectionType()));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(COKitSelectionType::COMPLEX),
                         static_cast<int>(pDocument->getSelectionTypeAndText("", nullptr, nullptr)));
}

void DesktopKitTest::testCalcSaveAs()
{
    COKitDocumentImpl* pDocument = loadDoc("sheets.ods");
    CPPUNIT_ASSERT(pDocument);

    // Enter some text, but don't commit.
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'X', 0);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 'X', 0);
    Scheduler::ProcessEventsToIdle();

    // Save as a new file.
    pDocument->saveAs(maTempFile.GetURL().toUtf8().getStr(), "ods", nullptr);
    closeDoc();

    // Load the new document and verify that the in-flight changes are saved.
    pDocument = loadDocUrl(maTempFile.GetURL(), COKitDocumentType::SPREADSHEET);
    CPPUNIT_ASSERT(pDocument);

    ViewCallback aView(pDocument);
    pDocument->initializeForRendering("{}");
    pDocument->registerCallback(&ViewCallback::callback, &aView);

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_RIGHT);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_RIGHT);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, KEY_LEFT);
    pDocument->postKeyEvent(COKitKeyEventType::UP, 0, KEY_LEFT);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL("X"_ostr, aView.m_aCellFormula);
}

void DesktopKitTest::testSpellcheckerMultiView()
{
    static constexpr OUString aLangISO(u"en-US"_ustr);
    SvtSysLocaleOptions aSysLocaleOptions;
    aSysLocaleOptions.SetLocaleConfigString(aLangISO);
    aSysLocaleOptions.SetUILocaleConfigString(aLangISO);
    comphelper::COKit::setLanguageTag(LanguageTag(aLangISO, true));

    auto aSavedSettings = Application::GetSettings();
    comphelper::ScopeGuard g([&]() { Application::SetSettings(aSavedSettings); });
    AllSettings aSettings(aSavedSettings);
    aSettings.SetLanguageTag(aLangISO, true);
    Application::SetSettings(aSettings);

    COKitDocumentImpl* pDocument = loadDoc("sheet_with_image.ods", COKitDocumentType::SPREADSHEET);
    pDocument->setViewLanguage(0, "en-US"); // For spellchecking.
    pDocument->initializeForRendering(nullptr);
    pDocument->registerCallback(&DesktopKitTest::callback, this);

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'a', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'a', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'a', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::ESCAPE);

    // Start spellchecking.
    pDocument->postUnoCommand(".uno:SpellDialog", nullptr, false);

    // Uncommenting this will result in a deadlock.
    // Because the language configuration above is not effective, and no
    // language is actually set, the spell-dialog finds no misspelled
    // words, and displays a message box, which must be dismissed to
    // continue.
    // Need to fix the language configuration issue to enable this.
    // Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, pDocument->getViewsCount());

    // Now create another view.
    const int nViewId = pDocument->createView();
    CPPUNIT_ASSERT_EQUAL(2, pDocument->getViewsCount());

    // And destroy it.
    pDocument->destroyView(nViewId);

    // We should survive the destroyed view.
    CPPUNIT_ASSERT_EQUAL(1, pDocument->getViewsCount());
}

void DesktopKitTest::testMultiDocuments()
{
    for (int i = 0; i < 3; i++)
    {
        // Load a document.
        std::unique_ptr<COKitDocumentImpl> document1 = loadDocImpl("blank_text.odt");
        COKitDocumentImpl* pDocument1 = document1.get();
        CPPUNIT_ASSERT_EQUAL(1, pDocument1->getViewsCount());
        const int nDocId1 = pDocument1->mnDocumentId;

        const int nDoc1View0 = pDocument1->getView();
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View0));
        const int nDoc1View1 = pDocument1->createView();
        CPPUNIT_ASSERT_EQUAL(nDoc1View1, pDocument1->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View1));
        CPPUNIT_ASSERT_EQUAL(2, pDocument1->getViewsCount());

        // Validate the views of document 1.
        std::vector<int> aViewIdsDoc1(2);
        CPPUNIT_ASSERT(pDocument1->getViewIds(aViewIdsDoc1.data(), aViewIdsDoc1.size()));
        // The expectation is that the most recently used shell is at the start
        CPPUNIT_ASSERT_EQUAL(nDoc1View1, aViewIdsDoc1[0]);
        CPPUNIT_ASSERT_EQUAL(nDoc1View0, aViewIdsDoc1[1]);

        CPPUNIT_ASSERT_EQUAL(nDoc1View1, pDocument1->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View1));
        pDocument1->setView(nDoc1View0);
        CPPUNIT_ASSERT_EQUAL(nDoc1View0, pDocument1->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View0));
        pDocument1->setView(nDoc1View1);
        CPPUNIT_ASSERT_EQUAL(nDoc1View1, pDocument1->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View1));
        CPPUNIT_ASSERT_EQUAL(2, pDocument1->getViewsCount());

        // Load another document.
        std::unique_ptr<COKitDocumentImpl> document2 = loadDocImpl("blank_presentation.odp");
        COKitDocumentImpl* pDocument2 = document2.get();
        CPPUNIT_ASSERT_EQUAL(1, pDocument2->getViewsCount());
        const int nDocId2 = pDocument2->mnDocumentId;

        const int nDoc2View0 = pDocument2->getView();
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View0));
        const int nDoc2View1 = pDocument2->createView();
        CPPUNIT_ASSERT_EQUAL(nDoc2View1, pDocument2->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View1));
        CPPUNIT_ASSERT_EQUAL(2, pDocument2->getViewsCount());

        // Validate the views of document 2.
        std::vector<int> aViewIdsDoc2(2);
        CPPUNIT_ASSERT(pDocument2->getViewIds(aViewIdsDoc2.data(), aViewIdsDoc2.size()));
        // The expectation is that the most recently used shell is at the start
        CPPUNIT_ASSERT_EQUAL(nDoc2View1, aViewIdsDoc2[0]);
        CPPUNIT_ASSERT_EQUAL(nDoc2View0, aViewIdsDoc2[1]);

        CPPUNIT_ASSERT_EQUAL(nDoc2View1, pDocument2->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View1));
        pDocument2->setView(nDoc2View0);
        CPPUNIT_ASSERT_EQUAL(nDoc2View0, pDocument2->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View0));
        pDocument2->setView(nDoc2View1);
        CPPUNIT_ASSERT_EQUAL(nDoc2View1, pDocument2->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View1));
        CPPUNIT_ASSERT_EQUAL(2, pDocument2->getViewsCount());

        // The views of document1 should be unchanged.
        CPPUNIT_ASSERT(pDocument1->getViewIds(aViewIdsDoc1.data(), aViewIdsDoc1.size()));
        // The expectation is that the most recently used shell is at the start
        CPPUNIT_ASSERT_EQUAL(nDoc1View1, aViewIdsDoc1[0]);
        CPPUNIT_ASSERT_EQUAL(nDoc1View0, aViewIdsDoc1[1]);
        // Switch views in the first doc.
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View0));
        pDocument1->setView(nDoc1View0);
        CPPUNIT_ASSERT_EQUAL(nDoc1View0, pDocument1->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId1, KitHelper::getDocumentIdOfView(nDoc1View1));
        pDocument1->destroyView(nDoc1View1);
        CPPUNIT_ASSERT_EQUAL(1, pDocument1->getViewsCount());

        // The views of document2 should be unchanged.
        CPPUNIT_ASSERT(pDocument2->getViewIds(aViewIdsDoc2.data(), aViewIdsDoc2.size()));
        // The expectation is that the most recently used shell is at the start
        CPPUNIT_ASSERT_EQUAL(nDoc2View1, aViewIdsDoc2[0]);
        CPPUNIT_ASSERT_EQUAL(nDoc2View0, aViewIdsDoc2[1]);
        // Switch views in the second doc.
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View0));
        pDocument2->setView(nDoc2View0);
        CPPUNIT_ASSERT_EQUAL(nDoc2View0, pDocument2->getView());
        CPPUNIT_ASSERT_EQUAL(nDocId2, KitHelper::getDocumentIdOfView(nDoc2View1));
        pDocument2->destroyView(nDoc2View1);
        CPPUNIT_ASSERT_EQUAL(1, pDocument2->getViewsCount());

        closeDoc(document2);

        closeDoc(document1);
    }
}

void DesktopKitTest::testControlState()
{
    COKitDocumentImpl* pDocument = loadDoc("search.ods");
    pDocument->postUnoCommand(".uno:StarShapes", nullptr, false);
    TestKitCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();

    boost::property_tree::ptree aState;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    pViewShell->GetViewFrame().GetBindings().Update();
    pViewShell->GetViewFrame().GetBindings().QueryControlState(SID_ATTR_TRANSFORM_WIDTH, aState);
    CPPUNIT_ASSERT(!aState.empty());
}

void DesktopKitTest::testMetricField()
{
    COKitDocumentImpl* pDocument = loadDoc("search.ods");
    pDocument->postUnoCommand(".uno:StarShapes", nullptr, false);
    SfxChildWindow* pSideBar = TestKitCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();

    vcl::Window* pWin = pSideBar->GetWindow();
    CPPUNIT_ASSERT(pWin);

    WindowUIObject aWinUI(pWin);
    std::unique_ptr<UIObject> pUIWin(aWinUI.get_child(u"selectwidth"_ustr));
    CPPUNIT_ASSERT(pUIWin);

    StringMap aMap;
    aMap[u"VALUE"_ustr] = u"75.06"_ustr;
    pUIWin->execute(u"VALUE"_ustr, aMap);

    StringMap aRet = pUIWin->get_state();
    CPPUNIT_ASSERT_EQUAL(aMap[u"VALUE"_ustr], aRet[u"Value"_ustr]);
}

void DesktopKitTest::testJumpCursor()
{
    COKitDocumentImpl* pDocument = loadDoc("blank_text.odt");
    pDocument->initializeForRendering("{}");

    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'B', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'o', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'l', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'i', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'v', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'i', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 'a', 0);
    pDocument->postKeyEvent(COKitKeyEventType::DOWN, 0, css::awt::Key::ESCAPE);
    Scheduler::ProcessEventsToIdle();

    // There is a cursor jump to (0, 0) due to
    // mpOutlinerView->SetOutputArea( PixelToLogic( tools::Rectangle(0,0,1,1) ) );
    // when creating a comment
    ViewCallback aView1(pDocument);

    pDocument->postUnoCommand(".uno:InsertAnnotation", nullptr, true);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(!aView1.m_bZeroCursor);
}

void DesktopKitTest::testRenderSearchResult_WriterNode()
{
    constexpr const bool bDumpBitmap = false;

    COKitDocumentImpl* pDocument = loadDoc("SearchIndexResultTest.odt");
    pDocument->initializeForRendering("{}");

    Scheduler::ProcessEventsToIdle();

    unsigned char* pBuffer = nullptr;
    OString aPayload =
    "<indexing>"
        "<paragraph node_type=\"writer\" index=\"19\">ABC</paragraph>"
    "</indexing>"_ostr;

    int nWidth = 0;
    int nHeight = 0;
    size_t nByteSize = 0;

    bool bResult = pDocument->renderSearchResult(aPayload.getStr(), &pBuffer, &nWidth, &nHeight, &nByteSize);

    CPPUNIT_ASSERT(bResult);
    CPPUNIT_ASSERT(pBuffer);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(642, nWidth);
    CPPUNIT_ASSERT_EQUAL(561, nHeight);
    CPPUNIT_ASSERT_EQUAL(size_t(1440648), nByteSize);

    const sal_uInt8* pD = reinterpret_cast<const sal_uInt8*>(pBuffer);
    Bitmap aBitmap = vcl::bitmap::CreateFromData(pD, nWidth, nHeight, nWidth * 4, /*nBitsPerPixel*/32, true, true);

    if (bDumpBitmap)
    {
        SvFileStream aStream(u"~/SearchResultBitmap.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        vcl::PngImageWriter aPNGWriter(aStream);
        aPNGWriter.write(aBitmap);
    }
    CPPUNIT_ASSERT_EQUAL(tools::Long(642), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(561), aBitmap.GetSizePixel().Height());

    std::free(pBuffer);
}

void DesktopKitTest::testRenderSearchResult_CommonNode()
{
    constexpr const bool bDumpBitmap = false;

    COKitDocumentImpl* pDocument = loadDoc("SearchIndexResultShapeTest.odt");
    pDocument->initializeForRendering("{}");

    Scheduler::ProcessEventsToIdle();

    unsigned char* pBuffer = nullptr;
    OString aPayload =
    "<indexing>"
        "<paragraph node_type=\"common\" index=\"0\" object_name=\"Shape 1\" />"
    "</indexing>"_ostr;

    int nWidth = 0;
    int nHeight = 0;
    size_t nByteSize = 0;

    bool bResult = pDocument->renderSearchResult(aPayload.getStr(), &pBuffer, &nWidth, &nHeight, &nByteSize);

    CPPUNIT_ASSERT(bResult);
    CPPUNIT_ASSERT(pBuffer);

    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(192, nWidth);
    CPPUNIT_ASSERT_EQUAL(96, nHeight);
    CPPUNIT_ASSERT_EQUAL(size_t(73728), nByteSize);

    const sal_uInt8* pD = reinterpret_cast<const sal_uInt8*>(pBuffer);
    Bitmap aBitmap = vcl::bitmap::CreateFromData(pD, nWidth, nHeight, nWidth * 4, /*nBitsPerPixel*/32, true, true);

    if (bDumpBitmap)
    {
        SvFileStream aStream(u"~/SearchResultBitmap.png"_ustr, StreamMode::WRITE | StreamMode::TRUNC);
        vcl::PngImageWriter aPNGWriter(aStream);
        aPNGWriter.write(aBitmap);
    }
    CPPUNIT_ASSERT_EQUAL(tools::Long(192), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(96), aBitmap.GetSizePixel().Height());

    std::free(pBuffer);
}

static void lcl_repeatKeyStroke(COKitDocumentImpl *pDocument, int nCharCode, int nKeyCode, size_t nCount)
{
    for (size_t nCtr = 0; nCtr < nCount; ++nCtr)
    {
        pDocument->postKeyEvent(COKitKeyEventType::DOWN, nCharCode, nKeyCode);
        pDocument->postKeyEvent(COKitKeyEventType::UP, nCharCode, nKeyCode);
    }
}

void DesktopKitTest::testNoDuplicateTableSelection()
{
    COKitDocumentImpl* pDocument = loadDoc("table-selection.odt");

    // Create view 1.
    pDocument->initializeForRendering("{}");
    ViewCallback aView1(pDocument);

    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, aView1.m_nTableSelectionCount);
    CPPUNIT_ASSERT(aView1.m_bEmptyTableSelection);

    aView1.m_nTableSelectionCount = 0;
    // Go to Table1.
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, aView1.m_nTableSelectionCount);
    CPPUNIT_ASSERT(!aView1.m_bEmptyTableSelection);

    aView1.m_nTableSelectionCount = 0;
    // Move to the last row in Table1.
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 2);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(0, aView1.m_nTableSelectionCount);

    // Go outside Table1.
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, aView1.m_nTableSelectionCount);
    CPPUNIT_ASSERT(aView1.m_bEmptyTableSelection);
}

void DesktopKitTest::testMultiViewTableSelection()
{
    COKitDocumentImpl* pDocument = loadDoc("table-selection.odt");

    // Create view 1.
    pDocument->initializeForRendering("{}");
    ViewCallback aView1(pDocument);
    int nView1 = pDocument->getView();

    // Create view 2.
    pDocument->createView();
    pDocument->initializeForRendering("{}");
    ViewCallback aView2(pDocument);
    int nView2 = pDocument->getView();

    // switch to view 1.
    pDocument->setView(nView1);
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, aView1.m_nTableSelectionCount);
    CPPUNIT_ASSERT_EQUAL(1, aView2.m_nTableSelectionCount);
    CPPUNIT_ASSERT(aView1.m_bEmptyTableSelection);
    CPPUNIT_ASSERT(aView2.m_bEmptyTableSelection);

    aView1.m_nTableSelectionCount = 0;
    aView2.m_nTableSelectionCount = 0;

    pDocument->setView(nView1);
    // Go to Table1.
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(1, aView1.m_nTableSelectionCount);
    CPPUNIT_ASSERT_EQUAL(0, aView2.m_nTableSelectionCount);

    aView1.m_nTableSelectionCount = 0;
    // Switch to view 2
    pDocument->setView(nView2);
    // Go to Table2 in view 2.
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 7);
    Scheduler::ProcessEventsToIdle();
    // View1 should not get any table selection messages.
    CPPUNIT_ASSERT_EQUAL(0, aView1.m_nTableSelectionCount);
    // View2 will first get table selection of Table1, then empty selection, and finally on 7th down arrow keypress,
    // it will get table-selection of Table2. So in total it should get 3 table selections.
    CPPUNIT_ASSERT_EQUAL(3, aView2.m_nTableSelectionCount);
    CPPUNIT_ASSERT(!aView2.m_bEmptyTableSelection);

    aView1.m_nTableSelectionCount = 0;
    aView2.m_nTableSelectionCount = 0;

    // Switch to view 1
    pDocument->setView(nView1);
    // Go out of Table1 and re-enter..
    lcl_repeatKeyStroke(pDocument, 0, KEY_UP, 1);
    lcl_repeatKeyStroke(pDocument, 0, KEY_DOWN, 1);
    Scheduler::ProcessEventsToIdle();
    // View1 should get one empty table selection, then get Table1 selection.
    CPPUNIT_ASSERT_EQUAL(2, aView1.m_nTableSelectionCount);
    // View2 should not get any table selection.
    CPPUNIT_ASSERT_EQUAL(0, aView2.m_nTableSelectionCount);
    CPPUNIT_ASSERT(!aView1.m_bEmptyTableSelection);
}

void DesktopKitTest::testColorPaletteCallback()
{
    COKitDocumentImpl* pDocument = loadDoc("ThemeDocument.docx");

    // Create view 1.
    pDocument->initializeForRendering("{}");
    ViewCallback aView1(pDocument);
    Scheduler::ProcessEventsToIdle();
    {
        CPPUNIT_ASSERT_EQUAL(1, aView1.m_nColorPaletteCallbackCount);
        boost::property_tree::ptree aValues = aView1.m_aColorPaletteCallbackResult.get_child("ThemeColors");
        CPPUNIT_ASSERT(!aValues.empty());
        CPPUNIT_ASSERT_EQUAL(size_t(6), aValues.size());
    }

    // Create view 2.
    pDocument->createView();
    pDocument->initializeForRendering("{}");
    ViewCallback aView2(pDocument);
    Scheduler::ProcessEventsToIdle();
    {
        CPPUNIT_ASSERT_EQUAL(1, aView2.m_nColorPaletteCallbackCount);
        boost::property_tree::ptree aValues = aView1.m_aColorPaletteCallbackResult.get_child("ThemeColors");
        CPPUNIT_ASSERT(!aValues.empty());
        CPPUNIT_ASSERT_EQUAL(size_t(6), aValues.size());
    }
}


CPPUNIT_TEST_SUITE_REGISTRATION(DesktopKitTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
