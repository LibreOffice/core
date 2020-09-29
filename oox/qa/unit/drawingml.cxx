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

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets one child of xShape, which one is specified by nIndex.
uno::Reference<drawing::XShape> getChildShape(const uno::Reference<drawing::XShape>& xShape,
                                              sal_Int32 nIndex)
{
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());

    CPPUNIT_ASSERT(xGroup->getCount() > nIndex);

    uno::Reference<drawing::XShape> xRet(xGroup->getByIndex(nIndex), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRet.is());

    return xRet;
}
}

/// oox drawingml tests.
class OoxDrawingmlTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void load(const OUString& rURL);
    void loadAndReload(const OUString& rURL, const OUString& rFilterName);
};

void OoxDrawingmlTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void OoxDrawingmlTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void OoxDrawingmlTest::load(const OUString& rURL) { mxComponent = loadFromDesktop(rURL); }

void OoxDrawingmlTest::loadAndReload(const OUString& rURL, const OUString& rFilterName)
{
    load(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    utl::TempFile aTempFile;
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    validate(aTempFile.GetFileName(), test::OOXML);
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

char const DATA_DIRECTORY[] = "/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTransparentText)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "transparent-text.pptx";
    loadAndReload(aURL, "Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraph(
        xShape->createEnumeration()->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xParagraph->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);

    sal_Int16 nTransparency = 0;
    xPortion->getPropertyValue("CharTransparence") >>= nTransparency;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 75
    // - Actual  : 0
    // i.e. the transparency of the character color was lost on import/export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(75), nTransparency);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTdf131082)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf131082.pptx";
    loadAndReload(aURL, "Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> XPropSet(getChildShape(getChildShape(xShape, 0), 0),
                                                 uno::UNO_QUERY);

    drawing::FillStyle eFillStyle = drawing::FillStyle_NONE;
    XPropSet->getPropertyValue("FillStyle") >>= eFillStyle;

    // Without the accompanying fix in place, this test would have failed with:
    // with drawing::FillStyle_NONE - 0
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, eFillStyle);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testPresetAdjustValue)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "preset-adjust-value.pptx";

    load(aURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aGeoPropSeq;
    xShapeProps->getPropertyValue("CustomShapeGeometry") >>= aGeoPropSeq;
    comphelper::SequenceAsHashMap aGeoPropMap(aGeoPropSeq);
    uno::Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
    aGeoPropMap.getValue("AdjustmentValues") >>= aAdjustmentSeq;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aAdjustmentSeq.getLength());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11587
    // - Actual  : 10813
    // i.e. the adjust value was set from the placeholder, not from the shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11587), aAdjustmentSeq[0].Value.get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testChartDataLabelCharColor)
{
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "chart-data-label-char-color.docx";
    load(aURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<chart2::XChartDocument> xModel(xShape->getPropertyValue("Model"),
                                                  uno::UNO_QUERY);
    uno::Reference<chart2::XCoordinateSystemContainer> xDiagram(xModel->getFirstDiagram(),
                                                                uno::UNO_QUERY);

    uno::Reference<chart2::XChartTypeContainer> xCoordinateSystem(
        xDiagram->getCoordinateSystems()[0], uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeriesContainer> xChartType(xCoordinateSystem->getChartTypes()[0],
                                                            uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeries> xDataSeries = xChartType->getDataSeries()[0];

    uno::Reference<beans::XPropertySet> xDataPoint = xDataSeries->getDataPointByIndex(0);

    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aLabels;
    xDataPoint->getPropertyValue("CustomLabelFields") >>= aLabels;
    uno::Reference<beans::XPropertySet> xLabel = aLabels[0];

    sal_Int32 nCharColor = 0;
    xLabel->getPropertyValue("CharColor") >>= nCharColor;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16777215
    // - Actual  : -1
    // i.e. the data label had no explicit (white) color.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffff), nCharColor);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testGradientMultiStepTransparency)
{
    // Load a document with a multi-step gradient.
    OUString aURL
        = m_directories.getURLFromSrc(DATA_DIRECTORY) + "gradient-multistep-transparency.pptx";
    load(aURL);

    // Check the end transparency of the gradient.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Rectangle 4"), xShape->getName());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    awt::Gradient aTransparence;
    xShapeProps->getPropertyValue("FillTransparenceGradient") >>= aTransparence;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 16777215 (0xffffff)
    // - Actual  : 3487029 (0x353535)
    // i.e. the end transparency was not 100%, but was 21%, leading to an unexpected visible line on
    // the right of this shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffff), aTransparence.EndColor);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testShapeTextAlignment)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "shape-text-alignment.pptx";
    load(aURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nParaAdjust = -1;
    CPPUNIT_ASSERT(xShape->getPropertyValue("ParaAdjust") >>= nParaAdjust);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 3
    // i.e. text which is meant to be left-aligned was centered at a paragraph level.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testShapeTextAdjustLeft)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "shape-text-adjust-left.pptx";
    load(aURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    drawing::TextHorizontalAdjust eAdjust;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3 (center)
    // - Actual  : 1 (block)
    // i.e. text was center-adjusted, not default-adjusted (~left).
    CPPUNIT_ASSERT(xShape->getPropertyValue("TextHorizontalAdjust") >>= eAdjust);
    CPPUNIT_ASSERT_EQUAL(drawing::TextHorizontalAdjust_BLOCK, eAdjust);
}

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testCameraRotationRevolution)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "camera-rotation-revolution.docx";
    load(aURL);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape0(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xShape1(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps0(xShape0, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShapeProps1(xShape1, uno::UNO_QUERY);
    sal_Int32 nRotateAngle0;
    sal_Int32 nRotateAngle1;
    xShapeProps0->getPropertyValue("RotateAngle") >>= nRotateAngle0;
    xShapeProps1->getPropertyValue("RotateAngle") >>= nRotateAngle1;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8000
    // - Actual  : 0
    // so the camera rotation would not have been factored into how the shape is displayed
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8000), nRotateAngle0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(27000), nRotateAngle1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
