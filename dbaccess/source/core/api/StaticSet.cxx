/*************************************************************************
 *
 *  $RCSfile: StaticSet.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-25 07:30:24 $
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
void OStaticSet::fillValueRow(ORowSetRow& _rRow)
{
    _rRow = *m_aSetIter;
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL OStaticSet::getBookmark( const ORowSetRow& _rRow ) throw(SQLException, RuntimeException)
{
    return makeAny((sal_Int32)getRow());
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    m_aSetIter = m_aSet.begin() + getINT32(bookmark);
    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    m_aSetIter = m_aSet.begin() + getINT32(bookmark) + rows - 1;
    return m_aSetIter != m_aSet.end();
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
    return getINT32(bookmark);
}
// -------------------------------------------------------------------------
sal_Bool OStaticSet::fetchRow()
{
    sal_Bool bRet;
    if(bRet = m_xDriverSet->next())
    {
        m_aSet.push_back(new connectivity::ORowVector< ORowSetValue >(m_xSetMetaData->getColumnCount()));
        m_aSetIter = m_aSet.end() - 1;
        (*(*m_aSetIter))[0] = (sal_Int32)(m_aSet.size() -1);
        OCacheSet::fillValueRow(*m_aSetIter);
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
            ORowSetRow pRow = new connectivity::ORowVector< ORowSetValue >(m_xSetMetaData->getColumnCount());
            m_aSet.push_back(pRow);
            (*pRow)[0] = (sal_Int32)(m_aSet.size() -1);
            OCacheSet::fillValueRow(pRow);
        }
        m_bEnd = sal_True;
    }
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL OStaticSet::next(  ) throw(SQLException, RuntimeException)
{
    if(isAfterLast())
        return sal_False;
    if(!m_bEnd)
    {
        if(m_bBeforeFirst)
        {
            if(fetchRow())
                m_bBeforeFirst = sal_False;
        }
        else
        {
            ++m_aSetIter;
            if(m_aSetIter == m_aSet.end())
                fetchRow();
        }
    }
    else
    {
        if(m_bBeforeFirst)
        {
            m_aSetIter = m_aSet.begin();
            m_bBeforeFirst = sal_False;
        }
        else
            ++m_aSetIter;
    }

    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    return !m_bBeforeFirst && m_aSetIter == m_aSet.end() && m_bEnd;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    return !m_bBeforeFirst && m_aSetIter == m_aSet.begin();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::isLast(  ) throw(SQLException, RuntimeException)
{
    return m_aSetIter == m_aSet.end()-1 && m_bEnd;
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    m_bBeforeFirst = sal_True;
    m_aSetIter = m_aSet.end();
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    m_bBeforeFirst = sal_False;
    fillAllRows();
    m_aSetIter = m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::first(  ) throw(SQLException, RuntimeException)
{
    m_bBeforeFirst = sal_False;
    m_aSetIter = m_aSet.begin();
    if(m_aSetIter == m_aSet.end())
        fetchRow();
    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::last(  ) throw(SQLException, RuntimeException)
{
    m_bBeforeFirst = sal_False;
    fillAllRows();
    if(m_aSet.size())
        m_aSetIter = m_aSet.end()-1;
    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStaticSet::getRow(  ) throw(SQLException, RuntimeException)
{
    if(isAfterLast())
        return m_aSet.size() - (m_aSet.end() - m_aSetIter);
    return m_aSet.size() - (m_aSet.end() - m_aSetIter) + 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    OSL_ENSHURE(row,"OStaticSet::absolute: INVALID row number!");
    // if row greater 0 than count from end - row means last
    if(row < 0)
    {
        if(!m_bEnd)
            fillAllRows();
        if(-row > (m_aSet.size()+1)) // + 1 because it can be before the first one
        {
            m_aSetIter = m_aSet.end();
            m_bBeforeFirst = sal_True;
        }
        else
        {
            m_aSetIter = m_aSet.end() + row;
            if(m_aSetIter == m_aSet.begin()-1)
                m_bBeforeFirst = sal_True;
        }

    }
    else if(row > 0)
    {
        m_bBeforeFirst = sal_False;
        if(row > m_aSet.size())
        {
            if(!m_bEnd)
            {
                for(sal_Int32 i=m_aSet.size();i < row;++i)
                    fetchRow();
            }
            if(row > (m_aSet.size()+1)) // + 1 because it can be after the last one
                m_aSetIter = m_aSet.end(); // check again
            else
                m_aSetIter = m_aSet.begin() + row -1;
        }
        else
            m_aSetIter = m_aSet.begin() + row -1;
    }
    else
        throw SQLException();

    return m_aSetIter != m_aSet.end();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    sal_Int32 nCurPos = getRow();
    return absolute(nCurPos+rows);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStaticSet::previous(  ) throw(SQLException, RuntimeException)
{
    if(m_bBeforeFirst)
        return sal_False;
    if(m_aSetIter == m_aSet.begin())
    {
        m_aSetIter = m_aSet.end();
        m_bBeforeFirst = sal_True;
    }
    else
        --m_aSetIter;

    return sal_True;
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
        deleteRow(*(m_aSet.begin() + connectivity::getINT32(*pBegin)),_xTable);
        aRet.getArray()[i] = m_bDeleted;
    }
    return aRet;
}
// -------------------------------------------------------------------------
void SAL_CALL OStaticSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OCacheSet::insertRow( _rInsertRow,_xTable);
    if(m_bInserted)
        m_aSet.push_back(new ORowVector< ORowSetValue >(*_rInsertRow)); // we don't where the new row is so we append it to the current rows
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
        m_aSet.erase(m_aSet.begin()+(*_rDeleteRow)[0].getInt32());
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/11 11:18:11  fs
    replace unotools with comphelper

    Revision 1.1.1.1  2000/09/19 00:15:38  hr
    initial import

    Revision 1.2  2000/09/18 14:52:47  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:24:08  oj
    rowset addon

    Revision 1.0 01.08.2000 09:05:11  oj
------------------------------------------------------------------------*/

