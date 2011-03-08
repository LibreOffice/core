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
#include "ado/AIndex.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include "ado/AColumns.hxx"
#include <comphelper/extract.hxx>
#include "TConnection.hxx"
#include <comphelper/types.hxx>

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
void SAL_CALL OAdoIndex::acquire() throw()
{
    OIndex_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoIndex::release() throw()
{
    OIndex_ADO::release();
}
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
