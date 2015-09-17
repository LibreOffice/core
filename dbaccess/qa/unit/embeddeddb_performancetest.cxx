/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <memory>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/time.h>
#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCloseable.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

void normaliseTimeValue(TimeValue* pVal)
{
    pVal->Seconds += pVal->Nanosec / 1000000000;
    pVal->Nanosec %= 1000000000;
}

void getTimeDifference(const TimeValue* pTimeStart,
                       const TimeValue* pTimeEnd,
                       TimeValue* pTimeDifference)
{
    // We add 1 second to the nanoseconds to ensure that we get a positive number
    // We have to normalise anyway so this doesn't cause any harm.
    // (Seconds/Nanosec are both unsigned)
    pTimeDifference->Seconds = pTimeEnd->Seconds - pTimeStart->Seconds - 1;
    pTimeDifference->Nanosec = 1000000000 + pTimeEnd->Nanosec - pTimeStart->Nanosec;
    normaliseTimeValue(pTimeDifference);
}

OUString getPrintableTimeValue(const TimeValue* pTimeValue)
{
    return OUString::number(
        (sal_uInt64(pTimeValue->Seconds) * SAL_CONST_UINT64(1000000000)
        + sal_uInt64(pTimeValue->Nanosec))/ 1000000
    );
}

/*
 * The recommended way to run this test is:
 * 'SAL_LOG="" DBA_PERFTEST=YES make CppunitTest_dbaccess_embeddeddb_performancetest'
 * This blocks the unnecessary exception output and show only the performance data.
 *
 * You also need to create the file dbacess/qa/unit/data/wordlist, this list cannot
 * contain any unescaped apostrophes (since the words are used directly to assemble
 * sql statement), apostrophes are escaped using a double apostrophe, i.e. ''.
 * one easy way of generating a list is using:
 * 'for WORD in $(aspell dump master); do echo ${WORD//\'/\'\'}; done > dbaccess/qa/unit/data/wordlist'
 *
 * Note that wordlist cannot have more than 220580 lines, this is due to a hard
 * limit in our hsqldb version.
 *
 * Also note that this unit test "fails" when doing performance testing, this is
 * since by default unit test output is hidden, and thus there is no way of
 * reading the results.
 */
class EmbeddedDBPerformanceTest
    : public DBTestBase
{
private:
    static const char our_sEnableTestEnvVar[];

    // We store the results and print them at the end due to the amount of warning
    // noise present which otherwise obscures the results.
    OUStringBuffer m_aOutputBuffer;

    void printTimes(const TimeValue* pTime1, const TimeValue* pTime2, const TimeValue* pTime3);

    void doPerformanceTestOnODB(const OUString& rDriverURL,
                                const OUString& rDBName,
                                const bool bUsePreparedStatement);

    void setupTestTable(uno::Reference< XConnection >& xConnection);

    SvFileStream *getWordListStream();

    // Individual Tests
    void performPreparedStatementInsertTest(
        uno::Reference< XConnection >& xConnection,
        const OUString& rDBName);
    void performStatementInsertTest(
        uno::Reference< XConnection >& xConnection,
        const OUString& rDBName);
    void performReadTest(
        uno::Reference< XConnection >& xConnection,
        const OUString& rDBName);

    // Perform all tests on a given DB.
    void testFirebird();
    void testHSQLDB();

public:
    void testPerformance();

    CPPUNIT_TEST_SUITE(EmbeddedDBPerformanceTest);
    CPPUNIT_TEST(testPerformance);
    CPPUNIT_TEST_SUITE_END();
};

SvFileStream* EmbeddedDBPerformanceTest::getWordListStream()
{
    OUString wlPath;
    createFileURL("wordlist", wlPath);
    return new SvFileStream(wlPath, StreamMode::READ);
}

void EmbeddedDBPerformanceTest::printTimes(
    const TimeValue* pTime1,
    const TimeValue* pTime2,
    const TimeValue* pTime3)
{
    m_aOutputBuffer.append(
        getPrintableTimeValue(pTime1) + "\t" +
        getPrintableTimeValue(pTime2) + "\t" +
        getPrintableTimeValue(pTime3) + "\t"
        "\n"
   );
}

const char EmbeddedDBPerformanceTest::our_sEnableTestEnvVar[] = "DBA_PERFTEST";

// TODO: we probably should create a document from scratch instead?

void EmbeddedDBPerformanceTest::testPerformance()
{
    OUString sEnabled;
    osl_getEnvironment(OUString(our_sEnableTestEnvVar).pData, &sEnabled.pData);

    if (sEnabled.isEmpty())
        return;

    m_aOutputBuffer.append("---------------------\n");
    testFirebird();
    m_aOutputBuffer.append("---------------------\n");
    testHSQLDB();
    m_aOutputBuffer.append("---------------------\n");

    fprintf(stdout, "Performance Test Results:\n");
    fprintf(stdout, "%s",
            OUStringToOString(m_aOutputBuffer.makeStringAndClear(),
                              RTL_TEXTENCODING_UTF8)
                .getStr()
    );

    // We want the results printed, but unit test output is only printed on failure
    // Hence we deliberately fail the test.
    CPPUNIT_ASSERT(false);
}

void EmbeddedDBPerformanceTest::testFirebird()
{

    m_aOutputBuffer.append("Standard Insert\n");
    doPerformanceTestOnODB("sdbc:embedded:firebird", "Firebird", false);
    m_aOutputBuffer.append("PreparedStatement Insert\n");
    doPerformanceTestOnODB("sdbc:embedded:firebird", "Firebird", true);
}

void EmbeddedDBPerformanceTest::testHSQLDB()
{
    m_aOutputBuffer.append("Standard Insert\n");
    doPerformanceTestOnODB("sdbc:embedded:hsqldb", "HSQLDB", false);
    m_aOutputBuffer.append("PreparedStatement Insert\n");
    doPerformanceTestOnODB("sdbc:embedded:hsqldb", "HSQLDB", true);
}

/**
 * Use an existing .odb to do performance tests on. The database cannot have
 * a table of the name PFTESTTABLE.
 */
void EmbeddedDBPerformanceTest::doPerformanceTestOnODB(
    const OUString& rDriverURL,
    const OUString& rDBName,
    const bool bUsePreparedStatement)
{
    ::utl::TempFile aFile;
    aFile.EnableKillingFile();

    {
        uno::Reference< XOfficeDatabaseDocument > xDocument(
            m_xSFactory->createInstance("com.sun.star.sdb.OfficeDatabaseDocument"),
            UNO_QUERY_THROW);
        uno::Reference< XStorable > xStorable(xDocument, UNO_QUERY_THROW);

        uno::Reference< XDataSource > xDataSource = xDocument->getDataSource();
        uno::Reference< XPropertySet > xPropertySet(xDataSource, UNO_QUERY_THROW);
        xPropertySet->setPropertyValue("URL", Any(rDriverURL));

        xStorable->storeAsURL(aFile.GetURL(), uno::Sequence< beans::PropertyValue >());
    }

    uno::Reference< XOfficeDatabaseDocument > xDocument(
        loadFromDesktop(aFile.GetURL()), UNO_QUERY_THROW);

    uno::Reference< XConnection > xConnection =
        getConnectionForDocument(xDocument);

    setupTestTable(xConnection);

    if (bUsePreparedStatement)
        performPreparedStatementInsertTest(xConnection, rDBName);
    else
        performStatementInsertTest(xConnection, rDBName);

    performReadTest(xConnection, rDBName);
}

void EmbeddedDBPerformanceTest::setupTestTable(
    uno::Reference< XConnection >& xConnection)
{
    uno::Reference< XStatement > xStatement = xConnection->createStatement();

    // Although not strictly necessary we use quoted identifiers to reflect
    // the fact that Base always uses quoted identifiers.
    xStatement->execute(
        "CREATE TABLE \"PFTESTTABLE\" ( \"ID\" INTEGER NOT NULL PRIMARY KEY "
        ", \"STRINGCOLUMNA\" VARCHAR (50) "
    ")");

    xConnection->commit();
}

void EmbeddedDBPerformanceTest::performPreparedStatementInsertTest(
    uno::Reference< XConnection >& xConnection,
    const OUString& rDBName)
{
    uno::Reference< XPreparedStatement > xPreparedStatement =
        xConnection->prepareStatement(
            "INSERT INTO \"PFTESTTABLE\" ( \"ID\", "
            "\"STRINGCOLUMNA\" "
            ") VALUES ( ?, ? )"
        );

    uno::Reference< XParameters > xParameters(xPreparedStatement, UNO_QUERY_THROW);

    std::unique_ptr< SvFileStream > pFile(getWordListStream());

    OUString aWord;
    sal_Int32 aID = 0;

    TimeValue aStart, aMiddle, aEnd;
    osl_getSystemTime(&aStart);

    while (pFile->ReadByteStringLine(aWord, RTL_TEXTENCODING_UTF8))
    {
        xParameters->setInt(1, aID++);
        xParameters->setString(2, aWord);
        xPreparedStatement->execute();
    }
    osl_getSystemTime(&aMiddle);
    xConnection->commit();
    osl_getSystemTime(&aEnd);


    TimeValue aTimeInsert, aTimeCommit, aTimeTotal;
    getTimeDifference(&aStart, &aMiddle, &aTimeInsert);
    getTimeDifference(&aMiddle, &aEnd, &aTimeCommit);
    getTimeDifference(&aStart, &aEnd, &aTimeTotal);
    m_aOutputBuffer.append("Insert: " + rDBName + "\n");
    printTimes(&aTimeInsert, &aTimeCommit, &aTimeTotal);

    pFile->Close();
}

void EmbeddedDBPerformanceTest::performStatementInsertTest(
    uno::Reference< XConnection >& xConnection,
    const OUString& rDBName)
{
    uno::Reference< XStatement > xStatement =
        xConnection->createStatement();

    std::unique_ptr< SvFileStream > pFile(getWordListStream());

    OUString aWord;
    sal_Int32 aID = 0;

    TimeValue aStart, aMiddle, aEnd;
    osl_getSystemTime(&aStart);

    while (pFile->ReadByteStringLine(aWord, RTL_TEXTENCODING_UTF8))
    {
        xStatement->execute(
            "INSERT INTO \"PFTESTTABLE\" ( \"ID\", "
            "\"STRINGCOLUMNA\" "
            ") VALUES ( "
            + OUString::number(aID++) + ", '" + aWord + "' )"
                    );
    }
    osl_getSystemTime(&aMiddle);
    xConnection->commit();
    osl_getSystemTime(&aEnd);

    TimeValue aTimeInsert, aTimeCommit, aTimeTotal;
    getTimeDifference(&aStart, &aMiddle, &aTimeInsert);
    getTimeDifference(&aMiddle, &aEnd, &aTimeCommit);
    getTimeDifference(&aStart, &aEnd, &aTimeTotal);
    m_aOutputBuffer.append("Insert: " + rDBName + "\n");
    printTimes(&aTimeInsert, &aTimeCommit, &aTimeTotal);

    pFile->Close();
}

void EmbeddedDBPerformanceTest::performReadTest(
    uno::Reference< XConnection >& xConnection,
    const OUString& rDBName)
{
    uno::Reference< XStatement > xStatement = xConnection->createStatement();

    TimeValue aStart, aMiddle, aEnd;
    osl_getSystemTime(&aStart);

    uno::Reference< XResultSet > xResults = xStatement->executeQuery("SELECT * FROM PFTESTTABLE");

    osl_getSystemTime(&aMiddle);

    uno::Reference< XRow > xRow(xResults, UNO_QUERY_THROW);

    while (xResults->next())
    {
        xRow->getString(2);
    }
    osl_getSystemTime(&aEnd);

    TimeValue aTimeSelect, aTimeIterate, aTimeTotal;
    getTimeDifference(&aStart, &aMiddle, &aTimeSelect);
    getTimeDifference(&aMiddle, &aEnd, &aTimeIterate);
    getTimeDifference(&aStart, &aEnd, &aTimeTotal);
    m_aOutputBuffer.append("Read from: " + rDBName + "\n");
    printTimes(&aTimeSelect, &aTimeIterate, &aTimeTotal);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EmbeddedDBPerformanceTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
