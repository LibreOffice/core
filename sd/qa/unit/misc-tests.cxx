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

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <vcl/scheduler.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <sfx2/request.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/eeitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <comphelper/base64.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <undo/undomanager.hxx>
#include <GraphicViewShell.hxx>
#include <sdpage.hxx>
#include <LayerTabBar.hxx>
#include <vcl/event.hxx>
#include <vcl/keycodes.hxx>
#include <svx/svdoashp.hxx>
#include <tools/gen.hxx>
#include <svx/view3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/sdmetitm.hxx>
#include <unomodel.hxx>

using namespace ::com::sun::star;

/// Impress miscellaneous tests.
class SdMiscTest : public SdModelTestBase
{
public:
    SdMiscTest()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }

    void testTdf99396();
    void testTableObjectUndoTest();
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
    void testTextColumns();
    void testTdf98839_ShearVFlipH();
    void testTdf130988();
    void testTdf131033();
    void testTdf129898LayerDrawnInSlideshow();
    void testTdf136956();
    void testTdf39519();
    void testEncodedTableStyles();
    void testTdf157117();

    CPPUNIT_TEST_SUITE(SdMiscTest);
    CPPUNIT_TEST(testTdf99396);
    CPPUNIT_TEST(testTableObjectUndoTest);
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
    CPPUNIT_TEST(testTextColumns);
    CPPUNIT_TEST(testTdf98839_ShearVFlipH);
    CPPUNIT_TEST(testTdf130988);
    CPPUNIT_TEST(testTdf131033);
    CPPUNIT_TEST(testTdf129898LayerDrawnInSlideshow);
    CPPUNIT_TEST(testTdf136956);
    CPPUNIT_TEST(testTdf39519);
    CPPUNIT_TEST(testEncodedTableStyles);
    CPPUNIT_TEST(testTdf157117);
    CPPUNIT_TEST_SUITE_END();
};

void SdMiscTest::testTdf99396()
{
    // Load the document and select the table.
    createSdImpressDoc("tdf99396.odp");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    SdPage* pPage = pViewShell->GetActualPage();
    SdrObject* pObject = pPage->GetObj(0);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pObject, pView->GetSdrPageView());

    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetUndoManager()->GetUndoActionCount());

    // Set the vertical alignment of the cells to bottom.
    sdr::table::SvxTableController* pTableController
        = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
    CPPUNIT_ASSERT(pTableController);
    SfxRequest aRequest(*pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
    pTableController->Execute(aRequest);
    // This was 0, it wasn't possible to undo a vertical alignment change.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetUndoManager()->GetUndoActionCount());
}

void SdMiscTest::testTableObjectUndoTest()
{
    // See tdf#99396 for the issue

    // Load the document and select the table.
    createSdImpressDoc("tdf99396.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    auto pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObject);
    SdrView* pView = pViewShell->GetView();
    pView->MarkObj(pTableObject, pView->GetSdrPageView());

    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    // Make sure that the undo stack is empty.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetUndoManager()->GetUndoActionCount());

    // Set horizontal and vertical adjustment during text edit.
    pView->SdrBeginTextEdit(pTableObject);
    CPPUNIT_ASSERT(pView->GetTextEditObject());
    {
        SfxRequest aRequest(*pViewShell->GetViewFrame(), SID_ATTR_PARA_ADJUST_RIGHT);
        SfxItemSet aEditAttr(pDoc->GetPool());
        pView->GetAttributes(aEditAttr);
        SfxItemSet aNewAttr(*(aEditAttr.GetPool()), aEditAttr.GetRanges());
        aNewAttr.Put(SvxAdjustItem(SvxAdjust::Right, EE_PARA_JUST));
        aRequest.Done(aNewAttr);
        const SfxItemSet* pArgs = aRequest.GetArgs();
        pView->SetAttributes(*pArgs);
    }
    const auto& pLocalUndoManager = pView->getViewLocalUndoManager();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pLocalUndoManager->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Apply attributes"), pLocalUndoManager->GetUndoActionComment());
    {
        auto pTableController
            = dynamic_cast<sdr::table::SvxTableController*>(pView->getSelectionController().get());
        CPPUNIT_ASSERT(pTableController);
        SfxRequest aRequest(*pViewShell->GetViewFrame(), SID_TABLE_VERT_BOTTOM);
        pTableController->Execute(aRequest);
    }
    // Global change "Format cell" is applied only - Change the vertical alignment to "Bottom"
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetUndoManager()->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Format cell"), pDoc->GetUndoManager()->GetUndoActionComment());

    pView->SdrEndTextEdit();

    // End of text edit, so the text edit action is added to the undo stack
    CPPUNIT_ASSERT_EQUAL(size_t(2), pDoc->GetUndoManager()->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Edit text of Table"),
                         pDoc->GetUndoManager()->GetUndoActionComment(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Format cell"), pDoc->GetUndoManager()->GetUndoActionComment(1));

    // Check that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust
            = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        CPPUNIT_ASSERT_EQUAL(int(drawing::TextVerticalAdjust_BOTTOM), static_cast<int>(eAdjust));
    }
    {
        const EditTextObject& rEdit
            = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = rParaAttribs.GetItem(EE_PARA_JUST);
        CPPUNIT_ASSERT_EQUAL(SvxAdjust::Right, pAdjust->GetAdjust());
    }

    // Now undo.
    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();

    // Undoing the last action - one left
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetUndoManager()->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Format cell"), pDoc->GetUndoManager()->GetUndoActionComment(0));

    // Check again that the result is what we expect.
    {
        uno::Reference<table::XTable> xTable = pTableObject->getTable();
        uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
        drawing::TextVerticalAdjust eAdjust
            = xCell->getPropertyValue("TextVerticalAdjust").get<drawing::TextVerticalAdjust>();
        // This failed: Undo() did not change it from drawing::TextVerticalAdjust_BOTTOM.
        CPPUNIT_ASSERT_EQUAL(int(drawing::TextVerticalAdjust_TOP), static_cast<int>(eAdjust));
    }
    {
        const EditTextObject& rEdit
            = pTableObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
        const SfxItemSet& rParaAttribs = rEdit.GetParaAttribs(0);
        auto pAdjust = rParaAttribs.GetItem(EE_PARA_JUST);
        CPPUNIT_ASSERT_EQUAL(SvxAdjust::Center, pAdjust->GetAdjust());
    }

    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetUndoManager()->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Format cell"), pDoc->GetUndoManager()->GetUndoActionComment(0));

    /*
     * now test tdf#103950 - Undo does not revert bundled font size changes for table cells
     */
    pTableObject = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    pView->MarkObj(pTableObject, pView->GetSdrPageView()); // select table
    {
        SfxRequest aRequest(*pViewShell->GetViewFrame(), SID_GROW_FONT_SIZE);
        static_cast<sd::DrawViewShell*>(pViewShell)->ExecChar(aRequest);
    }
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pDoc->GetUndoManager()->GetUndoActionCount());
    CPPUNIT_ASSERT_EQUAL(OUString("Apply attributes to Table"),
                         pDoc->GetUndoManager()->GetUndoActionComment(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Grow font size"),
                         pDoc->GetUndoManager()->GetUndoActionComment(1));
    CPPUNIT_ASSERT_EQUAL(OUString("Format cell"), pDoc->GetUndoManager()->GetUndoActionComment(2));
}

void SdMiscTest::testFillGradient()
{
    createSdImpressDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    // Insert a new page.
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->insertNewByIndex(0),
                                                 uno::UNO_SET_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
    // Create a rectangle
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());
    uno::Reference<drawing::XShape> xShape1(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xShape1, uno::UNO_QUERY_THROW);
    // Set FillStyle and FillGradient
    awt::Gradient aGradient;
    aGradient.StartColor = sal_Int32(COL_LIGHTRED);
    aGradient.EndColor = sal_Int32(COL_LIGHTGREEN);
    xPropSet->setPropertyValue("FillStyle", uno::Any(drawing::FillStyle_GRADIENT));
    xPropSet->setPropertyValue("FillGradient", uno::Any(aGradient));
    // Add the rectangle to the page.
    xShapes->add(xShape1);

    // Retrieve the shape and check FillStyle and FillGradient
    uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet2(xIndexAccess->getByIndex(0),
                                                  uno::UNO_QUERY_THROW);
    drawing::FillStyle eFillStyle;
    awt::Gradient2 aGradient2;
    CPPUNIT_ASSERT(xPropSet2->getPropertyValue("FillStyle") >>= eFillStyle);
    CPPUNIT_ASSERT_EQUAL(int(drawing::FillStyle_GRADIENT), static_cast<int>(eFillStyle));
    CPPUNIT_ASSERT(xPropSet2->getPropertyValue("FillGradient") >>= aGradient2);

    // MCGR: Use the completely imported gradient to check for correctness
    const basegfx::BColorStops aColorStops
        = model::gradient::getColorStopsFromUno(aGradient2.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, Color(aColorStops[1].getStopColor()));
}

void SdMiscTest::testTdf44774()
{
    createSdDrawDoc();
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::DrawDocShell* pDocShell = pXImpressDocument->GetDocShell();

    SfxStyleSheetBasePool* pSSPool = pDocShell->GetStyleSheetPool();

    // Create a new style with an empty name, like what happens in UI when creating a new style
    SfxStyleSheetBase& rStyleA
        = pSSPool->Make("", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);
    // Assign a new name, which does not yet set its ApiName
    rStyleA.SetName("StyleA");
    // Create another style
    SfxStyleSheetBase& rStyleB
        = pSSPool->Make("StyleB", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);
    // ... and set its parent to the first one
    rStyleB.SetParent("StyleA");

    // Now save the file and reload
    saveAndReload("draw8");
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDocShell = pXImpressDocument->GetDocShell();
    pSSPool = pDocShell->GetStyleSheetPool();

    SfxStyleSheetBase* pStyle = pSSPool->Find("StyleB", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);
    // The parent set in StyleB used to reset, because parent style's msApiName was empty
    CPPUNIT_ASSERT_EQUAL(OUString("StyleA"), pStyle->GetParent());
}

void SdMiscTest::testTdf38225()
{
    createSdImpressDoc();
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::DrawDocShell* pDocShell = pXImpressDocument->GetDocShell();

    SfxStyleSheetBasePool* pSSPool = pDocShell->GetStyleSheetPool();

    // Create a new style with a name
    pSSPool->Make("StyleWithName1", SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined);

    // Now save the file and reload
    saveAndReload("draw8");
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDocShell = pXImpressDocument->GetDocShell();
    pSSPool = pDocShell->GetStyleSheetPool();

    SfxStyleSheetBase* pStyle = pSSPool->Find("StyleWithName1", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);

    // Rename the style
    CPPUNIT_ASSERT(pStyle->SetName("StyleWithName2"));

    // Save the file and reload again
    saveAndReload("draw8");
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDocShell = pXImpressDocument->GetDocShell();
    pSSPool = pDocShell->GetStyleSheetPool();

    // The problem was that the style kept the old name upon reloading
    pStyle = pSSPool->Find("StyleWithName1", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(!pStyle);
    pStyle = pSSPool->Find("StyleWithName2", SfxStyleFamily::Para);
    CPPUNIT_ASSERT(pStyle);
}

void SdMiscTest::testTdf120527()
{
    createSdImpressDoc();

    // Load a bitmap into the bitmap table.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFactory.is());
    uno::Reference<container::XNameContainer> xBitmaps(
        xFactory->createInstance("com.sun.star.drawing.BitmapTable"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBitmaps.is());
    OUString aGraphicURL = createFileURL(u"tdf120527.jpg");
    xBitmaps->insertByName("test", uno::Any(aGraphicURL));

    // Create a graphic.
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProperySet(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapeProperySet.is());
    xShapeProperySet->setPropertyValue("GraphicURL", xBitmaps->getByName("test"));

    // Insert it.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
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
}

// Testing document model part of editengine-columns
void SdMiscTest::testTextColumns()
{
    createSdImpressDoc();
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    // Insert a new page.
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->insertNewByIndex(0),
                                                 uno::UNO_SET_THROW);
    uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);

    {
        // Create a text shape
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xFactory.is());
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance("com.sun.star.drawing.TextShape"), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xPropSet(xShape, uno::UNO_QUERY_THROW);

        // Add the shape to the page.
        xShapes->add(xShape);

        // Set up columns
        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);
        pTextObj->SetMergedItem(SfxInt16Item(SDRATTR_TEXTCOLUMNS_NUMBER, 2));
        pTextObj->SetMergedItem(SdrMetricItem(SDRATTR_TEXTCOLUMNS_SPACING, 1000));
    }

    {
        // Retrieve the shape and check columns
        uno::Reference<container::XIndexAccess> xIndexAccess(xDrawPage, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW);

        auto pTextObj = DynCastSdrTextObj(SdrObject::getSdrObjectFromXShape(xShape));
        CPPUNIT_ASSERT(pTextObj);

        CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pTextObj->GetTextColumnsSpacing());
    }
}

/// Draw miscellaneous tests.

// Since LO 6.2 the visible/printable/locked information for layers is always
// written as ODF attributes draw:display and draw:protected. It is only read from
// there, if the config items VisibleLayers, PrintableLayers and LockedLayers do
// not exist. The user option WriteLayerStateAsConfigItem can be set to 'true' to
// write these config items in addition to the ODF attributes for to produce
// documents for older LO versions or Apache OpenOffice. With value 'false' no
// config items are written. The 'testTdf101242_xyz' tests combine source
// files with and without config items with option values 'true' and 'false'.

void SdMiscTest::testTdf101242_ODF_add_settings()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the ODF attributes draw:display and draw:protected. The resaved document
    // should still have the ODF attributes and in addition the config items in settings.xml.
    // "Load" is needed for to handle layers, simple "loadURL" does not work.
    createSdDrawDoc("tdf101242_ODF.odg");

    // Saving including items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, pBatch);
    pBatch->commit();
    save("draw8");

    // Verify, that the saved document still has the ODF attributes
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    static constexpr OString sPathStart(
        "/office:document-styles/office:master-styles/draw:layer-set/draw:layer"_ostr);
    assertXPath(pXmlDoc,
                sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has got the items in settings.xml
    xmlDocUniquePtr pXmlDoc2 = parseExport("settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    static constexpr OString sPathStart2("/office:document-settings/office:settings/"
                                         "config:config-item-set[@config:name='ooo:view-settings']/"
                                         "config:config-item-map-indexed[@config:name='Views']/"
                                         "config:config-item-map-entry"_ostr);
    // Value is a bitfield with first Byte in order '* * * measurelines controls backgroundobjects background layout'
    // The first three bits depend on initialization and may change. The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x0F, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x17, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x04, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);
}

void SdMiscTest::testTdf101242_ODF_no_settings()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the ODF attributes draw:display and draw:protected. The resave document
    // should have only the ODF attributes and no config items in settings.xml.
    createSdDrawDoc("tdf101242_ODF.odg");

    // Saving without items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(false, pBatch);
    pBatch->commit();
    save("draw8");

    // Verify, that the saved document still has the ODF attributes
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    static constexpr OString sPathStart(
        "/office:document-styles/office:master-styles/draw:layer-set/draw:layer"_ostr);
    assertXPath(pXmlDoc,
                sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has no layer items in settings.xml
    xmlDocUniquePtr pXmlDoc2 = parseExport("settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    static constexpr OString sPathStart2("/office:document-settings/office:settings/"
                                         "config:config-item-set[@config:name='ooo:view-settings']/"
                                         "config:config-item-map-indexed[@config:name='Views']/"
                                         "config:config-item-map-entry"_ostr);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']", 0);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']", 0);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']", 0);
}

void SdMiscTest::testTdf101242_settings_keep()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the config items in settings.xml. That is the case for all old documents.
    // The resaved document should have the ODF attributes draw:display and draw:protected
    // and should still have these config items in settings.xml.
    createSdDrawDoc("tdf101242_settings.odg");

    // Saving including items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, pBatch);
    pBatch->commit();
    save("draw8");

    // Verify, that the saved document has the ODF attributes
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    static constexpr OString sPathStart(
        "/office:document-styles/office:master-styles/draw:layer-set/draw:layer"_ostr);
    assertXPath(pXmlDoc,
                sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document still has the items in settings.xml
    xmlDocUniquePtr pXmlDoc2 = parseExport("settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    static constexpr OString sPathStart2("/office:document-settings/office:settings/"
                                         "config:config-item-set[@config:name='ooo:view-settings']/"
                                         "config:config-item-map-indexed[@config:name='Views']/"
                                         "config:config-item-map-entry"_ostr);
    // Value is a bitfield with first Byte in order '* * * measurelines controls backgroundobjects background layout'
    // The first three bits depend on initialization and may change. The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x0F, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x17, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);

    sBase64 = getXPathContent(pXmlDoc2,
                              sPathStart2 + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x04, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0x1F);
}

void SdMiscTest::testTdf101242_settings_remove()
{
    // Loads a document, which has the visible/printable/locked information for layers
    // only in the config items in settings.xml. That is the case for all old documents.
    // The resaved document should have only the ODF attributes draw:display and draw:protected
    // and should have no config items in settings.xml.
    createSdDrawDoc("tdf101242_settings.odg");

    // Saving without config items in settings.xml
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(false, pBatch);
    pBatch->commit();
    save("draw8");

    // Verify, that the saved document has the ODF attributes
    xmlDocUniquePtr pXmlDoc = parseExport("styles.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'styles.xml'", pXmlDoc);
    static constexpr OString sPathStart(
        "/office:document-styles/office:master-styles/draw:layer-set/draw:layer"_ostr);
    assertXPath(pXmlDoc,
                sPathStart + "[@draw:name='backgroundobjects' and @draw:protected='true']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='controls' and @draw:display='screen']");
    assertXPath(pXmlDoc, sPathStart + "[@draw:name='measurelines' and @draw:display='printer']");

    // Verify, that the saved document has no layer items in settings.xml
    xmlDocUniquePtr pXmlDoc2 = parseExport("settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc2);
    static constexpr OString sPathStart2("/office:document-settings/office:settings/"
                                         "config:config-item-set[@config:name='ooo:view-settings']/"
                                         "config:config-item-map-indexed[@config:name='Views']/"
                                         "config:config-item-map-entry"_ostr);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='VisibleLayers']", 0);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='PrintableLayers']", 0);
    assertXPath(pXmlDoc2, sPathStart2 + "/config:config-item[@config:name='LockedLayers']", 0);
}

void SdMiscTest::testTdf119392()
{
    // Loads a document which has two user layers "V--" and "V-L". Inserts a new layer "-P-" between them.
    // Checks, that the bitfields in the saved file have the bits in the correct order, in case
    // option WriteLayerAsConfigItem is true and the config items are written.
    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::WriteLayerStateAsConfigItem::set(true, batch);
    batch->commit();

    createSdDrawDoc("tdf119392_InsertLayer.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    // Insert layer "-P-", not visible, printable, not locked
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrView* pView = pViewShell->GetView();
    pView->InsertNewLayer("-P-", 6); // 0..4 standard layer, 5 layer "V--"
    SdrPageView* pPageView = pView->GetSdrPageView();
    pPageView->SetLayerVisible("-P-", false);
    pPageView->SetLayerPrintable("-P-", true);
    pPageView->SetLayerLocked("-P-", false);
    save("draw8");

    // Verify correct bit order in bitfield in the config items in settings.xml
    xmlDocUniquePtr pXmlDoc = parseExport("settings.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'settings.xml'", pXmlDoc);
    static constexpr OString sPathStart("/office:document-settings/office:settings/"
                                        "config:config-item-set[@config:name='ooo:view-settings']/"
                                        "config:config-item-map-indexed[@config:name='Views']/"
                                        "config:config-item-map-entry"_ostr);
    // First Byte is in order 'V-L -P- V-- measurelines controls backgroundobjects background layout'
    // Bits need to be: visible=10111111=0xbf=191 printable=01011111=0x5f=95 locked=10000000=0x80=128
    // The values in file are Base64 encoded.
    OUString sBase64;
    uno::Sequence<sal_Int8> aDecodedSeq;
    sBase64 = getXPathContent(pXmlDoc,
                              sPathStart + "/config:config-item[@config:name='VisibleLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item VisibleLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0xbF,
                         static_cast<sal_uInt8>(aDecodedSeq[0])
                             & 0xff); // & 0xff forces unambiguous types for CPPUNIT_ASSERT_EQUAL

    sBase64 = getXPathContent(pXmlDoc,
                              sPathStart + "/config:config-item[@config:name='PrintableLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item PrintableLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x5f, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0xff);

    sBase64
        = getXPathContent(pXmlDoc, sPathStart + "/config:config-item[@config:name='LockedLayers']");
    CPPUNIT_ASSERT_MESSAGE("Item LockedLayers does not exists.", !sBase64.isEmpty());
    comphelper::Base64::decode(aDecodedSeq, sBase64);
    CPPUNIT_ASSERT_EQUAL(0x80, static_cast<sal_uInt8>(aDecodedSeq[0]) & 0xff);
}

void SdMiscTest::testTdf67248()
{
    // The document tdf67248.odg has been created with a German UI. It has a user layer named "Background".
    // On opening the user layer must still exists. The error was, that it was merged into the standard
    // layer "background".
    createSdDrawDoc("tdf67248.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), rLayerAdmin.GetLayerCount());
}

void SdMiscTest::testTdf119956()
{
    createSdDrawDoc("tdf119956.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    sd::GraphicViewShell* pGraphicViewShell = static_cast<sd::GraphicViewShell*>(pViewShell);
    CPPUNIT_ASSERT(pGraphicViewShell);
    sd::LayerTabBar* pLayerTabBar = pGraphicViewShell->GetLayerTabControl();
    CPPUNIT_ASSERT(pLayerTabBar);
    pLayerTabBar->StateChanged(StateChangedType::InitShow);

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
        aSyntheticMouseEvent
            = MouseEvent(aTabPos0Rect.Center(), 1, MouseEventModifiers::SYNTHETIC, MOUSE_LEFT, 0);
        pLayerTabBar->MouseButtonDown(aSyntheticMouseEvent);
    }
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(0), pLayerTabBar->GetCurPagePos());

    // Alt+Click on tab "Layer4"
    sal_uInt16 nIdOfTabPos3(pLayerTabBar->GetPageId(3));
    tools::Rectangle aTabPos3Rect(pLayerTabBar->GetPageRect(nIdOfTabPos3));
    aSyntheticMouseEvent = MouseEvent(aTabPos3Rect.Center(), 1, MouseEventModifiers::SYNTHETIC,
                                      MOUSE_LEFT, KEY_MOD2);
    pLayerTabBar->MouseButtonDown(aSyntheticMouseEvent);

    // Make sure, tab 3 is current tab now.
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pLayerTabBar->GetCurPagePos());
}

void SdMiscTest::testTdf98839_ShearVFlipH()
{
    // Loads a document with a sheared shape and mirrors it
    createSdDrawDoc("tdf98839_ShearVFlipH.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdPage* pPage = pViewShell->GetActualPage();
    SdrObjCustomShape* pShape = static_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    pShape->Mirror(Point(4000, 2000), Point(4000, 10000));

    // Save and examine attribute draw:transform
    save("draw8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT_MESSAGE("Failed to get 'content.xml'", pXmlDoc);
    static constexpr OString sPathStart(
        "/office:document-content/office:body/office:drawing/draw:page"_ostr);
    assertXPath(pXmlDoc, sPathStart);
    const OUString sTransform
        = getXPath(pXmlDoc, sPathStart + "/draw:custom-shape", "transform"_ostr);

    // Error was, that the shear angle had a wrong sign.
    CPPUNIT_ASSERT_MESSAGE("expected: draw:transform='skewX (-0.64350...)",
                           sTransform.startsWith("skewX (-"));
}

void SdMiscTest::testTdf130988()
{
    createSdDrawDoc("tdf130988_3D_create_lathe.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    //emulate command .uno:ConvertInto3DLathe
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    E3dView* pView = pViewShell->GetView();
    pView->MarkNextObj();
    pView->ConvertMarkedObjTo3D(false, basegfx::B2DPoint(8000.0, -3000.0),
                                basegfx::B2DPoint(3000.0, -8000.0));
    E3dScene* pObj
        = dynamic_cast<E3dScene*>(pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());
    CPPUNIT_ASSERT(pObj);

    // Error was, that the created 3D object had a wrong path. Instead examining
    // the path directly, I use the scene distance, because that is easier. The
    // scene distance is calculated from the object while creating.
    const double fDistance = pObj->GetDistance();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("D3DSceneDistance", 7071.0, fDistance, 0.5);
}

void SdMiscTest::testTdf131033()
{
    createSdDrawDoc("tdf131033_3D_SceneSizeIn2d.odg");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    // The document contains a polygon, so that emulate command .uno:ConvertInto3DLathe
    // by direct call of ConvertMarkedObjTo3D works.
    // It produces a rotation around a vertical axis, which is far away from the
    // generating shape.
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    E3dView* pView = pViewShell->GetView();
    pView->MarkNextObj();
    pView->ConvertMarkedObjTo3D(false, basegfx::B2DPoint(11000.0, -5000.0),
                                basegfx::B2DPoint(11000.0, -9000.0));
    E3dScene* pObj
        = dynamic_cast<E3dScene*>(pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());
    CPPUNIT_ASSERT(pObj);

    // Error was, that the 2D representation of the scene did not contain the default 20°
    // rotation of the new scene around x-axis and therefore was not high enough.
    const double fSnapRectHeight = pObj->GetSnapRect().getOpenHeight();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("2D height", 7096.0, fSnapRectHeight, 1.0);
}

void SdMiscTest::testTdf129898LayerDrawnInSlideshow()
{
    // Versions LO 6.2 to 6.4 have produced files, where the layer DrawnInSlideshow has
    // got visible=false and printable=false attributes. Those files should be repaired now.
    createSdImpressDoc("tdf129898_faulty_DrawnInSlideshow.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Verify model
    static constexpr OUString sName = u"DrawnInSlideshow"_ustr;
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
    SdrLayer* pLayer = rLayerAdmin.GetLayer(sName);
    CPPUNIT_ASSERT_MESSAGE("No layer DrawnInSlideshow", pLayer);
    CPPUNIT_ASSERT(pLayer->IsVisibleODF());
    CPPUNIT_ASSERT(pLayer->IsPrintableODF());

    // Verify view
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();
    SdrPageView* pPageView = pViewShell->GetView()->GetSdrPageView();
    CPPUNIT_ASSERT(pPageView->IsLayerVisible(sName));
    CPPUNIT_ASSERT(pPageView->IsLayerPrintable(sName));
}

void SdMiscTest::testTdf136956()
{
    createSdImpressDoc("odp/cellspan.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const SdrPage* pPage = pDoc->GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);

    uno::Reference<css::table::XMergeableCellRange> xRange(
        xTable->createCursorByRange(xTable->getCellRangeByPosition(0, 0, 3, 2)),
        uno::UNO_QUERY_THROW);

    // 4x3 Table before merge.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRowCount());

    xRange->merge();

    // 1x1 Table after merge.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRowCount());

    pXImpressDocument->GetDocShell()->GetUndoManager()->Undo();

    // 4x3 Table after undo. Undo crashed before.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getColumnCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRowCount());
}

void SdMiscTest::testTdf39519()
{
    createSdImpressDoc();
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);

    // Change the name of the first page in the newly created document
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    SdPage* pPage = static_cast<SdPage*>(pDoc->GetPage(1));
    pPage->SetName("Test");

    // Insert a bookmark as a new page using the same name
    std::vector<OUString> aBookmarkList = { "Test" };
    pDoc->InsertBookmarkAsPage(aBookmarkList, nullptr, false, false, 2, true, pDoc->GetDocSh(),
                               true, false, false);

    // Check if the copied page has a different name
    SdPage* pCopiedPage = static_cast<SdPage*>(pDoc->GetPage(2));
    // Without the fix in place, the names of the pages would not be different
    CPPUNIT_ASSERT(pCopiedPage->GetName() != pPage->GetName());
}

void SdMiscTest::testEncodedTableStyles()
{
    // Silence unrelated failure:
    // Error: element "table:table-template" is missing "first-row-start-column" attribute
    skipValidation();

    createSdDrawDoc();

    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY_THROW);
        uno::Reference<css::lang::XSingleServiceFactory> xTableStyleFamily(
            xStyleFamiliesSupplier->getStyleFamilies()->getByName("table"), uno::UNO_QUERY_THROW);
        uno::Reference<css::lang::XSingleServiceFactory> xCellStyleFamily(
            xStyleFamiliesSupplier->getStyleFamilies()->getByName("cell"), uno::UNO_QUERY_THROW);

        uno::Reference<style::XStyle> xTableStyle(xTableStyleFamily->createInstance(),
                                                  uno::UNO_QUERY_THROW);
        uno::Reference<style::XStyle> xCellStyle(xCellStyleFamily->createInstance(),
                                                 uno::UNO_QUERY_THROW);

        uno::Reference<container::XNameContainer>(xTableStyleFamily, uno::UNO_QUERY_THROW)
            ->insertByName("table_1", uno::Any(xTableStyle));
        uno::Reference<container::XNameContainer>(xCellStyleFamily, uno::UNO_QUERY_THROW)
            ->insertByName("table-body_1", uno::Any(xCellStyle));
        uno::Reference<container::XNameReplace>(xTableStyle, uno::UNO_QUERY_THROW)
            ->replaceByName("body", uno::Any(xCellStyle));
    }

    saveAndReload("draw8");

    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                             uno::UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xTableStyleFamily(
            xStyleFamiliesSupplier->getStyleFamilies()->getByName("table"), uno::UNO_QUERY_THROW);
        // Such style used to be exported as "table_5f_1" instead.
        CPPUNIT_ASSERT(xTableStyleFamily->hasByName("table_1"));

        uno::Reference<container::XNameAccess> xTableStyle(xTableStyleFamily->getByName("table_1"),
                                                           uno::UNO_QUERY_THROW);
        uno::Reference<style::XStyle> xCellStyle(xTableStyle->getByName("body"), uno::UNO_QUERY);
        // Such style used to not be found by the table style, as it was
        // searching for "table-body_5f_1" instead of "table-body_1".
        CPPUNIT_ASSERT(xCellStyle.is());
        CPPUNIT_ASSERT_EQUAL(OUString("table-body_1"), xCellStyle->getName());
    }
}

void SdMiscTest::testTdf157117()
{
    createSdImpressDoc();
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    sd::ViewShell* pViewShell = pXImpressDocument->GetDocShell()->GetViewShell();

    // insert two pages to make a total of 3 pages
    dispatchCommand(mxComponent, ".uno:InsertPage", {});
    dispatchCommand(mxComponent, ".uno:InsertPage", {});

    // assert the document has 3 standard pages
    SdDrawDocument* pDocument = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(3), pDocument->GetSdPageCount(PageKind::Standard));

    // alternate page insert method
    //    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    //    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    //    xDrawPages->insertNewByIndex(0);
    //    xDrawPages->insertNewByIndex(0);
    //    CPPUNIT_ASSERT_EQUAL(xDrawPages->getCount(), 3);

    // move to the last page
    dispatchCommand(mxComponent, ".uno:LastPage", {});

    SdPage* pPage = pViewShell->GetActualPage();
    auto nPageNum = pPage->GetPageNum();
    // assert move to last page
    CPPUNIT_ASSERT_EQUAL(2, (nPageNum - 1) / 2);

    // delete the last page
    dispatchCommand(mxComponent, ".uno:DeletePage", {});
    pPage = pViewShell->GetActualPage();
    nPageNum = pPage->GetPageNum();

    // Check that the new last page is moved to. Before, the first page was always moved to when
    // the last page was deleted.
    CPPUNIT_ASSERT_EQUAL(1, (nPageNum - 1) / 2);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdMiscTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
