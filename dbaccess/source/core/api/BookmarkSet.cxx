/*************************************************************************
 *
 *  $RCSfile: BookmarkSet.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-14 13:28:20 $
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
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
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
void SAL_CALL OBookmarkSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        throw SQLException();

    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
    {
        xUpd->moveToInsertRow();
        sal_Int32 i = 1;
        for(connectivity::ORowVector< ORowSetValue > ::const_iterator aIter = _rInsertRow->begin()+1;aIter != _rInsertRow->end();++aIter,++i)
            updateColumn(i,xUpdRow,*aIter);
        xUpd->insertRow();
        (*_rInsertRow->begin()) = m_xRowLocate->getBookmark();
    }
    else
        throw SQLException();
}
// -------------------------------------------------------------------------
void SAL_CALL OBookmarkSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    //  OCacheSet::updateRow( _rInsertRow,_rOrginalRow,_xTable);
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        throw SQLException();

    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::const_iterator aOrgIter = _rOrginalRow->begin()+1;
    for(connectivity::ORowVector< ORowSetValue > ::const_iterator aIter = _rInsertRow->begin()+1;aIter != _rInsertRow->end();++aIter,++i)
    {
        updateColumn(i,xUpdRow,*aIter);
    }


    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->updateRow();
    else
        throw SQLException();
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

    (*_rRow->begin()) = m_xRowLocate->getBookmark();
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
                    _xParameter->updateString(nPos,_rValue);
                    break;
                case DataType::BIT:
                    _xParameter->updateBoolean(nPos,_rValue);
                    break;
                case DataType::TINYINT:
                    _xParameter->updateByte(nPos,_rValue);
                    break;
                case DataType::SMALLINT:
                    _xParameter->updateShort(nPos,_rValue);
                    break;
                case DataType::INTEGER:
                    _xParameter->updateInt(nPos,_rValue);
                    break;
                case DataType::DOUBLE:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
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


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2000/10/11 11:18:10  fs
    replace unotools with comphelper

    Revision 1.2  2000/09/29 15:20:51  oj
    rowset impl

    Revision 1.1.1.1  2000/09/19 00:15:37  hr
    initial import

    Revision 1.3  2000/09/18 14:52:45  willem.vandorp
    OpenOffice header added.

    Revision 1.2  2000/09/05 13:23:34  rt
    typo

    Revision 1.1  2000/09/01 15:20:46  oj
    rowset addons

    Revision 1.0 01.08.2000 09:54:37  oj
------------------------------------------------------------------------*/

