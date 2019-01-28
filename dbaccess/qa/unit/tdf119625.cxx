/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <hsqlimport.hxx>

#include <osl/process.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <test/unoapi_test.hxx>
#include <svtools/miscopt.hxx>
#include <com/sun/star/util/Time.hpp>

using namespace dbahsql;

class Tdf119625Test : public DBTestBase
{
public:
    void testTime();

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(Tdf119625Test);

    CPPUNIT_TEST(testTime);

    CPPUNIT_TEST_SUITE_END();
};

void Tdf119625Test::setUp()
{
    DBTestBase::setUp();
    SvtMiscOptions aMiscOptions;
    osl_setEnvironment(OUString{ "DBACCESS_HSQL_MIGRATION" }.pData, OUString{ "1" }.pData);
}

struct expect_t
{
    sal_Int16 id;
    sal_Int16 h, m, s;
};

/* The values here assume that our results are in UTC.  However,
   tdf#119675 "Firebird: Migration: User dialog to set treatment of
   datetime and time values during migration" is going to change the
   final result of migration. If that change is implemented below
   the level we are testing, this test will have to allow for or set
   the destination timezone.
 */
static const expect_t expect[]
    = { { 0, 15, 10, 10 }, { 1, 23, 30, 30 }, { 2, 5, 0, 0 },  { 3, 4, 30, 0 },
        { 4, 3, 15, 10 },  { 5, 5, 0, 0 },    { 6, 3, 22, 22 } };

void Tdf119625Test::testTime()
{
    // the migration requires the file to be writable
    utl::TempFile const temp(createTempCopy("tdf119625.odb"));
    uno::Reference<XOfficeDatabaseDocument> const xDocument = getDocumentForUrl(temp.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);
    // at this point migration is already done
    /* In the presence of tdf#119625, terminal already has messages

           *value exceeds the range for a valid time
           caused by
           'isc_dsql_execute'

           warn:dbaccess:22435:22435:dbaccess/source/filter/hsqldb/hsqlimport.cxx:373: Error during migration

       In this case, we do not expect anything good from the following
       code, but I (tje, 2018-09-04) do not know how to detect this
       situation.  In particular, the migration has been observed to
       create the destination table (but truncated after the first
       row), and xConnection.is() returns true.
    */

    // select basically everything from the .odb
    uno::Reference<XStatement> statement = xConnection->createStatement();
    const OUString sql{ "  SELECT id, tst_dt, tst_d, tst_t "
                        "    FROM tst_data "
                        "ORDER BY id" };

    uno::Reference<XResultSet> xRes = statement->executeQuery(sql);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // check result
    for (auto& e : expect)
    {
        CPPUNIT_ASSERT(xRes->next());
        CPPUNIT_ASSERT_EQUAL(xRow->getShort(1), e.id);
        auto time_got = xRow->getTime(4);
        auto time_expected = com::sun::star::util::Time(0, e.s, e.m, e.h, false);
        auto equal_times = time_got == time_expected;
        CPPUNIT_ASSERT(equal_times);
    }
    CPPUNIT_ASSERT(!xRes->next());

    closeDocument(uno::Reference<lang::XComponent>(xDocument, uno::UNO_QUERY));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Tdf119625Test);

CPPUNIT_PLUGIN_IMPLEMENT();
