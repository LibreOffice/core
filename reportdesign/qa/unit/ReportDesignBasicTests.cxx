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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#if !defined(MACOSX) && !defined(_WIN32) //FIXME
class RptBasicTest : public ReportDesignTestBase
{
public:
    void testLoadingAndSaving(const OUString& rFilterName, const OUString& rReportName,
                              Reference<frame::XComponentLoader>& xComponentLoader,
                              Reference<XConnection>& xActiveConnection);
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

void RptBasicTest::testLoadingAndSaving(const OUString& rFilterName, const OUString& rReportName,
                                        Reference<frame::XComponentLoader>& xComponentLoader,
                                        Reference<XConnection>& xActiveConnection)
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
