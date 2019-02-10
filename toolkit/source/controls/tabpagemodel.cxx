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

#include <toolkit/controls/tabpagemodel.hxx>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/UnoControlDialogModelProvider.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/outdev.hxx>

#include <vcl/graph.hxx>
#include <vcl/image.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>

#include <helper/unopropertyarrayhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;

UnoControlTabPageModel::UnoControlTabPageModel( Reference< XComponentContext > const & i_factory )
    :ControlModelContainerBase( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );
}

OUString SAL_CALL UnoControlTabPageModel::getImplementationName()
{
    return OUString("stardiv.Toolkit.UnoControlTabPageModel");
}

css::uno::Sequence< OUString > SAL_CALL UnoControlTabPageModel::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > aNames = ControlModelContainerBase::getSupportedServiceNames( );
    aNames.realloc( aNames.getLength() + 1 );
    aNames[ aNames.getLength() - 1 ] = "com.sun.star.awt.tab.UnoControlTabPageModel";
    return aNames;
}

OUString UnoControlTabPageModel::getServiceName( )
{
    return OUString("com.sun.star.awt.tab.UnoControlTabPageModel");
}

Any UnoControlTabPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= OUString("com.sun.star.awt.tab.UnoControlTabPage");
            break;
        case BASEPROPERTY_USERFORMCONTAINEES:
        {
            // We do not have here any usercontainers (yet?), but let's return empty container back
            // so normal properties could be set without triggering UnknownPropertyException
            aAny <<= uno::Reference< XNameContainer >();
            break;
        }
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& UnoControlTabPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}
// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlTabPageModel::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
////----- XInitialization -------------------------------------------------------------------
void SAL_CALL UnoControlTabPageModel::initialize (const Sequence<Any>& rArguments)
{
    sal_Int16 nPageId = -1;
    if ( rArguments.getLength() == 1 )
    {
        if ( !( rArguments[ 0 ] >>= nPageId ))
             throw lang::IllegalArgumentException();
        m_nTabPageId = nPageId;
    }
    else if ( rArguments.getLength() == 2 )
    {
        if ( !( rArguments[ 0 ] >>= nPageId ))
             throw lang::IllegalArgumentException();
        m_nTabPageId = nPageId;
        OUString sURL;
        if ( !( rArguments[ 1 ] >>= sURL ))
            throw lang::IllegalArgumentException();
        Reference<container::XNameContainer > xDialogModel = awt::UnoControlDialogModelProvider::create( m_xContext, sURL );
        if ( xDialogModel.is() )
        {
            Sequence< OUString> aNames = xDialogModel->getElementNames();
            const OUString* pIter = aNames.getConstArray();
            const OUString* pEnd = pIter + aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                try
                {
                    Any aElement(xDialogModel->getByName(*pIter));
                    xDialogModel->removeByName(*pIter);
                    insertByName(*pIter,aElement);
                }
                catch(const Exception&) {}
            }
            Reference<XPropertySet> xDialogProp(xDialogModel,UNO_QUERY);
            if ( xDialogProp.is() )
            {
                static const char s_sResourceResolver[] = "ResourceResolver";
                Reference<XPropertySet> xThis(*this,UNO_QUERY);
                xThis->setPropertyValue(s_sResourceResolver,xDialogProp->getPropertyValue(s_sResourceResolver));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_TITLE),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_TITLE)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_HELPTEXT),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_HELPTEXT)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_HELPURL),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_HELPURL)));
            }
        }
    }
    else
        m_nTabPageId = -1;
}


UnoControlTabPage::UnoControlTabPage( const uno::Reference< uno::XComponentContext >& rxContext )
    :UnoControlTabPage_Base(rxContext)
    ,m_bWindowListener(false)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}
UnoControlTabPage::~UnoControlTabPage()
{
}

OUString UnoControlTabPage::GetComponentServiceName()
{
    return OUString("TabPageModel");
}

OUString SAL_CALL UnoControlTabPage::getImplementationName()
{
    return OUString("stardiv.Toolkit.UnoControlTabPage");
}

sal_Bool SAL_CALL UnoControlTabPage::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SAL_CALL UnoControlTabPage::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > aSeq { "com.sun.star.awt.tab.UnoControlTabPage" };
    return aSeq;
}

void UnoControlTabPage::dispose()
{
    SolarMutexGuard aSolarGuard;

    lang::EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    ControlContainerBase::dispose();
}

void SAL_CALL UnoControlTabPage::disposing( const lang::EventObject& Source )
{
     ControlContainerBase::disposing( Source );
}

void UnoControlTabPage::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer )
{
    SolarMutexGuard aSolarGuard;
    ImplUpdateResourceResolver();

    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    Reference < tab::XTabPage > xTabPage( getPeer(), UNO_QUERY );
    if ( xTabPage.is() )
    {
        if ( !m_bWindowListener )
        {
            Reference< XWindowListener > xWL( static_cast< cppu::OWeakObject*>( this ), UNO_QUERY );
            addWindowListener( xWL );
            m_bWindowListener = true;
        }
    }
}

static ::Size ImplMapPixelToAppFont( OutputDevice const * pOutDev, const ::Size& aSize )
{
    ::Size aTmp = pOutDev->PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));
    return aTmp;
}
// css::awt::XWindowListener
void SAL_CALL UnoControlTabPage::windowResized( const css::awt::WindowEvent& e )
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( !pOutDev || mbSizeModified )
        return;

    // Currentley we are simply using MapUnit::MapAppFont
    ::Size aAppFontSize( e.Width, e.Height );

    Reference< XControl > xDialogControl( *this, UNO_QUERY_THROW );
    Reference< XDevice > xDialogDevice( xDialogControl->getPeer(), UNO_QUERY );
    OSL_ENSURE( xDialogDevice.is(), "UnoDialogControl::windowResized: no peer, but a windowResized event?" );
    if ( xDialogDevice.is() )
    {
        DeviceInfo aDeviceInfo( xDialogDevice->getInfo() );
        aAppFontSize.AdjustWidth( -(aDeviceInfo.LeftInset + aDeviceInfo.RightInset) );
        aAppFontSize.AdjustHeight( -(aDeviceInfo.TopInset + aDeviceInfo.BottomInset) );
    }

    aAppFontSize = ImplMapPixelToAppFont( pOutDev, aAppFontSize );

    // Remember that changes have been done by listener. No need to
    // update the position because of property change event.
    mbSizeModified = true;
    Sequence< OUString > aProps( 2 );
    Sequence< Any > aValues( 2 );
    // Properties in a sequence must be sorted!
    aProps[0] = "Height";
    aProps[1] = "Width";
    aValues[0] <<= aAppFontSize.Height();
    aValues[1] <<= aAppFontSize.Width();

    ImplSetPropertyValues( aProps, aValues, true );
    mbSizeModified = false;

}

void SAL_CALL UnoControlTabPage::windowMoved( const css::awt::WindowEvent& e )
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( !pOutDev || mbPosModified )
        return;

    // Currentley we are simply using MapUnit::MapAppFont
    ::Size aTmp( e.X, e.Y );
    aTmp = ImplMapPixelToAppFont( pOutDev, aTmp );

    // Remember that changes have been done by listener. No need to
    // update the position because of property change event.
    mbPosModified = true;
    Sequence< OUString > aProps( 2 );
    Sequence< Any > aValues( 2 );
    aProps[0] = "PositionX";
    aProps[1] = "PositionY";
    aValues[0] <<= aTmp.Width();
    aValues[1] <<= aTmp.Height();

    ImplSetPropertyValues( aProps, aValues, true );
    mbPosModified = false;

}

void SAL_CALL UnoControlTabPage::windowShown( const css::lang::EventObject& ) {}

void SAL_CALL UnoControlTabPage::windowHidden( const css::lang::EventObject& ) {}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoControlTabPageModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoControlTabPageModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoControlTabPage_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoControlTabPage(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
