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


#include "RowSetBase.hxx"
#include "CRowSetDataColumn.hxx"
#include <connectivity/sdbcx/VCollection.hxx>
#include "RowSetCache.hxx"
#include "dbastrings.hrc"
#include "core_resource.hrc"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/seqstream.hxx>
#include <connectivity/dbexception.hxx>
#include <tools/debug.hxx>

using namespace dbaccess;
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace comphelper;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::cppu;
using namespace ::osl;

namespace dbaccess
{

// =========================================================================
// = OEmptyCollection
// =========================================================================
class OEmptyCollection : public sdbcx::OCollection
{
protected:
    virtual void impl_refresh() throw(RuntimeException);
    virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName);
public:
    OEmptyCollection(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) : OCollection(_rParent,sal_True,_rMutex,::std::vector< OUString>()){}
};

void OEmptyCollection::impl_refresh() throw(RuntimeException)
{
}

connectivity::sdbcx::ObjectType OEmptyCollection::createObject(const OUString& /*_rName*/)
{
    return connectivity::sdbcx::ObjectType();
}

// =========================================================================
// = ORowSetBase
// =========================================================================
DBG_NAME(ORowSetBase)

ORowSetBase::ORowSetBase( const Reference<XComponentContext>& _rContext, ::cppu::OBroadcastHelper& _rBHelper, ::osl::Mutex* _pMutex )
    :OPropertyStateContainer(_rBHelper)
    ,m_pMutex(_pMutex)
    ,m_pCache(NULL)
    ,m_pColumns(NULL)
    ,m_rBHelper(_rBHelper)
    ,m_pEmptyCollection( NULL )
    ,m_aContext( _rContext )
    ,m_aErrors( _rContext )
    ,m_nLastColumnIndex(-1)
    ,m_nDeletedPosition(-1)
    ,m_nResultSetType( ResultSetType::FORWARD_ONLY )
    ,m_nResultSetConcurrency( ResultSetConcurrency::READ_ONLY )
    ,m_bClone(sal_False)
    ,m_bIgnoreResult(sal_False)
    ,m_bBeforeFirst(sal_True) // changed from sal_False
    ,m_bAfterLast(sal_False)
    ,m_bIsInsertRow(sal_False)
{
    SAL_INFO("dbaccess", "ORowSetBase::ORowSetBase" );
    DBG_CTOR(ORowSetBase,NULL);

    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    sal_Int32 nInitialRowCountValue = 0;
    sal_Bool bInitialRowCountFinalValue( sal_False );
    registerPropertyNoMember( PROPERTY_ROWCOUNT,        PROPERTY_ID_ROWCOUNT,        nRBT, ::getCppuType( &nInitialRowCountValue ), &nInitialRowCountValue );
    registerPropertyNoMember( PROPERTY_ISROWCOUNTFINAL, PROPERTY_ID_ISROWCOUNTFINAL, nRBT, ::getBooleanCppuType(),                  &bInitialRowCountFinalValue );
}

ORowSetBase::~ORowSetBase()
{
    if(m_pColumns)
    {
        TDataColumns().swap(m_aDataColumns);
        m_pColumns->acquire();
        m_pColumns->disposing();
        delete m_pColumns;
        m_pColumns = NULL;
    }

    if ( m_pEmptyCollection )
        delete m_pEmptyCollection;

    DBG_DTOR(ORowSetBase,NULL);
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > ORowSetBase::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(ORowSetBase_BASE::getTypes(),OPropertyStateContainer::getTypes());
}

// com::sun::star::uno::XInterface
Any ORowSetBase::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aRet = ORowSetBase_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertyStateContainer::queryInterface(rType);
    return aRet;
}

void SAL_CALL ORowSetBase::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    //SAL_INFO("dbaccess", "ORowSetBase::getFastPropertyValue" );
    if(m_pCache)
    {
        switch(nHandle)
        {
        case PROPERTY_ID_ROWCOUNT:
            rValue <<= impl_getRowCount();
            break;
        case PROPERTY_ID_ISROWCOUNTFINAL:
            rValue.setValue(&m_pCache->m_bRowCountFinal,::getCppuBooleanType());
            break;
        default:
            OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
        };
    }
    else
        OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
}

// OComponentHelper
void SAL_CALL ORowSetBase::disposing(void)
{
    SAL_INFO("dbaccess", "ORowSetBase::disposing" );
    MutexGuard aGuard(*m_pMutex);

    if ( m_pColumns )
    {
        TDataColumns().swap(m_aDataColumns);
        m_pColumns->disposing();
    }
    if ( m_pCache )
    {
        m_pCache->deregisterOldRow(m_aOldRow);
        m_pCache->deleteIterator(this);
    }
    m_pCache = NULL;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetBase::createArrayHelper( ) const
{
    SAL_INFO("dbaccess", "ORowSetBase::createArrayHelper" );
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSetBase::getInfoHelper()
{
    SAL_INFO("dbaccess", "ORowSetBase::getInfoHelper" );
    return *const_cast<ORowSetBase*>(this)->getArrayHelper();
}

// XRow
sal_Bool SAL_CALL ORowSetBase::wasNull(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::wasNull" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return impl_wasNull();
}

sal_Bool ORowSetBase::impl_wasNull()
{
    SAL_INFO("dbaccess", "ORowSetBase::impl_wasNull" );
    return ((m_nLastColumnIndex != -1) && !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is()) ? ((*m_aCurrentRow)->get())[m_nLastColumnIndex].isNull() : sal_True;
}

const ORowSetValue& ORowSetBase::getValue(sal_Int32 columnIndex)
{
    SAL_INFO("dbaccess", "ORowSetBase::getValue" );
    checkCache();
    return impl_getValue(columnIndex);
}

const ORowSetValue& ORowSetBase::impl_getValue(sal_Int32 columnIndex)
{
    SAL_INFO("dbaccess", "ORowSetBase::impl_getValue" );
    if ( m_bBeforeFirst || m_bAfterLast )
    {
        SAL_WARN("dbaccess", "ORowSetBase::getValue: Illegal call here (we're before first or after last)!");
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_CURSOR_BEFORE_OR_AFTER ), SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
    }

    if ( impl_rowDeleted() )
    {
        return m_aEmptyValue;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    if ( !bValidCurrentRow )
    {
        // currentrow is null when the clone moves the window
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = sal_False;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    }

    if ( bValidCurrentRow )
    {
#if OSL_DEBUG_LEVEL > 0
        ORowSetMatrix::iterator aCacheEnd;
        ORowSetMatrix::iterator aCurrentRow;
        aCacheEnd = m_pCache->getEnd();
        aCurrentRow = m_aCurrentRow;
        ORowSetCacheMap::iterator aCacheIter = m_aCurrentRow.getIter();
        ORowSetCacheIterator_Helper aHelper = aCacheIter->second;
        ORowSetMatrix::iterator k = aHelper.aIterator;
        for (; k != m_pCache->getEnd(); ++k)
        {
            ORowSetValueVector* pTemp = k->get();
            OSL_ENSURE( pTemp != (void*)0xfeeefeee,"HALT!" );
        }
#endif
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow < m_pCache->getEnd() && aCacheIter != m_pCache->m_aCacheIterators.end(),"Invalid iterator set for currentrow!");
#if OSL_DEBUG_LEVEL > 0
        ORowSetRow rRow = (*m_aCurrentRow);
        OSL_ENSURE(rRow.is() && static_cast<sal_uInt16>(columnIndex) < (rRow->get()).size(),"Invalid size of vector!");
#endif
        return ((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex];
    }

    // we should normally never reach this
    return m_aEmptyValue;
}

OUString SAL_CALL ORowSetBase::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getString" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Bool SAL_CALL ORowSetBase::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getBoolean" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int8 SAL_CALL ORowSetBase::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getByte" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int16 SAL_CALL ORowSetBase::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getShort" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int32 SAL_CALL ORowSetBase::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getInt" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int64 SAL_CALL ORowSetBase::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getLong" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

float SAL_CALL ORowSetBase::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getFloat" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

double SAL_CALL ORowSetBase::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getDouble" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL ORowSetBase::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getBytes" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

::com::sun::star::util::Date SAL_CALL ORowSetBase::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getDate" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

::com::sun::star::util::Time SAL_CALL ORowSetBase::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getTime" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

::com::sun::star::util::DateTime SAL_CALL ORowSetBase::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getTimestamp" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getBinaryStream" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
    {
        SAL_WARN("dbaccess", "ORowSetBase::getBinaryStream: Illegal call here (we're before first or after last)!");
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_CURSOR_BEFORE_OR_AFTER ), SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
    }

    if ( impl_rowDeleted() )
    {
        return NULL;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    if ( !bValidCurrentRow )
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = sal_False;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getBinaryStream: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    }

    if ( bValidCurrentRow )
        return new ::comphelper::SequenceInputStream(((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());

    // we should normally never reach this
    return Reference< ::com::sun::star::io::XInputStream >();
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getCharacterStream" );
    return getBinaryStream(columnIndex);
}

Any SAL_CALL ORowSetBase::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getObject" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return getValue(columnIndex).makeAny();
}

Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getRef" );
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRef", *m_pMySelf );
    return NULL;
}

Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getBlob" );
    return Reference< XBlob >(getValue(columnIndex).makeAny(),UNO_QUERY);
}

Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getClob" );
    return Reference< XClob >(getValue(columnIndex).makeAny(),UNO_QUERY);
}

Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getArray" );
    ::dbtools::throwFeatureNotImplementedException( "XRow::getArray", *m_pMySelf );
    return NULL;
}

// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getBookmark" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::getBookmark() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_BOOKMARK_BEFORE_OR_AFTER ), SQL_INVALID_CURSOR_POSITION, *m_pMySelf );

    if ( impl_rowDeleted() )
        ::dbtools::throwSQLException( DBACORE_RESSTRING( RID_STR_NO_BOOKMARK_DELETED ), SQL_INVALID_CURSOR_POSITION, *m_pMySelf );

    OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::getBookmark: bookmark has no value!" );
    return m_aBookmark;
}

sal_Bool SAL_CALL ORowSetBase::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::moveToBookmark" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::moveToBookmark(Any) Clone = " << m_bClone);
    OSL_ENSURE(bookmark.hasValue(),"ORowSetBase::moveToBookmark bookmark has no value!");
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!bookmark.hasValue() || m_nResultSetType == ResultSetType::FORWARD_ONLY)
    {
        if(bookmark.hasValue())
            SAL_WARN("dbaccess", "MoveToBookmark is not possible when we are only forward");
        else
            SAL_WARN("dbaccess", "Bookmark is not valid");
        throwFunctionSequenceException(*m_pMySelf);
    }


    checkCache();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->moveToBookmark(bookmark);
        doCancelModification( );
        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );
        }
        else
        {
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire( );
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::moveToBookmark(Any) = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::moveRelativeToBookmark" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::moveRelativeToBookmark(Any," << rows << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->moveRelativeToBookmark(bookmark,rows);
        doCancelModification( );
        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );
        }
        else
            movementFailed();

        // - IsModified
        // - IsNew
        aNotifier.fire( );

        // RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::moveRelativeToBookmark(Any," << rows << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::compareBookmarks" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->compareBookmarks(_first,_second);
}

sal_Bool SAL_CALL ORowSetBase::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::hasOrderedBookmarks" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hasOrderedBookmarks();
}

sal_Int32 SAL_CALL ORowSetBase::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::hashBookmark" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hashBookmark(bookmark);
}

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > SAL_CALL ORowSetBase::getMetaData(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getMetaData" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    Reference< XResultSetMetaData > xMeta;
    if(m_pCache)
        xMeta = m_pCache->getMetaData();

    return xMeta;
}

// XColumnLocate
sal_Int32 SAL_CALL ORowSetBase::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::findColumn" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    // it is possible to save some time here when we remember the names - position relation in a map
    return m_pColumns ? m_pColumns->findColumn(columnName) : sal_Int32(0);
}

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< XNameAccess > SAL_CALL ORowSetBase::getColumns(  ) throw(RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getColumns" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(!m_pColumns)
    {
        if (!m_pEmptyCollection)
            m_pEmptyCollection = new OEmptyCollection(*m_pMySelf,m_aColumnsMutex);
        return m_pEmptyCollection;
    }

    return m_pColumns;
}

// XResultSet
sal_Bool SAL_CALL ORowSetBase::next(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::next" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::next() Clone = " << m_bClone);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        positionCache( MOVE_FORWARD );
        sal_Bool bAfterLast = m_pCache->isAfterLast();
        bRet = m_pCache->next();
        doCancelModification( );

        if ( bRet || bAfterLast != m_pCache->isAfterLast() )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( bRet, sal_True, aOldValues, aGuard );
            OSL_ENSURE(!m_bBeforeFirst,"BeforeFirst is true. I don't know why?");
        }
        else
        {
            // moved after the last row
            movementFailed();
            OSL_ENSURE(m_bAfterLast,"AfterLast is false. I don't know why?");
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::next() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::isBeforeFirst" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isBeforeFirst() = " << m_bBeforeFirst << " Clone = " << m_bClone);

    return m_bBeforeFirst;
}

sal_Bool SAL_CALL ORowSetBase::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::isAfterLast" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isAfterLast() = " << m_bAfterLast << " Clone = " << m_bClone);

    return m_bAfterLast;
}

sal_Bool ORowSetBase::isOnFirst()
{
    SAL_INFO("dbaccess", "ORowSetBase::isOnFirst" );
    return isFirst();
}

sal_Bool SAL_CALL ORowSetBase::isFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::isFirst" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isFirst() Clone = " << m_bClone);

    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return sal_False;

    if ( impl_rowDeleted() )
        return ( m_nDeletedPosition == 1 );

    sal_Bool bIsFirst = m_pCache->isFirst();

    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isFirst() = " << bIsFirst << " Clone = " << m_bClone);
    return bIsFirst;
}

sal_Bool ORowSetBase::isOnLast()
{
    SAL_INFO("dbaccess", "ORowSetBase::isOnLast" );
    return isLast();
}

sal_Bool SAL_CALL ORowSetBase::isLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::isLast" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isLast() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return sal_False;

    if ( impl_rowDeleted() )
    {
        if ( !m_pCache->m_bRowCountFinal )
            return sal_False;
        else
            return ( m_nDeletedPosition == impl_getRowCount() );
    }

    sal_Bool bIsLast = m_pCache->isLast();

    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::isLast() = " << bIsLast << " Clone = " << m_bClone);
    return bIsLast;
}

void SAL_CALL ORowSetBase::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::beforeFirst" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::beforeFirst() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

    if((bWasNew || !m_bBeforeFirst) && notifyAllListenersCursorBeforeMove(aGuard) )
    {
        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        if ( !m_bBeforeFirst )
        {
            ORowSetRow aOldValues = getOldRow(bWasNew);
            m_pCache->beforeFirst();
            doCancelModification( );

            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );

            // - IsModified
            // - Isnew
            aNotifier.fire();

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }

        // to be done _after_ the notifications!
        m_aOldRow->clearRow();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::beforeFirst() Clone = " << m_bClone);
}

void SAL_CALL ORowSetBase::afterLast(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::afterLast" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::afterLast() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

    if((bWasNew || !m_bAfterLast) && notifyAllListenersCursorBeforeMove(aGuard) )
    {
        // check if we are inserting a row
        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        if(!m_bAfterLast)
        {
            ORowSetRow aOldValues = getOldRow(bWasNew);

            m_pCache->afterLast();
            doCancelModification( );

            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );

            // - IsModified
            // - Isnew
            aNotifier.fire();

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::afterLast() Clone = " << m_bClone);
}

sal_Bool SAL_CALL ORowSetBase::move(    ::std::mem_fun_t<sal_Bool,ORowSetBase>& _aCheckFunctor,
                                        ::std::mem_fun_t<sal_Bool,ORowSetCache>& _aMovementFunctor)
{
    SAL_INFO("dbaccess", "ORowSetBase::move" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::move() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        sal_Bool bMoved = ( bWasNew || !_aCheckFunctor(this) );

        bRet = _aMovementFunctor(m_pCache);
        doCancelModification( );

        if ( bRet )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( bMoved, sal_True, aOldValues, aGuard );
        }
        else
        {   // first goes wrong so there is no row
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::move() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::first(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::first" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::first() Clone = " << m_bClone);
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioF_tmp(&ORowSetBase::isOnFirst);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> F_tmp(&ORowSetCache::first);
    return move(ioF_tmp,F_tmp);
}

sal_Bool SAL_CALL ORowSetBase::last(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::last" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::last() Clone = " << m_bClone);
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioL_tmp(&ORowSetBase::isOnLast);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> L_tmp(&ORowSetCache::last);
    return move(ioL_tmp,L_tmp);
}

sal_Int32 SAL_CALL ORowSetBase::getRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getRow" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::getRow() Clone = " << m_bClone);
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();
    return impl_getRow();
}

sal_Int32 ORowSetBase::impl_getRow()
{
    SAL_INFO("dbaccess", "ORowSetBase::impl_getRow" );
    sal_Int32  nPos = 0;
    if ( m_bBeforeFirst )
        nPos = 0;
    else if ( m_bAfterLast )
        nPos = impl_getRowCount() + 1;
    else if ( impl_rowDeleted() )
        nPos = m_nDeletedPosition;
    else if ( !m_bClone && m_pCache->m_bNew )
        nPos = 0;
    else
    {
        if  (   m_pCache->isAfterLast()
            ||  m_pCache->isBeforeFirst()
            ||  ( m_pCache->compareBookmarks( m_aBookmark, m_pCache->getBookmark() ) != CompareBookmark::EQUAL )
            )
        {
            positionCache( MOVE_NONE_REFRESH_ONLY );
        }
        nPos = m_pCache->getRow();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::impl_getRow() = " << nPos << " Clone = " << m_bClone);
    return nPos;
}

sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::absolute" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::absolute(" << row << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bRet = ( row > 0 )
                &&  notifyAllListenersCursorBeforeMove( aGuard );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->absolute(row);
        doCancelModification( );

        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );
        }
        else
        { // absolute movement goes wrong we stand left or right side of the rows
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::absolute(" << row << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::relative" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::relative(" << rows << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!rows)
        return sal_True; // in this case do nothing

    checkPositioningAllowed();

    sal_Bool bRet =
            (  ( !m_bAfterLast || rows <= 0 )
            && ( !m_bBeforeFirst || rows >= 0 )
            && notifyAllListenersCursorBeforeMove( aGuard )
            );

    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        positionCache( rows > 0 ? MOVE_FORWARD : MOVE_BACKWARD );
        bRet = m_pCache->relative(rows);
        doCancelModification( );

        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );
        }
        else
        {
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::relative(" << rows << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::previous(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::previous" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::previous() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    sal_Bool bRet = !m_bBeforeFirst
                &&  notifyAllListenersCursorBeforeMove(aGuard);

    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        positionCache( MOVE_BACKWARD );
        bRet = m_pCache->previous();
        doCancelModification( );

        // if m_bBeforeFirst is false and bRet is false than we stood on the first row
        if(!m_bBeforeFirst || bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( sal_True, sal_True, aOldValues, aGuard );
        }
        else
        {
            SAL_WARN("dbaccess", "ORowSetBase::previous: inconsistency!" );
                // we should never reach this place, as we should not get into this whole branch if m_bBeforeFirst
                // was |true| from the beginning
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // - RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::previous() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

void ORowSetBase::setCurrentRow( sal_Bool _bMoved, sal_Bool _bDoNotify, const ORowSetRow& _rOldValues, ::osl::ResettableMutexGuard& _rGuard )
{
    SAL_INFO("dbaccess", "ORowSetBase::setCurrentRow" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::setCurrentRow() Clone = " << m_bClone);
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        m_aBookmark     = m_pCache->getBookmark();
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = sal_False;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is null!");
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
        OSL_ENSURE(m_aCurrentRow->is(),"Currentrow isn't valid");
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");

        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = sal_False;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is nul after positionCache!");
#if OSL_DEBUG_LEVEL > 0
        ORowSetRow rRow = (*m_aCurrentRow);
        OSL_ENSURE(rRow.is() ,"Invalid size of vector!");
#endif
        // the cache could repositioned so we need to adjust the cache
        if ( _bMoved && m_aCurrentRow.isNull() )
        {
            positionCache( MOVE_NONE_REFRESH_ONLY );
            m_aCurrentRow   = m_pCache->m_aMatrixIter;
            m_bIsInsertRow  = sal_False;
            OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is nul after positionCache!");
        }
    }
    else
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
        m_aCurrentRow.setBookmark(m_aBookmark);
    }

    // notification order
    // - column values
    if ( _bDoNotify )
        firePropertyChange(_rOldValues);

    // TODO: can this be done before the notifications?
    if(!(m_bBeforeFirst || m_bAfterLast) && !m_aCurrentRow.isNull() && m_aCurrentRow->is() && m_aCurrentRow != m_pCache->getEnd())
        m_aOldRow->setRow(new ORowSetValueVector( *(*m_aCurrentRow) ));

    if ( _bMoved && _bDoNotify )
        // - cursorMoved
        notifyAllListenersCursorMoved( _rGuard );

    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::setCurrentRow() Clone = " << m_bClone);
}

void ORowSetBase::checkPositioningAllowed() throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "ORowSetBase::checkPositioningAllowed" );
    if(!m_pCache || m_nResultSetType == ResultSetType::FORWARD_ONLY)
        throwFunctionSequenceException(*m_pMySelf);
}

Reference< XInterface >  ORowSetBase::getStatement(void) throw( SQLException, RuntimeException )
{
    SAL_INFO("dbaccess", "ORowSetBase::getStatement" );
    return NULL;
}

void SAL_CALL ORowSetBase::refreshRow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::refreshRow" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    if ( impl_rowDeleted() )
        throwSQLException( "The current row is deleted", SQL_INVALID_CURSOR_STATE, Reference< XRowSet >( this ) );

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        sal_Bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();
        ORowSetRow aOldValues = getOldRow(bWasNew);
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_pCache->refreshRow();
        firePropertyChange(aOldValues);
    }
}

sal_Bool SAL_CALL ORowSetBase::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::rowUpdated" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( impl_rowDeleted() )
        return sal_False;

    return m_pCache->rowUpdated();
}

sal_Bool SAL_CALL ORowSetBase::rowInserted(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::rowInserted" );
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();

    if ( impl_rowDeleted() )
        return sal_False;

    return m_pCache->rowInserted();
}

sal_Bool SAL_CALL ORowSetBase::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::rowDeleted" );
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return impl_rowDeleted();
}

sal_Bool ORowSetBase::impl_rowDeleted(  )
{
    return !m_aBookmark.hasValue() && !m_bBeforeFirst && !m_bAfterLast;
}

// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::getWarnings" );
    ::osl::MutexGuard aGuard( *m_pMutex );

    if ( m_pCache )
    {
        Reference< XWarningsSupplier > xWarnings( m_pCache->m_xSet.get(), UNO_QUERY );
        if ( xWarnings.is() )
            return xWarnings->getWarnings();
    }

    return Any();
}

void SAL_CALL ORowSetBase::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("dbaccess", "ORowSetBase::clearWarnings" );
    ::osl::MutexGuard aGuard( *m_pMutex );

    if ( m_pCache )
    {
        Reference< XWarningsSupplier > xWarnings( m_pCache->m_xSet.get(), UNO_QUERY );
        if ( xWarnings.is() )
            xWarnings->clearWarnings();
    }
}

void ORowSetBase::firePropertyChange(const ORowSetRow& _rOldRow)
{
    if (!isPropertyChangeNotificationEnabled())
        return;

    SAL_INFO("dbaccess", "ORowSetBase::firePropertyChange" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::firePropertyChange() Clone = " << m_bClone);
    OSL_ENSURE(m_pColumns,"Columns can not be NULL here!");
    sal_Int32 i=0;
    try
    {
        TDataColumns::iterator aEnd = m_aDataColumns.end();
        for(TDataColumns::iterator aIter = m_aDataColumns.begin();aIter != aEnd;++aIter,++i)
            (*aIter)->fireValueChange(_rOldRow.is() ? (_rOldRow->get())[i+1] : ::connectivity::ORowSetValue());
    }
    catch (const Exception&)
    {
        SAL_WARN("dbaccess", "firePropertyChange: Exception");
    }
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::firePropertyChange() Clone = " << m_bClone);
}

void ORowSetBase::firePropertyChange(sal_Int32 _nPos,const ::connectivity::ORowSetValue& _rOldValue)
{
    OSL_ENSURE(_nPos < (sal_Int32)m_aDataColumns.size(),"nPos is invalid!");
    m_aDataColumns[_nPos]->fireValueChange(_rOldValue);
}

void ORowSetBase::fireRowcount()
{
    SAL_INFO("dbaccess", "ORowSetBase::fireRowcount" );
}

sal_Bool ORowSetBase::notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& /*_rGuard*/)
{
    SAL_INFO("dbaccess", "ORowSetBase::notifyAllListenersCursorBeforeMove" );
    return sal_True;
}

void ORowSetBase::notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& /*_rGuard*/)
{
    SAL_INFO("dbaccess", "ORowSetBase::notifyAllListenersCursorMoved" );
}

void ORowSetBase::notifyAllListeners(::osl::ResettableMutexGuard& /*_rGuard*/)
{
    SAL_INFO("dbaccess", "ORowSetBase::notifyAllListeners" );
}

sal_Bool ORowSetBase::isPropertyChangeNotificationEnabled() const
{
    return sal_True;
}

void ORowSetBase::fireProperty( sal_Int32 _nProperty, sal_Bool _bNew, sal_Bool _bOld )
{
    SAL_INFO("dbaccess", "ORowSetBase::fireProperty" );
    Any aNew = bool2any( _bNew );
    Any aOld = bool2any( _bOld );
    fire( &_nProperty, &aNew, &aOld, 1, sal_False );
}

void ORowSetBase::positionCache( CursorMoveDirection _ePrepareForDirection )
{
    SAL_INFO("dbaccess", "ORowSetBase::positionCache" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::positionCache() Clone = " << m_bClone);

    sal_Bool bSuccess = sal_False;
    if ( m_aBookmark.hasValue() )
    {
        if ( _ePrepareForDirection == MOVE_NONE_REFRESH_ONLY ||
             m_pCache->compareBookmarks( m_aBookmark, m_pCache->getBookmark() ) != CompareBookmark::EQUAL )
            bSuccess = m_pCache->moveToBookmark( m_aBookmark );
        else
            bSuccess = sal_True;
    }
    else
    {
        if ( m_bBeforeFirst )
        {
            bSuccess = m_pCache->beforeFirst();
        }
        else if ( m_bAfterLast )
        {
            bSuccess = m_pCache->afterLast();
        }
        else
        {
            OSL_ENSURE( m_nDeletedPosition >= 1, "ORowSetBase::positionCache: no bookmark, and no valid 'deleted position'!" );
            switch ( _ePrepareForDirection )
            {
            case MOVE_FORWARD:
                if ( m_nDeletedPosition > 1 )
                    bSuccess = m_pCache->absolute( m_nDeletedPosition - 1 );
                else
                {
                    m_pCache->beforeFirst();
                    bSuccess = sal_True;
                }
                break;

            case MOVE_BACKWARD:
                if ( m_pCache->m_bRowCountFinal && ( m_nDeletedPosition == impl_getRowCount() ) )
                {
                    m_pCache->afterLast();
                    bSuccess = sal_True;
                }
                else
                    bSuccess = m_pCache->absolute( m_nDeletedPosition );
                break;

            case MOVE_NONE_REFRESH_ONLY:
                bSuccess = sal_False;   // will be asserted below
                break;
            }
        }
    }
    OSL_ENSURE( bSuccess, "ORowSetBase::positionCache: failed!" );
    (void)bSuccess;

    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::positionCache() Clone = " << m_bClone);
}

void ORowSetBase::checkCache()
{
    SAL_INFO("dbaccess", "ORowSetBase::checkCache" );
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    if(!m_pCache)
        throwFunctionSequenceException(*m_pMySelf);
}

void ORowSetBase::movementFailed()
{
    SAL_INFO("dbaccess", "ORowSetBase::movementFailed" );
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::movementFailed() Clone = " << m_bClone);
    m_aOldRow->clearRow();
    m_aCurrentRow   = m_pCache->getEnd();
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    m_aBookmark     = Any();
    m_aCurrentRow.setBookmark(m_aBookmark);
    OSL_ENSURE(m_bBeforeFirst || m_bAfterLast,"BeforeFirst or AfterLast is wrong!");
    SAL_INFO("dbaccess", "DBACCESS ORowSetBase::movementFailed() Clone = " << m_bClone);
}

ORowSetRow ORowSetBase::getOldRow(sal_Bool _bWasNew)
{
    SAL_INFO("dbaccess", "ORowSetBase::getOldRow" );
    OSL_ENSURE(m_aOldRow.is(),"RowSetRowHElper isn't valid!");
    ORowSetRow aOldValues;
    if ( !_bWasNew && m_aOldRow->getRow().is() )
        aOldValues = new ORowSetValueVector( *(m_aOldRow->getRow()));    // remember the old values
    return aOldValues;
}

void ORowSetBase::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    SAL_INFO("dbaccess", "ORowSetBase::getPropertyDefaultByHandle" );
    _rDefault.clear();
}

void ORowSetBase::onDeleteRow( const Any& _rBookmark )
{
    SAL_INFO("dbaccess", "ORowSetBase::onDeleteRow" );
    if ( rowDeleted() )
        // not interested in
        return;

    ::osl::MutexGuard aGuard( *m_pMutex );
    //OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::onDeleteRow: Bookmark isn't valid!" );
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == 0 )
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_nDeletedPosition = m_pCache->getRow();
    }
}

void ORowSetBase::onDeletedRow( const Any& _rBookmark, sal_Int32 _nPos )
{
    SAL_INFO("dbaccess", "ORowSetBase::onDeletedRow" );
    if ( rowDeleted() )
    {
        // if we're a clone, and on a deleted row, and the main RowSet deleted another
        // row (only the main RowSet can, clones can't), which is *before* our
        // deleted position, then we have to adjust this position
        if ( m_bClone && ( _nPos < m_nDeletedPosition ) )
            --m_nDeletedPosition;
        return;
    }

    ::osl::MutexGuard aGuard( *m_pMutex );
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == 0 )
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
        m_aCurrentRow.setBookmark( m_aBookmark );
    }
}

sal_Int32 ORowSetBase::impl_getRowCount() const
{
    SAL_INFO("dbaccess", "ORowSetBase::impl_getRowCount" );
    sal_Int32 nRowCount( m_pCache->m_nRowCount );
    if ( const_cast< ORowSetBase* >( this )->rowDeleted() && !m_pCache->m_bNew )
        ++nRowCount;
    return nRowCount;
}
// =============================================================================
struct ORowSetNotifierImpl
{
    ::std::vector<sal_Int32>    aChangedColumns;
    ::std::vector<Any>          aChangedBookmarks;
    ORowSetValueVector::Vector  aRow;

};
DBG_NAME(ORowSetNotifier)

ORowSetNotifier::ORowSetNotifier( ORowSetBase* _pRowSet )
    :m_pRowSet( _pRowSet )
    ,m_bWasNew( sal_False )
    ,m_bWasModified( sal_False )
#ifdef DBG_UTIL
    ,m_bNotifyCalled( sal_False )
#endif
{
    DBG_CTOR(ORowSetNotifier,NULL);

    OSL_ENSURE( m_pRowSet, "ORowSetNotifier::ORowSetNotifier: invalid row set. This wil crash." );

    // remember the "inserted" and "modified" state for later firing
    m_bWasNew       = m_pRowSet->isNew( ORowSetBase::GrantNotifierAccess() );
    m_bWasModified  = m_pRowSet->isModified( ORowSetBase::GrantNotifierAccess() );

    // if the row set is on the insert row, then we need to cancel this
    if ( m_pRowSet->isModification( ORowSetBase::GrantNotifierAccess() ) )
        m_pRowSet->doCancelModification( ORowSetBase::GrantNotifierAccess() );
}

ORowSetNotifier::ORowSetNotifier( ORowSetBase* _pRowSet,const ORowSetValueVector::Vector& i_aRow )
    :m_pImpl(new ORowSetNotifierImpl)
    ,m_pRowSet( _pRowSet )
    ,m_bWasNew( sal_False )
    ,m_bWasModified( sal_False )
#ifdef DBG_UTIL
    ,m_bNotifyCalled( sal_False )
#endif
{
    DBG_CTOR(ORowSetNotifier,NULL);

    OSL_ENSURE( m_pRowSet, "ORowSetNotifier::ORowSetNotifier: invalid row set. This wil crash." );
    m_pImpl->aRow = i_aRow; // yes, create a copy to store the old values
}

ORowSetNotifier::~ORowSetNotifier( )
{
    SAL_INFO("dbaccess", "ORowSetNotifier::~ORowSetNotifier" );
    DBG_DTOR(ORowSetNotifier,NULL);
}

void ORowSetNotifier::fire()
{
    // we're not interested in firing changes FALSE->TRUE, only TRUE->FALSE.
    // (the former would be quite pathological, e.g. after a failed movement)

    if  (   m_bWasModified
        &&  ( m_bWasModified != m_pRowSet->isModified( ORowSetBase::GrantNotifierAccess() ) )
        )
        m_pRowSet->fireProperty( PROPERTY_ID_ISMODIFIED, sal_False, sal_True, ORowSetBase::GrantNotifierAccess() );

    if  (   m_bWasNew
        &&  ( m_bWasNew != m_pRowSet->isNew( ORowSetBase::GrantNotifierAccess() ) )
        )
        m_pRowSet->fireProperty( PROPERTY_ID_ISNEW, sal_False, sal_True, ORowSetBase::GrantNotifierAccess() );

#ifdef DBG_UTIL
    m_bNotifyCalled = sal_True;
#endif
}

::std::vector<sal_Int32>& ORowSetNotifier::getChangedColumns() const
{
    OSL_ENSURE(m_pImpl.get(),"Illegal CTor call, use the other one!");
    return m_pImpl->aChangedColumns;
}

void ORowSetNotifier::firePropertyChange()
{
    OSL_ENSURE(m_pImpl.get(),"Illegal CTor call, use the other one!");
    if( m_pImpl.get() )
    {
        ::std::vector<sal_Int32>::iterator aIter = m_pImpl->aChangedColumns.begin();
        for(;aIter != m_pImpl->aChangedColumns.end();++aIter)
        {
            m_pRowSet->firePropertyChange((*aIter)-1 ,m_pImpl->aRow[(*aIter)-1], ORowSetBase::GrantNotifierAccess());
        }
        if ( !m_pImpl->aChangedColumns.empty() )
            m_pRowSet->fireProperty(PROPERTY_ID_ISMODIFIED,sal_True,sal_False, ORowSetBase::GrantNotifierAccess());
    }
}
}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
