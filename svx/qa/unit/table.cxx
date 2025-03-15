/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <rtl/ustring.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>
#include <vcl/virdev.hxx>
#include <sdr/contact/objectcontactofobjlistpainter.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/table/tablecontroller.hxx>
#include <editeng/editobj.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/table/ code.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("svx/qa/unit/data/")
    {
    }

    drawinglayer::primitive2d::Primitive2DContainer
    renderPageToPrimitives(const uno::Reference<drawing::XDrawPage>& xDrawPage);
};

drawinglayer::primitive2d::Primitive2DContainer
Test::renderPageToPrimitives(const uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pSdrPage->GetObj(0) }, nullptr);
    const sdr::contact::ViewObjectContact& rDrawPageVOContact
        = pSdrPage->GetViewContact().GetViewObjectContact(aObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer aContainer;
    rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo, aContainer);
    return aContainer;
}

CPPUNIT_TEST_FIXTURE(Test, testTableShadowBlur)
{
    // Given a document containing a table with a blurry shadow:
    loadFromFile(u"table-shadow-blur.pptx");

    // When rendering the table shadow to primitives:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Then make sure that the cell fill part of the shadow has the expected transparency:
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    //- Expected: 0
    //- Actual  : 2
    //- In <>, XPath contents of child does not match
    // i.e. the shadow's transparency was miscalculated.
    assertXPathContent(pDocument, "count(//objectinfo/unifiedtransparence)"_ostr, "0");

    assertXPath(pDocument, "//objectinfo/shadow[1]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "//objectinfo/shadow[1]"_ostr, "blur"_ostr, "141");
    assertXPath(pDocument, "//objectinfo/shadow[2]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "//objectinfo/shadow[2]"_ostr, "blur"_ostr, "141");
    assertXPath(pDocument, "//objectinfo/shadow[3]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "//objectinfo/shadow[3]"_ostr, "blur"_ostr, "141");
    assertXPath(pDocument, "//objectinfo/shadow[4]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "//objectinfo/shadow[4]"_ostr, "blur"_ostr, "141");
    assertXPath(pDocument, "//objectinfo/shadow[5]"_ostr, "color"_ostr, "#ff0000");
    assertXPath(pDocument, "//objectinfo/shadow[5]"_ostr, "blur"_ostr, "141");

    assertXPath(pDocument, "//objectinfo/group/sdrCell[1]/unifiedtransparence"_ostr, 0);
    assertXPath(pDocument, "//objectinfo/group/sdrCell[2]/unifiedtransparence"_ostr, 0);
    assertXPath(pDocument, "//objectinfo/group/sdrCell[3]/unifiedtransparence"_ostr,
                "transparence"_ostr, "80");
    assertXPath(pDocument, "//objectinfo/group/sdrCell[4]/unifiedtransparence"_ostr,
                "transparence"_ostr, "80");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165521_fixedCellHeight)
{
    // Given a document containing a table whose size/text should match the textbox:
    // (In this Liberation Sans unit test, the text is too small and doesn't fill the table,
    // while in the bug report's Marianne font example, the text is too large, spilling out of it.)

    loadFromFile(u"tdf165521_fixedCellHeight.pptx");

    // When rendering the table:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // TODO: Then make sure the text in both table and textbox are the same line height
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    const OString sTextboxPath7
        = "/primitive2D/objectinfo[1]/unhandled/sdrblocktext/texthierarchyblock/texthierarchyparagraph/texthierarchyline[7]/textsimpleportion"_ostr;
    CPPUNIT_ASSERT(
        getXPath(pDocument, sTextboxPath7, "text"_ostr)
            .startsWith("Autofit Custom Shape forces Fixed Cell Height even in the table."));
    // const sal_Int32 nTextBoxFontLineHeight = getXPath(pDocument, sTextboxPath7, "y"_ostr).toInt32()
    //                                          - getXPath(pDocument, sTextboxPath6, "y"_ostr).toInt32();

    const OString sTablePath6
        = "/primitive2D/objectinfo[2]/sdrCell[2]/sdrblocktext/texthierarchyblock/texthierarchyparagraph/texthierarchyline[6]/textsimpleportion"_ostr;
    const OString sTablePath7
        = "/primitive2D/objectinfo[2]/sdrCell[2]/sdrblocktext/texthierarchyblock/texthierarchyparagraph/texthierarchyline[7]/textsimpleportion"_ostr;
    CPPUNIT_ASSERT(getXPath(pDocument, sTablePath7, "text"_ostr).startsWith("Qualcomm Snapdragon"));
    const sal_Int32 nTableFontLineHeight = getXPath(pDocument, sTablePath7, "y"_ostr).toInt32()
                                           - getXPath(pDocument, sTablePath6, "y"_ostr).toInt32();
    // Expected: 593.
    // Actual (without the acompanying patch) 553
    // CPPUNIT_ASSERT_EQUAL(nTextBoxFontLineHeight, nTableFontLineHeight);

    // All of the text must fit inside the table,
    // and the table must be approximately the same size as the lines of text.
    sal_Int32 nTableHeight
        = getXPath(pDocument,
                   "/primitive2D/objectinfo[2]/sdrCell[2]/polypolygoncolor/polypolygon"_ostr,
                   "height"_ostr)
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7 * nTableFontLineHeight, nTableHeight, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSvxTableControllerSetAttrToSelectedShape)
{
    // Given a document with a table shape, editing cell text:
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue("Rows", sal_Int32(2)),
            comphelper::makePropertyValue("Columns", sal_Int32(2)) };
    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    auto pSdrObject
        = dynamic_cast<sdr::table::SdrTableObj*>(pSdrPage->GetObj(pSdrPage->GetObjCount() - 1));
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    pSdrView->SdrBeginTextEdit(pSdrObject);
    CPPUNIT_ASSERT(pSdrView->IsTextEdit());
    const EditTextObject& rEdit = pSdrObject->getText(0)->GetOutlinerParaObject()->GetTextObject();
    SfxItemSet aSet(rEdit.GetParaAttribs(0));
    auto pTableController
        = dynamic_cast<sdr::table::SvxTableController*>(pSdrView->getSelectionController().get());

    // When applying attributes which only affect the cell text, not the table shape:
    pTableController->SetAttrToSelectedShape(aSet);

    // Then make sure the text edit is not ended:
    CPPUNIT_ASSERT(pSdrView->IsTextEdit());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
