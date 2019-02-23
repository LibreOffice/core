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

#include <connectivity/sdbcx/VIndex.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <TConnection.hxx>

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


OUString SAL_CALL OIndex::getImplementationName(  )
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VIndexDescriptor");
    return OUString("com.sun.star.sdbcx.VIndex");
}

css::uno::Sequence< OUString > SAL_CALL OIndex::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = "com.sun.star.sdbcx.IndexDescriptor";
    else
        aSupported[0] = "com.sun.star.sdbcx.Index";

    return aSupported;
}

sal_Bool SAL_CALL OIndex::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OIndex::OIndex(bool _bCase) : ODescriptor_BASE(m_aMutex)
                ,   ODescriptor(ODescriptor_BASE::rBHelper,_bCase,true)
                ,m_IsUnique(false)
                ,m_IsPrimaryKeyIndex(false)
                ,m_IsClustered(false)
{
}

OIndex::OIndex( const OUString& Name,
                const OUString& Catalog,
                bool _isUnique,
                bool _isPrimaryKeyIndex,
                bool _isClustered,
                bool _bCase) :  ODescriptor_BASE(m_aMutex)
                        ,ODescriptor(ODescriptor_BASE::rBHelper, _bCase)
                        ,m_Catalog(Catalog)
                        ,m_IsUnique(_isUnique)
                        ,m_IsPrimaryKeyIndex(_isPrimaryKeyIndex)
                        ,m_IsClustered(_isClustered)
{
    m_Name = Name;
}

OIndex::~OIndex( )
{
}

::cppu::IPropertyArrayHelper* OIndex::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper& SAL_CALL OIndex::getInfoHelper()
{
    return *OIndex_PROP::getArrayHelper(isNew() ? 1 : 0);
}

Any SAL_CALL OIndex::queryInterface( const Type & rType )
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

Sequence< Type > SAL_CALL OIndex::getTypes(  )
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes(),OIndex_BASE::getTypes());
}

void OIndex::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOG),         PROPERTY_ID_CATALOG,            nAttrib,&m_Catalog,         ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE),            PROPERTY_ID_ISUNIQUE,           nAttrib,&m_IsUnique,            cppu::UnoType<bool>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISPRIMARYKEYINDEX),PROPERTY_ID_ISPRIMARYKEYINDEX, nAttrib,&m_IsPrimaryKeyIndex,   cppu::UnoType<bool>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCLUSTERED),     PROPERTY_ID_ISCLUSTERED,        nAttrib,&m_IsClustered,     cppu::UnoType<bool>::get());
}

void OIndex::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pColumns)
        m_pColumns->disposing();
}

Reference< css::container::XNameAccess > SAL_CALL OIndex::getColumns(  )
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
        OSL_FAIL( "OIndex::getColumns: caught an exception!" );
    }

    return m_pColumns.get();
}

Reference< XPropertySet > SAL_CALL OIndex::createDataDescriptor(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(ODescriptor_BASE::rBHelper.bDisposed);


    return this;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OIndex::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OUString SAL_CALL OIndex::getName(  )
{
    return m_Name;
}

void SAL_CALL OIndex::setName( const OUString& /*aName*/ )
{
}

// XInterface
void SAL_CALL OIndex::acquire() throw()
{
    ODescriptor_BASE::acquire();
}

void SAL_CALL OIndex::release() throw()
{
    ODescriptor_BASE::release();
}

void OIndex::refreshColumns()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
