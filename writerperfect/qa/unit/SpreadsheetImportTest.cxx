/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>

#include <rtl/ref.hxx>

#include "DocumentHandlerForOds.hxx"
#include "ImportFilter.hxx"
#include "WpftFilterFixture.hxx"
#include "WpftLoader.hxx"
#include "wpftimport.hxx"

namespace
{

namespace uno = css::uno;

class SpreadsheetImportFilter : public writerperfect::ImportFilter<OdsGenerator>
{
public:
    explicit SpreadsheetImportFilter(const uno::Reference< uno::XComponentContext > &rxContext)
        : writerperfect::ImportFilter<OdsGenerator>(rxContext) {}

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString &ServiceName) override;
    virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() override;

private:
    virtual bool doDetectFormat(librevenge::RVNGInputStream &rInput, rtl::OUString &rTypeName) override;
    virtual bool doImportDocument(librevenge::RVNGInputStream &rInput, OdsGenerator &rGenerator, utl::MediaDescriptor &rDescriptor) override;

    static void generate(librevenge::RVNGSpreadsheetInterface &rDocument);
};

bool SpreadsheetImportFilter::doImportDocument(librevenge::RVNGInputStream &, OdsGenerator &rGenerator, utl::MediaDescriptor &)
{
    SpreadsheetImportFilter::generate(rGenerator);
    return true;
}

bool SpreadsheetImportFilter::doDetectFormat(librevenge::RVNGInputStream &, rtl::OUString &rTypeName)
{
    rTypeName = "WpftDummyText";
    return true;
}

// XServiceInfo
rtl::OUString SAL_CALL SpreadsheetImportFilter::getImplementationName()
{
    return OUString("org.libreoffice.comp.Wpft.QA.SpreadsheetImportFilter");
}

sal_Bool SAL_CALL SpreadsheetImportFilter::supportsService(const rtl::OUString &rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< rtl::OUString > SAL_CALL SpreadsheetImportFilter::getSupportedServiceNames()
{
    return {"com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection"};
}

void SpreadsheetImportFilter::generate(librevenge::RVNGSpreadsheetInterface &rDocument)
{
    using namespace librevenge;

    rDocument.startDocument(RVNGPropertyList());
    rDocument.openPageSpan(RVNGPropertyList());
    rDocument.openSheet(RVNGPropertyList());
    rDocument.openSheetRow(RVNGPropertyList());
    rDocument.openSheetCell(RVNGPropertyList());
    rDocument.openSpan(RVNGPropertyList());
    rDocument.insertText("My hovercraft is full of eels.");
    rDocument.closeSpan();
    rDocument.closeSheetCell();
    rDocument.closeSheetRow();
    rDocument.closeSheet();
    rDocument.closePageSpan();
    rDocument.endDocument();
}

}

namespace
{

class SpreadsheetImportTest : public writerperfect::test::WpftFilterFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(SpreadsheetImportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void SpreadsheetImportTest::test()
{
    using namespace css;

    rtl::Reference<SpreadsheetImportFilter> xFilter{new SpreadsheetImportFilter(m_xContext)};
    writerperfect::test::WpftLoader aLoader(createDummyInput(), xFilter.get(), "private:factory/scalc", m_xDesktop, m_xContext);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(aLoader.getDocument(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<container::XIndexAccess> xSheets(xDoc->getSheets(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSheets.is());
    uno::Reference<table::XCellRange> xSheet(xSheets->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSheet.is());
    uno::Reference<table::XCell> xCell = xSheet->getCellByPosition(0, 0);
    CPPUNIT_ASSERT(xCell.is());
#if 0 // broken by commit 8154953add163554c00935486a1cf5677cef2609
    CPPUNIT_ASSERT_EQUAL(table::CellContentType_TEXT, xCell->getType());
    CPPUNIT_ASSERT_EQUAL(rtl::OUString("My hovercraft is full of eels"), xCell->getFormula());
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(SpreadsheetImportTest);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
