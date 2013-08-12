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

const OUString OConnection::sDBLocation( "firebird.fdb" );

OConnection::OConnection(FirebirdDriver*    _pDriver)
                        :OConnection_BASE(m_aMutex),
                         OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
                         m_xMetaData(NULL),
                         m_bIsEmbedded(sal_False),
                         m_sConnectionURL(),
                         m_sURL(),
                         m_sUser(),
                         m_pDriver(_pDriver),
                         m_bClosed(sal_False),
                         m_bUseOldDateFormat(sal_False),
                         m_bAutoCommit(sal_False),
                         m_bReadOnly(sal_False),
                         m_aTransactionIsolation(TransactionIsolation::REPEATABLE_READ),
                         m_DBHandler(0),
                         m_transactionHandle(0),
                         m_xCatalog(0)
{
    SAL_INFO("connectivity.firebird", "OConnection().");

    m_pDriver->acquire();
}

OConnection::~OConnection()
{
    SAL_INFO("connectivity.firebird", "~OConnection().");

    if(!isClosed())
        close();
    m_pDriver->release();
    m_pDriver = NULL;
}

void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}

void OConnection::construct(const ::rtl::OUString& url, const Sequence< PropertyValue >& info)
    throw(SQLException)
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

        m_sURL = utl::TempFile::CreateTempName() + ".fdb";

        SAL_INFO("connectivity.firebird", "Temporary .fdb location:  "
                    << OUStringToOString(m_sURL,RTL_TEXTENCODING_UTF8 ).getStr());
        if (!bIsNewDatabase)
        {
            SAL_INFO("connectivity.firebird", "Extracting .fdb from .odb" );
            if (!m_xEmbeddedStorage->isStreamElement(sDBLocation))
            {
                ::connectivity::SharedResources aResources;
                const OUString sMessage = aResources.getResourceString(STR_ERROR_NEW_VERSION);
                ::dbtools::throwGenericSQLException(sMessage ,*this);
            }

            Reference< XStream > xDBStream(m_xEmbeddedStorage->openStreamElement(sDBLocation,
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

            xFileAccess->writeFile(m_sURL,xDBStream->getInputStream());
        }
        // TOOO: Get DB properties from XML

    }
    // External file AND/OR remote connection
    else if (url.startsWith("sdbc:firebird:"))
    {
        m_sURL = url.copy(OUString("sdbc:firebird:").getLength());
        if (m_sURL.startsWith("file://")) // TODO: are file urls really like this?
        {
            uno::Reference< ucb::XSimpleFileAccess > xFileAccess(
                ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext()),
                uno::UNO_QUERY);
            if (!xFileAccess->exists(m_sURL))
                bIsNewDatabase = true;
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

        if (m_bIsEmbedded)  // TODO: || m_bIsLocalFile
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
                                   m_sURL.getLength(),
                                   OUStringToOString(m_sURL,RTL_TEXTENCODING_UTF8).getStr(),
                                   &m_DBHandler,
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
                                   m_sURL.getLength(),
                                   OUStringToOString(m_sURL, RTL_TEXTENCODING_UTF8).getStr(),
                                   &m_DBHandler,
                                   dpbLength,
                                   dpbBuffer);
        if (aErr)
        {
            evaluateStatusVector(status, "isc_attach_database", *this);
        }
    }

    if (m_bIsEmbedded) // Add DocumentEventListener to save the .fdb as needed
    {
        uno::Reference< frame::XDesktop2 > xFramesSupplier =
            frame::Desktop::create(::comphelper::getProcessComponentContext());
        uno::Reference< frame::XFrames > xFrames( xFramesSupplier->getFrames(),
                                                                uno::UNO_QUERY);
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

            OUString aURL;

            if (xm.is())
                aURL = xm->getURL();
            if (aURL == aStorageURL)
            {
                uno::Reference<XDocumentEventBroadcaster> xBroadcaster( xm, UNO_QUERY);
                if (xBroadcaster.is())
                    xBroadcaster->addDocumentEventListener( this );
                //TODO: remove in the disposing?
            }
        }
    }

    osl_atomic_decrement( &m_refCount );
}

//----- XServiceInfo ---------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.firebird.OConnection",
                                                    "com.sun.star.sdbc.Connection")

Reference< XBlob> OConnection::createBlob(ISC_QUAD* pBlobId)
    throw(SQLException)
{
    SAL_INFO("connectivity.firebird", "createBlob()");
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    Reference< XBlob > xReturn = new Blob(&m_DBHandler,
                                          &m_transactionHandle,
                                          *pBlobId);

    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}


//----- XConnection ----------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement( )
                                        throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "createStatement().");

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

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

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement(
            const OUString& _sSql)
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "prepareStatement() "
             "called with sql: " << _sSql);
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    if(m_aTypeInfo.empty())
        buildTypeInfo();

    Reference< XPreparedStatement > xReturn = new OPreparedStatement(this,
                                                                     m_aTypeInfo,
                                                                     _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));

    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall(
                const OUString& _sSql ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "prepareCall(). "
             "_sSql: " << _sSql);

    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return NULL;
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& _sSql )
                                        throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    // We do not need to adapt the SQL for Firebird atm.
    return _sSql;
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit )
                                        throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_bAutoCommit = autoCommit;

    if (m_transactionHandle)
    {
        setupTransaction();
    }
}

sal_Bool SAL_CALL OConnection::getAutoCommit() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_bAutoCommit;
}

void OConnection::setupTransaction()
    throw (SQLException)
{
    MutexGuard aGuard( m_aMutex );
    ISC_STATUS status_vector[20];

    // TODO: is this sensible? If we have changed parameters then transaction
    // is lost...
    if (m_transactionHandle)
    {
        clearStatements();
        isc_rollback_transaction(status_vector, &m_transactionHandle);
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
    if (m_bAutoCommit)
        *pTPB++ = isc_tpb_autocommit;
    *pTPB++ = (!m_bReadOnly ? isc_tpb_write : isc_tpb_read);
    *pTPB++ = aTransactionIsolation;
    *pTPB++ = isc_tpb_wait;

    isc_start_transaction(status_vector,
                          &m_transactionHandle,
                          1,
                          &m_DBHandler,
                          pTPB - aTPB, // bytes used in TPB
                          aTPB);

    evaluateStatusVector(status_vector,
                         "isc_start_transaction",
                         *this);
}

isc_tr_handle& OConnection::getTransaction()
    throw (SQLException)
{
    MutexGuard aGuard( m_aMutex );
    if (!m_transactionHandle)
    {
        setupTransaction();
    }
    return m_transactionHandle;
}

void SAL_CALL OConnection::commit() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ISC_STATUS status_vector[20];

    if (!m_bAutoCommit && m_transactionHandle)
    {
        clearStatements();
        isc_commit_transaction(status_vector, &m_transactionHandle);
        evaluateStatusVector(status_vector,
                             "isc_commit_transaction",
                             *this);
    }
}

void SAL_CALL OConnection::rollback() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ISC_STATUS status_vector[20];

    if (!m_bAutoCommit && m_transactionHandle)
    {
        isc_rollback_transaction(status_vector, &m_transactionHandle);
    }
}

sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );

    // just simple -> we are close when we are disposed taht means someone called dispose(); (XComponent)
    return OConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

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

void SAL_CALL OConnection::setReadOnly(sal_Bool readOnly)
                                            throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_bReadOnly = readOnly;
    setupTransaction();
}

sal_Bool SAL_CALL OConnection::isReadOnly() throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_bReadOnly;
}

void SAL_CALL OConnection::setCatalog(const OUString& catalog)
                                            throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setCatalog", *this );
    (void) catalog;
}

OUString SAL_CALL OConnection::getCatalog() throw(SQLException, RuntimeException)
{
    // Unsupported
    return OUString();
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 level ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_aTransactionIsolation = level;
    setupTransaction();
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_aTransactionIsolation;
}

Reference< XNameAccess > SAL_CALL OConnection::getTypeMap() throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::getTypeMap", *this );
    return 0;
}

void SAL_CALL OConnection::setTypeMap(const Reference< XNameAccess >& typeMap)
                                            throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::setTypeMap", *this );
    (void) typeMap;
}

//----- XCloseable -----------------------------------------------------------
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close().");

    // we just dispose us
    {
        MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    // when you collected some warnings -> return it
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
// --------------------------------------------------------------------------------
// XDocumentEventListener
void SAL_CALL OConnection::documentEventOccured( const DocumentEvent& _Event )
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

            Reference< XStream > xDBStream(m_xEmbeddedStorage->openStreamElement(sDBLocation,
                                                            ElementModes::WRITE));

            using namespace ::comphelper;
            Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
            Reference< XInputStream > xInputStream;
            if (xContext.is())
                xInputStream =
                        OStorageHelper::GetInputStreamFromURL(m_sURL, xContext);
            if (xInputStream.is())
                OStorageHelper::CopyInputToOutput( xInputStream,
                                                xDBStream->getOutputStream());
            // TODO: ensure db is in safe state
        }
    }
}
// XEventListener
void SAL_CALL OConnection::disposing( const EventObject& Source ) throw (RuntimeException)
{
    (void) Source;
}
//--------------------------------------------------------------------
void OConnection::buildTypeInfo() throw( SQLException)
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

void OConnection::disposing()
{
    SAL_INFO("connectivity.firebird", "disposing().");

    MutexGuard aGuard(m_aMutex);

    clearStatements();

    m_bClosed   = sal_True;
    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    ISC_STATUS_ARRAY status;            /* status vector */
    if (m_transactionHandle)
    {
        // TODO: confirm whether we need to ask the user here.
        isc_rollback_transaction(status, &m_transactionHandle);
    }

    if (isc_detach_database(status, &m_DBHandler))
    {
        evaluateStatusVector(status, "isc_detach_database", *this);
    }
    // TODO: write to storage again?
    if (m_bIsEmbedded)
    {
        uno::Reference< ucb::XSimpleFileAccess > xFileAccess(
            ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext()),
            uno::UNO_QUERY);
        if (xFileAccess->exists(m_sURL))
            xFileAccess->kill(m_sURL);
    }

    dispose_ChildImpl();
    cppu::WeakComponentImplHelperBase::disposing();
}

void OConnection::clearStatements()
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

uno::Reference< XTablesSupplier > OConnection::createCatalog()
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
