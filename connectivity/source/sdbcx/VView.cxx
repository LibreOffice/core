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

#include "connectivity/sdbcx/VView.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include "connectivity/dbtools.hxx"
#include "TConnection.hxx"

// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OView,"com.sun.star.sdbcx.VView","com.sun.star.sdbcx.View");
// -------------------------------------------------------------------------
OView::OView(sal_Bool _bCase,
            const ::rtl::OUString& _Name,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData,
            sal_Int32 _CheckOption,
            const ::rtl::OUString& _Command,
            const ::rtl::OUString& _SchemaName,
            const ::rtl::OUString& _CatalogName) : ODescriptor(::comphelper::OMutexAndBroadcastHelper::m_aBHelper,_bCase)
            ,m_CatalogName(_CatalogName)
            ,m_SchemaName(_SchemaName)
            ,m_Command(_Command)
            ,m_CheckOption(_CheckOption)
            ,m_xMetaData(_xMetaData)

{
    m_Name = _Name;
    construct();
}
// -------------------------------------------------------------------------
OView::OView(sal_Bool _bCase,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData)
    : ODescriptor(::comphelper::OMutexAndBroadcastHelper::m_aBHelper,_bCase,sal_True)
    ,m_xMetaData(_xMetaData)
{
    construct();
}
// -------------------------------------------------------------------------
OView::~OView()
{
}
// -------------------------------------------------------------------------
void OView::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND),         PROPERTY_ID_COMMAND,    nAttrib,&m_Command,     ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CHECKOPTION),     PROPERTY_ID_CHECKOPTION,nAttrib,&m_CheckOption, ::getCppuType(static_cast< sal_Int32*>(0)));
}
// -------------------------------------------------------------------------
void OView::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OView::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OView_BASE::getTypes());
}
// -------------------------------------------------------------------------
Any SAL_CALL OView::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OView_BASE::queryInterface( rType);
    return aRet.hasValue() ? aRet : ODescriptor::queryInterface( rType);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OView::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OView::getInfoHelper()
{
    return *const_cast<OView*>(this)->getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OView::getName() throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sComposedName;
    if(m_xMetaData.is())
        sComposedName = ::dbtools::composeTableName( m_xMetaData, m_CatalogName, m_SchemaName, m_Name, sal_False, ::dbtools::eInDataManipulation );
    else
    {
        Any aValue;
        getFastPropertyValue(aValue,PROPERTY_ID_NAME);
        aValue >>= sComposedName;
    }
    return sComposedName;
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OView::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void SAL_CALL OView::setName( const ::rtl::OUString& ) throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OView::acquire() throw()
{
    OView_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OView::release() throw()
{
    OView_BASE::release();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
