/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/propertysequence.hxx>
#include <extendedprimitive2dxmldump.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/objsh.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/unopage.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/sdr/ code.
class SdrTest : public UnoApiTest
{
public:
    SdrTest()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
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

/// A filled shape of the given service on a new drawing, with a reflection that fades out over
/// its first quarter. The percentages are chosen so that their fractions print exactly.
uno::Reference<drawing::XShape>
insertReflectedShape(const uno::Reference<lang::XComponent>& xComponent,
                     const OUString& rServiceName)
{
    auto xFactory = xComponent.queryThrow<lang::XMultiServiceFactory>();
    auto xShape = xFactory->createInstance(rServiceName).queryThrow<drawing::XShape>();
    xShape->setPosition(awt::Point(1000, 2000));
    xShape->setSize(awt::Size(5000, 4000));
    xComponent.queryThrow<drawing::XDrawPagesSupplier>()
        ->getDrawPages()
        ->getByIndex(0)
        .queryThrow<drawing::XShapes>()
        ->add(xShape);

    auto xProperties = xShape.queryThrow<beans::XPropertySet>();
    xProperties->setPropertyValue(u"ReflectionEffect"_ustr, uno::Any(true));
    xProperties->setPropertyValue(u"ReflectionEffectDistance"_ustr, uno::Any(sal_Int32(100)));
    xProperties->setPropertyValue(u"ReflectionEffectBlurRadius"_ustr, uno::Any(sal_Int32(50)));
    xProperties->setPropertyValue(u"ReflectionEffectStartTransparency"_ustr,
                                  uno::Any(sal_Int16(50)));
    xProperties->setPropertyValue(u"ReflectionEffectEndPosition"_ustr, uno::Any(sal_Int16(25)));
    return xShape;
}

/// A reflected rectangle that is a custom shape, like the shapes of a PPTX import.
uno::Reference<drawing::XShape>
insertReflectedRectangle(const uno::Reference<lang::XComponent>& xComponent)
{
    uno::Reference<drawing::XShape> xShape
        = insertReflectedShape(xComponent, u"com.sun.star.drawing.CustomShape"_ustr);
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(
        u"CustomShapeGeometry"_ustr, uno::Any(comphelper::InitPropertySequence(
                                         { { u"Type"_ustr, uno::Any(u"rectangle"_ustr) } })));
    return xShape;
}

/// The mask a shape is clipped with, wherever it sits in the primitives of a page.
const drawinglayer::primitive2d::MaskPrimitive2D*
findMask(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitives)
{
    for (const auto& rPrimitive : rPrimitives)
    {
        if (auto pMask
            = dynamic_cast<const drawinglayer::primitive2d::MaskPrimitive2D*>(rPrimitive.get()))
            return pMask;
        if (auto pGroup
            = dynamic_cast<const drawinglayer::primitive2d::GroupPrimitive2D*>(rPrimitive.get()))
            if (auto pMask = findMask(pGroup->getChildren()))
                return pMask;
    }
    return nullptr;
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-684), fShadowY);
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
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[1]", "xy13", -567.8, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[1]", "xy23", 162.3, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[2]", "xy13", -1794.55, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[2]", "xy23", 162.3, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[3]", "xy13", -3021.3, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[3]", "xy23", 161.1, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[4]", "xy13", -567.8, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[4]", "xy23", -749.25, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[5]", "xy13", -3021.3, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[5]", "xy23", -750.45, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[6]", "xy13", -566.6, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[6]", "xy23", -1691.7, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[7]", "xy13", -1794.55, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[7]", "xy23", -1693.0, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[8]", "xy13", -3022.5, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[8]", "xy23", -1691.7, 0.001);

        assertXPathDoubleValue(pDocument, "(//shadow/transform)[9]", "xy13", -1794.55, 0.001);
        assertXPathDoubleValue(pDocument, "(//shadow/transform)[9]", "xy23", -750.45, 0.001);
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
        assertXPathDoubleValue(pDocument, "//shadow/transform", "xy13", -5196.45, 0.001);
        assertXPathDoubleValue(pDocument, "//shadow/transform", "xy23", -2290.05, 0.001);
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
    // Given a document with a slide that has a linked background image:
    loadFromFile(u"slide-background.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // Allow link updates and resolve the linked background image via the
    // LinkManager before rendering, so createNewSdrFillGraphicAttribute
    // sees a Bitmap and not a deferred GraphicType::Default item. The link
    // was registered per style sheet by the model's FillBitmapLinkTracker as
    // the background was set on the Background style during load.
    SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pShell);
    pShell->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(true);
    auto* pSvxDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pSvxDrawPage);
    SdrModel& rModel = pSvxDrawPage->GetSdrPage()->getSdrModelFromSdrPage();
    sfx2::LinkManager* pLinkMgr = rModel.GetLinkManager();
    CPPUNIT_ASSERT(pLinkMgr);
    pLinkMgr->UpdateAllLinks(false, u""_ustr);

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

CPPUNIT_TEST_FIXTURE(SdrTest, testMediaLinkNotFetchedWhenUpdatesDisallowed)
{
    // A media object referenced by an external xlink:href, with no copy of
    // the media stored inside the document.
    loadFromFile(u"media-link.fodp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    auto* pSvxDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pSvxDrawPage);
    auto* pMedia = dynamic_cast<SdrMediaObj*>(pSvxDrawPage->GetSdrPage()->GetObj(0));
    CPPUNIT_ASSERT(pMedia);

    // With link updates disallowed, as when loading or converting a document
    // without a user present to approve them, asking for the placeholder image
    // must not reach out to the external URL. Without the fix the snapshot is
    // generated (an empty placeholder bitmap even where no media backend is built,
    // and the referenced content where one is), so the returned graphic is set.
    SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pShell);
    pShell->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(false);
    CPPUNIT_ASSERT(!pMedia->getSnapshot().is());

#if HAVE_FEATURE_AVMEDIA
    // Once updates are allowed the same request produces a graphic, so it was
    // the permission alone that blocked the fetch above.
    pShell->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(true);
    CPPUNIT_ASSERT(pMedia->getSnapshot().is());
#endif
}

#if HAVE_FEATURE_AVMEDIA
CPPUNIT_TEST_FIXTURE(SdrTest, testMediaLinkPickedByUserIsFetched)
{
    loadFromFile(u"media-link.fodp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    auto* pSvxDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pSvxDrawPage);
    auto* pMedia = dynamic_cast<SdrMediaObj*>(pSvxDrawPage->GetSdrPage()->GetObj(0));
    CPPUNIT_ASSERT(pMedia);

    // The document as a whole may not update its links.
    SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pShell);
    pShell->getEmbeddedObjectContainer().setUserAllowsLinkUpdate(false);

    // A media file the user picks during the session is playable straight away, so the
    // placeholder image is fetched even though the document-wide permission is off.
    pMedia->setLinkAllowed(true);
    CPPUNIT_ASSERT(pMedia->getSnapshot().is());
}
#endif

CPPUNIT_TEST_FIXTURE(SdrTest, testMediaSettingsKeptWithoutPlayer)
{
    // The media file does not exist, so no player can play it.
    loadFromFile(u"media-link.fodp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    auto* pSvxDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pSvxDrawPage);
    SdrPage* pSdrPage = pSvxDrawPage->GetSdrPage();
    auto* pMedia = dynamic_cast<SdrMediaObj*>(pSdrPage->GetObj(0));
    CPPUNIT_ASSERT(pMedia);
    pMedia->setLinkAllowed(true);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    xShape->setPropertyValue(u"Loop"_ustr, uno::Any(true));
    xShape->setPropertyValue(u"Mute"_ustr, uno::Any(true));

    // Painting the page into a window gives the shape a media window.
    VclPtrInstance<WorkWindow> pWindow(nullptr, WB_STDWORK);
    pWindow->SetOutputSizePixel(Size(500, 500));
    pWindow->Show();
    SdrView aView(pSdrPage->getSdrModelFromSdrPage(), pWindow->GetOutDev());
    aView.ShowSdrPage(pSdrPage);
    aView.CompleteRedraw(pWindow->GetOutDev(), vcl::Region(pMedia->GetCurrentBoundRect()));
    Scheduler::ProcessEventsToIdle();

    // Without the fix in place, this test would have failed, as the media window without a
    // player reported loop and mute as off, and the shape took that over.
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"Loop"_ustr).get<bool>());
    CPPUNIT_ASSERT(xShape->getPropertyValue(u"Mute"_ustr).get<bool>());
    aView.HideSdrPage();
    pWindow.disposeAndClear();
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

CPPUNIT_TEST_FIXTURE(SdrTest, testGraphicClipPolyPolygonIsPainted)
{
    // Given a graphic shape whose clip polygon is a frame with a hole in it:
    loadFromURL(u"private:factory/sdraw"_ustr);
    auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
    auto xShape = xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr)
                      .queryThrow<drawing::XShape>();
    xShape->setPosition(awt::Point(1000, 2000));
    xShape->setSize(awt::Size(5000, 4000));
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();
    xDrawPage.queryThrow<drawing::XShapes>()->add(xShape);

    drawing::PolyPolygonBezierCoords aClip;
    aClip.Coordinates
        = { { awt::Point(0, 0), awt::Point(5000, 0), awt::Point(5000, 4000), awt::Point(0, 4000) },
            { awt::Point(1000, 1000), awt::Point(1000, 3000), awt::Point(4000, 3000),
              awt::Point(4000, 1000) } };
    aClip.Flags = { { drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL,
                      drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL },
                    { drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL,
                      drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL } };
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(u"GraphicClipPolyPolygon"_ustr,
                                                               uno::Any(aClip));

    // Then it is painted through that polygon. Without this the shape covered its whole frame, and
    // a clip the model held travelled from file to file without ever being drawn.
    const drawinglayer::primitive2d::MaskPrimitive2D* pMask
        = findMask(renderPageToPrimitives(xDrawPage));
    CPPUNIT_ASSERT(pMask);

    // Both contours reach the mask, or there would be no hole to see through.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), pMask->getMask().count());
    // The polygon states the shape's own coordinates; on the page it sits where the shape does.
    const basegfx::B2DRange aRange(pMask->getMask().getB2DRange());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, aRange.getMinX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2000.0, aRange.getMinY(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6000.0, aRange.getMaxX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6000.0, aRange.getMaxY(), 1.0);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testGraphicClipPolyPolygonFollowsAFlip)
{
    // Given a graphic shape clipped to the left fifth of its frame:
    loadFromURL(u"private:factory/sdraw"_ustr);
    auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
    auto xShape = xFactory->createInstance(u"com.sun.star.drawing.GraphicObjectShape"_ustr)
                      .queryThrow<drawing::XShape>();
    xShape->setPosition(awt::Point(1000, 2000));
    xShape->setSize(awt::Size(5000, 4000));
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();
    xDrawPage.queryThrow<drawing::XShapes>()->add(xShape);

    drawing::PolyPolygonBezierCoords aClip;
    aClip.Coordinates = { { awt::Point(0, 0), awt::Point(1000, 0), awt::Point(1000, 4000),
                            awt::Point(0, 4000) } };
    aClip.Flags = { { drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL,
                      drawing::PolygonFlags_NORMAL, drawing::PolygonFlags_NORMAL } };
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(u"GraphicClipPolyPolygon"_ustr,
                                                               uno::Any(aClip));

    // When the shape is flipped about its own vertical centre:
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
    CPPUNIT_ASSERT(pObject);
    pObject->Mirror(Point(3500, 2000), Point(3500, 6000));

    // Then the clip goes with it, and covers the right fifth. A flip reaches the graphic through
    // its own attributes and not the object matrix, so a mask that stayed behind showed the picture
    // turned round inside an outline that had not moved.
    const drawinglayer::primitive2d::MaskPrimitive2D* pMask
        = findMask(renderPageToPrimitives(xDrawPage));
    CPPUNIT_ASSERT(pMask);
    const basegfx::B2DRange aRange(pMask->getMask().getB2DRange());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5000.0, aRange.getMinX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6000.0, aRange.getMaxX(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2000.0, aRange.getMinY(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6000.0, aRange.getMaxY(), 1.0);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfShape)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    uno::Reference<drawing::XShape> xShape = insertReflectedRectangle(mxComponent);
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // The item values arrive in the reflection, and the percentages become fractions.
    assertXPath(pDocument, "//reflection", 1);
    assertXPath(pDocument, "//reflection", "distance", u"100");
    assertXPath(pDocument, "//reflection", "blurradius", u"50");
    assertXPath(pDocument, "//reflection", "starttransparency", u"0.5");
    assertXPath(pDocument, "//reflection", "startposition", u"0");
    assertXPath(pDocument, "//reflection", "endtransparency", u"1");
    assertXPath(pDocument, "//reflection", "endposition", u"0.25");

    // Turning the reflection off keeps the other values but removes the reflection.
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(u"ReflectionEffect"_ustr,
                                                               uno::Any(false));
    pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));
    assertXPath(pDocument, "//reflection", 0);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfShapeHasNoShadow)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    uno::Reference<drawing::XShape> xShape = insertReflectedRectangle(mxComponent);
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(u"Shadow"_ustr, uno::Any(true));
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // The shape casts a shadow, but its reflection shows the shape without it.
    assertXPath(pDocument, "//shadow", 1);
    assertXPath(pDocument, "//reflection", 1);
    assertXPath(pDocument, "//reflection//shadow", 0);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfLegacyRectangle)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    insertReflectedShape(mxComponent, u"com.sun.star.drawing.RectangleShape"_ustr);
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // A rectangle that is not a custom shape shows its reflection too.
    assertXPath(pDocument, "//reflection", 1);
    assertXPath(pDocument, "//reflection", "endposition", u"0.25");
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfLegacyEllipse)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    insertReflectedShape(mxComponent, u"com.sun.star.drawing.EllipseShape"_ustr);
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // An ellipse that is not a custom shape shows its reflection too.
    assertXPath(pDocument, "//reflection", 1);
    assertXPath(pDocument, "//reflection", "endposition", u"0.25");
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfCopiedLegacyRectangle)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    insertReflectedShape(mxComponent, u"com.sun.star.drawing.RectangleShape"_ustr);
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();

    // The copy replaces the original, so the page holds only the copy.
    rtl::Reference<SdrObject> pCopy
        = pSdrPage->GetObj(0)->CloneSdrObject(pSdrPage->getSdrModelFromSdrPage());
    pSdrPage->InsertObject(pCopy.get());
    pSdrPage->RemoveObject(0);

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // The copy of the rectangle shows the reflection like the original.
    assertXPath(pDocument, "//reflection", 1);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testGlowAndSoftEdgeOfCustomShapeAreDrawnOnce)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    auto xFactory = mxComponent.queryThrow<lang::XMultiServiceFactory>();
    auto xShape = xFactory->createInstance(u"com.sun.star.drawing.CustomShape"_ustr)
                      .queryThrow<drawing::XShape>();
    xShape->setPosition(awt::Point(1000, 2000));
    xShape->setSize(awt::Size(5000, 4000));
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();
    xDrawPage->add(xShape);

    // The smiley is made of several parts: the face, the eyes and the mouth.
    auto xProperties = xShape.queryThrow<beans::XPropertySet>();
    xProperties->setPropertyValue(
        u"CustomShapeGeometry"_ustr,
        uno::Any(comphelper::InitPropertySequence({ { u"Type"_ustr, uno::Any(u"smiley"_ustr) } })));
    xProperties->setPropertyValue(u"GlowEffectRadius"_ustr, uno::Any(sal_Int32(200)));
    xProperties->setPropertyValue(u"SoftEdgeRadius"_ustr, uno::Any(sal_Int32(100)));

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // The glow and the soft edge go around the whole smiley, and not around each part again.
    // The glow holds a copy of the softened smiley, which is drawn again on top of the glow.
    assertXPath(pDocument, "//glow", 1);
    assertXPath(pDocument, "//glow//glow", 0);
    assertXPath(pDocument, "//glow//softedge", 1);
    assertXPath(pDocument, "//softedge", 2);
    assertXPath(pDocument, "//softedge//softedge", 0);
}

CPPUNIT_TEST_FIXTURE(SdrTest, testReflectionOfCustomShapeIsDrawnOnce)
{
    loadFromURL(u"private:factory/sdraw"_ustr);
    uno::Reference<drawing::XShape> xShape
        = insertReflectedShape(mxComponent, u"com.sun.star.drawing.CustomShape"_ustr);
    xShape.queryThrow<beans::XPropertySet>()->setPropertyValue(
        u"CustomShapeGeometry"_ustr,
        uno::Any(comphelper::InitPropertySequence({ { u"Type"_ustr, uno::Any(u"smiley"_ustr) } })));
    auto xDrawPage = mxComponent.queryThrow<drawing::XDrawPagesSupplier>()
                         ->getDrawPages()
                         ->getByIndex(0)
                         .queryThrow<drawing::XDrawPage>();

    svx::ExtendedPrimitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(renderPageToPrimitives(xDrawPage));

    // The smiley has one reflection, and its parts have none of their own.
    assertXPath(pDocument, "//reflection", 1);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
