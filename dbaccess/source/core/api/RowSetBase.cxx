/*************************************************************************
 *
 *  $RCSfile: RowSetBase.cxx,v $
 *
 *  $Revision: 1.72 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:00:45 $
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
#ifndef _CONNECTIVITY_EMPTYMETADATA_HXX_
#include <connectivity/emptymetadata.hxx>
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
    virtual Reference< XNamed > createObject(const ::rtl::OUString& _rName);
public:
    OEmptyCollection(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) : OCollection(_rParent,sal_True,_rMutex,::std::vector< ::rtl::OUString>()){}
};
// -----------------------------------------------------------------------------
void OEmptyCollection::impl_refresh() throw(RuntimeException)
{
}
// -----------------------------------------------------------------------------
Reference< XNamed > OEmptyCollection::createObject(const ::rtl::OUString& _rName)
{
    return Reference< XNamed >();
}
// -----------------------------------------------------------------------------

// =========================================================================
// = ORowSetBase
// =========================================================================
// -------------------------------------------------------------------------
ORowSetBase::ORowSetBase(::cppu::OBroadcastHelper   &_rBHelper,::osl::Mutex* _pMutex)
            : OPropertyStateContainer(_rBHelper)
            , m_pMutex(_pMutex)
            , m_rBHelper(_rBHelper)
            , m_pCache(NULL)
            , m_pColumns(NULL)
            , m_nRowCount(0)
            , m_bBeforeFirst(sal_True) // changed from sal_False
            , m_bAfterLast(sal_False)
            , m_bRowCountFinal(sal_False)
            , m_bClone(sal_False)
            , m_nPosition(-1)
            , m_bIgnoreResult(sal_False)
            , m_nLastColumnIndex(-1)
            , m_pEmptyCollection( NULL )
{
    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    registerProperty(PROPERTY_ROWCOUNT,             PROPERTY_ID_ROWCOUNT,               nRBT,                           &m_nRowCount,           ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ISROWCOUNTFINAL,      PROPERTY_ID_ISROWCOUNTFINAL,        nRBT,                           &m_bRowCountFinal,      ::getBooleanCppuType());
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
            rValue <<= m_pCache->m_nRowCount;
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

    return ((m_nLastColumnIndex != -1) && m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd() && !m_aCurrentRow.isNull() && m_aCurrentRow->isValid()) ? (*(*m_aCurrentRow))[m_nLastColumnIndex].isNull() : sal_True;
}
// -----------------------------------------------------------------------------
const ORowSetValue& ORowSetBase::getValue(sal_Int32 columnIndex)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    OSL_ENSURE(!(m_bBeforeFirst || m_bAfterLast),"ORowSetBase::getValue: Illegal call here (we're before first or after last)!");

    if ( m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd() && !m_aCurrentRow.isNull() && m_aCurrentRow->isValid() )
    {
#if OSL_DEBUG_LEVEL > 0
        ORowSetMatrix::iterator aCacheEnd = m_pCache->getEnd();
        ORowSetMatrix::iterator aCurrentRow = m_aCurrentRow;
#endif
        OSL_ENSURE(m_aCurrentRow && m_aCurrentRow <= m_pCache->getEnd(),"Invalid iterator set for currentrow!");
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    }
    else
    {   // currentrow is null when the clone move the window
        if(!m_aCurrentRow || m_aCurrentRow.isNull())
        {
            positionCache();
            m_aCurrentRow   = m_pCache->m_aMatrixIter;

            OSL_ENSURE(m_aCurrentRow,"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");
            return getValue(columnIndex);
        }
        OSL_ENSURE(m_aCurrentRow && (m_bBeforeFirst || m_bAfterLast),"ORowSetBase::getValue: we don't stand on a valid row! Row is equal to end of matrix");
    }
    // we should normally never reach this here
    return m_aEmptyValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORowSetBase::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL ORowSetBase::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL ORowSetBase::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL ORowSetBase::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
float SAL_CALL ORowSetBase::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
double SAL_CALL ORowSetBase::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSetBase::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL ORowSetBase::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL ORowSetBase::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL ORowSetBase::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(columnIndex);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return new ::comphelper::SequenceInputStream((*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex].getSequence());
    else
    {
        if(m_aCurrentRow)
            OSL_ENSURE((m_bBeforeFirst || m_bAfterLast),"ORowSetBase::getValue: we don't stand on a valid row! Row is equal to end of matrix");
        else
        {
            positionCache();
            m_aCurrentRow   = m_pCache->m_aMatrixIter;

            OSL_ENSURE(m_aCurrentRow,"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");
            return getBinaryStream(columnIndex);
        }
    }


    return Reference< ::com::sun::star::io::XInputStream >();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return getBinaryStream(columnIndex);
}
// -------------------------------------------------------------------------
Any SAL_CALL ORowSetBase::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return Reference< XArray >();
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::getBookmark() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );

    if(!m_pCache || m_bBeforeFirst || m_bAfterLast)
    {
        OSL_ENSURE(0,"Ask for bookmark but we stand on invalid row.");
        throwFunctionSequenceException(*m_pMySelf);
    }

    OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
    return m_aBookmark;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::moveToBookmark(Any) Clone = %i\n",m_bClone);
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

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove(aGuard))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->moveToBookmark(bookmark);
        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
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


    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove(aGuard))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->moveRelativeToBookmark(bookmark,rows);
        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
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
sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();
    return m_pCache->compareBookmarks(first,second);
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
    else
        xMeta = new OEmptyMetaData();

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
    DBG_TRACE1("DBACCESS ORowSetBase::next() Clone = %i\n",m_bClone);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkCache();

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove(aGuard))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        if ( m_aBookmark.hasValue() ) // #104474# OJ
            positionCache();
        bRet = m_pCache->next();

        if ( bRet )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
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
    DBG_TRACE2("DBACCESS ORowSetBase::next() = %i Clone = %i\n",bRet,m_bClone);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::isBeforeFirst() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();
    DBG_TRACE2("DBACCESS ORowSetBase::isBeforeFirst() = %i Clone = %i\n",m_bBeforeFirst,m_bClone);

    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::isAfterLast() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();
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
    DBG_TRACE1("DBACCESS ORowSetBase::isFirst() Clone = %i\n",m_bClone);

    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bIsFirst = !(m_bBeforeFirst || m_bAfterLast);
    if(bIsFirst)
    {
        positionCache();
        bIsFirst = m_pCache->isFirst();
    }

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
    DBG_TRACE1("DBACCESS ORowSetBase::isLast() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bIsLast = !(m_bBeforeFirst || m_bAfterLast);
    if(bIsLast) // so we can't be on the last
    {
        positionCache();
        bIsLast = m_pCache->isLast();
    }
    DBG_TRACE2("DBACCESS ORowSetBase::isLast() = %i Clone = %i\n",bIsLast,m_bClone);
    return bIsLast;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::beforeFirst() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

    if((bWasNew || !m_bBeforeFirst) && notifyAllListenersCursorBeforeMove(aGuard) )
    {
        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        if(!m_bBeforeFirst)
        {
            m_pCache->beforeFirst();
            m_aBookmark     = Any();
            m_aCurrentRow   = m_pCache->getEnd();
            m_aCurrentRow.setBookmark(m_aBookmark);
            m_bBeforeFirst  = !(m_bAfterLast = sal_False);

            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();

        // to be done _after_ the notifications!
        m_aOldRow->clearRow();
    }
    DBG_TRACE1("DBACCESS ORowSetBase::beforeFirst() Clone = %i\n",m_bClone);
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::afterLast(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::afterLast() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

    if((bWasNew || !m_bAfterLast) && notifyAllListenersCursorBeforeMove(aGuard) )
    {
        // check if we are inserting a row
        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        if(!m_bAfterLast)
        {
            ORowSetRow aOldValues = getOldRow(bWasNew);

            m_pCache->afterLast();

            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);

            // - IsModified
            // - Isnew
            aNotifier.fire();

            // - RowCount/IsRowCountFinal
            fireRowcount();
        }
    }
    DBG_TRACE1("DBACCESS ORowSetBase::afterLast() Clone = %i\n",m_bClone);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::move(    ::std::mem_fun_t<sal_Bool,ORowSetBase>& _aCheckFunctor,
                                        ::std::mem_fun_t<sal_Bool,ORowSetCache>& _aMovementFunctor)
{
    DBG_TRACE1("DBACCESS ORowSetBase::move() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove(aGuard) )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        sal_Bool bMoved = ( bWasNew || !_aCheckFunctor(this) );

        bRet = _aMovementFunctor(m_pCache);

        if ( bRet )
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(bMoved,aOldValues,aGuard);
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
    DBG_TRACE1("DBACCESS ORowSetBase::first() Clone = %i\n",m_bClone);
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioF_tmp(&ORowSetBase::isOnFirst);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> F_tmp(&ORowSetCache::first);
    return move(ioF_tmp,F_tmp);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::last(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::last() Clone = %i\n",m_bClone);
    ::std::mem_fun_t<sal_Bool,ORowSetBase> ioL_tmp(&ORowSetBase::isOnLast);
    ::std::mem_fun_t<sal_Bool,ORowSetCache> L_tmp(&ORowSetCache::last);
    return move(ioL_tmp,L_tmp);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getRow(  ) throw(SQLException, RuntimeException)
{
    DBG_TRACE1("DBACCESS ORowSetBase::getRow() Clone = %i\n",m_bClone);
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    sal_Int32  nPos = 0;
    if(!(m_bAfterLast || m_bBeforeFirst)) // we are on no valid row
    {
        // check if we are inserting a row
        if(!(!m_bClone && m_pCache->m_bInserted))
        {
            if(!m_aBookmark.hasValue()) // check if we are standing on a deleted row
                nPos = m_nPosition;
            else
            {
                if(m_pCache->m_bAfterLast || m_pCache->m_bBeforeFirst || m_pCache->compareBookmarks(m_aBookmark,m_pCache->getBookmark()) != CompareBookmark::EQUAL)
                {
#ifdef DBG_UTIL
                    sal_Bool bRet = m_pCache->moveToBookmark(m_aBookmark);
                    OSL_ENSURE(bRet,"moveToBookamrk failed so the position isn't valid!");
#else
                    m_pCache->moveToBookmark(m_aBookmark);

#endif
                }
                nPos = m_pCache->getRow();
            }
        }
    }
    DBG_TRACE2("DBACCESS ORowSetBase::getRow() = %i Clone = %i\n",nPos,m_bClone);
    return nPos;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    DBG_TRACE2("DBACCESS ORowSetBase::absolute(%i) Clone = %i\n",row,m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );
    checkPositioningAllowed();

    sal_Bool bRet = !(m_bAfterLast && row > 1); // m_bAfterLast && row > 1 we are already behind the last row

    if ( bRet && (bRet = notifyAllListenersCursorBeforeMove(aGuard)) )
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        bRet = m_pCache->absolute(row);

        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
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

    sal_Bool bRet =!((m_bAfterLast && rows > 1) || (m_bBeforeFirst && rows < 0)); // we are already behind the last row or before the first


    if(bRet && (bRet = notifyAllListenersCursorBeforeMove(aGuard)))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        if ( m_aBookmark.hasValue() ) // #104474# OJ
            positionCache();
        bRet = m_pCache->relative(rows);

        if(bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
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
    DBG_TRACE1("DBACCESS ORowSetBase::previous() Clone = %i\n",m_bClone);
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::ResettableMutexGuard aGuard( *m_pMutex );

    checkPositioningAllowed();

    sal_Bool bRet = !m_bBeforeFirst;

    if(bRet && (bRet = notifyAllListenersCursorBeforeMove(aGuard)))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;

        ORowSetNotifier aNotifier( this );
            // this will call cancelRowModification on the cache if necessary

        ORowSetRow aOldValues = getOldRow(bWasNew);

        if ( m_aBookmark.hasValue() ) // #104474# OJ
            positionCache();
        bRet = m_pCache->previous();

        // if m_bBeforeFirst is false and bRet is false than we stood on the first row
        if(!m_bBeforeFirst || bRet)
        {
            // notification order
            // - column values
            // - cursorMoved
            setCurrentRow(sal_True,aOldValues,aGuard);
        }
        else
        {
            movementFailed();
        }

        // - IsModified
        // - IsNew
        aNotifier.fire();
    }
    DBG_TRACE2("DBACCESS ORowSetBase::previous() = %i Clone = %i\n",bRet,m_bClone);
    return bRet;
}
// -----------------------------------------------------------------------------
void ORowSetBase::setCurrentRow(sal_Bool _bMoved,const ORowSetRow& _rOldValues,::osl::ResettableMutexGuard& _rGuard)
{
    DBG_TRACE1("DBACCESS ORowSetBase::setCurrentRow() Clone = %i",m_bClone);
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        m_aBookmark     = m_pCache->getBookmark();
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(m_aCurrentRow,"CurrentRow is null!");
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");

        sal_Int32 nOldRow = m_pCache->getRow();
        positionCache();
        sal_Int32 nNewRow = m_pCache->getRow();
        OSL_ENSURE(nOldRow == nNewRow,"Old position is not equal to new postion");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        OSL_ENSURE(m_aCurrentRow,"CurrentRow is nul after positionCache!");
    }
    else
    {
        m_aOldRow->clearRow();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
        m_aCurrentRow.setBookmark(m_aBookmark);
    }

    if ( _bMoved )
    {
        // the cache could repositioned so we need to adjust the cache
        // #104144# OJ
        if ( m_aCurrentRow.isNull() )
        {
            positionCache();
            m_aCurrentRow   = m_pCache->m_aMatrixIter;
            OSL_ENSURE(m_aCurrentRow,"CurrentRow is nul after positionCache!");
        }
    }

    // notification order
    // - column values
    firePropertyChange(_rOldValues);

    // TODO: can this be done before the notifications?
    if(!(m_bBeforeFirst || m_bAfterLast) && !m_aCurrentRow.isNull() && m_aCurrentRow->isValid() && m_aCurrentRow != m_pCache->getEnd())
        m_aOldRow->setRow(new ORowSetValueVector(m_aCurrentRow->getBody()));

    if ( _bMoved )
        // - cursorMoved
        notifyAllListenersCursorMoved( _rGuard );

    DBG_TRACE1("DBACCESS ORowSetBase::setCurrentRow() Clone = %i\n",m_bClone);
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
    checkPositioningAllowed();

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        positionCache();
        m_pCache->refreshRow();
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return m_pCache->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return m_pCache->rowInserted();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( *m_pMutex );
    checkCache();

    return m_pCache->rowDeleted();
}
// -------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void ORowSetBase::firePropertyChange(const ORowSetRow& _rOldRow)
{
    DBG_TRACE1("DBACCESS ORowSetBase::firePropertyChange() Clone = %i\n",m_bClone);
    OSL_ENSURE(m_pColumns,"Columns can not be NULL here!");
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bNull = m_aCurrentRow.isNull();
    ORowSetMatrix::iterator atest = m_aCurrentRow;
#endif
    //  OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
    OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
    sal_Int32 i=0;
    try
    {
        for(TDataColumns::iterator aIter = m_aDataColumns.begin();aIter != m_aDataColumns.end();++aIter,++i) // #104278# OJ ++i inserted
            (*aIter)->fireValueChange(_rOldRow.isValid() ? (*_rOldRow)[i+1] : ::connectivity::ORowSetValue());
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"firePropertyChange: Exception");
    }
    DBG_TRACE1("DBACCESS ORowSetBase::firePropertyChange() Clone = %i\n",m_bClone);
}

// -----------------------------------------------------------------------------
sal_Bool ORowSetBase::isModification( )
{
    return m_pCache && m_pCache->m_bInserted;
}

// -----------------------------------------------------------------------------
void ORowSetBase::fireProperty( sal_Int32 _nProperty, sal_Bool _bNew, sal_Bool _bOld )
{
    Any aNew = bool2any( _bNew );
    Any aOld = bool2any( _bOld );
    fire( &_nProperty, &aNew, &aOld, 1, sal_False );
}

// -----------------------------------------------------------------------------
void ORowSetBase::positionCache()
{
    DBG_TRACE1("DBACCESS ORowSetBase::positionCache() Clone = %i\n",m_bClone);
    if(m_aBookmark.hasValue())
    {
        sal_Bool bOK = m_pCache->moveToBookmark(m_aBookmark);
        OSL_ENSURE(bOK ,"ORowSetBase::positionCache: positioning cache fails!");
    }
    else
        OSL_ENSURE(0,"ORowSetBase::positionCache: no bookmark set!");
    DBG_TRACE1("DBACCESS ORowSetBase::positionCache() Clone = %i\n",m_bClone);
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
    DBG_TRACE1("DBACCESS ORowSetBase::movementFailed() Clone = %i\n",m_bClone);
    m_aOldRow->clearRow();
    m_aCurrentRow   = m_pCache->getEnd();
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    m_aBookmark     = Any();
    m_aCurrentRow.setBookmark(m_aBookmark);
    OSL_ENSURE(m_bBeforeFirst || m_bAfterLast,"BeforeFirst or AfterLast is wrong!");
    DBG_TRACE1("DBACCESS ORowSetBase::movementFailed() Clone = %i\n",m_bClone);
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
Any ORowSetBase::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    return Any();
}
// =============================================================================
// -----------------------------------------------------------------------------
ORowSetNotifier::ORowSetNotifier( ORowSetBase* _pRowSet )
    :m_pRowSet( _pRowSet )
    ,m_bWasNew( sal_False )
    ,m_bWasModified( sal_False )
#ifdef DBG_UTIL
    ,m_bNotifyCalled( sal_False )
#endif
{
    OSL_ENSURE( m_pRowSet, "ORowSetNotifier::ORowSetNotifier: invalid row set. This wil crash." );

    // remember the "inserted" and "modified" state for later firing
    m_bWasNew       = m_pRowSet->isNew( ORowSetBase::GrantNotifierAccess() );;
    m_bWasModified  = m_pRowSet->isModified( ORowSetBase::GrantNotifierAccess() );

    // if the row set is on the insert row, then we need to cancel this
    if ( m_pRowSet->isModification( ORowSetBase::GrantNotifierAccess() ) )
        m_pRowSet->doCancelModification( ORowSetBase::GrantNotifierAccess() );
}

// -----------------------------------------------------------------------------
ORowSetNotifier::~ORowSetNotifier( )
{
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
