/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "Catalog.hxx"
#include "Connection.hxx"
#include "DatabaseMetaData.hxx"
#include "Driver.hxx"
#include "PreparedStatement.hxx"
#include "Statement.hxx"
#include "Tables.hxx"
#include "Util.hxx"

#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>

#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/hsqldb_res.hrc"
#include "resource/sharedresources.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace connectivity::firebird;
using namespace connectivity;

using namespace ::osl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

const OUString Connection::our_sDBLocation( "firebird.fdb" );

Connection::Connection(FirebirdDriver*    _pDriver)
    : Connection_BASE(m_aMutex)
    , OSubComponent<Connection, Connection_BASE>((::cppu::OWeakObject*)_pDriver, this)
    , m_pDriver(_pDriver)
    , m_sConnectionURL()
    , m_sFirebirdURL()
    , m_bIsEmbedded(sal_False)
    , m_xEmbeddedStorage(0)
    , m_bIsFile(sal_False)
    , m_sUser()
    , m_bIsAutoCommit(sal_False)
    , m_bIsReadOnly(sal_False)
    , m_aTransactionIsolation(TransactionIsolation::REPEATABLE_READ)
    , m_aDBHandle(0)
    , m_aTransactionHandle(0)
    , m_xCatalog(0)
    , m_xMetaData(0)
    , m_aStatements()
{
    m_pDriver->acquire();
}

Connection::~Connection()
{
    if(!isClosed())
        close();

    m_pDriver->release();
    m_pDriver = 0;
}

void SAL_CALL Connection::release() throw()
{
    relase_ChildImpl();
}

void Connection::construct(const ::rtl::OUString& url, const Sequence< PropertyValue >& info)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "construct().");

    osl_atomic_increment( &m_refCount );

    m_sConnectionURL = url;

    bool bIsNewDatabase = false;
    OUString aStorageURL;
    if (url.equals("sdbc:embedded:firebird"))
    {
        m_bIsEmbedded = true;
        const PropertyValue* pIter = info.getConstArray();
        const PropertyValue* pEnd = pIter + info.getLength();

        for (;pIter != pEnd; ++pIter)
        {
            if ( pIter->Name == "Storage" )
            {
                m_xEmbeddedStorage.set(pIter->Value,UNO_QUERY);
            }
            else if ( pIter->Name == "URL" )
            {
                pIter->Value >>= aStorageURL;
            }
        }

        if ( !m_xEmbeddedStorage.is() )
        {
            ::connectivity::SharedResources aResources;
            const OUString sMessage = aResources.getResourceString(STR_NO_STROAGE);
            ::dbtools::throwGenericSQLException(sMessage ,*this);
        }

        bIsNewDatabase = !m_xEmbeddedStorage->hasElements();

        m_pExtractedFDBFile.reset(new ::utl::TempFile(NULL, true));
        m_pExtractedFDBFile->EnableKillingFile();
        m_sFirebirdURL = m_pExtractedFDBFile->GetFileName() + "/firebird.fdb";

        SAL_INFO("connectivity.firebird", "Temporary .fdb location:  " << m_sFirebirdURL);

        if (!bIsNewDatabase)
        {
            SAL_INFO("connectivity.firebird", "Extracting .fdb from .odb" );
            if (!m_xEmbeddedStorage->isStreamElement(our_sDBLocation))
            {
                ::connectivity::SharedResources aResources;
                const OUString sMessage = aResources.getResourceString(STR_ERROR_NEW_VERSION);
                ::dbtools::throwGenericSQLException(sMessage ,*this);
            }

            Reference< XStream > xDBStream(m_xEmbeddedStorage->openStreamElement(our_sDBLocation,
                                                            ElementModes::READ));

            uno::Reference< ucb::XSimpleFileAccess2 > xFileAccess(
                    ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ),
                                                                uno::UNO_QUERY );
            if ( !xFileAccess.is() )
            {
                ::connectivity::SharedResources aResources;
                const OUString sMessage = aResources.getResourceString(STR_ERROR_NEW_VERSION);
                ::dbtools::throwGenericSQLException(sMessage ,*this);
            }

            xFileAccess->writeFile(m_sFirebirdURL,xDBStream->getInputStream());
        }
        // TOOO: Get DB properties from XML

    }
    // External file AND/OR remote connection
    else if (url.startsWith("sdbc:firebird:"))
    {
        m_sFirebirdURL = url.copy(OUString("sdbc:firebird:").getLength());
        if (m_sFirebirdURL.startsWith("file://"))
        {
            m_bIsFile = true;
            uno::Reference< ucb::XSimpleFileAccess > xFileAccess(
                ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext()),
                uno::UNO_QUERY);
            if (!xFileAccess->exists(m_sFirebirdURL))
                bIsNewDatabase = true;

            m_sFirebirdURL = m_sFirebirdURL.copy(OUString("file://").getLength());
        }
    }

    char dpbBuffer[1 + 3 + 257 + 257 ]; // Expand as needed
    int dpbLength = 0;
    {
        char* dpb;
        char userName[256] = "";
        char userPassword[256] = "";

        dpb = dpbBuffer;
        *dpb++ = isc_dpb_version1;

        *dpb++ = isc_dpb_sql_dialect;
        *dpb++ = 1; // 1 byte long
        *dpb++ = FIREBIRD_SQL_DIALECT;
        // Do any more dpbBuffer additions here

        if (m_bIsEmbedded || m_bIsFile)
        {
            *dpb++ = isc_dpb_trusted_auth;
            *dpb++ = 1; // Length of data
            *dpb++ = 1; // TRUE
        }
        else
        {
            // TODO: parse password from connection string as needed?
        }

        if (strlen(userName))
        {
            int nUsernameLength = strlen(userName);
            *dpb++ = isc_dpb_user_name;
            *dpb++ = (char) nUsernameLength;
            strcpy(dpb, userName);
            dpb+= nUsernameLength;
        }

        if (strlen(userPassword))
        {
            int nPasswordLength = strlen(userPassword);
            *dpb++ = isc_dpb_password;
            *dpb++ = (char) nPasswordLength;
            strcpy(dpb, userPassword);
            dpb+= nPasswordLength;
        }

        dpbLength = dpb - dpbBuffer;
    }

    ISC_STATUS_ARRAY status;            /* status vector */
    ISC_STATUS aErr;
    if (bIsNewDatabase)
    {
        aErr = isc_create_database(status,
                                   m_sFirebirdURL.getLength(),
                                   OUStringToOString(m_sFirebirdURL,RTL_TEXTENCODING_UTF8).getStr(),
                                   &m_aDBHandle,
                                   dpbLength,
                                   dpbBuffer,
                                   0);
        if (aErr)
        {
            evaluateStatusVector(status, "isc_create_database", *this);
        }
    }
    else
    {
        aErr = isc_attach_database(status,
                                   m_sFirebirdURL.getLength(),
                                   OUStringToOString(m_sFirebirdURL, RTL_TEXTENCODING_UTF8).getStr(),
                                   &m_aDBHandle,
                                   dpbLength,
                                   dpbBuffer);
        if (aErr)
        {
            evaluateStatusVector(status, "isc_attach_database", *this);
        }
    }

    if (m_bIsEmbedded) // Add DocumentEventListener to save the .fdb as needed
    {
        // TODO: this is only needed when we change icu versions, so ideally
        // we somehow keep track of which icu version we have. There might
        // be something db internal that we can check, or we might have to store
        // it in the .odb.
        rebuildIndexes();

        attachAsDocumentListener(aStorageURL);
    }

    osl_atomic_decrement( &m_refCount );
}

void Connection::attachAsDocumentListener(const OUString& rStorageURL)
{
    // We can't directly access the Document that is using this connection
    // (since a Connection can in fact be used independently of a DB document)
    // hence we need to iterate through all Frames to find our Document.
    uno::Reference< frame::XDesktop2 > xFramesSupplier =
        frame::Desktop::create(::comphelper::getProcessComponentContext());
    uno::Reference< frame::XFrames > xFrames(xFramesSupplier->getFrames(),
                                             uno::UNO_QUERY);
    if (!xFrames.is())
        return;

    uno::Sequence< uno::Reference<frame::XFrame> > xFrameList =
        xFrames->queryFrames( frame::FrameSearchFlag::ALL );

    for (sal_Int32 i = 0; i < xFrameList.getLength(); i++)
    {
        uno::Reference< frame::XFrame > xf = xFrameList[i];
        uno::Reference< XController > xc;
        if (xf.is())
            xc = xf->getController();

        uno::Reference< XModel > xm;
        if (xc.is())
            xm = xc->getModel();

        if (xm.is() && xm->getURL() == rStorageURL)
        {
            uno::Reference<XDocumentEventBroadcaster> xBroadcaster( xm, UNO_QUERY);
            if (xBroadcaster.is())
            {
                xBroadcaster->addDocumentEventListener(this);
                return;
            }
            //TODO: remove in the disposing?
        }
    }
    assert(false); // If we have an embedded DB we must have a document
}

//----- XServiceInfo ---------------------------------------------------------
IMPLEMENT_SERVICE_INFO(Connection, "com.sun.star.sdbc.drivers.firebird.Connection",
                                                    "com.sun.star.sdbc.Connection")

Reference< XBlob> Connection::createBlob(ISC_QUAD* pBlobId)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "createBlob()");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    Reference< XBlob > xReturn = new Blob(&m_aDBHandle,
                                          &m_aTransactionHandle,
                                          *pBlobId);

    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}


//----- XConnection ----------------------------------------------------------
Reference< XStatement > SAL_CALL Connection::createStatement( )
                                        throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "createStatement().");

    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    // the pre
    if(m_aTypeInfo.empty())
        buildTypeInfo();

    SAL_INFO("connectivity.firebird", "createStatement(). "
             "Creating statement.");

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL Connection::prepareStatement(
            const OUString& _sSql)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "prepareStatement() "
             "called with sql: " << _sSql);
    MutexGuard aGuard(m_aMutex);
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    if(m_aTypeInfo.empty())
        buildTypeInfo();

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,
                                                                     m_aTypeInfo,
                                                                     _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL Connection::prepareCall(
                const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "prepareCall(). "
             "_sSql: " << _sSql);

    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return NULL;
}

OUString SAL_CALL Connection::nativeSQL( const OUString& _sSql )
                                        throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    // We do not need to adapt the SQL for Firebird atm.
    return _sSql;
}

void SAL_CALL Connection::setAutoCommit( sal_Bool autoCommit )
                                        throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    m_bIsAutoCommit = autoCommit;

    if (m_aTransactionHandle)
    {
        setupTransaction();
    }
}

sal_Bool SAL_CALL Connection::getAutoCommit() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    return m_bIsAutoCommit;
}

void Connection::setupTransaction()
    throw (SQLException)
{
    MutexGuard aGuard( m_aMutex );
    ISC_STATUS status_vector[20];

    // TODO: is this sensible? If we have changed parameters then transaction
    // is lost...
    if (m_aTransactionHandle)
    {
        disposeStatements();
        isc_rollback_transaction(status_vector, &m_aTransactionHandle);
    }

    char aTransactionIsolation = 0;
    switch (m_aTransactionIsolation)
    {
        // TODO: confirm that these are correct.
        case(TransactionIsolation::READ_UNCOMMITTED):
            aTransactionIsolation = isc_tpb_concurrency;
            break;
        case(TransactionIsolation::READ_COMMITTED):
            aTransactionIsolation = isc_tpb_read_committed;
            break;
        case(TransactionIsolation::REPEATABLE_READ):
            aTransactionIsolation = isc_tpb_consistency;
            break;
        case(TransactionIsolation::SERIALIZABLE):
            aTransactionIsolation = isc_tpb_consistency;
            break;
        default:
            assert( false ); // We must have a valid TransactionIsolation.
    }

    // You cannot pass an empty tpb parameter so we have to do some pointer
    // arithmetic to avoid problems. (i.e. aTPB[x] = 0 is invalid)
    char aTPB[5];
    char* pTPB = aTPB;

    *pTPB++ = isc_tpb_version3;
    if (m_bIsAutoCommit)
        *pTPB++ = isc_tpb_autocommit;
    *pTPB++ = (!m_bIsReadOnly ? isc_tpb_write : isc_tpb_read);
    *pTPB++ = aTransactionIsolation;
    *pTPB++ = isc_tpb_wait;

    isc_start_transaction(status_vector,
                          &m_aTransactionHandle,
                          1,
                          &m_aDBHandle,
                          pTPB - aTPB, // bytes used in TPB
                          aTPB);

    evaluateStatusVector(status_vector,
                         "isc_start_transaction",
                         *this);
}

isc_tr_handle& Connection::getTransaction()
    throw (SQLException)
{
    MutexGuard aGuard( m_aMutex );
    if (!m_aTransactionHandle)
    {
        setupTransaction();
    }
    return m_aTransactionHandle;
}

void SAL_CALL Connection::commit() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    ISC_STATUS status_vector[20];

    if (!m_bIsAutoCommit && m_aTransactionHandle)
    {
        disposeStatements();
        isc_commit_transaction(status_vector, &m_aTransactionHandle);
        evaluateStatusVector(status_vector,
                             "isc_commit_transaction",
                             *this);
    }
}

void SAL_CALL Connection::rollback() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    ISC_STATUS status_vector[20];

    if (!m_bIsAutoCommit && m_aTransactionHandle)
    {
        isc_rollback_transaction(status_vector, &m_aTransactionHandle);
    }
}

sal_Bool SAL_CALL Connection::isClosed(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed taht means someone called dispose(); (XComponent)
    return Connection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL Connection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL Connection::setReadOnly(sal_Bool readOnly)
                                            throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    m_bIsReadOnly = readOnly;
    setupTransaction();
}

sal_Bool SAL_CALL Connection::isReadOnly() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    return m_bIsReadOnly;
}

void SAL_CALL Connection::setCatalog(const OUString& /*catalog*/)
    throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException("setCatalog", *this);
}

OUString SAL_CALL Connection::getCatalog()
    throw(SQLException, RuntimeException)
{
    ::dbtools::throwFunctionNotSupportedException("getCatalog", *this);
    return OUString();
}

void SAL_CALL Connection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    m_aTransactionIsolation = level;
    setupTransaction();
}

sal_Int32 SAL_CALL Connection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(Connection_BASE::rBHelper.bDisposed);

    return m_aTransactionIsolation;
}

Reference< XNameAccess > SAL_CALL Connection::getTypeMap() throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::getTypeMap", *this );
    return 0;
}

void SAL_CALL Connection::setTypeMap(const Reference< XNameAccess >& typeMap)
                                            throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
    (void) typeMap;
}

//----- XCloseable -----------------------------------------------------------
void SAL_CALL Connection::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close().");

    // we just dispose us
    {
        MutexGuard aGuard( m_aMutex );
        checkDisposed(Connection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL Connection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    // when you collected some warnings -> return it
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL Connection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
// --------------------------------------------------------------------------------
// XDocumentEventListener
void SAL_CALL Connection::documentEventOccured( const DocumentEvent& _Event )
                                                        throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    if (!m_bIsEmbedded)
        return;

    if (_Event.EventName == "OnSave" || _Event.EventName == "OnSaveAs")
    {
        commit(); // Commit and close transaction
        if ( m_bIsEmbedded && m_xEmbeddedStorage.is() )
        {
            SAL_INFO("connectivity.firebird", "Writing .fdb into .odb" );

            Reference< XStream > xDBStream(m_xEmbeddedStorage->openStreamElement(our_sDBLocation,
                                                            ElementModes::WRITE));

            using namespace ::comphelper;
            Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
            Reference< XInputStream > xInputStream;
            if (xContext.is())
                xInputStream =
                        OStorageHelper::GetInputStreamFromURL(m_sFirebirdURL, xContext);
            if (xInputStream.is())
                OStorageHelper::CopyInputToOutput( xInputStream,
                                                xDBStream->getOutputStream());
            // TODO: ensure db is in safe state
        }
    }
}
// XEventListener
void SAL_CALL Connection::disposing(const EventObject& /*rSource*/)
    throw (RuntimeException)
{
}
//--------------------------------------------------------------------
void Connection::buildTypeInfo() throw( SQLException)
{
    SAL_INFO("connectivity.firebird", "buildTypeInfo().");

    MutexGuard aGuard( m_aMutex );

    Reference< XResultSet> xRs = getMetaData ()->getTypeInfo ();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type

    // Loop on the result set until we reach end of file

    while (xRs->next ())
    {
        OTypeInfo aInfo;
        aInfo.aTypeName         = xRow->getString   (1);
        aInfo.nType             = xRow->getShort    (2);
        aInfo.nPrecision        = xRow->getInt      (3);
        aInfo.aLiteralPrefix    = xRow->getString   (4);
        aInfo.aLiteralSuffix    = xRow->getString   (5);
        aInfo.aCreateParams     = xRow->getString   (6);
        aInfo.bNullable         = xRow->getBoolean  (7) == ColumnValue::NULLABLE;
        aInfo.bCaseSensitive    = xRow->getBoolean  (8);
        aInfo.nSearchType       = xRow->getShort    (9);
        aInfo.bUnsigned         = xRow->getBoolean  (10);
        aInfo.bCurrency         = xRow->getBoolean  (11);
        aInfo.bAutoIncrement    = xRow->getBoolean  (12);
        aInfo.aLocalTypeName    = xRow->getString   (13);
        aInfo.nMinimumScale     = xRow->getShort    (14);
        aInfo.nMaximumScale     = xRow->getShort    (15);
        aInfo.nNumPrecRadix     = (sal_Int16)xRow->getInt(18);



        // Now that we have the type info, save it
        // in the Hashtable if we don't already have an
        // entry for this SQL type.

        m_aTypeInfo.push_back(aInfo);
    }

    SAL_INFO("connectivity.firebird", "buildTypeInfo(). "
             "Type info built.");

    // Close the result set/statement.

    Reference< XCloseable> xClose(xRs,UNO_QUERY);
    xClose->close();

    SAL_INFO("connectivity.firebird", "buildTypeInfo(). "
             "Closed.");
}

void Connection::disposing()
{
    SAL_INFO("connectivity.firebird", "disposing().");

    MutexGuard aGuard(m_aMutex);

    disposeStatements();

    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    ISC_STATUS_ARRAY status;            /* status vector */
    if (m_aTransactionHandle)
    {
        // TODO: confirm whether we need to ask the user here.
        isc_rollback_transaction(status, &m_aTransactionHandle);
    }

    if (isc_detach_database(status, &m_aDBHandle))
    {
        evaluateStatusVector(status, "isc_detach_database", *this);
    }
    // TODO: write to storage again?

    dispose_ChildImpl();
    cppu::WeakComponentImplHelperBase::disposing();
}

void Connection::disposeStatements()
{
    MutexGuard aGuard(m_aMutex);
    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();
}

uno::Reference< XTablesSupplier > Connection::createCatalog()
{
    MutexGuard aGuard(m_aMutex);

    // m_xCatalog is a weak reference. Reuse it if it still exists.
    Reference< XTablesSupplier > xCatalog = m_xCatalog;
    if (xCatalog.is())
    {
        return xCatalog;
    }
    else
    {
        xCatalog = new Catalog(this);
        m_xCatalog = xCatalog;
        return m_xCatalog;
    }

}

void Connection::rebuildIndexes() throw(SQLException)
{
    SAL_INFO("connectivity.firebird", "rebuildIndexes()");
    MutexGuard aGuard(m_aMutex);

    // We only need to do this for character based columns on user-created tables.

    // Ideally we'd use a FOR SELECT ... INTO .... DO ..., but that seems to
    // only be possible using PSQL, i.e. using a stored procedure.
    OUString sSql(
        // multiple columns possible per index, only select once
        "SELECT DISTINCT indices.RDB$INDEX_NAME "
        "FROM RDB$INDICES indices "
        "JOIN RDB$INDEX_SEGMENTS index_segments "
        "ON (indices.RDB$INDEX_NAME = index_segments.RDB$INDEX_NAME) "
        "JOIN RDB$RELATION_FIELDS relation_fields "
        "ON (index_segments.RDB$FIELD_NAME = relation_fields.RDB$FIELD_NAME) "
        "JOIN RDB$FIELDS fields "
        "ON (relation_fields.RDB$FIELD_SOURCE = fields.RDB$FIELD_NAME) "

        "WHERE (indices.RDB$SYSTEM_FLAG = 0) "
        // TODO: what about blr_text2 etc. ?
        "AND ((fields.RDB$FIELD_TYPE = " + OUString::number((int) blr_text) + ") "
        "     OR (fields.RDB$FIELD_TYPE = " + OUString::number((int) blr_varying) + ")) "
        "AND (indices.RDB$INDEX_INACTIVE IS NULL OR indices.RDB$INDEX_INACTIVE = 0) "
    );

    uno::Reference< XStatement > xCharIndicesStatement = createStatement();
    uno::Reference< XResultSet > xCharIndices =
                                    xCharIndicesStatement->executeQuery(sSql);
    uno::Reference< XRow > xRow(xCharIndices, UNO_QUERY_THROW);

    uno::Reference< XStatement > xAlterIndexStatement = createStatement();

    // ALTER is a DDL statement, hence using Statement will cause a commit
    // after every alter -- in this case this is inappropriate (xCharIndicesStatement
    // and its ResultSet become invalidated) hence we use the native api.
    while (xCharIndices->next())
    {
        OUString sIndexName(sanitizeIdentifier(xRow->getString(1)));
        SAL_INFO("connectivity.firebird", "rebuilding index " + sIndexName);
        OString sAlterIndex = "ALTER INDEX \""
                               + OUStringToOString(sIndexName, RTL_TEXTENCODING_UTF8)
                               + "\" ACTIVE";

        ISC_STATUS_ARRAY aStatusVector;
        ISC_STATUS aErr;

        aErr = isc_dsql_execute_immediate(aStatusVector,
                                          &getDBHandle(),
                                          &getTransaction(),
                                          0, // Length: 0 for null terminated
                                          sAlterIndex.getStr(),
                                          FIREBIRD_SQL_DIALECT,
                                          NULL);
        if (aErr)
            evaluateStatusVector(aStatusVector,
                                 "rebuildIndexes:isc_dsql_execute_immediate",
                                 *this);
    }
    commit();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
