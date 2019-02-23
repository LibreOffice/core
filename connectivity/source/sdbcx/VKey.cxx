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

#include <connectivity/sdbcx/VKey.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <TConnection.hxx>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


OUString SAL_CALL OKey::getImplementationName(  )
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VKeyDescriptor");
    return OUString("com.sun.star.sdbcx.VKey");
}

css::uno::Sequence< OUString > SAL_CALL OKey::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = "com.sun.star.sdbcx.KeyDescriptor";
    else
        aSupported[0] = "com.sun.star.sdbcx.Key";

    return aSupported;
}

sal_Bool SAL_CALL OKey::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OKey::OKey(bool _bCase) :   ODescriptor_BASE(m_aMutex)
            ,   ODescriptor(ODescriptor_BASE::rBHelper, _bCase, true)
            ,   m_aProps(new KeyProperties())
{
}

OKey::OKey(const OUString& Name,const std::shared_ptr<KeyProperties>& _rProps, bool _bCase)
: ODescriptor_BASE(m_aMutex)
 ,ODescriptor(ODescriptor_BASE::rBHelper, _bCase)
 ,m_aProps(_rProps)
{
    m_Name = Name;
}
//OKey::OKey(   const OUString& _Name,
//          const OUString& _ReferencedTable,
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

OKey::~OKey( )
{
}

Any SAL_CALL OKey::queryInterface( const Type & rType )
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

Sequence< Type > SAL_CALL OKey::getTypes(  )
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes());

    return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes(),OKey_BASE::getTypes());
}

void OKey::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE), PROPERTY_ID_REFERENCEDTABLE,    nAttrib,&m_aProps->m_ReferencedTable,   ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,               nAttrib,&m_aProps->m_Type,          ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_UPDATERULE),      PROPERTY_ID_UPDATERULE,         nAttrib,&m_aProps->m_UpdateRule,        ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE),      PROPERTY_ID_DELETERULE,         nAttrib,&m_aProps->m_DeleteRule,        ::cppu::UnoType<sal_Int32>::get());
}

void SAL_CALL OKey::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pColumns)
        m_pColumns->disposing();

    ODescriptor_BASE::disposing();
}

::cppu::IPropertyArrayHelper* OKey::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & OKey::getInfoHelper()
{
    return *getArrayHelper(isNew() ? 1 : 0);
}

Reference< css::container::XNameAccess > SAL_CALL OKey::getColumns(  )
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

    return m_pColumns.get();
}

Reference< XPropertySet > SAL_CALL OKey::createDataDescriptor(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);


    return this;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OKey::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OUString SAL_CALL OKey::getName(  )
{
    return m_Name;
}

void SAL_CALL OKey::setName( const OUString& /*aName*/ )
{
}

// XInterface
void SAL_CALL OKey::acquire() throw()
{
    ODescriptor_BASE::acquire();
}

void SAL_CALL OKey::release() throw()
{
    ODescriptor_BASE::release();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
