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

#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/property.hxx>

#include <tools/debug.hxx>

using namespace com::sun::star;


//  class UnoControlBase


bool UnoControlBase::ImplHasProperty( sal_uInt16 nPropId )
{
    const OUString& aPropName( GetPropertyName( nPropId ) );
    return ImplHasProperty( aPropName );
}

bool UnoControlBase::ImplHasProperty( const OUString& aPropertyName )
{
    css::uno::Reference< css::beans::XPropertySet >  xPSet( mxModel, css::uno::UNO_QUERY );
    if ( !xPSet.is() )
        return false;
    css::uno::Reference< css::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
    if ( !xInfo.is() )
        return false;

    return xInfo->hasPropertyByName( aPropertyName );
}

void UnoControlBase::ImplSetPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues, bool bUpdateThis )
{
    css::uno::Reference< css::beans::XMultiPropertySet > xMPS( mxModel, css::uno::UNO_QUERY );
    if ( !mxModel.is() )
        return;

    DBG_ASSERT( xMPS.is(), "UnoControlBase::ImplSetPropertyValues: no multi property set interface!" );
    if ( xMPS.is() )
    {
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotifications( aPropertyNames, true );

        try
        {
            xMPS->setPropertyValues( aPropertyNames, aValues );
        }
        catch( const css::uno::Exception& )
        {
            if ( !bUpdateThis )
                ImplLockPropertyChangeNotifications( aPropertyNames, false );
        }
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotifications( aPropertyNames, false );
    }
}

void UnoControlBase::ImplSetPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue, bool bUpdateThis )
{
    // Model might be logged off already but an event still fires
    if ( mxModel.is() )
    {
        css::uno::Reference< css::beans::XPropertySet >  xPSet( mxModel, css::uno::UNO_QUERY );
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotification( aPropertyName, true );

        try
        {
            xPSet->setPropertyValue( aPropertyName, aValue );
        }
        catch( const css::uno::Exception& )
        {
            if ( !bUpdateThis )
                ImplLockPropertyChangeNotification( aPropertyName, false );
            throw;
        }
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotification( aPropertyName, false );
    }
}

css::uno::Any UnoControlBase::ImplGetPropertyValue( const OUString& aPropertyName )
{
    css::uno::Reference< css::beans::XPropertySet >  xPSet( mxModel, css::uno::UNO_QUERY );
    if ( xPSet.is() )
        return xPSet->getPropertyValue( aPropertyName );
    else
        return css::uno::Any();
}

template <typename T> T UnoControlBase::ImplGetPropertyValuePOD( sal_uInt16 nProp )
{
    T t(0);
    if ( mxModel.is() )
    {
        css::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= t;
    }
    return t;
}

template <typename T> T UnoControlBase::ImplGetPropertyValueClass( sal_uInt16 nProp )
{
    T t;
    if ( mxModel.is() )
    {
        css::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= t;
    }
    return t;
}

bool UnoControlBase::ImplGetPropertyValue_BOOL( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<bool>(nProp);
}

sal_Int16 UnoControlBase::ImplGetPropertyValue_INT16( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Int16>(nProp);
}

sal_Int32 UnoControlBase::ImplGetPropertyValue_INT32( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Int32>(nProp);
}

double UnoControlBase::ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<double>(nProp);
}

OUString UnoControlBase::ImplGetPropertyValue_UString( sal_uInt16 nProp )
{
    return ImplGetPropertyValueClass<OUString>(nProp);
}

util::Date UnoControlBase::ImplGetPropertyValue_Date( sal_uInt16 nProp )
{
    return ImplGetPropertyValueClass<util::Date>(nProp);
}

util::Time UnoControlBase::ImplGetPropertyValue_Time( sal_uInt16 nProp )
{
    return ImplGetPropertyValueClass<util::Time>(nProp);
}

css::awt::Size UnoControlBase::Impl_getMinimumSize()
{
    css::awt::Size aSz;
    css::uno::Reference< css::awt::XWindowPeer >  xP = ImplGetCompatiblePeer();
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        css::uno::Reference< css::awt::XLayoutConstrains >  xL( xP, css::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize();

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

css::awt::Size UnoControlBase::Impl_getPreferredSize()
{
    css::awt::Size aSz;
    css::uno::Reference< css::awt::XWindowPeer >  xP = ImplGetCompatiblePeer();
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        css::uno::Reference< css::awt::XLayoutConstrains >  xL( xP, css::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getPreferredSize();

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

css::awt::Size UnoControlBase::Impl_calcAdjustedSize( const css::awt::Size& rNewSize )
{
    css::awt::Size aSz;
    css::uno::Reference< css::awt::XWindowPeer >  xP = ImplGetCompatiblePeer();
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        css::uno::Reference< css::awt::XLayoutConstrains > xL( xP, css::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->calcAdjustedSize( rNewSize );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

css::awt::Size UnoControlBase::Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    css::awt::Size aSz;
    css::uno::Reference< css::awt::XWindowPeer >  xP = ImplGetCompatiblePeer();
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        css::uno::Reference< css::awt::XTextLayoutConstrains >  xL( xP, css::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize( nCols, nLines );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

void UnoControlBase::Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    css::uno::Reference< css::awt::XWindowPeer >  xP = ImplGetCompatiblePeer();
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        css::uno::Reference< css::awt::XTextLayoutConstrains >  xL( xP, css::uno::UNO_QUERY );
        if ( xL.is() )
            xL->getColumnsAndLines( nCols, nLines );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
