/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// core, please keep it alphabetically ordered
#include <comphelper/configuration.hxx>
#include <formula/errorcodes.hxx>
#include <formulacell.hxx>
#include "helper/qahelper.hxx"
#include <test/unoapi_test.hxx>
#include <unotools/saveopt.hxx>

// API, please keep it alphabetically ordered
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

using namespace css;
using namespace css::uno;

/* Implementation of Filters test, volume 5*/

class ScFiltersTest5 : public ScModelTestBase
{
public:
    ScFiltersTest5()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf167134_LOOKUP_extRef)
{
    // testing the correct handling of external references in LOOKUP parameters
    // The file lookup_target.fods uses external links to file lookup_source.fods
    // The test requires that both are located in directory sc/qa/unit/data/fods;
    createScDoc("fods/lookup_target.fods");

    ScDocument* pDoc = getScDoc();
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->ReloadAllLinks();
    pDocSh->DoHardRecalc();

    // compare the data loaded from external links (column 0)
    // with the expected result stored in the test file (column 1)
    for (SCROW nRow = 3; nRow <= 26; nRow++)
    {
        OUString aResult = pDoc->GetString(ScAddress(0, nRow, 0));
        OUString aExpected = pDoc->GetString(ScAddress(1, nRow, 0));
        CPPUNIT_ASSERT_EQUAL(aExpected, aResult);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf162963)
{
    //tests xlsx -> ods -> ods of property "TotalsRow"
    createScDoc("xlsx/tdf162963_TableWithTotalsEnabled.xlsx");

    static constexpr OUString sDBName(u"myData"_ustr);
    static constexpr OUString sPropName(u"TotalsRow"_ustr);

    // Make sure the database range "myData" has TotalsRow TRUE after import from xlsx.
    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xDocPropSet(xDoc, UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xNameAccess(
            xDocPropSet->getPropertyValue(u"DatabaseRanges"_ustr), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xDBRangePropSet(xNameAccess->getByName(sDBName),
                                                            UNO_QUERY_THROW);
        bool bTotalsRow = false;
        xDBRangePropSet->getPropertyValue(sPropName) >>= bTotalsRow;
        CPPUNIT_ASSERT_MESSAGE("xlsx-import", bTotalsRow);
    }
    // Make sure TotalsRow is still TRUE after save to ods and reload.
    // The error was, that the property "TotalsRow" was not written to ods at all.
    // With fix it is written as calcext:contains-footer.
    {
        saveAndReload(u"calc8"_ustr);
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xDocPropSet(xDoc, UNO_QUERY_THROW);
        uno::Reference<container::XNameAccess> xNameAccess(
            xDocPropSet->getPropertyValue(u"DatabaseRanges"_ustr), UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xDBRangePropSet(xNameAccess->getByName(sDBName),
                                                            UNO_QUERY_THROW);
        bool bTotalsRow = true;
        xDBRangePropSet->getPropertyValue(sPropName) >>= bTotalsRow;
        CPPUNIT_ASSERT_MESSAGE("save ods, reload", bTotalsRow);
    }
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf162963_ODF)
{
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });

    // Verify, that calcext:contains-footer is only written in extended file format versions.
    // The parameter in DefaultVersion::set need to be adapted, when attribute contains-footer
    // is included in ODF strict, see issue OFFICE-4169 at OASIS.
    createScDoc("fods/tdf162963_DatabaseRange.fods");

    // enable TotalsRow
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDocPropSet(xDoc, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNameAccess(
        xDocPropSet->getPropertyValue(u"DatabaseRanges"_ustr), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDBRangePropSet(xNameAccess->getByName(u"myData"_ustr),
                                                        UNO_QUERY_THROW);
    xDBRangePropSet->setPropertyValue(u"TotalsRow"_ustr, uno::Any(true));

    // Save to ODF 1.3 extended.
    // Adapt to a concrete version when attribute contains-footer is available in ODF strict.
    // Make sure attribute is written in calcext namespace
    save(u"calc8"_ustr);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/"
                "table:database-ranges/table:database-range[@calcext:contains-footer='true']"_ostr);

    // Save to ODF 1.3 strict. Make sure attribute is not written.
    // Adapt to ODF 1.4 strict, when it is available.
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
    save(u"calc8"_ustr); // this saves to .ods not to .fods
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/"
                "table:database-ranges/table:database-range/contains-footer",
                0);
}

CPPUNIT_TEST_FIXTURE(ScFiltersTest5, testTdf162177_EastersundayODF14)
{
    // EASTERSUNDAY was added to ODFF in ODF 1.4. LibreOffice has written it as
    // ORG.OPENOFFICE.EASTERSUNDAY for ODF 1.2 and ODF 1.3.
    Resetter resetter([]() { SetODFDefaultVersion(SvtSaveOptions::ODFVER_LATEST); });
    createScDoc("fods/tdf162177_Eastersunday.fods");

    // File has it as ORG.OPENOFFICE.EASTERSUNDAY in ODF 1.3. Test, that it is read correctly.
    ScDocument* pDoc = getScDoc();
    OUString aFormula = pDoc->GetFormula(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL(u"=EASTERSUNDAY(2024)"_ustr, aFormula);

    // Verify that saving to ODF1.3 produces ORG.OPENOFFICE.EASTERSUNDAY
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_013);
    save(u"calc8"_ustr); // this saves to .ods not to .fods
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    const OString sPath = "/office:document-content/office:body/office:spreadsheet/table:table/"
                          "table:table-row/table:table-cell"_ostr;
    assertXPath(pXmlDoc, sPath, "formula", u"of:=ORG.OPENOFFICE.EASTERSUNDAY(2024)");

    // Verify that saving to ODF1.4 produces EASTERSUNDAY
    SetODFDefaultVersion(SvtSaveOptions::ODFDefaultVersion::ODFVER_014);
    save(u"calc8"_ustr); // this saves to .ods not to .fods
    pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, sPath, "formula", u"of:=EASTERSUNDAY(2024)");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
