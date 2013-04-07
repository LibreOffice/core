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

#include "connectivity/sdbcx/VKeyColumn.hxx"
#include "TConnection.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace cppu;
// -----------------------------------------------------------------------------
OUString SAL_CALL OKeyColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VKeyColumnDescription");
    return OUString("com.sun.star.sdbcx.VKeyColumn");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< OUString > SAL_CALL OKeyColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = OUString("com.sun.star.sdbcx.KeyColumnDescription");
    else
        aSupported[0] = OUString("com.sun.star.sdbcx.KeyColumn");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeyColumn::supportsService( const OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn(sal_Bool _bCase) : OColumn(_bCase)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn( const OUString&  _ReferencedColumn,
                        const OUString&  _Name,
                        const OUString&  _TypeName,
                        const OUString&  _DefaultValue,
                        sal_Int32               _IsNullable,
                        sal_Int32               _Precision,
                        sal_Int32               _Scale,
                        sal_Int32               _Type,
                        sal_Bool                _IsAutoIncrement,
                        sal_Bool                _IsRowVersion,
                        sal_Bool                _IsCurrency,
                        sal_Bool                _bCase,
                        const OUString& _CatalogName,
                        const OUString& _SchemaName,
                        const OUString& _TableName
                        ) : OColumn(_Name,
                            _TypeName,
                            _DefaultValue,
                            OUString(),
                            _IsNullable,
                            _Precision,
                            _Scale,
                            _Type,
                            _IsAutoIncrement,
                            _IsRowVersion,
                            _IsCurrency,
                            _bCase,
                            _CatalogName,
                            _SchemaName,
                            _TableName)
                        ,   m_ReferencedColumn(_ReferencedColumn)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::~OKeyColumn()
{
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OKeyColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OKeyColumn::getInfoHelper()
{
    return *OKeyColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
void OKeyColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(static_cast< OUString*>(0)));
}
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
