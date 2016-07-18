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

#include "calc/CConnection.hxx"
#include "calc/CDatabaseMetaData.hxx"
#include "calc/CCatalog.hxx"
#include "calc/CDriver.hxx"
#include "resource/calc_res.hrc"
#include "resource/sharedresources.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <tools/urlobj.hxx>
#include "calc/CPreparedStatement.hxx"
#include "calc/CStatement.hxx"
#include <unotools/pathoptions.hxx>
#include <unotools/closeveto.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/processfactory.hxx>

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
    throw(SQLException, RuntimeException, DeploymentException, std::exception)
{
    //  open file

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    OUString aDSN(url.copy(nLen+1));

    m_aFileName = aDSN;
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
    m_aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    m_sPassword.clear();
    const char* pPwd        = "password";

    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd   = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if(pIter->Name.equalsAscii(pPwd))
        {
            pIter->Value >>= m_sPassword;
            break;
        }
    } // for(;pIter != pEnd;++pIter)
    ODocHolder aDocHodler(this); // just to test that the doc can be loaded
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
    Sequence<PropertyValue> aArgs(2);
    aArgs[0].Name = "Hidden";
    aArgs[0].Value <<= true;
    aArgs[1].Name = "ReadOnly";
    aArgs[1].Value <<= true;

    if ( !m_sPassword.isEmpty() )
    {
        const sal_Int32 nPos = aArgs.getLength();
        aArgs.realloc(nPos+1);
        aArgs[nPos].Name = "Password";
        aArgs[nPos].Value <<= m_sPassword;
    }

    Reference< XDesktop2 > xDesktop = Desktop::create( getDriver()->getComponentContext() );
    Reference< XComponent > xComponent;
    Any aLoaderException;
    try
    {
        xComponent = xDesktop->loadComponentFromURL(
            m_aFileName, "_blank", 0, aArgs );
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

            SQLException aDetailException;
            aDetailException.Message = m_aResources.getResourceStringWithSubstitution(
                STR_LOAD_FILE_ERROR_MESSAGE,
                "$exception_type$", aLoaderException.getValueTypeName(),
                "$error_message$", aLoaderError.Message
            );
            aErrorDetails <<= aDetailException;
        }

        const OUString sError( m_aResources.getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE,
            "$filename$", m_aFileName
         ) );
        ::dbtools::throwGenericSQLException( sError, *this, aErrorDetails );
    }
    osl_atomic_increment(&m_nDocCount);
    m_pCloseListener.reset(new utl::CloseVeto(m_xDoc, true));
    return m_xDoc;
}

void OCalcConnection::releaseDoc()
{
    if ( osl_atomic_decrement(&m_nDocCount) == 0 )
    {
        m_pCloseListener.reset(); // dispose m_xDoc
        m_xDoc.clear();
    }
}

void OCalcConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nDocCount = 0;
    m_pCloseListener.reset(); // dispose m_xDoc
    m_xDoc.clear();

    OConnection::disposing();
}

// XServiceInfo


IMPLEMENT_SERVICE_INFO(OCalcConnection, "com.sun.star.sdbc.drivers.calc.Connection", "com.sun.star.sdbc.Connection")


Reference< XDatabaseMetaData > SAL_CALL OCalcConnection::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
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
        OCalcCatalog *pCat = new OCalcCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}


Reference< XStatement > SAL_CALL OCalcConnection::createStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new OCalcStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}


Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareStatement( const OUString& sql )
    throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OCalcPreparedStatement* pStmt = new OCalcPreparedStatement(this);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}


Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareCall( const OUString& /*sql*/ )
    throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException( "XConnection::prepareCall", *this );
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
