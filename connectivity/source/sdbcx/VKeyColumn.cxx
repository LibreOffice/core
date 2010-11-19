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
#include "connectivity/sdbcx/VKeyColumn.hxx"
#include "TConnection.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace cppu;
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKeyColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VKeyColumnDescription"));
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.VKeyColumn"));
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OKeyColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.KeyColumnDescription"));
    else
        aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.KeyColumn"));

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeyColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
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
OKeyColumn::OKeyColumn( const ::rtl::OUString&  _ReferencedColumn,
                        const ::rtl::OUString&  _Name,
                        const ::rtl::OUString&  _TypeName,
                        const ::rtl::OUString&  _DefaultValue,
                        sal_Int32               _IsNullable,
                        sal_Int32               _Precision,
                        sal_Int32               _Scale,
                        sal_Int32               _Type,
                        sal_Bool                _IsAutoIncrement,
                        sal_Bool                _IsRowVersion,
                        sal_Bool                _IsCurrency,
                        sal_Bool                _bCase
                        ) : OColumn(_Name,
                            _TypeName,
                            _DefaultValue,
                            ::rtl::OUString(),
                            _IsNullable,
                            _Precision,
                            _Scale,
                            _Type,
                            _IsAutoIncrement,
                            _IsRowVersion,
                            _IsCurrency,
                            _bCase)
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
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
