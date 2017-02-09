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

#include <unotools/textsearch.hxx>

using namespace connectivity::mork;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;


extern
::std::vector<bool> entryMatchedByExpression(MQueryHelper* _aQuery, MQueryExpression* _aExpr, MQueryHelperResultEntry* entry);

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
    :m_rColumnAlias( _ca )
    ,m_aError()
{
    m_aResults.clear();
}

MQueryHelper::~MQueryHelper()
{
    clear_results();
}


void MQueryHelper::setAddressbook(OUString &ab)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aAddressbook = ab;
}

void MQueryHelper::append(MQueryHelperResultEntry* resEnt)
{
    if ( resEnt != nullptr ) {
        m_aResults.push_back( resEnt );
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
    clear_results();
    m_aError.reset();
}

MQueryHelperResultEntry*
MQueryHelper::getByIndex(sal_uInt32 nRow)
{
    // Row numbers are from 1 to N, need to ensure this, and then
    // subtract 1
    if ( nRow < 1 ) {
        return nullptr;
    }
    return m_aResults[nRow -1];
}

sal_Int32 MQueryHelper::getResultCount() const
{
    sal_Int32 result = static_cast<sal_Int32>(m_aResults.size());

    return result;
}


bool MQueryHelper::checkRowAvailable( sal_Int32 nDBRow )
{
/*
    while (!queryComplete() && getResultCount() <= (sal_uInt32)nDBRow)
    {
        if ( !m_aQueryHelper->waitForRow( nDBRow ) ) {
            m_aError = m_aQueryHelper->getError();
            return sal_False;
        }
    }
*/
    return getResultCount() > nDBRow;
}


bool MQueryHelper::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const OUString& aDBColumnName, sal_Int32 nType )
{
    MQueryHelperResultEntry* pResEntry = getByIndex( nDBRow );

    OSL_ENSURE( pResEntry != nullptr, "xResEntry == NULL");
    if (pResEntry == nullptr )
    {
        rValue.setNull();
        return false;
    }
    switch ( nType )
    {
        case DataType::VARCHAR:
            rValue = pResEntry->getValue( m_rColumnAlias.getProgrammaticNameOrFallbackToUTF8Alias( aDBColumnName ) );
            break;

        default:
            rValue.setNull();
            break;
    }

    return true;
}

sal_Int32 MQueryHelper::executeQuery(OConnection* xConnection, MQueryExpression & expr)
{
    reset();

    OString oStringTable = OUStringToOString( m_aAddressbook, RTL_TEXTENCODING_UTF8 );
    std::set<int> listRecords;
    bool handleListTable = false;
    MorkParser* pMork;

    // check if we are retrieving the default table
    if (oStringTable == "AddressBook" || oStringTable == "CollectedAddressBook")
    {
        pMork = xConnection->getMorkParser(oStringTable);
    }
    else
    {
        // Let's try to retrieve the list in Collected Addresses book
        pMork = xConnection->getMorkParser(OString("CollectedAddressBook"));
        if (std::find(pMork->lists_.begin(), pMork->lists_.end(), m_aAddressbook) == pMork->lists_.end())
        {
            // so the list is in Address book
            // TODO : manage case where an address book has been created
            pMork = xConnection->getMorkParser(OString("AddressBook"));
        }
        handleListTable = true;
        // retrieve row ids for that list table
        std::string listTable = oStringTable.getStr();
        pMork->getRecordKeysForListTable(listTable, listRecords);
    }
    MorkTableMap::Map::iterator tableIter;
    MorkTableMap *Tables = pMork->getTables( 0x80 );
    if (!Tables)
        return -1;
    MorkRowMap *Rows = nullptr;
    MorkRowMap::Map::const_iterator rowIter;

    // Iterate all tables
    for ( tableIter = Tables->map.begin(); tableIter != Tables->map.end(); ++tableIter )
    {
        if (tableIter->first != 1) break;
        Rows = MorkParser::getRows( 0x80, &tableIter->second );
        if ( Rows )
        {
            // Iterate all rows
            for ( rowIter = Rows->map.begin(); rowIter != Rows->map.end(); ++rowIter )
            {
                // list specific table
                // only retrieve rowIds that belong to that list table.
                if (handleListTable)
                {
                    int rowId = rowIter->first;
                    // belongs this row id to the list table?
                    if (listRecords.end() == listRecords.find(rowId))
                    {
                        // no, skip it
                        continue;
                    }
                }

                MQueryHelperResultEntry* entry = new MQueryHelperResultEntry();
                for (MorkCells::const_iterator CellsIter = rowIter->second.begin();
                     CellsIter != rowIter->second.end(); ++CellsIter )
                {
                    std::string column = pMork->getColumn(CellsIter->first);
                    std::string value = pMork->getValue(CellsIter->second);
                    OString key(column.c_str(), static_cast<sal_Int32>(column.size()));
                    OString valueOString(value.c_str(), static_cast<sal_Int32>(value.size()));
                    OUString valueOUString = OStringToOUString( valueOString, RTL_TEXTENCODING_UTF8 );
                    entry->setValue(key, valueOUString);
                }
                ::std::vector<bool> vector = entryMatchedByExpression(this, &expr, entry);
                bool result = true;
                for (::std::vector<bool>::const_iterator iter = vector.begin(); iter != vector.end(); ++iter)
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

::std::vector<bool> entryMatchedByExpression(MQueryHelper* _aQuery, MQueryExpression* _aExpr, MQueryHelperResultEntry* entry)
{
    ::std::vector<bool> resultVector;
    MQueryExpression::ExprVector::const_iterator evIter;
    for( evIter = _aExpr->getExpressions().begin();
         evIter != _aExpr->getExpressions().end();
         ++evIter )
    {
        if ( (*evIter)->isStringExpr() ) {
            MQueryExpressionString* evStr = static_cast<MQueryExpressionString*> (*evIter);
            // Set the 'name' property of the boolString.
            OString attrName = _aQuery->getColumnAlias().getProgrammaticNameOrFallbackToUTF8Alias( evStr->getName() );
            SAL_INFO("connectivity.mork", "Name = " << attrName.getStr());
            bool bRequiresValue = true;
            OUString currentValue = entry->getValue(attrName);
            if (evStr->getCond() == MQueryOp::Exists || evStr->getCond() == MQueryOp::DoesNotExist)
            {
                bRequiresValue = false;
            }
            if (bRequiresValue)
            {
                SAL_INFO("connectivity.mork", "Value = " << evStr->getValue() );
                OUString searchedValue = evStr->getValue();
                if (evStr->getCond() == MQueryOp::Is) {
                    SAL_INFO("connectivity.mork", "MQueryOp::Is; done");
                    resultVector.push_back(currentValue == searchedValue);
                } else if (evStr->getCond() == MQueryOp::IsNot) {
                    SAL_INFO("connectivity.mork", "MQueryOp::IsNot; done");
                    resultVector.push_back(currentValue != searchedValue);
                } else if (evStr->getCond() == MQueryOp::EndsWith) {
                    SAL_INFO("connectivity.mork", "MQueryOp::EndsWith; done");
                    resultVector.push_back(currentValue.endsWith(searchedValue));
                } else if (evStr->getCond() == MQueryOp::BeginsWith) {
                    SAL_INFO("connectivity.mork", "MQueryOp::BeginsWith; done");
                    resultVector.push_back(currentValue.startsWith(searchedValue));
                } else if (evStr->getCond() == MQueryOp::Contains) {
                    SAL_INFO("connectivity.mork", "MQueryOp::Contains; done");
                    resultVector.push_back(currentValue.indexOf(searchedValue) != -1);
                } else if (evStr->getCond() == MQueryOp::DoesNotContain) {
                    SAL_INFO("connectivity.mork", "MQueryOp::DoesNotContain; done");
                    resultVector.push_back(currentValue.indexOf(searchedValue) == -1);
                } else if (evStr->getCond() == MQueryOp::RegExp) {
                    SAL_INFO("connectivity.mork", "MQueryOp::RegExp; done");
                    utl::SearchParam param(
                        searchedValue, utl::SearchParam::SearchType::Regexp);
                    utl::TextSearch ts(param, LANGUAGE_DONTKNOW);
                    sal_Int32 start = 0;
                    sal_Int32 end = currentValue.getLength();
                    resultVector.push_back(
                        ts.SearchForward(currentValue, &start, &end));
                }
            } else if (evStr->getCond() == MQueryOp::Exists) {
                SAL_INFO("connectivity.mork", "MQueryOp::Exists; done");
                resultVector.push_back(!currentValue.isEmpty());
            } else if (evStr->getCond() == MQueryOp::DoesNotExist) {
                SAL_INFO("connectivity.mork", "MQueryOp::DoesNotExist; done");
                resultVector.push_back(currentValue.isEmpty());
            }
        }
        else if ( (*evIter)->isExpr() ) {
            SAL_INFO("connectivity.mork", "Appending Subquery Expression");
            MQueryExpression* queryExpression = static_cast<MQueryExpression*> (*evIter);
            // recursive call
            ::std::vector<bool> subquery_result = entryMatchedByExpression(_aQuery, queryExpression, entry);
            MQueryExpression::bool_cond condition = queryExpression->getExpressionCondition();
            if (condition == MQueryExpression::OR) {
                bool result = false;
                for (::std::vector<bool>::const_iterator iter =  subquery_result.begin(); iter != subquery_result.end(); ++iter) {
                    result = result || *iter;
                }
                resultVector.push_back(result);
            } else if (condition == MQueryExpression::AND) {
                bool result = true;
                for (::std::vector<bool>::const_iterator iter = subquery_result.begin(); iter != subquery_result.end(); ++iter) {
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
