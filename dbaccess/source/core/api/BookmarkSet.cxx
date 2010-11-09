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
#include "BookmarkSet.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <connectivity/dbexception.hxx>
#include <rtl/logfile.hxx>

#include <limits>

using namespace dbaccess;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
//  using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//  using namespace ::cppu;
using namespace ::osl;

void OBookmarkSet::construct(const Reference< XResultSet>& _xDriverSet,const ::rtl::OUString& i_sRowSetFilter)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::construct" );
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);
    m_xRowLocate.set(_xDriverSet,UNO_QUERY);
}

Any SAL_CALL OBookmarkSet::getBookmark() throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::getBookmark" );
    return m_xRowLocate->getBookmark();
}

sal_Bool SAL_CALL OBookmarkSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::moveToBookmark" );
    return m_xRowLocate->moveToBookmark(bookmark);
}

sal_Bool SAL_CALL OBookmarkSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::moveRelativeToBookmark" );
    return m_xRowLocate->moveRelativeToBookmark(bookmark,rows);
}

sal_Int32 SAL_CALL OBookmarkSet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::compareBookmarks" );
    return m_xRowLocate->compareBookmarks(_first,_second);
}

sal_Bool SAL_CALL OBookmarkSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::hasOrderedBookmarks" );
    return m_xRowLocate->hasOrderedBookmarks();
}

sal_Int32 SAL_CALL OBookmarkSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::hashBookmark" );
    return m_xRowLocate->hashBookmark(bookmark);
}

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OBookmarkSet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& /*_xTable*/) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::deleteRows" );
    Reference< ::com::sun::star::sdbcx::XDeleteRows> xDeleteRow(m_xRowLocate,UNO_QUERY);
    if(xDeleteRow.is())
    {
        return xDeleteRow->deleteRows(rows);
    }
    return Sequence< sal_Int32 >();
}

void SAL_CALL OBookmarkSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::insertRow" );
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_XROWUPDATE ), SQL_GENERAL_ERROR, *this );

    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
    {
        xUpd->moveToInsertRow();
        sal_Int32 i = 1;
        connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
        for(connectivity::ORowVector< ORowSetValue > ::Vector::iterator aIter = _rInsertRow->get().begin()+1;aIter != aEnd;++aIter,++i)
        {
            aIter->setSigned(m_aSignedFlags[i-1]);
            updateColumn(i,xUpdRow,*aIter);
        }
        xUpd->insertRow();
        (*_rInsertRow->get().begin()) = m_xRowLocate->getBookmark();
    }
    else
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_XRESULTSETUPDATE ), SQL_GENERAL_ERROR, *this );
}

void SAL_CALL OBookmarkSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::updateRow" );
    Reference<XRowUpdate> xUpdRow(m_xRowLocate,UNO_QUERY);
    if(!xUpdRow.is())
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_XROWUPDATE ), SQL_GENERAL_ERROR, *this );

    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::Vector::const_iterator aOrgIter = _rOrginalRow->get().begin()+1;
    connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
    for(connectivity::ORowVector< ORowSetValue > ::Vector::iterator aIter = _rInsertRow->get().begin()+1;aIter != aEnd;++aIter,++i,++aOrgIter)
    {
        aIter->setSigned(aOrgIter->isSigned());
        updateColumn(i,xUpdRow,*aIter);
    }


    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->updateRow();
    else
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_XRESULTSETUPDATE ), SQL_GENERAL_ERROR, *this );
}

void SAL_CALL OBookmarkSet::deleteRow(const ORowSetRow& /*_rDeleteRow*/ ,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::deleteRow" );
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);

    xUpd->deleteRow();
}

void SAL_CALL OBookmarkSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::cancelRowUpdates" );
}

void SAL_CALL OBookmarkSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::moveToInsertRow" );
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->moveToInsertRow();
}

void SAL_CALL OBookmarkSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::moveToCurrentRow" );
}

void OBookmarkSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::fillValueRow" );
    OCacheSet::fillValueRow(_rRow,_nPosition);
}

void OBookmarkSet::updateColumn(sal_Int32 nPos,Reference< XRowUpdate > _xParameter,const ORowSetValue& _rValue)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OBookmarkSet::updateColumn" );
    if(_rValue.isBound() && _rValue.isModified())
    {
        if(_rValue.isNull())
            _xParameter->updateNull(nPos);
        else
        {

            switch(_rValue.getTypeKind())
            {
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    _xParameter->updateNumericObject(nPos,_rValue.makeAny(),m_xSetMetaData->getScale(nPos));
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                //case DataType::DECIMAL:
                //case DataType::NUMERIC:
                    _xParameter->updateString(nPos,_rValue);
                    break;
                case DataType::BIGINT:
                    if ( _rValue.isSigned() )
                        _xParameter->updateLong(nPos,_rValue);
                    else
                        _xParameter->updateString(nPos,_rValue);
                    break;
                case DataType::BIT:
                case DataType::BOOLEAN:
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
                case DataType::BLOB:
                case DataType::CLOB:
                    _xParameter->updateObject(nPos,_rValue.getAny());
                    break;
            }
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
