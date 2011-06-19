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
#include "hsqldb/HColumns.hxx"
#include "TConnection.hxx"


using namespace ::comphelper;
using namespace connectivity::hsqldb;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OHSQLColumns::OHSQLColumns( ::cppu::OWeakObject& _rParent
                                ,sal_Bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const TStringVector &_rVector
                                ,sal_Bool _bUseHardRef
            ) : OColumnsHelper(_rParent,_bCase,_rMutex,_rVector,_bUseHardRef)
{
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OHSQLColumns::createDescriptor()
{
    return new OHSQLColumn(sal_True);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
OHSQLColumn::OHSQLColumn(   sal_Bool    _bCase)
    : connectivity::sdbcx::OColumn( _bCase )
{
    construct();
}
// -------------------------------------------------------------------------
void OHSQLColumn::construct()
{
    m_sAutoIncrement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IDENTITY"));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION),PROPERTY_ID_AUTOINCREMENTCREATION,0,&m_sAutoIncrement, ::getCppuType(&m_sAutoIncrement));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OHSQLColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SAL_CALL OHSQLColumn::getInfoHelper()
{
    return *OHSQLColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OHSQLColumn::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Column"));

    return aSupported;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
