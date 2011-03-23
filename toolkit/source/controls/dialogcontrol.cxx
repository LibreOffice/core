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
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
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
#include <map>
#include <algorithm>
#include <functional>
#include "tools/urlobj.hxx"
#include "osl/file.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

#define PROPERTY_DIALOGSOURCEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogSourceURL" ))
#define PROPERTY_IMAGEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageURL" ))
#define PROPERTY_GRAPHIC ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ))
//
////HELPER
::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl );

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
UnoControlDialogModel::UnoControlDialogModel( const Reference< XMultiServiceFactory >& i_factory )
    :ControlModelContainerBase( i_factory )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
//  ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
//  ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    ImplRegisterProperty( BASEPROPERTY_DESKTOP_AS_PARENT );
    ImplRegisterProperty( BASEPROPERTY_DECORATION );
    ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );
    ImplRegisterProperty( BASEPROPERTY_GRAPHIC );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    : ControlModelContainerBase( rModel )
{
}

UnoControlDialogModel::~UnoControlDialogModel()
{
}

UnoControlModel* UnoControlDialogModel::Clone() const
{
    // clone the container itself
    UnoControlDialogModel* pClone = new UnoControlDialogModel( *this );

    Clone_Impl(*pClone);

    return pClone;
}


::rtl::OUString UnoControlDialogModel::getServiceName( ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialogModel );
}

Any UnoControlDialogModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialog );
            break;
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& UnoControlDialogModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoControlDialogModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// ============================================================================
// = class UnoDialogControl
// ============================================================================

UnoDialogControl::UnoDialogControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
    :UnoDialogControl_Base( i_factory )
    ,maTopWindowListeners( *this )
    ,mbWindowListener(false)
{
    maComponentInfos.nWidth = 300;
    maComponentInfos.nHeight = 450;
 }

UnoDialogControl::~UnoDialogControl()
{
}

::rtl::OUString UnoDialogControl::GetComponentServiceName()
{

    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return ::rtl::OUString::createFromAscii( "Dialog" );
    else
        return ::rtl::OUString::createFromAscii( "TabPage" );
}

void UnoDialogControl::dispose() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    maTopWindowListeners.disposeAndClear( aEvt );
    ControlContainerBase::dispose();
}

void SAL_CALL UnoDialogControl::disposing(
    const EventObject& Source )
throw(RuntimeException)
{
    ControlContainerBase::disposing( Source );
}

sal_Bool UnoDialogControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
        // #Can we move all the Resource stuff to the ControlContainerBase ?
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        sal_Bool bRet = ControlContainerBase::setModel( rxModel );
    ImplStartListingForResourceEvents();
    return bRet;
}

void UnoDialogControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    Reference < XTopWindow > xTW( getPeer(), UNO_QUERY );
    if ( xTW.is() )
    {
        xTW->setMenuBar( mxMenuBar );

        if ( !mbWindowListener )
        {
            Reference< XWindowListener > xWL( static_cast< cppu::OWeakObject*>( this ), UNO_QUERY );
            addWindowListener( xWL );
            mbWindowListener = true;
        }

        if ( maTopWindowListeners.getLength() )
            xTW->addTopWindowListener( &maTopWindowListeners );
    }
}

void UnoDialogControl::PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc )
{
    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( !bDecoration )
    {
        // Now we have to manipulate the WindowDescriptor
        rDesc.WindowAttributes = rDesc.WindowAttributes | ::com::sun::star::awt::WindowAttribute::NODECORATION;
    }

    // We have to set the graphic property before the peer
    // will be created. Otherwise the properties will be copied
    // into the peer via propertiesChangeEvents. As the order of
    // can lead to overwrites we have to set the graphic property
    // before the propertiesChangeEvents are sent!
    ::rtl::OUString aImageURL;
    Reference< graphic::XGraphic > xGraphic;
    if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
        ( aImageURL.getLength() > 0 ))
    {
        ::rtl::OUString absoluteUrl =
            getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                 ImplGetPropertyValue( PROPERTY_IMAGEURL ));

        xGraphic = ControlContainerBase::Impl_getGraphicFromURL_nothrow( absoluteUrl );
        ImplSetPropertyValue( PROPERTY_GRAPHIC, uno::makeAny( xGraphic ), sal_True );
    }
}

void UnoDialogControl::addTopWindowListener( const Reference< XTopWindowListener >& rxListener ) throw (RuntimeException)
{
    maTopWindowListeners.addInterface( rxListener );
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->addTopWindowListener( &maTopWindowListeners );
    }
}

void UnoDialogControl::removeTopWindowListener( const Reference< XTopWindowListener >& rxListener ) throw (RuntimeException)
{
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->removeTopWindowListener( &maTopWindowListeners );
    }
    maTopWindowListeners.removeInterface( rxListener );
}

void UnoDialogControl::toFront(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    mxMenuBar = rxMenuBar;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->setMenuBar( mxMenuBar );
    }
}
static ::Size ImplMapPixelToAppFont( OutputDevice* pOutDev, const ::Size& aSize )
{
    ::Size aTmp = pOutDev->PixelToLogic( aSize, MAP_APPFONT );
    return aTmp;
}
// ::com::sun::star::awt::XWindowListener
void SAL_CALL UnoDialogControl::windowResized( const ::com::sun::star::awt::WindowEvent& e )
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

void SAL_CALL UnoDialogControl::windowMoved( const ::com::sun::star::awt::WindowEvent& e )
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

void SAL_CALL UnoDialogControl::windowShown( const EventObject& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL UnoDialogControl::windowHidden( const EventObject& e ) throw (RuntimeException)
{
    (void)e;
}

void SAL_CALL UnoDialogControl::endDialog( ::sal_Int32 i_result ) throw (RuntimeException)
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->endDialog( i_result );
}

void SAL_CALL UnoDialogControl::setHelpId( const rtl::OUString& i_id ) throw (RuntimeException)
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->setHelpId( i_id );
}

void UnoDialogControl::setTitle( const ::rtl::OUString& Title ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    sal_Int16 nDone = -1;
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            GetComponentInfos().bVisible = sal_True;
            nDone = xDlg->execute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
    return nDone;
}

void UnoDialogControl::endExecute() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            xDlg->endExecute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
}

// XModifyListener
void SAL_CALL UnoDialogControl::modified(
    const lang::EventObject& /*rEvent*/ )
throw (RuntimeException)
{
    ImplUpdateResourceResolver();
}

void UnoDialogControl::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
{
    sal_Int32 nLen = rEvents.getLength();
    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
        Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
        sal_Bool bOwnModel = (XControlModel*)xModel.get() == (XControlModel*)getModel().get();
        if ( bOwnModel && rEvt.PropertyName.equalsAsciiL( "ImageURL", 8 ))
        {
            ::rtl::OUString aImageURL;
            Reference< graphic::XGraphic > xGraphic;
            if (( ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ) ) >>= aImageURL ) &&
                ( aImageURL.getLength() > 0 ))
            {
                ::rtl::OUString absoluteUrl =
                    getPhysicalLocation( ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL )),
                                         uno::makeAny(aImageURL));

                xGraphic = Impl_getGraphicFromURL_nothrow( absoluteUrl );
            }
            ImplSetPropertyValue(  GetPropertyName( BASEPROPERTY_GRAPHIC), uno::makeAny( xGraphic ), sal_True );
            break;
        }
    }
    ControlContainerBase::ImplModelPropertiesChanged(rEvents);
}
