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
#include <string.h>


#include <connectivity/dbexception.hxx>

using namespace connectivity::mork;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace connectivity;


MQueryHelperResultEntry::MQueryHelperResultEntry()
{
}

MQueryHelperResultEntry::~MQueryHelperResultEntry()
{
}

void MQueryHelperResultEntry::insert( const rtl::OString &key, rtl::OUString &value )
{
    m_Fields[ key ] = value;
}

rtl::OUString MQueryHelperResultEntry::getValue( const rtl::OString &key ) const
{
    FieldMap::const_iterator iter = m_Fields.find( key );
    if ( iter == m_Fields.end() )
    {
        return rtl::OUString();
    }
    else
    {
        return iter->second;
    }
}

void MQueryHelperResultEntry::setValue( const rtl::OString &key, const rtl::OUString & rValue)
{
//    SAL_INFO("connectivity.mork", "MQueryHelper::setValue()" );
//    SAL_INFO("connectivity.mork", "key: " << &key << " value: " << &rValue);

    m_Fields[ key ] = rValue;
}

MQueryHelper::MQueryHelper(const OColumnAlias& _ca)
    :m_nIndex( 0 )
    ,m_bHasMore( sal_True )
    ,m_bAtEnd( sal_False )
    ,m_rColumnAlias( _ca )
{
    m_aResults.clear();
}

MQueryHelper::~MQueryHelper()
{
    OSL_TRACE("IN MQueryHelper::~MQueryHelper()");
    clear_results();
    OSL_TRACE("OUT MQueryHelper::~MQueryHelper()");
}

void MQueryHelper::append(MQueryHelperResultEntry* resEnt)
{
//    SAL_INFO("connectivity.mork", "MQueryHelper::append()" );

    if ( resEnt != NULL ) {
        m_aResults.push_back( resEnt );
        m_bAtEnd   = sal_False;
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
    m_bHasMore = sal_True;
    m_bAtEnd = sal_False;
    clear_results();
}

MQueryHelperResultEntry* MQueryHelper::next()
{
    MQueryHelperResultEntry* result;
    sal_uInt32 index;

    m_aMutex.acquire();
    index = m_nIndex;
    m_aMutex.release();

    result = getByIndex( index + 1) ; // Add 1 as Row is numbered from 1 to N

    if ( result ) {
        m_aMutex.acquire();
        m_nIndex++;
        m_aMutex.release();
    }

    return( result );
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
    SAL_INFO("connectivity.mork", "MQueryHelper::getResultCount()" );
    sal_Int32 result = static_cast<sal_Int32>(m_aResults.size());
    SAL_INFO("connectivity.mork", "result: " << result);

    return result;
}

// -------------------------------------------------------------------------

sal_Bool MQueryHelper::queryComplete() const
{
    return sal_True;
}

sal_Bool MQueryHelper::checkRowAvailable( sal_Int32 nDBRow )
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


sal_Bool MQueryHelper::getRowValue( ORowSetValue& rValue, sal_Int32 nDBRow,const rtl::OUString& aDBColumnName, sal_Int32 nType )
{
    SAL_INFO("connectivity.mork", "MQueryHelper::getRowValue()" );
    MQueryHelperResultEntry* xResEntry = getByIndex( nDBRow );

    OSL_ENSURE( xResEntry != NULL, "xResEntry == NULL");
    if (xResEntry == NULL )
    {
        rValue.setNull();
        return sal_False;
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

    return sal_True;
}

sal_Int32 MQueryHelper::executeQuery(OConnection* xConnection)
{
//    OSL_FAIL( "MQueryHelper::executeQuery" );
    SAL_INFO("connectivity.mork", "MQueryHelper::executeQuery()" );
    reset();

    MorkTableMap::iterator tableIter;
    MorkTableMap *Tables = xConnection->getMorkParser()->getTables( 0x80 );
    MorkRowMap *Rows = 0;
    MorkRowMap::iterator rowIter;

    for ( tableIter = Tables->begin(); tableIter != Tables->end(); tableIter++ )
    {
        // Iterate all tables
        for ( tableIter = Tables->begin(); tableIter != Tables->end(); tableIter++ )
        {
            if (tableIter->first != 1) break;
            Rows =  xConnection->getMorkParser()->getRows( 0x80, &tableIter->second );
            if ( Rows )
            {
                // Iterate all rows
                for ( rowIter = Rows->begin(); rowIter != Rows->end(); rowIter++ )
                {
                    MQueryHelperResultEntry* entry = new MQueryHelperResultEntry();
                    for (MorkCells::iterator CellsIter = rowIter->second.begin();
                         CellsIter != rowIter->second.end(); CellsIter++ )
                    {
                        std::string column = xConnection->getMorkParser()->getColumn(CellsIter->first);
                        std::string value = xConnection->getMorkParser()->getValue(CellsIter->second);

                        //SAL_INFO("connectivity.mork", "key: " << column << " value: " << value);

                        OString key(column.c_str(), static_cast<sal_Int32>(column.size()));
                        OString valueOString(value.c_str(), static_cast<sal_Int32>(value.size()));
                        rtl::OUString valueOUString = ::rtl::OStringToOUString( valueOString, RTL_TEXTENCODING_UTF8 );
                        entry->setValue(key, valueOUString);
                    }
                    append(entry);
                }
            }
        }
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
