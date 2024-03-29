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

#include <connectivity/sdbcx/VView.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/sequence.hxx>
#include <connectivity/dbtools.hxx>
#include <TConnection.hxx>
#include <utility>


using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OView,"com.sun.star.sdbcx.VView","com.sun.star.sdbcx.View");

OView::OView(bool _bCase,
            const OUString& Name,
            css::uno::Reference< css::sdbc::XDatabaseMetaData >  _xMetaData,
            OUString Command,
            OUString SchemaName,
            OUString CatalogName) : ODescriptor(::comphelper::OMutexAndBroadcastHelper::m_aBHelper,_bCase)
            ,m_CatalogName(std::move(CatalogName))
            ,m_SchemaName(std::move(SchemaName))
            ,m_Command(std::move(Command))
            ,m_CheckOption(0)
            ,m_xMetaData(std::move(_xMetaData))

{
    m_Name = Name;
    construct();
}

OView::OView(bool _bCase, css::uno::Reference< css::sdbc::XDatabaseMetaData > _xMetaData)
    : ODescriptor(::comphelper::OMutexAndBroadcastHelper::m_aBHelper, _bCase, true)
    ,m_xMetaData(std::move(_xMetaData))
{
    construct();
}

OView::~OView()
{
}

void OView::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND),         PROPERTY_ID_COMMAND,    nAttrib,&m_Command,     ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CHECKOPTION),     PROPERTY_ID_CHECKOPTION,nAttrib,&m_CheckOption, ::cppu::UnoType<sal_Int32>::get());
}

Sequence< Type > SAL_CALL OView::getTypes(  )
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OView_BASE::getTypes());
}

Any SAL_CALL OView::queryInterface( const Type & rType )
{
    Any aRet = OView_BASE::queryInterface( rType);
    return aRet.hasValue() ? aRet : ODescriptor::queryInterface( rType);
}

::cppu::IPropertyArrayHelper* OView::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & OView::getInfoHelper()
{
    return *getArrayHelper(isNew() ? 1 : 0);
}

OUString SAL_CALL OView::getName()
{
    OUString sComposedName;
    if(m_xMetaData.is())
        sComposedName = ::dbtools::composeTableName( m_xMetaData, m_CatalogName, m_SchemaName, m_Name, false, ::dbtools::EComposeRule::InDataManipulation );
    else
    {
        Any aValue;
        getFastPropertyValue(aValue,PROPERTY_ID_NAME);
        aValue >>= sComposedName;
    }
    return sComposedName;
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OView::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void SAL_CALL OView::setName( const OUString& )
{
}

void SAL_CALL OView::acquire() noexcept
{
    OView_BASE::acquire();
}

void SAL_CALL OView::release() noexcept
{
    OView_BASE::release();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
