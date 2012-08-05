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

#include "ado/AKey.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include "ado/AColumns.hxx"
#include "ado/AConnection.hxx"

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
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
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
void SAL_CALL OAdoKey::acquire() throw()
{
    OKey_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoKey::release() throw()
{
    OKey_ADO::release();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
