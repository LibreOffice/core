/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/property.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/debug.hxx>

//  ----------------------------------------------------
//  class UnoControlBase
//  ----------------------------------------------------

UnoControlBase::UnoControlBase()
    :UnoControl( ::comphelper::getProcessServiceFactory() )
{
    OSL_ENSURE( false, "UnoControlBase::UnoControlBase: not implemented. Well, not really." );
    // just implemented to let the various FooImplInheritanceHelper compile, you should use the
    // version taking a service factory
}

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
    // Model ggf. schon abgemeldet, aber ein Event schlaegt noch zu...
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

sal_uInt16 UnoControlBase::ImplGetPropertyValue_UINT16( sal_uInt16 nProp )
{
    sal_uInt16 n = 0;
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

sal_uInt32 UnoControlBase::ImplGetPropertyValue_UINT32( sal_uInt16 nProp )
{
    sal_uInt32 n = 0;
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



