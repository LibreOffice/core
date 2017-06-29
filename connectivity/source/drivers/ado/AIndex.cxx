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


OAdoIndex::OAdoIndex(bool _bCase,OConnection* _pConnection,ADOIndex* _pIndex)
    : sdbcx::OIndex(OUString(),OUString(),false,false,false,_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aIndex = WpADOIndex(_pIndex);
    fillPropertyValues();
}

OAdoIndex::OAdoIndex(bool _bCase,OConnection* _pConnection)
    : sdbcx::OIndex(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aIndex.Create();
}


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


Sequence< sal_Int8 > OAdoIndex::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

// css::lang::XUnoTunnel

sal_Int64 OAdoIndex::getSomething( const Sequence< sal_Int8 > & rId )
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : sdbcx::OIndex::getSomething(rId);
}

void SAL_CALL OAdoIndex::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    if(m_aIndex.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    OUString aVal;
                    rValue >>= aVal;
                    m_aIndex.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_CATALOG:
                {
                    OUString aVal;
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
    sdbcx::OIndex::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
