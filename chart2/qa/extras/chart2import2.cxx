/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XAxisXSupplier.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/text/XText.hpp>

class Chart2ImportTest2 : public ChartTest
{
public:
    void testTdf114179();
    void testTdf124243();
    void testTdf127393();
    void testTdf128733();
    void testTdf128432();
    void testTdf128627();
    void testTdf128634();
    void testTdf130657();
    void testDeletedDataLabel();
    void testDataPointInheritedColorDOCX();
    void testExternalStrRefsXLSX();
    void testSourceNumberFormatComplexCategoriesXLS();
    void testSimpleCategoryAxis();
    void testMultilevelCategoryAxis();
    void testXaxisValues();
    void testTdf123504();
    void testTdf122765();
    void testTdf123206CustomLabelField();
    void testTdf125444PercentageCustomLabel();
    void testDataPointLabelCustomPos();
    void testTdf130032();
    void testTdf134978();
    void testTdf119138MissingAutoTitleDeleted();
    void testStockChartShiftedCategoryPosition();
    void testTdf133376();
    void testTdf134225();
    void testTdf136105();
    void testTdf91250();
    void testTdf134111();
    void testTdf136752();
    void testTdf137505();
    void testTdf137734();
    void testTdf137874();
    void testTdf146463();
    void testTdfCustomShapePos();
    void testTdf121281();
    void testTdf139658();
    void testTdf146066();
    void testTdf150034();

    CPPUNIT_TEST_SUITE(Chart2ImportTest2);

    CPPUNIT_TEST(testTdf114179);
    CPPUNIT_TEST(testTdf124243);
    CPPUNIT_TEST(testTdf127393);
    CPPUNIT_TEST(testTdf128733);
    CPPUNIT_TEST(testTdf128432);
    CPPUNIT_TEST(testTdf128627);
    CPPUNIT_TEST(testTdf128634);
    CPPUNIT_TEST(testTdf130657);
    CPPUNIT_TEST(testDeletedDataLabel);
    CPPUNIT_TEST(testDataPointInheritedColorDOCX);
    CPPUNIT_TEST(testExternalStrRefsXLSX);
    CPPUNIT_TEST(testSourceNumberFormatComplexCategoriesXLS);
    CPPUNIT_TEST(testSimpleCategoryAxis);
    CPPUNIT_TEST(testMultilevelCategoryAxis);
    CPPUNIT_TEST(testXaxisValues);
    CPPUNIT_TEST(testTdf123504);
    CPPUNIT_TEST(testTdf122765);
    CPPUNIT_TEST(testTdf123206CustomLabelField);
    CPPUNIT_TEST(testTdf125444PercentageCustomLabel);
    CPPUNIT_TEST(testDataPointLabelCustomPos);
    CPPUNIT_TEST(testTdf130032);
    CPPUNIT_TEST(testTdf134978);
    CPPUNIT_TEST(testTdf119138MissingAutoTitleDeleted);
    CPPUNIT_TEST(testStockChartShiftedCategoryPosition);
    CPPUNIT_TEST(testTdf133376);
    CPPUNIT_TEST(testTdf134225);
    CPPUNIT_TEST(testTdf136105);
    CPPUNIT_TEST(testTdf91250);
    CPPUNIT_TEST(testTdf134111);
    CPPUNIT_TEST(testTdf136752);
    CPPUNIT_TEST(testTdf137505);
    CPPUNIT_TEST(testTdf137734);
    CPPUNIT_TEST(testTdf137874);
    CPPUNIT_TEST(testTdf146463);
    CPPUNIT_TEST(testTdfCustomShapePos);
    CPPUNIT_TEST(testTdf121281);
    CPPUNIT_TEST(testTdf139658);
    CPPUNIT_TEST(testTdf146066);
    CPPUNIT_TEST(testTdf150034);

    CPPUNIT_TEST_SUITE_END();
};

void Chart2ImportTest2::testTdf114179()
{
    load(u"/chart2/qa/extras/data/docx/", u"testTdf114179.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    css::uno::Reference<chart2::XDiagram> xDiagram;
    xDiagram.set(xChartDoc->getFirstDiagram());
    CPPUNIT_ASSERT_MESSAGE("There is a Diagram.", xDiagram.is());
    awt::Size aPage = getPageSize(xChartDoc);
    awt::Size aSize = getSize(xDiagram, aPage);
    CPPUNIT_ASSERT(aSize.Width > 0);
    CPPUNIT_ASSERT(aSize.Height > 0);
}

void Chart2ImportTest2::testTdf124243()
{
    load(u"/chart2/qa/extras/data/docx/", u"tdf124243.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    Reference<beans::XPropertySet> xPS(xAxis, uno::UNO_QUERY_THROW);
    bool bShow = true;
    // test X Axis is not visible.
    bool bSuccess = xPS->getPropertyValue("Show") >>= bShow;
    CPPUNIT_ASSERT(bSuccess);
    CPPUNIT_ASSERT(!bShow);
}

void Chart2ImportTest2::testTdf127393()
{
    load(u"/chart2/qa/extras/data/pptx/", u"tdf127393.pptx");

    // 1st chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData1 = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData1.Categories.is());
    CPPUNIT_ASSERT(aScaleData1.ShiftedCategoryPosition);

    // 2nd chart
    xChartDoc.set(getChartDocFromDrawImpress(1, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    xAxis.set(getAxisFromDoc(xChartDoc, 0, 0, 0));
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData2 = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData2.Categories.is());
    CPPUNIT_ASSERT(!aScaleData2.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf128733()
{
    load(u"/chart2/qa/extras/data/odt/", u"tdf128733.odt");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    // test secondary X axis ShiftedCategoryPosition value
    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 1);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf128432()
{
    load(u"/chart2/qa/extras/data/ods/", u"tdf128432.ods");

    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf128627()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf128627.xlsx");
    // Test ShiftedCategoryPosition for Radar Chart
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(!aScaleData.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf128634()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf128634.xlsx");
    // Test ShiftedCategoryPosition for 3D Charts
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf130657()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf130657.xlsx");
    // Test ShiftedCategoryPosition for charts which is not contain a "crossbetween" OOXML tag.
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);
}

namespace
{
void checkDataLabelProperties(const Reference<chart2::XDataSeries>& xDataSeries,
                              sal_Int32 nDataPointIndex, bool bValueVisible)
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        xDataSeries->getDataPointByIndex(nDataPointIndex), uno::UNO_SET_THROW);
    chart2::DataPointLabel aLabel;
    xPropertySet->getPropertyValue("Label") >>= aLabel;
    CPPUNIT_ASSERT_EQUAL(bValueVisible, static_cast<bool>(aLabel.ShowNumber));
    CPPUNIT_ASSERT_EQUAL(false, static_cast<bool>(aLabel.ShowNumberInPercent));
}
}

void Chart2ImportTest2::testDeletedDataLabel()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"deleted_data_labels.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartCompFromSheet(0, mxComponent),
                                                     UNO_QUERY_THROW);
    Reference<chart2::XDataSeries> xDataSeries0 = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries0.is());
    checkDataLabelProperties(xDataSeries0, 0, true);
    checkDataLabelProperties(xDataSeries0, 1, false);
    checkDataLabelProperties(xDataSeries0, 2, true);
    Reference<chart2::XDataSeries> xDataSeries1 = getDataSeriesFromDoc(xChartDoc, 1);
    CPPUNIT_ASSERT(xDataSeries1.is());
    checkDataLabelProperties(xDataSeries1, 0, false);
    checkDataLabelProperties(xDataSeries1, 1, false);
    checkDataLabelProperties(xDataSeries1, 2, false);
}

void Chart2ImportTest2::testDataPointInheritedColorDOCX()
{
    load(u"/chart2/qa/extras/data/docx/", u"data_point_inherited_color.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    css::uno::Reference<chart2::XDiagram> xDiagram(xChartDoc->getFirstDiagram(), UNO_SET_THROW);

    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                     uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());
    sal_Int32 nColor = xPropertySet->getPropertyValue("FillColor").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16776960), nColor);
}

void Chart2ImportTest2::testExternalStrRefsXLSX()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"external_str_ref.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartCompFromSheet(0, mxComponent),
                                                     UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    css::uno::Sequence<css::uno::Any> aValues = aScaleData.Categories->getValues()->getData();
    CPPUNIT_ASSERT_EQUAL(OUString("test1"), aValues[0].get<OUString>());
    CPPUNIT_ASSERT_EQUAL(OUString("test2"), aValues[1].get<OUString>());
}

void Chart2ImportTest2::testSourceNumberFormatComplexCategoriesXLS()
{
    load(u"/chart2/qa/extras/data/xls/", u"source_number_format_axis.xls");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartCompFromSheet(0, mxComponent),
                                                     UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    sal_Int32 nNumberFormat = aScaleData.Categories->getValues()->getNumberFormatKeyByIndex(-1);
    CPPUNIT_ASSERT(nNumberFormat != 0);
}

void Chart2ImportTest2::testSimpleCategoryAxis()
{
    load(u"/chart2/qa/extras/data/docx/", u"testSimpleCategoryAxis.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // Test the internal data.
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the category labels.
    Sequence<Sequence<OUString>> aCategories = xDescAccess->getComplexRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCategories[0].getLength());
    CPPUNIT_ASSERT(aCategories[0][0].endsWith("ria 1"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCategories[1].getLength());
    CPPUNIT_ASSERT(aCategories[1][0].endsWith("ria 2"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCategories[2].getLength());
    CPPUNIT_ASSERT(aCategories[2][0].endsWith("ria 3"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aCategories[3].getLength());
    CPPUNIT_ASSERT(aCategories[3][0].endsWith("ria 4"));
}

void Chart2ImportTest2::testMultilevelCategoryAxis()
{
    load(u"/chart2/qa/extras/data/docx/", u"testMultilevelCategoryAxis.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // Test the internal data.
    CPPUNIT_ASSERT(xChartDoc->hasInternalDataProvider());

    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the complex category labels.
    Sequence<Sequence<OUString>> aCategories = xDescAccess->getComplexRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCategories.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("2011"), aCategories[0][0]);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aCategories[1][0]);
    CPPUNIT_ASSERT_EQUAL(OUString("2012"), aCategories[2][0]);
    CPPUNIT_ASSERT_EQUAL(OUString(""), aCategories[3][0]);
    CPPUNIT_ASSERT_EQUAL(OUString("Categoria 1"), aCategories[0][1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Categoria 2"), aCategories[1][1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Categoria 3"), aCategories[2][1]);
    CPPUNIT_ASSERT_EQUAL(OUString("Categoria 4"), aCategories[3][1]);
}

void Chart2ImportTest2::testXaxisValues()
{
    load(u"/chart2/qa/extras/data/docx/", u"tdf124083.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    const uno::Reference<chart2::data::XDataSequence> xDataSeq
        = getDataSequenceFromDocByRole(xChartDoc, u"values-x");
    Sequence<uno::Any> xSequence = xDataSeq->getData();
    // test X values
    CPPUNIT_ASSERT_EQUAL(uno::Any(0.04), xSequence[0]);
    CPPUNIT_ASSERT(std::isnan(*static_cast<const double*>(xSequence[1].getValue())));
    CPPUNIT_ASSERT_EQUAL(uno::Any(0.16), xSequence[2]);
    CPPUNIT_ASSERT_EQUAL(uno::Any(0.11), xSequence[3]);
    CPPUNIT_ASSERT(std::isnan(*static_cast<const double*>(xSequence[4].getValue())));
}

void Chart2ImportTest2::testTdf123504()
{
    load(u"/chart2/qa/extras/data/ods/", u"pie_chart_100_and_0.ods");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<chart2::XChartDocument> xChartDoc2(xChartDoc, UNO_QUERY_THROW);
    Reference<chart2::XChartType> xChartType(getChartTypeFromDoc(xChartDoc2, 0), UNO_SET_THROW);
    std::vector aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
    CPPUNIT_ASSERT_EQUAL(size_t(1), aDataSeriesYValues.size());

    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xSeriesSlices(getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=0"),
                                             UNO_SET_THROW);

    Reference<container::XIndexAccess> xIndexAccess(xSeriesSlices, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    Reference<drawing::XShape> xSlice(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Check size and position of the only slice in the chart (100%)
    // In the regressed state, it used to be 0-sized at position 0,0
    awt::Point aSlicePosition = xSlice->getPosition();
    CPPUNIT_ASSERT_GREATER(sal_Int32(3000), aSlicePosition.X);
    CPPUNIT_ASSERT_GREATER(sal_Int32(150), aSlicePosition.Y);
    awt::Size aSliceSize = xSlice->getSize();
    CPPUNIT_ASSERT_GREATER(sal_Int32(8500), aSliceSize.Height);
    CPPUNIT_ASSERT_GREATER(sal_Int32(8500), aSliceSize.Width);
}

void Chart2ImportTest2::testTdf122765()
{
    // The horizontal position of the slices was wrong.
    load(u"/chart2/qa/extras/data/pptx/", u"tdf122765.pptx");
    Reference<chart::XChartDocument> xChartDoc = getChartDocFromDrawImpress(0, 0);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xSeriesSlices(getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=0"),
                                             UNO_SET_THROW);

    Reference<container::XIndexAccess> xIndexAccess(xSeriesSlices, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), xIndexAccess->getCount());
    Reference<drawing::XShape> xSlice(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);

    // Check position of the first slice, all slices move together, so enough to check only one.
    // Wrong position was around 5856.
    awt::Point aSlicePosition = xSlice->getPosition();
    CPPUNIT_ASSERT_GREATER(sal_Int32(7000), aSlicePosition.X);
}

void Chart2ImportTest2::testTdf123206CustomLabelField()
{
    // File contains the deprecated "custom-label-field" attribute of the
    // "data-point" element. It should be interpreted and stored as a data point
    // property.
    uno::Reference<chart2::XChartDocument> xChartDoc(
        getChartDocFromImpress(u"/chart2/qa/extras/data/odp/", "tdf123206.odp"),
        uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xDp = xDataSeries->getDataPointByIndex(1);
    Sequence<Reference<chart2::XDataPointCustomLabelField>> aLabelFields;
    CPPUNIT_ASSERT(xDp->getPropertyValue("CustomLabelFields") >>= aLabelFields);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aLabelFields.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("Kiskacsa"), aLabelFields[0]->getString());
}

void Chart2ImportTest2::testTdf125444PercentageCustomLabel()
{
    load(u"/chart2/qa/extras/data/pptx/", u"tdf125444.pptx");

    // 1st chart
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xDp = xDataSeries->getDataPointByIndex(1);
    Sequence<Reference<chart2::XDataPointCustomLabelField>> aLabelFields;
    CPPUNIT_ASSERT(xDp->getPropertyValue("CustomLabelFields") >>= aLabelFields);
    // There are three label field: a value label, a newline and a percentage label. We want
    // to assert the latter.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aLabelFields.getLength());
    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType_PERCENTAGE,
                         aLabelFields[2]->getFieldType());
}

void Chart2ImportTest2::testDataPointLabelCustomPos()
{
    // test CustomLabelPosition on Bar chart
    load(u"/chart2/qa/extras/data/xlsx/", u"testDataPointLabelCustomPos.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                     uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());

    chart2::RelativePosition aCustomLabelPosition;
    xPropertySet->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Primary, -0.14621409921671025, 1e-7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Secondary, -5.2887961029923464E-2, 1e-7);

    sal_Int32 aPlacement;
    xPropertySet->getPropertyValue("LabelPlacement") >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(chart::DataLabelPlacement::OUTSIDE, aPlacement);
}

void Chart2ImportTest2::testTdf130032()
{
    // test CustomLabelPosition on Line chart
    load(u"/chart2/qa/extras/data/xlsx/", u"testTdf130032.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(1),
                                                     uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());

    chart2::RelativePosition aCustomLabelPosition;
    xPropertySet->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Primary, -0.0438333333333334, 1e-7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aCustomLabelPosition.Secondary, 0.086794050743657, 1e-7);

    sal_Int32 aPlacement;
    xPropertySet->getPropertyValue("LabelPlacement") >>= aPlacement;
    CPPUNIT_ASSERT_EQUAL(chart::DataLabelPlacement::RIGHT, aPlacement);
}

void Chart2ImportTest2::testTdf134978()
{
    // test CustomLabelPosition on Pie chart
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf134978.xlsx");
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(2),
                                                     uno::UNO_SET_THROW);
    CPPUNIT_ASSERT(xPropertySet.is());

    chart2::RelativePosition aCustomLabelPosition;
    xPropertySet->getPropertyValue("CustomLabelPosition") >>= aCustomLabelPosition;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.040273622047244093, aCustomLabelPosition.Primary, 1e-7);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.25635352872557599, aCustomLabelPosition.Secondary, 1e-7);
}

void Chart2ImportTest2::testTdf119138MissingAutoTitleDeleted()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf119138-missing-autotitledeleted.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());

    Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
    uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT_MESSAGE(
        "Missing autoTitleDeleted is implied to be True if title text is present", xTitle.is());
}

void Chart2ImportTest2::testStockChartShiftedCategoryPosition()
{
    load(u"/chart2/qa/extras/data/odt/", u"stock_chart_LO_6_2.odt");

    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xAxis.is());

    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT(aScaleData.Categories.is());
    CPPUNIT_ASSERT(aScaleData.ShiftedCategoryPosition);
}

void Chart2ImportTest2::testTdf133376()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    load(u"/chart2/qa/extras/data/xlsx/", u"tdf133376.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xDataPointLabel(
        getShapeByName(xShapes,
                       "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=2"),
        UNO_SET_THROW);

    CPPUNIT_ASSERT(xDataPointLabel.is());
    // Check the position of the 3rd data point label, which is out from the pie slice
    awt::Point aLabelPosition = xDataPointLabel->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1082, aLabelPosition.X, 30);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5462, aLabelPosition.Y, 30);
}

void Chart2ImportTest2::testTdf134225()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf134225.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xDataPointLabel1(
        getShapeByName(xShapes,
                       "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0"),
        UNO_SET_THROW);
    CPPUNIT_ASSERT(xDataPointLabel1.is());

    Reference<drawing::XShape> xDataPointLabel2(
        getShapeByName(xShapes,
                       "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=1"),
        UNO_SET_THROW);
    CPPUNIT_ASSERT(xDataPointLabel2.is());

#if defined(_WIN32)
    // font is MS Comic Sans which we can only assume is available under windows
    awt::Point aLabelPosition1 = xDataPointLabel1->getPosition();
    awt::Point aLabelPosition2 = xDataPointLabel2->getPosition();

    // Check the distance between the position of the 1st data point label and the second one
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1669, sal_Int32(aLabelPosition2.X - aLabelPosition1.X), 30);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2166, sal_Int32(aLabelPosition2.Y - aLabelPosition1.Y), 30);
#endif
}

void Chart2ImportTest2::testTdf136105()
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    load(u"/chart2/qa/extras/data/xlsx/", u"tdf136105.xlsx");
    // 1st chart with fix inner position and size
    {
        Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                                   UNO_QUERY_THROW);

        Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
        Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
        Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
        Reference<drawing::XShape> xDataPointLabel(
            getShapeByName(xShapes,
                           "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0"),
            UNO_SET_THROW);

        CPPUNIT_ASSERT(xDataPointLabel.is());
        // Check the position of the 1st data point label, which is out from the pie slice
        awt::Point aLabelPosition = xDataPointLabel->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(8797, aLabelPosition.X, 500);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1374, aLabelPosition.Y, 500);
    }
    // 2nd chart with auto inner position and size
    {
        Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(1, mxComponent),
                                                   UNO_QUERY_THROW);

        Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
        Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
        Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
        Reference<drawing::XShape> xDataPointLabel(
            getShapeByName(xShapes,
                           "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0"),
            UNO_SET_THROW);

        CPPUNIT_ASSERT(xDataPointLabel.is());
        // Check the position of the 1st data point label, which is out from the pie slice
        awt::Point aLabelPosition = xDataPointLabel->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(7978, aLabelPosition.X, 500);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1048, aLabelPosition.Y, 500);
    }
}

void Chart2ImportTest2::testTdf91250()
{
    load(u"/chart2/qa/extras/data/docx/", u"tdf91250.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the category labels.
    Sequence<OUString> aCategories = xDescAccess->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aCategories.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("12.3254"), aCategories[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("11.62315"), aCategories[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("9.26"), aCategories[2]);
    CPPUNIT_ASSERT_EQUAL(OUString("8.657"), aCategories[3]);
}

void Chart2ImportTest2::testTdf134111()
{
    // tdf134111 : To check TextBreak value is true
    load(u"/chart2/qa/extras/data/docx/", u"tdf134111.docx");
    uno::Reference<chart::XChartDocument> xChartDoc = getChartDocFromWriter(0);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    uno::Reference<chart::XDiagram> mxDiagram(xChartDoc->getDiagram());
    CPPUNIT_ASSERT(mxDiagram.is());
    uno::Reference<chart::XAxisXSupplier> xAxisXSupp(mxDiagram, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAxisXSupp.is());
    uno::Reference<beans::XPropertySet> xAxisProp(xAxisXSupp->getXAxis());
    bool bTextBreak = false;
    xAxisProp->getPropertyValue("TextBreak") >>= bTextBreak;
    // Expected value of 'TextBreak' is true
    CPPUNIT_ASSERT(bTextBreak);
}

void Chart2ImportTest2::testTdf136752()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf136752.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xDataPointLabel(
        getShapeByName(xShapes,
                       "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0"),
        UNO_SET_THROW);

    CPPUNIT_ASSERT(xDataPointLabel.is());
    // Check the position of the 1st data point label, which is out from the pie slice
    awt::Point aLabelPosition = xDataPointLabel->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(8675, aLabelPosition.X, 500);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1458, aLabelPosition.Y, 500);
}

void Chart2ImportTest2::testTdf137505()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf137505.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);

    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());

    float nFontSize;
    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    Reference<text::XTextCursor> xAt = xRange->createTextCursor();
    Reference<beans::XPropertySet> xProps(xAt, UNO_QUERY);
    // check the text size of custom shape, inside the chart.
    CPPUNIT_ASSERT(xProps->getPropertyValue("CharHeight") >>= nFontSize);
    CPPUNIT_ASSERT_EQUAL(float(12), nFontSize);
}

void Chart2ImportTest2::testTdf137734()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf137734.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("VaryColorsByPoint");
    bool bVaryColor = true;
    CPPUNIT_ASSERT(aAny >>= bVaryColor);
    CPPUNIT_ASSERT(!bVaryColor);

    // tdf#126133 Test primary X axis Rotation value
    Reference<chart2::XAxis> xXAxis = getAxisFromDoc(xChartDoc, 0, 0, 0);
    CPPUNIT_ASSERT(xXAxis.is());
    Reference<chart2::XTitled> xTitled(xXAxis, uno::UNO_QUERY_THROW);
    Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
    CPPUNIT_ASSERT(xTitle.is());
    Reference<beans::XPropertySet> xTitlePropSet(xTitle, uno::UNO_QUERY_THROW);
    uno::Any aAny2 = xTitlePropSet->getPropertyValue("TextRotation");
    double nRotation = -1;
    CPPUNIT_ASSERT(aAny2 >>= nRotation);
    CPPUNIT_ASSERT_EQUAL(0.0, nRotation);
}

void Chart2ImportTest2::testTdf137874()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"piechart_legend.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xLegendEntry;
    xLegendEntry
        = getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=0:LegendEntry=0");
    CPPUNIT_ASSERT(xLegendEntry.is());
}

void Chart2ImportTest2::testTdf146463()
{
    load(u"/chart2/qa/extras/data/ods/", u"tdf146463.ods");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xLegend = getShapeByName(xShapes, "CID/D=0:Legend=");
    CPPUNIT_ASSERT(xLegend.is());

    awt::Size aSize = xLegend->getSize();

    // Without the fix in place, this test would have failed with
    // - Expected: 598
    // - Actual  : 7072
    CPPUNIT_ASSERT_DOUBLES_EQUAL(598, aSize.Height, 30);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4256, aSize.Width, 30);
    awt::Point aPosition = xLegend->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11534, aPosition.X, 30);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4201, aPosition.Y, 30);
}

void Chart2ImportTest2::testTdfCustomShapePos()
{
    load(u"/chart2/qa/extras/data/docx/", u"testcustomshapepos.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    // test position and size of a custom shape within a chart, rotated by 0 degree.
    {
        Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
        awt::Point aPosition = xCustomShape->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(8845, aPosition.X, 300);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(855, aPosition.Y, 300);
        awt::Size aSize = xCustomShape->getSize();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4831, aSize.Width, 300);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1550, aSize.Height, 300);
    }
    // test position and size of a custom shape within a chart, rotated by 90 degree.
    {
        Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
        awt::Point aPosition = xCustomShape->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1658, aPosition.X, 300);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(6119, aPosition.Y, 300);
        awt::Size aSize = xCustomShape->getSize();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(4165, aSize.Width, 300);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1334, aSize.Height, 300);
    }
}

void Chart2ImportTest2::testTdf121281()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"incorrect_label_position.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xDataPointLabel(
        getShapeByName(xShapes,
                       "CID/MultiClick/CID/D=0:CS=0:CT=0:Series=0:DataLabels=:DataLabel=0"),
        UNO_SET_THROW);

    CPPUNIT_ASSERT(xDataPointLabel.is());
    awt::Point aLabelPosition = xDataPointLabel->getPosition();
    // This failed, if the data label flowed out of the chart area.
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), aLabelPosition.Y);
}

void Chart2ImportTest2::testTdf139658()
{
    load(u"/chart2/qa/extras/data/docx/", u"tdf139658.docx");
    uno::Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XInternalDataProvider> xInternalProvider(xChartDoc->getDataProvider(),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInternalProvider.is());

    Reference<chart::XComplexDescriptionAccess> xDescAccess(xInternalProvider, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDescAccess.is());

    // Get the category labels.
    Sequence<OUString> aCategories = xDescAccess->getRowDescriptions();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aCategories.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("category1"), aCategories[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("\"category2\""), aCategories[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("category\"3"), aCategories[2]);
}

void Chart2ImportTest2::testTdf146066()
{
    load(u"/chart2/qa/extras/data/ods/", u"tdf146066.ods");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShapes.is());

    uno::Reference<drawing::XShape> xYAxisShape = getShapeByName(
        xShapes, "CID/D=0:CS=0:Axis=1,0", // Y Axis
        // Axis occurs twice in chart xshape representation so need to get the one related to labels
        [](const uno::Reference<drawing::XShape>& rXShape) -> bool {
            uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
            CPPUNIT_ASSERT(xAxisShapes.is());
            uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
            uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xChildShape,
                                                                       uno::UNO_QUERY_THROW);
            return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
        });
    CPPUNIT_ASSERT(xYAxisShape.is());

    // Check label count
    uno::Reference<container::XIndexAccess> xIndexAccess(xYAxisShape, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), xIndexAccess->getCount());

    // Check text
    uno::Reference<text::XTextRange> xLabel0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("0"), xLabel0->getString());
    uno::Reference<text::XTextRange> xLabel1(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("5"), xLabel1->getString());
    uno::Reference<text::XTextRange> xLabel2(xIndexAccess->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("10"), xLabel2->getString());
    uno::Reference<text::XTextRange> xLabel3(xIndexAccess->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("15"), xLabel3->getString());
    uno::Reference<text::XTextRange> xLabel4(xIndexAccess->getByIndex(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("20"), xLabel4->getString());
    uno::Reference<text::XTextRange> xLabel5(xIndexAccess->getByIndex(5), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("25"), xLabel5->getString());
    uno::Reference<text::XTextRange> xLabel6(xIndexAccess->getByIndex(6), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("30"), xLabel6->getString());
    uno::Reference<text::XTextRange> xLabel7(xIndexAccess->getByIndex(7), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("35"), xLabel7->getString());
}

void Chart2ImportTest2::testTdf150034()
{
    load(u"/chart2/qa/extras/data/xlsx/", u"tdf150034.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xLegend = getShapeByName(xShapes, "CID/D=0:Legend=");
    CPPUNIT_ASSERT(xLegend.is());
    awt::Point aPosition = xLegend->getPosition();

    // This failed, if the legend flowed out of the chart area.
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), aPosition.Y);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ImportTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
