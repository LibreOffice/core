/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <officecfg/Office/Common.hxx>
#include "sdmodeltestbase.hxx"

#include <vcl/svapp.hxx>
#include <sddll.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XLoadable.hpp>

#include <vcl/scheduler.hxx>
#include <osl/thread.hxx>
#include <FactoryIds.hxx>
#include <sdmod.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <ImpressViewShellBase.hxx>
#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <controller/SlsPageSelector.hxx>
#include <undo/undomanager.hxx>
#include <GraphicViewShell.hxx>
#include <chrono>
#include <sdpage.hxx>
#include <comphelper/base64.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <LayerTabBar.hxx>
#include <vcl/window.hxx>
#include <vcl/event.hxx>
#include <vcl/keycodes.hxx>


using namespace ::com::sun::star;

/// Impress miscellaneous tests.
class SdMiscTest : public SdModelTestBaseXML
{
public:
    void testTdf96206();
    void testTdf96708();
    void testTdf99396();
    void testTdf99396TextEdit();
    void testFillGradient();
    void testTdf44774();
    void testTdf38225();
    void testTdf101242_ODF_no_settings();
    void testTdf101242_ODF_add_settings();
    void testTdf101242_settings_keep();
    void testTdf101242_settings_remove();
    void testTdf119392();
    void testTdf67248();
    void testTdf119956();
    void testTdf120527();

    CPPUNIT_TEST_SUITE(SdMiscTest);
    CPPUNIT_TEST(testTdf96206);
    CPPUNIT_TEST(testTdf96708);
    CPPUNIT_TEST(testTdf99396);
    CPPUNIT_TEST(testTdf99396TextEdit);
    CPPUNIT_TEST(testFillGradient);
    CPPUNIT_TEST(testTdf44774);
    CPPUNIT_TEST(testTdf38225);
    CPPUNIT_TEST(testTdf101242_ODF_no_settings);
    CPPUNIT_TEST(testTdf101242_ODF_add_settings);
    CPPUNIT_TEST(testTdf101242_settings_keep);
    CPPUNIT_TEST(testTdf101242_settings_remove);
    CPPUNIT_TEST(testTdf119392);
    CPPUNIT_TEST(testTdf67248);
    CPPUNIT_TEST(testTdf119956);
    CPPUNIT_TEST(testTdf120527);
    CPPUNIT_TEST_SUITE_END();

virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override
    {
        static const struct { char const * pPrefix; char const * pURI; } namespaces[] =
        {
            // ODF
            { "config", "urn:oasis:names:tc:opendocument:xmlns:config:1.0"},
            { "draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" },
            { "fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" },
            { "loext", "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0" },
            { "office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0" },
            { "style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0" },
            { "svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" },
            { "text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0" },
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(namespaces); ++i)
        {
            xmlXPathRegisterNs(pXmlXPathCtx,
                reinterpret_cast<xmlChar const *>(namespaces[i].pPrefix),
                reinterpret_cast<xmlChar const *>(namespaces[i].pURI));
        }
    }

private:
    sd::DrawDocShellRef Load(const OUString& rURL, sal_Int32 nFormat);
};

sd::DrawDocShellRef SdMiscTest::Load(const OUString& rURL, sal_Int32 nFormat)
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    // create a frame
    uno::Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame("_blank", 0);
    CPPUNIT_ASSERT(xTargetFrame.is());

    // This ContainerWindow corresponds to the outermost window of a running LibreOffice.
    // It needs a non-zero size and must be shown. Otherwise visible elements like the
    // LayerTabBar in Draw have zero size and cannot get mouse events.
    // The here used size is freely chosen.
    uno::Reference<awt::XWindow> xContainerWindow = xTargetFrame->getContainerWindow();
    CPPUNIT_ASSERT(xContainerWindow.is());
    xContainerWindow->setPosSize(0, 0, 1024, 768, awt::PosSize::SIZE);
    VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow(xContainerWindow);
    CPPUNIT_ASSERT(pContainerWindow);
    pContainerWindow->Show(true);

    // 1. Open the document
    sd::DrawDocShellRef xDocSh = loadURL(rURL, nFormat);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocSh.is());

    uno::Reference< frame::XModel2 > xModel2(xDocSh->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel2.is());

    uno::Reference< frame::XController2 > xController(xModel2->createDefaultViewController(xTargetFrame), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xController.is());

    // introduce model/view/controller to each other
    xController->attachModel(xModel2.get());
    xModel2->connectController(xController.get());
    xTargetFrame->setComponent(xController->getComponentWindow(), xController.get());
    xController->attachFrame(xTargetFrame);
    xModel2->setCurrentController(xController.get());

    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    CPPUNIT_ASSERT(pViewShell);

    // Draw has no slidesorter, Impress never shows a LayerTabBar
    if (sd::ViewShell::ST_DRAW == pViewShell->GetShellType())
    {
        sd::LayerTabBar* pLayerTabBar = static_cast<sd::GraphicViewShell*>(pViewShell)->GetLayerTabControl();
        CPPUNIT_ASSERT(pLayerTabBar);
        pLayerTabBar->StateChanged(StateChangedType::InitShow);
    }
    else
    {
        sd::slidesorter::SlideSorterViewShell* pSSVS = nullptr;
        for (int i = 0; i < 1000; i++)
        {
            // Process all Tasks - slide sorter is created here
            while (Scheduler::ProcessTaskScheduling());
            if ((pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase())) != nullptr)
                break;
            osl::Thread::wait(std::chrono::milliseconds(100));
        }
        CPPUNIT_ASSERT(pSSVS);
    }

    return xDocSh;
}

void SdMiscTest::testTdf96206()
{
    // Copying/pasting slide referring to a non-default master with a text duplicated the master

    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf96206.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& rSSController = pSSVS->GetSlideSorter().GetController();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(2), nMasterPageCnt1);
    rSSController.GetClipboard().DoCopy();
    rSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

void SdMiscTest::testTdf96708()
{
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/odp/tdf96708.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    auto pSSVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(pViewShell->GetViewShellBase());
    auto& rSSController = pSSVS->GetSlideSorter().GetController();
    auto& rPageSelector = rSSController.GetPageSelector();

    const sal_uInt16 nMasterPageCnt1 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), nMasterPageCnt1);
    rPageSelector.SelectAllPages();
    rSSController.GetClipboard().DoCopy();

    // Now wait for timers to trigger creation of auto-layout
    osl::Thread::wait(std::chrono::milliseconds(100));
    Scheduler::ProcessTaskScheduling();

    rSSController.GetClipboard().DoPaste();
    const sal_uInt16 nMasterPageCnt2 = xDocSh->GetDoc()->GetMasterSdPageCount(PageKind::Standard);
    CPPUNIT_ASSERT_EQUAL(nMasterPageCnt1, nMasterPageCnt2);

    xDocSh->DoClose();
}

void SdMiscTest::testTdf99396()
{
    // Load the document and select the table.
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf99396.odp"), ODP);
    sd::ViewShell *pViewShell = xDocSh->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    // Set the vertical alignment of the cells to bottom.
    sdr::table::SvxTableController* pTableController = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
    CPPUNIT_ASSERT(pTableController);
    SfxRequest aRequest(pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
    pTableController->Execute(aRequest);
    // This was 0, it wasn't possible to undo a vertical alignment change.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    xDocSh->DoClose();
}

void SdMiscTest::testTdf99396TextEdit()
{
    // Load the document and select the table.
    sd::DrawDocShellRef xDocSh = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf99396.odp"), ODP);
    sd::ViewShell* pViewShell = xDocSh->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTableObject, pView->GetSdrPageView());

    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());

    // Set horizontal and vertical adjustment during text edit.
    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    {
        SfxRequest aRequest(pViewShell->GetViewFrame(), SID_ATTR_PARA_ADJUST_RIGHT);
        SfxItemSet aEditAttr(xDocSh->GetDoc()->GetPool());
        pView->GetAttributes(aEditAttr);
        SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
        aNewAttr.Put(SvxAdjustItem(SvxAdjust::Right, EE_PARA_JUST));
        aRequest.Done(aNewAttr);
        const SfxItemSet* pArgs = aRequest.GetArgs();
        pView->SetAttributes(*pArgs);
    }
    {
        auto pTableController = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
        CPPUNIT_ASSERT(pTableController);
        SfxRequest aRequest(pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
        pTableController->Execute(aRequest);
    }
    pView->SdrEndTextEdit();

    // Check that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        CPPUNIT_ASSERT_EQUAL(int(drawing::TextVerticalAdjust_BOTTOM), static_cast<int>(eAdjust));
    }
    {
        const EditTextObject& rEdit = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = rParaAttribs.GetItem(EE_PARA_JUST);
        CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, pAdjust->GetAdjust());
    }

    // Now undo.
    xDocSh->GetUndoManager()->Undo();

    // Check again that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        // This failed: Undo() did not change it from drawing::TextVerticalAdjust_BOTTOM.
        CPPUNIT_ASSERT_EQUAL(int(drawing::TextVerticalAdjust_TOP), static_cast<int>(eAdjust));
    }
    {
        const EditTextObject& rEdit = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = rParaAttribs.GetItem(EE_PARA_JUST);
        CPPUNIT_ASSERT_EQUAL(SvxAdjust::Center, pAdjust->GetAdjust());
    }


    /*
     * now test tdf#103950 - Undo does not revert bundled font size changes for table cells
     */
    pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    pView->MarkObj(pTableObject, pView->GetSdrPageView()); // select table
    {
        SfxRequest aRequest(pViewShell->GetViewFrame(), SID_GROW_FONT_SIZE);
        static_cast<sd::DrawViewShell*>(pViewShell)->ExecChar(aRequest);
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), xDocSh->GetDoc()->GetUndoManager()->GetUndoActionCount());


    xDocSh->DoClose();
}

void SdMiscTest::testFillGradient()
{
    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Impress);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier = getDoc( xDocShRef );
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    // Insert a new page.
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->insertNewByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference<drawing::XShapes> xShapes(xDrawPage,uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiServiceFactory> const xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);
    // Create a rectangle
    uno::Reference<drawing::XShape> xShape1(xDoc->createInstance("com.sun.star.drawing.RectangleShape"),uno::UNO_QUERY_THROW );
    uno::Reference<beans::XPropertySet> xPropSet(xShape1, uno::UNO_QUERY_THROW);
    // Set FillStyle and FillGradient
    awt::Gradient aGradient;
    aGradient.StartColor = sal_Int32(Color(255, 0, 0));
    aGradient.EndColor = sal_Int32(Color(0, 255, 0));
    xPropSet->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_GRADIENT));
    xPropSet->setPropertyValue("FillGradient", uno::makeAny(aGradient));
    // Add the rectangle to the page.
    xShapes->add(xShape1);

    // Retrieve the shape and check FillStyle and FillGradient
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet > xPropSet2(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);
    drawing::FillStyle eFillStyle;
    awt::Gradient aGradient2;
    CPPUNIT_ASSERT(xPropSet2->getPropertyValue("FillStyle") >>= eFillStyle);
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(eFillStyle));
    CPPUNIT_ASSERT(xPropSet2->getPropertyValue("FillGradient") >>= aGradient2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(Color(255, 0, 0)),aGradient2.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(Color(0, 255, 0)),aGradient2.EndColor);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf44774()
{
    sd::DrawDocShellRef xDocShRef = new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false,
        DocumentType::Draw);
    const uno::Reference<frame::XLoadable> xLoadable(xDocShRef->GetModel(), uno::UNO_QUERY_THROW);
    xLoadable->initNew();
    SfxStyleSheetBasePool* pSSPool = xDocShRef->GetStyleSheetPool();

    // Create a new style with an empty name, like what happens in UI when creating a new style
    SfxStyleSheetBase& rStyleA = pSSPool->Make("", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);
    // Assign a new name, which does not yet set its ApiName
    rStyleA.SetName("StyleA");
    // Create another style
    SfxStyleSheetBase& rStyleB = pSSPool->Make("StyleB", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);
    // ... and set its parent to the first one
    rStyleB.SetParent("StyleA");

    // Now save the file and reload
    xDocShRef = saveAndReload(xDocShRef.get(), ODG);
    pSSPool = xDocShRef->GetStyleSheetPool();

    SfxStyleSheetBase* pStyle = pSSPool->Find("StyleB", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);
    // The parent set in StyleB used to reset, because parent style's msApiName was empty
    CPPUNIT_ASSERT_EQUAL(OUString("StyleA"), pStyle->GetParent());

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf38225()
{
    sd::DrawDocShellRef xDocShRef = new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false,
        DocumentType::Draw);
    const uno::Reference<frame::XLoadable> xLoadable(xDocShRef->GetModel(), uno::UNO_QUERY_THROW);
    xLoadable->initNew();
    SfxStyleSheetBasePool* pSSPool = xDocShRef->GetStyleSheetPool();

    // Create a new style with a name
    pSSPool->Make("StyleWithName1", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);

    // Now save the file and reload
    xDocShRef = saveAndReload(xDocShRef.get(), ODG);
    pSSPool = xDocShRef->GetStyleSheetPool();

    SfxStyleSheetBase* pStyle = pSSPool->Find("StyleWithName1", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);

    // Rename the style
    CPPUNIT_ASSERT(pStyle->SetName("StyleWithName2"));

    // Save the file and reload again
    xDocShRef = saveAndReload(xDocShRef.get(), ODG);
    pSSPool = xDocShRef->GetStyleSheetPool();

    // The problem was that the style kept the old name upon reloading
    pStyle = pSSPool->Find("StyleWithName1", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(!pStyle);
    pStyle = pSSPool->Find("StyleWithName2", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf120527()
{
    sd::DrawDocShellRef xDocShRef
        = new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Draw);
    uno::Reference<frame::XLoadable> xLoadable(xDocShRef->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLoadable.is());
    xLoadable->initNew();

    // Load a bitmap into the bitmap table.
    uno::Reference<lang::XMultiServiceFactory> xFactory(xDocShRef->GetModel(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBitmaps.is());
    OUString aGraphicURL = m_directories.getURLFromSrc("/sd/qa/unit/data/tdf120527.jpg");
    xBitmaps->insertByName("test", uno::makeAny(aGraphicURL));

    // Create a graphic.
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperySet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProperySet.is());
    xShapeProperySet->setPropertyValue("GraphicURL", xBitmaps->getByName("test"));

    // Insert it.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xDocShRef->GetModel(),
                                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPage.is());
    // This failed with a lang.IllegalArgumentException.
    xDrawPage->add(xShape);

    // Verify that the graphic was actually consumed.
    uno::Reference<graphic::XGraphic> xGraphic;
    xShapeProperySet->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());

    xDocShRef->DoClose();
}

/// Draw miscellaneous tests.

// Since LO 6.2 the visible/printable/locked information for layers is always
// written as ODF attributes draw:display and draw:protected. It is only read from
// there, if the config items VisibleLayers, PrintableLayers and LockedLayers do
// not exist. The user option WriteLayerStateAsConfigItem can be set to 'true' to
// write these config items in addition to the ODF attributes for to produce
// documents for older LO versions or Apache OpenOffice. With value 'false' no
// config items are written. The 'testTdf101242_xyz' tests combinate source
// files with and without config items with option values 'true' and 'false'.

void SdMiscTest::testTdf101242_ODF_add_settings()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the ODF attributes draw:display and draw:protected. The resaved document
    // should still have the ODF attributes and in addition the config items in settings.xml.
    // "Load" is needed for to handle layers, simple "loadURL" does not work.
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf101242_ODF.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());

    // Saving including items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, pBatch);
    pBatch->commit();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    save(xDocShRef.get(), getFormat(ODG), aTempFile );

    // Verify, that the saved document still has the ODF attributes
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    const OString sPathStart("/office:document-styles/office:master-styles/draw:layer-set/draw:layer");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has got the items in settings.xml
    xmlDocPtr pXmlDoc2 = parseExport(aTempFile, "settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    const OString sPathStart2("/office:document-settings/office:settings/config:config-item-set[@config:name='ooo:view-settings']/config:config-item-map-indexed[@config:name='Views']/config:config-item-map-entry");
    // Value is a bitfield with first Byte in order '* * * measurelines controls backgroundobjects background layout'
    // The first three bits depend on initialization and may change. The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x0F, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F );

    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x17, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x04, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf101242_ODF_no_settings()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the ODF attributes draw:display and draw:protected. The resave document
    // should have only the ODF attributes and no config items in settings.xml.
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf101242_ODF.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());

    // Saving without items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(false, pBatch);
    pBatch->commit();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    save(xDocShRef.get(), getFormat(ODG), aTempFile );

    // Verify, that the saved document still has the ODF attributes
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    const OString sPathStart("/office:document-styles/office:master-styles/draw:layer-set/draw:layer");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has no layer items in settings.xml
    xmlDocPtr pXmlDoc2 = parseExport(aTempFile, "settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    const OString sPathStart2("/office:document-settings/office:settings/config:config-item-set[@config:name='ooo:view-settings']/config:config-item-map-indexed[@config:name='Views']/config:config-item-map-entry");
    xmlXPathObjectPtr pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf101242_settings_keep()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the config items in settings.xml. That is the case for all old documents.
    // The resaved document should have the ODF attributes draw:display and draw:protected
    // and should still have these config items in settings.xml.
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf101242_settings.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());

    // Saving including items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, pBatch);
    pBatch->commit();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    save(xDocShRef.get(), getFormat(ODG), aTempFile );

    // Verify, that the saved document has the ODF attributes
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    const OString sPathStart("/office:document-styles/office:master-styles/draw:layer-set/draw:layer");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document still has the items in settings.xml
    xmlDocPtr pXmlDoc2 = parseExport(aTempFile, "settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    const OString sPathStart2("/office:document-settings/office:settings/config:config-item-set[@config:name='ooo:view-settings']/config:config-item-map-indexed[@config:name='Views']/config:config-item-map-entry");
    // Value is a bitfield with first Byte in order '* * * measurelines controls backgroundobjects background layout'
    // The first three bits depend on initialization and may change. The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x0F, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F );

    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x17, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x04, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf101242_settings_remove()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the config items in settings.xml. That is the case for all old documents.
    // The resaved document should have only the ODF attributes draw:display and draw:protected
    // and should have no config items in settings.xml.
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("/sd/qa/unit/data/tdf101242_settings.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());

    // Saving without config items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(false, pBatch);
    pBatch->commit();
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    save(xDocShRef.get(), getFormat(ODG), aTempFile );

    // Verify, that the saved document has the ODF attributes
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    const OString sPathStart("/office:document-styles/office:master-styles/draw:layer-set/draw:layer");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has no layer items in settings.xml
    xmlDocPtr pXmlDoc2 = parseExport(aTempFile, "settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    const OString sPathStart2("/office:document-settings/office:settings/config:config-item-set[@config:name='ooo:view-settings']/config:config-item-map-indexed[@config:name='Views']/config:config-item-map-entry");
    xmlXPathObjectPtr pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);
    pXmlObj=getXPathNode(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_EQUAL(0, xmlXPathNodeSetGetLength(pXmlObj->nodesetval));
    xmlXPathFreeObject(pXmlObj);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf119392()
{
    // Loads a document which has two user layers "V--" and "V-L". Inserts a new layer "-P-" between them.
    // Checks, that the bitfields in the saved file have the bits in the correct order, in case
    // option WriteLayerAsConfigItem is true and the config items are written.
    std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, batch);
    batch->commit();

    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("sd/qa/unit/data/tdf119392_InsertLayer.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());
    // Insert layer "-P-", not visible, printable, not locked
    SdrView* pView = xDocShRef -> GetViewShell()->GetView();
    pView -> InsertNewLayer("-P-", 6); // 0..4 standard layer, 5 layer "V--"
    SdrPageView* pPageView = pView -> GetSdrPageView();
    pPageView -> SetLayerVisible("-P-", false);
    pPageView -> SetLayerPrintable("-P-", true);
    pPageView -> SetLayerLocked("-P-", false);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    save(xDocShRef.get(), getFormat(ODG), aTempFile );

    // Verify correct bit order in bitfield in the config items in settings.xml
    xmlDocPtr pXmlDoc = parseExport(aTempFile, "settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc);
    const OString sPathStart("/office:document-settings/office:settings/config:config-item-set[@config:name='ooo:view-settings']/config:config-item-map-indexed[@config:name='Views']/config:config-item-map-entry");
    // First Byte is in order 'V-L -P- V-- measurelines controls backgroundobjects background layout'
    // Bits need to be: visible=10111111=0xbf=191 printable=01011111=0x5f=95 locked=10000000=0x80=128
    // The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc, sPathStart + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0xbF, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0xff); // & 0xff forces unambiguous types for CPPUNIT_ASSERT_EQUAL

    sBase64 = getXPathContent(pXmlDoc, sPathStart + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x5f, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0xff);

    sBase64 = getXPathContent(pXmlDoc, sPathStart + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE( "Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL( 0x80, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0xff);

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf67248()
{
    // The document tdf67248.odg has been created with a German UI. It has a user layer named "Background".
    // On opening the user layer must still exists. The error was, that it was merged into the standard
    // layer "background".
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("sd/qa/unit/data/tdf67248.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());
    SdrLayerAdmin& rLayerAdmin = xDocShRef->GetDoc()->GetLayerAdmin();
    CPPUNIT_ASSERT_EQUAL( sal_uInt16(6), rLayerAdmin.GetLayerCount());

    xDocShRef->DoClose();
}

void SdMiscTest::testTdf119956()
{
    sd::DrawDocShellRef xDocShRef = Load(m_directories.getURLFromSrc("sd/qa/unit/data/tdf119956.odg"), ODG);
    CPPUNIT_ASSERT_MESSAGE("Failed to load file.", xDocShRef.is());
    sd::GraphicViewShell* pGraphicViewShell = static_cast<sd::GraphicViewShell*>(xDocShRef -> GetViewShell());
    CPPUNIT_ASSERT(pGraphicViewShell);
    sd::LayerTabBar* pLayerTabBar = pGraphicViewShell->GetLayerTabControl();
    CPPUNIT_ASSERT(pLayerTabBar);

    // Alt+Click sets a tab in edit mode, so that you can rename it.
    // The error was, that Alt+Click on a tab, which was not the current tab, did not set the clicked tab
    // as current tab. As a result, the entered text was applied to the wrong tab.

    // The test document has the layer tabs "layout", "controls", "measurelines" and "Layer4" in this order
    // The "pagePos" is 0, 1, 2, 3
    // Make sure, that tab "layout" is the current tab.
    MouseEvent aSyntheticMouseEvent;
    if (pLayerTabBar->GetCurPagePos() != 0)
    {
        sal_uInt16 nIdOfTabPos0(pLayerTabBar->GetPageId(0));
        tools::Rectangle aTabPos0Rect(pLayerTabBar->GetPageRect(nIdOfTabPos0));
        aSyntheticMouseEvent = MouseEvent(aTabPos0Rect.Center(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0);
        pLayerTabBar->MouseButtonDown(aSyntheticMouseEvent);
    }
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pLayerTabBar->GetCurPagePos());

    // Alt+Click on tab "Layer4"
    sal_uInt16 nIdOfTabPos3(pLayerTabBar->GetPageId(3));
    tools::Rectangle aTabPos3Rect(pLayerTabBar->GetPageRect(nIdOfTabPos3));
    aSyntheticMouseEvent = MouseEvent(aTabPos3Rect.Center(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, KEY_MOD2);
    pLayerTabBar->MouseButtonDown(aSyntheticMouseEvent);

    // Make sure, tab 3 is current tab now.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pLayerTabBar->GetCurPagePos());

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdMiscTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
