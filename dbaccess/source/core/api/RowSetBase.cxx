/*************************************************************************
 *
 *  $RCSfile: RowSetBase.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:19:18 $
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
{
    sal_Int32 nRBT  = PropertyAttribute::READONLY   | PropertyAttribute::BOUND      | PropertyAttribute::TRANSIENT;

    registerProperty(PROPERTY_ROWCOUNT,             PROPERTY_ID_ROWCOUNT,               nRBT,                           &m_nRowCount,           ::getCppuType(reinterpret_cast< sal_Int32*>(NULL)));
    registerProperty(PROPERTY_ISROWCOUNTFINAL,      PROPERTY_ID_ISROWCOUNTFINAL,        nRBT,                           &m_bRowCountFinal,      ::getBooleanCppuType());
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
    {
        m_pColumns->disposing();
        delete m_pColumns;
        m_pColumns = NULL;
    }

    m_pCache    = NULL;
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
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return (m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd()) ? (*(*m_aCurrentRow))[m_nLastColumnIndex].isNull() : sal_True;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORowSetBase::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];

    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);
    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL ORowSetBase::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return sal_Int8(0);
}
// -------------------------------------------------------------------------
sal_Int16 SAL_CALL ORowSetBase::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return sal_Int16(0);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return sal_Int32(0);
}
// -------------------------------------------------------------------------
sal_Int64 SAL_CALL ORowSetBase::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return sal_Int64();
}
// -------------------------------------------------------------------------
float SAL_CALL ORowSetBase::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return float(0.0);
}
// -------------------------------------------------------------------------
double SAL_CALL ORowSetBase::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return double(0.0);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL ORowSetBase::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL ORowSetBase::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return ::com::sun::star::util::Date();
}
// -------------------------------------------------------------------------
::com::sun::star::util::Time SAL_CALL ORowSetBase::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return ::com::sun::star::util::Time();
}
// -------------------------------------------------------------------------
::com::sun::star::util::DateTime SAL_CALL ORowSetBase::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return (*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex];
    return ::com::sun::star::util::DateTime();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return new ::comphelper::SequenceInputStream((*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex].getSequence());
    return Reference< ::com::sun::star::io::XInputStream >();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ORowSetBase::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_aCurrentRow && m_aCurrentRow != m_pCache->getEnd())
        return new ::comphelper::SequenceInputStream((*(*m_aCurrentRow))[m_nLastColumnIndex = columnIndex].getSequence());
    return Reference< ::com::sun::star::io::XInputStream >();
}
// -------------------------------------------------------------------------
Any SAL_CALL ORowSetBase::getObject( sal_Int32 columnIndex, const Reference< XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return Any();
}
// -------------------------------------------------------------------------
Reference< XRef > SAL_CALL ORowSetBase::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return Reference< XRef >();
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ORowSetBase::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return Reference< XBlob >();
}
// -------------------------------------------------------------------------
Reference< XClob > SAL_CALL ORowSetBase::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return Reference< XClob >();
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ORowSetBase::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return Reference< XArray >();
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XRowLocate
Any SAL_CALL ORowSetBase::getBookmark(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache || m_bBeforeFirst || m_bAfterLast)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    OSL_ENSURE(m_aBookmark.hasValue(),"Bookmark has no value!");
    return m_aBookmark;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(bookmark.hasValue(),"ORowSetBase::moveToBookmark bookmark has no value!");
    if(!bookmark.hasValue())
        throw FunctionSequenceException(*m_pMySelf);
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

    notifyAllListenersCursorBeforeMove();

//  if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    sal_Bool bRet = m_pCache->moveToBookmark(bookmark);
    if(bRet)
    {
        m_bAfterLast    = m_bBeforeFirst = sal_False; // all false because we stand on a valid row
        m_aBookmark     = bookmark;
        m_aCurrentRow   = m_pCache->m_aMatrixIter;

        m_aCurrentRow.setBookmark(m_aBookmark);
        notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);

        m_aOldRow       = (*m_aCurrentRow);
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

//  if(m_bBeforeFirst)
//      m_pCache->beforeFirst();
//  else if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    notifyAllListenersCursorBeforeMove();

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    sal_Bool bRet = m_pCache->moveRelativeToBookmark(bookmark,rows);
    if(bRet)
    {
        m_bAfterLast    = m_pCache->isAfterLast();
        m_bBeforeFirst = m_pCache->isBeforeFirst(); // all false because we stand on a valid row
        if(m_bAfterLast || m_bBeforeFirst)
        {
            m_aCurrentRow   = m_pCache->getEnd();
            m_aOldRow       = NULL;
            m_aCurrentRow.setBookmark(Any());
        }
        else
        {
            m_aBookmark     = m_pCache->getBookmark();
            m_aCurrentRow   = m_pCache->m_aMatrixIter;
            m_aCurrentRow.setBookmark(m_aBookmark);
            notifyAllListenersCursorMoved();
            firePropertyChange(aOldValues);
            m_aOldRow       = (*m_aCurrentRow);
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);
    return m_pCache->compareBookmarks(first,second);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);
    return m_pCache->hasOrderedBookmarks();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);
    return m_pCache->hashBookmark(bookmark);
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// XResultSetMetaDataSupplier
Reference< XResultSetMetaData > SAL_CALL ORowSetBase::getMetaData(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(m_pCache)
        return m_pCache->getMetaData();
    return new OEmptyMetaData();
}
// -------------------------------------------------------------------------

// XColumnLocate
sal_Int32 SAL_CALL ORowSetBase::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aColumnsMutex );
    // it is possible to save some time her when we remember the names - position relation in a map
    return m_pColumns->findColumn(columnName);
}
// -------------------------------------------------------------------------

// ::com::sun::star::sdbcx::XColumnsSupplier
Reference< XNameAccess > SAL_CALL ORowSetBase::getColumns(  ) throw(RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pColumns)
        return new OEmptyCollection(*m_pMySelf,m_aColumnsMutex);

    ::osl::MutexGuard aGuard( m_aColumnsMutex );

    return m_pColumns;
}
// -------------------------------------------------------------------------
// XResultSet
sal_Bool SAL_CALL ORowSetBase::next(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

//  if(m_bBeforeFirst)
//      m_pCache->beforeFirst();
//  else if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    notifyAllListenersCursorBeforeMove();

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
    {
        aOldValues = &m_aOldRow;     // remember the old values
    }
    sal_Bool bRet = m_pCache->next();
    if(bRet)
    {
        m_bBeforeFirst  = sal_False;
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);
        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_aCurrentRow   = m_pCache->getEnd();
        m_aOldRow       = NULL;
        m_aCurrentRow.setBookmark(Any());
        m_bAfterLast    = sal_True;
    }
    fireRowcount();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);
//  if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);
    return m_bBeforeFirst;
    //  m_pCache->isBeforeFirst();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    return m_bAfterLast;
    //  return m_aCurrentRow == m_pCache->getEnd();

//  if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);
//  return m_pCache->isAfterLast();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isFirst(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_bBeforeFirst || m_bAfterLast) // so we can't be on the first
        return sal_False;

    ::osl::MutexGuard aGuard( m_rMutex );

    if(m_aBookmark.hasValue())
        m_pCache->moveToBookmark(m_aBookmark);

    return m_pCache->isFirst();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::isLast(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_bBeforeFirst || m_bAfterLast) // so we can't be on the last
        return sal_False;

    ::osl::MutexGuard aGuard( m_rMutex );
    if(m_aBookmark.hasValue())
        m_pCache->moveToBookmark(m_aBookmark);

    return m_pCache->isLast();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    checkInsert();

    if(!m_bBeforeFirst)
    {
        sal_Bool bMoved = sal_False;
        notifyAllListenersCursorBeforeMove();

        m_pCache->beforeFirst();
        m_aBookmark     = Any();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aCurrentRow.setBookmark(m_aBookmark);
        m_aOldRow       = NULL;
        m_bBeforeFirst  = !(m_bAfterLast = sal_False);
        notifyAllListenersCursorMoved();
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::afterLast(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    // check if we are inserting a row
    checkInsert();

    if(!m_bAfterLast)
    {
        notifyAllListenersCursorBeforeMove();
        m_pCache->afterLast();
        m_aBookmark     = Any();
        m_aCurrentRow   = m_pCache->getEnd();
        m_aCurrentRow.setBookmark(m_aBookmark);
        m_aOldRow       = NULL;
        m_bAfterLast    = !(m_bBeforeFirst = sal_False);
        notifyAllListenersCursorMoved();
        fireRowcount();
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::first(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

//  if(!(m_bAfterLast || m_bBeforeFirst) && m_aBookmark.hasValue()) // set the cache to the right position
//      m_pCache->moveToBookmark(m_aBookmark);

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    m_bAfterLast = m_bBeforeFirst = sal_False; // all false

    sal_Bool bMoved = sal_False;
    if(!isFirst() || bWasNew)
    {
        notifyAllListenersCursorBeforeMove();
        bMoved = sal_True;
    }

    sal_Bool bRet = m_pCache->first();
    if(bRet)
    {
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        if(bMoved)
            notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);
        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_bAfterLast = m_bBeforeFirst = sal_True; // all true
        m_aCurrentRow   = m_pCache->getEnd();
        m_aCurrentRow.setBookmark(Any());
    }
    fireRowcount();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::last(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

//  if(!(m_bAfterLast || m_bBeforeFirst) && m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    m_bAfterLast = m_bBeforeFirst = sal_False; // all false

    sal_Bool bMoved = sal_False;
    if(!isLast() || bWasNew)
    {
        notifyAllListenersCursorBeforeMove();
        bMoved = sal_True;
    }
    sal_Bool bRet = m_pCache->last();
    if(bRet)
    {
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        if(bMoved)
            notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);
        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_bAfterLast    = m_bBeforeFirst = sal_True; // all true
        m_aCurrentRow   = m_pCache->getEnd();
        m_aOldRow       = NULL;
        m_aCurrentRow.setBookmark(Any());
    }

    fireRowcount();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ORowSetBase::getRow(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_bAfterLast || m_bBeforeFirst) // we are on no valid row
        return 0;

    // check if we are inserting a row
    if(!m_bClone && m_pCache->m_bInserted)
        return 0;

    if(!m_aBookmark.hasValue()) // check if we are standing on a deleted row
        return m_nPosition;

    ::osl::MutexGuard aGuard( m_rMutex );

    if(m_pCache->m_bAfterLast || m_pCache->m_bBeforeFirst || m_pCache->compareBookmarks(m_aBookmark,m_pCache->getBookmark()) != CompareBookmark::EQUAL)
        m_pCache->moveToBookmark(m_aBookmark);
    return m_pCache->getRow();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if((m_bAfterLast && row > 1))
        return sal_False;// we are already behind the last row

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

//  if(m_bBeforeFirst)
//      m_pCache->beforeFirst();
//  else if(m_aBookmark.hasValue())
//  {
//      m_pCache->moveToBookmark(m_aBookmark);
//      if(m_pCache->getRow() == row)
//          return sal_True;
//  }
    m_bAfterLast = m_bBeforeFirst = sal_False; // all false
    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    notifyAllListenersCursorBeforeMove();
    sal_Bool bRet = m_pCache->absolute(row);
    if(bRet)
    {
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);
        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_aCurrentRow   = m_pCache->getEnd();
        m_aCurrentRow.setBookmark(Any());
        m_aOldRow       = NULL;
        m_bAfterLast    = !(m_bBeforeFirst = m_pCache->isBeforeFirst());
    }
    fireRowcount();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!rows)
        return sal_True; // in this case do nothing

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if((m_bAfterLast && rows > 1) || (m_bBeforeFirst && rows < 0))
        return sal_False;// we are already behind the last row or before the first

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

    m_bAfterLast = m_bBeforeFirst = sal_False; // all false
//  if(m_bBeforeFirst)
//      m_pCache->beforeFirst();
//  else if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    notifyAllListenersCursorBeforeMove();
    sal_Bool bRet = m_pCache->relative(rows);
    if(bRet)
    {
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);

        OSL_ENSURE(m_aCurrentRow->isValid(),"Currentrow isn't valid");
        notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);

        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_aCurrentRow   = m_pCache->getEnd();
        m_aCurrentRow.setBookmark(Any());
        m_aOldRow       = NULL;
        m_bAfterLast    = !(m_bBeforeFirst = m_pCache->isBeforeFirst());
    }

    fireRowcount();
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::previous(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    if(m_bBeforeFirst)
        return sal_False;

    ::osl::MutexGuard aGuard( m_rMutex );

    // check if we are inserting a row
    sal_Bool bWasNew = m_pCache->m_bInserted || m_pCache->m_bDeleted;
    checkInsert();

    m_bAfterLast = m_bBeforeFirst = sal_False; // all false
    // move the cache back to right position
//  if(m_bAfterLast)
//      m_pCache->afterLast();
//  else if(m_aBookmark.hasValue())
//      m_pCache->moveToBookmark(m_aBookmark);

    ORowSetMatrix::iterator aOldValues = NULL;
    if(!bWasNew && m_aOldRow.isValid())
        aOldValues = &m_aOldRow;     // remember the old values

    notifyAllListenersCursorBeforeMove();
    sal_Bool bRet = m_pCache->previous();
    if(!(m_bBeforeFirst = m_pCache->isBeforeFirst()) && bRet)
    {
        m_aBookmark     = m_pCache->getBookmark();
        m_aCurrentRow   = m_pCache->m_aMatrixIter;
        m_aCurrentRow.setBookmark(m_aBookmark);
        notifyAllListenersCursorMoved();
        firePropertyChange(aOldValues);

        m_aOldRow       = (*m_aCurrentRow);
    }
    else
    {
        m_aCurrentRow= m_pCache->getEnd();
        m_aCurrentRow.setBookmark(Any());
        m_aOldRow       = NULL;
        m_bAfterLast    = !m_bBeforeFirst;
    }
    return bRet;
}
//------------------------------------------------------------------------------
Reference< XInterface >  ORowSetBase::getStatement(void) throw( SQLException, RuntimeException )
{
    return NULL;
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::refreshRow(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    if(m_aBookmark.hasValue())
        m_pCache->moveToBookmark(m_aBookmark);
    m_pCache->refreshRow();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    return m_pCache->rowUpdated();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowInserted(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    return m_pCache->rowInserted();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetBase::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        throw FunctionSequenceException(*m_pMySelf);

    ::osl::MutexGuard aGuard( m_rMutex );
    return m_pCache->rowDeleted();
}
// -------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL ORowSetBase::getWarnings(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pCache)
        return Any();

    ::osl::MutexGuard aGuard( m_rMutex );

    return m_pCache->getWarnings();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetBase::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    if (m_rBHelper.bDisposed)
        throw DisposedException();

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


}   // namespace dbaccess
