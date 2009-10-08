/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowSetBase.cxx,v $
 * $Revision: 1.95 $
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
#ifndef DBACCESS_CORE_API_ROWSETBASE_HXX
#include "RowSetBase.hxx"
#endif
#ifndef DBACCESS_CORE_API_CROWSETDATACOLUMN_HXX
#include "CRowSetDataColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include <connectivity/sdbcx/VCollection.hxx>
#endif
#ifndef DBACCESS_CORE_API_ROWSETCACHE_HXX
#include "RowSetCache.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
// -------------------------------------------------------------------------
class OEmptyCollection : public sdbcx::OCollection
{
protected:
    virtual void impl_refresh() throw(RuntimeException);
    virtual connectivity::sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
public:
    OEmptyCollection(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) : OCollection(_rParent,sal_True,_rMutex,::std::vector< ::rtl::OUString>()){}
};
// -----------------------------------------------------------------------------
void OEmptyCollection::impl_refresh() throw(RuntimeException)
{
}
// -----------------------------------------------------------------------------
connectivity::sdbcx::ObjectType OEmptyCollection::createObject(const ::rtl::OUString& /*_rName*/)
{
    return connectivity::sdbcx::ObjectType();
}
// -----------------------------------------------------------------------------

// =========================================================================
// = ORowSetBase
// =========================================================================
DBG_NAME(ORowSetBase)
// -------------------------------------------------------------------------
ORowSetBase::ORowSetBase( const ::comphelper::ComponentContext& _rContext, ::cppu::OBroadcastHelper& _rBHelper, ::osl::Mutex* _pMutex )
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
{
    DBG_CTOR(ORowSetBase,NULL);

    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    sal_Int32 nInitialRowCountValue = 0;
    sal_Bool bInitialRowCountFinalValue( sal_False );
    registerPropertyNoMember( PROPERTY_ROWCOUNT,        PROPERTY_ID_ROWCOUNT,        nRBT, ::getCppuType( &nInitialRowCountValue ), &nInitialRowCountValue );
    registerPropertyNoMember( PROPERTY_ISROWCOUNTFINAL, PROPERTY_ID_ISROWCOUNTFINAL, nRBT, ::getBooleanCppuType(),                  &bInitialRowCountFinalValue );
}
// -----------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
Sequence< Type > ORowSetBase::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(ORowSetBase_BASE::getTypes(),OPropertyStateContainer::getTypes());
}
// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any ORowSetBase::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aRet = ORowSetBase_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertyStateContainer::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
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
            rValue.setValue(&m_pCache->m_bRowCountFinal,::getCppuBooleanType());
            break;
        default:
            OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
        };
    }
    else
        OPropertyStateContainer::getFastPropertyValue(rValue,nHandle);
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL ORowSetBase::disposing(void)
{
    MutexGuard aGuard(*m_pMutex);

    if ( m_pColumns )
    {
        TDataColumns().swap(m_aDataColumns);
        m_pColumns->disposing();
    }
    if ( m_pCache )
        m_pCache->deregisterOldRow(m_aOldRow);
    m_pCache = NULL;
}
// -------------------------------------------------------------------------
// comphelper::OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* ORowSetBase::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
// cppu::OPropertySetHelper
::cppu::IPropertyArrayHelper& SAL_CALL ORowSetBase::getInfoHelper()
{
    return *const_cast<ORowSetBase*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
// XRow
sal_Bool SAL_CALL ORowSetBase::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();


    return ((m_nLastColumnIndex != -1) && !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->isValid()) ? ((*m_aCurrentRow)->get())[m_nLastColumnIndex].isNull() : sal_True;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
const ORowSetValue& ORowSetBase::getValue(sal_Int32 columnIndex)
{
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
    {
        OSL_ENSURE(0,"ORowSetBase::getValue: Illegal call here (we're before first or after last)!");
        throwSQLException( "The cursor points to before the first or after the last row.", SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
            // TODO: resource
    }

    if ( rowDeleted() )
    {
        return m_aEmptyValue;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->isValid() );
    if ( !bValidCurrentRow )
    {
        // currentrow is null when the clone moves the window
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->isValid() );
    }

    if ( bValidCurrentRow )
    {
#if OSL_DEBUG_LEVEL > 0
        ORowSetMatrix::iterator aCacheEnd;
        ORowSetMatrix::iterator aCurrentRow;
        aCacheEnd = m_pCache->getEnd();
        aCurrentRow = m_aCurrentRow;
        ORowSetCacheMap::iterator aCacheIter = m_aCurrentRow.getIter();
        sal_Int32 n = aCacheIter->first;
        n = n;
        ORowSetCacheIterator_Helper aHelper = aCacheIter->second;
        ORowSetMatrix::iterator k = aHelper.aIterator;
        for (; k != m_pCache->getEnd(); ++k)
        {
            ORowSetValueVector* pTemp = k->getBodyPtr();
            OSL_ENSURE( pTemp != (void*)0xfeeefeee,"HALT!" );
        }
#endif
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow < m_pCache->getEnd() && aCacheIter != m_pCache->m_aCacheIterators.end(),"Invalid iterator set for currentrow!");
#if OSL_DEBUG_LEVEL > 0
        ORowSetRow rRow = (*m_aCurrentRow);
        OSL_ENSURE(rRow.isValid() && static_cast<sal_uInt16>(columnIndex) < (rRow->get()).size(),"Invalid size of vector!");
#endif
        return ((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex];
    }

    // we should normally never reach this
    return m_aEmptyValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORowSetBase::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL ORowSetBase::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL ORowSetBase::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL ORowSetBase::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL ORowSetBase::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL ORowSetBase::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSetBase::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL ORowSetBase::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL ORowSetBase::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL ORowSetBase::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
    {
        OSL_ENSURE(0,"ORowSetBase::getBinaryStream: Illegal call here (we're before first or after last)!");
        throwSQLException( "The cursor points to before the first or after the last row.", SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
            // TODO: resource
    }

    if ( rowDeleted() )
    {
        return NULL;
    }

    bool bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->isValid() );
    if ( !bValidCurrentRow )
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"ORowSetBase::getBinaryStream: we don't stand on a valid row! Row is null.");

        bValidCurrentRow = ( !m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd() && m_aCurrentRow->isValid() );
    }

    if ( bValidCurrentRow )
        return new ::comphelper::SequenceInputStream(((*m_aCurrentRow)->get())[m_nLastColumnIndex = columnIndex].getSequence());

    // we should normally never reach this
    return Reference< ::com::sun::star::io::XInputStream >();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL ORowSetBase::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return getValue(columnIndex).makeAny();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRef", *m_pMySelf );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBlob", *m_pMySelf );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getClob", *m_pMySelf );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getArray", *m_pMySelf );
    return NULL;
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::getBookmark() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        throwSQLException( "The rows before the first and after the last row don't have a bookmark.", SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
            // TODO: resource

    if ( rowDeleted() )
        throwSQLException( "The current row is deleted, and thus doesn't have a bookmark.", SQL_INVALID_CURSOR_POSITION, *m_pMySelf );
            // TODO: resource

    OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::getBookmark: bookmark has no value!" );
    return m_aBookmark;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::moveToBookmark(Any) Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    OSL_ENSURE(bookmark.hasValue(),"ORowSetBase::moveToBookmark bookmark has no value!");
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    if(!bookmark.hasValue() || m_nResultSetType == ResultSetType::FORWARD_ONLY)
    {
        if(bookmark.hasValue())
            OSL_ENSURE(0,"MoveToBookmark is not possible when we are only forward");
        else
            OSL_ENSURE(0,"Bookmark is not valid");
        throwFunctionSequenceException(*m_pMySelf);
    }


    checkCache();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
    DBG_TRACE2("DBACCESS ORowSetBase::moveToBookmark(Any) = %i Clone = %i\n",bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::moveRelativeToBookmark(Any,%i) Clone = %i\n",rows,m_bClone);
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
    DBG_TRACE3("DBACCESS ORowSetBase::moveRelativeToBookmark(Any,%i) = %i Clone = %i\n",rows,bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& _first, const Any& _second ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->compareBookmarks(_first,_second);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->hashBookmark(bookmark);
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > SAL_CALL ORowSetBase::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    Reference< XResultSetMetaData > xMeta;
    if(m_pCache)
        xMeta = m_pCache->getMetaData();

    return xMeta;
}
// -------------------------------------------------------------------------

// XColumnLocate
sal_Int32 SAL_CALL ORowSetBase::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    // it is possible to save some time her when we remember the names - position relation in a map
    return m_pColumns ? m_pColumns->findColumn(columnName) : sal_Int32(0);
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< XNameAccess > SAL_CALL ORowSetBase::getColumns(  ) throw(RuntimeException)
{
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
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL ORowSetBase::next(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::next() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    sal_Bool bRet( notifyAllListenersCursorBeforeMove( aGuard ) );
    if ( bRet )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
    DBG_TRACE3("DBACCESS ORowSetBase::next() = %i Clone = %i ID = %i\n",bRet,m_bClone,osl_getThreadIdentifier(NULL));
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    DBG_TRACE2("DBACCESS ORowSetBase::isBeforeFirst() = %i Clone = %i\n",m_bBeforeFirst,m_bClone);

    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    DBG_TRACE2("DBACCESS ORowSetBase::isAfterLast() = %i Clone = %i\n",m_bAfterLast,m_bClone);

    return m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetBase::isOnFirst()
{
    return isFirst();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isFirst(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::isFirst() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));

    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return sal_False;

    if ( rowDeleted() )
        return ( m_nDeletedPosition == 1 );

    positionCache( MOVE_NONE_REFRESH_ONLY );
    sal_Bool bIsFirst = m_pCache->isFirst();

    DBG_TRACE2("DBACCESS ORowSetBase::isFirst() = %i Clone = %i\n",bIsFirst,m_bClone);
    return bIsFirst;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetBase::isOnLast()
{
    return isLast();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isLast(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::isLast() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if ( m_bBeforeFirst || m_bAfterLast )
        return sal_False;

    if ( rowDeleted() )
    {
        if ( !m_pCache->m_bRowCountFinal )
            return sal_False;
        else
            return ( m_nDeletedPosition == impl_getRowCount() );
    }

    positionCache( MOVE_NONE_REFRESH_ONLY );
    sal_Bool bIsLast = m_pCache->isLast();

    DBG_TRACE2("DBACCESS ORowSetBase::isLast() = %i Clone = %i\n",bIsLast,m_bClone);
    return bIsLast;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::beforeFirst() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
    DBG_TRACE2("DBACCESS ORowSetBase::beforeFirst() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::afterLast(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::afterLast() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bWasNew = m_pCache->m_bNew || rowDeleted();

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
    DBG_TRACE2("DBACCESS ORowSetBase::afterLast() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::move(    ::std::mem_fun_t<sal_Bool,ORowSetBase>& _aCheckFunctor,
                                        ::std::mem_fun_t<sal_Bool,ORowSetCache>& _aMovementFunctor)
{
    DBG_TRACE2("DBACCESS ORowSetBase::move() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
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
    DBG_TRACE2("DBACCESS ORowSetBase::move() = %i Clone = %i\n",bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::first(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::first() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioF_tmp(&ORowSetBase::isOnFirst);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> F_tmp(&ORowSetCache::first);
    return move(ioF_tmp,F_tmp);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::last(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::last() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioL_tmp(&ORowSetBase::isOnLast);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> L_tmp(&ORowSetCache::last);
    return move(ioL_tmp,L_tmp);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getRow(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::getRow() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();


    sal_Int32  nPos = 0;
    if ( m_bBeforeFirst )
        nPos = 0;
    else if ( m_bAfterLast )
        nPos = impl_getRowCount() + 1;
    else if ( rowDeleted() )
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
    DBG_TRACE3("DBACCESS ORowSetBase::getRow() = %i Clone = %i ID = %i\n",nPos,m_bClone,osl_getThreadIdentifier(NULL));
    return nPos;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::absolute(%i) Clone = %i\n",row,m_bClone);
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
    DBG_TRACE3("DBACCESS ORowSetBase::absolute(%i) = %i Clone = %i\n",row,bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::relative(%i) Clone = %i\n",rows,m_bClone);
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
    DBG_TRACE3("DBACCESS ORowSetBase::relative(%i) = %i Clone = %i\n",rows,bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::previous(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::previous() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
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
            DBG_ERROR( "ORowSetBase::previous: inconsistency!" );
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
    DBG_TRACE2("DBACCESS ORowSetBase::previous() = %i Clone = %i\n",bRet,m_bClone);
    return bRet;
}
// -----------------------------------------------------------------------------
void ORowSetBase::setCurrentRow( sal_Bool _bMoved, sal_Bool _bDoNotify, const ORowSetRow& _rOldValues, ::osl::ResettableMutexGuard& _rGuard )
{
    DBG_TRACE2("DBACCESS ORowSetBase::setCurrentRow() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    //m_pCache->resetInsertRow(sal_True);

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        m_aBookmark     = m_pCache->getBookmark();
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is null!");
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");

#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nOldRow = m_pCache->getRow();
#endif
        positionCache( MOVE_NONE_REFRESH_ONLY );
#if OSL_DEBUG_LEVEL > 0
        sal_Int32 nNewRow = m_pCache->getRow();
#endif
        OSL_ENSURE(nOldRow == nNewRow,"Old position is not equal to new postion");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(!m_aCurrentRow.isNull(),"CurrentRow is nul after positionCache!");
#if OSL_DEBUG_LEVEL > 0
        ORowSetRow rRow = (*m_aCurrentRow);
        OSL_ENSURE(rRow.isValid() ,"Invalid size of vector!");
#endif
        // the cache could repositioned so we need to adjust the cache
        // #104144# OJ
        if ( _bMoved && m_aCurrentRow.isNull() )
        {
            positionCache( MOVE_NONE_REFRESH_ONLY );
            m_aCurrentRow   = m_pCache->m_aMatrixIter;
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
    if(!(m_bBeforeFirst || m_bAfterLast) && !m_aCurrentRow.isNull() && m_aCurrentRow->isValid() && m_aCurrentRow != m_pCache->getEnd())
        m_aOldRow->setRow(new ORowSetValueVector(m_aCurrentRow->getBody()));

    if ( _bMoved && _bDoNotify )
        // - cursorMoved
        notifyAllListenersCursorMoved( _rGuard );

    DBG_TRACE2("DBACCESS ORowSetBase::setCurrentRow() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}
// -----------------------------------------------------------------------------
void ORowSetBase::checkPositioningAllowed() throw( SQLException, RuntimeException )
{
    if(!m_pCache || m_nResultSetType == ResultSetType::FORWARD_ONLY)
        throwFunctionSequenceException(*m_pMySelf);
}
//------------------------------------------------------------------------------
Reference< XInterface >  ORowSetBase::getStatement(void) throw( SQLException, RuntimeException )
{
    return NULL;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    if ( rowDeleted() )
        throwSQLException( "The current row is deleted", SQL_INVALID_CURSOR_STATE, Reference< XRowSet >( this ) );

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_pCache->refreshRow();
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();


    if ( rowDeleted() )
        return sal_False;

    if ( rowDeleted() )
        return sal_False;

    return m_pCache->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );

    checkCache();


    if ( rowDeleted() )
        return sal_False;

    if ( rowDeleted() )
        return sal_False;

    return m_pCache->rowInserted();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return !m_aBookmark.hasValue() && !m_bBeforeFirst && !m_bAfterLast;
}
// -------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    Reference< XWarningsSupplier > xWarnings( m_pCache->m_xSet.get(), UNO_QUERY );
    if ( xWarnings.is() )
        return xWarnings->getWarnings();

    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();


    Reference< XWarningsSupplier > xWarnings( m_pCache->m_xSet.get(), UNO_QUERY );
    if ( xWarnings.is() )
        xWarnings->clearWarnings();
}
// -------------------------------------------------------------------------
void ORowSetBase::firePropertyChange(const ORowSetRow& _rOldRow)
{
    DBG_TRACE2("DBACCESS ORowSetBase::firePropertyChange() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    OSL_ENSURE(m_pColumns,"Columns can not be NULL here!");
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bNull;
    ORowSetMatrix::iterator atest;
    bNull = m_aCurrentRow.isNull();
    atest = m_aCurrentRow;
#endif
    sal_Int32 i=0;
    try
    {
        for(TDataColumns::iterator aIter = m_aDataColumns.begin();aIter != m_aDataColumns.end();++aIter,++i) // #104278# OJ ++i inserted
            (*aIter)->fireValueChange(_rOldRow.isValid() ? (_rOldRow->get())[i+1] : ::connectivity::ORowSetValue());
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"firePropertyChange: Exception");
    }
    DBG_TRACE2("DBACCESS ORowSetBase::firePropertyChange() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}

// -----------------------------------------------------------------------------
void ORowSetBase::fireRowcount()
{
}

// -----------------------------------------------------------------------------
sal_Bool ORowSetBase::notifyAllListenersCursorBeforeMove(::osl::ResettableMutexGuard& /*_rGuard*/)
{
    return sal_True;
}

// -----------------------------------------------------------------------------
void ORowSetBase::notifyAllListenersCursorMoved(::osl::ResettableMutexGuard& /*_rGuard*/)
{
}

// -----------------------------------------------------------------------------
void ORowSetBase::notifyAllListeners(::osl::ResettableMutexGuard& /*_rGuard*/)
{
}

// -----------------------------------------------------------------------------
void ORowSetBase::fireProperty( sal_Int32 _nProperty, sal_Bool _bNew, sal_Bool _bOld )
{
    Any aNew = bool2any( _bNew );
    Any aOld = bool2any( _bOld );
    fire( &_nProperty, &aNew, &aOld, 1, sal_False );
}

// -----------------------------------------------------------------------------
void ORowSetBase::positionCache( CursorMoveDirection _ePrepareForDirection )
{
    DBG_TRACE2("DBACCESS ORowSetBase::positionCache() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));

    sal_Bool bSuccess = sal_False;
    if ( m_aBookmark.hasValue() )
    {
        bSuccess = m_pCache->moveToBookmark( m_aBookmark );
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

    DBG_TRACE2("DBACCESS ORowSetBase::positionCache() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}
// -----------------------------------------------------------------------------
void ORowSetBase::checkCache()
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    if(!m_pCache)
        throwFunctionSequenceException(*m_pMySelf);
}
// -----------------------------------------------------------------------------
void ORowSetBase::movementFailed()
{
    DBG_TRACE2("DBACCESS ORowSetBase::movementFailed() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
    m_aOldRow->clearRow();
    m_aCurrentRow   = m_pCache->getEnd();
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    m_aBookmark     = Any();
    m_aCurrentRow.setBookmark(m_aBookmark);
    OSL_ENSURE(m_bBeforeFirst || m_bAfterLast,"BeforeFirst or AfterLast is wrong!");
    DBG_TRACE2("DBACCESS ORowSetBase::movementFailed() Clone = %i ID = %i\n",m_bClone,osl_getThreadIdentifier(NULL));
}
// -----------------------------------------------------------------------------
ORowSetRow ORowSetBase::getOldRow(sal_Bool _bWasNew)
{
    OSL_ENSURE(m_aOldRow.isValid(),"RowSetRowHElper isn't valid!");
    ORowSetRow aOldValues;
    if ( !_bWasNew && m_aOldRow->getRow().isValid() )
        aOldValues = new ORowSetValueVector( m_aOldRow->getRow().getBody());     // remember the old values
    return aOldValues;
}
// -----------------------------------------------------------------------------
void ORowSetBase::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}
// -----------------------------------------------------------------------------
void ORowSetBase::onDeleteRow( const Any& _rBookmark )
{
    if ( rowDeleted() )
        // not interested in
        return;

    ::osl::MutexGuard aGuard( *m_pMutex );
    OSL_ENSURE( m_aBookmark.hasValue(), "ORowSetBase::onDeleteRow: Bookmark isn't valid!" );
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == 0 )
    {
        positionCache( MOVE_NONE_REFRESH_ONLY );
        m_nDeletedPosition = m_pCache->getRow();
    }
}
// -----------------------------------------------------------------------------
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
    if ( compareBookmarks( _rBookmark, m_aBookmark ) == 0 )
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
        m_aCurrentRow.setBookmark( m_aBookmark );
    }
}
// -----------------------------------------------------------------------------
sal_Int32 ORowSetBase::impl_getRowCount() const
{
    sal_Int32 nRowCount( m_pCache->m_nRowCount );
    if ( const_cast< ORowSetBase* >( this )->rowDeleted() && !m_pCache->m_bNew )
        ++nRowCount;
    return nRowCount;
}
// =============================================================================
DBG_NAME(ORowSetNotifier)
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
ORowSetNotifier::~ORowSetNotifier( )
{
    DBG_DTOR(ORowSetNotifier,NULL);
}

// -----------------------------------------------------------------------------
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

}   // namespace dbaccess
