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

#include "WrappedResultSet.hxx"
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

void WrappedResultSet::construct(const Reference< XResultSet>& _xDriverSet,const OUString& i_sRowSetFilter)
{
    SAL_INFO("dbaccess", "WrappedResultSet::construct" );
    OCacheSet::construct(_xDriverSet,i_sRowSetFilter);
    m_xUpd.set(_xDriverSet,UNO_QUERY_THROW);
    m_xRowLocate.set(_xDriverSet,UNO_QUERY_THROW);
    m_xUpdRow.set(_xDriverSet,UNO_QUERY_THROW);
}

Any SAL_CALL WrappedResultSet::getBookmark() throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::getBookmark" );
    if ( m_xRowLocate.is() )
    {
        return m_xRowLocate->getBookmark( );
    }
    return makeAny(m_xDriverSet->getRow());
}

sal_Bool SAL_CALL WrappedResultSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::moveToBookmark" );
    return m_xRowLocate->moveToBookmark( bookmark );
}

sal_Bool SAL_CALL WrappedResultSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::moveRelativeToBookmark" );
    return m_xRowLocate->moveRelativeToBookmark( bookmark,rows );
}

sal_Int32 SAL_CALL WrappedResultSet::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::compareBookmarks" );
    return m_xRowLocate->compareBookmarks( _first,_second );
}

sal_Bool SAL_CALL WrappedResultSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::hasOrderedBookmarks" );
    return m_xRowLocate->hasOrderedBookmarks();
}

sal_Int32 SAL_CALL WrappedResultSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::hashBookmark" );
    return m_xRowLocate->hashBookmark(bookmark);
}

// ::com::sun::star::sdbcx::XDeleteRows
Sequence< sal_Int32 > SAL_CALL WrappedResultSet::deleteRows( const Sequence< Any >& rows ,const connectivity::OSQLTable& /*_xTable*/) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::deleteRows" );
    Reference< ::com::sun::star::sdbcx::XDeleteRows> xDeleteRow(m_xRowLocate,UNO_QUERY);
    if(xDeleteRow.is())
    {
        return xDeleteRow->deleteRows(rows);
    }
    return Sequence< sal_Int32 >();
}

void SAL_CALL WrappedResultSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& /*_xTable*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::insertRow" );

    m_xUpd->moveToInsertRow();
    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
    for(connectivity::ORowVector< ORowSetValue > ::Vector::iterator aIter = _rInsertRow->get().begin()+1;aIter != aEnd;++aIter,++i)
    {
        aIter->setSigned(m_aSignedFlags[i-1]);
        updateColumn(i,m_xUpdRow,*aIter);
    }
    m_xUpd->insertRow();
    (*_rInsertRow->get().begin()) = getBookmark();
}

void SAL_CALL WrappedResultSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::updateRow" );
    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::Vector::const_iterator aOrgIter = _rOrginalRow->get().begin()+1;
    connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
    for(connectivity::ORowVector< ORowSetValue > ::Vector::iterator aIter = _rInsertRow->get().begin()+1;aIter != aEnd;++aIter,++i,++aOrgIter)
    {
        aIter->setSigned(aOrgIter->isSigned());
        updateColumn(i,m_xUpdRow,*aIter);
    }
    m_xUpd->updateRow();
}

void SAL_CALL WrappedResultSet::deleteRow(const ORowSetRow& /*_rDeleteRow*/ ,const connectivity::OSQLTable& /*_xTable*/  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::deleteRow" );
    m_xUpd->deleteRow();
}

void SAL_CALL WrappedResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::cancelRowUpdates" );
    m_xUpd->cancelRowUpdates();
}

void SAL_CALL WrappedResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::moveToInsertRow" );
    m_xUpd->moveToInsertRow();
}

void SAL_CALL WrappedResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "WrappedResultSet::moveToCurrentRow" );
    m_xUpd->moveToCurrentRow();
}

void WrappedResultSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
    SAL_INFO("dbaccess", "WrappedResultSet::fillValueRow" );
    OCacheSet::fillValueRow(_rRow,_nPosition);
}

void WrappedResultSet::updateColumn(sal_Int32 nPos,Reference< XRowUpdate > _xParameter,const ORowSetValue& _rValue)
{
    SAL_INFO("dbaccess", "WrappedResultSet::updateColumn" );
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
