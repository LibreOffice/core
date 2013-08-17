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


#include "dbastrings.hrc"
#include "apitools.hxx"
#include "CRowSetColumn.hxx"

#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <comphelper/types.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;

namespace dbaccess
{

ORowSetColumn::ORowSetColumn(   const Reference < XResultSetMetaData >& _xMetaData, const Reference < XRow >& _xRow, sal_Int32 _nPos,
                const Reference< XDatabaseMetaData >& _rxDBMeta, const OUString& _rDescription, const OUString& i_sLabel,ORowSetCacheIterator& _rColumnValue )
    :ORowSetDataColumn( _xMetaData, _xRow, NULL, _nPos, _rxDBMeta, _rDescription, i_sLabel,_rColumnValue )
{
}

::cppu::IPropertyArrayHelper* ORowSetColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_SEQUENCE(21)

    DECL_PROP1( CATALOGNAME,                OUString,    READONLY );
    DECL_PROP1( DISPLAYSIZE,                sal_Int32,          READONLY );
    DECL_PROP1_BOOL( ISAUTOINCREMENT,                           READONLY );
    DECL_PROP1_BOOL( ISCASESENSITIVE,                           READONLY );
    DECL_PROP1_BOOL( ISCURRENCY,                                READONLY );
    DECL_PROP1_BOOL( ISDEFINITELYWRITABLE,                      READONLY );
    DECL_PROP1( ISNULLABLE,                 sal_Int32,          READONLY );
    DECL_PROP1_BOOL( ISREADONLY,                                BOUND );
    DECL_PROP1_BOOL( ISROWVERSION,                              READONLY );
    DECL_PROP1_BOOL( ISSEARCHABLE,                              READONLY );
    DECL_PROP1_BOOL( ISSIGNED,                                  READONLY );
    DECL_PROP1_BOOL( ISWRITABLE,                                READONLY );
    DECL_PROP1( LABEL,                      OUString,    READONLY );
    DECL_PROP1( PRECISION,                  sal_Int32,          READONLY );
    DECL_PROP1( SCALE,                      sal_Int32,          READONLY );
    DECL_PROP1( SCHEMANAME,                 OUString,    READONLY );
    DECL_PROP1( SERVICENAME,                OUString,    READONLY );
    DECL_PROP1( TABLENAME,                  OUString,    READONLY );
    DECL_PROP1( TYPE,                       sal_Int32,          READONLY );
    DECL_PROP1( TYPENAME,                   OUString,    READONLY );
    DECL_PROP2( VALUE,                      Any,                READONLY, BOUND );

    END_PROPERTY_SEQUENCE()

    Sequence< Property > aRegisteredProperties;
    describeProperties( aRegisteredProperties );

    return new ::cppu::OPropertyArrayHelper( ::comphelper::concatSequences( aDescriptor, aRegisteredProperties ), sal_False );
}

::cppu::IPropertyArrayHelper& ORowSetColumn::getInfoHelper()
{
    return *static_cast< ::comphelper::OPropertyArrayUsageHelper< ORowSetColumn >* >(this)->getArrayHelper();
}

void SAL_CALL ORowSetColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )throw (Exception)
{
    OSL_ENSURE( nHandle != PROPERTY_ID_VALUE, "ORowSetColumn::setFastPropertyValue_NoBroadcast: hmm? This property is marked as READONLY!" );
    if ( nHandle != PROPERTY_ID_VALUE )
        ORowSetDataColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
