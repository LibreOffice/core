/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AKey.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:14:19 $
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

#ifndef _CONNECTIVITY_ADO_KEY_HXX_
#include "ado/AKey.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

// -------------------------------------------------------------------------
OAdoKey::OAdoKey(sal_Bool _bCase,OConnection* _pConnection, ADOKey* _pKey)
    : OKey_ADO(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aKey = WpADOKey(_pKey);
    fillPropertyValues();
}
// -------------------------------------------------------------------------
OAdoKey::OAdoKey(sal_Bool _bCase,OConnection* _pConnection)
    : OKey_ADO(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aKey.Create();
}
// -------------------------------------------------------------------------
void OAdoKey::refreshColumns()
{
    TStringVector aVector;

    WpADOColumns aColumns;
    if ( m_aKey.IsValid() )
    {
        aColumns = m_aKey.get_Columns();
        aColumns.fillElementNames(aVector);
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new OColumns(*this,m_aMutex,aVector,aColumns,isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoKey::getUnoTunnelImplementationId()
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

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoKey::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OKey_ADO::getSomething(rId);
}
// -------------------------------------------------------------------------
void OAdoKey::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aKey.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_Name(aVal);
                    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
                }
                break;
            case PROPERTY_ID_TYPE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_Type(Map2KeyRule(nVal));
                    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
                }
                break;
            case PROPERTY_ID_REFERENCEDTABLE:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_RelatedTable(aVal);
                    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
                }
                break;
            case PROPERTY_ID_UPDATERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_UpdateRule(Map2Rule(nVal));
                    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
                }
                break;
            case PROPERTY_ID_DELETERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_DeleteRule(Map2Rule(nVal));
                    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
                }
                break;
        }
    }
    OKey_ADO::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoKey::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OKey_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoKey::release() throw(::com::sun::star::uno::RuntimeException)
{
    OKey_ADO::release();
}
// -----------------------------------------------------------------------------


