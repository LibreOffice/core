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
#include "precompiled_toolkit.hxx"

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <vos/mutex.hxx>
#include <toolkit/controls/tabpagemodel.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/UnoControlDialogModelProvider.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <tools/list.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graph.hxx>
#include <vcl/image.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>

#include <map>
#include <algorithm>
#include <functional>
#include "tools/urlobj.hxx"
#include "osl/file.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

////HELPER
::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl );

//  ----------------------------------------------------
//  class TabPageModel
//  ----------------------------------------------------

//TabPageModel::TabPageModel()
//{
//}
//TabPageModel::TabPageModel( uno::Reference< uno::XComponentContext > const & xCompContext)
//{
//  (void) xCompContext;
//}
//
//TabPageModel::~TabPageModel()
//{
//}
//
//////----- XInitialization -------------------------------------------------------------------
//void SAL_CALL TabPageModel::initialize (const Sequence<Any>& rArguments)
//{
//  sal_Int16 nPageId;
//  if ( rArguments.getLength() == 1 )
//    {
//         if ( !( rArguments[ 0 ] >>= nPageId ))
//             throw lang::IllegalArgumentException();
//        m_nTabPageId = nPageId;
//    }
//  else
//      m_nTabPageId = -1;
//}
//::sal_Int16 SAL_CALL TabPageModel::getTabPageID() throw (::com::sun::star::uno::RuntimeException)
//{
//  return m_nTabPageId;
//}
//::sal_Bool SAL_CALL TabPageModel::getEnabled() throw (::com::sun::star::uno::RuntimeException)
//{
//  return m_bEnabled;
//}
//void SAL_CALL TabPageModel::setEnabled( ::sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException)
//{
//  m_bEnabled = _enabled;
//}
//::rtl::OUString SAL_CALL TabPageModel::getTitle() throw (::com::sun::star::uno::RuntimeException)
//{
//  return m_sTitle;
//}
//void SAL_CALL TabPageModel::setTitle( const ::rtl::OUString& _title ) throw (::com::sun::star::uno::RuntimeException)
//{
//  m_sTitle = _title;
//}
//::rtl::OUString SAL_CALL TabPageModel::getImageURL() throw (::com::sun::star::uno::RuntimeException)
//{
//  return m_sImageURL;
//}
//void SAL_CALL TabPageModel::setImageURL( const ::rtl::OUString& _imageurl ) throw (::com::sun::star::uno::RuntimeException)
//{
//  m_sImageURL = _imageurl;
//}
//::rtl::OUString SAL_CALL TabPageModel::getTooltip() throw (::com::sun::star::uno::RuntimeException)
//{
//  return m_sTooltip;
//}
//void SAL_CALL TabPageModel::setTooltip( const ::rtl::OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException)
//{
//  m_sTooltip = _tooltip;
//}

//  ----------------------------------------------------
//  class UnoControlTabPageModel
//  ----------------------------------------------------
UnoControlTabPageModel::UnoControlTabPageModel( Reference< XMultiServiceFactory > const & i_factory )
    :ControlModelContainerBase( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
}

::rtl::OUString UnoControlTabPageModel::getServiceName( ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageModel );
}

Any UnoControlTabPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPage );
            break;
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& UnoControlTabPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}
// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlTabPageModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
////----- XInitialization -------------------------------------------------------------------
void SAL_CALL UnoControlTabPageModel::initialize (const Sequence<Any>& rArguments)
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
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
        ::rtl::OUString sURL;
        if ( !( rArguments[ 1 ] >>= sURL ))
            throw lang::IllegalArgumentException();
        Reference<container::XNameContainer > xDialogModel = awt::UnoControlDialogModelProvider::create( maContext.getUNOContext(),sURL);
        if ( xDialogModel.is() )
        {
            Sequence< ::rtl::OUString> aNames = xDialogModel->getElementNames();
            const ::rtl::OUString* pIter = aNames.getConstArray();
            const ::rtl::OUString* pEnd = pIter + aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                try
                {
                    Any aElement(xDialogModel->getByName(*pIter));
                    xDialogModel->removeByName(*pIter);
                    insertByName(*pIter,aElement);
                }
                catch(const Exception& ex)
                {
                    (void)ex;
                }
            }
            Reference<XPropertySet> xDialogProp(xDialogModel,UNO_QUERY);
            if ( xDialogProp.is() )
            {
                static const ::rtl::OUString s_sResourceResolver(RTL_CONSTASCII_USTRINGPARAM("ResourceResolver"));
                Reference<XPropertySet> xThis(*this,UNO_QUERY);
                xThis->setPropertyValue(s_sResourceResolver,xDialogProp->getPropertyValue(s_sResourceResolver));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_TITLE),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_TITLE)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_IMAGEURL),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_IMAGEURL)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_HELPTEXT),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_HELPTEXT)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_ENABLED),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_ENABLED)));
                xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_HELPURL),xDialogProp->getPropertyValue(GetPropertyName(BASEPROPERTY_HELPURL)));
            }
        }
    }
    else
        m_nTabPageId = -1;
}
//===== Service ===============================================================
::rtl::OUString UnoControlTabPageModel_getImplementationName (void) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.awt.tab.UnoControlTabPageModel");
}

Sequence<rtl::OUString> SAL_CALL UnoControlTabPageModel_getSupportedServiceNames (void)
     throw (RuntimeException)
{
     static const ::rtl::OUString sServiceName(
         ::rtl::OUString::createFromAscii("com.sun.star.awt.tab.UnoControlTabPageModel"));
     return Sequence<rtl::OUString>(&sServiceName, 1);
}
//=============================================================================
// = class UnoControlTabPage
// ============================================================================

UnoControlTabPage::UnoControlTabPage( const Reference< XMultiServiceFactory >& i_factory )
    :UnoControlTabPage_Base( i_factory )
    ,m_bWindowListener(false)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}
UnoControlTabPage::~UnoControlTabPage()
{
}

::rtl::OUString UnoControlTabPage::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "TabPageModel" );
}

void UnoControlTabPage::dispose() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    ControlContainerBase::dispose();
}

void SAL_CALL UnoControlTabPage::disposing(    const EventObject& Source )throw(RuntimeException)
{
     ControlContainerBase::disposing( Source );
}

void UnoControlTabPage::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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

static ::Size ImplMapPixelToAppFont( OutputDevice* pOutDev, const ::Size& aSize )
{
    ::Size aTmp = pOutDev->PixelToLogic( aSize, MAP_APPFONT );
    return aTmp;
}
// ::com::sun::star::awt::XWindowListener
void SAL_CALL UnoControlTabPage::windowResized( const ::com::sun::star::awt::WindowEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev && !mbSizeModified )
    {
        // Currentley we are simply using MAP_APPFONT
        ::Size aAppFontSize( e.Width, e.Height );

        Reference< XControl > xDialogControl( *this, UNO_QUERY_THROW );
        Reference< XDevice > xDialogDevice( xDialogControl->getPeer(), UNO_QUERY );
        OSL_ENSURE( xDialogDevice.is(), "UnoDialogControl::windowResized: no peer, but a windowResized event?" );
        if ( xDialogDevice.is() )
        {
            DeviceInfo aDeviceInfo( xDialogDevice->getInfo() );
            aAppFontSize.Width() -= aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
            aAppFontSize.Height() -= aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
        }

        aAppFontSize = ImplMapPixelToAppFont( pOutDev, aAppFontSize );

        // Remember that changes have been done by listener. No need to
        // update the position because of property change event.
        mbSizeModified = true;
        Sequence< rtl::OUString > aProps( 2 );
        Sequence< Any > aValues( 2 );
        // Properties in a sequence must be sorted!
        aProps[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ));
        aProps[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width"  ));
        aValues[0] <<= aAppFontSize.Height();
        aValues[1] <<= aAppFontSize.Width();

        ImplSetPropertyValues( aProps, aValues, true );
        mbSizeModified = false;
    }
}

void SAL_CALL UnoControlTabPage::windowMoved( const ::com::sun::star::awt::WindowEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev && !mbPosModified )
    {
        // Currentley we are simply using MAP_APPFONT
        Any    aAny;
        ::Size aTmp( e.X, e.Y );
        aTmp = ImplMapPixelToAppFont( pOutDev, aTmp );

        // Remember that changes have been done by listener. No need to
        // update the position because of property change event.
        mbPosModified = true;
        Sequence< rtl::OUString > aProps( 2 );
        Sequence< Any > aValues( 2 );
        aProps[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX"  ));
        aProps[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ));
        aValues[0] <<= aTmp.Width();
        aValues[1] <<= aTmp.Height();

        ImplSetPropertyValues( aProps, aValues, true );
        mbPosModified = false;
    }
}

void SAL_CALL UnoControlTabPage::windowShown( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    (void)e;
}

void SAL_CALL UnoControlTabPage::windowHidden( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    (void)e;
}
