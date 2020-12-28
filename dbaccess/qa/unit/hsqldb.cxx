/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

class HSQLDBTest : public DBTestBase
{
public:
    void testEmptyDBConnection();

    CPPUNIT_TEST_SUITE(HSQLDBTest);
    CPPUNIT_TEST(testEmptyDBConnection);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * Test the loading of an "empty" file, i.e. the embedded database has not yet
 * been initialised (as occurs when a new .odb is created and opened by base).
 */
void HSQLDBTest::testEmptyDBConnection()
{
    auto const file = createTempCopy(u"hsqldb_empty.odb");
    uno::Reference<XOfficeDatabaseDocument> xDocument = getDocumentForUrl(file.GetURL());

    getConnectionForDocument(xDocument);

    css::uno::Reference<css::lang::XComponent>(xDocument, css::uno::UNO_QUERY_THROW)->dispose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(HSQLDBTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
