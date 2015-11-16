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

#include <sal/config.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "file/FConnection.hxx"
#include "file/FDatabaseMetaData.hxx"
#include "file/FDriver.hxx"
#include "file/FStatement.hxx"
#include "file/FPreparedStatement.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <tools/urlobj.hxx>
#include "file/FCatalog.hxx"
#include <unotools/pathoptions.hxx>
#include <ucbhelper/content.hxx>
#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>
#include <osl/thread.h>
#include <osl/nlsupport.h>
#include "resource/file_res.hrc"

using namespace connectivity::file;
using namespace dbtools;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::ucb;
using namespace ::ucbhelper;
typedef connectivity::OMetaConnection OConnection_BASE;

OConnection::OConnection(OFileDriver*   _pDriver)
    : OSubComponent<OConnection, OConnection_BASE>(static_cast<cppu::OWeakObject*>(_pDriver), this)
    , m_pDriver(_pDriver)
    , m_bClosed(false)
    , m_bAutoCommit(false)
    , m_bReadOnly(false)
    , m_bShowDeleted(false)
    , m_bCaseSensitiveExtension( true )
    , m_bCheckSQL92(false)
    , m_bDefaultTextEncoding(false)
{
    m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
}

OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();
}

void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}


bool OConnection::matchesExtension( const OUString& _rExt ) const
{
    if ( isCaseSensitveExtension() )
        return ( getExtension() == _rExt );

    OUString sMyExtension( getExtension().toAsciiLowerCase() );
    OUString sExt( _rExt.toAsciiLowerCase() );

    return sMyExtension == sExt;
}


void OConnection::construct(const OUString& url,const Sequence< PropertyValue >& info)
    throw( css::sdbc::SQLException,
           css::uno::RuntimeException,
           css::uno::DeploymentException,
           std::exception)
{
    osl_atomic_increment( &m_refCount );

    OUString aExt;
    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd    = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if( pIter->Name == "Extension" )
            OSL_VERIFY( pIter->Value >>= aExt );
        else if( pIter->Name == "CharSet" )
        {
            OUString sIanaName;
            OSL_VERIFY( pIter->Value >>= sIanaName );

            ::dbtools::OCharsetMap aLookupIanaName;
            ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(sIanaName, ::dbtools::OCharsetMap::IANA());
            if (aLookup != aLookupIanaName.end())
                m_nTextEncoding = (*aLookup).getEncoding();
            else
                m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
        }
        else if( pIter->Name == "ShowDeleted" )
        {
            OSL_VERIFY( pIter->Value >>= m_bShowDeleted );
        }
        else if( pIter->Name == "EnableSQL92Check" )
        {
            pIter->Value >>= m_bCheckSQL92;
        }
    } // for(;pIter != pEnd;++pIter)

    {
        sal_Int32 nLen = url.indexOf(':');
        nLen = url.indexOf(':',nLen+1);
        OUString aDSN(url.copy(nLen+1));

        OUString aFileName = aDSN;
        INetURLObject aURL;
        aURL.SetSmartProtocol(INetProtocol::File);
        {
            SvtPathOptions aPathOptions;
            aFileName = aPathOptions.SubstituteVariable(aFileName);
        }

        aURL.SetSmartURL(aFileName);

        setURL(aURL.GetMainURL(INetURLObject::NO_DECODE));
    }

    if ( m_nTextEncoding == RTL_TEXTENCODING_DONTKNOW )
    {
        //m_nTextEncoding = osl_getTextEncodingFromLocale(NULL);
        m_nTextEncoding = osl_getThreadTextEncoding();
        m_bDefaultTextEncoding = true;
    }

    if ( !aExt.isEmpty() )
        m_aFilenameExtension = aExt;

    try
    {
        ::ucbhelper::Content aFile;
        try
        {
            aFile = ::ucbhelper::Content(getURL(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
        }
        catch(ContentCreationException& e)
        {
            throwUrlNotValid(getURL(),e.Message);
        }

        // set fields to fetch
        Sequence< OUString > aProps { "Title" };

        try
        {
            if (aFile.isFolder())
            {
                m_xDir = aFile.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
                m_xContent = aFile.get();
            }
            else if (aFile.isDocument())
            {
                Reference<XContent> xParent(Reference<XChild>(aFile.get(),UNO_QUERY)->getParent(),UNO_QUERY);
                Reference<XContentIdentifier> xIdent = xParent->getIdentifier();
                m_xContent = xParent;

                ::ucbhelper::Content aParent(xIdent->getContentIdentifier(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
                m_xDir = aParent.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
            }
            else
            {
                OSL_FAIL("OConnection::construct: ::ucbhelper::Content isn't a folder nor a document! How that?!");
                throw SQLException();
            }
        }
        catch(Exception& e) // a exception is thrown when no file exists
        {
            throwUrlNotValid(getURL(),e.Message);
        }
        if(!m_xDir.is() || !m_xContent.is())
            throwUrlNotValid(getURL(),OUString());

        if (m_aFilenameExtension.indexOf('*') >= 0 || m_aFilenameExtension.indexOf('?') >= 0)
            throw SQLException();
    }
    catch(const Exception&)
    {
        osl_atomic_decrement( &m_refCount );
        throw;
    }

    osl_atomic_decrement( &m_refCount );
}
// XServiceInfo

IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.file.Connection", "com.sun.star.sdbc.Connection")


Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OPreparedStatement* pStmt = new OPreparedStatement(this);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& /*sql*/ ) throw(SQLException, RuntimeException, std::exception)
{
    throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    return nullptr;
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    return sql;
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_bAutoCommit = autoCommit;
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_bAutoCommit;
}

void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException, std::exception)
{
}

void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException, std::exception)
{
}

sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new ODatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_bReadOnly = readOnly;
}

sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return m_bReadOnly;
}

void SAL_CALL OConnection::setCatalog( const OUString& /*catalog*/ ) throw(SQLException, RuntimeException, std::exception)
{
    throwFeatureNotImplementedSQLException( "XConnection::setCatalog", *this );
}

OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException, std::exception)
{
    return OUString();
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException, std::exception)
{
    throwFeatureNotImplementedSQLException( "XConnection::setTransactionIsolation", *this );
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException, std::exception)
{
    return 0;
}

Reference< XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException, std::exception)
{
}

// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
}

void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OConnection_BASE::disposing();

    m_bClosed   = true;
m_xDir.clear();
m_xContent.clear();
    m_xCatalog  = WeakReference< XTablesSupplier>();

    dispose_ChildImpl();
}

Reference< XTablesSupplier > OConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        xTab = new OFileCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}

Reference< XDynamicResultSet > OConnection::getDir() const
{
    Reference<XDynamicResultSet> xContent;
    Sequence< OUString > aProps { "Title" };
    try
    {
        Reference<XContentIdentifier> xIdent = getContent()->getIdentifier();
        ::ucbhelper::Content aParent(xIdent->getContentIdentifier(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
        xContent = aParent.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
    }
    catch(Exception&)
    {
    }
    return xContent;
}

sal_Int64 SAL_CALL OConnection::getSomething( const Sequence< sal_Int8 >& rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        ? reinterpret_cast< sal_Int64 >( this )
        : (sal_Int64)0;
}

Sequence< sal_Int8 > OConnection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void OConnection::throwUrlNotValid(const OUString & _rsUrl,const OUString & _rsMessage)
{
    SQLException aError;
    aError.Message = getResources().getResourceStringWithSubstitution(
                STR_NO_VALID_FILE_URL,
                "$URL$", _rsUrl
            );

    aError.SQLState = "S1000";
    aError.ErrorCode = 0;
    aError.Context = static_cast< XConnection* >(this);
    if (!_rsMessage.isEmpty())
        aError.NextException <<= SQLException(_rsMessage, aError.Context, OUString(), 0, Any());

    throw aError;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
