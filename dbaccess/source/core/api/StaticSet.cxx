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


#include "StaticSet.hxx"
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include "stringconstants.hxx"
#include "apitools.hxx"
#include <connectivity/CommonTools.hxx>
#include <comphelper/types.hxx>

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::osl;

void OStaticSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 /*_nPosition*/)
{
    _rRow = *m_aSetIter;
}

// css::sdbcx::XRowLocate
Any SAL_CALL OStaticSet::getBookmark()
{
    return makeAny(getRow());
}

bool SAL_CALL OStaticSet::moveToBookmark( const Any& bookmark )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    return absolute(::comphelper::getINT32(bookmark));
}

sal_Int32 SAL_CALL OStaticSet::compareBookmarks( const Any& _first, const Any& _second )
{
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;
    return (nFirst < nSecond) ? CompareBookmark::LESS : ((nFirst > nSecond) ? CompareBookmark::GREATER : CompareBookmark::EQUAL);
}

bool SAL_CALL OStaticSet::hasOrderedBookmarks(  )
{
    return true;
}

sal_Int32 SAL_CALL OStaticSet::hashBookmark( const Any& bookmark )
{
    return ::comphelper::getINT32(bookmark);
}

bool OStaticSet::fetchRow()
{
    bool bRet = false;
    if ( !m_bEnd && (!m_nMaxRows || sal_Int32(m_aSet.size()) < m_nMaxRows) )
        bRet = m_xDriverSet->next();
    if ( bRet )
    {
        m_aSet.push_back(new connectivity::ORowVector< connectivity::ORowSetValue >(m_xSetMetaData->getColumnCount()));
        m_aSetIter = m_aSet.end() - 1;
        ((*m_aSetIter)->get())[0] = getRow();
        OCacheSet::fillValueRow(*m_aSetIter,((*m_aSetIter)->get())[0]);
    }
    else
        m_bEnd = true;
    return bRet;
}

void OStaticSet::fillAllRows()
{
    if(!m_bEnd)
    {
        sal_Int32 nColumnCount = m_xSetMetaData->getColumnCount();
        while(m_xDriverSet->next())
        {
            ORowSetRow pRow = new connectivity::ORowVector< connectivity::ORowSetValue >(nColumnCount);
            m_aSet.push_back(pRow);
            m_aSetIter = m_aSet.end() - 1;
            (pRow->get())[0] = getRow();
            OCacheSet::fillValueRow(pRow,(pRow->get())[0]);
        }
        m_bEnd = true;
    }
}

// XResultSet
bool SAL_CALL OStaticSet::next()
{
    m_bInserted = m_bUpdated = m_bDeleted = false;

    if(isAfterLast())
        return false;
    if(!m_bEnd) // not yet all records fetched
    {
        ++m_aSetIter;
        if(m_aSetIter == m_aSet.end() && !fetchRow())
            m_aSetIter = m_aSet.end();
    }
    else if(!isAfterLast())
        ++m_aSetIter;
    return !isAfterLast();
}

bool SAL_CALL OStaticSet::isBeforeFirst(  )
{
    return m_aSetIter == m_aSet.begin();
}

bool SAL_CALL OStaticSet::isAfterLast(  )
{
    return m_aSetIter == m_aSet.end() && m_bEnd;
}

void SAL_CALL OStaticSet::beforeFirst(  )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    m_aSetIter = m_aSet.begin();
}

void SAL_CALL OStaticSet::afterLast(  )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    fillAllRows();
    m_aSetIter = m_aSet.end();
}

bool SAL_CALL OStaticSet::first()
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    m_aSetIter = m_aSet.begin()+1;
    if(m_aSetIter == m_aSet.end() && !fetchRow())
        m_aSetIter = m_aSet.end();

    return m_aSetIter != m_aSet.end();
}

bool SAL_CALL OStaticSet::last()
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    fillAllRows();
    m_aSetIter = m_aSet.end()-1;

    return !isBeforeFirst() && !isAfterLast();
}

sal_Int32 SAL_CALL OStaticSet::getRow(  )
{
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    sal_Int32 nPos = m_aSet.size() - (m_aSet.end() - m_aSetIter);
    OSL_ENSURE(nPos > 0,"RowPos is < 0");
    return nPos;
}

bool SAL_CALL OStaticSet::absolute( sal_Int32 row )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;
    OSL_ENSURE(row,"OStaticSet::absolute: INVALID row number!");
    // if row greater 0 than count from end - row means last
    if(row < 0)
    {
        if(!m_bEnd)
            fillAllRows();

        sal_Int32 nRow = getRow();
        nRow += row;
        if(nRow <= (sal_Int32)m_aSet.size())
            m_aSetIter = m_aSet.begin() + nRow;
        else
            m_aSetIter = m_aSet.begin();
    }
    else if(row > 0)
    {
        if(row >= (sal_Int32)m_aSet.size())
        {
            if(!m_bEnd)
            {
                bool bNext = true;
                for(sal_Int32 i=m_aSet.size()-1;i < row && bNext;++i)
                    bNext = fetchRow();
            }

            if(row > (sal_Int32)m_aSet.size())
                m_aSetIter = m_aSet.end();  // check again
            else
                m_aSetIter = m_aSet.begin() + row;
        }
        else
            m_aSetIter = m_aSet.begin() + row;
    }

    return m_aSetIter != m_aSet.end() && m_aSetIter != m_aSet.begin();
}

bool SAL_CALL OStaticSet::previous(  )
{
    m_bInserted = m_bUpdated = m_bDeleted = false;

    if(m_aSetIter != m_aSet.begin())
        --m_aSetIter;

    return m_aSetIter != m_aSet.begin();
}

void SAL_CALL OStaticSet::refreshRow(  )
{
}

bool SAL_CALL OStaticSet::rowUpdated(  )
{
    return m_bUpdated;
}

bool SAL_CALL OStaticSet::rowInserted(  )
{
    return m_bInserted;
}

bool SAL_CALL OStaticSet::rowDeleted(  )
{
    return m_bDeleted;
}

void SAL_CALL OStaticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable )
{
    OCacheSet::insertRow( _rInsertRow,_xTable);
    if(m_bInserted)
    {
        m_aSet.push_back(new ORowVector< ORowSetValue >(*_rInsertRow)); // we don't know where the new row is so we append it to the current rows
        m_aSetIter = m_aSet.end() - 1;
        ((*m_aSetIter)->get())[0] = (_rInsertRow->get())[0] = getBookmark();
        m_bEnd = false;
    }
}

void SAL_CALL OStaticSet::deleteRow(const ORowSetRow& _rDeleteRow ,const connectivity::OSQLTable& _xTable  )
{
    OCacheSet::deleteRow(_rDeleteRow,_xTable);
    if(m_bDeleted)
    {
        ORowSetMatrix::iterator aPos = m_aSet.begin()+(_rDeleteRow->get())[0].getInt32();
        if(aPos == (m_aSet.end()-1))
            m_aSetIter = m_aSet.end();
        m_aSet.erase(aPos);
    }
}

void OStaticSet::reset(const Reference< XResultSet> &_xDriverSet)
{
    OCacheSet::construct(_xDriverSet, m_sRowSetFilter);
    {
        ORowSetMatrix t;
        m_aSet.swap(t);
    }
    m_aSetIter = m_aSet.end();
    m_bEnd = false;
    m_aSet.push_back(nullptr); // this is the beforefirst record
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
