/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_connectivity.hxx"
#include "connectivity/sdbcx/VIndex.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/sdbcx/VColumn.hxx"
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>
#include "connectivity/sdbcx/VCollection.hxx"
#include "TConnection.hxx"
// -------------------------------------------------------------------------
using namespace ::connectivity;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::connectivity::sdbcx;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OIndex::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VIndexDescriptor"));
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VIndex"));
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OIndex::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.IndexDescriptor"));
    else
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Index"));

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OIndex::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OIndex::OIndex(sal_Bool _bCase) :   ODescriptor_BASE(m_aMutex)
                ,   ODescriptor(ODescriptor_BASE::rBHelper,_bCase,sal_True)
                ,m_IsUnique(sal_False)
                ,m_IsPrimaryKeyIndex(sal_False)
                ,m_IsClustered(sal_False)
                ,m_pColumns(NULL)
{
}
// -------------------------------------------------------------------------
OIndex::OIndex( const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Catalog,
                sal_Bool _isUnique,
                sal_Bool _isPrimaryKeyIndex,
                sal_Bool _isClustered,
                sal_Bool _bCase) :  ODescriptor_BASE(m_aMutex)
                        ,ODescriptor(ODescriptor_BASE::rBHelper,_bCase)
                        ,m_Catalog(_Catalog)
                        ,m_IsUnique(_isUnique)
                        ,m_IsPrimaryKeyIndex(_isPrimaryKeyIndex)
                        ,m_IsClustered(_isClustered)
                        ,m_pColumns(NULL)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OIndex::~OIndex( )
{
    delete m_pColumns;
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OIndex::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OIndex::getInfoHelper()
{
    return *OIndex_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
Any SAL_CALL OIndex::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OIndex_BASE::queryInterface(rType);
        if(!aRet.hasValue())
            aRet = ODescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OIndex::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes(),OIndex_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OIndex::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOG),         PROPERTY_ID_CATALOG,            nAttrib,&m_Catalog,         ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE),            PROPERTY_ID_ISUNIQUE,           nAttrib,&m_IsUnique,            ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISPRIMARYKEYINDEX),PROPERTY_ID_ISPRIMARYKEYINDEX, nAttrib,&m_IsPrimaryKeyIndex,   ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCLUSTERED),     PROPERTY_ID_ISCLUSTERED,        nAttrib,&m_IsClustered,     ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
void OIndex::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pColumns)
        m_pColumns->disposing();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OIndex::getColumns(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if  ( !m_pColumns )
            refreshColumns();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        OSL_ENSURE( false, "OIndex::getColumns: caught an exception!" );
    }

    return const_cast<OIndex*>(this)->m_pColumns;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OIndex::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);


    return this;
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OIndex::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OIndex::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OIndex::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OIndex::acquire() throw()
{
    ODescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OIndex::release() throw()
{
    ODescriptor_BASE::release();
}
// -----------------------------------------------------------------------------
void OIndex::refreshColumns()
{
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
