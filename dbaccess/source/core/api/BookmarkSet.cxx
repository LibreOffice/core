/*************************************************************************
 *
 *  $RCSfile: BookmarkSet.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:58:15 $
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
#ifndef DBACCESS_CORE_API_BOOKMARKSET_HXX
#include "BookmarkSet.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#include <limits>

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

void OBookmarkSet::construct(const Reference< XResultSet>& _xDriverSet)
{
    OCacheSet::construct(_xDriverSet);
    m_xRowLocate.set(_xDriverSet,UNO_QUERY);
}
// -----------------------------------------------------------------------------
Any SAL_CALL OBookmarkSet::getBookmark( const ORowSetRow& _rRow ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->getBookmark();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->moveToBookmark(bookmark);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->moveRelativeToBookmark(bookmark,rows);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OBookmarkSet::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->compareBookmarks(first,second);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OBookmarkSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    return m_xRowLocate->hashBookmark(bookmark);
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OBookmarkSet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& _xTable) throw(SQLException, RuntimeException)
{
    Reference< ::com::sun::star::sdbcx::XDeleteRows> xDeleteRow(m_xRowLocate,UNO_QUERY);
    if(xDeleteRow.is())
    {
        return xDeleteRow->deleteRows(rows);
    }
    return Sequence< sal_Int32 >();
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_XROWUPDATE),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());

    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
    {
        xUpd->moveToInsertRow();
        sal_Int32 i = 1;
        for(connectivity::ORowVector< ORowSetValue > ::iterator aIter = _rInsertRow->begin()+1;aIter != _rInsertRow->end();++aIter,++i)
        {
            aIter->setSigned(m_aSignedFlags[i-1]);
            updateColumn(i,xUpdRow,*aIter);
        }
        xUpd->insertRow();
        (*_rInsertRow->begin()) = m_xRowLocate->getBookmark();
    }
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_XRESULTSETUPDATE),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    //  OCacheSet::updateRow( _rInsertRow,_rOrginalRow,_xTable);
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_XROWUPDATE),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());

    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::const_iterator aOrgIter = _rOrginalRow->begin()+1;
    for(connectivity::ORowVector< ORowSetValue > ::iterator aIter = _rInsertRow->begin()+1;aIter != _rInsertRow->end();++aIter,++i,++aOrgIter)
    {
        aIter->setSigned(aOrgIter->isSigned());
        updateColumn(i,xUpdRow,*aIter);
    }


    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->updateRow();
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_XRESULTSETUPDATE),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::deleteRow(const ORowSetRow& _rDeleteRow ,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);

    xUpd->deleteRow();
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->moveToInsertRow();
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void OBookmarkSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
    OCacheSet::fillValueRow(_rRow,_nPosition);
}
// -------------------------------------------------------------------------
void OBookmarkSet::updateColumn(sal_Int32 nPos,Reference< XRowUpdate > _xParameter,const ORowSetValue& _rValue)
{
    if(_rValue.isBound() && _rValue.isModified())
    {
        if(_rValue.isNull())
            _xParameter->updateNull(nPos);
        else
        {

            switch(_rValue.getTypeKind())
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    _xParameter->updateString(nPos,_rValue);
                    break;
                case DataType::BIGINT:
                    if ( _rValue.isSigned() )
                        _xParameter->updateLong(nPos,_rValue);
                    else
                        _xParameter->updateString(nPos,_rValue);
                    break;
                case DataType::BIT:
                    _xParameter->updateBoolean(nPos,_rValue);
                    break;
                case DataType::TINYINT:
                    if ( _rValue.isSigned() )
                        _xParameter->updateByte(nPos,_rValue);
                    else
                        _xParameter->updateShort(nPos,_rValue);
                    break;
                case DataType::SMALLINT:
                    if ( _rValue.isSigned() )
                        _xParameter->updateShort(nPos,_rValue);
                    else
                        _xParameter->updateInt(nPos,_rValue);
                    break;
                case DataType::INTEGER:
                    if ( _rValue.isSigned() )
                        _xParameter->updateInt(nPos,_rValue);
                    else
                        _xParameter->updateLong(nPos,_rValue);
                    break;
                case DataType::FLOAT:
                    _xParameter->updateFloat(nPos,_rValue);
                    break;
                case DataType::DOUBLE:
                case DataType::REAL:
                    _xParameter->updateDouble(nPos,_rValue);
                    break;
                case DataType::DATE:
                    _xParameter->updateDate(nPos,_rValue);
                    break;
                case DataType::TIME:
                    _xParameter->updateTime(nPos,_rValue);
                    break;
                case DataType::TIMESTAMP:
                    _xParameter->updateTimestamp(nPos,_rValue);
                    break;
                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    _xParameter->updateBytes(nPos,_rValue);
                    break;

            }
        }
    }
}

