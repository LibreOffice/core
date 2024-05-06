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


#include <file/FTable.hxx>
#include <file/FColumns.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <comphelper/servicehelper.hxx>
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
                ,m_nFilePos(0)
                ,m_nBufferSize(0)
                ,m_bWriteable(false)
{
    construct();
    m_aColumns = new OSQLColumns();
}

OFileTable::OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                        const OUString& Name,
                        const OUString& Type,
                        const OUString& Description ,
                        const OUString& SchemaName,
                        const OUString& CatalogName )
    : OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers(),
                     Name,
                     Type,
                     Description,
                     SchemaName,
                     CatalogName)
    , m_pConnection(_pConnection)
    , m_nFilePos(0)
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
    ::std::vector< OUString> aVector;
    Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(Any(),
                                                                               m_SchemaName,m_Name, u"%"_ustr);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(4));
    }

    if(m_xColumns)
        m_xColumns->reFill(aVector);
    else
        m_xColumns.reset(new OColumns(this,m_aMutex,aVector));
}

void OFileTable::refreshKeys()
{
}

void OFileTable::refreshIndexes()
{
}

Any SAL_CALL OFileTable::queryInterface( const Type & rType )
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

void OFileTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_pFileStream.reset();
    m_pBuffer.reset();
}

bool OFileTable::InsertRow(OValueRefVector& /*rRow*/, const css::uno::Reference< css::container::XIndexAccess>& /*_xCols*/)
{
    return false;
}

bool OFileTable::DeleteRow(const OSQLColumns& /*_rCols*/)
{
    return false;
}

bool OFileTable::UpdateRow(OValueRefVector& /*rRow*/, OValueRefRow& /*pOrgRow*/,const css::uno::Reference< css::container::XIndexAccess>& /*_xCols*/)
{
    return false;
}

void OFileTable::addColumn(const css::uno::Reference< css::beans::XPropertySet>& /*descriptor*/)
{
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}

void OFileTable::dropColumn(sal_Int32 /*_nPos*/)
{
    OSL_FAIL( "OFileTable::addColumn: not implemented!" );
}


std::unique_ptr<SvStream> OFileTable::createStream_simpleError( const OUString& _rFileName, StreamMode _eOpenMode)
{
    std::unique_ptr<SvStream> pReturn(::utl::UcbStreamHelper::CreateStream( _rFileName, _eOpenMode, bool(_eOpenMode & StreamMode::NOCREATE)));
    if (pReturn && (ERRCODE_NONE != pReturn->GetErrorCode()))
    {
        pReturn.reset();
    }
    return pReturn;
}


void OFileTable::refreshHeader()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
