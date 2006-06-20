/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LConnection.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:22:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_EVOAB_LCONNECTION_HXX_
#include "LConnection.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LDATABASEMETADATA_HXX_
#include "LDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LCATALOG_HXX_
#include "LCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_EVOAB_LPREPAREDSTATEMENT_HXX_
#include "LPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LSTATEMENT_HXX_
#include "LStatement.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBCHARSET_HXX_
#include <connectivity/dbcharset.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "LDebug.hxx"
#endif
#ifndef CONNECTIVITY_DIAGNOSE_EX_H
#include "diagnose_ex.h"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace connectivity::evoab;
using namespace connectivity::file;
using namespace vos;

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
         aExceptionType = ::rtl::OUString( ::rtl::OUString::createFromAscii("Unknown" ) );

     ::rtl::OUString aTypeLine( ::rtl::OUString::createFromAscii("\nType: " ) );
     aTypeLine += aExceptionType;

     ::rtl::OUString aMessageLine( ::rtl::OUString::createFromAscii("\nMessage: " ) );
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
    OArgumentList aArgs(2,&aArg1,&aArg2);

    EVO_TRACE_STRING("OEvoabConnection::construct()::aCLICommand = %s\n", aCLICommand );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aWorkingDirPath = %s\n", aWorkingDirPath );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aArg1 = %s\n", aArg1 );
    EVO_TRACE_STRING("OEvoabConnection::construct()::aArg2 = %s\n", aArg2 );
    OProcess aApp( aCLICommand,aWorkingDirPath);
    OSL_VERIFY_EQUALS(
        aApp.execute( (OProcess::TProcessOption)(OProcess::TOption_Hidden | OProcess::TOption_Wait | OProcess::TOption_SearchPath),aArgs),
        OProcess::E_None,
        "Error at execute evolution-addressbook-export to get VCards");


    Sequence<PropertyValue> aDriverParam;
    ::std::vector<PropertyValue> aParam;

    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("EnableSQL92Check"), 0, Any(), PropertyState_DIRECT_VALUE));
     ::dbtools::OCharsetMap aLookupIanaName;
     ::dbtools::OCharsetMap::const_iterator aLookup = aLookupIanaName.find(RTL_TEXTENCODING_UTF8);
     aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("CharSet"), 0,
                                    makeAny((*aLookup).getIanaName()), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("Extension"), 0, makeAny(getDriver()->getFileExt()), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("HeaderLine"), 0, makeAny(m_bHeaderLine), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("FieldDelimiter"), 0, makeAny(::rtl::OUString(&m_cFieldDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("StringDelimiter"), 0, makeAny(::rtl::OUString(&m_cStringDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("DecimalDelimiter"), 0, makeAny(::rtl::OUString(&m_cDecimalDelimiter,1)), PropertyState_DIRECT_VALUE));
    aParam.push_back(PropertyValue(::rtl::OUString::createFromAscii("ThousandDelimiter"), 0, makeAny(::rtl::OUString(&m_cThousandDelimiter,1)), PropertyState_DIRECT_VALUE));

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
            ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii("No subschema provided"),NULL);
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
