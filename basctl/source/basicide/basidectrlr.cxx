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

#include <basidectrlr.hxx>
#include <basidesh.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

namespace basctl
{

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

namespace
{

int const nPropertyIconId = 1;
constexpr OUStringLiteral sPropertyIconId(u"IconId");

}

Controller::Controller (Shell* pViewShell)
    :OPropertyContainer( m_aBHelper )
    ,SfxBaseController( pViewShell )
    ,m_nIconId( ICON_MACROLIBRARY )
{
    registerProperty(
        sPropertyIconId, nPropertyIconId,
        PropertyAttribute::READONLY,
        &m_nIconId, cppu::UnoType<decltype(m_nIconId)>::get()
    );
}

Controller::~Controller()
{ }

// XInterface
Any SAL_CALL Controller::queryInterface( const Type & rType )
{
    Any aReturn = SfxBaseController::queryInterface( rType );
    if ( !aReturn.hasValue() )
        aReturn = OPropertyContainer::queryInterface( rType );

    return aReturn;
}

void SAL_CALL Controller::acquire() noexcept
{
    SfxBaseController::acquire();
}

void SAL_CALL Controller::release() noexcept
{
    SfxBaseController::release();
}

// XTypeProvider ( ::SfxBaseController )
Sequence< Type > SAL_CALL Controller::getTypes()
{
    Sequence< Type > aTypes = ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        getBaseTypes()
    );

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL Controller::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XPropertySet
Reference< beans::XPropertySetInfo > SAL_CALL Controller::getPropertySetInfo()
{
    Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// OPropertySetHelper
::cppu::IPropertyArrayHelper& Controller::getInfoHelper()
{
    return *getArrayHelper();
}

// OPropertyArrayUsageHelper
::cppu::IPropertyArrayHelper* Controller::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
