/*************************************************************************
 *
 *  $RCSfile: FDatabaseMetaDataResultSet.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 14:01:50 $
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


#ifndef _CONNECTIVITY_FDATABASEMETADATARESULTSET_HXX_
#include "FDatabaseMetaDataResultSet.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_DATABASEMETADATARESULTSETMETADATA_HXX_
#include "FDatabaseMetaDataResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_PROCEDURERESULT_HPP_
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity;
using namespace connectivity::dbtools;
using namespace cppu;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet()
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(ODatabaseMetaDataResultSet_BASE::rBHelper)
    ,m_aStatement(NULL)
    ,m_xMetaData(NULL)
    ,m_nRowPos(0)
    ,m_bBOF(sal_True)
{
    construct();
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::construct()
{
    registerProperty(PROPERTY_FETCHSIZE,            PROPERTY_ID_FETCHSIZE,          0,&m_nFetchSize,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_RESULTSETTYPE,        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::READONLY,&m_nResultSetType,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_FETCHDIRECTION,       PROPERTY_ID_FETCHDIRECTION,     0,  &m_nFetchDirection, ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(PROPERTY_RESULTSETCONCURRENCY, PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::READONLY,&m_nResultSetConcurrency,                ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_aStatement    = NULL;
    m_xMetaData     = NULL;
    m_aRowsIter = m_aRows.end();
    m_aRows.clear();
    m_aRowsIter = m_aRows.end();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::release() throw(::com::sun::star::uno::RuntimeException)
{
    ODatabaseMetaDataResultSet_BASE::release();
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed )
        throw DisposedException();

    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) : columnName.equalsIgnoreCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());

    m_nColPos = columnIndex;
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return sal_False;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return 0;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return Sequence< sal_Int8 >();

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return ::com::sun::star::util::Date(0,0,0);

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return 0.0;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return 0.0;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return 0;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_nRowPos;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
                throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return sal_Int64();

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData(this);

    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
                throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return NULL;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return Any();

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
                throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;
    return (*m_aRowsIter)[columnIndex].makeAny();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return 0;

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return ::rtl::OUString();

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());


    m_nColPos = columnIndex;

    return (*m_aRowsIter)[columnIndex];
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(m_aRowsIter == m_aRows.end())
        return ::com::sun::star::util::Time(0,0,0,0);

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());
    m_nColPos = columnIndex;

;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return ::com::sun::star::util::DateTime(0,0,0,0,0,0,0);

    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString(),0,Any());

    m_nColPos = columnIndex;
;

    return (*m_aRowsIter)[columnIndex];
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
                        throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    throw ::dbtools::FunctionSequenceException(*this);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_bBOF)
    {
       m_aRowsIter = m_aRows.begin();
       m_bBOF = sal_False;
       return m_aRowsIter != m_aRows.end();
    }

    if(m_aRowsIter == m_aRows.end())
        return sal_False;

    ++m_aRowsIter;

    return m_aRowsIter != m_aRows.end();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_aRowsIter == m_aRows.end())
        return sal_True;

    return (*m_aRowsIter)[m_nColPos].isNull();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *const_cast<ODatabaseMetaDataResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProceduresMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setCatalogsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setSchemasMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setSchemasMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnsMap()
{

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablesMap()
{

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProcedureColumnsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setIndexInfoMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCrossReferenceMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setVersionColumnsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setVersionColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setBestRowIdentifierMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setBestRowIdentifierMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTypeInfoMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTableTypes()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setTableTypes();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setExportedKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setExportedKeysMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setImportedKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(this);
    pMetaData->setImportedKeysMap();
    m_xMetaData = pMetaData;
}
// -----------------------------------------------------------------------------


