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

#include <calc/CConnection.hxx>
#include <calc/CDatabaseMetaData.hxx>
#include <calc/CCatalog.hxx>
#include <calc/CDriver.hxx>
#include <resource/sharedresources.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <tools/urlobj.hxx>
#include <component/CPreparedStatement.hxx>
#include <component/CStatement.hxx>
#include <unotools/pathoptions.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <strings.hrc>

using namespace connectivity::calc;
using namespace connectivity::file;

typedef connectivity::file::OConnection OConnection_BASE;


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::sheet;


OCalcConnection::OCalcConnection(ODriver* _pDriver) : OConnection(_pDriver),m_nDocCount(0)
{
    // m_aFilenameExtension is not used
}

OCalcConnection::~OCalcConnection()
{
}

void OCalcConnection::construct(const OUString& url,const Sequence< PropertyValue >& info)
{
    //  open file

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);

    m_aFileName = url.copy(nLen+1); // DSN

    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    {
        SvtPathOptions aPathOptions;
        m_aFileName = aPathOptions.SubstituteVariable(m_aFileName);
    }
    aURL.SetSmartURL(m_aFileName);
    if ( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        //  don't pass invalid URL to loadComponentFromURL
        throw SQLException();
    }
    m_aFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    m_sPassword.clear();

    for (auto& propval : info)
    {
        if (propval.Name == "password")
        {
            propval.Value >>= m_sPassword;
            break;
        }
    }
    ODocHolder aDocHolder(this); // just to test that the doc can be loaded
    acquireDoc();
}

Reference< XSpreadsheetDocument> const & OCalcConnection::acquireDoc()
{
    if ( m_xDoc.is() )
    {
        osl_atomic_increment(&m_nDocCount);
        return m_xDoc;
    }
    //  open read-only as long as updating isn't implemented
    Sequence<PropertyValue> aArgs(m_sPassword.isEmpty() ? 2 : 3);
    auto pArgs = aArgs.getArray();
    pArgs[0].Name = "Hidden";
    pArgs[0].Value <<= true;
    pArgs[1].Name = "ReadOnly";
    pArgs[1].Value <<= true;

    if ( !m_sPassword.isEmpty() )
    {
        pArgs[2].Name = "Password";
        pArgs[2].Value <<= m_sPassword;
    }

    Reference< XDesktop2 > xDesktop = Desktop::create( getDriver()->getComponentContext() );
    Reference< XComponent > xComponent;
    Any aLoaderException;
    try
    {
        xComponent = xDesktop->loadComponentFromURL(
            m_aFileName, u"_blank"_ustr, 0, aArgs );
    }
    catch( const Exception& )
    {
        aLoaderException = ::cppu::getCaughtException();
    }

    m_xDoc.set(xComponent, UNO_QUERY );

    //  if the URL is not a spreadsheet document, throw the exception here
    //  instead of at the first access to it
    if ( !m_xDoc.is() )
    {
        Any aErrorDetails;
        if ( aLoaderException.hasValue() )
        {
            Exception aLoaderError;
            OSL_VERIFY( aLoaderException >>= aLoaderError );

            SQLException aDetailException(m_aResources.getResourceStringWithSubstitution(
                                              STR_LOAD_FILE_ERROR_MESSAGE, "$exception_type$",
                                              aLoaderException.getValueTypeName(),
                                              "$error_message$", aLoaderError.Message),
                                          {}, {}, 0, {});
            aErrorDetails <<= aDetailException;
        }

        const OUString sError( m_aResources.getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE,
            "$filename$", m_aFileName
         ) );
        ::dbtools::throwGenericSQLException( sError, *this, aErrorDetails );
    }
    osl_atomic_increment(&m_nDocCount);
    m_xCloseVetoButTerminateListener.set(new CloseVetoButTerminateListener);
    m_xCloseVetoButTerminateListener->start(m_xDoc, xDesktop);
    return m_xDoc;
}

void OCalcConnection::releaseDoc()
{
    if ( osl_atomic_decrement(&m_nDocCount) == 0 )
    {
        if (m_xCloseVetoButTerminateListener.is())
        {
            m_xCloseVetoButTerminateListener->stop();   // dispose m_xDoc
            m_xCloseVetoButTerminateListener.clear();
        }
        m_xDoc.clear();
    }
}

void OCalcConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nDocCount = 0;
    if (m_xCloseVetoButTerminateListener.is())
    {
        m_xCloseVetoButTerminateListener->stop();   // dispose m_xDoc
        m_xCloseVetoButTerminateListener.clear();
    }
    m_xDoc.clear();

    OConnection::disposing();
}

// XServiceInfo


IMPLEMENT_SERVICE_INFO(OCalcConnection, u"com.sun.star.sdbc.drivers.calc.Connection"_ustr, u"com.sun.star.sdbc.Connection"_ustr)


Reference< XDatabaseMetaData > SAL_CALL OCalcConnection::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OCalcDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}


css::uno::Reference< XTablesSupplier > OCalcConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        xTab = new OCalcCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}


Reference< XStatement > SAL_CALL OCalcConnection::createStatement(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new connectivity::component::OComponentStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}


Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareStatement( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    rtl::Reference<connectivity::component::OComponentPreparedStatement> pStmt = new connectivity::component::OComponentPreparedStatement(this);
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}


Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareCall( const OUString& /*sql*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( u"XConnection::prepareCall"_ustr, *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
