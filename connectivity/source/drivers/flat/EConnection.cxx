/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EConnection.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:28:00 $
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

#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_EDATABASEMETADATA_HXX_
#include "flat/EDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_CATALOG_HXX_
#include "flat/ECatalog.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_FLAT_ODRIVER_HXX_
#include "flat/EDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_FLAT_DPREPAREDSTATEMENT_HXX_
#include "flat/EPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_DSTATEMENT_HXX_
#include "flat/EStatement.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace connectivity::flat;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

// --------------------------------------------------------------------------------
OFlatConnection::OFlatConnection(ODriver*   _pDriver) : OConnection(_pDriver)
    ,m_bHeaderLine(sal_True)
    ,m_cFieldDelimiter(';')
    ,m_cStringDelimiter('"')
    ,m_cDecimalDelimiter(',')
    ,m_cThousandDelimiter('.')
{
}
//-----------------------------------------------------------------------------
OFlatConnection::~OFlatConnection()
{
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OFlatConnection, "com.sun.star.sdbc.drivers.flat.Connection", "com.sun.star.sdbc.Connection")

//-----------------------------------------------------------------------------
void OFlatConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );

    ::rtl::OUString aExt;
    const PropertyValue *pBegin  = info.getConstArray();
    const PropertyValue *pEnd    = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("HeaderLine"))
            m_bHeaderLine = cppu::any2bool(pBegin->Value);
        else if(!pBegin->Name.compareToAscii("FieldDelimiter"))
        {
            ::rtl::OUString aVal;
            pBegin->Value >>= aVal; m_cFieldDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("StringDelimiter"))
        {
            ::rtl::OUString aVal;
            pBegin->Value >>= aVal; m_cStringDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("DecimalDelimiter"))
        {
            ::rtl::OUString aVal;
            pBegin->Value >>= aVal; m_cDecimalDelimiter = aVal.toChar();
        }
        else if(!pBegin->Name.compareToAscii("ThousandDelimiter"))
        {
            ::rtl::OUString aVal;
            pBegin->Value >>= aVal; m_cThousandDelimiter = aVal.toChar();
        }
    }

    osl_decrementInterlockedCount( &m_refCount );
    OConnection::construct(url,info);
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OFlatConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OFlatDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OFlatConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!xTab.is())
    {
        OFlatCatalog *pCat = new OFlatCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OFlatConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    OFlatStatement* pStmt = new OFlatStatement(this);

    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);


    OFlatPreparedStatement* pStmt = new OFlatPreparedStatement(this);
    Reference< XPreparedStatement > xStmt = pStmt;
    pStmt->construct(sql);

    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareCall( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OConnection_B::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedException( "XConnection::prepareCall", *this );
    return NULL;
}


