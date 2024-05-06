/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/chart2/DataPointCustomLabelFieldType.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

using uno::Reference;
using beans::XPropertySet;

class Chart2ExportTest2 : public ChartTest
{
public:
    Chart2ExportTest2()
        : ChartTest(u"/chart2/qa/extras/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testSetSeriesToSecondaryAxisXLSX)
{
    loadFromFile(u"xlsx/add_series_secondary_axis.xlsx");
    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    // Second series
    Reference<chart2::XDataSeries> xSeries = getDataSeriesFromDoc(xChartDoc, 1);
    CPPUNIT_ASSERT(xSeries.is());

    Reference<beans::XPropertySet> xPropSet(xSeries, uno::UNO_QUERY_THROW);
    sal_Int32 AxisIndex = 1;
    // Attach the second series to the secondary axis. (The third series is already attached.)
    xPropSet->setPropertyValue(u"AttachedAxisIndex"_ustr, uno::Any(AxisIndex));

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Check there are only two <lineChart> tag in the XML, one for the primary and one for the secondary axis.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCombinedChartSecondaryAxisXLSX)
{
    // Original file was created with MS Office
    loadFromFile(u"xlsx/combined_chart_secondary_axis.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on secondary Axis
    OUString XValueIdOfBarchart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOfBarchart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]"_ostr, "val"_ostr);
    // Collect linechart axID on primary Axis
    OUString XValueIdOfLinechart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOfLinechart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]"_ostr, "val"_ostr);
    // Check which c:catAx and c:valAx contain the AxisId of charttypes
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId"_ostr, "val"_ostr,
                XValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId"_ostr, "val"_ostr,
                YValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId"_ostr, "val"_ostr,
                XValueIdOfBarchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId"_ostr, "val"_ostr,
                YValueIdOfBarchart);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCombinedChartSecondaryAxisODS)
{
    // Original file was created with LibreOffice
    loadFromFile(u"ods/combined_chart_secondary_axis.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Collect barchart axID on secondary Axis
    OUString XValueIdOfBarchart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOfBarchart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]"_ostr, "val"_ostr);
    // Collect linechart axID on primary Axis
    OUString XValueIdOfLinechart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueIdOfLinechart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]"_ostr, "val"_ostr);
    // Check which c:catAx and c:valAx contain the AxisId of charttypes
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[1]/c:axId"_ostr, "val"_ostr,
                XValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[1]/c:axId"_ostr, "val"_ostr,
                YValueIdOfLinechart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:axId"_ostr, "val"_ostr,
                XValueIdOfBarchart);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx[2]/c:axId"_ostr, "val"_ostr,
                YValueIdOfBarchart);
    // do not need CT_crosses tag if the actual axis is deleted, so we need to make sure it is not saved
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx[2]/c:crosses"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCrossBetweenXLSX)
{
    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    // Original files were created with MS Office
    {
        loadFromFile(u"xlsx/tdf127777.xlsx");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr,
                    "val"_ostr, u"between"_ustr);
    }
    {
        loadFromFile(u"xlsx/tdf132076.xlsx");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr,
                    "val"_ostr, u"between"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCrossBetweenWithDeletedAxis)
{
    // Original file was created with MS Office (the category axis is deleted in the file)
    loadFromFile(u"xlsx/tdf128633.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr, "val"_ostr,
                u"between"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCrossBetweenODS)
{
    // Original file was created with LibreOffice
    loadFromFile(u"ods/test_CrossBetween.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr, "val"_ostr,
                u"between"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testAxisTitleRotationXLSX)
{
    loadFromFile(u"xlsx/axis_title_rotation.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:tx/c:rich/a:bodyPr"_ostr,
                "rot"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testAxisTitlePositionDOCX)
{
    loadFromFile(u"docx/testAxisTitlePosition.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // test X Axis title position
    OUString aXVal = getXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:title/c:layout/c:manualLayout/c:x"_ostr,
        "val"_ostr);
    double nX = aXVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.698208543867708, nX, 1e-3);
    OUString aYVal = getXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:title/c:layout/c:manualLayout/c:y"_ostr,
        "val"_ostr);
    double nY = aYVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.805152435594555, nY, 1e-3);

    // test Y Axis title position
    aXVal = getXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:layout/c:manualLayout/c:x"_ostr,
        "val"_ostr);
    nX = aXVal.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0253953671500755, nX, 1e-3);
    aYVal = getXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:title/c:layout/c:manualLayout/c:y"_ostr,
        "val"_ostr);
    nY = aYVal.toDouble();
    // just test the first two decimal digits because it is not perfect in docx yet.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.384070199122511, nY, 1e-2);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testAxisCrossBetweenDOCX)
{
    loadFromFile(u"odt/axis-position.odt");

    // FIXME: validation error in OOXML export: Errors: 3
    skipValidation();

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    assertXPath(pXmlDoc, "(//c:crossBetween)[1]"_ostr, "val"_ostr, u"midCat"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testPieChartDataPointExplosionXLSX)
{
    loadFromFile(u"xlsx/pie_chart_datapoint_explosion.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dPt/c:explosion"_ostr,
                "val"_ostr, u"28"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCustomDataLabel)
{
    loadFromFile(u"pptx/tdf115107.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    sal_Int32 nCharUnderline;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // 1
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"90.0 = "_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"90"_ustr, aFields[1]->getString());
    CPPUNIT_ASSERT_EQUAL(u"{0C576297-5A9F-4B4E-A675-B6BA406B7D87}"_ustr, aFields[1]->getGuid());

    // 2
    xPropertySet.set(xDataSeries->getDataPointByIndex(1), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr, aFields[0]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" : "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CATEGORYNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, aFields[2]->getString());
    CPPUNIT_ASSERT_EQUAL(u"{0CCAAACD-B393-42CE-8DBD-82F9F9ADC852}"_ustr, aFields[2]->getGuid());
    aFields[2]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[2]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(16), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xed7d31), nFontColor);

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE,
        aFields[3]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[4]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Multi"_ustr, aFields[4]->getString());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[5]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"line"_ustr, aFields[5]->getString());
    aFields[5]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[5]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(11.97), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xbf9000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_NEWLINE,
        aFields[6]->getFieldType());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[7]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Abc"_ustr, aFields[7]->getString());
    aFields[7]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[7]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    aFields[7]->getPropertyValue(u"CharUnderline"_ustr) >>= nCharUnderline;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(12), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xa9d18e), nFontColor);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nCharUnderline);

    // 3
    xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"DATA"_ustr, aFields[0]->getString());
    CPPUNIT_ASSERT_EQUAL(u"{C8F3EB90-8960-4F9A-A3AD-B4FAC4FE4566}"_ustr, aFields[0]->getGuid());

    // 4
    xPropertySet.set(xDataSeries->getDataPointByIndex(3), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLREF,
        aFields[0]->getFieldType());
    //CPPUNIT_ASSERT_EQUAL(OUString("70"), aFields[0]->getString()); TODO: Not implemented yet

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" <CELLREF"_ustr, aFields[1]->getString());

    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // Check the data labels font color for the complete data series
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:txPr/a:p/a:pPr/"
                "a:defRPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, u"404040"_ustr);
}

/// Test for tdf#94235
CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testDataSeriesName)
{
    // ODF
    {
        loadFromFile(u"ods/ser_labels.ods");
        saveAndReload(u"calc8"_ustr);
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet;
        chart2::DataPointLabel aDataPointLabel;
        xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue(u"Label"_ustr) >>= aDataPointLabel;
        CPPUNIT_ASSERT_EQUAL(sal_True, aDataPointLabel.ShowSeriesName);
    }

    // OOXML
    {
        loadFromFile(u"xlsx/ser_labels.xlsx");
        saveAndReload(u"Calc Office Open XML"_ustr);
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet;
        chart2::DataPointLabel aDataPointLabel;
        xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue(u"Label"_ustr) >>= aDataPointLabel;
        CPPUNIT_ASSERT_EQUAL(sal_True, aDataPointLabel.ShowSeriesName);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCustomPositionofDataLabel)
{
    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    loadFromFile(u"xlsx/testCustomPosDataLabels.xlsx");
    {
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);

        // test custom position of data label (xlsx)
        assertXPath(
            pXmlDoc,
            "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[1]/c:idx"_ostr,
            "val"_ostr, u"2"_ustr);
        OUString aXVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[1]/c:layout/c:manualLayout/c:x"_ostr,
                                  "val"_ostr);
        double nX = aXVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.11027682973075476, nX, 1e-7);

        OUString aYVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[1]/c:layout/c:manualLayout/c:y"_ostr,
                                  "val"_ostr);
        double nY = aYVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.0742140311063737, nY, 1e-7);
    }

    loadFromFile(u"docx/testTdf108110.docx");
    {
        save(u"Office Open XML Text"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);

        // test custom position of data label (docx)
        assertXPath(
            pXmlDoc,
            "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/c:dLbl[2]/c:idx"_ostr,
            "val"_ostr, u"2"_ustr);
        OUString aXVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[2]/c:layout/c:manualLayout/c:x"_ostr,
                                  "val"_ostr);
        double nX = aXVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0227256488772236, nX, 1e-7);

        OUString aYVal = getXPath(pXmlDoc,
                                  "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:dLbls/"
                                  "c:dLbl[2]/c:layout/c:manualLayout/c:y"_ostr,
                                  "val"_ostr);
        double nY = aYVal.toDouble();
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.172648731408574, nY, 1e-7);
    }

    loadFromFile(u"ods/tdf136024.ods");
    {
        saveAndReload(u"calc8"_ustr);
        // tdf#136024: test custom position of pie chart data label after an ods export
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());

        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                         uno::UNO_SET_THROW);

        chart2::RelativePosition aCustomLabelPosition;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"CustomLabelPosition"_ustr)
                       >>= aCustomLabelPosition);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.0961935120945059, aCustomLabelPosition.Primary, 1e-5);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.209578842093566, aCustomLabelPosition.Secondary, 1e-5);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCustomDataLabelMultipleSeries)
{
    loadFromFile(u"pptx/tdf115107-2.pptx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromDrawImpress(0, 0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    float nFontSize;
    sal_Int64 nFontColor;
    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;

    // First series
    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"4.3"_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xc00000), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Bars"_ustr, aFields[2]->getString());

    // Second series
    xDataSeries = getDataSeriesFromDoc(xChartDoc, 0, 1);
    CPPUNIT_ASSERT(xDataSeries.is());

    xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aFields.getLength());

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_VALUE,
                         aFields[0]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aFields[0]->getString());
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    aFields[0]->getPropertyValue(u"CharColor"_ustr) >>= nFontColor;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(18), nFontSize);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(0xffd966), nFontColor);

    CPPUNIT_ASSERT_EQUAL(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT,
                         aFields[1]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u" "_ustr, aFields[1]->getString());

    CPPUNIT_ASSERT_EQUAL(
        chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_SERIESNAME,
        aFields[2]->getFieldType());
    CPPUNIT_ASSERT_EQUAL(u"Line"_ustr, aFields[2]->getString());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testLeaderLines)
{
    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    loadFromFile(u"xlsx/testTdf90749.xlsx");
    {
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[1]/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines"_ostr,
                    "val"_ostr, u"1"_ustr);
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser[2]/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines"_ostr,
                    "val"_ostr, u"0"_ustr);
    }
    loadFromFile(u"docx/MSO_Custom_Leader_Line.docx");
    {
        save(u"Office Open XML Text"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        // tdf#134571: Check the leader line is switch off.
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:extLst/c:ext/"
                    "c15:showLeaderLines"_ostr,
                    "val"_ostr, u"0"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testNumberFormatExportPPTX)
{
    loadFromFile(u"pptx/tdf115859.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:numFmt"_ostr,
                "formatCode"_ostr, u"#,##0.00,\\K"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:numFmt"_ostr,
                "sourceLinked"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testLabelSeparatorExportDOCX)
{
    loadFromFile(u"docx/testLabelSeparator.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // The text separator should be a new line
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:separator"_ostr,
        u"\n"_ustr);
    // The text separator should be a comma
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:dLbls/c:separator"_ostr,
        u", "_ustr);
    // The text separator should be a semicolon
    assertXPathContent(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:dLbls/c:separator"_ostr,
        u"; "_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testChartTitlePropertiesColorFillPPTX)
{
    loadFromFile(u"pptx/testChartTitlePropertiesColorFill.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, u"ff0000"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testChartTitlePropertiesGradientFillPPTX)
{
    loadFromFile(u"pptx/testChartTitlePropertiesGradientFill.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[1]/a:srgbClr"_ostr,
                "val"_ostr, u"f6f8fc"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:title/c:spPr/a:gradFill/a:gsLst/a:gs[2]/a:srgbClr"_ostr,
                "val"_ostr, u"c7d5ed"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testChartTitlePropertiesBitmapFillPPTX)
{
    loadFromFile(u"pptx/testChartTitlePropertiesBitmapFill.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:blipFill/a:blip"_ostr,
                "embed"_ostr, u"rId1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:title/c:spPr/a:ln/a:noFill"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testxAxisLabelsRotation)
{
    loadFromFile(u"xlsx/xAxisLabelsRotation.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc1 = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc1);

    // Chart1 xAxis labels should be 45 degree
    assertXPath(pXmlDoc1, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:bodyPr"_ostr,
                "rot"_ostr, u"2700000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testMultipleCategoryAxisLablesXLSX)
{
    loadFromFile(u"ods/multilevelcat.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // check category axis labels number of first level
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:multiLvlStrRef/"
                "c:multiLvlStrCache/c:ptCount"_ostr,
                "val"_ostr, u"6"_ustr);
    // check category axis labels text of first level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[1]/c:v"_ostr,
                       u"Categoria 1"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[6]/c:v"_ostr,
                       u"Categoria 6"_ustr);
    // check category axis labels text of second level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[1]/c:v"_ostr,
                       u"2011"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[3]/c:v"_ostr,
                       u"2013"_ustr);
    // check the 'noMultiLvlLbl' tag - ChartExport.cxx:2950 FIXME: seems not support, so check the default noMultiLvlLbl value.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:noMultiLvlLbl"_ostr,
                "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testMultipleCategoryAxisLablesDOCX)
{
    loadFromFile(u"odt/multilevelcat.odt");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // check category axis labels number of first level
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:multiLvlStrRef/"
                "c:multiLvlStrCache/c:ptCount"_ostr,
                "val"_ostr, u"4"_ustr);
    // check category axis labels text of first level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[1]/c:v"_ostr,
                       u"Categoria 1"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[1]/c:pt[4]/c:v"_ostr,
                       u"Categoria 4"_ustr);
    // check category axis labels text of second level
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[1]/c:v"_ostr,
                       u"2011"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/"
                       "c:multiLvlStrRef/c:multiLvlStrCache/c:lvl[2]/c:pt[2]/c:v"_ostr,
                       u"2012"_ustr);
    // check the 'noMultiLvlLbl' tag - ChartExport.cxx:2950 FIXME: seems not support, so check the default noMultiLvlLbl value.
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:noMultiLvlLbl"_ostr,
                "val"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf116163)
{
    loadFromFile(u"pptx/tdf116163.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:txPr/a:bodyPr"_ostr,
                "rot"_ostr, u"-5400000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf111824)
{
    loadFromFile(u"xlsx/tdf111824.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Collect 3D barchart Z axID
    OUString zAxisIdOf3DBarchart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:bar3DChart/c:axId[3]"_ostr, "val"_ostr);
    // 3D barchart Z axis properties should be in a serAx OOXML tag instead of catAx
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:serAx/c:axId"_ostr, "val"_ostr,
                zAxisIdOf3DBarchart);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, test3DAreaChartZAxis)
{
    loadFromFile(u"xlsx/test3DAreaChartZAxis.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Collect 3D area chart Z axID
    OUString zAxisIdOf3DAreachart = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:area3DChart/c:axId[3]"_ostr, "val"_ostr);
    // 3D area chart z-axis properties should be in a serAx OOXML element instead of catAx
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:serAx/c:axId"_ostr, "val"_ostr,
                zAxisIdOf3DAreachart);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf119029)
{
    loadFromFile(u"odp/tdf119029.odp");
    // Only use "chart", without number, because the number depends on the previous tests
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:txPr/a:bodyPr"_ostr,
                "rot"_ostr, u"-5400000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf108022)
{
    loadFromFile(u"odt/tdf108022.odt");
    saveAndReload(u"Office Open XML Text"_ustr);

    // assert we really have two charts
    Reference<chart2::XChartDocument> xChartDoc1(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc1.is());
    Reference<chart2::XChartDocument> xChartDoc2(getChartDocFromWriter(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc2.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf121744)
{
    loadFromFile(u"docx/tdf121744.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    OUString XValueId = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[1]"_ostr, "val"_ostr);
    OUString YValueId = getXPath(
        pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:axId[2]"_ostr, "val"_ostr);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[1]"_ostr, "val"_ostr,
                XValueId);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:axId[2]"_ostr, "val"_ostr,
                YValueId);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf121189)
{
    loadFromFile(u"odp/tdf121189.odp");
    saveAndReload(u"Impress Office Open XML"_ustr);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());
    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.OLE2Shape"_ustr, xShape->getShapeType());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf122031)
{
    //Checks pie chart data label format.
    loadFromFile(u"xlsx/tdf122031.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:numFmt"_ostr,
                "formatCode"_ostr, u"0.000%"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:numFmt"_ostr,
                "formatCode"_ostr, u"0.000%"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:numFmt"_ostr,
                "formatCode"_ostr, u"0.000%"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[3]/c:numFmt"_ostr,
                "formatCode"_ostr, u"0.000%"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf115012)
{
    loadFromFile(u"xlsx/tdf115012.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // workaround: use-zero instead of leave-gap to show the original line chart
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs"_ostr, "val"_ostr, u"zero"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf134118)
{
    loadFromFile(u"xlsx/tdf134118.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // workaround: use leave-gap instead of zero to show the original line chart
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:dispBlanksAs"_ostr, "val"_ostr, u"gap"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf123206_customLabelText)
{
    loadFromFile(u"docx/tdf123206.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r/a:t"_ostr,
                       u"kiscica"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCustomLabelText)
{
    loadFromFile(u"docx/testCustomlabeltext.docx");

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // FIXME: validation error in OOXML export: Errors: 3
    skipValidation();

    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:idx"_ostr,
                "val"_ostr, u"2"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:tx/"
                       "c:rich/a:p/a:r[1]/a:t"_ostr,
                       u"3.5"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[1]/c:tx/"
                       "c:rich/a:p/a:r[3]/a:t"_ostr,
                       u"CustomLabel 1"_ustr);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:idx"_ostr,
                "val"_ostr, u"3"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r[1]/a:t"_ostr,
                       u"4.5"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl[2]/c:tx/"
                       "c:rich/a:p/a:r[3]/a:t"_ostr,
                       u"CustomLabel 2"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testDeletedLegendEntries)
{
    loadFromFile(u"xlsx/deleted_legend_entry.xlsx");
    {
        saveAndReload(u"Calc Office Open XML"_ustr);
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 1));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
        bool bShowLegendEntry = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"ShowLegendEntry"_ustr)
                       >>= bShowLegendEntry);
        CPPUNIT_ASSERT(!bShowLegendEntry);
    }

    loadFromFile(u"xlsx/deleted_legend_entry2.xlsx");
    {
        saveAndReload(u"Calc Office Open XML"_ustr);
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
        bool bShowLegendEntry = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"ShowLegendEntry"_ustr)
                       >>= bShowLegendEntry);
        CPPUNIT_ASSERT(!bShowLegendEntry);

        Reference<chart2::XChartDocument> xChartDoc2 = getChartDocFromSheet(1, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries2(getDataSeriesFromDoc(xChartDoc2, 0));
        CPPUNIT_ASSERT(xDataSeries2.is());
        Reference<beans::XPropertySet> xPropertySet2(xDataSeries2, uno::UNO_QUERY_THROW);
        Sequence<sal_Int32> deletedLegendEntriesSeq;
        CPPUNIT_ASSERT(xPropertySet2->getPropertyValue(u"DeletedLegendEntries"_ustr)
                       >>= deletedLegendEntriesSeq);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), deletedLegendEntriesSeq[0]);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf60316)
{
    loadFromFile(u"pptx/tdf60316.pptx");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, the shape would have had a solidFill background
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:noFill"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:solidFill"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf130225)
{
    loadFromFile(u"docx/piechart_deleted_legend_entry.docx");
    saveAndReload(u"Office Open XML Text"_ustr);
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropertySet(xDataSeries, uno::UNO_QUERY_THROW);
    Sequence<sal_Int32> deletedLegendEntriesSeq;
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"DeletedLegendEntries"_ustr)
                   >>= deletedLegendEntriesSeq);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), deletedLegendEntriesSeq[0]);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf59857)
{
    loadFromFile(u"ods/tdf59857.ods");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:floor/c:spPr/a:ln/a:noFill"_ostr, 1);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:floor/c:spPr/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, u"cccccc"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:backWall/c:spPr/a:ln/a:noFill"_ostr, 0);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:backWall/c:spPr/a:ln/a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, u"b3b3b3"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf126076)
{
    loadFromFile(u"xlsx/auto_marker_excel10.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 12: all series exported with square markers
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:marker/c:symbol[@val='square']"_ostr,
        0);
    // instead of skipping markers
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:lineChart/c:ser/c:marker"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf75330)
{
    loadFromFile(u"ods/legend_overlay.ods");
    saveAndReload(u"calc8"_ustr);
    {
        uno::Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart::XChartDocument> xChartDoc(xChart2Doc, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
        Reference<beans::XPropertySet> xPropertySet(xLegend, uno::UNO_QUERY_THROW);
        bool bOverlay = false;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Overlay"_ustr) >>= bOverlay);
        CPPUNIT_ASSERT(bOverlay);
    }
    saveAndReload(u"Calc Office Open XML"_ustr);
    {
        uno::Reference<chart2::XChartDocument> xChart2Doc = getChartDocFromSheet(0, mxComponent);
        uno::Reference<chart::XChartDocument> xChartDoc(xChart2Doc, uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xLegend = xChartDoc->getLegend();
        Reference<beans::XPropertySet> xPropertySet(xLegend, uno::UNO_QUERY_THROW);
        bool bOverlay = false;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Overlay"_ustr) >>= bOverlay);
        CPPUNIT_ASSERT(bOverlay);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf127792)
{
    loadFromFile(u"docx/MSO_axis_position.docx");
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr, "val"_ostr,
                u"between"_ustr);

    pXmlDoc = parseExport(u"word/charts/chart2.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:valAx/c:crossBetween"_ostr, "val"_ostr,
                u"midCat"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf131979)
{
    loadFromFile(u"ods/tdf131115.ods");
    {
        saveAndReload(u"calc8"_ustr);
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet;
        xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
        bool blinknumberformattosource = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT)
                       >>= blinknumberformattosource);
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to false.",
                               !blinknumberformattosource);
    }

    loadFromFile(u"ods/tdf131979.ods");
    {
        saveAndReload(u"calc8"_ustr);
        Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
        CPPUNIT_ASSERT(xDataSeries.is());
        Reference<beans::XPropertySet> xPropertySet;
        xPropertySet.set(xDataSeries->getDataPointByIndex(2), uno::UNO_SET_THROW);
        bool blinknumberformattosource = true;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT)
                       >>= blinknumberformattosource);
        CPPUNIT_ASSERT_MESSAGE("\"LinkNumberFormatToSource\" should be set to true.",
                               blinknumberformattosource);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf132076)
{
    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    {
        loadFromFile(u"ods/tdf132076.ods");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:numFmt"_ostr,
                    "formatCode"_ostr, u"dd"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:catAx/c:numFmt"_ostr,
                    "sourceLinked"_ostr, u"0"_ustr);
    }
    {
        loadFromFile(u"xlsx/tdf132076.xlsx");
        save(u"Calc Office Open XML"_ustr);
        xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:numFmt"_ostr,
                    "formatCode"_ostr, u"dd"_ustr);
        assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:numFmt"_ostr,
                    "sourceLinked"_ostr, u"0"_ustr);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf125812)
{
    loadFromFile(u"odp/ellipticalGradientFill.odp");
    save(u"Impress MS PowerPoint 2007 XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path"_ostr,
                "path"_ostr, u"circle"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect"_ostr,
                "l"_ostr, u"50000"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect"_ostr,
                "t"_ostr, u"49000"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect"_ostr,
                "r"_ostr, u"50000"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:gradFill/a:path/a:fillToRect"_ostr,
                "b"_ostr, u"51000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf133190)
{
    loadFromFile(u"xlsx/tdf133190_tdf133191.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test word wrap of data point label
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[1]/c:txPr/a:bodyPr"_ostr,
        "wrap"_ostr, u"none"_ustr);
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[2]/c:txPr/a:bodyPr"_ostr,
        "wrap"_ostr, u"square"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf133191)
{
    loadFromFile(u"xlsx/tdf133190_tdf133191.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test rotation of data point label
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:dLbl[3]/c:txPr/a:bodyPr"_ostr,
        "rot"_ostr, u"-4500000"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf132594)
{
    loadFromFile(u"xlsx/chart_pie2007.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:cat"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf134255)
{
    loadFromFile(u"docx/tdf134255.docx");
    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // import test
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    Reference<beans::XPropertySet> xPropSet(xDataSeries, UNO_QUERY_THROW);
    bool bWrap = false;
    CPPUNIT_ASSERT((xPropSet->getPropertyValue(u"TextWordWrap"_ustr) >>= bWrap));
    CPPUNIT_ASSERT(bWrap);

    // FIXME: validation error in OOXML export: Errors: 11
    skipValidation();

    // export test
    save(u"Office Open XML Text"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:pieChart/c:ser/c:dLbls/c:txPr/a:bodyPr"_ostr,
                "wrap"_ostr, u"square"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf134977)
{
    loadFromFile(u"xlsx/custom_data_label.xlsx");

    //import test
    uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());
    Reference<chart2::XDataSeries> xDataSeries = getDataSeriesFromDoc(xChartDoc, 0);
    CPPUNIT_ASSERT(xDataSeries.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xDataSeries->getDataPointByIndex(0),
                                                     uno::UNO_SET_THROW);
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;
    float nFontSize;
    xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
    aFields[0]->getPropertyValue(u"CharHeight"_ustr) >>= nFontSize;
    CPPUNIT_ASSERT_EQUAL(static_cast<float>(9), nFontSize);

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    //export test
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:dLbls/c:dLbl/c:tx/c:rich/a:p/"
                "a:r/a:rPr"_ostr,
                "sz"_ostr, u"900"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf123647)
{
    loadFromFile(u"xlsx/empty_chart.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf136267)
{
    loadFromFile(u"xlsx/tdf136267.xlsx");

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:cat/c:strRef/c:strCache/c:pt/c:v"_ostr,
        u"John"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testDataLabelPlacementPieChart)
{
    loadFromFile(u"xlsx/tdf134978.xlsx");
    saveAndReload(u"calc8"_ustr);
    uno::Reference<chart::XChartDocument> xChartDoc(getChartCompFromSheet(0, 0, mxComponent),
                                                    UNO_QUERY_THROW);
    // test the placement of the manually positioned label
    Reference<beans::XPropertySet> xDataPointPropSet(
        xChartDoc->getDiagram()->getDataPointProperties(2, 0), uno::UNO_SET_THROW);
    uno::Any aAny = xDataPointPropSet->getPropertyValue(u"LabelPlacement"_ustr);
    CPPUNIT_ASSERT(aAny.hasValue());
    sal_Int32 nLabelPlacement = 0;
    CPPUNIT_ASSERT(aAny >>= nLabelPlacement);
    CPPUNIT_ASSERT_EQUAL(chart::DataLabelPlacement::OUTSIDE, nLabelPlacement);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf137917)
{
    loadFromFile(u"xlsx/tdf137917.xlsx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:baseTimeUnit"_ostr,
                "val"_ostr, u"days"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:majorUnit"_ostr, "val"_ostr,
                u"1"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:majorTimeUnit"_ostr,
                "val"_ostr, u"months"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:minorUnit"_ostr, "val"_ostr,
                u"7"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:dateAx/c:minorTimeUnit"_ostr,
                "val"_ostr, u"days"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf138204)
{
    loadFromFile(u"xlsx/tdf138204.xlsx");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    struct CustomLabelsTestData
    {
        sal_Int32 nSeriesIdx;
        sal_Int32 nNumFields;
        // First field attributes.
        chart2::DataPointCustomLabelFieldType eFieldType;
        OUString aCellRange;
        OUString aString;
    };

    const CustomLabelsTestData aTestEntries[2] = {
        {
            // series id of c:ser[1] is 0.
            0, // nSeriesIdx
            1, // nNumFields
            chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLRANGE,
            u"Munka1!$F$9"_ustr, // aCellRange
            u"67,5%"_ustr, // aString
        },
        {

            // series id of c:ser[2] is 1.
            1, // nSeriesIdx
            1, // nNumFields
            chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLRANGE,
            u"Munka1!$G$9"_ustr, // aCellRange
            u"32,3%"_ustr, // aString
        },
    };

    for (const auto& aTestEntry : aTestEntries)
    {
        uno::Reference<chart2::XDataSeries> xDataSeries(
            getDataSeriesFromDoc(xChartDoc, aTestEntry.nSeriesIdx));
        CPPUNIT_ASSERT(xDataSeries.is());

        uno::Reference<beans::XPropertySet> xPropertySet;
        uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;
        xPropertySet.set(xDataSeries->getDataPointByIndex(0), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
        CPPUNIT_ASSERT_EQUAL(aTestEntry.nNumFields, aFields.getLength());

        CPPUNIT_ASSERT_EQUAL(aTestEntry.eFieldType, aFields[0]->getFieldType());
        CPPUNIT_ASSERT_EQUAL(aTestEntry.aCellRange, aFields[0]->getCellRange());
        CPPUNIT_ASSERT_EQUAL(aTestEntry.aString, aFields[0]->getString());
    }

    // FIXME: validation error in OOXML export: Errors: 2
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the first data label field type
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:dLbls/c:dLbl/c:tx/c:rich/a:p/a:fld"_ostr,
        "type"_ostr, u"CELLRANGE"_ustr);

    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:dLbls/c:dLbl/c:tx/c:rich/a:p/a:fld"_ostr,
        "type"_ostr, u"CELLRANGE"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf138181)
{
    loadFromFile(u"xlsx/piechart_deleted_legendentry.xlsx");
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    Reference<drawing::XShape> xLegendEntry1, xLegendEntry2, xLegendEntry3;

    // first legend entry is visible
    xLegendEntry1 = getShapeByName(
        xShapes, u"CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=0:LegendEntry=0"_ustr);
    CPPUNIT_ASSERT(xLegendEntry1.is());

    // second legend entry is not visible
    xLegendEntry2 = getShapeByName(
        xShapes, u"CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=1:LegendEntry=0"_ustr);
    CPPUNIT_ASSERT(!xLegendEntry2.is());

    // third legend entry is visible
    xLegendEntry3 = getShapeByName(
        xShapes, u"CID/MultiClick/D=0:CS=0:CT=0:Series=0:Point=2:LegendEntry=0"_ustr);
    CPPUNIT_ASSERT(xLegendEntry3.is());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testCustomShapeText)
{
    loadFromFile(u"ods/tdf72776.ods");
    saveAndReload(u"calc8"_ustr);
    Reference<chart::XChartDocument> xChartDoc(getChartDocFromSheet(0, mxComponent),
                                               UNO_QUERY_THROW);
    // test that the text of custom shape exists inside the chart
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());

    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xRange->getString().isEmpty());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testuserShapesXLSX)
{
    loadFromFile(u"xlsx/tdf128621.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    // test that the custom shape exists
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());
    // test type of shape
    CPPUNIT_ASSERT(xCustomShape->getShapeType().endsWith("CustomShape"));
    // test custom shape position
    awt::Point aPosition = xCustomShape->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1356, aPosition.X, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9107, aPosition.Y, 300);
    // test custom shape size
    awt::Size aSize = xCustomShape->getSize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9520, aSize.Width, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1805, aSize.Height, 300);
    // test custom shape text
    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xRange->getString().isEmpty());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testuserShapesDOCX)
{
    loadFromFile(u"docx/tdf143130.docx");
    saveAndReload(u"Office Open XML Text"_ustr);

    Reference<chart2::XChartDocument> xChartDoc(getChartDocFromWriter(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xChartDoc.is());

    // test that the custom shape exists
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());
    // test type of shape
    CPPUNIT_ASSERT(xCustomShape->getShapeType().endsWith("CustomShape"));
    // test custom shape position
    awt::Point aPosition = xCustomShape->getPosition();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(9824, aPosition.X, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(547, aPosition.Y, 300);
    // test custom shape size
    awt::Size aSize = xCustomShape->getSize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1848, aSize.Width, 300);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1003, aSize.Height, 300);
    // test custom shape text
    Reference<text::XText> xRange(xCustomShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(!xRange->getString().isEmpty());
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testGraphicBlipXLSX)
{
    loadFromFile(u"xlsx/tdf143127.xlsx");
    saveAndReload(u"Calc Office Open XML"_ustr);

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    // test that the Graphic shape exists
    Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, UNO_QUERY_THROW);
    Reference<drawing::XDrawPage> xDrawPage(xDrawPageSupplier->getDrawPage(), UNO_SET_THROW);
    Reference<drawing::XShape> xCustomShape(xDrawPage->getByIndex(1), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xCustomShape.is());
    // test type of shape
    CPPUNIT_ASSERT(xCustomShape->getShapeType().endsWith("GraphicObjectShape"));
    Reference<XPropertySet> xShapeProps(xCustomShape, UNO_QUERY);

    uno::Reference<graphic::XGraphic> xGraphic;
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"Graphic"_ustr) >>= xGraphic);

    Graphic aGraphic(xGraphic);
    GfxLink aLink = aGraphic.GetGfxLink();
    std::size_t nDataSize = aLink.GetDataSize();

    // test the image size is bigger then 0.
    CPPUNIT_ASSERT_GREATER(size_t(0), nDataSize);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testNameRangeXLSX)
{
    loadFromFile(u"xlsx/chart_with_name_range.xlsx");
    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // test the syntax of local range name on the local sheet.
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:cat/c:strRef/c:f"_ostr,
                       u"Sheet1!local_name_range"_ustr);
    // test the syntax of a global range name.
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser/c:val/c:numRef/c:f"_ostr,
                       u"[0]!series1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testTdf143942)
{
    loadFromFile(u"xlsx/tdf143942.xlsx");

    Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference<chart2::XDataSeries> xDataSeries(getDataSeriesFromDoc(xChartDoc, 0));
    CPPUNIT_ASSERT(xDataSeries.is());

    constexpr size_t nLabels = 4;
    OUString aCellRange = u"Sheet1!$A$2:$A$5"_ustr;
    OUString aLabels[nLabels] = {
        u"Test1"_ustr,
        u"Test2"_ustr,
        u"Tes3"_ustr,
        u"Test4"_ustr,
    };

    uno::Reference<beans::XPropertySet> xPropertySet;
    uno::Sequence<uno::Reference<chart2::XDataPointCustomLabelField>> aFields;
    for (size_t i = 0; i < nLabels; ++i)
    {
        xPropertySet.set(xDataSeries->getDataPointByIndex(i), uno::UNO_SET_THROW);
        xPropertySet->getPropertyValue(u"CustomLabelFields"_ustr) >>= aFields;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aFields.getLength());
        CPPUNIT_ASSERT_EQUAL(
            chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_CELLRANGE,
            aFields[0]->getFieldType());
        CPPUNIT_ASSERT_EQUAL(aCellRange, aFields[0]->getCellRange());
        CPPUNIT_ASSERT_EQUAL(aLabels[i], aFields[0]->getString());
    }

    // FIXME: validation error in OOXML export: Errors: 4
    skipValidation();

    save(u"Calc Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"xl/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:extLst/c:ext"_ostr,
                "uri"_ostr, u"{02D57815-91ED-43cb-92C2-25804820EDAC}"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:extLst/c:ext/"
                "c15:datalabelsRange/c15:dlblRangeCache/c:ptCount"_ostr,
                "val"_ostr, u"4"_ustr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:extLst/c:ext/"
                       "c15:datalabelsRange/c15:f"_ostr,
                       aCellRange);
    for (size_t i = 0; i < nLabels; ++i)
    {
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dLbls/c:dLbl["
                        + OString::number(i + 1) + "]/c:tx/c:rich/a:p/a:fld",
                    "type"_ostr, u"CELLRANGE"_ustr);
        assertXPath(pXmlDoc,
                    "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:dLbls/c:dLbl["
                        + OString::number(i + 1) + "]/c:extLst/c:ext/c15:showDataLabelsRange",
                    "val"_ostr, u"1"_ustr);
        // Check if the actual label is stored under c15:datalabelsRange
        assertXPathContent(pXmlDoc,
                           "/c:chartSpace/c:chart/c:plotArea/c:scatterChart/c:ser[1]/c:extLst/"
                           "c:ext/c15:datalabelsRange/c15:dlblRangeCache/c:pt["
                               + OString::number(i + 1) + "]/c:v",
                           aLabels[i]);
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testDateCategoriesPPTX)
{
    loadFromFile(u"pptx/bnc889755.pptx");

    // FIXME: validation error in OOXML export: Errors: 1
    skipValidation();

    save(u"Impress Office Open XML"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/charts/chart1.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    constexpr size_t nCats = 16;
    double aDates[nCats] = {
        41183, 41214, 41244, 41275, 41306, 41334, 41365, 41395,
        41426, 41456, 41487, 41518, 41548, 41579, 41609, 41640,
    };

    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat"_ostr);
    assertXPathContent(pXmlDoc,
                       "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:numRef/"
                       "c:numCache/c:formatCode"_ostr,
                       u"mmm\\-yy"_ustr);
    assertXPath(
        pXmlDoc,
        "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:numRef/c:numCache/c:ptCount"_ostr,
        "val"_ostr, OUString::number(nCats));

    for (size_t i = 0; i < nCats; ++i)
    {
        assertXPath(
            pXmlDoc,
            "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:numRef/c:numCache/c:pt["
                + OString::number(i + 1) + "]",
            "idx"_ostr, OUString::number(i));
        assertXPathContent(
            pXmlDoc,
            "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:cat/c:numRef/c:numCache/c:pt["
                + OString::number(i + 1) + "]/c:v",
            OUString::number(aDates[i]));
    }
}

CPPUNIT_TEST_FIXTURE(Chart2ExportTest2, testDataTableImportExport)
{
    loadFromFile(u"xlsx/ChartDataTable.xlsx");
    {
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        auto xDiagram = xChartDoc->getFirstDiagram();
        CPPUNIT_ASSERT(xDiagram.is());
        auto xDataTable = xDiagram->getDataTable();
        CPPUNIT_ASSERT(xDataTable.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xDataTable, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPropertySet.is());
        bool bHBorder;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"HBorder"_ustr) >>= bHBorder);
        CPPUNIT_ASSERT_EQUAL(true, bHBorder);
        bool bVBorder;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"VBorder"_ustr) >>= bVBorder);
        CPPUNIT_ASSERT_EQUAL(true, bVBorder);
        bool bOutline;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Outline"_ustr) >>= bOutline);
        CPPUNIT_ASSERT_EQUAL(false, bOutline);
        bool bKeys;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Keys"_ustr) >>= bKeys);
        CPPUNIT_ASSERT_EQUAL(false, bKeys);
    }
    saveAndReload(u"calc8"_ustr);
    {
        uno::Reference<chart2::XChartDocument> xChartDoc = getChartDocFromSheet(0, mxComponent);
        CPPUNIT_ASSERT(xChartDoc.is());
        auto xDiagram = xChartDoc->getFirstDiagram();
        CPPUNIT_ASSERT(xDiagram.is());
        auto xDataTable = xDiagram->getDataTable();
        CPPUNIT_ASSERT(xDataTable.is());
        uno::Reference<beans::XPropertySet> xPropertySet(xDataTable, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPropertySet.is());
        bool bHBorder;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"HBorder"_ustr) >>= bHBorder);
        CPPUNIT_ASSERT_EQUAL(true, bHBorder);
        bool bVBorder;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"VBorder"_ustr) >>= bVBorder);
        CPPUNIT_ASSERT_EQUAL(true, bVBorder);
        bool bOutline;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Outline"_ustr) >>= bOutline);
        CPPUNIT_ASSERT_EQUAL(false, bOutline);
        bool bKeys;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue(u"Keys"_ustr) >>= bKeys);
        CPPUNIT_ASSERT_EQUAL(false, bKeys);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
