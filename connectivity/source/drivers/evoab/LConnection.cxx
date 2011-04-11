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
#include "LConnection.hxx"
#include "LDatabaseMetaData.hxx"
#include "LCatalog.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <tools/urlobj.hxx>
#include "LPreparedStatement.hxx"
#include "LStatement.hxx"
#include <comphelper/extract.hxx>
#include <connectivity/dbcharset.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/process.h>
#include <tools/debug.hxx>
#include "LDebug.hxx"
#include "diagnose_ex.h"
#include <comphelper/sequence.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/common_res.hrc"

using namespace connectivity::evoab;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

::rtl::OUString implGetExceptionMsg( Exception& e, const ::rtl::OUString& aExceptionType_ )
{
     ::rtl::OUString aExceptionType = aExceptionType_;
     if( aExceptionType.getLength() == 0 )
         aExceptionType = ::rtl::OUString( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unknown")) );

     ::rtl::OUString aTypeLine( RTL_CONSTASCII_USTRINGPARAM("\nType: ") );
     aTypeLine += aExceptionType;

     ::rtl::OUString aMessageLine( RTL_CONSTASCII_USTRINGPARAM("\nMessage: ") );
         aMessageLine += ::rtl::OUString( e.Message );

     ::rtl::OUString aMsg(aTypeLine);
     aMsg += aMessageLine;
         return aMsg;
}

 // Exception type unknown
::rtl::OUString implGetExceptionMsg( Exception& e )
{
         ::rtl::OUString aMsg = implGetExceptionMsg( e, ::rtl::OUString() );
         return aMsg;
}

// --------------------------------------------------------------------------------
OEvoabConnection::OEvoabConnection(OEvoabDriver*    _pDriver) : OConnection(_pDriver)
    ,m_bHeaderLine(sal_True)
    ,m_cFieldDelimiter(',')
    ,m_cStringDelimiter('"')
    ,m_cDecimalDelimiter('.')
    ,m_cThousandDelimiter(' ')
{
    // Initialise m_aColumnAlias.
    m_aColumnAlias.setAlias(_pDriver->getFactory());
}
//-----------------------------------------------------------------------------
OEvoabConnection::~OEvoabConnection()
{
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OEvoabConnection, "com.sun.star.sdbc.drivers.evoab.Connection", "com.sun.star.sdbc.Connection")

//-----------------------------------------------------------------------------
void OEvoabConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );
    EVO_TRACE_STRING("OEvoabConnection::construct()::url = %s\n", url );


    ::rtl::OUString aCLICommand = getDriver()->getEvoab_CLI_EffectiveCommand();
    ::rtl::OUString aWorkingDirPath = getDriver()->getWorkingDirPath();
    ::rtl::OUString aArg1 = ::rtl::OUString::createFromAscii(OEvoabDriver::getEVOAB_CLI_ARG_LIST_FOLDERS());
    ::rtl::OUString aArg2 = ::rtl::OUString::createFromAscii(OEvoabDriver::getEVOAB_CLI_ARG_OUTPUT_FILE_PREFIX());
    aArg2 += aWorkingDirPath;
    aArg2 += getDriver()->getEvoFolderListFileName();

    const sal_uInt32 nArgsCount = 2;
    rtl_uString* pPargs[nArgsCount] = { aArg1.pData, aArgs2.pData };

    EVO_TRACE_STRING("OEvoabConnection::construct()::aCLICommand = %s\n", aCLICommand );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aWorkingDirPath = %s\n", aWorkingDirPath );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aArg1 = %s\n", aArg1 );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aArg2 = %s\n", aArg2 );

    oslProcess aProcess;
    if ( osl_Process_E_None != osl_executeProcess(
                aCLICommand.pData,
                pArgs,
                nArgsCount,
                osl_Process_HIDDEN | osl_Process_WAIT | osl_Process_SEARCHPATH,
                NULL,
                aWorkingDirPath.pData,
                NULL, 0,
                &aProcess ) )
    {
        OSL_TRACE("Error at execute evolution-addressbook-export to get VCards");
        ::dbtools::throwGenericSQLException(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Error at execute evolution-addressbook-export to get VCards")),NULL);
    }

    Sequence<PropertyValue> aDriverParam;
    ::std::vector<PropertyValue> aParam;

    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EnableSQL92Check")), 0, Any(), PropertyState_DIRECT_VALUE));
     ::dbtools::OCharsetMap aLookupIanaName;
     ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(RTL_TEXTENCODING_UTF8);
     aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet")), 0,
                                    makeAny((*aLookup).getIanaName()), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Extension")), 0, makeAny(getDriver()->getFileExt()), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderLine")), 0, makeAny(m_bHeaderLine), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldDelimiter")), 0, makeAny(::rtl::OUString(&m_cFieldDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StringDelimiter")), 0, makeAny(::rtl::OUString(&m_cStringDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DecimalDelimiter")), 0, makeAny(::rtl::OUString(&m_cDecimalDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ThousandDelimiter")), 0, makeAny(::rtl::OUString(&m_cThousandDelimiter,1)), PropertyState_DIRECT_VALUE));

    // build a new parameter sequence from the original parameters, appended by the new parameters from above
        PropertyValue *pParams = aParam.empty() ? 0 : &aParam[0];
        aDriverParam = ::comphelper::concatSequences(
        info,
        Sequence< PropertyValue >( pParams, aParam.size() )
    );

    // transform "sdbc:address:evolution" part of URL to "sdbc:flat:file:///..."
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));
    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( aAddrbookURI.getLength() > 0 )
        {
            aAddrbookScheme= aAddrbookURI;
        }
        else
        {
            OSL_TRACE( "No subschema given!!!\n");
            throwGenericSQLException(STR_URI_SYNTAX_ERROR ,*this);
        }
    }
    else
    {
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);
    }

    EVO_TRACE_STRING("OEvoabConnection::construct()::URI = %s\n", aAddrbookURI );
    EVO_TRACE_STRING("OEvoabConnection::construct()::Scheme = %s\n", aAddrbookScheme );

    //
    // Now we have a URI convert it to a Evolution CLI flat file URI
    //
    // The Mapping being used is:
    //
    // * for Evolution
    //      "sdbc:address:evolution:"        -> "sdbc:flat:file:///(file path generated)

    rtl::OUString aEvoFlatURI;
    if ( aAddrbookScheme.compareToAscii( OEvoabDriver::getSDBC_SCHEME_EVOLUTION() ) == 0 )
    {
        aEvoFlatURI = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "sdbc:flat:" ));
    }


    aEvoFlatURI += getDriver()->getWorkingDirURL();
    EVO_TRACE_STRING("OEvoabConnection::construct()::m_aEvoFlatURI = %s\n", aEvoFlatURI );
    //setURL(aEvoFlatURI);
    m_aEvoFlatURI = aEvoFlatURI;

    osl_decrementInterlockedCount( &m_refCount );
    OConnection::construct(aEvoFlatURI,aDriverParam);
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OEvoabConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OEvoabDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OEvoabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        OEvoabCatalog *pCat = new OEvoabCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OEvoabConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    OEvoabStatement* pStmt = new OEvoabStatement(this);

    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OEvoabConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    OEvoabPreparedStatement* pStmt = new OEvoabPreparedStatement(this);
    Reference< XPreparedStatement > xStmt = pStmt;
    pStmt->construct(sql);

    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OEvoabConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
