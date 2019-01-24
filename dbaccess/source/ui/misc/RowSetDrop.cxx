
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

#include <TokenWriter.hxx>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <sqlmessage.hxx>
#include <stringconstants.hxx>
#include <com/sun/star/sdbc/XRowUpdate.hpp>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;

// export data
ORowSetImportExport::ORowSetImportExport(   vcl::Window* _pParent,
                                            const Reference< XResultSetUpdate >& _xResultSetUpdate,
                                            const svx::ODataAccessDescriptor& _aDataDescriptor,
                                            const Reference< XComponentContext >& _rM
                                            )
                                            : ODatabaseImportExport(_aDataDescriptor,_rM,nullptr)
                                            ,m_xTargetResultSetUpdate(_xResultSetUpdate)
                                            ,m_xTargetRowUpdate(_xResultSetUpdate,UNO_QUERY)
                                            ,m_pParent(_pParent)
                                            ,m_bAlreadyAsked(false)
{
    OSL_ENSURE(_pParent,"Window can't be null!");
}

void ORowSetImportExport::initialize()
{
    ODatabaseImportExport::initialize();
    // do namemapping
    Reference<XColumnLocate> xColumnLocate(m_xResultSet,UNO_QUERY);
    OSL_ENSURE(xColumnLocate.is(),"The rowset normally should support this");

    m_xTargetResultSetMetaData = Reference<XResultSetMetaDataSupplier>(m_xTargetResultSetUpdate,UNO_QUERY)->getMetaData();
    if(!m_xTargetResultSetMetaData.is() || !xColumnLocate.is() || !m_xResultSetMetaData.is() )
        throw SQLException(DBA_RES(STR_UNEXPECTED_ERROR),*this,"S1000",0,Any());

    sal_Int32 nCount = m_xTargetResultSetMetaData->getColumnCount();
    m_aColumnMapping.reserve(nCount);
    m_aColumnTypes.reserve(nCount);
    for (sal_Int32 i = 1;i <= nCount; ++i)
    {
        sal_Int32 nPos = COLUMN_POSITION_NOT_FOUND; // means column is autoincrement or doesn't exist
        if(!m_xTargetResultSetMetaData->isAutoIncrement(i))
        {
            try
            {
                OUString sColumnName = m_xTargetResultSetMetaData->getColumnName(i);
                nPos = xColumnLocate->findColumn(sColumnName);
            }
            catch(const SQLException&)
            {
                if(m_xTargetResultSetMetaData->isNullable(i))
                    nPos = 0; // column doesn't exist but we could set it to null
            }
        }

        m_aColumnMapping.push_back(nPos);
        if(nPos > 0)
            m_aColumnTypes.push_back(m_xResultSetMetaData->getColumnType(nPos));
        else
            m_aColumnTypes.push_back(DataType::OTHER);
    }
}

bool ORowSetImportExport::Write()
{
    return true;
}

bool ORowSetImportExport::Read()
{
    // check if there is any column to copy
    if(std::none_of(m_aColumnMapping.begin(),m_aColumnMapping.end(),
                        [](sal_Int32 n) { return n > 0; }))
        return false;
    bool bContinue = true;
    if(m_aSelection.getLength())
    {
        const Any* pBegin = m_aSelection.getConstArray();
        const Any* pEnd   = pBegin + m_aSelection.getLength();
        for(;pBegin != pEnd && bContinue;++pBegin)
        {
            sal_Int32 nPos = -1;
            *pBegin >>= nPos;
            OSL_ENSURE(nPos != -1,"Invalid position!");
            bContinue = (m_xResultSet.is() && m_xResultSet->absolute(nPos) && insertNewRow());
        }
    }
    else
    {
        Reference<XPropertySet> xProp(m_xResultSet,UNO_QUERY);
        sal_Int32 nRowCount = 0;
        if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISROWCOUNTFINAL) )
        {
            bool bFinal = false;
            xProp->getPropertyValue(PROPERTY_ISROWCOUNTFINAL) >>= bFinal;
            if ( !bFinal )
                m_xResultSet->afterLast();
            xProp->getPropertyValue(PROPERTY_ROWCOUNT) >>= nRowCount;
        }
        if ( !nRowCount )
        {
            m_xResultSet->afterLast();
            nRowCount = m_xResultSet->getRow();
        }
        OSL_ENSURE(nRowCount,"RowCount is 0!");
        m_xResultSet->beforeFirst();
        while(m_xResultSet.is() && m_xResultSet->next() && bContinue && nRowCount )
        {
            --nRowCount;
            bContinue = insertNewRow();
        }
    }
    return true;
}

bool ORowSetImportExport::insertNewRow()
{
    try
    {
        m_xTargetResultSetUpdate->moveToInsertRow();
        sal_Int32 i = 1;
        for (auto const& column : m_aColumnMapping)
        {
            if(column > 0)
            {
                Any aValue;
                switch(m_aColumnTypes[i-1])
                {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aValue <<= m_xRow->getString(column);
                        break;
                    case DataType::DECIMAL:
                    case DataType::NUMERIC:
                        aValue <<= m_xRow->getDouble(column);
                        break;
                    case DataType::BIGINT:
                        aValue <<= m_xRow->getLong(column);
                        break;
                    case DataType::FLOAT:
                        aValue <<= m_xRow->getFloat(column);
                        break;
                    case DataType::DOUBLE:
                        aValue <<= m_xRow->getDouble(column);
                        break;
                    case DataType::LONGVARCHAR:
                        aValue <<= m_xRow->getString(column);
                        break;
                    case DataType::LONGVARBINARY:
                        aValue <<= m_xRow->getBytes(column);
                        break;
                    case DataType::DATE:
                        aValue <<= m_xRow->getDate(column);
                        break;
                    case DataType::TIME:
                        aValue <<= m_xRow->getTime(column);
                        break;
                    case DataType::TIMESTAMP:
                        aValue <<= m_xRow->getTimestamp(column);
                        break;
                    case DataType::BIT:
                    case DataType::BOOLEAN:
                        aValue <<= m_xRow->getBoolean(column);
                        break;
                    case DataType::TINYINT:
                        aValue <<= m_xRow->getByte(column);
                        break;
                    case DataType::SMALLINT:
                        aValue <<= m_xRow->getShort(column);
                        break;
                    case DataType::INTEGER:
                        aValue <<= m_xRow->getInt(column);
                        break;
                    case DataType::REAL:
                        aValue <<= m_xRow->getDouble(column);
                        break;
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                        aValue <<= m_xRow->getBytes(column);
                        break;
                    case DataType::BLOB:
                        aValue <<= m_xRow->getBlob(column);
                        break;
                    case DataType::CLOB:
                        aValue <<= m_xRow->getClob(column);
                        break;
                    default:
                        SAL_WARN("dbaccess.ui", "Unknown type");
                }
                if(m_xRow->wasNull())
                    m_xTargetRowUpdate->updateNull(i);
                else
                    m_xTargetRowUpdate->updateObject(i,aValue);
            }
            else if(column == 0)//now we have know that we to set this column to null
                m_xTargetRowUpdate->updateNull(i);
            ++i;
        }
        m_xTargetResultSetUpdate->insertRow();
    }
    catch(const SQLException&)
    {
        if(!m_bAlreadyAsked)
        {
            OUString sAskIfContinue = DBA_RES(STR_ERROR_OCCURRED_WHILE_COPYING);
            OSQLWarningBox aDlg(m_pParent ? m_pParent->GetFrameWeld() : nullptr, sAskIfContinue, MessBoxStyle::YesNo | MessBoxStyle::DefaultYes);
            if (aDlg.run() == RET_YES)
                m_bAlreadyAsked = true;
            else
                return false;
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
