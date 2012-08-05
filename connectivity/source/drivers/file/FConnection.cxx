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
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::ucb;
using namespace ::ucbhelper;
using rtl::OUString;
typedef connectivity::OMetaConnection OConnection_BASE;
// --------------------------------------------------------------------------------
OConnection::OConnection(OFileDriver*   _pDriver)
                         : OSubComponent<OConnection, OConnection_BASE>((::cppu::OWeakObject*)_pDriver, this)
                         ,m_pDriver(_pDriver)
                         ,m_bClosed(sal_False)
                         ,m_bShowDeleted(sal_False)
                         ,m_bCaseSensitiveExtension( sal_True )
                         ,m_bCheckSQL92(sal_False)
                         ,m_bDefaultTextEncoding(false)
{
    m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
}
//-----------------------------------------------------------------------------
OConnection::~OConnection()
{
    if(!isClosed(  ))
        close();
}
//-----------------------------------------------------------------------------
void SAL_CALL OConnection::release() throw()
{
    relase_ChildImpl();
}

//-----------------------------------------------------------------------------
sal_Bool OConnection::matchesExtension( const String& _rExt ) const
{
    if ( isCaseSensitveExtension() )
        return ( getExtension() == _rExt );

    String sMyExtension( getExtension() );
    sMyExtension.ToLowerAscii();
    String sExt( _rExt );
    sExt.ToLowerAscii();

    return sMyExtension == sExt;
}

//-----------------------------------------------------------------------------
void OConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );

    ::rtl::OUString aExt;
    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd    = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if(0 == pIter->Name.compareToAscii("Extension"))
            OSL_VERIFY( pIter->Value >>= aExt );
        else if(0 == pIter->Name.compareToAscii("CharSet"))
        {
            ::rtl::OUString sIanaName;
            OSL_VERIFY( pIter->Value >>= sIanaName );

            ::dbtools::OCharsetMap aLookupIanaName;
            ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(sIanaName, ::dbtools::OCharsetMap::IANA());
            if (aLookup != aLookupIanaName.end())
                m_nTextEncoding = (*aLookup).getEncoding();
            else
                m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
        }
        else if (0 == pIter->Name.compareToAscii("ShowDeleted"))
        {
            OSL_VERIFY( pIter->Value >>= m_bShowDeleted );
        }
        else if (0 == pIter->Name.compareToAscii("EnableSQL92Check"))
        {
            pIter->Value >>= m_bCheckSQL92;
        }
    } // for(;pIter != pEnd;++pIter)

    {
        sal_Int32 nLen = url.indexOf(':');
        nLen = url.indexOf(':',nLen+1);
        ::rtl::OUString aDSN(url.copy(nLen+1)),aUID,aPWD;

        String aFileName = aDSN;
        INetURLObject aURL;
        aURL.SetSmartProtocol(INET_PROT_FILE);
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
            aFile = ::ucbhelper::Content(getURL(),Reference< XCommandEnvironment >());
        }
        catch(ContentCreationException& e)
        {
            throwUrlNotValid(getURL(),e.Message);
        }

        // set fields to fetch
        Sequence< OUString > aProps(1);
        OUString* pProps = aProps.getArray();
        pProps[ 0 ] = OUString("Title");

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

                ::ucbhelper::Content aParent(xIdent->getContentIdentifier(),Reference< XCommandEnvironment >());
                m_xDir = aParent.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
            }
            else
            {
                OSL_FAIL("OConnection::construct: ::ucbhelper::Content isn't a folde nor a document! How that?!");
                throw SQLException();
            }
        }
        catch(Exception& e) // a execption is thrown when no file exists
        {
            throwUrlNotValid(getURL(),e.Message);
        }
        if(!m_xDir.is() || !m_xContent.is())
            throwUrlNotValid(getURL(),::rtl::OUString());

        if (m_aFilenameExtension.Search('*') != STRING_NOTFOUND || m_aFilenameExtension.Search('?') != STRING_NOTFOUND)
            throw SQLException();
    }
    catch(const Exception&)
    {
        osl_decrementInterlockedCount( &m_refCount );
        throw;
    }

    osl_decrementInterlockedCount( &m_refCount );
}
// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OConnection, "com.sun.star.sdbc.drivers.file.Connection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OPreparedStatement* pStmt = new OPreparedStatement(this);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::nativeSQL( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    return sql;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_bAutoCommit = autoCommit;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_bAutoCommit;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::commit(  ) throw(SQLException, RuntimeException)
{
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::rollback(  ) throw(SQLException, RuntimeException)
{
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  ) throw(SQLException, RuntimeException)
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
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_bReadOnly = readOnly;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return m_bReadOnly;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setCatalog( const ::rtl::OUString& /*catalog*/ ) throw(SQLException, RuntimeException)
{
    throwFeatureNotImplementedException( "XConnection::setCatalog", *this );
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    throwFeatureNotImplementedException( "XConnection::setTransactionIsolation", *this );
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// --------------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::setTypeMap( const Reference< XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL OConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL OConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
//------------------------------------------------------------------------------
void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OConnection_BASE::disposing();

    m_bClosed   = sal_True;
m_xDir.clear();
m_xContent.clear();
    m_xCatalog  = WeakReference< XTablesSupplier>();

    dispose_ChildImpl();
}
//------------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XDynamicResultSet > OConnection::getDir() const
{
    Reference<XDynamicResultSet> xContent;
    Sequence< ::rtl::OUString > aProps(1);
    ::rtl::OUString* pProps = aProps.getArray();
    pProps[ 0 ] = ::rtl::OUString("Title");
    try
    {
        Reference<XContentIdentifier> xIdent = getContent()->getIdentifier();
        ::ucbhelper::Content aParent(xIdent->getContentIdentifier(),Reference< XCommandEnvironment >());
        xContent = aParent.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
    }
    catch(Exception&)
    {
    }
    return xContent;
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OConnection::getSomething( const Sequence< sal_Int8 >& rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        ? reinterpret_cast< sal_Int64 >( this )
        : (sal_Int64)0;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OConnection::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
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
// -----------------------------------------------------------------------------
void OConnection::throwUrlNotValid(const ::rtl::OUString & _rsUrl,const ::rtl::OUString & _rsMessage)
{
    SQLException aError;
    aError.Message = getResources().getResourceStringWithSubstitution(
                STR_NO_VALID_FILE_URL,
                "$URL$", _rsUrl
            );

    aError.SQLState = ::rtl::OUString("S1000");
    aError.ErrorCode = 0;
    aError.Context = static_cast< XConnection* >(this);
    if (!_rsMessage.isEmpty())
        aError.NextException <<= SQLException(_rsMessage, aError.Context, ::rtl::OUString(), 0, Any());

    throw aError;
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
