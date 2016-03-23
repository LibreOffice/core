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


#include "file/FTable.hxx"
#include "file/FColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <unotools/ucbstreamhelper.hxx>

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

OFileTable::OFileTable(sdbcx::OCollection* _pTables,OConnection* _pConnection)
: OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
                ,m_pConnection(_pConnection)
                ,m_pFileStream(nullptr)
                ,m_nFilePos(0)
                ,m_pBuffer(nullptr)
                ,m_nBufferSize(0)
                ,m_bWriteable(false)
{
    construct();
    m_aColumns = new OSQLColumns();
}

OFileTable::OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                        const OUString& _Name,
                        const OUString& _Type,
                        const OUString& _Description ,
                        const OUString& _SchemaName,
                        const OUString& _CatalogName )
    : OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers(),
                     _Name,
                     _Type,
                     _Description,
                     _SchemaName,
                     _CatalogName)
    , m_pConnection(_pConnection)
    , m_pFileStream(nullptr)
    , m_nFilePos(0)
    , m_pBuffer(nullptr)
    , m_nBufferSize(0)
    , m_bWriteable(false)
{
    m_aColumns = new OSQLColumns();
    construct();
    //  refreshColumns();
}

OFileTable::~OFileTable( )
{
}

void OFileTable::refreshColumns()
{
    TStringVector aVector;
    Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(Any(),
                                                                               m_SchemaName,m_Name, "%");

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(4));
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OColumns(this,m_aMutex,aVector);
}

void OFileTable::refreshKeys()
{
}

void OFileTable::refreshIndexes()
{
}

Any SAL_CALL OFileTable::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XKeysSupplier>::get()||
        rType == cppu::UnoType<XRename>::get()||
        rType == cppu::UnoType<XAlterTable>::get()||
        rType == cppu::UnoType<XIndexesSupplier>::get()||
        rType == cppu::UnoType<XDataDescriptorFactory>::get())
        return Any();

    return OTable_TYPEDEF::queryInterface(rType);
}

void SAL_CALL OFileTable::disposing()
{
    OTable::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    FileClose();
}

Sequence< sal_Int8 > OFileTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
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

sal_Int64 OFileTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}

void OFileTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (m_pFileStream && m_pFileStream->IsWritable())
        m_pFileStream->Flush();

    delete m_pFileStream;
    m_pFileStream = nullptr;

    if (m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = nullptr;
    }
}

void SAL_CALL OFileTable::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}

void SAL_CALL OFileTable::release() throw()
{
    OTable_TYPEDEF::release();
}

bool OFileTable::InsertRow(OValueRefVector& /*rRow*/, bool /*bFlush*/, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& /*_xCols*/)
{
    return false;
}

bool OFileTable::DeleteRow(const OSQLColumns& /*_rCols*/)
{
    return false;
}

bool OFileTable::UpdateRow(OValueRefVector& /*rRow*/, OValueRefRow& /*pOrgRow*/,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& /*_xCols*/)
{
    return false;
}

void OFileTable::addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& /*descriptor*/)
{
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}

void OFileTable::dropColumn(sal_Int32 /*_nPos*/)
{
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}


SvStream* OFileTable::createStream_simpleError( const OUString& _rFileName, StreamMode _eOpenMode)
{
    SvStream* pReturn = ::utl::UcbStreamHelper::CreateStream( _rFileName, _eOpenMode, bool(_eOpenMode & StreamMode::NOCREATE));
    if (pReturn && (ERRCODE_NONE != pReturn->GetErrorCode()))
    {
        delete pReturn;
        pReturn = nullptr;
    }
    return pReturn;
}


void OFileTable::refreshHeader()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
