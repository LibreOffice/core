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
