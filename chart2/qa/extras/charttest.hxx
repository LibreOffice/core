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
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <unotools/tempfile.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <iostream>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>

#include <libxml/xmlwriter.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;


/**
 * Macro to declare a new test (with full round-trip. To test
 * import only use the DECLARE_SW_IMPORT_TEST macro instead).
 * In order to add a new test, one only needs to use this macro
 * and then specify the test content, like this:
 *
 * DECLARE_SW_ROUNDTRIP_TEST(MyTest, "myfilename.docx", Test)
 * {
 *      CPPUNIT_ASSERT_EQUAL(blabla);
 * }
 *
 */
#define DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, BaseClass) \
    class TestName : public BaseClass { \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST(Import_Export_Import); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import() { \
        executeImportTest(filename);\
    }\
    void Import_Export_Import() {\
        executeImportExportImportTest(filename);\
    }\
    void verify();\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()


class ChartTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    ChartTest(const char* pTestDocumentPath = "", const char* pFilter = "")
        : mpFilter(pFilter),
        m_nStartTime(0),
        mpXmlBuffer(0),
        mpTestDocumentPath(pTestDocumentPath),
        m_bExported(false)
    {
    }

    void load( const char* pDir, const char* pName );
    void reload( const OUString& rFilterName );
    uno::Sequence < OUString > getImpressChartColumnDescriptions( const char* pDir, const char* pName );

    void loadDocx(const char* pDir, const char* pName);
    void reloadDocx(const char* pFilter, const char* filename);
    void executeImportTest(const char* pName);
    void executeImportExportImportTest(const char* pName);

    virtual void setUp();
    virtual void tearDown();

protected:
    Reference< lang::XComponent > mxComponent;

    /**
     * Override this function if interested in skipping import test for this file
     */
     virtual bool mustTestImportOf(const char* /* filename */) const
     {
        return true;
     }

    /**
     * Function overloaded by unit test. See DECLARE_SW_*_TEST macros
     */
    virtual void verify()
    {
        CPPUNIT_FAIL( "verify method must be overriden" );
    }

    /**
     * Override this function if some special filename-specific setup is needed
     */
    virtual void preTest(const char* /*filename*/)
    {
    }

    /**
     * Override this function if some special filename-specific teardown is needed
     */
    virtual void postTest(const char* /*filename*/)
    {
    }

    /**
     * Override this function if not calcing layout is needed
     */
    virtual bool mustCalcLayoutOf(const char* /*filename*/)
    {
        return true;
    }

    void header()
    {
        std::cout << "File tested,Execution Time (ms)" << std::endl;
    }

    void finish()
    {
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        std::cout << (nEndTime - m_nStartTime) << std::endl;
        if (mpXmlBuffer)
        {
            xmlBufferFree(mpXmlBuffer);
            mpXmlBuffer = 0;
        }
    }

private:
    void calcLayout()
    {
        SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
        pDoc->GetCurrentViewShell()->CalcLayout();
    }

protected:
    const char* mpFilter;
    sal_uInt32 m_nStartTime;
    xmlBufferPtr mpXmlBuffer;
    const char* mpTestDocumentPath;
    utl::TempFile m_aTempFile;
    bool m_bExported;
};

void ChartTest::load( const char* pDir, const char* pName )
{
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());
}

void ChartTest::reload(const OUString& rFilterName)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= rFilterName;
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aArgs);
    mxComponent->dispose();
    mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.sheet.SpreadsheetDocument");
    std::cout << aTempFile.GetURL();
    CPPUNIT_ASSERT(mxComponent.is());
}

void ChartTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set( com::sun::star::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) ) );
}

void ChartTest::tearDown()
{
    if(mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();

}
Reference< lang::XComponent > getChartCompFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent )
{
    // let us assume that we only have one chart per sheet

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xIA.is());

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(nSheet), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartSupplier.is());

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChart.is());

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xEmbObjectSupplier.is());

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartComp.is());

    return xChartComp;

}
Reference< chart2::XChartDocument > getChartDocFromSheet( sal_Int32 nSheet, uno::Reference< lang::XComponent > xComponent )
{
    uno::Reference< chart2::XChartDocument > xChartDoc ( getChartCompFromSheet(nSheet, xComponent), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartDoc.is());
    return xChartDoc;
}

Reference< chart2::XChartType > getChartTypeFromDoc( Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nChartType, sal_Int32 nCooSys = 0 )
{
    CPPUNIT_ASSERT( xChartDoc.is() );

    Reference <chart2::XDiagram > xDiagram = xChartDoc->getFirstDiagram();
    CPPUNIT_ASSERT( xDiagram.is() );

    Reference< chart2::XCoordinateSystemContainer > xCooSysContainer( xDiagram, UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xCooSysContainer.is() );

    Sequence< Reference< chart2::XCoordinateSystem > > xCooSysSequence( xCooSysContainer->getCoordinateSystems());
    CPPUNIT_ASSERT( xCooSysSequence.getLength() > nCooSys );

    Reference< chart2::XChartTypeContainer > xChartTypeContainer( xCooSysSequence[nCooSys], UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xChartTypeContainer.is() );

    Sequence< Reference< chart2::XChartType > > xChartTypeSequence( xChartTypeContainer->getChartTypes() );
    CPPUNIT_ASSERT( xChartTypeSequence.getLength() > nChartType );

    return xChartTypeSequence[nChartType];
}

Reference< chart2::XDataSeries > getDataSeriesFromDoc( uno::Reference< chart2::XChartDocument > xChartDoc,
                                                                sal_Int32 nDataSeries, sal_Int32 nChartType = 0, sal_Int32 nCooSys = 0 )
{
    Reference< chart2::XChartType > xChartType = getChartTypeFromDoc( xChartDoc, nChartType, nCooSys );
    Reference< chart2::XDataSeriesContainer > xDataSequenceContainer( xChartType, UNO_QUERY_THROW );
    CPPUNIT_ASSERT ( xDataSequenceContainer.is() );

    Sequence< Reference< chart2::XDataSeries > > xSeriesSequence( xDataSequenceContainer->getDataSeries() );
    CPPUNIT_ASSERT( xSeriesSequence.getLength() > nDataSeries );

    Reference< chart2::XDataSeries > xSeries = xSeriesSequence[nDataSeries];

    return xSeries;
}


uno::Sequence < OUString > getWriterChartColumnDescriptions( Reference< lang::XComponent > mxComponent )
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT( xShape.is() );
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    uno::Reference< chart2::XChartDocument > xChartDoc;
    xChartDoc.set( xPropertySet->getPropertyValue( "Model" ), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xChartDoc.is() );
    CPPUNIT_ASSERT( xChartDoc->getDataProvider().is() );
    uno::Reference<beans::XPropertySet> xProp(xChartDoc->getDataProvider(), uno::UNO_QUERY );
    uno::Reference< chart2::XAnyDescriptionAccess > xAnyDescriptionAccess ( xChartDoc->getDataProvider(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT( xAnyDescriptionAccess.is() );
    uno::Sequence< OUString > seriesList = xAnyDescriptionAccess->getColumnDescriptions();
    return seriesList;
}

uno::Sequence < OUString > ChartTest::getImpressChartColumnDescriptions( const char* pDir, const char* pName )
{
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.comp.Draw.PresentationDocument");
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(mxComponent, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xPage.is());
    uno::Reference< beans::XPropertySet > xShapeProps(
        xPage->getByIndex(0), uno::UNO_QUERY );
    CPPUNIT_ASSERT(xShapeProps.is());
    uno::Reference< frame::XModel > xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel.is());
    uno::Reference< chart::XChartDocument > xChart1Doc( xDocModel, uno::UNO_QUERY_THROW );
    uno::Reference< chart::XChartDataArray > xChartData ( xChart1Doc->getData(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartData.is());
    uno::Sequence < OUString > seriesList = xChartData->getColumnDescriptions();
    return seriesList;
}


//*****************************OOXML****************************************//

void ChartTest::loadDocx(const char* pDir, const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    // Output name early, so in the case of a hang, the name of the hanging input file is visible.
    std::cout << pName << ",";
    m_nStartTime = osl_getGlobalTimer();
    mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.text.TextDocument");
    if (mustCalcLayoutOf(pName))
        calcLayout();
}

void ChartTest::reloadDocx(const char* pFilter, const char* filename)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs(1);
    aArgs[0].Name = "FilterName";
    aArgs[0].Value <<= OUString::createFromAscii(pFilter);
    m_aTempFile.EnableKillingFile();
    xStorable->storeToURL(m_aTempFile.GetURL(), aArgs);
    uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    m_bExported = true;
    mxComponent = loadFromDesktop(m_aTempFile.GetURL(), "com.sun.star.text.TextDocument");
    if (mpXmlBuffer)
    {
        xmlBufferFree(mpXmlBuffer);
        mpXmlBuffer = 0;
    }
    if (mustCalcLayoutOf(filename))
        calcLayout();
}


/**
 * Helper func used by each unit test to test the 'import' code.
 * (Loads the requested file and then calls 'verify' method)
 */
void ChartTest::executeImportTest(const char* pName)
{
    // If the testcase is stored in some other format, it's pointless to test.
    if (mustTestImportOf(pName))
    {
        header();
        preTest(pName);
        loadDocx(mpTestDocumentPath, pName);
        postTest(pName);
        verify();
        finish();
    }
}

/**
 * Helper func used by each unit test to test the 'export' code.
 * (Loads the requested file, save it to temp file, load the
 * temp file and then calls 'verify' method)
 */
void ChartTest::executeImportExportImportTest(const char* pName)
{
    header();
    preTest(pName);
    loadDocx(mpTestDocumentPath, pName);
    reloadDocx(mpFilter, pName);
    postTest(pName);
    verify();
    finish();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
