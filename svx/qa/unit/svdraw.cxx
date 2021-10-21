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
#include <sal/log.hxx>

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

    // Then make sure that grab-bag is empty to avoid loosing the new text.
    xGroupProps->getPropertyValue("InteropGrabBag") >>= aGrabBag;
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: !aGrabBag.hasElements()
    // i.e. the grab-bag was still around after modifying the shape, and that grab-bag contained the
    // old text.
    CPPUNIT_ASSERT(!aGrabBag.hasElements());
}

CPPUNIT_TEST_FIXTURE(SvdrawTest, testAutoHeightMultiColShape)
{
    // Given a document containing a shape that has:
    // 1) automatic height (resize shape to fix text)
    // 2) multiple columns (2)
    OUString aURL
        = m_directories.getURLFromSrc(u"svx/qa/unit/data/auto-height-multi-col-shape.pptx");

    // When loading that document:
    getComponent().set(loadFromDesktop(aURL, "com.sun.star.presentation.PresentationDocument"));

    // Make sure the in-file shape height is kept, even if nominally the the shape height is
    // automatic:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6882
    // - Actual  : 3452
    // i.e. the shape height was smaller than expected, leading to a 2 columns layout instead of
    // laying out all the text in the first column.
    // 2477601 is from slide1.xml, <a:ext cx="4229467" cy="2477601"/>, 360 is emu -> mm100.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(static_cast<sal_Int32>(2477601 / 360), xShape->getSize().Height,
                                 1);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
