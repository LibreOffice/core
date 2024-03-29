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

#include <component/CTable.hxx>
#include <component/CColumns.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace connectivity;
using namespace connectivity::component;
using namespace connectivity::file;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;


OComponentTable::OComponentTable(sdbcx::OCollection* _pTables,file::OConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description ,
                    const OUString& SchemaName,
                    const OUString& CatalogName
                ) : OComponentTable_BASE(_pTables,_pConnection,Name,
                                  Type,
                                  Description,
                                  SchemaName,
                                  CatalogName)
                ,m_nDataRows(0)
{
}

void OComponentTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< OUString> aVector;

    for(const auto& rxColumn : *m_aColumns)
        aVector.push_back(Reference< XNamed>(rxColumn,UNO_QUERY_THROW)->getName());

    if(m_xColumns)
        m_xColumns->reFill(aVector);
    else
        m_xColumns.reset(new component::OComponentColumns(this,m_aMutex,aVector));
}

void OComponentTable::refreshIndexes()
{
    //  Writer or Calc table has no index
}


Sequence< Type > SAL_CALL OComponentTable::getTypes(  )
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(   *pBegin == cppu::UnoType<XKeysSupplier>::get()||
                *pBegin == cppu::UnoType<XIndexesSupplier>::get()||
                *pBegin == cppu::UnoType<XRename>::get()||
                *pBegin == cppu::UnoType<XAlterTable>::get()||
                *pBegin == cppu::UnoType<XDataDescriptorFactory>::get()))
            aOwnTypes.push_back(*pBegin);
    }
    aOwnTypes.push_back(cppu::UnoType<css::lang::XUnoTunnel>::get());

    return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
}


Any SAL_CALL OComponentTable::queryInterface( const Type & rType )
{
    if( rType == cppu::UnoType<XKeysSupplier>::get()||
        rType == cppu::UnoType<XIndexesSupplier>::get()||
        rType == cppu::UnoType<XRename>::get()||
        rType == cppu::UnoType<XAlterTable>::get()||
        rType == cppu::UnoType<XDataDescriptorFactory>::get())
        return Any();

    return OTable_TYPEDEF::queryInterface(rType);
}


sal_Int32 OComponentTable::getCurrentLastPos() const
{
    return m_nDataRows;
}

bool OComponentTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    // prepare positioning:

    sal_uInt32 nNumberOfRecords = m_nDataRows;
    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::NEXT:
            m_nFilePos++;
            break;
        case IResultSetHelper::PRIOR:
            if (m_nFilePos > 0)
                m_nFilePos--;
            break;
        case IResultSetHelper::FIRST:
            m_nFilePos = 1;
            break;
        case IResultSetHelper::LAST:
            m_nFilePos = nNumberOfRecords;
            break;
        case IResultSetHelper::RELATIVE1:
            m_nFilePos = (m_nFilePos + nOffset < 0) ? 0
                            : static_cast<sal_uInt32>(m_nFilePos + nOffset);
            break;
        case IResultSetHelper::ABSOLUTE1:
        case IResultSetHelper::BOOKMARK:
            m_nFilePos = static_cast<sal_uInt32>(nOffset);
            break;
    }

    if (m_nFilePos > static_cast<sal_Int32>(nNumberOfRecords))
        m_nFilePos = static_cast<sal_Int32>(nNumberOfRecords) + 1;

    if (m_nFilePos == 0 || m_nFilePos == static_cast<sal_Int32>(nNumberOfRecords) + 1)
    {
        switch(eCursorPosition)
        {
            case IResultSetHelper::PRIOR:
            case IResultSetHelper::FIRST:
                m_nFilePos = 0;
                break;
            case IResultSetHelper::LAST:
            case IResultSetHelper::NEXT:
            case IResultSetHelper::ABSOLUTE1:
            case IResultSetHelper::RELATIVE1:
                if (nOffset > 0)
                    m_nFilePos = nNumberOfRecords + 1;
                else if (nOffset < 0)
                    m_nFilePos = 0;
                break;
            case IResultSetHelper::BOOKMARK:
                m_nFilePos = nTempPos;   // previous position
                break;
        }
        return false;
    }

    //! read buffer / setup row object etc?
    nCurPos = m_nFilePos;
    return true;
}

void OComponentTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OComponentTable_BASE::FileClose();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
