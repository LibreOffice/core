/*************************************************************************
 *
 *  $RCSfile: AResultSet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 11:23:39 $
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


#ifndef _CONNECTIVITY_ADO_ARESULTSET_HXX_
#include "ado/AResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ARESULTSETMETADATA_HXX_
#include "ado/AResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include <oledb.h>

#define CHECK_RETURN(x)                                                 \
    if(!SUCCEEDED(x))                                                               \
        ADOS::ThrowException(*m_pStmt->m_pConnection->getConnection(),*this);

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.AResultSet","com.sun.star.sdbc.ResultSet");
::rtl::OUString SAL_CALL OResultSet::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)  \
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ado.ResultSet");
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OResultSet::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
    aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
OResultSet::OResultSet(ADORecordset* _pRecordSet,OStatement_Base* pStmt) :  OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_aStatement((::cppu::OWeakObject*)pStmt)
                        ,m_pStmt(pStmt)
                        ,m_nRowPos(0)
                        ,m_xMetaData(NULL)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_bEOF(sal_False)
{
    osl_incrementInterlockedCount( &m_refCount );
    OSL_ENSHURE(_pRecordSet,"No RecordSet !");
    if(!_pRecordSet)
        throw SQLException();
    m_pRecordSet->AddRef();
    VARIANT_BOOL bIsAtBOF;
    CHECK_RETURN(m_pRecordSet->get_BOF(&bIsAtBOF))
    m_bOnFirstAfterOpen = !(sal_Bool)bIsAtBOF;
    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
OResultSet::OResultSet(ADORecordset* _pRecordSet) : OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_aStatement(NULL)
                        ,m_xMetaData(NULL)
                        ,m_pRecordSet(_pRecordSet)
                        ,m_bEOF(sal_False)
{
    osl_incrementInterlockedCount( &m_refCount );
    OSL_ENSHURE(_pRecordSet,"No RecordSet !");
    if(!_pRecordSet)
        throw SQLException();
    m_pRecordSet->AddRef();
    VARIANT_BOOL bIsAtBOF;
    CHECK_RETURN(m_pRecordSet->get_BOF(&bIsAtBOF))
    m_bOnFirstAfterOpen = !(sal_Bool)bIsAtBOF;
    osl_decrementInterlockedCount( &m_refCount );
    //  allocBuffer();
}

// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    if(m_pRecordSet)
        m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pRecordSet)
        m_pRecordSet->Close();
    m_aStatement    = NULL;
    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
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
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    PositionEnum aPos;
    m_pRecordSet->get_AbsolutePosition(&aPos);
    return  (aPos > 0) ? aPos : m_nRowPos;
    // return the rowcount from driver if the driver doesn't support this return our count
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    return sal_Int64(0);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pRecordSet);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{

    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    m_aValue = aField.get_Value();
    return m_aValue;
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADO_GETFIELD(columnIndex);
    aField.get_Value(m_aValue);
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int16 bIsAtEOF;
    CHECK_RETURN(m_pRecordSet->get_EOF(&bIsAtEOF))
    return bIsAtEOF;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return sal_True;
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(first())
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OResultSet_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(SUCCEEDED(m_pRecordSet->MoveFirst()))
    {
        m_nRowPos = 1;
        return sal_True;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MoveLast());
    if(bRet)
        m_pRecordSet->get_RecordCount(&m_nRowPos);
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!row)                 // absolute with zero not allowed
        throw SQLException();

    sal_Bool bCheck = sal_True;
    if(row < 0)
    {
        if(bCheck = SUCCEEDED(m_pRecordSet->MoveLast()))
        {
            while(++row < 0 && bCheck)
                bCheck = m_pRecordSet->MovePrevious();
        }
    }
    else
    {
        first();
        OLEVariant aEmpty;
        aEmpty.setNoArg();
        bCheck = SUCCEEDED(m_pRecordSet->Move(row-1,aEmpty)); // move to row -1 because we stand already on the first
        if(bCheck)
            m_nRowPos = row;
    }
    return bCheck;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    sal_Bool bRet = SUCCEEDED(m_pRecordSet->Move(row,aEmpty));
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Bool bRet = SUCCEEDED(m_pRecordSet->MovePrevious());
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    sal_Bool  bRet = (eRec & adRecDeleted) == adRecDeleted;
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    sal_Bool  bRet = (eRec & adRecNew) == adRecNew;
    if(bRet)
        ++m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    RecordStatusEnum eRec;
    m_pRecordSet->get_Status((sal_Int32*)&eRec);
    return (eRec & adRecModified) == adRecModified;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OSL_ENSHURE(!m_nRowPos,"OResultSet::isBeforeFirst: Error in setting m_nRowPos!");
    sal_Int16 bIsAtBOF;
    m_pRecordSet->get_BOF(&bIsAtBOF);
    return bIsAtBOF;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Bool bRet = sal_True;
    if(m_bOnFirstAfterOpen)
    {
        m_bOnFirstAfterOpen = sal_False;
        ++m_nRowPos;
    }
    else
    {
        bRet = SUCCEEDED(m_pRecordSet->MoveNext());

        if(bRet)
        {
            VARIANT_BOOL bIsAtEOF;
            CHECK_RETURN(m_pRecordSet->get_EOF(&bIsAtEOF))
            bRet = !(sal_Bool)bIsAtEOF;
            ++m_nRowPos;
        }
        else
            ADOS::ThrowException(*m_pStmt->m_pConnection->getConnection(),*this);
    }

    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_aValue.isNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pRecordSet->Cancel();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->AddNew(aEmpty,aEmpty);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OLEVariant aEmpty;
    aEmpty.setNoArg();
    m_pRecordSet->Update(aEmpty,aEmpty);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pRecordSet->Delete(adAffectCurrent);
    m_pRecordSet->UpdateBatch(adAffectCurrent);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pRecordSet->CancelUpdate();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    OLEVariant x;
    x.setNull();
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();


    ADO_GETFIELD(columnIndex);
    aField.PutValue(x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_pRecordSet->Resync(adAffectCurrent,adResyncAllValues);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{

}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{

    OSL_ENSHURE(0,"OResultSet::updateNumericObject: NYI");
}
//------------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_nRowPos < m_aBookmarks.size()) // this bookmark was already fetched
        return makeAny(sal_Int32(m_nRowPos-1));

    OLEVariant aVar;
    m_pRecordSet->get_Bookmark(&aVar);
    m_aBookmarks.push_back(aVar);
    return makeAny((sal_Int32)(m_aBookmarks.size()-1));

}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveToBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nPos;
    bookmark >>= nPos;
    OSL_ENSHURE(nPos >= 0 && nPos < m_aBookmarks.size(),"Invalid Index for vector");
    if(nPos < 0 || nPos >= m_aBookmarks.size())
        throw SQLException();

    return SUCCEEDED(m_pRecordSet->Move(0,m_aBookmarks[nPos]));
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const Any& bookmark, sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nPos;
    bookmark >>= nPos;
    nPos += rows;
    OSL_ENSHURE(nPos >= 0 && nPos < m_aBookmarks.size(),"Invalid Index for vector");
    if(nPos < 0 || nPos >= m_aBookmarks.size())
        throw SQLException();
    return SUCCEEDED(m_pRecordSet->Move(rows,m_aBookmarks[nPos]));
}
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::compareBookmarks( const Any& first, const Any& second ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nPos1;
    first >>= nPos1;
    sal_Int32 nPos2;
    second >>= nPos2;
    if(nPos1 == nPos2)  // they should be equal
        return sal_True;

    OSL_ENSHURE((nPos1 >= 0 && nPos1 < m_aBookmarks.size()) || (nPos1 >= 0 && nPos2 < m_aBookmarks.size()),"Invalid Index for vector");

    CompareEnum eNum;
    m_pRecordSet->CompareBookmarks(m_aBookmarks[nPos1],m_aBookmarks[nPos2],&eNum);
    return ((sal_Int32)eNum) +1;
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADOProperties* pProps = NULL;
    m_pRecordSet->get_Properties(&pProps);
    WpOLEAppendCollection<ADOProperties, ADOProperty, WpADOProperty> aProps(pProps);
    ADOS::ThrowException(*((OConnection*)m_pStmt->getConnection().get())->getConnection(),*this);
    OSL_ENSHURE(aProps.IsValid(),"There are no properties at the connection");

    WpADOProperty aProp(aProps.GetItem(::rtl::OUString::createFromAscii("Bookmarks Ordered")));
    OLEVariant aVar;
    if(aProp.IsValid())
        aVar = aProp.GetValue();
    else
        ADOS::ThrowException(*((OConnection*)m_pStmt->getConnection().get())->getConnection(),*this);

    sal_Bool bValue(sal_False);
    if(!aVar.isNull() && !aVar.isEmpty())
        bValue = aVar;
    return bValue;
}
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::hashBookmark( const Any& bookmark ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nPos;
    bookmark >>= nPos;
    return nPos;
}
//------------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const Sequence< Any >& rows ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OLEVariant aVar;
    sal_Int32 nPos;

    // Create SafeArray Bounds and initialize the array
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound   = 0;
    rgsabound[0].cElements = rows.getLength();
    SAFEARRAY *psa         = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    const Any* pBegin = rows.getConstArray();
    const Any* pEnd = pBegin + rows.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin,++i)
    {
        *pBegin >>= nPos;
        SafeArrayPutElement(psa,&i,&m_aBookmarks[nPos]);
    }

    // Initialize and fill the SafeArray
    OLEVariant vsa;
    vsa.setArray(psa,VT_VARIANT);

    m_pRecordSet->put_Filter(vsa);
    m_pRecordSet->Delete(adAffectGroup);
    m_pRecordSet->UpdateBatch(adAffectGroup);

    Sequence< sal_Int32 > aSeq(rows.getLength());
    if(first())
    {
        sal_Int32* pSeq = aSeq.getArray();
        sal_Int32 i=0;
        do
        {
            OSL_ENSHURE(i<aSeq.getLength(),"Index greater than length of sequence");
            m_pRecordSet->get_Status(&pSeq[i]);
            if(pSeq[i++] == adRecDeleted)
                --m_nRowPos;
        }
        while(next());
    }
    return aSeq;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetConcurrency() const
{
    sal_Int32 nValue=0;
    LockTypeEnum eRet;
    if(!SUCCEEDED(m_pRecordSet->get_LockType(&eRet)))
    {
        switch(eRet)
        {
            case adLockReadOnly:
                nValue = ResultSetConcurrency::READ_ONLY;
                break;
            default:
                nValue = ResultSetConcurrency::UPDATABLE;
                break;
        }
    }
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetType() const
{
    sal_Int32 nValue=0;
    CursorTypeEnum eRet;
    if(!SUCCEEDED(m_pRecordSet->get_CursorType(&eRet)))
    {
        switch(eRet)
        {
            case adOpenUnspecified:
            case adOpenForwardOnly:
                nValue = ResultSetType::FORWARD_ONLY;
                break;
            case adOpenStatic:
            case adOpenKeyset:
                nValue = ResultSetType::SCROLL_INSENSITIVE;
                break;
            case adOpenDynamic:
                nValue = ResultSetType::SCROLL_SENSITIVE;
                break;
        }
    }
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchDirection() const
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchSize() const
{
    sal_Int32 nValue=-1;
    m_pRecordSet->get_CacheSize(&nValue);
    return nValue;
}
//------------------------------------------------------------------------------
::rtl::OUString OResultSet::getCursorName() const
{
    return ::rtl::OUString();
}

//------------------------------------------------------------------------------
void OResultSet::setFetchDirection(sal_Int32 _par0)
{
}
//------------------------------------------------------------------------------
void OResultSet::setFetchSize(sal_Int32 _par0)
{
    m_pRecordSet->put_CacheSize(_par0);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;

    //  DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
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
void OResultSet::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(getINT32(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OResultSet::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            {
                VARIANT_BOOL bBool;
                m_pRecordSet->Supports(adBookmark,&bBool);
                sal_Bool bRet = bBool == VARIANT_TRUE;
                rValue.setValue(&bRet, ::getCppuBooleanType() );
            }
            break;
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


