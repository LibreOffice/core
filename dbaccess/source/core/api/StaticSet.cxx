/*************************************************************************
 *
 *  $RCSfile: StaticSet.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:01:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBACCESS_CORE_API_STATICSET_HXX
#include "StaticSet.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
//  using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//  using namespace ::cppu;
using namespace ::osl;

// -------------------------------------------------------------------------
void OStaticSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
    _rRow = *m_aSetIter;
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL OStaticSet::getBookmark( const ORowSetRow& _rRow ) throw(SQLException, RuntimeException)
{
    return makeAny(getRow());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return absolute(::comphelper::getINT32(bookmark));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return absolute(::comphelper::getINT32(bookmark)+rows);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStaticSet::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    sal_Int32 nFirst,nSecond;
    first >>= nFirst;
    second >>= nSecond;
    return (nFirst < nSecond) ? CompareBookmark::LESS : ((nFirst > nSecond) ? CompareBookmark::GREATER : CompareBookmark::EQUAL);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStaticSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    return ::comphelper::getINT32(bookmark);
}
// -------------------------------------------------------------------------
sal_Bool OStaticSet::fetchRow()
{
    sal_Bool bRet = sal_False;
    if(!m_bEnd && (bRet = m_xDriverSet->next()))
    {
        m_aSet.push_back(new connectivity::ORowVector< connectivity::ORowSetValue >(m_xSetMetaData->getColumnCount()));
        m_aSetIter = m_aSet.end() - 1;
        (*(*m_aSetIter))[0] = getRow();
        OCacheSet::fillValueRow(*m_aSetIter,(*(*m_aSetIter))[0]);
    }
    else
        m_bEnd = sal_True;
    return bRet;
}
// -------------------------------------------------------------------------
void OStaticSet::fillAllRows()
{
    if(!m_bEnd)
    {
        while(m_xDriverSet->next())
        {
            ORowSetRow pRow = new connectivity::ORowVector< connectivity::ORowSetValue >(m_xSetMetaData->getColumnCount());
            m_aSet.push_back(pRow);
            m_aSetIter = m_aSet.end() - 1;
            (*pRow)[0] = getRow();
            OCacheSet::fillValueRow(pRow,(*pRow)[0]);
        }
        m_bEnd = sal_True;
    }
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL OStaticSet::next(  ) throw(SQLException, RuntimeException)
{
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    return m_aSetIter == m_aSet.begin();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    return m_aSetIter == m_aSet.end() && m_bEnd;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    return m_aSetIter == m_aSet.begin()+1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isLast(  ) throw(SQLException, RuntimeException)
{
    return m_aSetIter == m_aSet.end()-1 && m_bEnd;
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aSetIter = m_aSet.begin();
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aSetIter = m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::first(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_aSetIter = m_aSet.begin()+1;
    if(m_aSetIter == m_aSet.end() && !fetchRow())
        m_aSetIter = m_aSet.end();

    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::last(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    fillAllRows();
    m_aSetIter = m_aSet.end()-1;

    return !isBeforeFirst() && !isAfterLast();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStaticSet::getRow(  ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(!isAfterLast(),"getRow is not allowed when afterlast record!");
    OSL_ENSURE(!isBeforeFirst(),"getRow is not allowed when beforefirst record!");

    sal_Int32 nPos = m_aSet.size() - (m_aSet.end() - m_aSetIter);
    OSL_ENSURE(nPos > 0,"RowPos is < 0");
    return nPos;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    if(!rows)
        return sal_True;

    sal_Int32 nCurPos = getRow();
    return absolute(nCurPos+rows);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::previous(  ) throw(SQLException, RuntimeException)
{
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;

    if(m_aSetIter != m_aSet.begin())
        --m_aSetIter;

    return m_aSetIter != m_aSet.begin();
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    return m_bUpdated;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    return m_bInserted;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    return m_bDeleted;
}
// -------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL OStaticSet::deleteRows( const Sequence< Any >& rows,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
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
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OCacheSet::insertRow( _rInsertRow,_xTable);
    if(m_bInserted)
    {
        m_aSet.push_back(new ORowVector< ORowSetValue >(*_rInsertRow)); // we don't know where the new row is so we append it to the current rows
        m_aSetIter = m_aSet.end() - 1;
        (*(*m_aSetIter))[0] = (*_rInsertRow)[0] = getBookmark(_rInsertRow);
        m_bEnd = sal_False;
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    OCacheSet::updateRow( _rInsertRow,_rOrginalRow,_xTable);
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::deleteRow(const ORowSetRow& _rDeleteRow ,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    OCacheSet::deleteRow(_rDeleteRow,_xTable);
    if(m_bDeleted)
    {
        ORowSetMatrix::iterator aPos = m_aSet.begin()+(*_rDeleteRow)[0].getInt32();
        if(aPos == (m_aSet.end()-1))
            m_aSetIter = m_aSet.end();
        m_aSet.erase(aPos);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

