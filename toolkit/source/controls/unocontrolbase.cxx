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

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/property.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/debug.hxx>

//  ----------------------------------------------------
//  class UnoControlBase
//  ----------------------------------------------------

sal_Bool UnoControlBase::ImplHasProperty( sal_uInt16 nPropId )
{
    OUString aPropName( GetPropertyName( nPropId ) );
    return ImplHasProperty( aPropName );
}

sal_Bool UnoControlBase::ImplHasProperty( const OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    if ( !xPSet.is() )
        return sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
    if ( !xInfo.is() )
        return sal_False;

    return xInfo->hasPropertyByName( aPropertyName );
}

void UnoControlBase::ImplSetPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Bool bUpdateThis )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > xMPS( mxModel, ::com::sun::star::uno::UNO_QUERY );
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
        catch( const ::com::sun::star::uno::Exception& )
        {
            if ( !bUpdateThis )
                ImplLockPropertyChangeNotifications( aPropertyNames, false );
        }
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotifications( aPropertyNames, false );
    }
    else
    {
        int dummy = 0;
        (void)dummy;
    }
}

void UnoControlBase::ImplSetPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Bool bUpdateThis )
{
    // Model might be logged off already but an event still fires
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotification( aPropertyName, true );

        try
        {
            xPSet->setPropertyValue( aPropertyName, aValue );
        }
        catch( const com::sun::star::uno::Exception& )
        {
            if ( !bUpdateThis )
                ImplLockPropertyChangeNotification( aPropertyName, false );
            throw;
        }
        if ( !bUpdateThis )
            ImplLockPropertyChangeNotification( aPropertyName, false );
    }
}

::com::sun::star::uno::Any UnoControlBase::ImplGetPropertyValue( const OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    if ( xPSet.is() )
        return xPSet->getPropertyValue( aPropertyName );
    else
        return ::com::sun::star::uno::Any();
}

template <typename T> T UnoControlBase::ImplGetPropertyValuePOD( sal_uInt16 nProp )
{
    T t(0);
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= t;
    }
    return t;
}

template <typename T> T UnoControlBase::ImplGetPropertyValueClass( sal_uInt16 nProp )
{
    T t;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= t;
    }
    return t;
}

sal_Bool UnoControlBase::ImplGetPropertyValue_BOOL( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Bool>(nProp);
}

sal_Int16 UnoControlBase::ImplGetPropertyValue_INT16( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Int16>(nProp);
}

sal_Int32 UnoControlBase::ImplGetPropertyValue_INT32( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Int32>(nProp);
}

sal_Int64 UnoControlBase::ImplGetPropertyValue_INT64( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<sal_Int64>(nProp);
}

double UnoControlBase::ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp )
{
    return ImplGetPropertyValuePOD<double>(nProp);
}

OUString UnoControlBase::ImplGetPropertyValue_UString( sal_uInt16 nProp )
{
    return ImplGetPropertyValueClass<OUString>(nProp);
}

::com::sun::star::awt::Size UnoControlBase::Impl_getMinimumSize()
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize();

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_getPreferredSize()
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getPreferredSize();

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize )
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains > xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->calcAdjustedSize( rNewSize );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

::com::sun::star::awt::Size UnoControlBase::Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    ::com::sun::star::awt::Size aSz;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            aSz = xL->getMinimumSize( nCols, nLines );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
    return aSz;
}

void UnoControlBase::Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  xP = ImplGetCompatiblePeer( sal_True );
    DBG_ASSERT( xP.is(), "Layout: No Peer!" );
    if ( xP.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains >  xL( xP, ::com::sun::star::uno::UNO_QUERY );
        if ( xL.is() )
            xL->getColumnsAndLines( nCols, nLines );

        if ( !getPeer().is() || ( getPeer() != xP ) )
            xP->dispose();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
