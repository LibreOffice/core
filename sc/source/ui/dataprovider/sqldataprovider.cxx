/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sqldataprovider.hxx"
#include <datatransformation.hxx>
#include <salhelper/thread.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/Date.hpp>
#include <comphelper/string.hxx>
#include <miscuno.hxx>
#include <dbdocutl.hxx>

using namespace css;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

namespace sc
{
class SQLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;
    OUString maID;
    const std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;
    std::function<void()> maImportFinishedHdl;

public:
    SQLFetchThread(ScDocument& rDoc, const OUString&, const OUString& rID,
                   std::function<void()> aImportFinishedHdl,
                   const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations);

    virtual void execute() override;
};

SQLFetchThread::SQLFetchThread(
    ScDocument& rDoc, const OUString& rURL, const OUString& rID,
    std::function<void()> aImportFinishedHdl,
    const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations)
    : salhelper::Thread("SQL Fetch Thread")
    , mrDocument(rDoc)
    , maURL(rURL)
    , maID(rID)
    , maDataTransformations(rTransformations)
    , maImportFinishedHdl(aImportFinishedHdl)
{
}

void SQLFetchThread::execute()
{
    sal_Int32 nIndex = maID.indexOf("@");
    if (nIndex == -1)
        return;

    OUString aTable = maID.copy(0, nIndex);
    OUString aDatabase = maID.copy(nIndex + 1);

    try
    {
        uno::Reference<sdb::XDatabaseContext> xContext
            = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());
        uno::Any aSourceAny = xContext->getByName(aDatabase);

        uno::Reference<sdb::XCompletedConnection> xSource(
            ScUnoHelpFunctions::AnyToInterface(aSourceAny), uno::UNO_QUERY);
        if (!xSource.is())
            return;

        uno::Reference<task::XInteractionHandler> xHandler(
            task::InteractionHandler::createWithParent(comphelper::getProcessComponentContext(),
                                                       nullptr),
            uno::UNO_QUERY_THROW);

        uno::Reference<sdbc::XConnection> xConnection = xSource->connectWithCompletion(xHandler);

        uno::Reference<sdbc::XStatement> xStatement = xConnection->createStatement();

        uno::Reference<sdbc::XResultSet> xResult
            = xStatement->executeQuery("SELECT * FROM " + aTable);

        if (xResult.is())
        {
            Reference<sdbc::XResultSetMetaDataSupplier> xMetaDataSupplier(xResult, UNO_QUERY);

            Reference<sdbc::XResultSetMetaData> xMetaData = xMetaDataSupplier->getMetaData();

            Reference<XRow> xRow(xResult, UNO_QUERY);

            long nColCount = xMetaData->getColumnCount();

            // Insert column names
            for (long nCol = 0; nCol < nColCount; nCol++)
            {
                mrDocument.SetString(nCol, 0, 0, xMetaData->getColumnName(nCol + 1));
            }

            // Insert data
            while (xResult->next())
            {
                long nRow = xResult->getRow();

                for (long nCol = 0; nCol < nColCount; nCol++)
                {
                    ScDatabaseDocUtil::PutData(&mrDocument, (SCCOL)nCol, (SCROW)nRow, (SCTAB)0,
                                               xRow, nCol + 1, xMetaData->getColumnType(nCol + 1),
                                               false);
                }
            }
        }
    }
    catch (uno::Exception&)
    {
        OSL_FAIL("exception in database");
    }

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    maImportFinishedHdl();
}

SQLDataProvider::SQLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource)
    : DataProvider(rDataSource)
    , mpDocument(pDoc)
{
}

SQLDataProvider::~SQLDataProvider()
{
    if (mxSQLFetchThread.is())
    {
        SolarMutexReleaser aReleaser;
        mxSQLFetchThread->join();
    }
}

void SQLDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxSQLFetchThread = new SQLFetchThread(*mpDoc, mrDataSource.getURL(), mrDataSource.getID(),
                                          std::bind(&SQLDataProvider::ImportFinished, this),
                                          mrDataSource.getDataTransformation());
    mxSQLFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxSQLFetchThread->join();
    }
}

void SQLDataProvider::ImportFinished()
{
    mrDataSource.getDBManager()->WriteToDoc(*mpDoc);
    mxSQLFetchThread.clear();
    mpDoc.reset();
}

const OUString& SQLDataProvider::GetURL() const { return mrDataSource.getURL(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
