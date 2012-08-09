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
    ::rtl::OUString aPropName( GetPropertyName( nPropId ) );
    return ImplHasProperty( aPropName );
}

sal_Bool UnoControlBase::ImplHasProperty( const ::rtl::OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    if ( !xPSet.is() )
        return sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo = xPSet->getPropertySetInfo();
    if ( !xInfo.is() )
        return sal_False;

    return xInfo->hasPropertyByName( aPropertyName );
}

void UnoControlBase::ImplSetPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Bool bUpdateThis )
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

void UnoControlBase::ImplSetPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Bool bUpdateThis )
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

::com::sun::star::uno::Any UnoControlBase::ImplGetPropertyValue( const ::rtl::OUString& aPropertyName )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xPSet( mxModel, ::com::sun::star::uno::UNO_QUERY );
    if ( xPSet.is() )
        return xPSet->getPropertyValue( aPropertyName );
    else
        return ::com::sun::star::uno::Any();
}

sal_Bool UnoControlBase::ImplGetPropertyValue_BOOL( sal_uInt16 nProp )
{
    sal_Bool b = sal_False;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= b;
    }
    return b;
}

sal_Int16 UnoControlBase::ImplGetPropertyValue_INT16( sal_uInt16 nProp )
{
    sal_Int16 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

sal_Int32 UnoControlBase::ImplGetPropertyValue_INT32( sal_uInt16 nProp )
{
    sal_Int32 n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

double UnoControlBase::ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp )
{
    double n = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= n;
    }
    return n;
}

::rtl::OUString UnoControlBase::ImplGetPropertyValue_UString( sal_uInt16 nProp )
{
    ::rtl::OUString aStr;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( nProp ) );
        aVal >>= aStr;
    }
    return aStr;
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
