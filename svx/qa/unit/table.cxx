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

#include <extendedprimitive2dxmldump.hxx>
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
        : UnoApiXmlTest(u"svx/qa/unit/data/"_ustr)
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
    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    //- Expected: 0
    //- Actual  : 2
    //- In <>, XPath contents of child does not match
    // i.e. the shadow's transparency was miscalculated.
    assertXPathContent(pDocument, "count(//objectinfo/unifiedtransparence)"_ostr, u"0"_ustr);

    assertXPath(pDocument, "//objectinfo/shadow[1]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[1]"_ostr, "blur"_ostr, u"141"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[2]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[2]"_ostr, "blur"_ostr, u"141"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[3]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[3]"_ostr, "blur"_ostr, u"141"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[4]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[4]"_ostr, "blur"_ostr, u"141"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[5]"_ostr, "color"_ostr, u"#ff0000"_ustr);
    assertXPath(pDocument, "//objectinfo/shadow[5]"_ostr, "blur"_ostr, u"141"_ustr);

    assertXPath(pDocument, "//objectinfo/group/sdrCell[1]/unifiedtransparence"_ostr, 0);
    assertXPath(pDocument, "//objectinfo/group/sdrCell[2]/unifiedtransparence"_ostr, 0);
    assertXPath(pDocument, "//objectinfo/group/sdrCell[3]/group/unifiedtransparence"_ostr,
                "transparence"_ostr, u"80"_ustr);
    assertXPath(pDocument, "//objectinfo/group/sdrCell[4]/group/unifiedtransparence"_ostr,
                "transparence"_ostr, u"80"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testSvxTableControllerSetAttrToSelectedShape)
{
    // Given a document with a table shape, editing cell text:
    mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr,
                                  u"com.sun.star.presentation.PresentationDocument"_ustr);
    uno::Sequence<beans::PropertyValue> aArgs
        = { comphelper::makePropertyValue(u"Rows"_ustr, sal_Int32(2)),
            comphelper::makePropertyValue(u"Columns"_ustr, sal_Int32(2)) };
    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);
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
