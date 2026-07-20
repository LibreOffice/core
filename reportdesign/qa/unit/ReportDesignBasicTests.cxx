/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ReportDesignTest_base.cxx"

#include <com/sun/star/sdb/application/XDatabaseDocumentUI.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <osl/process.h>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sdb/XSubDocument.hpp>
#include <comphelper/scopeguard.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#if !defined(MACOSX) && !defined(_WIN32) //FIXME
class RptBasicTest : public ReportDesignTestBase
{
public:
    void testLoadingAndSaving(const OUString& rFilterName, const OUString& rReportName,
                              const Reference<frame::XComponentLoader>& xComponentLoader,
                              const Reference<XConnection>& xActiveConnection);
    OUString renderReportText(const OUString& rReportName,
                              const Reference<frame::XComponentLoader>& xComponentLoader,
                              const Reference<XConnection>& xActiveConnection,
                              bool bCppReportBuilder);
};

CPPUNIT_TEST_FIXTURE(RptBasicTest, roundTripTest)
{
    // Test loading and saving an already prepared Report
    // This is a very general test designed to catch crashes
    // on import and saving
    loadURLCopy(u"roundTrip.odb");

    Reference<frame::XModel> xModel(mxComponent, UNO_QUERY_THROW);
    Reference<frame::XController> xController(xModel->getCurrentController());
    Reference<sdb::application::XDatabaseDocumentUI> xUI(xController, UNO_QUERY_THROW);

    xUI->connect();
    Reference<XConnection> xActiveConnection = xUI->getActiveConnection();

    Reference<XReportDocumentsSupplier> xSupp(xModel, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xNameAccess = xSupp->getReportDocuments();
    const Sequence<OUString> aReportNames(xNameAccess->getElementNames());

    Reference<frame::XComponentLoader> xComponentLoader(xNameAccess, UNO_QUERY_THROW);

    testLoadingAndSaving(u"writer8"_ustr, aReportNames[0], xComponentLoader, xActiveConnection);
    testLoadingAndSaving(u"calc8"_ustr, aReportNames[1], xComponentLoader, xActiveConnection);
}

CPPUNIT_TEST_FIXTURE(RptBasicTest, multiGroupingSameFieldIntervals)
{
    // ODB fixture must contain a table and a report that groups on the same field twice
    // with different settings (e.g., INTERVAL 5 and INTERVAL 10). The report should be
    // prepared to reproduce the original failure (Column not found: INT_count_...).
    loadURLCopy(u"ReportBuilder_grouping_same_field.odb");

    Reference<frame::XModel> xModel(mxComponent, UNO_QUERY_THROW);
    Reference<frame::XController> xController(xModel->getCurrentController());
    Reference<sdb::application::XDatabaseDocumentUI> xUI(xController, UNO_QUERY_THROW);

    xUI->connect();
    Reference<XConnection> xActiveConnection = xUI->getActiveConnection();

    Reference<XReportDocumentsSupplier> xSupp(xModel, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xNameAccess = xSupp->getReportDocuments();
    const Sequence<OUString> aReportNames(xNameAccess->getElementNames());
    CPPUNIT_ASSERT(aReportNames.getLength() > 0);

    Reference<frame::XComponentLoader> xComponentLoader(xNameAccess, UNO_QUERY_THROW);

    // Execute all reports via writer export. This will fail the test if any report
    // throws ReportExecutionException like "Column not found: INT_count_Number_*".
    for (const OUString& rName : aReportNames)
        testLoadingAndSaving(u"writer8"_ustr, rName, xComponentLoader, xActiveConnection);

    // Additionally, load the textual outputs of all reports and compare their lengths.
    // They should be identical.
    if (aReportNames.getLength() >= 2)
    {
        ::comphelper::NamedValueCollection aLoadArgs;
        aLoadArgs.put(u"ActiveConnection"_ustr, xActiveConnection);

        Reference<lang::XComponent> xComp1 = xComponentLoader->loadComponentFromURL(
            aReportNames[0], u"_blank"_ustr, 0, aLoadArgs.getPropertyValues());
        Reference<lang::XComponent> xComp2 = xComponentLoader->loadComponentFromURL(
            aReportNames[1], u"_blank"_ustr, 0, aLoadArgs.getPropertyValues());
        Reference<text::XTextDocument> xTextDoc1(xComp1, UNO_QUERY);
        Reference<text::XTextDocument> xTextDoc2(xComp2, UNO_QUERY);

        if (xTextDoc1.is() && xTextDoc2.is())
        {
            sal_Int32 nLen1 = xTextDoc1->getText()->getString().getLength();
            sal_Int32 nLen2 = xTextDoc2->getText()->getString().getLength();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Report textual output length differs", nLen1, nLen2);
        }

        Reference<util::XCloseable> xClose1(xComp1, UNO_QUERY);
        if (xClose1.is())
            xClose1->close(true);
        Reference<util::XCloseable> xClose2(xComp2, UNO_QUERY);
        if (xClose2.is())
            xClose2->close(true);
    }
}

CPPUNIT_TEST_FIXTURE(RptBasicTest, nestedGroupBoundary)
{
    // Reproduces a bug where the C++ ReportBuilder path (SAL_ENABLE_PENTAHO_FREE_REPORTBUILDER=1)
    // failed to open a new nested group when an inner group's own value coincidentally repeats
    // across an outer group's boundary (e.g. Region changes from "North" to "South", but
    // SaleMonth's value stays 6 on both sides of that boundary). The nested group's header/footer
    // must still appear at the outer boundary and not silently merge the two groups.
    //
    // ODB file: table "Sales" (Region, SaleMonth, Amount), report "Sales" grouped by
    // Region (outer) then SaleMonth (inner, each-value grouping), with a "SaleMonth" label in
    // the inner group header. The Java/Pentaho path is treated as the reference/correct output.
    loadURLCopy(u"nested_group_boundary.odb");

    Reference<frame::XModel> xModel(mxComponent, UNO_QUERY_THROW);
    Reference<frame::XController> xController(xModel->getCurrentController());
    Reference<sdb::application::XDatabaseDocumentUI> xUI(xController, UNO_QUERY_THROW);

    xUI->connect();
    Reference<XConnection> xActiveConnection = xUI->getActiveConnection();

    Reference<XReportDocumentsSupplier> xSupp(xModel, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xNameAccess = xSupp->getReportDocuments();
    const Sequence<OUString> aReportNames(xNameAccess->getElementNames());
    CPPUNIT_ASSERT(aReportNames.hasElements());

    Reference<frame::XComponentLoader> xComponentLoader(xNameAccess, UNO_QUERY_THROW);

    OUString sPentahoText
        = renderReportText(aReportNames[0], xComponentLoader, xActiveConnection, false);
    OUString sCppText
        = renderReportText(aReportNames[0], xComponentLoader, xActiveConnection, true);

    auto countOccurrences = [](const OUString& rText, std::u16string_view rNeedle) {
        sal_Int32 nCount = 0;
        sal_Int32 nPos = 0;
        while ((nPos = rText.indexOf(rNeedle, nPos)) != -1)
        {
            ++nCount;
            nPos += rNeedle.size();
        }
        return nCount;
    };

    sal_Int32 nPentahoHeaders = countOccurrences(sPentahoText, u"SaleMonth");
    sal_Int32 nCppHeaders = countOccurrences(sCppText, u"SaleMonth");

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "C++ ReportBuilder path lost a nested SaleMonth group boundary at a Region change",
        nPentahoHeaders, nCppHeaders);
}

CPPUNIT_TEST_FIXTURE(RptBasicTest, emptyCommandNoCrash)
{
    // Executing a report with no Command (query/table) set must fail cleanly
    // (IllegalArgumentException) under the C++ ReportBuilder path, same as the
    // Pentaho path already does - not reach the undefined behavior that used to
    // sit behind an unguarded empty-column-list access in getColumnNameString().
    loadURLCopy(u"nested_group_boundary.odb");

    Reference<frame::XModel> xModel(mxComponent, UNO_QUERY_THROW);
    Reference<frame::XController> xController(xModel->getCurrentController());
    Reference<sdb::application::XDatabaseDocumentUI> xUI(xController, UNO_QUERY_THROW);

    xUI->connect();
    Reference<XConnection> xActiveConnection = xUI->getActiveConnection();

    Reference<XReportDocumentsSupplier> xSupp(xModel, UNO_QUERY_THROW);
    Reference<container::XNameAccess> xNameAccess = xSupp->getReportDocuments();
    const Sequence<OUString> aReportNames(xNameAccess->getElementNames());
    CPPUNIT_ASSERT(aReportNames.hasElements());

    Reference<sdb::XSubDocument> xSubDoc(xNameAccess->getByName(aReportNames[0]), UNO_QUERY_THROW);
    Reference<report::XReportDefinition> xReportDef(xSubDoc->openDesign(), UNO_QUERY_THROW);
    xReportDef->setCommand(u""_ustr);
    xSubDoc->store();
    xSubDoc->close();

    const OUString sVarName(u"SAL_ENABLE_PENTAHO_FREE_REPORTBUILDER"_ustr);
    osl_setEnvironment(sVarName.pData, u"1"_ustr.pData);
    comphelper::ScopeGuard aEnvGuard([&sVarName] { osl_clearEnvironment(sVarName.pData); });

    ::comphelper::NamedValueCollection aLoadArgs;
    aLoadArgs.put(u"ActiveConnection"_ustr, xActiveConnection);
    Reference<frame::XComponentLoader> xComponentLoader(xNameAccess, UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xComponentLoader->loadComponentFromURL(aReportNames[0], u"_blank"_ustr, 0,
                                                                aLoadArgs.getPropertyValues()),
                         lang::IllegalArgumentException);
}

OUString RptBasicTest::renderReportText(const OUString& rReportName,
                                        const Reference<frame::XComponentLoader>& xComponentLoader,
                                        const Reference<XConnection>& xActiveConnection,
                                        bool bCppReportBuilder)
{
    const OUString sVarName(u"SAL_ENABLE_PENTAHO_FREE_REPORTBUILDER"_ustr);
    if (bCppReportBuilder)
        osl_setEnvironment(sVarName.pData, u"1"_ustr.pData);
    else
        osl_clearEnvironment(sVarName.pData);
    comphelper::ScopeGuard aEnvGuard([&sVarName] { osl_clearEnvironment(sVarName.pData); });

    ::comphelper::NamedValueCollection aLoadArgs;
    aLoadArgs.put(u"ActiveConnection"_ustr, xActiveConnection);

    Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL(
        rReportName, u"_blank"_ustr, 0, aLoadArgs.getPropertyValues());

    Reference<text::XTextDocument> xTextDoc(xComponent, UNO_QUERY_THROW);
    OUStringBuffer sText(xTextDoc->getText()->getString());

    // Report sections are laid out as Writer tables (one per section instance), not as plain
    // body-text paragraphs, so the body text alone is empty - collect every table cell's text too.
    Reference<text::XTextTablesSupplier> xTablesSupp(xComponent, UNO_QUERY);
    if (xTablesSupp.is())
    {
        Reference<container::XNameAccess> xTables(xTablesSupp->getTextTables());
        if (xTables.is())
        {
            const Sequence<OUString> aTableNames = xTables->getElementNames();
            for (const OUString& rTableName : aTableNames)
            {
                Reference<text::XTextTable> xTable(xTables->getByName(rTableName), UNO_QUERY);
                if (!xTable.is())
                    continue;
                const Sequence<OUString> aCellNames = xTable->getCellNames();
                for (const OUString& rCellName : aCellNames)
                {
                    Reference<text::XText> xCellText(xTable->getCellByName(rCellName), UNO_QUERY);
                    if (xCellText.is())
                        sText.append(xCellText->getString());
                }
            }
        }
    }

    Reference<util::XCloseable> xCloseable(xComponent, UNO_QUERY);
    if (xCloseable.is())
        xCloseable->close(true);

    return sText.makeStringAndClear();
}

void RptBasicTest::testLoadingAndSaving(const OUString& rFilterName, const OUString& rReportName,
                                        const Reference<frame::XComponentLoader>& xComponentLoader,
                                        const Reference<XConnection>& xActiveConnection)
{
    ::comphelper::NamedValueCollection aLoadArgs;
    aLoadArgs.put(u"ActiveConnection"_ustr, xActiveConnection);

    ::comphelper::NamedValueCollection aSaveArgs;
    aSaveArgs.put(u"FilterName"_ustr, rFilterName);

    Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL(
        rReportName, u"_blank"_ustr, 0, aLoadArgs.getPropertyValues());

    Reference<frame::XStorable> xStorable(xComponent, UNO_QUERY_THROW);
    xStorable->storeAsURL(maTempFile.GetURL(), aSaveArgs.getPropertyValues());

    Reference<util::XCloseable> xCloseable(xComponent, UNO_QUERY_THROW);
    xCloseable->close(true);
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
