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


#include "connectivity/sdbcx/VKey.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <comphelper/sequence.hxx>
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/sdbcx/VCollection.hxx"
#include "TConnection.hxx"
// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKey::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VKeyDescription"));
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VKey"));
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OKey::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.KeyDescription"));
    else
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Key"));

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKey::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OKey::OKey(sal_Bool _bCase) :   ODescriptor_BASE(m_aMutex)
            ,   ODescriptor(ODescriptor_BASE::rBHelper,_bCase,sal_True)
            ,   m_aProps(new KeyProperties())
            ,   m_pColumns(NULL)
{
}
// -------------------------------------------------------------------------
OKey::OKey(const ::rtl::OUString& _Name,const TKeyProperties& _rProps,sal_Bool _bCase)
: ODescriptor_BASE(m_aMutex)
 ,ODescriptor(ODescriptor_BASE::rBHelper,_bCase)
 ,m_aProps(_rProps)
 ,m_pColumns(NULL)
{
    m_Name = _Name;
}
//OKey::OKey(   const ::rtl::OUString& _Name,
//          const ::rtl::OUString& _ReferencedTable,
//          sal_Int32       _Type,
//          sal_Int32       _UpdateRule,
//          sal_Int32       _DeleteRule,
//          sal_Bool _bCase) :  ODescriptor_BASE(m_aMutex)
//                      ,ODescriptor(ODescriptor_BASE::rBHelper,_bCase)
//                      ,m_ReferencedTable(_ReferencedTable)
//                      ,m_Type(_Type)
//                      ,m_UpdateRule(_UpdateRule)
//                      ,m_DeleteRule(_DeleteRule)
//                        ,m_pColumns(NULL)
//{
//  m_Name = _Name;
//}
// -------------------------------------------------------------------------
OKey::~OKey( )
{
    delete m_pColumns;
}
// -------------------------------------------------------------------------
Any SAL_CALL OKey::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OKey_BASE::queryInterface(rType);
        if(!aRet.hasValue())
            aRet = ODescriptor_BASE::queryInterface( rType);
    }

    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OKey::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes());

    return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes(),OKey_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OKey::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE), PROPERTY_ID_REFERENCEDTABLE,    nAttrib,&m_aProps->m_ReferencedTable,   ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,               nAttrib,&m_aProps->m_Type,          ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_UPDATERULE),      PROPERTY_ID_UPDATERULE,         nAttrib,&m_aProps->m_UpdateRule,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE),      PROPERTY_ID_DELETERULE,         nAttrib,&m_aProps->m_DeleteRule,        ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
}
// -------------------------------------------------------------------------
void SAL_CALL OKey::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pColumns)
        m_pColumns->disposing();

    ODescriptor_BASE::disposing();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OKey::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OKey::getInfoHelper()
{
    return *const_cast<OKey*>(this)->getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OKey::getColumns(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pColumns )
            refreshColumns();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OKey*>(this)->m_pColumns;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OKey::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);


    return this;
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OKey::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKey::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OKey::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OKey::acquire() throw()
{
    ODescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OKey::release() throw()
{
    ODescriptor_BASE::release();
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
