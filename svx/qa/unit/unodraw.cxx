/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/tempfile.hxx>
#include <svx/unopage.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <drawinglayer/tools/primitive2dxmldump.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <test/xmltesttools.hxx>

#include <sdr/contact/objectcontactofobjlistpainter.hxx>

using namespace ::com::sun::star;

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/svx/qa/unit/data/";

/// Tests for svx/source/unodraw/ code.
class UnodrawTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
protected:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
};

void UnodrawTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void UnodrawTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testWriterGraphicExport)
{
    // Load a document with a Writer picture in it.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "unodraw-writer-image.odt";
    mxComponent = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<lang::XComponent> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // Export it as JPEG.
    uno::Reference<drawing::XGraphicExportFilter> xExportFilter
        = drawing::GraphicExportFilter::create(mxComponentContext);
    // This resulted in a css::lang::IllegalArgumentException for a Writer
    // picture.
    xExportFilter->setSourceDocument(xShape);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "URL", uno::Any(aTempFile.GetURL()) },
                                           { "MediaType", uno::Any(OUString("image/jpeg")) } }));
    CPPUNIT_ASSERT(xExportFilter->filter(aProperties));
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTdf93998)
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf93998.odp");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPagesSupplier.is());

    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDrawPage.is());

    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    uno::Reference<lang::XMultiServiceFactory> xFactory = comphelper::getProcessServiceFactory();
    uno::Reference<awt::XControlModel> xModel(
        xFactory->createInstance("com.sun.star.awt.UnoControlDialogModel"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModel.is());

    uno::Reference<beans::XPropertySet> xModelProps(xModel, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xModelProps.is());

    // This resulted in a uno::RuntimeException, assigning a shape to a dialog model's image was
    // broken.
    xModelProps->setPropertyValue("ImageURL", xShape->getPropertyValue("GraphicURL"));
    uno::Reference<graphic::XGraphic> xGraphic;
    xModelProps->getPropertyValue("Graphic") >>= xGraphic;
    CPPUNIT_ASSERT(xGraphic.is());
}

CPPUNIT_TEST_FIXTURE(UnodrawTest, testTableShadowDirect)
{
    // Create an Impress document an insert a table shape.
    mxComponent = loadFromDesktop("private:factory/simpress",
                                  "com.sun.star.presentation.PresentationDocument");
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.TableShape"), uno::UNO_QUERY);
    xShape->setPosition(awt::Point(1000, 1000));
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xSupplier->getDrawPages();
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY);
    xDrawPage->add(xShape);

    // Create a red shadow on it without touching its style.
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with throwing a
    // beans.UnknownPropertyException, as shadow-as-direct-formatting on tables were not possible.
    xShapeProps->setPropertyValue("Shadow", uno::makeAny(true));
    sal_Int32 nRed = 0xff0000;
    xShapeProps->setPropertyValue("ShadowColor", uno::makeAny(nRed));
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("ShadowColor") >>= nRed);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xff0000), nRed);

    // Add text.
    uno::Reference<table::XCellRange> xTable(xShapeProps->getPropertyValue("Model"),
                                             uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByPosition(0, 0), uno::UNO_QUERY);
    xCell->setString("A1");

    // Generates drawinglayer primitives for the shape.
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

    // Check the primitives.
    drawinglayer::Primitive2dXmlDump aDumper;
    xmlDocUniquePtr pDocument = aDumper.dumpAndParse(xPrimitiveSequence);
    assertXPath(pDocument, "//shadow", /*nNumberOfNodes=*/1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. there was shadow for the cell text, while here PowerPoint-compatible output is expected,
    // which has no shadow for cell text (only for cell borders and cell background).
    assertXPath(pDocument, "//shadow//sdrblocktext", /*nNumberOfNodes=*/0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
