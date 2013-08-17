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

#include "ContainerMediator.hxx"
#include "apitools.hxx"
#include "column.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "dbastrings.hrc"
#include "sdbcoretools.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <comphelper/basicio.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/TTableHelper.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <algorithm>

using namespace dbaccess;
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

// OColumn
OColumn::OColumn( const bool _bNameIsReadOnly )
        :OColumnBase( m_aMutex )
        ,::comphelper::OPropertyContainer( OColumnBase::rBHelper )
{
    DBG_CTOR(OColumn, NULL);

    registerProperty( PROPERTY_NAME, PROPERTY_ID_NAME, _bNameIsReadOnly ? PropertyAttribute::READONLY : 0,
        &m_sName, ::getCppuType( &m_sName ) );
}

OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(
        OColumnBase::getTypes(),
        ::comphelper::OPropertyContainer::getTypes()
    );
}

// com::sun::star::uno::XInterface
IMPLEMENT_FORWARD_XINTERFACE2( OColumn, OColumnBase, ::comphelper::OPropertyContainer )

// ::com::sun::star::lang::XServiceInfo
OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.sdb.OColumn");
}

sal_Bool OColumn::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    return aSNS;
}

// OComponentHelper
void OColumn::disposing()
{
    OPropertyContainer::disposing();
}

// com::sun::star::beans::XPropertySet
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

OUString SAL_CALL OColumn::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_sName;
}

void SAL_CALL OColumn::setName( const OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    m_sName = _rName;
}

void OColumn::fireValueChange(const ::connectivity::ORowSetValue& /*_rOldValue*/)
{
    OSL_FAIL( "OColumn::fireValueChange: not implemented!" );
}

void OColumn::registerProperty( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, void* _pPointerToMember, const Type& _rMemberType )
{
    ::comphelper::OPropertyContainer::registerProperty( _rName, _nHandle, _nAttributes, _pPointerToMember, _rMemberType );
}

void OColumn::registerMayBeVoidProperty( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, Any* _pPointerToMember, const Type& _rExpectedType )
{
    ::comphelper::OPropertyContainer::registerMayBeVoidProperty( _rName, _nHandle, _nAttributes, _pPointerToMember, _rExpectedType );
}

void OColumn::registerPropertyNoMember( const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes, const Type& _rType, const void* _pInitialValue )
{
    ::comphelper::OPropertyContainer::registerPropertyNoMember( _rName, _nHandle, _nAttributes, _rType, _pInitialValue );
}

// OColumns
DBG_NAME(OColumns);

OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< OUString> &_rVector,
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

OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
        sal_Bool _bCaseSensitive,const ::std::vector< OUString> &_rVector,
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

OColumns::~OColumns()
{
    DBG_DTOR(OColumns, NULL);
}

// XServiceInfo
OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.sdb.OColumns");
}

sal_Bool OColumns::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

Sequence< OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    return aSNS;
}

void OColumns::append( const OUString& _rName, OColumn* _pColumn )
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSURE( _pColumn, "OColumns::append: invalid column!" );
    OSL_ENSURE( !m_pElements->exists( _rName ),"OColumns::append: Column already exists");

    _pColumn->m_sName = _rName;

    // now really insert the column
    insertElement( _rName, _pColumn );
}

void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    disposing();
}

void SAL_CALL OColumns::disposing(void)
{
    MutexGuard aGuard(m_rMutex);
    m_xDrvColumns = NULL;
    m_pMediator = NULL;
    m_pColFactoryImpl = NULL;
    OColumns_BASE::disposing();
}

void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    if (m_pRefreshColumns)
        m_pRefreshColumns->refreshColumns();
}

connectivity::sdbcx::ObjectType OColumns::createObject(const OUString& _rName)
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

// XAppend
sdbcx::ObjectType OColumns::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
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
        {
            Reference< ::com::sun::star::sdb::tools::XTableAlteration> xAlterService = m_pTable->getAlterService();
            if ( xAlterService.is() )
            {
                xAlterService->addColumn(m_pTable,descriptor);
                xReturn = createObject( _rForName );
            }
            else
                xReturn = OColumns_BASE::appendObject( _rForName, descriptor );
        }
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

// XDrop
void OColumns::dropObject(sal_Int32 _nPos,const OUString _sElementName)
{
    Reference< XDrop > xDrop( m_xDrvColumns, UNO_QUERY );
    if ( xDrop.is() )
    {
        xDrop->dropByName( _sElementName );
    }
    else if ( m_pTable && !m_pTable->isNew() )
    {
        if ( m_bDropColumn )
        {
            Reference< ::com::sun::star::sdb::tools::XTableAlteration> xAlterService = m_pTable->getAlterService();
            if ( xAlterService.is() )
                xAlterService->dropColumn(m_pTable,_sElementName);
            else
                OColumns_BASE::dropObject(_nPos,_sElementName);
        }
        else
            ::dbtools::throwGenericSQLException( DBA_RES( RID_STR_NO_COLUMN_DROP ), static_cast<XChild*>(static_cast<TXChild*>(this)) );
    }

    if ( m_pColFactoryImpl )
        m_pColFactoryImpl->columnDropped(_sElementName);

    ::dbaccess::notifyDataSourceModified(m_xParent,sal_True);
}

Reference< XInterface > SAL_CALL OColumns::getParent(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_xParent;
}

void SAL_CALL OColumns::setParent( const Reference< XInterface >& _xParent ) throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    m_xParent = _xParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
