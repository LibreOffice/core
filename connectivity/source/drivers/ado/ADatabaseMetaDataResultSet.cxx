/*************************************************************************
 *
 *  $RCSfile: ADatabaseMetaDataResultSet.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-24 16:11:26 $
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


#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSET_HXX_
#include "ado/ADatabaseMetaDataResultSet.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE ado
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSETMETADATA_HXX_
#include "ado/ADatabaseMetaDataResultSetMetaData.hxx"
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
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#include <oledb.h>


using namespace connectivity::ado;
using namespace cppu;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(ADORecordset* _pRecordSet) : ODatabaseMetaDataResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)
                        ,m_aStatement(NULL)
                        ,m_xMetaData(NULL)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_bEOF(sal_False)
{
    osl_incrementInterlockedCount( &m_refCount );
    m_aColMapping.push_back(-1);
    if(_pRecordSet)
    {
        m_pRecordSet->AddRef();
        VARIANT_BOOL bIsAtBOF;
        m_pRecordSet->get_BOF(&bIsAtBOF);
        m_bOnFirstAfterOpen = !(sal_Bool)bIsAtBOF;
    }
    else
        m_bOnFirstAfterOpen = sal_False;
    osl_decrementInterlockedCount( &m_refCount );
    //  allocBuffer();
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
    m_aStatement    = NULL;
    m_xMetaData     = NULL;
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
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue); return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (*m_aStrValueRangeIter).second[m_aValue];
    return m_aValue;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (*m_aStrValueRangeIter).second[m_aValue];
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    return 0;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    return sal_Int64(0);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);

    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)m_aValue];
    else if(m_aStrValueRange.size() && (m_aStrValueRangeIter = m_aStrValueRange.find(columnIndex)) != m_aStrValueRange.end())
        return (*m_aStrValueRangeIter).second[m_aValue];
    return m_aValue;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);

    if(m_aIntValueRange.size() && (m_aIntValueRangeIter = m_aIntValueRange.find(columnIndex)) != m_aIntValueRange.end())
        return (*m_aIntValueRangeIter).second[m_aValue];
    return m_aValue;
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue); return m_aValue;
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    columnIndex = mapColumn(columnIndex);
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue); return m_aValue;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    sal_Int16 bIsAtEOF;
    m_pRecordSet->get_EOF(&bIsAtEOF);
    return bIsAtEOF;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    return sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    if(first())
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    if(last())
        next();
    m_bEOF = sal_True;
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
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pRecordSet)
        return sal_False;

    sal_Bool bRet;
    if(bRet = SUCCEEDED(m_pRecordSet->MoveFirst()))
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed )
        throw DisposedException();

    return m_pRecordSet ? SUCCEEDED(m_pRecordSet->MoveLast()) : sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(first())
    {
        OLEVariant aEmpty;
        aEmpty.setNoArg();
        sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
        if(bRet)
            m_nRowPos = row;
        return bRet;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pRecordSet)
        return sal_False;

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pRecordSet)
        return sal_False;

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecDeleted) == adRecDeleted;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecNew) == adRecNew;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecModified) == adRecModified;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pRecordSet)
        return sal_True;

    sal_Int16 bIsAtBOF;
    m_pRecordSet->get_BOF(&bIsAtBOF);
    return bIsAtBOF;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pRecordSet)
        return sal_False;

    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = sal_False;
        return sal_True;
    }
    else
        return SUCCEEDED(m_pRecordSet->MoveNext());
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    return m_aValue.isNull();
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    m_pRecordSet->Resync(adAffectCurrent,adResyncAllValues);
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!m_pRecordSet)
        throw SQLException();

    m_pRecordSet->Cancel();
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
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetConcurrency() const
{
    return ResultSetConcurrency::READ_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetType() const
{
    return ResultSetType::FORWARD_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchDirection() const
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchSize() const
{
    sal_Int32 nValue=-1;
    if(m_pRecordSet)
        m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}
//------------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaDataResultSet::getCursorName() const
{
    return ::rtl::OUString();
}

//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchDirection(sal_Int32 _par0)
{
}
//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchSize(sal_Int32 _par0)
{
    if(m_pRecordSet)
        m_pRecordSet->put_CacheSize(_par0);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,          ::rtl::OUString);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,       sal_Int32);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *const_cast<ODatabaseMetaDataResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception();
            break;
        default:
            OSL_ENSHURE(0,"setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProceduresMap()
{

    for(sal_Int32 i=1;i<4;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(4);

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[DB_PT_UNKNOWN]     = ProcedureResult::UNKNOWN;
    aMap[DB_PT_PROCEDURE]   = ProcedureResult::NONE;
    aMap[DB_PT_FUNCTION]    = ProcedureResult::RETURN;
    m_aValueRange[4] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    m_aColMapping.push_back(1);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setSchemasMap()
{
    m_aColMapping.push_back(2);

    m_xMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{

    m_aColMapping.push_back(3);
    m_aColMapping.push_back(4);
    m_aColMapping.push_back(5);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(9);
    m_aColMapping.push_back(10);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setColumnsMap()
{

    for(sal_Int32 i=1;i<5;++i)
        m_aColMapping.push_back(i);

    m_aColMapping.push_back(12);
    m_aColMapping.push_back(12); // is used as TYPE_NAME

    m_aColMapping.push_back(14);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(18);

    m_aColMapping.push_back(11);
    m_aColMapping.push_back(29);
    m_aColMapping.push_back(9);
    m_aColMapping.push_back(18);
    m_aColMapping.push_back(18);

    m_aColMapping.push_back(15);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(11);

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[DBTYPE_EMPTY] = DataType::SQLNULL;
    aMap[DBTYPE_NULL] = DataType::SQLNULL;
    aMap[DBTYPE_I2] = DataType::SMALLINT;
    aMap[DBTYPE_I4] = DataType::INTEGER;
    aMap[DBTYPE_R4] = DataType::FLOAT;
    aMap[DBTYPE_R8] = DataType::DOUBLE;
    aMap[DBTYPE_CY] = DataType::BIGINT;
    aMap[DBTYPE_DATE] = DataType::DATE;
    aMap[DBTYPE_BSTR] = DataType::VARCHAR;
    aMap[DBTYPE_IDISPATCH] = DataType::OBJECT;
    aMap[DBTYPE_ERROR] = DataType::OTHER;
    aMap[DBTYPE_BOOL] = DataType::BIT;
    aMap[DBTYPE_VARIANT] = DataType::STRUCT;
    aMap[DBTYPE_IUNKNOWN] = DataType::OTHER;
    aMap[DBTYPE_DECIMAL] = DataType::DECIMAL;
    aMap[DBTYPE_UI1] = DataType::TINYINT;
    aMap[DBTYPE_ARRAY] = DataType::ARRAY;
    aMap[DBTYPE_BYREF] = DataType::REF;
    aMap[DBTYPE_I1] = DataType::CHAR;
    aMap[DBTYPE_UI2] = DataType::SMALLINT;
    aMap[DBTYPE_UI4] = DataType::INTEGER;

    // aMap[The] = ;
    // aMap[in] = ;
    aMap[DBTYPE_I8] = DataType::BIGINT;
    aMap[DBTYPE_UI8] = DataType::BIGINT;
    aMap[DBTYPE_GUID] = DataType::OTHER;
    aMap[DBTYPE_VECTOR] = DataType::OTHER;
    aMap[DBTYPE_FILETIME] = DataType::OTHER;
    aMap[DBTYPE_RESERVED] = DataType::OTHER;

    // aMap[The] = ;
    aMap[DBTYPE_BYTES] = DataType::VARBINARY;
    aMap[DBTYPE_STR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_WSTR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_NUMERIC] = DataType::NUMERIC;
    aMap[DBTYPE_UDT] = DataType::OTHER;
    aMap[DBTYPE_DBDATE] = DataType::DATE;
    aMap[DBTYPE_DBTIME] = DataType::TIME;
    aMap[DBTYPE_DBTIMESTAMP] = DataType::TIMESTAMP;
    aMap[DBTYPE_HCHAPTER] = DataType::OTHER;
    aMap[DBTYPE_PROPVARIANT] = DataType::OTHER;
    aMap[DBTYPE_VARNUMERIC] = DataType::NUMERIC;

    m_aValueRange[12] = aMap;

    ::std::map< sal_Int32,::rtl::OUString> aMap2;
    aMap2[0] = ::rtl::OUString::createFromAscii("YES");
    aMap2[1] = ::rtl::OUString::createFromAscii("NO");
    m_aIntValueRange[18] = aMap2;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablesMap()
{

    for(sal_Int32 i=1;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(6);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setProcedureColumnsMap()
{

    for(sal_Int32 i=1;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(10);
    m_aColMapping.push_back(16);
    m_aColMapping.push_back(13);
    m_aColMapping.push_back(11);
    m_aColMapping.push_back(12);

    m_aColMapping.push_back(9);
    m_aColMapping.push_back(14);

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[DBTYPE_EMPTY] = DataType::SQLNULL;
    aMap[DBTYPE_NULL] = DataType::SQLNULL;
    aMap[DBTYPE_I2] = DataType::SMALLINT;
    aMap[DBTYPE_I4] = DataType::INTEGER;
    aMap[DBTYPE_R4] = DataType::FLOAT;
    aMap[DBTYPE_R8] = DataType::DOUBLE;
    aMap[DBTYPE_CY] = DataType::BIGINT;
    aMap[DBTYPE_DATE] = DataType::DATE;
    aMap[DBTYPE_BSTR] = DataType::VARCHAR;
    aMap[DBTYPE_IDISPATCH] = DataType::OBJECT;
    aMap[DBTYPE_ERROR] = DataType::OTHER;
    aMap[DBTYPE_BOOL] = DataType::BIT;
    aMap[DBTYPE_VARIANT] = DataType::STRUCT;
    aMap[DBTYPE_IUNKNOWN] = DataType::OTHER;
    aMap[DBTYPE_DECIMAL] = DataType::DECIMAL;
    aMap[DBTYPE_UI1] = DataType::TINYINT;
    aMap[DBTYPE_ARRAY] = DataType::ARRAY;
    aMap[DBTYPE_BYREF] = DataType::REF;
    aMap[DBTYPE_I1] = DataType::CHAR;
    aMap[DBTYPE_UI2] = DataType::SMALLINT;
    aMap[DBTYPE_UI4] = DataType::INTEGER;

    // aMap[The] = ;
    // aMap[in] = ;
    aMap[DBTYPE_I8] = DataType::BIGINT;
    aMap[DBTYPE_UI8] = DataType::BIGINT;
    aMap[DBTYPE_GUID] = DataType::OTHER;
    aMap[DBTYPE_VECTOR] = DataType::OTHER;
    aMap[DBTYPE_FILETIME] = DataType::OTHER;
    aMap[DBTYPE_RESERVED] = DataType::OTHER;

    // aMap[The] = ;
    aMap[DBTYPE_BYTES] = DataType::VARBINARY;
    aMap[DBTYPE_STR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_WSTR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_NUMERIC] = DataType::NUMERIC;
    aMap[DBTYPE_UDT] = DataType::OTHER;
    aMap[DBTYPE_DBDATE] = DataType::DATE;
    aMap[DBTYPE_DBTIME] = DataType::TIME;
    aMap[DBTYPE_DBTIMESTAMP] = DataType::TIMESTAMP;
    aMap[DBTYPE_HCHAPTER] = DataType::OTHER;
    aMap[DBTYPE_PROPVARIANT] = DataType::OTHER;
    aMap[DBTYPE_VARNUMERIC] = DataType::NUMERIC;

    m_aValueRange[10] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{

    sal_Int32 i=1;
    for(;i<5;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(7);
    m_aColMapping.push_back(8);

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setIndexInfoMap()
{

    sal_Int32 i=1;
    for(;i<4;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(8);
    m_aColMapping.push_back(4);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(10);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(18);
    m_aColMapping.push_back(21);
    m_aColMapping.push_back(22);
    m_aColMapping.push_back(23);
    m_aColMapping.push_back(24);

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[DBPROPVAL_IT_HASH] = IndexType::HASHED;
    aMap[DBPROPVAL_IT_CONTENT] = IndexType::OTHER;
    aMap[DBPROPVAL_IT_OTHER] = IndexType::OTHER;
    aMap[DBPROPVAL_IT_BTREE] = IndexType::OTHER;

    m_aValueRange[10] = aMap;

    ::std::map<sal_Int32,sal_Int32> aMap2;
    aMap[0] = 1;
    aMap[1] = 0;
    m_aValueRange[8] = aMap2;

    ::std::map< sal_Int32,::rtl::OUString> aMap3;
    aMap3[0]                    = ::rtl::OUString();
    aMap3[DB_COLLATION_ASC]     = ::rtl::OUString::createFromAscii("A");
    aMap3[DB_COLLATION_DESC]    = ::rtl::OUString::createFromAscii("D");

    m_aIntValueRange[21] = aMap3;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{

    sal_Int32 i=3;
    for(;i<6;i++)
        m_aColMapping.push_back(i);
    m_aColMapping.push_back(1);
    m_aColMapping.push_back(2);
    m_aColMapping.push_back(6);
    m_aColMapping.push_back(7);

    ::std::map< sal_Int32,::rtl::OUString> aMap;
    aMap[0] = ::rtl::OUString::createFromAscii("YES");
    aMap[1] = ::rtl::OUString::createFromAscii("NO");
    m_aIntValueRange[7] = aMap;


    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setCrossReferenceMap()
{

    sal_Int32 i=1;
    for(;i<5;i++)
        m_aColMapping.push_back(i);
    for(i=7;i<11;i++)
        m_aColMapping.push_back(i);

    m_aColMapping.push_back(13);
    m_aColMapping.push_back(14);
    m_aColMapping.push_back(15);
    m_aColMapping.push_back(17);
    m_aColMapping.push_back(16);
    m_aColMapping.push_back(18);

    ::std::map< ::rtl::OUString,sal_Int32> aMap;
    aMap[ ::rtl::OUString::createFromAscii("CASCADE")] = KeyRule::CASCADE;
    aMap[ ::rtl::OUString::createFromAscii("RESTRICT")] = KeyRule::RESTRICT;
    aMap[ ::rtl::OUString::createFromAscii("SET NULL")] = KeyRule::SET_NULL;
    aMap[ ::rtl::OUString::createFromAscii("SET DEFAULT")] = KeyRule::SET_DEFAULT;
    aMap[ ::rtl::OUString::createFromAscii("NO ACTION")] = KeyRule::NO_ACTION;

    m_aStrValueRange[14] = aMap;
    m_aStrValueRange[15] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setTypeInfoMap()
{
    sal_Int32 i=1;
    for(;i<19;i++)
        m_aColMapping.push_back(i);

    ::std::map< ::rtl::OUString,sal_Int32> aMap1;
    aMap1[ ::rtl::OUString()] = 10;

    m_aStrValueRange[18] = aMap1;

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[DBTYPE_EMPTY] = DataType::SQLNULL;
    aMap[DBTYPE_NULL] = DataType::SQLNULL;
    aMap[DBTYPE_I2] = DataType::SMALLINT;
    aMap[DBTYPE_I4] = DataType::INTEGER;
    aMap[DBTYPE_R4] = DataType::FLOAT;
    aMap[DBTYPE_R8] = DataType::DOUBLE;
    aMap[DBTYPE_CY] = DataType::BIGINT;
    aMap[DBTYPE_DATE] = DataType::DATE;
    aMap[DBTYPE_BSTR] = DataType::VARCHAR;
    aMap[DBTYPE_IDISPATCH] = DataType::OBJECT;
    aMap[DBTYPE_ERROR] = DataType::OTHER;
    aMap[DBTYPE_BOOL] = DataType::BIT;
    aMap[DBTYPE_VARIANT] = DataType::STRUCT;
    aMap[DBTYPE_IUNKNOWN] = DataType::OTHER;
    aMap[DBTYPE_DECIMAL] = DataType::DECIMAL;
    aMap[DBTYPE_UI1] = DataType::TINYINT;
    aMap[DBTYPE_ARRAY] = DataType::ARRAY;
    aMap[DBTYPE_BYREF] = DataType::REF;
    aMap[DBTYPE_I1] = DataType::CHAR;
    aMap[DBTYPE_UI2] = DataType::SMALLINT;
    aMap[DBTYPE_UI4] = DataType::INTEGER;

    // aMap[The] = ;
    // aMap[in] = ;
    aMap[DBTYPE_I8] = DataType::BIGINT;
    aMap[DBTYPE_UI8] = DataType::BIGINT;
    aMap[DBTYPE_GUID] = DataType::OTHER;
    aMap[DBTYPE_VECTOR] = DataType::OTHER;
    aMap[DBTYPE_FILETIME] = DataType::OTHER;
    aMap[DBTYPE_RESERVED] = DataType::OTHER;

    // aMap[The] = ;
    aMap[DBTYPE_BYTES] = DataType::VARBINARY;
    aMap[DBTYPE_STR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_WSTR] = DataType::LONGVARCHAR;
    aMap[DBTYPE_NUMERIC] = DataType::NUMERIC;
    aMap[DBTYPE_UDT] = DataType::OTHER;
    aMap[DBTYPE_DBDATE] = DataType::DATE;
    aMap[DBTYPE_DBTIME] = DataType::TIME;
    aMap[DBTYPE_DBTIMESTAMP] = DataType::TIMESTAMP;
    aMap[DBTYPE_HCHAPTER] = DataType::OTHER;
    aMap[DBTYPE_PROPVARIANT] = DataType::OTHER;
    aMap[DBTYPE_VARNUMERIC] = DataType::NUMERIC;

    m_aValueRange[2] = aMap;

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData(m_pRecordSet,this);
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}

