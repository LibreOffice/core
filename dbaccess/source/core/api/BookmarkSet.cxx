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

#include "BookmarkSet.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <connectivity/dbexception.hxx>

#include <limits>

using namespace dbaccess;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::osl;

void OBookmarkSet::construct(const Reference< XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter)
{
   SAL_INFO("dbaccess", "OBookmarkSet::construct" );
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);
    m_xRowLocate.set(_xDriverSet,UNO_QUERY);
}

Any SAL_CALL OBookmarkSet::getBookmark() throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::getBookmark" );
    return m_xRowLocate->getBookmark();
}

sal_Bool SAL_CALL OBookmarkSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::moveToBookmark" );
    return m_xRowLocate->moveToBookmark(bookmark);
}

sal_Bool SAL_CALL OBookmarkSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::moveRelativeToBookmark" );
    return m_xRowLocate->moveRelativeToBookmark(bookmark,rows);
}

sal_Int32 SAL_CALL OBookmarkSet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::compareBookmarks" );
    return m_xRowLocate->compareBookmarks(_first,_second);
}

sal_Bool SAL_CALL OBookmarkSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::hasOrderedBookmarks" );
    return m_xRowLocate->hasOrderedBookmarks();
}

sal_Int32 SAL_CALL OBookmarkSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::hashBookmark" );
    return m_xRowLocate->hashBookmark(bookmark);
}

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL OBookmarkSet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& /*_xTable*/) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::deleteRows" );
    Reference< ::com::sun::star::sdbcx::XDeleteRows> xDeleteRow(m_xRowLocate,UNO_QUERY);
    if(xDeleteRow.is())
    {
        return xDeleteRow->deleteRows(rows);
    }
    return Sequence< sal_Int32 >();
}

void SAL_CALL OBookmarkSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::insertRow" );
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
   SAL_INFO("dbaccess", "OBookmarkSet::updateRow" );
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
   SAL_INFO("dbaccess", "OBookmarkSet::deleteRow" );
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);

    xUpd->deleteRow();
}

void SAL_CALL OBookmarkSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::cancelRowUpdates" );
}

void SAL_CALL OBookmarkSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::moveToInsertRow" );
    Reference<XResultSetUpdate> xUpd(m_xRowLocate,UNO_QUERY);
    if(xUpd.is())
        xUpd->moveToInsertRow();
}

void SAL_CALL OBookmarkSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
   SAL_INFO("dbaccess", "OBookmarkSet::moveToCurrentRow" );
}

void OBookmarkSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
   SAL_INFO("dbaccess", "OBookmarkSet::fillValueRow" );
    OCacheSet::fillValueRow(_rRow,_nPosition);
}

void OBookmarkSet::updateColumn(sal_Int32 nPos,Reference< XRowUpdate > _xParameter,const ORowSetValue& _rValue)
{
   SAL_INFO("dbaccess", "OBookmarkSet::updateColumn" );
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
