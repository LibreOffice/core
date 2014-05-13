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


#include "MColumnAlias.hxx"
#include "MQueryHelper.hxx"
#include "MConnection.hxx"

#include "MorkParser.hxx"
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>

#include "resource/mork_res.hrc"
#include "resource/common_res.hrc"

#include <connectivity/dbexception.hxx>
#include <unotools/textsearch.hxx>

using namespace connectivity::mork;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;


extern
::std::vector< sal_Bool > entryMatchedByExpression(MQueryHelper* _aQuery, MQueryExpression* _aExpr, MQueryHelperResultEntry* entry);

MQueryHelperResultEntry::MQueryHelperResultEntry()
{
}

MQueryHelperResultEntry::~MQueryHelperResultEntry()
{
}

OUString MQueryHelperResultEntry::getValue( const OString &key ) const
{
    FieldMap::const_iterator iter = m_Fields.find( key );
    if ( iter == m_Fields.end() )
    {
        return OUString();
    }
    else
    {
        return iter->second;
    }
}

void MQueryHelperResultEntry::setValue( const OString &key, const OUString & rValue)
{
    m_Fields[ key ] = rValue;
}

MQueryHelper::MQueryHelper(const OColumnAlias& _ca)
    :m_nIndex( 0 )
    ,m_bHasMore( true )
    ,m_bAtEnd( false )
    ,m_rColumnAlias( _ca )
    ,m_aError()
{
    m_aResults.clear();
}

MQueryHelper::~MQueryHelper()
{
    clear_results();
    OSL_TRACE("OUT MQueryHelper::~MQueryHelper()");
}


void MQueryHelper::setAddressbook(OUString &ab)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aAddressbook = ab;

    OSL_TRACE("\tOUT MQuery::setAddressbook()");
}

void MQueryHelper::setExpression( MQueryExpression &_expr )
{
    OSL_TRACE("IN MQueryHelper::setExpression()");
    ::osl::MutexGuard aGuard(m_aMutex);

    m_aExpr = _expr;

    OSL_TRACE("\tOUT MQuery::setExpression()");
}

void MQueryHelper::append(MQueryHelperResultEntry* resEnt)
{
    if ( resEnt != NULL ) {
        m_aResults.push_back( resEnt );
        m_bAtEnd   = false;
    }
}

void MQueryHelper::clear_results()
{
    resultsArray::iterator iter = m_aResults.begin();
    while ( iter != m_aResults.end() ) {
        delete (*iter);
        ++iter;
    }
    m_aResults.clear();
}

void MQueryHelper::reset()
{
    m_nIndex = 0;
    m_bHasMore = true;
    m_bAtEnd = false;
    clear_results();
    m_aError.reset();
}

MQueryHelperResultEntry*
MQueryHelper::getByIndex(sal_uInt32 nRow)
{
    // Row numbers are from 1 to N, need to ensure this, and then
    // substract 1
    if ( nRow < 1 ) {
        return( NULL );
    }
    return m_aResults[nRow -1];
}

sal_Int32 MQueryHelper::getResultCount() const
{
    sal_Int32 result = static_cast<sal_Int32>(m_aResults.size());

    return result;
}



bool MQueryHelper::queryComplete() const
{
    return true;
}

bool MQueryHelper::checkRowAvailable( sal_Int32 nDBRow )
{
/*
    while (!queryComplete() && getResultCount() <= (sal_uInt32)nDBRow)
    {
        if ( !m_aQueryHelper->waitForRow( nDBRow ) ) {
            m_aError = m_aQueryHelper->getError();
            return( sal_False );
        }
    }
*/
    return( getResultCount() > nDBRow );
}


bool MQueryHelper::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType )
{
    MQueryHelperResultEntry* xResEntry = getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        rValue.setNull();
        return false;
    }
    switch ( nType )
    {
        case DataType::VARCHAR:
            rValue = xResEntry->getValue( m_rColumnAlias.getProgrammaticNameOrFallbackToUTF8Alias( aDBColumnName ) );
            break;

        default:
            rValue.setNull();
            break;
    }

    return true;
}

sal_Int32 MQueryHelper::executeQuery(OConnection* xConnection)
{
    reset();

    OString oStringTable = OUStringToOString( m_aAddressbook, RTL_TEXTENCODING_UTF8 );
    std::set<int> listRecords;
    bool handleListTable = false;
    MorkParser* xMork = xConnection->getMorkParser(oStringTable);

    // check if we are retrieving the default table
    if (oStringTable != "AddressBook" && oStringTable != "CollectedAddressBook")
    {
        handleListTable = true;
        // retrieve row ids for that list table
        std::string listTable = oStringTable.getStr();
        xMork->getRecordKeysForListTable(listTable, listRecords);
    }
    MorkTableMap::iterator tableIter;
    MorkTableMap *Tables = xMork->getTables( 0x80 );
    if (!Tables)
        return -1;
    MorkRowMap *Rows = 0;
    MorkRowMap::iterator rowIter;

    // Iterate all tables
    for ( tableIter = Tables->begin(); tableIter != Tables->end(); ++tableIter )
    {
        if (tableIter->first != 1) break;
        Rows = xMork->getRows( 0x80, &tableIter->second );
        if ( Rows )
        {
            // Iterate all rows
            for ( rowIter = Rows->begin(); rowIter != Rows->end(); ++rowIter )
            {
                // list specific table
                // only retrieve rowIds that belong to that list table.
                if (handleListTable)
                {
                    int rowId = rowIter->first;
                    // belongs this row id to the list table?
                    if (listRecords.end() == std::find(listRecords.begin(), listRecords.end(), rowId))
                    {
                        // no, skip it
                        continue;
                    }
                }

                MQueryHelperResultEntry* entry = new MQueryHelperResultEntry();
                for (MorkCells::iterator CellsIter = rowIter->second.begin();
                     CellsIter != rowIter->second.end(); ++CellsIter )
                {
                    std::string column = xMork->getColumn(CellsIter->first);
                    std::string value = xMork->getValue(CellsIter->second);
                    OString key(column.c_str(), static_cast<sal_Int32>(column.size()));
                    OString valueOString(value.c_str(), static_cast<sal_Int32>(value.size()));
                    OUString valueOUString = OStringToOUString( valueOString, RTL_TEXTENCODING_UTF8 );
                    entry->setValue(key, valueOUString);
                }
                ::std::vector< sal_Bool > vector = entryMatchedByExpression(this, &m_aExpr, entry);
                bool result = true;
                for (::std::vector<sal_Bool>::iterator iter = vector.begin(); iter != vector.end(); ++iter)
                {
                    result = result && *iter;
                }
                if (result)
                {
                    append(entry);
                }
                else
                {
                    delete entry;
                }
            }
        }
    }
    return 0;
}

::std::vector< sal_Bool > entryMatchedByExpression(MQueryHelper* _aQuery, MQueryExpression* _aExpr, MQueryHelperResultEntry* entry)
{
    ::std::vector< sal_Bool > resultVector;
    MQueryExpression::ExprVector::iterator evIter;
    for( evIter = _aExpr->getExpressions().begin();
         evIter != _aExpr->getExpressions().end();
         ++evIter )
    {
        if ( (*evIter)->isStringExpr() ) {
            MQueryExpressionString* evStr = static_cast<MQueryExpressionString*> (*evIter);
            // Set the 'name' property of the boolString.
            OString attrName = _aQuery->getColumnAlias().getProgrammaticNameOrFallbackToUTF8Alias( evStr->getName() );
            SAL_INFO("connectivity.mork", "Name = " << attrName.getStr());
            bool requiresValue = true;
            OUString currentValue = entry->getValue(attrName);
            if (evStr->getCond() == MQueryOp::Exists || evStr->getCond() == MQueryOp::DoesNotExist)
            {
                requiresValue = false;
            }
            if (requiresValue)
            {
                SAL_INFO("connectivity.mork", "Value = " << evStr->getValue() );
                OUString searchedValue = evStr->getValue();
                if (evStr->getCond() == MQueryOp::Is) {
                    SAL_INFO("connectivity.mork", "MQueryOp::Is; done");
                    resultVector.push_back((currentValue == searchedValue) ? sal_True : sal_False);
                } else if (evStr->getCond() == MQueryOp::IsNot) {
                    SAL_INFO("connectivity.mork", "MQueryOp::IsNot; done");
                    resultVector.push_back((currentValue == searchedValue) ? sal_False : sal_True);
                } else if (evStr->getCond() == MQueryOp::EndsWith) {
                    SAL_INFO("connectivity.mork", "MQueryOp::EndsWith; done");
                    resultVector.push_back((currentValue.endsWith(searchedValue)) ? sal_True : sal_False);
                } else if (evStr->getCond() == MQueryOp::BeginsWith) {
                    SAL_INFO("connectivity.mork", "MQueryOp::BeginsWith; done");
                    resultVector.push_back((currentValue.startsWith(searchedValue)) ? sal_True : sal_False);
                } else if (evStr->getCond() == MQueryOp::Contains) {
                    SAL_INFO("connectivity.mork", "MQueryOp::Contains; done");
                    resultVector.push_back((currentValue.indexOf(searchedValue) == -1) ? sal_False : sal_True);
                } else if (evStr->getCond() == MQueryOp::DoesNotContain) {
                    SAL_INFO("connectivity.mork", "MQueryOp::DoesNotContain; done");
                    resultVector.push_back((currentValue.indexOf(searchedValue) == -1) ? sal_True : sal_False);
                } else if (evStr->getCond() == MQueryOp::RegExp) {
                    SAL_INFO("connectivity.mork", "MQueryOp::RegExp; done");
                    utl::SearchParam param(
                        searchedValue, utl::SearchParam::SRCH_REGEXP);
                    utl::TextSearch ts(param, LANGUAGE_DONTKNOW);
                    sal_Int32 start = 0;
                    sal_Int32 end = currentValue.getLength();
                    resultVector.push_back(
                        ts.SearchForward(currentValue, &start, &end));
                }
            } else if (evStr->getCond() == MQueryOp::Exists) {
                SAL_INFO("connectivity.mork", "MQueryOp::Exists; done");
                resultVector.push_back((currentValue.isEmpty()) ? sal_False : sal_True);
            } else if (evStr->getCond() == MQueryOp::DoesNotExist) {
                SAL_INFO("connectivity.mork", "MQueryOp::DoesNotExist; done");
                resultVector.push_back((currentValue.isEmpty()) ? sal_True : sal_False);
            }
        }
        else if ( (*evIter)->isExpr() ) {
            SAL_INFO("connectivity.mork", "Appending Subquery Expression");
            MQueryExpression* queryExpression = static_cast<MQueryExpression*> (*evIter);
            // recursive call
            ::std::vector<sal_Bool> subquery_result = entryMatchedByExpression(_aQuery, queryExpression, entry);
            MQueryExpression::bool_cond condition = queryExpression->getExpressionCondition();
            if (condition == MQueryExpression::OR) {
                bool result = false;
                for (::std::vector<sal_Bool>::iterator iter =  subquery_result.begin(); iter != subquery_result.end(); ++iter) {
                    result = result || *iter;
                }
                resultVector.push_back(result);
            } else if (condition == MQueryExpression::AND) {
                bool result = true;
                for (::std::vector<sal_Bool>::iterator iter = subquery_result.begin(); iter != subquery_result.end(); ++iter) {
                    result = result && *iter;
                }
                resultVector.push_back(result);
            } else {
                OSL_FAIL("Unknown Expression Type");
            }
        }
        else {
            // Should never see this...
            SAL_WARN("connectivity.mork", "Unknown Expression Type!");
            _aQuery->getError().setResId(STR_ERROR_GET_ROW);
            return resultVector;
        }
    }
    return resultVector;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
