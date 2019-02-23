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

#include <connectivity/sdbcx/VKeyColumn.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <TConnection.hxx>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace cppu;

OUString SAL_CALL OKeyColumn::getImplementationName(  )
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VKeyColumnDescriptor");
    return OUString("com.sun.star.sdbcx.VKeyColumn");
}

css::uno::Sequence< OUString > SAL_CALL OKeyColumn::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = "com.sun.star.sdbcx.KeyColumnDescriptor";
    else
        aSupported[0] = "com.sun.star.sdbcx.KeyColumn";

    return aSupported;
}

sal_Bool SAL_CALL OKeyColumn::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OKeyColumn::OKeyColumn(bool _bCase) : OColumn(_bCase)
{
    construct();
}

OKeyColumn::OKeyColumn( const OUString&  ReferencedColumn,
                        const OUString&  Name,
                        const OUString&  TypeName,
                        const OUString&  DefaultValue,
                        sal_Int32        IsNullable,
                        sal_Int32        Precision,
                        sal_Int32        Scale,
                        sal_Int32        Type,
                        bool             _bCase,
                        const OUString&  CatalogName,
                        const OUString&  SchemaName,
                        const OUString&  TableName
                        ) : OColumn(Name,
                            TypeName,
                            DefaultValue,
                            OUString(),
                            IsNullable,
                            Precision,
                            Scale,
                            Type,
                            false/*IsAutoIncrement*/,
                            false/*IsRowVersion*/,
                            false/*IsCurrency*/,
                            _bCase,
                            CatalogName,
                            SchemaName,
                            TableName)
                        ,   m_ReferencedColumn(ReferencedColumn)
{
    construct();
}

OKeyColumn::~OKeyColumn()
{
}

::cppu::IPropertyArrayHelper* OKeyColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper& SAL_CALL OKeyColumn::getInfoHelper()
{
    return *OKeyColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}

void OKeyColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::cppu::UnoType<OUString>::get());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
