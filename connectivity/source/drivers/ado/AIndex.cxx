/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AIndex.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:13:56 $
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

#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AIndex.hxx"
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
OAdoIndex::OAdoIndex(sal_Bool _bCase,OConnection* _pConnection,ADOIndex* _pIndex)
    : OIndex_ADO(::rtl::OUString(),::rtl::OUString(),sal_False,sal_False,sal_False,_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aIndex = WpADOIndex(_pIndex);
    fillPropertyValues();
}
// -------------------------------------------------------------------------
OAdoIndex::OAdoIndex(sal_Bool _bCase,OConnection* _pConnection)
    : OIndex_ADO(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aIndex.Create();
}

// -------------------------------------------------------------------------

void OAdoIndex::refreshColumns()
{
    TStringVector aVector;

    WpADOColumns aColumns;
    if ( m_aIndex.IsValid() )
    {
        aColumns = m_aIndex.get_Columns();
        aColumns.fillElementNames(aVector);
    }

    if ( m_pColumns )
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new OColumns(*this,m_aMutex,aVector,aColumns,isCaseSensitive(),m_pConnection);
}

// -------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoIndex::getUnoTunnelImplementationId()
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
sal_Int64 OAdoIndex::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OIndex_ADO::getSomething(rId);
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoIndex::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aIndex.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aIndex.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_CATALOG:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aIndex.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_ISUNIQUE:
                m_aIndex.put_Unique(getBOOL(rValue));
                break;
            case PROPERTY_ID_ISPRIMARYKEYINDEX:
                m_aIndex.put_PrimaryKey(getBOOL(rValue));
                break;
            case PROPERTY_ID_ISCLUSTERED:
                m_aIndex.put_Clustered(getBOOL(rValue));
                break;
        }
    }
    OIndex_ADO::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoIndex::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OIndex_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoIndex::release() throw(::com::sun::star::uno::RuntimeException)
{
    OIndex_ADO::release();
}
// -----------------------------------------------------------------------------




