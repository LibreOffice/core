/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "StaticSet.hxx"
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include "dbastrings.hrc"
#include "apitools.hxx"
#include <connectivity/CommonTools.hxx>
#include <comphelper/types.hxx>
#include <rtl/logfile.hxx>

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::fillValueRow" );
    _rRow = *m_aSetIter;
}

// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL OStaticSet::getBookmark() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::getBookmark" );
    return makeAny(getRow());
}

sal_Bool SAL_CALL OStaticSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::moveToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return absolute(::comphelper::getINT32(bookmark));
}

sal_Bool SAL_CALL OStaticSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::moveRelativeToBookmark" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return absolute(::comphelper::getINT32(bookmark)+rows);
}

sal_Int32 SAL_CALL OStaticSet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::compareBookmarks" );
    sal_Int32 nFirst = 0, nSecond = 0;
    _first >>= nFirst;
    _second >>= nSecond;
    return (nFirst < nSecond) ? CompareBookmark::LESS : ((nFirst > nSecond) ? CompareBookmark::GREATER : CompareBookmark::EQUAL);
}

sal_Bool SAL_CALL OStaticSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::hasOrderedBookmarks" );
    return sal_True;
}

sal_Int32 SAL_CALL OStaticSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::hashBookmark" );
    return ::comphelper::getINT32(bookmark);
}

sal_Bool OStaticSet::fetchRow()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::fetchRow" );
    sal_Bool bRet = sal_False;
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
        m_bEnd = sal_True;
    return bRet;
}

void OStaticSet::fillAllRows()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::fillAllRows" );
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
        m_bEnd = sal_True;
    }
}

// XResultSet
sal_Bool SAL_CALL OStaticSet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::next" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(isAfterLast())
        return sal_False;
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

sal_Bool SAL_CALL OStaticSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::isBeforeFirst" );
    return m_aSetIter == m_aSet.begin();
}

sal_Bool SAL_CALL OStaticSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::isAfterLast" );
    return m_aSetIter == m_aSet.end() && m_bEnd;
}

sal_Bool SAL_CALL OStaticSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::isFirst" );
    return m_aSetIter == m_aSet.begin()+1;
}

sal_Bool SAL_CALL OStaticSet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::isLast" );
    return m_aSetIter == m_aSet.end()-1 && m_bEnd;
}

void SAL_CALL OStaticSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::beforeFirst" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aSetIter = m_aSet.begin();
}

void SAL_CALL OStaticSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aSetIter = m_aSet.end();
}

sal_Bool SAL_CALL OStaticSet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::first" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aSetIter = m_aSet.begin()+1;
    if(m_aSetIter == m_aSet.end() && !fetchRow())
        m_aSetIter = m_aSet.end();

    return m_aSetIter != m_aSet.end();
}

sal_Bool SAL_CALL OStaticSet::last(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::last" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aSetIter = m_aSet.end()-1;

    return !isBeforeFirst() && !isAfterLast();
}

sal_Int32 SAL_CALL OStaticSet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::getRow" );
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    sal_Int32 nPos = m_aSet.size() - (m_aSet.end() - m_aSetIter);
    OSL_ENSURE(nPos > 0,"RowPos is < 0");
    return nPos;
}

sal_Bool SAL_CALL OStaticSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::absolute" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
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
                sal_Bool bNext = sal_True;
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

sal_Bool SAL_CALL OStaticSet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::relative" );
    if(!rows)
        return sal_True;

    sal_Int32 nCurPos = getRow();
    return absolute(nCurPos+rows);
}

sal_Bool SAL_CALL OStaticSet::previous(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::previous" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(m_aSetIter != m_aSet.begin())
        --m_aSetIter;

    return m_aSetIter != m_aSet.begin();
}

void SAL_CALL OStaticSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::refreshRow" );
}

sal_Bool SAL_CALL OStaticSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::rowUpdated" );
    return m_bUpdated;
}

sal_Bool SAL_CALL OStaticSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::rowInserted" );
    return m_bInserted;
}

sal_Bool SAL_CALL OStaticSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::rowDeleted" );
    return m_bDeleted;
}

Sequence< sal_Int32 > SAL_CALL OStaticSet::deleteRows( const Sequence< Any >& rows,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::deleteRows" );
    Sequence< sal_Int32 > aRet(rows.getLength());
    const Any* pBegin   = rows.getConstArray();
    const Any* pEnd     = pBegin + rows.getLength();
    for(sal_Int32 i=0;pBegin != pEnd; ++pBegin,++i)
    {
        deleteRow(*(m_aSet.begin() + comphelper::getINT32(*pBegin)),_xTable);
        aRet.getArray()[i] = m_bDeleted;
    }
    return aRet;
}

void SAL_CALL OStaticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::insertRow" );
    OCacheSet::insertRow( _rInsertRow,_xTable);
    if(m_bInserted)
    {
        m_aSet.push_back(new ORowVector< ORowSetValue >(*_rInsertRow)); // we don't know where the new row is so we append it to the current rows
        m_aSetIter = m_aSet.end() - 1;
        ((*m_aSetIter)->get())[0] = (_rInsertRow->get())[0] = getBookmark();
        m_bEnd = sal_False;
    }
}

void SAL_CALL OStaticSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::updateRow" );
    OCacheSet::updateRow( _rInsertRow,_rOrginalRow,_xTable);
}

void SAL_CALL OStaticSet::deleteRow(const ORowSetRow& _rDeleteRow ,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::deleteRow" );
    OCacheSet::deleteRow(_rDeleteRow,_xTable);
    if(m_bDeleted)
    {
        ORowSetMatrix::iterator aPos = m_aSet.begin()+(_rDeleteRow->get())[0].getInt32();
        if(aPos == (m_aSet.end()-1))
            m_aSetIter = m_aSet.end();
        m_aSet.erase(aPos);
    }
}

void SAL_CALL OStaticSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::cancelRowUpdates" );
}

void SAL_CALL OStaticSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::moveToInsertRow" );
}

void SAL_CALL OStaticSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OStaticSet::moveToCurrentRow" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
