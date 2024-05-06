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
#include <comphelper/servicehelper.hxx>
#include <file/FConnection.hxx>
#include <file/FDatabaseMetaData.hxx>
#include <file/FDriver.hxx>
#include <file/FStatement.hxx>
#include <file/FPreparedStatement.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <tools/urlobj.hxx>
#include <file/FCatalog.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/pathoptions.hxx>
#include <ucbhelper/content.hxx>
#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>
#include <o3tl/any.hxx>
#include <osl/thread.h>
#include <strings.hrc>

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
    : m_pDriver(_pDriver)
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

bool OConnection::matchesExtension( const OUString& _rExt ) const
{
    if ( isCaseSensitiveExtension() )
        return ( getExtension() == _rExt );

    OUString sMyExtension( getExtension().toAsciiLowerCase() );
    OUString sExt( _rExt.toAsciiLowerCase() );

    return sMyExtension == sExt;
}


void OConnection::construct(const OUString& url,const Sequence< PropertyValue >& info)
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
            if (auto const numeric = o3tl::tryAccess<sal_uInt16>(pIter->Value))
            {
                m_nTextEncoding = *numeric;
            }
            else
            {
                OUString sIanaName;
                OSL_VERIFY( pIter->Value >>= sIanaName );

                ::dbtools::OCharsetMap aLookupIanaName;
                ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.findIanaName(sIanaName);
                if (aLookup != aLookupIanaName.end())
                    m_nTextEncoding = (*aLookup).getEncoding();
                else
                    m_nTextEncoding = RTL_TEXTENCODING_DONTKNOW;
            }
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
        if (!comphelper::IsFuzzing())
        {
            SvtPathOptions aPathOptions;
            aFileName = aPathOptions.SubstituteVariable(aFileName);
        }

        aURL.SetSmartURL(aFileName);

        setURL(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE));
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
        Sequence< OUString > aProps { u"Title"_ustr };

        try
        {
            if (aFile.isFolder())
            {
                m_xDir = aFile.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
                m_xContent = aFile.get();
            }
            else if (aFile.isDocument())
            {
                Reference<XContent> xParent(Reference<XChild>(aFile.get(),UNO_QUERY_THROW)->getParent(),UNO_QUERY_THROW);
                Reference<XContentIdentifier> xIdent = xParent->getIdentifier();
                m_xContent = xParent;

                ::ucbhelper::Content aParent(xIdent->getContentIdentifier(), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
                m_xDir = aParent.createDynamicCursor(aProps, ::ucbhelper::INCLUDE_DOCUMENTS_ONLY );
            }
            else
            {
                OSL_FAIL("OConnection::construct: ::ucbhelper::Content is neither a folder nor a document! How's that?!");
                throw SQLException();
            }
        }
        catch(Exception& e) // an exception is thrown when no file exists
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

IMPLEMENT_SERVICE_INFO(OConnection, u"com.sun.star.sdbc.drivers.file.Connection"_ustr, u"com.sun.star.sdbc.Connection"_ustr)


Reference< XStatement > SAL_CALL OConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new OStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    rtl::Reference<OPreparedStatement> pStmt = new OPreparedStatement(this);
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

Reference< XPreparedStatement > SAL_CALL OConnection::prepareCall( const OUString& /*sql*/ )
{
    throwFeatureNotImplementedSQLException( u"XConnection::prepareCall"_ustr, *this );
}

OUString SAL_CALL OConnection::nativeSQL( const OUString& sql )
{
    return sql;
}

void SAL_CALL OConnection::setAutoCommit( sal_Bool autoCommit )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    m_bAutoCommit = autoCommit;
}

sal_Bool SAL_CALL OConnection::getAutoCommit(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    return m_bAutoCommit;
}

void SAL_CALL OConnection::commit(  )
{
}

void SAL_CALL OConnection::rollback(  )
{
}

sal_Bool SAL_CALL OConnection::isClosed(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return OConnection_BASE::rBHelper.bDisposed;
}

Reference< XDatabaseMetaData > SAL_CALL OConnection::getMetaData(  )
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

void SAL_CALL OConnection::setReadOnly( sal_Bool readOnly )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    m_bReadOnly = readOnly;
}

sal_Bool SAL_CALL OConnection::isReadOnly(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    return m_bReadOnly;
}

void SAL_CALL OConnection::setCatalog( const OUString& /*catalog*/ )
{
    throwFeatureNotImplementedSQLException( u"XConnection::setCatalog"_ustr, *this );
}

OUString SAL_CALL OConnection::getCatalog(  )
{
    return OUString();
}

void SAL_CALL OConnection::setTransactionIsolation( sal_Int32 /*level*/ )
{
    throwFeatureNotImplementedSQLException( u"XConnection::setTransactionIsolation"_ustr, *this );
}

sal_Int32 SAL_CALL OConnection::getTransactionIsolation(  )
{
    return 0;
}

Reference< XNameAccess > SAL_CALL OConnection::getTypeMap(  )
{
    return nullptr;
}

void SAL_CALL OConnection::setTypeMap( const Reference< XNameAccess >& /*typeMap*/ )
{
}

// XCloseable
void SAL_CALL OConnection::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    }
    dispose();
}

// XWarningsSupplier
Any SAL_CALL OConnection::getWarnings(  )
{
    return Any();
}

void SAL_CALL OConnection::clearWarnings(  )
{
}

void OConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OConnection_BASE::disposing();

    m_xDir.clear();
    m_xContent.clear();
    m_xCatalog  = WeakReference< XTablesSupplier>();
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
    Sequence< OUString > aProps { u"Title"_ustr };
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

sal_Int64 SAL_CALL OConnection::getSomething( const Sequence< sal_Int8 >& rId )
{
    return comphelper::getSomethingImpl(rId, this);
}

const Sequence< sal_Int8 > & OConnection::getUnoTunnelId()
{
    static const comphelper::UnoIdInit implId;
    return implId.getSeq();
}

void OConnection::throwUrlNotValid(const OUString & _rsUrl,const OUString & _rsMessage)
{
    XConnection* context = this;
    css::uno::Any next;
    if (!_rsMessage.isEmpty())
        next <<= SQLException(_rsMessage, context, OUString(), 0, Any());
    SQLException aError(
        getResources().getResourceStringWithSubstitution(STR_NO_VALID_FILE_URL, "$URL$", _rsUrl),
        context, u"S1000"_ustr, 0, next);

    throw aError;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
