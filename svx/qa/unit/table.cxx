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
    loadFromURL(u"table-shadow-blur.pptx");

    // When rendering the table shadow to primitives:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Then make sure that the cell fill part of the shadow is excluded from blurring:
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    // - number of nodes is incorrect
    // - Expected: 1
    // - Actual  : 0
    // i.e. the shadow itself was not transparent and that resulted in a non-transparent rendering
    // as well, while the rendering transparency should be based on the transparency of the shadow
    // itself and the transparency of the cell fill.
    assertXPath(pDocument, "//objectinfo/unifiedtransparence[1]", "transparence", "80");
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
