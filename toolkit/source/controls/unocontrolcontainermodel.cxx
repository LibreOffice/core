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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <toolkit/controls/unocontrolcontainermodel.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>

#include "helper/unopropertyarrayhelper.hxx"

//  class UnoControlContainerModel

UnoControlContainerModel::UnoControlContainerModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory )
    :UnoControlModel( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

OUString UnoControlContainerModel::getServiceName() throw(css::uno::RuntimeException, std::exception)
{
    return OUString::createFromAscii( szServiceName_UnoControlContainerModel );
}

OUString UnoControlContainerModel::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("stardiv.Toolkit.UnoControlContainerModel");
}

css::uno::Sequence<OUString>
UnoControlContainerModel::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    auto s(UnoControlModel::getSupportedServiceNames());
    s.realloc(s.getLength() + 2);
    s[s.getLength() - 2] = "com.sun.star.awt.UnoControlContainerModel";
    s[s.getLength() - 1] = "stardiv.vcl.controlmodel.ControlContainer";
    return s;
}

css::uno::Any UnoControlContainerModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    css::uno::Any aDefault;
    if ( nPropId == BASEPROPERTY_BORDER )
        aDefault <<= (sal_Int16) 0;
    else
        aDefault <<= UnoControlModel::ImplGetDefaultValue( nPropId );
    return aDefault;
}


css::uno::Reference< css::beans::XPropertySetInfo > UnoControlContainerModel::getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception)
{
    static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& UnoControlContainerModel::getInfoHelper()
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        css::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoControlContainerModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoControlContainerModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
