/*************************************************************************
 *
 *  $RCSfile: RowSetBase.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-01 14:34:01 $
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
    virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException)
    {
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > createObject(const ::rtl::OUString& _rName)
    {
        return ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >();
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createEmptyObject()
    {
        return ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >();
    }
public:
    OEmptyCollection(::cppu::OWeakObject& _rParent,::osl::Mutex& _rMutex) : OCollection(_rParent,sal_True,_rMutex,::std::vector< ::rtl::OUString>()){}
};

// =========================================================================
// = ORowSetBase
// =========================================================================
// -------------------------------------------------------------------------
ORowSetBase::ORowSetBase(::cppu::OBroadcastHelper   &_rBHelper,::osl::Mutex& _rMutex)
            : OPropertyContainer(_rBHelper)
            , m_rMutex(_rMutex)
            , m_rBHelper(_rBHelper)
            , m_aListeners(m_rMutex)
            , m_aApproveListeners(m_rMutex)
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
        delete m_pColumns;
        m_pColumns = NULL;
    }
}
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ORowSetBase::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(ORowSetBase_BASE::getTypes(),OPropertyContainer::getTypes());
}
// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any ORowSetBase::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aRet = OPropertyContainer::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ORowSetBase_BASE::queryInterface(rType);
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
            OPropertyContainer::getFastPropertyValue(rValue,nHandle);
        };
    }
    else
        OPropertyContainer::getFastPropertyValue(rValue,nHandle);
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL ORowSetBase::disposing(void)
{
    MutexGuard aGuard(m_rMutex);

    if(m_pColumns)
        m_pColumns->disposing();
    m_xEmptyCollection  = NULL;
    m_pCache            = NULL;
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
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return ((m_nLastColumnIndex != -1) && m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd()) ? (*(*m_aCurrentRow))[m_nLastColumnIndex].isNull() : sal_True;
}
// -----------------------------------------------------------------------------
const ORowSetValue& ORowSetBase::getValue(sal_Int32 columnIndex)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();
    OSL_ENSURE(!(m_bBeforeFirst || m_bAfterLast),"Illegal call here!");

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    else
    {
        OSL_ENSURE(m_aCurrentRow && (m_bBeforeFirst || m_bAfterLast),"ORowSetBase::getValue: we don't stand on a valid row! Row is equal to end of matrix");
        if(!m_aCurrentRow)
        {
            positionCache();
            m_aCurrentRow   = m_pCache->m_aMatrixIter;

            OSL_ENSURE(m_aCurrentRow,"ORowSetBase::getValue: we don't stand on a valid row! Row is null.");
            return getValue(columnIndex);
        }
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
    ::osl::MutexGuard aGuard( m_rMutex );
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
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return Reference< XArray >();
}
// -------------------------------------------------------------------------
// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );

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
    OSL_ENSURE(bookmark.hasValue(),"ORowSetBase::moveToBookmark bookmark has no value!");
    ::osl::MutexGuard aGuard( m_rMutex );

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
    if(bRet = notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        bRet = m_pCache->moveToBookmark(bookmark);
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
        }
        else
        {
            OSL_ENSURE(0,"MoveToBookmark doesn't work!");
            movementFailed();
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_rMutex );

    checkPositioningAllowed();


    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        bRet = m_pCache->moveRelativeToBookmark(bookmark,rows);
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
        }
        else
            movementFailed();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();
    return m_pCache->compareBookmarks(first,second);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();
    return m_pCache->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
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
        if(!m_xEmptyCollection.is())
            m_xEmptyCollection = new OEmptyCollection(*m_pMySelf,m_aColumnsMutex);
        return m_xEmptyCollection;
    }

    return m_pColumns;
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL ORowSetBase::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
        {
            aOldValues = &m_aOldRow;     // remember the old values
        }
        bRet = m_pCache->next();
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
            OSL_ENSURE(!m_bBeforeFirst,"BeforeFirst is true. I don't know why?");
        }
        else
        {
            // moved after the last row
            movementFailed();
            OSL_ENSURE(m_bAfterLast,"AfterLast is false. I don't know why?");
        }
        fireRowcount();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    return m_bBeforeFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    return m_bAfterLast;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    sal_Bool bIsFirst = !(m_bBeforeFirst || m_bAfterLast);
    if(bIsFirst)
    {
        positionCache();
        bIsFirst = m_pCache->isFirst();
    }

    return bIsFirst;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isLast(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    sal_Bool bIsLast = !(m_bBeforeFirst || m_bAfterLast);
    if(bIsLast) // so we can't be on the last
    {
        positionCache();
        bIsLast = m_pCache->isLast();
    }
    return bIsLast;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );

    checkPositioningAllowed();

    if(notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        if(!m_bBeforeFirst)
        {   // we have to move
            ORowSetMatrix::iterator aOldValues = NULL;
            if(!bWasNew && m_aOldRow.isValid())
                aOldValues = &m_aOldRow;     // remember the old values

            m_pCache->beforeFirst();
            m_aBookmark     = Any();
            m_aCurrentRow   = m_pCache->getEnd();
            m_aCurrentRow.setBookmark(m_aBookmark);
            m_bBeforeFirst  = !(m_bAfterLast = sal_False);
            notifyAllListenersCursorMoved();
            firePropertyChange(aOldValues);
            m_aOldRow       = NULL;
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    if(notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        if(!m_bAfterLast)
        {
            ORowSetMatrix::iterator aOldValues = NULL;
            if(!bWasNew && m_aOldRow.isValid())
                aOldValues = &m_aOldRow;     // remember the old values

            m_pCache->afterLast();
            m_aBookmark     = Any();
            m_aCurrentRow   = m_pCache->getEnd();
            m_aCurrentRow.setBookmark(m_aBookmark);
            m_bAfterLast    = !(m_bBeforeFirst = sal_False);
            notifyAllListenersCursorMoved();
            fireRowcount();
            firePropertyChange(aOldValues);
            m_aOldRow       = NULL;
        }
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::first(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        sal_Bool bMoved = sal_False;
        if(!isFirst() || bWasNew)
            bMoved = sal_True;

        bRet = m_pCache->first();
        if(bRet)
            setCurrentRow(bMoved,aOldValues);
        else
        {// first goes wrong so there is no row
            movementFailed();
        }
        fireRowcount();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::last(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    sal_Bool bRet;
    if(bRet = notifyAllListenersCursorBeforeMove())
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        sal_Bool bMoved = sal_False;
        if(!isLast() || bWasNew)
            bMoved = sal_True;

        bRet = m_pCache->last();
        if(bRet)
        {
            setCurrentRow(bMoved,aOldValues);
        }
        else
        { // last goes wrong so there is no row
            movementFailed();
            OSL_ENSURE(m_bAfterLast && m_bBeforeFirst,"Last failed so there is no row but AfterLast or BeforeFirst are wrong!");
        }

        fireRowcount();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
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
    return nPos;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );
    checkPositioningAllowed();

    sal_Bool bRet = !(m_bAfterLast && row > 1); // m_bAfterLast && row > 1 we are already behind the last row

    if(bRet && (bRet = notifyAllListenersCursorBeforeMove()))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        bRet = m_pCache->absolute(row);
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
        }
        else
        { // absolute movement goes wrong we stand left or right side of the rows
            movementFailed();
        }
        fireRowcount();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_rMutex );

    if(!rows)
        return sal_True; // in this case do nothing

    checkPositioningAllowed();

    sal_Bool bRet =!((m_bAfterLast && rows > 1) || (m_bBeforeFirst && rows < 0)); // we are already behind the last row or before the first


    if(bRet && (bRet = notifyAllListenersCursorBeforeMove()))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        bRet = m_pCache->relative(rows);
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
        }
        else
        {
            movementFailed();
        }

        fireRowcount();
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::previous(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_rMutex );

    checkPositioningAllowed();

    sal_Bool bRet = !m_bBeforeFirst;

    if(bRet && (bRet = notifyAllListenersCursorBeforeMove()))
    {
        // check if we are inserting a row
        sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
        checkInsert();

        ORowSetMatrix::iterator aOldValues = NULL;
        if(!bWasNew && m_aOldRow.isValid())
            aOldValues = &m_aOldRow;     // remember the old values

        bRet = m_pCache->previous();
        //  if(!(m_bBeforeFirst = m_pCache->isBeforeFirst()) && bRet)     // TODO have a look at it
        if(bRet)
        {
            setCurrentRow(sal_True,aOldValues);
        }
        else
        {
            movementFailed();
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void ORowSetBase::setCurrentRow(sal_Bool _bMoved,const ORowSetMatrix::iterator& _rOldValues)
{
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();

    if(!(m_bBeforeFirst || m_bAfterLast))
    {
        m_aBookmark     = m_pCache->getBookmark();
        OSL_ENSURE(m_aBookmark.hasValue(),"Bookamrk has no value!");
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE((*(*m_aCurrentRow))[0].makeAny().hasValue(),"Bookamrk has no value!");

        OSL_ENSURE(!m_aCurrentRow.isNull() && m_aCurrentRow != m_pCache->getEnd(),"Position of matrix iterator isn't valid!");
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
    }
    else
    {
        m_aOldRow       = NULL;
        m_aCurrentRow   = m_pCache->getEnd();
        m_aBookmark     = Any();
        m_aCurrentRow.setBookmark(m_aBookmark);
    }

    if(_bMoved)
        notifyAllListenersCursorMoved();
    firePropertyChange(_rOldValues);

    if(!(m_bBeforeFirst || m_bAfterLast))
        m_aOldRow       = (*m_aCurrentRow);
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
    ::osl::MutexGuard aGuard( m_rMutex );
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
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return m_pCache->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return m_pCache->rowInserted();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutex );
    checkCache();

    return m_pCache->rowDeleted();
}
// -------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    if(!m_pCache)
        return Any();

    ::osl::MutexGuard aGuard( m_rMutex );

    return m_pCache->getWarnings();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(m_rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_rMutex );
    if(m_pCache)
        m_pCache->clearWarnings();
}
// -------------------------------------------------------------------------
void ORowSetBase::firePropertyChange(const ORowSetMatrix::iterator& _rOldRow)
{
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel;

    ORowSetRow aRow;
    if(_rOldRow && _rOldRow != m_pCache->m_pMatrix->end() && _rOldRow->isValid())
        aRow = *_rOldRow;

    OSL_ENSURE(m_pColumns,"Columns can not be NULL here!");
    sal_Int32 i=0;
    try
    {
        for(;i<m_pColumns->getCount();++i)
        {
            if(::cppu::extractInterface(xTunnel,m_pColumns->getByIndex(i)) && xTunnel.is())
            {
                OColumn* pColumn = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());
                if(pColumn)
                {
                    OSL_ENSURE(!aRow.isValid() || (i+1) < sal_Int32(aRow->size()),"Index is greater than vector size!");
                    pColumn->fireValueChange(aRow.isValid() ? (*aRow)[i+1].makeAny() : Any());
                }
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"firePropertyChange: Exception");
    }
}
// -----------------------------------------------------------------------------
void ORowSetBase::positionCache()
{
    if(m_aBookmark.hasValue())
    {
        sal_Bool bOK = m_pCache->moveToBookmark(m_aBookmark);
        OSL_ENSURE(bOK ,"ORowSetBase::positionCache: positioning cache fails!");
    }
    else
        OSL_ENSURE(0,"ORowSetBase::positionCache: no bookmark set!");
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
    m_aOldRow       = NULL;
    m_aCurrentRow   = m_pCache->getEnd();
    m_bBeforeFirst  = m_pCache->isBeforeFirst();
    m_bAfterLast    = m_pCache->isAfterLast();
    m_aBookmark     = Any();
    m_aCurrentRow.setBookmark(m_aBookmark);
}
// -----------------------------------------------------------------------------


}   // namespace dbaccess
