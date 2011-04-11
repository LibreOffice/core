/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "calc/CConnection.hxx"
#include "calc/CDatabaseMetaData.hxx"
#include "calc/CCatalog.hxx"
#include "calc/CDriver.hxx"
#include "resource/calc_res.hrc"
#include "resource/sharedresources.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <tools/urlobj.hxx>
#include "calc/CPreparedStatement.hxx"
#include "calc/CStatement.hxx"
#include <unotools/pathoptions.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/logfile.hxx>

using namespace connectivity::calc;
using namespace connectivity::file;

typedef connectivity::file::OConnection OConnection_BASE;

//------------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::sheet;

// --------------------------------------------------------------------------------

OCalcConnection::OCalcConnection(ODriver* _pDriver) : OConnection(_pDriver),m_nDocCount(0)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::OCalcConnection" );
    // m_aFilenameExtension is not used
}

OCalcConnection::~OCalcConnection()
{
}

void OCalcConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)
    throw(SQLException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::construct" );
    //  open file

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1));

    m_aFileName = aDSN;
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    {
        SvtPathOptions aPathOptions;
        m_aFileName = aPathOptions.SubstituteVariable(m_aFileName);
    }
    aURL.SetSmartURL(m_aFileName);
    if ( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        //  don't pass invalid URL to loadComponentFromURL
        throw SQLException();
    }
    m_aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    m_sPassword = ::rtl::OUString();
    const char* pPwd        = "password";

    const PropertyValue *pIter  = info.getConstArray();
    const PropertyValue *pEnd   = pIter + info.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if(!pIter->Name.compareToAscii(pPwd))
        {
            pIter->Value >>= m_sPassword;
            break;
        }
    } // for(;pIter != pEnd;++pIter)
    ODocHolder aDocHodler(this); // just to test that the doc can be loaded
    acquireDoc();
}
// -----------------------------------------------------------------------------
Reference< XSpreadsheetDocument> OCalcConnection::acquireDoc()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::acquireDoc" );
    if ( m_xDoc.is() )
    {
        osl_incrementInterlockedCount(&m_nDocCount);
        return m_xDoc;
    }
    //  open read-only as long as updating isn't implemented
    Sequence<PropertyValue> aArgs(2);
    aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
    aArgs[0].Value <<= (sal_Bool) sal_True;
    aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
    aArgs[1].Value <<= (sal_Bool) sal_True;

    if ( m_sPassword.getLength() )
    {
        const sal_Int32 nPos = aArgs.getLength();
        aArgs.realloc(nPos+1);
        aArgs[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Password"));
        aArgs[nPos].Value <<= m_sPassword;
    }

    Reference< XComponentLoader > xDesktop( getDriver()->getFactory()->createInstance(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), UNO_QUERY );
    if (!xDesktop.is())
    {
        OSL_FAIL("no desktop");
        throw SQLException();
    }
    Reference< XComponent > xComponent;
    Any aLoaderException;
    try
    {
        xComponent = xDesktop->loadComponentFromURL(
            m_aFileName, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")), 0, aArgs );
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

        const ::rtl::OUString sError( m_aResources.getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE,
            "$filename$", m_aFileName
         ) );
        ::dbtools::throwGenericSQLException( sError, *this, aErrorDetails );
    }
    osl_incrementInterlockedCount(&m_nDocCount);
    return m_xDoc;
}
// -----------------------------------------------------------------------------
void OCalcConnection::releaseDoc()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::releaseDoc" );
    if ( osl_decrementInterlockedCount(&m_nDocCount) == 0 )
        ::comphelper::disposeComponent( m_xDoc );
}
// -----------------------------------------------------------------------------
void OCalcConnection::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::disposing" );
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nDocCount = 0;
    ::comphelper::disposeComponent( m_xDoc );

    OConnection::disposing();
}

// XServiceInfo
// --------------------------------------------------------------------------------

IMPLEMENT_SERVICE_INFO(OCalcConnection, "com.sun.star.sdbc.drivers.calc.Connection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------

Reference< XDatabaseMetaData > SAL_CALL OCalcConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::getMetaData" );
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

//------------------------------------------------------------------------------

::com::sun::star::uno::Reference< XTablesSupplier > OCalcConnection::createCatalog()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::createCatalog" );
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

// --------------------------------------------------------------------------------

Reference< XStatement > SAL_CALL OCalcConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::createStatement" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    Reference< XStatement > xReturn = new OCalcStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

// --------------------------------------------------------------------------------

Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareStatement( const ::rtl::OUString& sql )
    throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::prepareStatement" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);


    OCalcPreparedStatement* pStmt = new OCalcPreparedStatement(this);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

// --------------------------------------------------------------------------------

Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareCall( const ::rtl::OUString& /*sql*/ )
    throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcConnection::prepareCall" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
