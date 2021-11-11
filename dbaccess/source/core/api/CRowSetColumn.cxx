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


#include <stringconstants.hxx>
#include <apitools.hxx>
#include "CRowSetColumn.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;

namespace dbaccess
{

ORowSetColumn::ORowSetColumn( const Reference < XResultSetMetaData >& _xMetaData,
                              const Reference < XRow >& _xRow, sal_Int32 _nPos,
                              const Reference< XDatabaseMetaData >& _rxDBMeta,
                              const OUString& _rDescription,
                              const OUString& i_sLabel,
                              const std::function<const ::connectivity::ORowSetValue& (sal_Int32)> &_getValue )
    :ORowSetDataColumn( _xMetaData, _xRow, nullptr, _nPos, _rxDBMeta, _rDescription, i_sLabel, _getValue )
{
}

::cppu::IPropertyArrayHelper* ORowSetColumn::createArrayHelper( ) const
{
    css::uno::Sequence< css::beans::Property> aDescriptor(21);
    css::beans::Property* pDesc = aDescriptor.getArray();
    sal_Int32 nPos = 0;

    DECL_PROP_IMPL(CATALOGNAME, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(DISPLAYSIZE, cppu::UnoType<sal_Int32>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISAUTOINCREMENT, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISCASESENSITIVE, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISCURRENCY, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISDEFINITELYWRITABLE, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISNULLABLE, cppu::UnoType<sal_Int32>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISREADONLY, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::BOUND );
    DECL_PROP_IMPL(ISROWVERSION, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISSEARCHABLE, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISSIGNED, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(ISWRITABLE, cppu::UnoType<bool>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(LABEL, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(PRECISION, cppu::UnoType<sal_Int32>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(SCALE, cppu::UnoType<sal_Int32>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(SCHEMANAME, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(SERVICENAME, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(TABLENAME, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(TYPE, cppu::UnoType<sal_Int32>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(TYPENAME, cppu::UnoType<OUString>::get()) css::beans::PropertyAttribute::READONLY );
    DECL_PROP_IMPL(VALUE, cppu::UnoType<Any>::get()) css::beans::PropertyAttribute::READONLY | css::beans::PropertyAttribute::BOUND);

    OSL_ENSURE(nPos == aDescriptor.getLength(), "forgot to adjust the count ?");

    Sequence< Property > aRegisteredProperties;
    describeProperties( aRegisteredProperties );

    return new ::cppu::OPropertyArrayHelper( ::comphelper::concatSequences( aDescriptor, aRegisteredProperties ), false );
}

::cppu::IPropertyArrayHelper& ORowSetColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< ORowSetColumn >* >(this)->getArrayHelper();
}

void SAL_CALL ORowSetColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )
{
    OSL_ENSURE( nHandle != PROPERTY_ID_VALUE, "ORowSetColumn::setFastPropertyValue_NoBroadcast: hmm? This property is marked as READONLY!" );
    if ( nHandle != PROPERTY_ID_VALUE )
        ORowSetDataColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
