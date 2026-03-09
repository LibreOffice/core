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
#include <svx/svdoashp.hxx>
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
        : UnoApiXmlTest(u"svx/qa/unit/data/"_ustr)
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
    loadFromFile(u"shadow-scale-origin.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Examine the created primitives.
    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    sal_Int32 fShadowX = getXPath(pDocument, "//shadow/transform", "xy13").toInt32();
    sal_Int32 fShadowY = getXPath(pDocument, "//shadow/transform", "xy23").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -705
    // - Actual  : -158
    // i.e. the shadow origin was not the top right corner for scaling (larger x position, so it was
    // visible on the right of the shape as well).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-705), fShadowX);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-685), fShadowY);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testShadowAlignment)
{
    loadFromFile(u"tdf150020-shadow-alignment.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    {
        // Page 1 contains 9 shapes with each shadow alignment
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
        drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
            = renderPageToPrimitives(xDrawPage);

        // Examine the created primitives.
        svx::ExtendedPrimitive2dXmlDump aDumper;
        xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: -568
        // - Actual  : 162
        // - In <>, attribute 'xy13' of '(//shadow/transform)[1]' incorrect value.
        // i.e. shadow alignment was ignored while scaling the shadow.
        assertXPath(pDocument, "(//shadow/transform)[1]", "xy13", u"-568");
        assertXPath(pDocument, "(//shadow/transform)[1]", "xy23", u"162");

        assertXPath(pDocument, "(//shadow/transform)[2]", "xy13", u"-1795");
        assertXPath(pDocument, "(//shadow/transform)[2]", "xy23", u"162");

        assertXPath(pDocument, "(//shadow/transform)[3]", "xy13", u"-3021");
        assertXPath(pDocument, "(//shadow/transform)[3]", "xy23", u"161");

        assertXPath(pDocument, "(//shadow/transform)[4]", "xy13", u"-568");
        assertXPath(pDocument, "(//shadow/transform)[4]", "xy23", u"-749");

        assertXPath(pDocument, "(//shadow/transform)[5]", "xy13", u"-3021");
        assertXPath(pDocument, "(//shadow/transform)[5]", "xy23", u"-750");

        assertXPath(pDocument, "(//shadow/transform)[6]", "xy13", u"-567");
        assertXPath(pDocument, "(//shadow/transform)[6]", "xy23", u"-1692");

        assertXPath(pDocument, "(//shadow/transform)[7]", "xy13", u"-1795");
        assertXPath(pDocument, "(//shadow/transform)[7]", "xy23", u"-1693");

        assertXPath(pDocument, "(//shadow/transform)[8]", "xy13", u"-3023");
        assertXPath(pDocument, "(//shadow/transform)[8]", "xy23", u"-1692");

        assertXPath(pDocument, "(//shadow/transform)[9]", "xy13", u"-1795");
        assertXPath(pDocument, "(//shadow/transform)[9]", "xy23", u"-750");
    }
    {
        // Page 2 contains a table with shadow alignment center
        uno::Reference<drawing::XDrawPage> xDrawPage(
            xDrawPagesSupplier->getDrawPages()->getByIndex(1), uno::UNO_QUERY);
        drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
            = renderPageToPrimitives(xDrawPage);

        // Examine the created primitives.
        svx::ExtendedPrimitive2dXmlDump aDumper;
        xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: -5196
        // - Actual  : 0
        // - In<>, attribute 'xy13' of '//shadow/transform' incorrect value.
        assertXPath(pDocument, "//shadow/transform", "xy13", u"-5196");
        assertXPath(pDocument, "//shadow/transform", "xy23", u"-2290");
    }
}

CPPUNIT_TEST_FIXTURE(SdrTest, testZeroWidthTextWrap)
{
    // Load a document containing a 0-width shape with text.
    loadFromFile(u"0-width-text-wrap.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Examine the created primitives.
    svx::ExtendedPrimitive2dXmlDump aDumper;
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
    loadFromFile(u"slide-background.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // When rendering that document:
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = renderPageToPrimitives(xDrawPage);

    // Then make sure that the background has a bitmap:
    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the rendering did not find the bitmap.
    assertXPath(pDocument, "//bitmap", 1);
}

CPPUNIT_TEST_FIXTURE(SdrTest, test3DRotatedText)
{
    // The document contains a shape with text "Vertical" and a 3D scene camera rotation of 90 deg.
    // The text should appear rotated and positioned ~at the top-left of the text frame.

    loadFromFile(u"3d_rotated_text.pptx");

    // verify the camera rotation was imported correctly
    auto xDrawPages = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()->getDrawPages();
    auto xDrawPage = xDrawPages->getByIndex(0).queryThrow<drawing::XDrawPage>();
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    auto* pSdrTextObj = static_cast<SdrTextObj*>(pDrawPage->GetSdrPage()->GetObj(0));
    CPPUNIT_ASSERT(pSdrTextObj);
    CPPUNIT_ASSERT_EQUAL(90.0, pSdrTextObj->GetCameraZRotation());

    auto aPrimitives = renderPageToPrimitives(xDrawPage);
    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(aPrimitives);
    CPPUNIT_ASSERT(pDocument);

    assertXPath(pDocument, "//textsimpleportion", "text", u"Vertical");
    // x/y are the unrotated text position (in page coordinates, before the transform)
    assertXPath(pDocument, "//textsimpleportion", "x", u"7799");
    assertXPath(pDocument, "//textsimpleportion", "y", u"6303");

    assertXPath(pDocument, "//transform", 1);
    double fXY11 = getXPath(pDocument, "//transform", "xy11").toDouble();
    double fXY12 = getXPath(pDocument, "//transform", "xy12").toDouble();
    double fXY13 = getXPath(pDocument, "//transform", "xy13").toDouble();
    double fXY21 = getXPath(pDocument, "//transform", "xy21").toDouble();
    double fXY22 = getXPath(pDocument, "//transform", "xy22").toDouble();
    double fXY23 = getXPath(pDocument, "//transform", "xy23").toDouble();
    basegfx::B2DHomMatrix aTransform(fXY11, fXY12, fXY13, fXY21, fXY22, fXY23);
    basegfx::B2DTuple aScale, aTranslate;
    double fRotate, fShearX;
    aTransform.decompose(aScale, aTranslate, fRotate, fShearX);

    // no scaling, no shear
    CPPUNIT_ASSERT_EQUAL(1.0, aScale.getX());
    CPPUNIT_ASSERT_EQUAL(1.0, aScale.getY());
    CPPUNIT_ASSERT_EQUAL(0.0, fShearX);

    // The text is rotated 90 degrees counterclockwise around its unrotated center.
    CPPUNIT_ASSERT_EQUAL(-M_PI_2, fRotate);
    // The translation values reflect current state; they may change a bit (the position of the
    // rotated text is not pixel-perfect; it is about one pixel off compared to Powerpoint).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2744.0, aTranslate.getX(), 10.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(14896.0, aTranslate.getY(), 10.0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
