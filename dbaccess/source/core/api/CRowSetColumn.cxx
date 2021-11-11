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

    pDesc[nPos++] = css::beans::Property(PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_DISPLAYSIZE, PROPERTY_ID_DISPLAYSIZE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISAUTOINCREMENT, PROPERTY_ID_ISAUTOINCREMENT, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISCASESENSITIVE, PROPERTY_ID_ISCASESENSITIVE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISCURRENCY, PROPERTY_ID_ISCURRENCY, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISDEFINITELYWRITABLE, PROPERTY_ID_ISDEFINITELYWRITABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISNULLABLE, PROPERTY_ID_ISNULLABLE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISREADONLY, PROPERTY_ID_ISREADONLY, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::BOUND );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISROWVERSION, PROPERTY_ID_ISROWVERSION, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISSEARCHABLE, PROPERTY_ID_ISSEARCHABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISSIGNED, PROPERTY_ID_ISSIGNED, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_ISWRITABLE, PROPERTY_ID_ISWRITABLE, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_LABEL, PROPERTY_ID_LABEL, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_PRECISION, PROPERTY_ID_PRECISION, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_SCALE, PROPERTY_ID_SCALE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_SERVICENAME, PROPERTY_ID_SERVICENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_TABLENAME, PROPERTY_ID_TABLENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_TYPE, PROPERTY_ID_TYPE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_TYPENAME, PROPERTY_ID_TYPENAME, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY );
    pDesc[nPos++] = css::beans::Property(PROPERTY_VALUE, PROPERTY_ID_VALUE, cppu::UnoType<Any>::get(), css::beans::PropertyAttribute::READONLY | css::beans::PropertyAttribute::BOUND);

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
