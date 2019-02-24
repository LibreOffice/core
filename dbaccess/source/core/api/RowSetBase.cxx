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
#include <stringconstants.hxx>
#include <sal/log.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <comphelper/sequence.hxx>
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

// OEmptyCollection
class OEmptyCollection : public sdbcx::OCollection
{
protected:
    virtual void impl_refresh() override;
    virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName) override;
public:
    OEmptyCollection(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) : OCollection(_rParent, true, _rMutex, std::vector< OUString>()){}
};

void OEmptyCollection::impl_refresh()
{
}

connectivity::sdbcx::ObjectType OEmptyCollection::createObject(const OUString& /*_rName*/)
{
    return connectivity::sdbcx::ObjectType();
}

// ORowSetBase

ORowSetBase::ORowSetBase( const Reference<XComponentContext>& _rContext, ::cppu::OBroadcastHelper& _rBHelper, ::osl::Mutex* _pMutex )
    :OPropertyStateContainer(_rBHelper)
    ,m_pMutex(_pMutex)
    ,m_pMySelf(nullptr)
    ,m_rBHelper(_rBHelper)
    ,m_aContext( _rContext )
    ,m_nLastColumnIndex(-1)
    ,m_nDeletedPosition(-1)
    ,m_nResultSetType( ResultSetType::FORWARD_ONLY )
    ,m_nResultSetConcurrency( ResultSetConcurrency::READ_ONLY )
    ,m_bClone(false)
    ,m_bIgnoreResult(false)
    ,m_bBeforeFirst(true) // changed from sal_False
    ,m_bAfterLast(false)
    ,m_bIsInsertRow(false)
{
    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    registerPropertyNoMember( PROPERTY_ROWCOUNT,        PROPERTY_ID_ROWCOUNT,        nRBT, cppu::UnoType<sal_Int32>::get(), css::uno::makeAny<sal_Int32>(0) );
    registerPropertyNoMember( PROPERTY_ISROWCOUNTFINAL, PROPERTY_ID_ISROWCOUNTFINAL, nRBT, cppu::UnoType<bool>::get(), css::uno::Any(false) );
}

ORowSetBase::~ORowSetBase()
{
    if(m_pColumns)
    {
        TDataColumns().swap(m_aDataColumns);
        m_pColumns->acquire();
        m_pColumns->disposing();
    }
}

// css::lang::XTypeProvider
Sequence< Type > ORowSetBase::getTypes()
{
    return ::comphelper::concatSequences(ORowSetBase_BASE::getTypes(),OPropertyStateContainer::getTypes());
}

// css::uno::XInterface
Any ORowSetBase::queryInterface( const Type & rType )
{
    Any aRet = ORowSetBase_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertyStateContainer::queryInterface(rType);
    return aRet;
}

void SAL_CALL ORowSetBase::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_pCache)
    {
        switch(nHandle)
        {
        case PROPERTY_ID_ROWCOUNT:
            rValue <<= impl_getRowCount();
            break;
        case PROPERTY_ID_ISROWCOUNTFINAL:
            rValue <<= m_pCache->m_bRowCountFinal;
            break;
        default:
            OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
        }
    }
    else
        OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
}

// OComponentHelper
void SAL_CALL ORowSetBase::disposing()
{
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
    m_pCache = nullptr;
}

// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetBase::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSetBase::getInfoHelper()
{
    return *getArrayHelper();
}

// XRow
sal_Bool SAL_CALL ORowSetBase::wasNull(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return !((m_nLastColumnIndex != -1) && !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is())
           || ((*m_aCurrentRow)->get())[m_nLastColumnIndex].isNull();
}

const ORowSetValue& ORowSetBase::getValue(sal_Int32 columnIndex)
{
    checkCache();
    return impl_getValue(columnIndex);
}

const ORowSetValue& ORowSetBase::impl_getValue(sal_Int32 columnIndex)
{
    if ( m_bBeforeFirst || m_bAfterLast )
    {
        SAL_WARN("dbaccess", "ORowSetBase::getValue: Illegal call here (we're before first or after last)!");
        ::dbtools::throwSQLException( DBA_RES( RID_STR_CURSOR_BEFORE_OR_AFTER ), StandardSQLState::INVALID_CURSOR_POSITION, *m_pMySelf );
    }

    if ( impl_rowDeleted() )
    {
        return m_aEmptyValue;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    if ( !bValidCurrentRow )
    {
        // currentrow is null when the clone moves the window
        positionCache( MOVE_NONE );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = false;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    }

    if ( bValidCurrentRow )
    {
#if OSL_DEBUG_LEVEL > 0
        ORowSetMatrix::const_iterator aCacheEnd;
        ORowSetMatrix::iterator aCurrentRow;
        aCacheEnd = m_pCache->getEnd();
        aCurrentRow = m_aCurrentRow;
        ORowSetCacheMap::const_iterator aCacheIter = m_aCurrentRow.getIter();
        ORowSetCacheIterator_Helper aHelper = aCacheIter->second;
        ORowSetMatrix::const_iterator k = aHelper.aIterator;
        for (; k != m_pCache->getEnd(); ++k)
        {
            ORowSetValueVector* pTemp = k->get();
            OSL_ENSURE( pTemp != reinterpret_cast<void*>(0xfeeefeee),"HALT!" );
        }
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow < m_pCache->getEnd() && aCacheIter != m_pCache->m_aCacheIterators.end(),"Invalid iterator set for currentrow!");
        ORowSetRow rRow = *m_aCurrentRow;
        OSL_ENSURE(rRow.is() && static_cast<sal_uInt16>(columnIndex) < (rRow->get()).size(),"Invalid size of vector!");
#endif
        return ((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex];
    }

    // we should normally never reach this
    return m_aEmptyValue;
}

OUString SAL_CALL ORowSetBase::getString( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Bool SAL_CALL ORowSetBase::getBoolean( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return bool(getValue(columnIndex));
}

sal_Int8 SAL_CALL ORowSetBase::getByte( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int16 SAL_CALL ORowSetBase::getShort( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int32 SAL_CALL ORowSetBase::getInt( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

sal_Int64 SAL_CALL ORowSetBase::getLong( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

float SAL_CALL ORowSetBase::getFloat( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

double SAL_CALL ORowSetBase::getDouble( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL ORowSetBase::getBytes( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

css::util::Date SAL_CALL ORowSetBase::getDate( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

css::util::Time SAL_CALL ORowSetBase::getTime( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

css::util::DateTime SAL_CALL ORowSetBase::getTimestamp( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}

Reference< css::io::XInputStream > SAL_CALL ORowSetBase::getBinaryStream( sal_Int32 columnIndex )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
    {
        SAL_WARN("dbaccess", "ORowSetBase::getBinaryStream: Illegal call here (we're before first or after last)!");
        ::dbtools::throwSQLException( DBA_RES( RID_STR_CURSOR_BEFORE_OR_AFTER ), StandardSQLState::INVALID_CURSOR_POSITION, *m_pMySelf );
    }

    if ( impl_rowDeleted() )
    {
        return nullptr;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    if ( !bValidCurrentRow )
    {
        positionCache( MOVE_NONE );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = false;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getBinaryStream: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->is() );
    }

    if ( bValidCurrentRow )
        return new ::comphelper::SequenceInputStream(((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());

    // we should normally never reach this
    return Reference< css::io::XInputStream >();
}

Reference< css::io::XInputStream > SAL_CALL ORowSetBase::getCharacterStream( sal_Int32 columnIndex )
{
    return getBinaryStream(columnIndex);
}

Any SAL_CALL ORowSetBase::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& /*typeMap*/ )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return getValue(columnIndex).makeAny();
}

Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getRef", *m_pMySelf );
    return nullptr;
}

Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 columnIndex )
{
    return Reference< XBlob >(getValue(columnIndex).makeAny(),UNO_QUERY);
}

Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 columnIndex )
{
    return Reference< XClob >(getValue(columnIndex).makeAny(),UNO_QUERY);
}

Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 /*columnIndex*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRow::getArray", *m_pMySelf );
    return nullptr;
}

// css::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::getBookmark() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_BOOKMARK_BEFORE_OR_AFTER ), StandardSQLState::INVALID_CURSOR_POSITION, *m_pMySelf );

    if ( impl_rowDeleted() )
        ::dbtools::throwSQLException( DBA_RES( RID_STR_NO_BOOKMARK_DELETED ), StandardSQLState::INVALID_CURSOR_POSITION, *m_pMySelf );

    OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::getBookmark: bookmark has no value!" );
    return m_aBookmark;
}

sal_Bool SAL_CALL ORowSetBase::moveToBookmark( const Any& bookmark )
{
    SAL_INFO("dbaccess", "ORowSetBase::moveToBookmark(Any) Clone = " << m_bClone);
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

    bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );
        }
        else
        {
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire( );
    }
    SAL_INFO("dbaccess", "ORowSetBase::moveToBookmark(Any) = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows )
{
    SAL_INFO("dbaccess", "ORowSetBase::moveRelativeToBookmark(Any," << rows << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );
        }
        else
            movementFailed();

        // - IsModified
        // - IsNew
        aNotifier.fire( );

        // RowCount/IsRowCountFinal
        fireRowcount();
    }
    SAL_INFO("dbaccess", "ORowSetBase::moveRelativeToBookmark(Any," << rows << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& _first, const Any& _second )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->compareBookmarks(_first,_second);
}

sal_Bool SAL_CALL ORowSetBase::hasOrderedBookmarks(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hasOrderedBookmarks();
}

sal_Int32 SAL_CALL ORowSetBase::hashBookmark( const Any& bookmark )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hashBookmark(bookmark);
}

// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > SAL_CALL ORowSetBase::getMetaData(  )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    Reference< XResultSetMetaData > xMeta;
    if(m_pCache)
        xMeta = m_pCache->getMetaData();

    return xMeta;
}

// XColumnLocate
sal_Int32 SAL_CALL ORowSetBase::findColumn( const OUString& columnName )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    // it is possible to save some time here when we remember the names - position relation in a map
    return m_pColumns ? m_pColumns->findColumn(columnName) : sal_Int32(0);
}

// css::sdbcx::XColumnsSupplier
Reference< XNameAccess > SAL_CALL ORowSetBase::getColumns(  )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    if(!m_pColumns)
    {
        if (!m_pEmptyCollection)
            m_pEmptyCollection.reset( new OEmptyCollection(*m_pMySelf,m_aColumnsMutex) );
        return m_pEmptyCollection.get();
    }

    return m_pColumns.get();
}

// XResultSet
sal_Bool SAL_CALL ORowSetBase::next(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::next() Clone = " << m_bClone);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        positionCache( MOVE_FORWARD );
        bool bAfterLast = m_pCache->isAfterLast();
        bRet = m_pCache->next();
        doCancelModification( );

        // if we were afterLast before next() then we still are,
        // i.e. bAfterLast implies m_pCache->isAfterLast()
        if (bAfterLast)
            assert(m_pCache->isAfterLast());
        // so the only way bAfterLast != m_pCache->isAfterLast()
        // would be that we just arrived there,
        if (bAfterLast != m_pCache->isAfterLast())
        {
            assert(!bAfterLast);
            assert(m_pCache->isAfterLast());
        }
        // in which case we *did* move the cursor
        if ( bRet || bAfterLast != m_pCache->isAfterLast() )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( true, true, aOldValues, aGuard );
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
    SAL_INFO("dbaccess", "ORowSetBase::next() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    SAL_INFO("dbaccess", "ORowSetBase::isBeforeFirst() = " << m_bBeforeFirst << " Clone = " << m_bClone);

    return m_bBeforeFirst;
}

sal_Bool SAL_CALL ORowSetBase::isAfterLast(  )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    SAL_INFO("dbaccess", "ORowSetBase::isAfterLast() = " << m_bAfterLast << " Clone = " << m_bClone);

    return m_bAfterLast;
}

bool ORowSetBase::isOnFirst()
{
    return isFirst();
}

sal_Bool SAL_CALL ORowSetBase::isFirst(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::isFirst() Clone = " << m_bClone);

    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return false;

    if ( impl_rowDeleted() )
        return ( m_nDeletedPosition == 1 );

    positionCache( MOVE_NONE );
    bool bIsFirst = m_pCache->isFirst();

    SAL_INFO("dbaccess", "ORowSetBase::isFirst() = " << bIsFirst << " Clone = " << m_bClone);
    return bIsFirst;
}

bool ORowSetBase::isOnLast()
{
    return isLast();
}

sal_Bool SAL_CALL ORowSetBase::isLast(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::isLast() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return false;

    if ( impl_rowDeleted() )
    {
        if ( !m_pCache->m_bRowCountFinal )
            return false;
        else
            return ( m_nDeletedPosition == impl_getRowCount() );
    }

    positionCache( MOVE_NONE );
    bool bIsLast = m_pCache->isLast();

    SAL_INFO("dbaccess", "ORowSetBase::isLast() = " << bIsLast << " Clone = " << m_bClone);
    return bIsLast;
}

void SAL_CALL ORowSetBase::beforeFirst(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::beforeFirst() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    // check if we are inserting a row
    bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );

            // - IsModified
            // - Isnew
            aNotifier.fire();

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }

        // to be done _after_ the notifications!
        m_aOldRow->clearRow();
    }
    SAL_INFO("dbaccess", "ORowSetBase::beforeFirst() Clone = " << m_bClone);
}

void SAL_CALL ORowSetBase::afterLast(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::afterLast() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );

            // - IsModified
            // - Isnew
            aNotifier.fire();

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }
    }
    SAL_INFO("dbaccess", "ORowSetBase::afterLast() Clone = " << m_bClone);
}

bool SAL_CALL ORowSetBase::move(std::function<bool(ORowSetBase *)> const & _aCheckFunctor,
                                std::function<bool(ORowSetCache *)> const & _aMovementFunctor)
{
    SAL_INFO("dbaccess", "ORowSetBase::move() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bool bMoved = ( bWasNew || !_aCheckFunctor(this) );

        bRet = _aMovementFunctor(m_pCache.get());
        doCancelModification( );

        if ( bRet )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( bMoved, true, aOldValues, aGuard );
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
    SAL_INFO("dbaccess", "ORowSetBase::move() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::first(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::first() Clone = " << m_bClone);
    auto ioF_tmp = std::mem_fn(&ORowSetBase::isOnFirst);
    auto F_tmp = std::mem_fn(&ORowSetCache::first);
    return move(ioF_tmp,F_tmp);
}

sal_Bool SAL_CALL ORowSetBase::last(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::last() Clone = " << m_bClone);
    auto ioL_tmp = std::mem_fn(&ORowSetBase::isOnLast);
    auto L_tmp = std::mem_fn(&ORowSetCache::last);
    return move(ioL_tmp,L_tmp);
}

sal_Int32 SAL_CALL ORowSetBase::getRow(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::getRow() Clone = " << m_bClone);
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();
    return impl_getRow();
}

sal_Int32 ORowSetBase::impl_getRow()
{
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
        positionCache( MOVE_NONE );
        nPos = m_pCache->getRow();
    }
    SAL_INFO("dbaccess", "ORowSetBase::impl_getRow() = " << nPos << " Clone = " << m_bClone);
    return nPos;
}

sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row )
{
    SAL_INFO("dbaccess", "ORowSetBase::absolute(" << row << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    bool bRet = ( row > 0 )
                &&  notifyAllListenersCursorBeforeMove( aGuard );
    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );
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
    SAL_INFO("dbaccess", "ORowSetBase::absolute(" << row << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::relative( sal_Int32 rows )
{
    SAL_INFO("dbaccess", "ORowSetBase::relative(" << rows << ") Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!rows)
        return true; // in this case do nothing

    checkPositioningAllowed();

    bool bRet =
            (  ( !m_bAfterLast || rows <= 0 )
            && ( !m_bBeforeFirst || rows >= 0 )
            && notifyAllListenersCursorBeforeMove( aGuard )
            );

    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
            setCurrentRow( true, true, aOldValues, aGuard );
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
    SAL_INFO("dbaccess", "ORowSetBase::relative(" << rows << ") = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

sal_Bool SAL_CALL ORowSetBase::previous(  )
{
    SAL_INFO("dbaccess", "ORowSetBase::previous() Clone = " << m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    bool bRet = !m_bBeforeFirst
                &&  notifyAllListenersCursorBeforeMove(aGuard);

    if ( bRet )
    {
        // check if we are inserting a row
        bool bWasNew = m_pCache->m_bNew || rowDeleted();

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        positionCache( MOVE_BACKWARD );
        bRet = m_pCache->previous();
        doCancelModification( );

        // if m_bBeforeFirst is false and bRet is false then we stood on the first row
        if(!m_bBeforeFirst || bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow( true, true, aOldValues, aGuard );
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
    SAL_INFO("dbaccess", "ORowSetBase::previous() = " << bRet << " Clone = " << m_bClone);
    return bRet;
}

void ORowSetBase::setCurrentRow( bool _bMoved, bool _bDoNotify, const ORowSetRow& _rOldValues, ::osl::ResettableMutexGuard& _rGuard )
{
    SAL_INFO("dbaccess", "ORowSetBase::setCurrentRow() Clone = " << m_bClone);
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        m_aBookmark     = m_pCache->getBookmark();
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = false;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is null!");
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
        OSL_ENSURE(m_aCurrentRow->is(),"Currentrow isn't valid");
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");

        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_bIsInsertRow  = false;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is nul after positionCache!");
#if OSL_DEBUG_LEVEL > 0
        ORowSetRow rRow = *m_aCurrentRow;
        OSL_ENSURE(rRow.is() ,"Invalid size of vector!");
#endif

        // notification order
        // - column values
        if ( _bDoNotify )
            firePropertyChange(_rOldValues);

    }
    else
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
    }

    // TODO: can this be done before the notifications?
    if(!(m_bBeforeFirst || m_bAfterLast) && !m_aCurrentRow.isNull() && m_aCurrentRow->is() && m_aCurrentRow != m_pCache->getEnd())
        m_aOldRow->setRow(new ORowSetValueVector( *(*m_aCurrentRow) ));

    if ( _bMoved && _bDoNotify )
        // - cursorMoved
        notifyAllListenersCursorMoved( _rGuard );

    SAL_INFO("dbaccess", "ORowSetBase::setCurrentRow() Clone = " << m_bClone);
}

void ORowSetBase::checkPositioningAllowed()
{
    if(!m_pCache || m_nResultSetType == ResultSetType::FORWARD_ONLY)
        throwFunctionSequenceException(*m_pMySelf);
}

Reference< XInterface >  ORowSetBase::getStatement()
{
    return nullptr;
}

void SAL_CALL ORowSetBase::refreshRow(  )
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    if ( impl_rowDeleted() )
        throwSQLException( "The current row is deleted", StandardSQLState::INVALID_CURSOR_STATE, Reference< XRowSet >( this ) );

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        bool bWasNew = m_pCache->m_bNew || impl_rowDeleted();
        ORowSetRow aOldValues = getOldRow(bWasNew);
        positionCache( MOVE_NONE );
        m_pCache->refreshRow();
        firePropertyChange(aOldValues);
    }
}

sal_Bool SAL_CALL ORowSetBase::rowUpdated(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( impl_rowDeleted() )
        return false;

    return m_pCache->rowUpdated();
}

sal_Bool SAL_CALL ORowSetBase::rowInserted(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();

    if ( impl_rowDeleted() )
        return false;

    return m_pCache->rowInserted();
}

sal_Bool SAL_CALL ORowSetBase::rowDeleted(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return impl_rowDeleted();
}

bool ORowSetBase::impl_rowDeleted(  )
{
    return !m_aBookmark.hasValue() && !m_bBeforeFirst && !m_bAfterLast;
}

// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  )
{
    ::osl::MutexGuard aGuard( *m_pMutex );

    if ( m_pCache )
    {
        Reference< XWarningsSupplier > xWarnings( m_pCache->m_xSet.get(), UNO_QUERY );
        if ( xWarnings.is() )
            return xWarnings->getWarnings();
    }

    return Any();
}

void SAL_CALL ORowSetBase::clearWarnings(  )
{
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
    SAL_INFO("dbaccess", "ORowSetBase::firePropertyChange() Clone = " << m_bClone);
    OSL_ENSURE(m_pColumns,"Columns can not be NULL here!");
    sal_Int32 i=0;
    for (auto const& dataColumn : m_aDataColumns)
    {
        try
        {
            dataColumn->fireValueChange(_rOldRow.is() ? (_rOldRow->get())[i+1] : ::connectivity::ORowSetValue());
        }
        catch (const Exception&)
        {
            SAL_WARN("dbaccess", "firePropertyChange: Exception on column " << i);
        }
        ++i;
    }
    SAL_INFO("dbaccess", "ORowSetBase::firePropertyChange() Clone = " << m_bClone);
}

void ORowSetBase::firePropertyChange(sal_Int32 _nPos,const ::connectivity::ORowSetValue& _rOldValue)
{
    OSL_ENSURE(_nPos < static_cast<sal_Int32>(m_aDataColumns.size()),"nPos is invalid!");
    m_aDataColumns[_nPos]->fireValueChange(_rOldValue);
}

void ORowSetBase::fireRowcount()
{
}

bool ORowSetBase::notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& /*_rGuard*/)
{
    return true;
}

void ORowSetBase::notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& /*_rGuard*/)
{
}

bool ORowSetBase::isPropertyChangeNotificationEnabled() const
{
    return true;
}

void ORowSetBase::fireProperty( sal_Int32 _nProperty, bool _bNew, bool _bOld )
{
    Any aNew = css::uno::makeAny( _bNew );
    Any aOld = css::uno::makeAny( _bOld );
    fire( &_nProperty, &aNew, &aOld, 1, false );
}

void ORowSetBase::positionCache( CursorMoveDirection _ePrepareForDirection )
{
    SAL_INFO("dbaccess", "ORowSetBase::positionCache() Clone = " << m_bClone);

    bool bSuccess = false;
    if ( m_aBookmark.hasValue() )
    {
        if (_ePrepareForDirection == MOVE_NONE_REFRESH ||
            (m_pCache->isAfterLast() != bool(isAfterLast())) || ( m_pCache->isBeforeFirst() != bool(isBeforeFirst()) ) ||
             m_pCache->compareBookmarks( m_aBookmark, m_pCache->getBookmark() ) != CompareBookmark::EQUAL )
            bSuccess = m_pCache->moveToBookmark( m_aBookmark );
        else
            bSuccess = true;
    }
    else
    {
        if ( m_bBeforeFirst )
        {
            m_pCache->beforeFirst();
            bSuccess = true;
        }
        else if ( m_bAfterLast )
        {
            m_pCache->afterLast();
            bSuccess = true;
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
                    bSuccess = true;
                }
                break;

            case MOVE_BACKWARD:
                if ( m_pCache->m_bRowCountFinal && ( m_nDeletedPosition == impl_getRowCount() ) )
                {
                    m_pCache->afterLast();
                    bSuccess = true;
                }
                else
                    bSuccess = m_pCache->absolute( m_nDeletedPosition );
                break;

            case MOVE_NONE:
            case MOVE_NONE_REFRESH:
                bSuccess = false;   // will be asserted below
                break;
            }
        }
    }
    OSL_ENSURE( bSuccess, "ORowSetBase::positionCache: failed!" );

    SAL_INFO("dbaccess", "ORowSetBase::positionCache() Clone = " << m_bClone);
}

void ORowSetBase::checkCache()
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    if(!m_pCache)
        throwFunctionSequenceException(*m_pMySelf);
}

void ORowSetBase::movementFailed()
{
    SAL_INFO("dbaccess", "ORowSetBase::movementFailed() Clone = " << m_bClone);
    m_aOldRow->clearRow();
    m_aCurrentRow   = m_pCache->getEnd();
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    m_aBookmark     = Any();
    OSL_ENSURE(m_bBeforeFirst || m_bAfterLast,"BeforeFirst or AfterLast is wrong!");
    SAL_INFO("dbaccess", "ORowSetBase::movementFailed() Clone = " << m_bClone);
}

ORowSetRow ORowSetBase::getOldRow(bool _bWasNew)
{
    OSL_ENSURE(m_aOldRow.is(),"RowSetRowHElper isn't valid!");
    ORowSetRow aOldValues;
    if ( !_bWasNew && m_aOldRow->getRow().is() )
        aOldValues = new ORowSetValueVector( *(m_aOldRow->getRow()));    // remember the old values
    return aOldValues;
}

void ORowSetBase::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}

void ORowSetBase::onDeleteRow( const Any& _rBookmark )
{
    if ( rowDeleted() )
        // not interested in
        return;

    ::osl::MutexGuard aGuard( *m_pMutex );
    //OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::onDeleteRow: Bookmark isn't valid!" );
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == CompareBookmark::EQUAL )
    {
        positionCache( MOVE_NONE );
        m_nDeletedPosition = m_pCache->getRow();
    }
}

void ORowSetBase::onDeletedRow( const Any& _rBookmark, sal_Int32 _nPos )
{
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
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == CompareBookmark::EQUAL )
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
    }
}

sal_Int32 ORowSetBase::impl_getRowCount() const
{
    sal_Int32 nRowCount( m_pCache->m_nRowCount );
    if ( const_cast< ORowSetBase* >( this )->rowDeleted() && !m_pCache->m_bNew )
        ++nRowCount;
    return nRowCount;
}

struct ORowSetNotifierImpl
{
    std::vector<sal_Int32>    aChangedColumns;
    ORowSetValueVector::Vector  aRow;
};


ORowSetNotifier::ORowSetNotifier( ORowSetBase* _pRowSet )
    :m_pRowSet( _pRowSet )
    ,m_bWasNew( false )
    ,m_bWasModified( false )
{

    OSL_ENSURE( m_pRowSet, "ORowSetNotifier::ORowSetNotifier: invalid row set. This will crash." );

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
    ,m_bWasNew( false )
    ,m_bWasModified( false )
{

    OSL_ENSURE( m_pRowSet, "ORowSetNotifier::ORowSetNotifier: invalid row set. This will crash." );
    m_pImpl->aRow = i_aRow; // yes, create a copy to store the old values
}

ORowSetNotifier::~ORowSetNotifier( )
{
}

void ORowSetNotifier::fire()
{
    // we're not interested in firing changes FALSE->TRUE, only TRUE->FALSE.
    // (the former would be quite pathological, e.g. after a failed movement)

    if  (   m_bWasModified
        &&  ( m_bWasModified != m_pRowSet->isModified( ORowSetBase::GrantNotifierAccess() ) )
        )
        m_pRowSet->fireProperty( PROPERTY_ID_ISMODIFIED, false, true, ORowSetBase::GrantNotifierAccess() );

    if  (   m_bWasNew
        &&  ( m_bWasNew != m_pRowSet->isNew( ORowSetBase::GrantNotifierAccess() ) )
        )
        m_pRowSet->fireProperty( PROPERTY_ID_ISNEW, false, true, ORowSetBase::GrantNotifierAccess() );
}

std::vector<sal_Int32>& ORowSetNotifier::getChangedColumns() const
{
    OSL_ENSURE(m_pImpl, "Illegal CTor call, use the other one!");
    return m_pImpl->aChangedColumns;
}

void ORowSetNotifier::firePropertyChange()
{
    OSL_ENSURE(m_pImpl, "Illegal CTor call, use the other one!");
    if (m_pImpl)
    {
        for (auto const& changedColumn : m_pImpl->aChangedColumns)
        {
            m_pRowSet->firePropertyChange(changedColumn-1 ,m_pImpl->aRow[changedColumn-1], ORowSetBase::GrantNotifierAccess());
        }
        if ( !m_pImpl->aChangedColumns.empty() )
            m_pRowSet->fireProperty(PROPERTY_ID_ISMODIFIED,true,false, ORowSetBase::GrantNotifierAccess());
    }
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
