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

#include <connectivity/sdbcx/VColumn.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <TConnection.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;


OUString SAL_CALL OColumn::getImplementationName(  )
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VColumnDescriptor");
    return OUString("com.sun.star.sdbcx.VColumn");
}

css::uno::Sequence< OUString > SAL_CALL OColumn::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = "com.sun.star.sdbcx.ColumnDescriptor";
    else
        aSupported[0] = "com.sun.star.sdbcx.Column";

    return aSupported;
}

sal_Bool SAL_CALL OColumn::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OColumn::OColumn(bool _bCase)
    :OColumnDescriptor_BASE(m_aMutex)
    ,ODescriptor(OColumnDescriptor_BASE::rBHelper,_bCase,true)
    ,m_IsNullable(ColumnValue::NULLABLE)
    ,m_Precision(0)
    ,m_Scale(0)
    ,m_Type(0)
    ,m_IsAutoIncrement(false)
    ,m_IsRowVersion(false)
    ,m_IsCurrency(false)
{
    construct();
}

OColumn::OColumn(   const OUString& Name,
                    const OUString& TypeName,
                    const OUString& DefaultValue,
                    const OUString& Description,
                    sal_Int32       IsNullable,
                    sal_Int32       Precision,
                    sal_Int32       Scale,
                    sal_Int32       Type,
                    bool            IsAutoIncrement,
                    bool            IsRowVersion,
                    bool            IsCurrency,
                    bool            _bCase,
                    const OUString& CatalogName,
                    const OUString& SchemaName,
                    const OUString& TableName)
    :OColumnDescriptor_BASE(m_aMutex)
    ,ODescriptor(OColumnDescriptor_BASE::rBHelper,_bCase)
    ,m_TypeName(TypeName)
    ,m_Description(Description)
    ,m_DefaultValue(DefaultValue)
    ,m_IsNullable(IsNullable)
    ,m_Precision(Precision)
    ,m_Scale(Scale)
    ,m_Type(Type)
    ,m_IsAutoIncrement(IsAutoIncrement)
    ,m_IsRowVersion(IsRowVersion)
    ,m_IsCurrency(IsCurrency)
    ,m_CatalogName(CatalogName)
    ,m_SchemaName(SchemaName)
    ,m_TableName(TableName)
{
    m_Name = Name;

    construct();
}

OColumn::~OColumn()
{
}

::cppu::IPropertyArrayHelper* OColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper& SAL_CALL OColumn::getInfoHelper()
{
    return *OColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}

void SAL_CALL OColumn::acquire() throw()
{
    OColumnDescriptor_BASE::acquire();
}

void SAL_CALL OColumn::release() throw()
{
    OColumnDescriptor_BASE::release();
}

Any SAL_CALL OColumn::queryInterface( const Type & rType )
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OColumn_BASE::queryInterface(rType);
        if(!aRet.hasValue())
            aRet = OColumnDescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}

Sequence< Type > SAL_CALL OColumn::getTypes(  )
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),OColumnDescriptor_BASE::getTypes());

    return ::comphelper::concatSequences(ODescriptor::getTypes(),OColumn_BASE::getTypes(),OColumnDescriptor_BASE::getTypes());
}

void OColumn::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME),        PROPERTY_ID_TYPENAME,           nAttrib, &m_TypeName,        cppu::UnoType<decltype(m_TypeName)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION),     PROPERTY_ID_DESCRIPTION,        nAttrib, &m_Description,     cppu::UnoType<decltype(m_Description)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE),    PROPERTY_ID_DEFAULTVALUE,       nAttrib, &m_DefaultValue,    cppu::UnoType<decltype(m_DefaultValue)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION),       PROPERTY_ID_PRECISION,          nAttrib, &m_Precision,       cppu::UnoType<decltype(m_Precision)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,               nAttrib, &m_Type,            cppu::UnoType<decltype(m_Type)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE),           PROPERTY_ID_SCALE,              nAttrib, &m_Scale,           cppu::UnoType<decltype(m_Scale)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE),      PROPERTY_ID_ISNULLABLE,         nAttrib, &m_IsNullable,      cppu::UnoType<decltype(m_IsNullable)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT), PROPERTY_ID_ISAUTOINCREMENT,    nAttrib, &m_IsAutoIncrement, cppu::UnoType<bool>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISROWVERSION),    PROPERTY_ID_ISROWVERSION,       nAttrib, &m_IsRowVersion,    cppu::UnoType<bool>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY),      PROPERTY_ID_ISCURRENCY,         nAttrib, &m_IsCurrency,      cppu::UnoType<bool>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,        nAttrib, &m_CatalogName,     cppu::UnoType<decltype(m_CatalogName)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME,         nAttrib, &m_SchemaName,      cppu::UnoType<decltype(m_SchemaName)>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TABLENAME),       PROPERTY_ID_TABLENAME,          nAttrib, &m_TableName,       cppu::UnoType<decltype(m_TableName)>::get());
}

void OColumn::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OColumnDescriptor_BASE::rBHelper.bDisposed);

}

Reference< XPropertySet > SAL_CALL OColumn::createDataDescriptor(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OColumnDescriptor_BASE::rBHelper.bDisposed);


    OColumn* pNewColumn = new OColumn(  m_Name,
                                        m_TypeName,
                                        m_DefaultValue,
                                        m_Description,
                                        m_IsNullable,
                                        m_Precision,
                                        m_Scale,
                                        m_Type,
                                        m_IsAutoIncrement,
                                        m_IsRowVersion,
                                        m_IsCurrency,
                                        isCaseSensitive(),
                                        m_CatalogName,
                                        m_SchemaName,
                                        m_TableName);
    pNewColumn->setNew(true);
    return pNewColumn;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OColumn::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

// XNamed
OUString SAL_CALL OColumn::getName(  )
{
    return m_Name;
}

void SAL_CALL OColumn::setName( const OUString& aName )
{
    m_Name = aName;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
