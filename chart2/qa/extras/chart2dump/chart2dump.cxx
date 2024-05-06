/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <charttest.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

#include <editeng/unoprnms.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <fstream>
#include <string_view>

#if defined(X86)
#define INT_EPS     2.1
#else
#define INT_EPS     0.1
#endif

#define DECLARE_DUMP_TEST(TestName, BaseClass, DumpMode) \
    class TestName : public BaseClass { \
        protected:\
            virtual OUString getTestName() override { return u"" #TestName ""_ustr; } \
        public:\
            TestName() : BaseClass(DumpMode) {}; \
            CPPUNIT_TEST_SUITE(TestName); \
            CPPUNIT_TEST(verify); \
            CPPUNIT_TEST_SUITE_END(); \
            virtual void verify() override;\
    };\
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()


#define CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(aActual) \
    if(isInDumpMode()) \
        writeActual(OUString::number(aActual), u"" #aActual ""_ustr); \
    else \
        { \
            OString sTestFileName = OUStringToOString(getTestFileName(), RTL_TEXTENCODING_UTF8); \
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Failing test file is: " + sTestFileName).getStr(), readExpected(u ## #aActual), OUString(OUString::number(aActual))); \
        }

#define CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aActual, EPS_) \
    if(isInDumpMode()) \
        writeActual(OUString::number(aActual), u"" #aActual ""_ustr); \
        else \
        { \
            OString sTestFileName = OUStringToOString(getTestFileName(), RTL_TEXTENCODING_UTF8); \
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(OString("Failing test file is: " + sTestFileName).getStr(), readExpectedDouble(u ## #aActual), aActual, EPS_); \
        }

#define CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(aActual) \
    if(isInDumpMode()) \
        writeActual(aActual, u"" #aActual ""_ustr); \
    else \
    { \
        OString sTestFileName = OUStringToOString(getTestFileName(), RTL_TEXTENCODING_UTF8); \
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Failing test file is: " + sTestFileName).getStr(), readExpected(u ## #aActual), aActual.trim()); \
    }

#define CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aActual, EPS_) \
    if(isInDumpMode()) \
        writeActualTransformation(aActual, u"" #aActual ""_ustr); \
    else \
    { \
        OUString expectedTransform; \
        if (!readAndCheckTransformation (aActual, u ## #aActual, EPS_, expectedTransform)) \
        { \
            OString sTestFileName = OUStringToOString(getTestFileName(), RTL_TEXTENCODING_UTF8); \
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Failing test file is: " + sTestFileName).getStr(), expectedTransform, transformationToOneLineString(aActual)); \
        } \
    }

class Chart2DumpTest : public ChartTest
{
protected:
    Chart2DumpTest(bool bDumpMode)
        : ChartTest(u"/chart2/qa/extras/chart2dump/data/"_ustr)
    {
        m_bDumpMode = bDumpMode;
    }

    virtual ~Chart2DumpTest() override
    {
    }

    void CPPUNIT_DUMP_ASSERT_NOTE(OUString const & Note) {
        if(isInDumpMode())
            writeNote(Note);
        else
            readNote(Note);
    }

    bool isInDumpMode () const {return m_bDumpMode;}

    virtual OUString getTestName() { return OUString(); }
    OUString const & getTestFileName() const { return m_sTestFileName; }
    OUString getReferenceDirName()
    {
        return "/chart2/qa/extras/chart2dump/reference/" + getTestName().toAsciiLowerCase() + "/";
    }

    void setTestFileName (const OUString& sName)
    {
        m_sTestFileName = sName;

        OUString sFileName = m_sTestFileName;
        assert(sFileName.lastIndexOf('.') < sFileName.getLength());
        sFileName = OUString::Concat(sFileName.subView(0, sFileName.lastIndexOf('.'))) + ".txt";
        if (!m_bDumpMode)
        {
            if (m_aReferenceFile.is_open())
                m_aReferenceFile.close();
            OString sReferenceFile = OUStringToOString(Concat2View(m_directories.getPathFromSrc(getReferenceDirName()) + sFileName), RTL_TEXTENCODING_UTF8);
            m_aReferenceFile.open(sReferenceFile.getStr(), std::ios_base::in);
            CPPUNIT_ASSERT_MESSAGE(OString("Can't open reference file: " + sReferenceFile).getStr(), m_aReferenceFile.is_open());
        }
        else
        {
            if (m_aDumpFile.is_open())
                m_aDumpFile.close();
            OString sDumpFile = OUStringToOString(Concat2View(m_directories.getPathFromSrc(getReferenceDirName()) + sFileName), RTL_TEXTENCODING_UTF8);
            m_aDumpFile.open(sDumpFile.getStr(), std::ios_base::out | std::ofstream::binary | std::ofstream::trunc);
            CPPUNIT_ASSERT_MESSAGE(OString("Can't open dump file: " + sDumpFile).getStr(), m_aDumpFile.is_open());
        }
    }

    virtual void verify()
    {
        CPPUNIT_FAIL("verify method must be overridden");
    }

    OUString readExpected(std::u16string_view sCheck)
    {
        assert(!m_bDumpMode);
        assert(m_aReferenceFile.is_open());
        std::string sTemp;
        getline(m_aReferenceFile, sTemp);
        OString sAssertMessage =
            "The reference file does not contain the right content. Maybe it needs an update:"
            + OUStringToOString(m_sTestFileName, RTL_TEXTENCODING_UTF8);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sAssertMessage.getStr(), OUString(OUString::Concat("// ") + sCheck), OUString(sTemp.data(), sTemp.length(), RTL_TEXTENCODING_UTF8));
        getline(m_aReferenceFile, sTemp);
        return OUString(sTemp.data(), sTemp.length(), RTL_TEXTENCODING_UTF8);
    }

    void writeActual(std::u16string_view sActualValue, const OUString& sCheck)
    {
        assert(m_bDumpMode);
        assert(m_aDumpFile.is_open());
        m_aDumpFile << "// " << sCheck << "\n";   // Add check string to make dump file readable
        m_aDumpFile << OUString(sActualValue) << "\n";      // Write out the checked value, will be used as reference later
    }

    void readNote(std::u16string_view sNote)
    {
        assert(!m_bDumpMode);
        assert(m_aReferenceFile.is_open());
        std::string sTemp;
        getline(m_aReferenceFile, sTemp);
        OString sAssertMessage =
            "The reference file does not contain the right content. Maybe it needs an update:"
            + OUStringToOString(m_sTestFileName, RTL_TEXTENCODING_UTF8);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sAssertMessage.getStr(), OUString(OUString::Concat("/// ") + sNote), OUString(sTemp.data(), sTemp.length(), RTL_TEXTENCODING_UTF8));
    }

    void writeNote(const OUString& sNote)
    {
        assert(m_bDumpMode);
        assert(m_aDumpFile.is_open());
        m_aDumpFile << "/// " << sNote << "\n";
    }

    double readExpectedDouble(std::u16string_view sCheck)
    {
        OUString sExpected = readExpected(sCheck);
        return sExpected.toDouble();
    }

    void writeActualTransformation(const drawing::HomogenMatrix3& rTransform, const OUString& sCheck)
    {
        writeActual(transformationToOneLineString(rTransform), sCheck);
    }

    bool readAndCheckTransformation(const drawing::HomogenMatrix3& rTransform, std::u16string_view sCheck, const double fEPS, OUString& rExpectedTransform)
    {
        rExpectedTransform = readExpected(sCheck); // Reference transformation string

        // Convert string back to a transformation;
        drawing::HomogenMatrix3 aExpectedTransform;
        sal_Int32 nIdx {0};
        aExpectedTransform.Line1.Column1 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line1.Column2 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line1.Column3 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line2.Column1 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line2.Column2 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line2.Column3 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line3.Column1 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line3.Column2 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));
        aExpectedTransform.Line3.Column3 = o3tl::toDouble(o3tl::getToken(rExpectedTransform, 0, ';', nIdx));

        // Check the equality of the two transformation
        return (std::abs(aExpectedTransform.Line1.Column1 - rTransform.Line1.Column1) < fEPS &&
            std::abs(aExpectedTransform.Line1.Column2 - rTransform.Line1.Column2) < fEPS &&
            std::abs(aExpectedTransform.Line1.Column3 - rTransform.Line1.Column3) < fEPS &&
            std::abs(aExpectedTransform.Line2.Column1 - rTransform.Line2.Column1) < fEPS &&
            std::abs(aExpectedTransform.Line2.Column2 - rTransform.Line2.Column2) < fEPS &&
            std::abs(aExpectedTransform.Line2.Column3 - rTransform.Line2.Column3) < fEPS &&
            std::abs(aExpectedTransform.Line3.Column1 - rTransform.Line3.Column1) < fEPS &&
            std::abs(aExpectedTransform.Line3.Column2 - rTransform.Line3.Column2) < fEPS &&
            std::abs(aExpectedTransform.Line3.Column3 - rTransform.Line3.Column3) < fEPS);
    }

    OUString sequenceToOneLineString(const uno::Sequence<OUString>& rSeq)
    {
        OUStringBuffer aBuffer;
        for (const OUString& seqItem : rSeq)
        {
            aBuffer.append(seqItem + ";");
        }
        return aBuffer.makeStringAndClear();
    }

    OUString doubleVectorToOneLineString(const std::vector<double>& rVector)
    {
        OUStringBuffer aBuffer;
        for (const double& vectorItem : rVector)
        {
            aBuffer.append(OUString::number(vectorItem) + ";");
        }
        return aBuffer.makeStringAndClear();
    }

    OUString transformationToOneLineString(const drawing::HomogenMatrix3& rTransform)
    {
        return OUString::number(rTransform.Line1.Column1) + ";" + OUString::number(rTransform.Line1.Column2) + ";" + OUString::number(rTransform.Line1.Column3) + ";" +
            OUString::number(rTransform.Line2.Column1) + ";" + OUString::number(rTransform.Line2.Column2) + ";" + OUString::number(rTransform.Line2.Column3) + ";" +
            OUString::number(rTransform.Line3.Column1) + ";" + OUString::number(rTransform.Line3.Column2) + ";" + OUString::number(rTransform.Line3.Column3);
    }

private:
    OUString         m_sTestFileName;
    bool             m_bDumpMode;
    std::ifstream    m_aReferenceFile;
    std::ofstream    m_aDumpFile;
};

DECLARE_DUMP_TEST(ChartDataTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"simple_chart.ods"_ustr,
        u"multiple_categories.ods"_ustr
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        setTestFileName(aTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc (getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);

        // Check title
        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XTitled> xTitled(xChartDoc, uno::UNO_QUERY_THROW);
        uno::Reference<chart2::XTitle> xTitle = xTitled->getTitleObject();
        if(xTitle.is())
        {
            OUString sChartTitle = getTitleString(xTitled);
            CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sChartTitle);
        }

        // Check chart type
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());
        OUString sChartType = xChartType->getChartType();
        CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sChartType);

        // Check axis titles and number format
        // x Axis
        Reference<chart2::XAxis> xAxis = getAxisFromDoc(xChartDoc2, 0, 0, 0);
        Reference<chart2::XTitled> xAxisTitled(xAxis, UNO_QUERY_THROW);
        uno::Reference<chart2::XTitle> xAxisTitle = xAxisTitled->getTitleObject();
        if (xAxisTitle.is())
        {
            OUString sXAxisTitle = getTitleString(xAxisTitled);
            CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sXAxisTitle);
        }
        sal_Int32 nXAxisNumberFormat = getNumberFormatFromAxis(xAxis);
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nXAxisNumberFormat);
        sal_Int16 nXAxisNumberType = getNumberFormatType(xChartDoc2, nXAxisNumberFormat);
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nXAxisNumberType);

        // y Axis
        xAxis.set(getAxisFromDoc(xChartDoc2, 0, 1, 0));
        xAxisTitled.set(xAxis, UNO_QUERY_THROW);
        xAxisTitle.set(xAxisTitled->getTitleObject());
        if (xAxisTitle.is())
        {
            OUString sYAxisTitle = getTitleString(xAxisTitled);
            CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sYAxisTitle);
        }
        sal_Int32 nYAxisNumberFormat = getNumberFormatFromAxis(xAxis);
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nYAxisNumberFormat);
        sal_Int16 nYAxisNumberType = getNumberFormatType(xChartDoc2, nYAxisNumberFormat);
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nYAxisNumberType);

        // Check column labels
        uno::Reference< chart::XChartDataArray > xChartData(xChartDoc->getData(), UNO_QUERY_THROW);
        uno::Sequence < OUString > aColumnLabels = xChartData->getColumnDescriptions();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(aColumnLabels.getLength());
        OUString sColumnLabels = sequenceToOneLineString(aColumnLabels);
        CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sColumnLabels);

        // Check row labels
        uno::Sequence< OUString > aRowLabels = xChartData->getRowDescriptions();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(aRowLabels.getLength());
        OUString sRowLabels = sequenceToOneLineString(aRowLabels);
        CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sRowLabels);

        // Check Y values
        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(aDataSeriesYValues.size());
        for (const std::vector<double>& aYValuesOfSeries : aDataSeriesYValues)
        {
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(aYValuesOfSeries.size());
            OUString sYValuesOfSeries = doubleVectorToOneLineString(aYValuesOfSeries);
            CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sYValuesOfSeries);
        }

        // Check source ranges
        for (size_t nIndex = 0; nIndex < aDataSeriesYValues.size(); ++nIndex)
        {
            Reference< chart2::data::XDataSequence > xDataSeq = getDataSequenceFromDocByRole(xChartDoc2, u"values-x", nIndex);
            if (xDataSeq.is())
            {
                OUString aXValuesSourceRange = xDataSeq->getSourceRangeRepresentation();
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(aXValuesSourceRange);
            }
            xDataSeq.set(getDataSequenceFromDocByRole(xChartDoc2, u"values-y", nIndex));
            if (xDataSeq.is())
            {
                OUString aYValuesSourceRange = xDataSeq->getSourceRangeRepresentation();
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(aYValuesSourceRange);
            }
            xDataSeq.set(getDataSequenceFromDocByRole(xChartDoc2, u"categories", nIndex));
            if (xDataSeq.is())
            {
                OUString aCategoriesSourceRange = xDataSeq->getSourceRangeRepresentation();
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(aCategoriesSourceRange);
            }
        }
    }
}

DECLARE_DUMP_TEST(LegendTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"legend_on_right_side.odp"_ustr,
        u"legend_on_bottom.odp"_ustr,
        u"legend_on_left_side.odp"_ustr,
        u"legend_on_top.odp"_ustr,
        u"many_legend_entries.odp"_ustr,
        u"custom_legend_position.odp"_ustr,
        u"multiple_categories.odp"_ustr,
        u"minimal_legend_test.odp"_ustr
    };

    for (const OUString& aTestFile : aTestFiles)
    {
        setTestFileName(aTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0, 0), UNO_SET_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        // Get legend shape
        uno::Reference<drawing::XShape> xLegend = getShapeByName(xShapes, u"CID/D=0:Legend="_ustr);
        CPPUNIT_ASSERT(xLegend.is());

        /* Check legend position and size
        awt::Point aLegendPosition = xLegend->getPosition();
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendPosition.X, INT_EPS);
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendPosition.Y, INT_EPS);
        awt::Size aLegendSize = xLegend->getSize();
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendSize.Width, INT_EPS);
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendSize.Height, INT_EPS);*/

        // Check legend entries
        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());
        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        size_t nLegendEntryCount = aDataSeriesYValues.size();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nLegendEntryCount);
        // Check legend entries geometry
        for (size_t nSeriesIndex = 0; nSeriesIndex < nLegendEntryCount; ++nSeriesIndex)
        {
            uno::Reference<drawing::XShape> xLegendEntry = getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=" + OUString::number(nSeriesIndex) + ":LegendEntry=0");
            CPPUNIT_ASSERT(xLegendEntry.is());

            /* Check position and size
            awt::Point aLegendEntryPosition = xLegendEntry->getPosition();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendEntryPosition.X, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendEntryPosition.Y, INT_EPS);
            awt::Size aLegendEntrySize = xLegendEntry->getSize();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendEntrySize.Height, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLegendEntrySize.Width, INT_EPS);

            // Check transformation
            Reference< beans::XPropertySet > xLegendEntryPropSet(xLegendEntry, UNO_QUERY_THROW);
            drawing::HomogenMatrix3 aLegendEntryTransformation;
            xLegendEntryPropSet->getPropertyValue("Transformation") >>= aLegendEntryTransformation;
            CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aLegendEntryTransformation, INT_EPS);*/

            uno::Reference<container::XIndexAccess> xLegendEntryContainer(xLegendEntry, UNO_QUERY_THROW);
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(xLegendEntryContainer->getCount());
            for (sal_Int32 nEntryGeometryElement = 1; nEntryGeometryElement < xLegendEntryContainer->getCount(); ++nEntryGeometryElement)
            {
                uno::Reference<drawing::XShape> xLegendEntryGeom(xLegendEntryContainer->getByIndex(nEntryGeometryElement), UNO_QUERY_THROW);

                // Check geometry
                uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xLegendEntryGeom, uno::UNO_QUERY_THROW);
                OUString sEntryGeomShapeType = xShapeDescriptor->getShapeType();
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sEntryGeomShapeType);

                // Check display color
                Reference< beans::XPropertySet > xPropSet(xLegendEntryGeom, UNO_QUERY_THROW);
                util::Color aEntryGeomColor = 0;
                xPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aEntryGeomColor;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aEntryGeomColor));
            }
        }
        // Check legend entries' text
        uno::Reference<container::XIndexAccess> xLegendContainer(xLegend, UNO_QUERY_THROW);
        for (sal_Int32 i = 0; i < xLegendContainer->getCount(); ++i)
        {
            uno::Reference<drawing::XShape> xShape(xLegendContainer->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xShape, uno::UNO_QUERY_THROW);
            OUString sShapeType = xShapeDescriptor->getShapeType();

            if (sShapeType == "com.sun.star.drawing.TextShape")
            {
                uno::Reference<text::XText> xLegendEntryText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(xLegendEntryText->getString());
            }
        }
    }
}

DECLARE_DUMP_TEST(GridTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"vertical_grid.ods"_ustr,
        u"horizontal_grid.ods"_ustr,
        u"minor_grid.ods"_ustr,
        u"formated_grid_line.ods"_ustr
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        const std::vector<OUString> aGridShapeNames =
        {
            u"CID/D=0:CS=0:Axis=1,0:Grid=0"_ustr, // Major vertical grid
            u"CID/D=0:CS=0:Axis=0,0:Grid=0"_ustr, // Major horizontal grid
            u"CID/D=0:CS=0:Axis=1,0:Grid=0:SubGrid=0"_ustr, // Minor vertical grid
            u"CID/D=0:CS=0:Axis=0,0:Grid=0:SubGrid=0"_ustr  // Minor horizontal grid
        };

        for (const OUString& sGridShapeName : aGridShapeNames)
        {
            uno::Reference<drawing::XShape> xGrid = getShapeByName(xShapes, sGridShapeName);
            if (xGrid.is())
            {
                CPPUNIT_DUMP_ASSERT_NOTE(sGridShapeName);
                // Check position and size
                awt::Point aGridPosition = xGrid->getPosition();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aGridPosition.X, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aGridPosition.Y, INT_EPS);
                awt::Size aGridSize = xGrid->getSize();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aGridSize.Height, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aGridSize.Width, INT_EPS);

                // Check transformation
                Reference< beans::XPropertySet > xPropSet(xGrid, UNO_QUERY_THROW);
                drawing::HomogenMatrix3 aGridTransformation;
                xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aGridTransformation;
                CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aGridTransformation, INT_EPS);

                // Check line properties
                uno::Reference<container::XIndexAccess> xIndexAccess(xGrid, UNO_QUERY_THROW);
                uno::Reference<drawing::XShape> xGridLine(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);
                Reference< beans::XPropertySet > xGridLinePropSet(xGridLine, UNO_QUERY_THROW);
                // Line type
                drawing::LineDash aLineDash;
                xGridLinePropSet->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
                OUString sGridLineDash =
                    OUString::number(static_cast<sal_Int32>(aLineDash.Style)) + ";" + OUString::number(aLineDash.Dots) + ";" + OUString::number(aLineDash.DotLen) +
                    OUString::number(aLineDash.Dashes) + ";" + OUString::number(aLineDash.DashLen) + ";" + OUString::number(aLineDash.Distance);
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sGridLineDash);
                // Line color
                util::Color aLineColor = 0;
                xGridLinePropSet->getPropertyValue(u"LineColor"_ustr) >>= aLineColor;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aLineColor));
                // Line width
                sal_Int32 nLineWidth = 0;
                xGridLinePropSet->getPropertyValue(u"LineWidth"_ustr) >>= nLineWidth;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nLineWidth);
            }
        }
    }
}

DECLARE_DUMP_TEST(AxisGeometryTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"default_formated_axis.odp"_ustr,
        u"axis_special_positioning.odp"_ustr,
        u"formated_axis_lines.odp"_ustr,
        u"rotated_axis_labels.odp"_ustr
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0, 0), UNO_SET_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        const std::vector<OUString> aAxisShapeNames =
        {
            u"CID/D=0:CS=0:Axis=0,0"_ustr, // X Axis
            u"CID/D=0:CS=0:Axis=1,0"_ustr, // Y Axis
        };

        for (const OUString& sAxisShapeName : aAxisShapeNames)
        {
            uno::Reference<drawing::XShape> xXAxis = getShapeByName(xShapes, sAxisShapeName);
            CPPUNIT_ASSERT(xXAxis.is());

            CPPUNIT_DUMP_ASSERT_NOTE(sAxisShapeName);
            // Check position and size
            awt::Point aAxisPosition = xXAxis->getPosition();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAxisPosition.X, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAxisPosition.Y, INT_EPS);
            awt::Size aAxisSize = xXAxis->getSize();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAxisSize.Height, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAxisSize.Width, INT_EPS);

            // Check transformation
            Reference< beans::XPropertySet > xPropSet(xXAxis, UNO_QUERY_THROW);
            drawing::HomogenMatrix3 aAxisTransformation;
            xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aAxisTransformation;
            CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aAxisTransformation, INT_EPS);

            // Check line properties
            uno::Reference<container::XIndexAccess> xIndexAccess(xXAxis, UNO_QUERY_THROW);
            sal_Int32 nAxisGeometriesCount = xIndexAccess->getCount();
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nAxisGeometriesCount);
            uno::Reference<drawing::XShape> xAxisLine(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);
            Reference< beans::XPropertySet > xAxisLinePropSet(xAxisLine, UNO_QUERY_THROW);
            // Line type
            drawing::LineDash aLineDash;
            xAxisLinePropSet->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
            OUString sAxisLineDash =
                OUString::number(static_cast<sal_Int32>(aLineDash.Style)) + ";" + OUString::number(aLineDash.Dots) + ";" + OUString::number(aLineDash.DotLen) +
                OUString::number(aLineDash.Dashes) + ";" + OUString::number(aLineDash.DashLen) + ";" + OUString::number(aLineDash.Distance);
            CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sAxisLineDash);
            // Line color
            util::Color aAxisLineColor = 0;
            xAxisLinePropSet->getPropertyValue(u"LineColor"_ustr) >>= aAxisLineColor;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aAxisLineColor));
            // Line width
            sal_Int32 nAxisLineWidth = 0;
            xAxisLinePropSet->getPropertyValue(u"LineWidth"_ustr) >>= nAxisLineWidth;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nAxisLineWidth);
        }
    }
}

DECLARE_DUMP_TEST(AxisLabelTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"default_formated_axis.odp"_ustr,
        u"rotated_axis_labels.odp"_ustr,
        u"formated_axis_labels.odp"_ustr,
        u"percent_stacked_column_chart.odp"_ustr,
        u"tdf118150.xlsx"_ustr,
        u"date-categories.pptx"_ustr,
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0, 0), UNO_SET_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        const std::vector<OUString> aAxisShapeNames =
        {
            u"CID/D=0:CS=0:Axis=0,0"_ustr, // X Axis
            u"CID/D=0:CS=0:Axis=1,0"_ustr, // Y Axis
        };

        for (const OUString& sAxisShapeName : aAxisShapeNames)
        {
            uno::Reference<drawing::XShape> xXAxis = getShapeByName(xShapes, sAxisShapeName,
                // Axis occurs twice in chart xshape representation so need to get the one related to labels
                [](const uno::Reference<drawing::XShape>& rXShape) -> bool
                {
                    uno::Reference<drawing::XShapes> xAxisShapes(rXShape, uno::UNO_QUERY);
                    CPPUNIT_ASSERT(xAxisShapes.is());
                    uno::Reference<drawing::XShape> xChildShape(xAxisShapes->getByIndex(0), uno::UNO_QUERY);
                    uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor(xChildShape, uno::UNO_QUERY_THROW);
                    return (xShapeDescriptor->getShapeType() == "com.sun.star.drawing.TextShape");
                });
            CPPUNIT_ASSERT(xXAxis.is());
            CPPUNIT_DUMP_ASSERT_NOTE(sAxisShapeName);

            // Check label count
            uno::Reference<container::XIndexAccess> xIndexAccess(xXAxis, UNO_QUERY_THROW);
            sal_Int32 nAxisLabelsCount = xIndexAccess->getCount();
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nAxisLabelsCount);

            // Check labels's text, positioning and font properties
            for (sal_Int32 nLabelIndex = 0; nLabelIndex < nAxisLabelsCount; ++nLabelIndex)
            {
                // Check text
                uno::Reference<text::XTextRange> xLabel(xIndexAccess->getByIndex(nLabelIndex), uno::UNO_QUERY);
                CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(xLabel->getString());

                // Check size and position
                uno::Reference<drawing::XShape> xLabelShape(xLabel, uno::UNO_QUERY);
                /*awt::Point aLabelPosition = xLabelShape->getPosition();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLabelPosition.X, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLabelPosition.Y, INT_EPS);
                awt::Size aLabelSize = xLabelShape->getSize();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLabelSize.Height, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLabelSize.Width, INT_EPS);*/

                // Check transformation
                Reference< beans::XPropertySet > xPropSet(xLabelShape, UNO_QUERY_THROW);
                /*drawing::HomogenMatrix3 aLabelTransformation;
                xPropSet->getPropertyValue("Transformation") >>= aLabelTransformation;
                CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aLabelTransformation, INT_EPS);*/

                // Check font color and height
                util::Color aLabelFontColor = 0;
                xPropSet->getPropertyValue(u"CharColor"_ustr) >>= aLabelFontColor;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aLabelFontColor));
                float fLabelFontHeight = 0.0f;
                xPropSet->getPropertyValue(u"CharHeight"_ustr) >>= fLabelFontHeight;
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(fLabelFontHeight, 1E-12);
            }
        }
    }
}

DECLARE_DUMP_TEST(ColumnBarChartTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"normal_column_chart.ods"_ustr,
        u"stacked_column_chart.ods"_ustr,
        u"percent_stacked_column_chart.ods"_ustr,
        u"column_chart_small_spacing.ods"_ustr,
        u"normal_bar_chart.ods"_ustr,
        u"stacked_bar_chart.ods"_ustr,
        u"percent_stacked_bar_chart.ods"_ustr,
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());
        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        size_t nSeriesCount = aDataSeriesYValues.size();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nSeriesCount);

        for (size_t nSeries = 0; nSeries < nSeriesCount; ++nSeries)
        {
            uno::Reference<drawing::XShape> xSeriesColumnsOrBars = getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=" + OUString::number(nSeries));
            CPPUNIT_ASSERT(xSeriesColumnsOrBars.is());
            CPPUNIT_DUMP_ASSERT_NOTE("Series " + OUString::number(nSeries) + " ColumnsOrBars");

            // Check column/bar count in the series
            uno::Reference<container::XIndexAccess> xIndexAccess(xSeriesColumnsOrBars, UNO_QUERY_THROW);
            sal_Int32 nColumnOrBarCountInSeries = xIndexAccess->getCount();
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nColumnOrBarCountInSeries);

            // Check column/bar fill style and color
            Reference< beans::XPropertySet > xColumnOrBarPropSet(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);
            drawing::FillStyle aSeriesColumnOrBarFillStyle;
            xColumnOrBarPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= aSeriesColumnOrBarFillStyle;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aSeriesColumnOrBarFillStyle));
            util::Color aSeriesColumnOrBarFillColor = 0;
            xColumnOrBarPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aSeriesColumnOrBarFillColor;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aSeriesColumnOrBarFillColor));

            for (sal_Int32 nColumnOrBar = 0; nColumnOrBar < nColumnOrBarCountInSeries; ++nColumnOrBar)
            {
                uno::Reference<drawing::XShape> xColumnOrBar(xIndexAccess->getByIndex(nColumnOrBar), UNO_QUERY_THROW);
                uno::Reference<container::XNamed> xNamedShape(xIndexAccess->getByIndex(nColumnOrBar), uno::UNO_QUERY);
                CPPUNIT_DUMP_ASSERT_NOTE(xNamedShape->getName());

                // Check size and position
                awt::Point aColumnOrBarPosition = xColumnOrBar->getPosition();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aColumnOrBarPosition.X, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aColumnOrBarPosition.Y, INT_EPS);
                awt::Size aColumnOrBarSize = xColumnOrBar->getSize();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aColumnOrBarSize.Height, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aColumnOrBarSize.Width, INT_EPS);

                // Check transformation
                Reference< beans::XPropertySet > xPropSet(xColumnOrBar, UNO_QUERY_THROW);
                drawing::HomogenMatrix3 aColumnOrBarTransformation;
                xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aColumnOrBarTransformation;
                CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aColumnOrBarTransformation, INT_EPS);
            }
        }
    }
}

DECLARE_DUMP_TEST(ChartWallTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"chartwall_auto_adjust_with_titles.ods"_ustr,
        u"chartwall_auto_adjust_without_titles.ods"_ustr,
        u"chartwall_custom_positioning.ods"_ustr
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromDrawImpress(0, 0), UNO_SET_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference<drawing::XShape> xChartWall = getShapeByName(xShapes, u"CID/DiagramWall="_ustr);
        CPPUNIT_ASSERT(xChartWall.is());

        // Check position and size
        /*awt::Point aChartWallPosition = xChartWall->getPosition();
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aChartWallPosition.X, INT_EPS);
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aChartWallPosition.Y, INT_EPS);
        awt::Size aChartWallSize = xChartWall->getSize();
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aChartWallSize.Height, INT_EPS);
        CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aChartWallSize.Width, INT_EPS);*/

        // Check transformation
        Reference< beans::XPropertySet > xPropSet(xChartWall, UNO_QUERY_THROW);
        /*drawing::HomogenMatrix3 aChartWallTransformation;
        xPropSet->getPropertyValue("Transformation") >>= aChartWallTransformation;
        CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aChartWallTransformation, INT_EPS);*/

        // Check fill properties
        drawing::FillStyle aChartWallFillStyle;
        xPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= aChartWallFillStyle;
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aChartWallFillStyle));
        util::Color aChartWallFillColor = 0;
        xPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aChartWallFillColor;
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aChartWallFillColor));

        // Check line properties
        // Line type
        drawing::LineDash aLineDash;
        xPropSet->getPropertyValue(u"LineDash"_ustr) >>= aLineDash;
        OUString sChartWallLineDash =
            OUString::number(static_cast<sal_Int32>(aLineDash.Style)) + ";" + OUString::number(aLineDash.Dots) + ";" + OUString::number(aLineDash.DotLen) +
            OUString::number(aLineDash.Dashes) + ";" + OUString::number(aLineDash.DashLen) + ";" + OUString::number(aLineDash.Distance);
        CPPUNIT_DUMP_ASSERT_STRINGS_EQUAL(sChartWallLineDash);
        // Line color
        util::Color aChartWallLineColor = 0;
        xPropSet->getPropertyValue(u"LineColor"_ustr) >>= aChartWallLineColor;
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aChartWallLineColor));
        // Line width
        sal_Int32 nChartWallLineWidth = 0;
        xPropSet->getPropertyValue(u"LineWidth"_ustr) >>= nChartWallLineWidth;
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nChartWallLineWidth);
    }
}

DECLARE_DUMP_TEST(PieChartTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"normal_pie_chart.ods"_ustr,
        u"rotated_pie_chart.ods"_ustr,
        u"exploded_pie_chart.ods"_ustr,
        u"donut_chart.ods"_ustr,
        u"pie_chart_many_slices.ods"_ustr,
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());

        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        size_t nSeriesCount = aDataSeriesYValues.size();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nSeriesCount);

        for (size_t nSeries = 0; nSeries < nSeriesCount; ++nSeries)
        {
            uno::Reference<drawing::XShape> xSeriesSlices = getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=" + OUString::number(nSeries));
            if (!xSeriesSlices.is())
                break; // Normal pie chart displays only one series
            CPPUNIT_DUMP_ASSERT_NOTE("Series " + OUString::number(nSeries) + " slices");

            // Check slice count in the series
            uno::Reference<container::XIndexAccess> xIndexAccess(xSeriesSlices, UNO_QUERY_THROW);
            sal_Int32 nSlicesCountInSeries = xIndexAccess->getCount();
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nSlicesCountInSeries);

            // Check slices properties
            for (sal_Int32 nSlice = 0; nSlice < nSlicesCountInSeries; ++nSlice)
            {
                uno::Reference<drawing::XShape> xSlice(xIndexAccess->getByIndex(nSlice), UNO_QUERY_THROW);
                uno::Reference<container::XNamed> xNamedShape(xIndexAccess->getByIndex(nSlice), uno::UNO_QUERY);
                OUString sName = xNamedShape->getName();
                CPPUNIT_DUMP_ASSERT_NOTE(sName.copy(sName.lastIndexOf("/D=0")));

                // Check size and position
                awt::Point aSlicePosition = xSlice->getPosition();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aSlicePosition.X, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aSlicePosition.Y, INT_EPS);
                awt::Size aSliceSize = xSlice->getSize();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aSliceSize.Height, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aSliceSize.Width, INT_EPS);

                // Check transformation
                Reference< beans::XPropertySet > xPropSet(xSlice, UNO_QUERY_THROW);
                drawing::HomogenMatrix3 aSliceTransformation;
                xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aSliceTransformation;
                CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aSliceTransformation, INT_EPS);

                // Check slice fill style and color
                drawing::FillStyle aSliceFillStyle;
                xPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= aSliceFillStyle;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aSliceFillStyle));
                util::Color aSliceFillColor = 0;
                xPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aSliceFillColor;
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aSliceFillColor));
            }
        }
    }
}

DECLARE_DUMP_TEST(AreaChartTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"normal_area_chart.ods"_ustr,
        u"stacked_area_chart.ods"_ustr,
        u"percent_stacked_area_chart.ods"_ustr
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());

        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        size_t nSeriesCount = aDataSeriesYValues.size();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nSeriesCount);

        for (size_t nSeries = 0; nSeries < nSeriesCount; ++nSeries)
        {
            uno::Reference<drawing::XShape> xSeries = getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=" + OUString::number(nSeries));
            CPPUNIT_ASSERT(xSeries.is());
            CPPUNIT_DUMP_ASSERT_NOTE("Series " + OUString::number(nSeries));

            // One area for one series
            uno::Reference<container::XIndexAccess> xIndexAccess(xSeries, UNO_QUERY_THROW);
            uno::Reference<container::XIndexAccess> xIndexAccess2(xIndexAccess->getByIndex(0), UNO_QUERY_THROW); // Why this second group shape is here?
            uno::Reference<drawing::XShape> xArea(xIndexAccess2->getByIndex(0), UNO_QUERY_THROW);

            // Check size and position
            awt::Point aAreaPosition = xArea->getPosition();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAreaPosition.X, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAreaPosition.Y, INT_EPS);
            awt::Size aAreaSize = xArea->getSize();
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAreaSize.Height, INT_EPS);
            CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aAreaSize.Width, INT_EPS);

            // Check transformation
            Reference< beans::XPropertySet > xPropSet(xArea, UNO_QUERY_THROW);
            drawing::HomogenMatrix3 aAreaTransformation;
            xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aAreaTransformation;
            CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aAreaTransformation, INT_EPS);

            // Check area fill style and color
            drawing::FillStyle aAreaFillStyle;
            xPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= aAreaFillStyle;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aAreaFillStyle));
            util::Color aAreaFillColor = 0;
            xPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aAreaFillColor;
            CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aAreaFillColor));
        }
    }
}


DECLARE_DUMP_TEST(PointLineChartTest, Chart2DumpTest, false)
{
    const std::vector<OUString> aTestFiles =
    {
        u"normal_line_chart_lines_only.ods"_ustr,
        u"normal_line_chart_points_only.ods"_ustr,
        u"normal_line_chart_lines_and_points.ods"_ustr,
        u"stacked_line_chart_lines_only.ods"_ustr,
        u"stacked_line_chart_points_only.ods"_ustr,
        u"stacked_line_chart_lines_and_points.ods"_ustr,
        u"percent_stacked_line_chart_lines_only.ods"_ustr,
        u"percent_stacked_line_chart_points_only.ods"_ustr,
        u"percent_stacked_line_chart_lines_and_points.ods"_ustr,
        u"scatter_chart_points_only.ods"_ustr,
        u"scatter_chart_lines_only.ods"_ustr,
        u"scatter_chart_lines_and_points.ods"_ustr,
    };

    for (const OUString& sTestFile : aTestFiles)
    {
        setTestFileName(sTestFile);
        loadFromFile(getTestFileName());
        uno::Reference< chart::XChartDocument > xChartDoc(getChartDocFromSheet(0, mxComponent), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xChartDoc, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShapes> xShapes(xDrawPage->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShapes.is());

        uno::Reference< chart2::XChartDocument > xChartDoc2(xChartDoc, UNO_QUERY_THROW);
        Reference<chart2::XChartType> xChartType = getChartTypeFromDoc(xChartDoc2, 0);
        CPPUNIT_ASSERT(xChartType.is());

        std::vector<std::vector<double> > aDataSeriesYValues = getDataSeriesYValuesFromChartType(xChartType);
        size_t nSeriesCount = aDataSeriesYValues.size();
        CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nSeriesCount);

        for (size_t nSeries = 0; nSeries < nSeriesCount; ++nSeries)
        {
            uno::Reference<drawing::XShape> xSeries = getShapeByName(xShapes, "CID/D=0:CS=0:CT=0:Series=" + OUString::number(nSeries));
            CPPUNIT_ASSERT(xSeries.is());
            CPPUNIT_DUMP_ASSERT_NOTE("Series " + OUString::number(nSeries));

            uno::Reference<container::XIndexAccess> xIndexAccess(xSeries, UNO_QUERY_THROW);
            uno::Reference<container::XIndexAccess> xIndexAccess2(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);
            uno::Reference<drawing::XShape> xLine(xIndexAccess2->getByIndex(0), UNO_QUERY_THROW);
            Reference< beans::XPropertySet > xPropSet(xLine, UNO_QUERY_THROW);

            // Check whether we have line
            drawing::LineStyle aSeriesLineStyle;
            xPropSet->getPropertyValue(UNO_NAME_LINESTYLE) >>= aSeriesLineStyle;
            if (aSeriesLineStyle != drawing::LineStyle_NONE)
            {
                CPPUNIT_DUMP_ASSERT_NOTE(u"Lines are displayed"_ustr);
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aSeriesLineStyle));

                // Check line shape geometry
                awt::Point aLinePosition = xLine->getPosition();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLinePosition.X, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLinePosition.Y, INT_EPS);
                awt::Size aLineSize = xLine->getSize();
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLineSize.Height, INT_EPS);
                CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aLineSize.Width, INT_EPS);
                CPPUNIT_ASSERT(xPropSet.is());
                drawing::HomogenMatrix3 aLineTransformation;
                xPropSet->getPropertyValue(u"Transformation"_ustr) >>= aLineTransformation;
                CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aLineTransformation, INT_EPS);
            }

            // Check points of series
            if (xIndexAccess->getCount() >= 2)
            {
                CPPUNIT_DUMP_ASSERT_NOTE(u"Points are displayed"_ustr);
                uno::Reference<container::XIndexAccess> xPointsOfSeries(xIndexAccess->getByIndex(1), UNO_QUERY_THROW);
                sal_Int32 nPointCountInSeries = xPointsOfSeries->getCount();
                CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(nPointCountInSeries);
                for (sal_Int32 nPoint = 0; nPoint < nPointCountInSeries; ++nPoint)
                {
                    uno::Reference<container::XIndexAccess> XPointContainer (
                        getShapeByName(xShapes, "CID/MultiClick/D=0:CS=0:CT=0:Series=" + OUString::number(nSeries) + ":Point=" + OUString::number(nPoint)), UNO_QUERY_THROW);
                    uno::Reference<drawing::XShape> XPoint(XPointContainer->getByIndex(0), UNO_QUERY_THROW);
                    uno::Reference<container::XNamed> xNamedShape(XPointContainer, uno::UNO_QUERY);
                    CPPUNIT_DUMP_ASSERT_NOTE(xNamedShape->getName());

                    // Check size and position
                    awt::Point aPointPosition = XPoint->getPosition();
                    CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aPointPosition.X, INT_EPS);
                    CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aPointPosition.Y, INT_EPS);
                    awt::Size aPointSize = XPoint->getSize();
                    CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aPointSize.Height, INT_EPS);
                    CPPUNIT_DUMP_ASSERT_DOUBLES_EQUAL(aPointSize.Width, INT_EPS);

                    // Check transformation
                    Reference< beans::XPropertySet > xPointPropSet(XPoint, UNO_QUERY_THROW);
                    drawing::HomogenMatrix3 aPointTransformation;
                    xPointPropSet->getPropertyValue(u"Transformation"_ustr) >>= aPointTransformation;
                    CPPUNIT_DUMP_ASSERT_TRANSFORMATIONS_EQUAL(aPointTransformation, INT_EPS);

                    // Check fill style and color
                    drawing::FillStyle aPointFillStyle;
                    xPointPropSet->getPropertyValue(UNO_NAME_FILLSTYLE) >>= aPointFillStyle;
                    CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aPointFillStyle));
                    util::Color aPointFillColor = 0;
                    xPointPropSet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= aPointFillColor;
                    CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL(static_cast<sal_Int32>(aPointFillColor));
                }
            }
        }
    }
}

DECLARE_DUMP_TEST( PivotChartDataButtonTest, Chart2DumpTest, false )
{
    setTestFileName( u"pivotchart_data_button.ods"_ustr );
    loadFromFile(getTestFileName());

    // Check that we have pivot chart in the document
    uno::Reference<table::XTablePivotCharts> xTablePivotCharts = getTablePivotChartsFromSheet( 1, mxComponent );
    uno::Reference<container::XIndexAccess> xIndexAccess( xTablePivotCharts, UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xIndexAccess->getCount() );

    // Get the pivot chart document so we ca access its data
    uno::Reference<chart2::XChartDocument> xChartDoc;
    xChartDoc.set( getPivotChartDocFromSheet( xTablePivotCharts, 0 ) );
    CPPUNIT_ASSERT( xChartDoc.is() );

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier( xChartDoc, uno::UNO_QUERY );
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShapes> xShapes( xDrawPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xShapes.is() );

    // Get the shape that represents the "Data" button.
    uno::Reference<drawing::XShape> xButton = getShapeByName( xShapes, u"FieldButton.Row.8"_ustr,
                                                              []( const uno::Reference<drawing::XShape>& xShapeNode )
                                                              {
                                                                  return xShapeNode->getShapeType() == "com.sun.star.drawing.TextShape";
                                                              } );
    CPPUNIT_ASSERT_MESSAGE( "Cannot find Data button shape", xButton.is() );

    // Make sure that there is no arrow shape with the Data button
    uno::Reference<drawing::XShape> xArrow = getShapeByName( xShapes, u"FieldButton.Row.8"_ustr,
                                                             []( const uno::Reference<drawing::XShape>& xShapeNode )
                                                             {
                                                                 return xShapeNode->getShapeType() == "com.sun.star.drawing.PolyPolygonShape";
                                                             } );
    CPPUNIT_ASSERT_MESSAGE( "Arrow shape should not be present for the Data button", !xArrow.is() );

    // Assert the background color of the Data button
    util::Color aButtonFillColor = 0;
    uno::Reference<beans::XPropertySet> xPropSet( xButton, UNO_QUERY_THROW );
    xPropSet->getPropertyValue( UNO_NAME_FILLCOLOR ) >>= aButtonFillColor;
    CPPUNIT_DUMP_ASSERT_NUMBERS_EQUAL( static_cast<sal_Int32>( aButtonFillColor ) );
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
