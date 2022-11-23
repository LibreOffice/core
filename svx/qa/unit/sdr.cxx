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

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/sdr/ code.
class SdrTest : public UnoApiXmlTest
{
public:
    SdrTest()
        : UnoApiXmlTest("svx/qa/unit/data/")
    {
    }

    drawinglayer::primitive2d::Primitive2DContainer
    renderPageToPrimitives(const uno::Reference<drawing::XDrawPage>& xDrawPage);
};

drawinglayer::primitive2d::Primitive2DContainer
SdrTest::renderPageToPrimitives(const uno::Reference<drawing::XDrawPage>& xDrawPage)
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

CPPUNIT_TEST_FIXTURE(SdrTest, testShadowScaleOrigin)
{
    // Load a document containing a custom shape.
    loadFromURL(u"shadow-scale-origin.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Examine the created primitives.
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    sal_Int32 fShadowX = getXPath(pDocument, "//shadow/transform", "xy13").toInt32();
    sal_Int32 fShadowY = getXPath(pDocument, "//shadow/transform", "xy23").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -705
    // - Actual  : -158
    // i.e. the shadow origin was not the top right corner for scaling (larger x position, so it was
    // visible on the right of the shape as well).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-705), fShadowX);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-684), fShadowY);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testZeroWidthTextWrap)
{
    // Load a document containing a 0-width shape with text.
    loadFromURL(u"0-width-text-wrap.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Examine the created primitives.
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 12
    // i.e. the text on the only shape on the slide had 12 lines, not a single one.
    assertXPath(pDocument, "//textsimpleportion", 1);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testSlideBackground)
{
    // Given a document with a slide what has a linked background image:
    loadFromURL(u"slide-background.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // When rendering that document:
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Then make sure that the background has a bitmap:
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the rendering did not find the bitmap.
    assertXPath(pDocument, "//bitmap", 1);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
