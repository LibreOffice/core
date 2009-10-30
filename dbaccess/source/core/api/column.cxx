/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: column.cxx,v $
 * $Revision: 1.55 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include <connectivity/TTableHelper.hxx>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif

#include <algorithm>

using namespace dbaccess;
using namespace connectivity;
using namespace connectivity;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

DBG_NAME(OColumn)

//============================================================
//= OColumn
//============================================================
//--------------------------------------------------------------------------
OColumn::OColumn()
        :OColumnBase(m_aMutex)
        , OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
}

//--------------------------------------------------------------------------
OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                           OColumnBase::getTypes());
    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OColumn::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OColumnBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XMultiPropertySet * >( this ));

    return aIface;
}

//--------------------------------------------------------------------------
void OColumn::acquire() throw()
{
    OColumnBase::acquire();
}

//--------------------------------------------------------------------------
void OColumn::release() throw()
{
    OColumnBase::release();
}

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumn");
}

//------------------------------------------------------------------------------
sal_Bool OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OColumn::disposing()
{
    OPropertySetHelper::disposing();
}

// com::sun::star::beans::XPropertySet
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
void OColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumn::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            OSL_ENSURE(rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(NAME) : invalid value !");
            rValue >>= m_sName;
            break;
    }
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OColumn::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OColumn::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_sName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::setName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    m_sName = _rName;
}
// -----------------------------------------------------------------------------
void OColumn::fireValueChange(const ::connectivity::ORowSetValue& /*_rOldValue*/)
{
    DBG_ERROR( "OColumn::fireValueChange: not implemented!" );
}
//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
                   IColumnFactory* _pColFactory,
                   ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                   sal_Bool _bAddColumn,
                   sal_Bool _bDropColumn,
                   sal_Bool _bUseHardRef)
                   : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector,_bUseHardRef)
    ,m_pMediator(NULL)
    ,m_xDrvColumns(NULL)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
{
    DBG_CTOR(OColumns, NULL);
}
// -------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
        sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
        IColumnFactory* _pColFactory,
        ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
        sal_Bool _bAddColumn,
        sal_Bool _bDropColumn,
        sal_Bool _bUseHardRef)
       : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector,_bUseHardRef)
    ,m_pMediator(NULL)
    ,m_xDrvColumns(_rxDrvColumns)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
{
    DBG_CTOR(OColumns, NULL);
}
//--------------------------------------------------------------------------
OColumns::~OColumns()
{
    DBG_DTOR(OColumns, NULL);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumns");
}

//------------------------------------------------------------------------------
sal_Bool OColumns::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    return aSNS;
}

//------------------------------------------------------------------
void OColumns::append( const ::rtl::OUString& _rName, OColumn* _pColumn )
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSURE( _pColumn, "OColumns::append: invalid column!" );
    OSL_ENSURE( !m_pElements->exists( _rName ),"OColumns::append: Column already exists");

    _pColumn->m_sName = _rName;

    // now really insert the column
    insertElement( _rName, _pColumn );
}

//------------------------------------------------------------------
void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    disposing();
}

// -----------------------------------------------------------------------------
void SAL_CALL OColumns::disposing(void)
{
    MutexGuard aGuard(m_rMutex);
    m_xDrvColumns = NULL;
    m_pMediator = NULL;
    m_pColFactoryImpl = NULL;
    OColumns_BASE::disposing();
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    if (m_pRefreshColumns)
        m_pRefreshColumns->refreshColumns();
}

// -------------------------------------------------------------------------
connectivity::sdbcx::ObjectType OColumns::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_pColFactoryImpl, "OColumns::createObject: no column factory!");

    connectivity::sdbcx::ObjectType xRet;
    if ( m_pColFactoryImpl )
    {
        xRet = m_pColFactoryImpl->createColumn(_rName);
        Reference<XChild> xChild(xRet,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(static_cast<XChild*>(static_cast<TXChild*>(this)));
    }

    Reference<XPropertySet> xDest(xRet,UNO_QUERY);
    if ( m_pMediator && xDest.is() )
        m_pMediator->notifyElementCreated(_rName,xDest);

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createDescriptor()
{
    if ( m_pColFactoryImpl )
    {
        Reference<XPropertySet> xRet = m_pColFactoryImpl->createColumnDescriptor();
        Reference<XChild> xChild(xRet,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(static_cast<XChild*>(static_cast<TXChild*>(this)));
        return xRet;
    }
    else
        return Reference< XPropertySet >();
}
// -------------------------------------------------------------------------
Any SAL_CALL OColumns::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(m_xDrvColumns.is())
    {
        aRet = m_xDrvColumns->queryInterface(rType);
        if ( aRet.hasValue() )
            aRet = OColumns_BASE::queryInterface( rType);
        if ( !aRet.hasValue() )
            aRet = TXChild::queryInterface( rType);
        return aRet;
    }
    else if(!m_pTable || (m_pTable && !m_pTable->isNew()))
    {
        if(!m_bAddColumn    && rType == getCppuType( (Reference<XAppend>*)0))
            return Any();
        if(!m_bDropColumn   && rType == getCppuType( (Reference<XDrop>*)0))
            return Any();
    }

    aRet = OColumns_BASE::queryInterface( rType);
    if ( !aRet.hasValue() )
        aRet = TXChild::queryInterface( rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumns::getTypes(  ) throw(RuntimeException)
{
    sal_Bool bAppendFound = sal_False,bDropFound = sal_False;

    sal_Int32 nSize = 0;
    Type aAppendType = getCppuType( (Reference<XAppend>*)0);
    Type aDropType   = getCppuType( (Reference<XDrop>*)0);
    if(m_xDrvColumns.is())
    {
        Reference<XTypeProvider> xTypes(m_xDrvColumns,UNO_QUERY);
        Sequence< Type > aTypes(xTypes->getTypes());

        Sequence< Type > aSecTypes(OColumns_BASE::getTypes());


        const Type* pBegin = aTypes.getConstArray();
        const Type* pEnd = pBegin + aTypes.getLength();
        for (;pBegin != pEnd ; ++pBegin)
        {
            if(aAppendType == *pBegin)
                bAppendFound = sal_True;
            else if(aDropType == *pBegin)
                bDropFound = sal_True;
        }
        nSize = (bDropFound ? (bAppendFound ? 0 : 1) : (bAppendFound ? 1 : 2));
    }
    else
    {
        nSize = ((m_pTable && m_pTable->isNew()) ? 0 :
                    ((m_bDropColumn ?
                        (m_bAddColumn ? 0 : 1) : (m_bAddColumn ? 1 : 2))));
        bDropFound      = (m_pTable && m_pTable->isNew()) || m_bDropColumn;
        bAppendFound    = (m_pTable && m_pTable->isNew()) || m_bAddColumn;
    }
    Sequence< Type > aTypes(::comphelper::concatSequences(OColumns_BASE::getTypes(),TXChild::getTypes()));
    Sequence< Type > aRet(aTypes.getLength() - nSize);

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aAppendType && *pBegin != aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bDropFound && *pBegin == aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bAppendFound && *pBegin == aAppendType)
            aRet.getArray()[i++] = *pBegin;
    }
    return aRet;
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    sdbcx::ObjectType xReturn;

    Reference< XAppend > xAppend( m_xDrvColumns, UNO_QUERY );
    if ( xAppend.is() )
    {
        xAppend->appendByDescriptor(descriptor);
        xReturn = createObject( _rForName );
    }
    else if ( m_pTable && !m_pTable->isNew() )
    {
        if ( m_bAddColumn )
            xReturn = OColumns_BASE::appendObject( _rForName, descriptor );
        else
            ::dbtools::throwGenericSQLException( DBA_RES( RID_STR_NO_COLUMN_ADD ), static_cast<XChild*>(static_cast<TXChild*>(this)) );
    }
    else
        xReturn = cloneDescriptor( descriptor );

    if ( m_pColFactoryImpl )
        m_pColFactoryImpl->columnAppended( descriptor );

    ::dbaccess::notifyDataSourceModified(m_xParent,sal_True);

    return xReturn;
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XDrop > xDrop( m_xDrvColumns, UNO_QUERY );
    if ( xDrop.is() )
    {
        xDrop->dropByName( _sElementName );
    }
    else if ( m_pTable && !m_pTable->isNew() )
    {
        if ( m_bDropColumn )
            OColumns_BASE::dropObject(_nPos,_sElementName);
        else
            ::dbtools::throwGenericSQLException( DBA_RES( RID_STR_NO_COLUMN_DROP ), static_cast<XChild*>(static_cast<TXChild*>(this)) );
    }

    if ( m_pColFactoryImpl )
        m_pColFactoryImpl->columnDropped(_sElementName);

    ::dbaccess::notifyDataSourceModified(m_xParent,sal_True);
}
// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL OColumns::getParent(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumns::setParent( const Reference< XInterface >& _xParent ) throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    m_xParent = _xParent;
}
// -----------------------------------------------------------------------------
