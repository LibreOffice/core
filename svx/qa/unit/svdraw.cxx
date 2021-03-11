/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <rtl/ustring.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>
#include <vcl/virdev.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdview.hxx>
#include <svx/unoapi.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/svdraw/ code.
class SvdrawTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
protected:
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override
    {
        test::BootstrapFixture::setUp();
        mxDesktop.set(frame::Desktop::create(m_xContext));
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            mxComponent->dispose();
        }
        test::BootstrapFixture::tearDown();
    }
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

CPPUNIT_TEST_FIXTURE(SvdrawTest, testSemiTransparentText)
{
    // Create a new Draw document with a rectangle.
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    xShape->setPosition(awt::Point(1000, 1000));

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Add semi-transparent text on the rectangle.
    uno::Reference<text::XTextRange> xShapeText(xShape, uno::UNO_QUERY);
    xShapeText->getText()->setString("hello");

    uno::Reference<beans::XPropertySet> xShapeProperties(xShape, uno::UNO_QUERY);
    xShapeProperties->setPropertyValue("CharColor", uno::makeAny(COL_RED));
    sal_Int16 nTransparence = 75;
    xShapeProperties->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));

    // Generates drawinglayer primitives for the page.
    auto pDrawPage = dynamic_cast<SvxDrawPage*>(xDrawPage.get());
    CPPUNIT_ASSERT(pDrawPage);
    SdrPage* pSdrPage = pDrawPage->GetSdrPage();
    ScopedVclPtrInstance<VirtualDevice> aVirtualDevice;
    sdr::contact::ObjectContactOfObjListPainter aObjectContact(*aVirtualDevice,
                                                               { pSdrPage->GetObj(0) }, nullptr);
    const sdr::contact::ViewObjectContact& rDrawPageVOContact
        = pSdrPage->GetViewContact().GetViewObjectContact(aObjectContact);
    sdr::contact::DisplayInfo aDisplayInfo;
    drawinglayer::primitive2d::Primitive2DContainer xPrimitiveSequence
        = rDrawPageVOContact.getPrimitive2DSequenceHierarchy(aDisplayInfo);

    // Make sure the text is semi-transparent.
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//unifiedtransparence' number of nodes is incorrect
    // i.e. the text was just plain red, not semi-transparent.
    double fTransparence = getXPath(pDocument, "//unifiedtransparence", "transparence").toDouble();
    CPPUNIT_ASSERT_EQUAL(nTransparence,
                         static_cast<sal_Int16>(basegfx::fround(fTransparence * 100)));
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testHandlePathObjScale)
{
    // Given a path object:
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.ClosedBezierShape"), uno::UNO_QUERY);

    // When setting its scale by both using setSize() and scaling in a transform matrix:
    // Set size and basic properties.
    xShape->setPosition(awt::Point(2512, 6062));
    xShape->setSize(awt::Size(112, 112));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_SOLID));
    xShapeProps->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_SOLID));
    xShapeProps->setPropertyValue("FillColor", uno::makeAny(static_cast<sal_Int32>(0)));
    // Add it to the draw page.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    xDrawPage->add(xShape);
    // Set polygon coordinates.
    drawing::PolyPolygonBezierCoords aPolyPolygonBezierCoords;
    aPolyPolygonBezierCoords.Coordinates = {
        {
            awt::Point(2624, 6118),
            awt::Point(2624, 6087),
            awt::Point(2599, 6062),
            awt::Point(2568, 6062),
            awt::Point(2537, 6062),
            awt::Point(2512, 6087),
            awt::Point(2512, 6118),
            awt::Point(2512, 6149),
            awt::Point(2537, 6175),
            awt::Point(2568, 6174),
            awt::Point(2599, 6174),
            awt::Point(2625, 6149),
            awt::Point(2624, 6118),
        },
    };
    aPolyPolygonBezierCoords.Flags = {
        {
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_CONTROL,
            drawing::PolygonFlags_NORMAL,
        },
    };
    xShapeProps->setPropertyValue("PolyPolygonBezier", uno::makeAny(aPolyPolygonBezierCoords));
    drawing::HomogenMatrix3 aMatrix;
    aMatrix.Line1.Column1 = 56;
    aMatrix.Line2.Column1 = -97;
    aMatrix.Line3.Column1 = 0;
    aMatrix.Line1.Column2 = 97;
    aMatrix.Line2.Column2 = 56;
    aMatrix.Line3.Column2 = 0;
    aMatrix.Line1.Column3 = 3317;
    aMatrix.Line2.Column3 = 5583;
    aMatrix.Line3.Column3 = 1;
    xShapeProps->setPropertyValue("Transformation", uno::makeAny(aMatrix));

    // Then make sure the scaling is only applied once:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 113
    // - Actual  : 12566
    // i.e. the scaling was applied twice.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(113), xShape->getSize().Width);
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testTextEditEmptyGrabBag)
{
    // Given a document with a groupshape, which has 2 children.
    getComponent() = loadFromDesktop("private:factory/sdraw");
    uno::Reference<lang::XMultiServiceFactory> xFactory(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xRect1(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xRect1->setPosition(awt::Point(1000, 1000));
    xRect1->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShape> xRect2(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xRect2->setPosition(awt::Point(1000, 1000));
    xRect2->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XShapes> xGroup(
        xFactory->createInstance("com.sun.star.drawing.GroupShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xGroupShape(xGroup, uno::UNO_QUERY);
    xDrawPage->add(xGroupShape);
    xGroup->add(xRect1);
    xGroup->add(xRect2);
    uno::Reference<text::XTextRange> xRect2Text(xRect2, uno::UNO_QUERY);
    xRect2Text->setString("x");
    uno::Sequence<beans::PropertyValue> aGrabBag = {
        comphelper::makePropertyValue("OOXLayout", true),
    };
    uno::Reference<beans::XPropertySet> xGroupProps(xGroup, uno::UNO_QUERY);
    xGroupProps->setPropertyValue("InteropGrabBag", uno::makeAny(aGrabBag));

    // When editing the shape text of the 2nd rectangle (insert a char at the start).
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SdrView* pSdrView = pViewShell->GetDrawView();
    SdrObject* pObject = GetSdrObjectFromXShape(xRect2);
    pSdrView->SdrBeginTextEdit(pObject);
    EditView& rEditView = pSdrView->GetTextEditOutlinerView()->GetEditView();
    rEditView.InsertText("y");
    pSdrView->SdrEndTextEdit();

    // Then make sure that grab-bag is empty to avoid losing the new text.
    xGroupProps->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: !aGrabBag.hasElements()
    // i.e. the grab-bag was still around after modifying the shape, and that grab-bag contained the
    // old text.
    CPPUNIT_ASSERT(!aGrabBag.hasElements());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
