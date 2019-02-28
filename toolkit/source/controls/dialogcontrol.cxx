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


#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <osl/mutex.hxx>
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/sequence.hxx>
#include <vcl/outdev.hxx>

#include <vcl/graph.hxx>
#include <vcl/image.hxx>
#include <cppuhelper/implbase.hxx>
#include <algorithm>
#include <functional>
#include <map>
#include <unordered_map>
#include <osl/file.hxx>

#include <vcl/tabctrl.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/controls/unocontrols.hxx>

#include <helper/unopropertyarrayhelper.hxx>
#include "controlmodelcontainerbase_internal.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

#define PROPERTY_DIALOGSOURCEURL "DialogSourceURL"
#define PROPERTY_IMAGEURL "ImageURL"
#define PROPERTY_GRAPHIC "Graphic"


// we probably will need both a hash of control models and hash of controls
// => use some template magic

template< typename T >
class SimpleNamedThingContainer : public ::cppu::WeakImplHelper< container::XNameContainer >
{
    std::unordered_map< OUString, Reference< T > > things;
    ::osl::Mutex m_aMutex;
public:
    // css::container::XNameContainer, XNameReplace, XNameAccess
    virtual void SAL_CALL replaceByName( const OUString& aName, const Any& aElement ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual Any SAL_CALL getByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        return uno::makeAny( things[ aName ] );
    }
    virtual Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return comphelper::mapKeysToSequence( things );
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( things.find( aName ) != things.end() );
    }
    virtual void SAL_CALL insertByName( const OUString& aName, const Any& aElement ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw ElementExistException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( things.erase( aName ) == 0 )
            throw NoSuchElementException();
    }
    virtual Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<T>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( !things.empty() );
    }
};

namespace {

class UnoControlDialogModel :   public ControlModelContainerBase
{
protected:
    css::uno::Reference< css::graphic::XGraphicObject > mxGrfObj;
    css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;
    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
public:
    explicit UnoControlDialogModel( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    UnoControlDialogModel( const UnoControlDialogModel& rModel );

    rtl::Reference<UnoControlModel> Clone() const override;
    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return OUString("stardiv.Toolkit.UnoControlDialogModel"); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        auto s(ControlModelContainerBase::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlDialogModel";
        s[s.getLength() - 1] = "stardiv.vcl.controlmodel.Dialog";
        return s;
    }
};

UnoControlDialogModel::UnoControlDialogModel( const Reference< XComponentContext >& rxContext )
    :ControlModelContainerBase( rxContext )
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
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
    ImplRegisterProperty( BASEPROPERTY_SCROLLWIDTH );
    ImplRegisterProperty( BASEPROPERTY_SCROLLHEIGHT );
    ImplRegisterProperty( BASEPROPERTY_SCROLLTOP );
    ImplRegisterProperty( BASEPROPERTY_SCROLLLEFT );

    Any aBool;
    aBool <<= true;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    // #TODO separate class for 'UserForm' ( instead of re-using Dialog ? )
    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >;
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    : ControlModelContainerBase( rModel )
{
    // need to clone BASEPROPERTY_USERFORMCONTAINEES too
    Reference< XNameContainer > xSrcNameCont( const_cast< UnoControlDialogModel& >(rModel).getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );
    Reference<XNameContainer > xNameCont( new SimpleNamedThingContainer< XControlModel > );

    uno::Sequence< OUString > sNames = xSrcNameCont->getElementNames();
    for ( OUString const & name : sNames )
    {
        if ( xSrcNameCont->hasByName( name ) )
            xNameCont->insertByName( name, xSrcNameCont->getByName( name ) );
    }
    setFastPropertyValue_NoBroadcast( BASEPROPERTY_USERFORMCONTAINEES, makeAny( xNameCont ) );
}

rtl::Reference<UnoControlModel> UnoControlDialogModel::Clone() const
{
    // clone the container itself
    UnoControlDialogModel* pClone = new UnoControlDialogModel( *this );

    Clone_Impl(*pClone);

    return pClone;
}


OUString UnoControlDialogModel::getServiceName( )
{
    return OUString("stardiv.vcl.controlmodel.Dialog");
}

Any UnoControlDialogModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= OUString::createFromAscii( szServiceName_UnoControlDialog );
            break;
        case BASEPROPERTY_SCROLLWIDTH:
        case BASEPROPERTY_SCROLLHEIGHT:
        case BASEPROPERTY_SCROLLTOP:
        case BASEPROPERTY_SCROLLLEFT:
            aAny <<= sal_Int32(0);
            break;
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& UnoControlDialogModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoControlDialogModel::getPropertySetInfo(  )
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void SAL_CALL UnoControlDialogModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
{
    ControlModelContainerBase::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    try
    {
        if ( nHandle == BASEPROPERTY_IMAGEURL && ImplHasProperty( BASEPROPERTY_GRAPHIC ) )
        {
            OUString sImageURL;
            OSL_VERIFY( rValue >>= sImageURL );
            setPropertyValue( GetPropertyName( BASEPROPERTY_GRAPHIC ), uno::makeAny( ImageHelper::getGraphicAndGraphicObjectFromURL_nothrow( mxGrfObj, sImageURL ) ) );
        }
    }
    catch( const css::uno::Exception& )
    {
        OSL_ENSURE( false, "UnoControlDialogModel::setFastPropertyValue_NoBroadcast: caught an exception while setting ImageURL properties!" );
    }
}

}


// = class UnoDialogControl


UnoDialogControl::UnoDialogControl( const uno::Reference< uno::XComponentContext >& rxContext )
    :UnoDialogControl_Base( rxContext )
    ,maTopWindowListeners( *this )
    ,mbWindowListener(false)
{
    maComponentInfos.nWidth = 300;
    maComponentInfos.nHeight = 450;
 }

UnoDialogControl::~UnoDialogControl()
{
}

OUString UnoDialogControl::GetComponentServiceName()
{

    bool bDecoration( true );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return OUString("Dialog");
    else
        return OUString("TabPage");
}

void UnoDialogControl::dispose()
{
    SolarMutexGuard aGuard;

    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    maTopWindowListeners.disposeAndClear( aEvt );
    ControlContainerBase::dispose();
}

void SAL_CALL UnoDialogControl::disposing(
    const EventObject& Source )
{
    ControlContainerBase::disposing( Source );
}

sal_Bool UnoDialogControl::setModel( const Reference< XControlModel >& rxModel )
{
        // #Can we move all the Resource stuff to the ControlContainerBase ?
    SolarMutexGuard aGuard;
    bool bRet = ControlContainerBase::setModel( rxModel );
    ImplStartListingForResourceEvents();
    return bRet;
}

void UnoDialogControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer )
{
    SolarMutexGuard aGuard;

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
        // there must be a better way than doing this, we can't
        // process the scrolltop & scrollleft in XDialog because
        // the children haven't been added when those props are applied
        ImplSetPeerProperty( GetPropertyName( BASEPROPERTY_SCROLLTOP ), ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLTOP ) ) );
        ImplSetPeerProperty( GetPropertyName( BASEPROPERTY_SCROLLLEFT ), ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLLEFT ) ) );

    }
}

OUString UnoDialogControl::getImplementationName()
{
    return OUString("stardiv.Toolkit.UnoDialogControl");
}

sal_Bool UnoDialogControl::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UnoDialogControl::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        OUString::createFromAscii(szServiceName2_UnoControlDialog),
        "stardiv.vcl.control.Dialog"};
}

void UnoDialogControl::PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc )
{
    UnoControlContainer::PrepareWindowDescriptor( rDesc );
    bool bDecoration( true );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( !bDecoration )
    {
        // Now we have to manipulate the WindowDescriptor
        rDesc.WindowAttributes = rDesc.WindowAttributes | css::awt::WindowAttribute::NODECORATION;
    }

    // We have to set the graphic property before the peer
    // will be created. Otherwise the properties will be copied
    // into the peer via propertiesChangeEvents. As the order of
    // can lead to overwrites we have to set the graphic property
    // before the propertiesChangeEvents are sent!
    OUString aImageURL;
    Reference< graphic::XGraphic > xGraphic;
    if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
        ( !aImageURL.isEmpty() ))
    {
        OUString absoluteUrl = getPhysicalLocation(ImplGetPropertyValue(PROPERTY_DIALOGSOURCEURL), uno::makeAny(aImageURL));
        xGraphic = ImageHelper::getGraphicFromURL_nothrow( absoluteUrl );
        ImplSetPropertyValue( PROPERTY_GRAPHIC, uno::makeAny( xGraphic ), true );
    }
}

void UnoDialogControl::addTopWindowListener( const Reference< XTopWindowListener >& rxListener )
{
    maTopWindowListeners.addInterface( rxListener );
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->addTopWindowListener( &maTopWindowListeners );
    }
}

void UnoDialogControl::removeTopWindowListener( const Reference< XTopWindowListener >& rxListener )
{
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->removeTopWindowListener( &maTopWindowListeners );
    }
    maTopWindowListeners.removeInterface( rxListener );
}

void UnoDialogControl::toFront(  )
{
    SolarMutexGuard aGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  )
{
    SolarMutexGuard aGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar )
{
    SolarMutexGuard aGuard;
    mxMenuBar = rxMenuBar;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->setMenuBar( mxMenuBar );
    }
}
static ::Size ImplMapPixelToAppFont( OutputDevice const * pOutDev, const ::Size& aSize )
{
    ::Size aTmp = pOutDev->PixelToLogic(aSize, MapMode(MapUnit::MapAppFont));
    return aTmp;
}
// css::awt::XWindowListener
void SAL_CALL UnoDialogControl::windowResized( const css::awt::WindowEvent& e )
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( !pOutDev || mbSizeModified )
        return;

    // Currently we are simply using MapUnit::MapAppFont
    ::Size aAppFontSize( e.Width, e.Height );

    Reference< XControl > xDialogControl( *this, UNO_QUERY_THROW );
    Reference< XDevice > xDialogDevice( xDialogControl->getPeer(), UNO_QUERY );
    OSL_ENSURE( xDialogDevice.is(), "UnoDialogControl::windowResized: no peer, but a windowResized event?" );

    // #i87592 In design mode the drawing layer works with sizes with decoration.
    // Therefore we have to subtract them before writing back to the properties (model).
    if ( xDialogDevice.is() && mbDesignMode )
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

void SAL_CALL UnoDialogControl::windowMoved( const css::awt::WindowEvent& e )
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

void SAL_CALL UnoDialogControl::windowShown( const EventObject& ) {}

void SAL_CALL UnoDialogControl::windowHidden( const EventObject& ) {}

void SAL_CALL UnoDialogControl::endDialog( ::sal_Int32 i_result )
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->endDialog( i_result );
}

void SAL_CALL UnoDialogControl::setHelpId( const OUString& i_id )
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->setHelpId( i_id );
}

void UnoDialogControl::setTitle( const OUString& Title )
{
    SolarMutexGuard aGuard;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), uno::Any(Title), true );
}

OUString UnoDialogControl::getTitle()
{
    SolarMutexGuard aGuard;
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute()
{
    SolarMutexGuard aGuard;
    sal_Int16 nDone = -1;
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            GetComponentInfos().bVisible = true;
            nDone = xDlg->execute();
            GetComponentInfos().bVisible = false;
        }
    }
    return nDone;
}

void UnoDialogControl::endExecute()
{
    SolarMutexGuard aGuard;
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            xDlg->endExecute();
            GetComponentInfos().bVisible = false;
        }
    }
}

// XModifyListener
void SAL_CALL UnoDialogControl::modified(
    const lang::EventObject& /*rEvent*/ )
{
    ImplUpdateResourceResolver();
}

void UnoDialogControl::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents )
{
    sal_Int32 nLen = rEvents.getLength();
    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
        Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
        bool bOwnModel = xModel.get() == getModel().get();
        if ( bOwnModel && rEvt.PropertyName == "ImageURL" )
        {
            OUString aImageURL;
            Reference< graphic::XGraphic > xGraphic;
            if (( ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ) ) >>= aImageURL ) &&
                ( !aImageURL.isEmpty() ))
            {
                OUString absoluteUrl = getPhysicalLocation(ImplGetPropertyValue(GetPropertyName(BASEPROPERTY_DIALOGSOURCEURL)), uno::makeAny(aImageURL));
                xGraphic = ImageHelper::getGraphicFromURL_nothrow( absoluteUrl );
            }
            ImplSetPropertyValue(  GetPropertyName( BASEPROPERTY_GRAPHIC), uno::makeAny( xGraphic ), true );
            break;
        }
    }
    ControlContainerBase::ImplModelPropertiesChanged(rEvents);
}


//  class MultiPageControl

UnoMultiPageControl::UnoMultiPageControl( const uno::Reference< uno::XComponentContext >& rxContext ) : ControlContainerBase(rxContext), maTabListeners( *this )
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoMultiPageControl::~UnoMultiPageControl()
{
}
// XTabListener

void SAL_CALL UnoMultiPageControl::inserted( SAL_UNUSED_PARAMETER ::sal_Int32 )
{
}
void SAL_CALL UnoMultiPageControl::removed( SAL_UNUSED_PARAMETER ::sal_Int32 )
{
}
void SAL_CALL UnoMultiPageControl::changed( SAL_UNUSED_PARAMETER ::sal_Int32,
                                            SAL_UNUSED_PARAMETER const Sequence< NamedValue >& )
{
}
void SAL_CALL UnoMultiPageControl::activated( ::sal_Int32 ID )
{
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), false );

}
void SAL_CALL UnoMultiPageControl::deactivated( SAL_UNUSED_PARAMETER ::sal_Int32 )
{
}
void SAL_CALL UnoMultiPageControl::disposing(const EventObject&)
{
}

void SAL_CALL UnoMultiPageControl::dispose()
{
    lang::EventObject aEvt;
    aEvt.Source = static_cast<cppu::OWeakObject*>(this);
    maTabListeners.disposeAndClear( aEvt );
    ControlContainerBase::dispose();
}

// css::awt::XSimpleTabController
::sal_Int32 SAL_CALL UnoMultiPageControl::insertTab()
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    return xMultiPage->insertTab();
}

void SAL_CALL UnoMultiPageControl::removeTab( ::sal_Int32 ID )
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    xMultiPage->removeTab( ID );
}

void SAL_CALL UnoMultiPageControl::setTabProps( ::sal_Int32 ID, const Sequence< NamedValue >& Properties )
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    xMultiPage->setTabProps( ID, Properties );
}

Sequence< NamedValue > SAL_CALL UnoMultiPageControl::getTabProps( ::sal_Int32 ID )
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    return xMultiPage->getTabProps( ID );
}

void SAL_CALL UnoMultiPageControl::activateTab( ::sal_Int32 ID )
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    xMultiPage->activateTab( ID );
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), true );

}

::sal_Int32 SAL_CALL UnoMultiPageControl::getActiveTabID()
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY_THROW );
    return xMultiPage->getActiveTabID();
}

void SAL_CALL UnoMultiPageControl::addTabListener( const Reference< XTabListener >& Listener )
{
    maTabListeners.addInterface( Listener );
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->addTabListener( &maTabListeners );
}

void SAL_CALL UnoMultiPageControl::removeTabListener( const Reference< XTabListener >& Listener )
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->removeTabListener( &maTabListeners );
    maTabListeners.removeInterface( Listener );
}

IMPL_IMPLEMENTATION_ID( UnoMultiPageControl )

// lang::XTypeProvider
css::uno::Sequence< css::uno::Type > UnoMultiPageControl::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<awt::XSimpleTabController>::get(),
        cppu::UnoType<awt::XTabListener>::get(),
        ControlContainerBase::getTypes()
    );
    return aTypeList.getTypes();
}

// uno::XInterface
uno::Any UnoMultiPageControl::queryAggregation( const uno::Type & rType )
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< awt::XTabListener* >(this),
                                        static_cast< awt::XSimpleTabController* >(this) );
    return (aRet.hasValue() ? aRet : ControlContainerBase::queryAggregation( rType ));
}

OUString UnoMultiPageControl::GetComponentServiceName()
{
    bool bDecoration( true );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return OUString("tabcontrol");
    // Hopefully we can tweak the tabcontrol to display without tabs
    return OUString("tabcontrolnotabs");
}

void UnoMultiPageControl::bindPage( const uno::Reference< awt::XControl >& _rxControl )
{
    uno::Reference< awt::XWindowPeer > xPage( _rxControl->getPeer() );
    uno::Reference< awt::XSimpleTabController > xTabCntrl( getPeer(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProps( _rxControl->getModel(), uno::UNO_QUERY );

    VCLXTabPage* pXPage = dynamic_cast< VCLXTabPage* >( xPage.get() );
    TabPage* pPage = pXPage ? pXPage->getTabPage() : nullptr;
    if ( xTabCntrl.is() && pPage )
    {
        VCLXMultiPage* pXTab = dynamic_cast< VCLXMultiPage* >( xTabCntrl.get() );
        if ( pXTab )
        {
            OUString sTitle;
            xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ) ) >>= sTitle;
            pXTab->insertTab( pPage, sTitle);
        }
    }

}

void UnoMultiPageControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer )
{
    SolarMutexGuard aSolarGuard;

    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();
    sal_uInt32 nCtrls = aCtrls.getLength();
    for( sal_uInt32 n = 0; n < nCtrls; n++ )
       bindPage( aCtrls[ n ] );
    sal_Int32 nActiveTab(0);
    Reference< XPropertySet > xMultiProps( getModel(), UNO_QUERY );
    xMultiProps->getPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ) ) >>= nActiveTab;

    uno::Reference< awt::XSimpleTabController > xTabCntrl( getPeer(), uno::UNO_QUERY );
    if ( xTabCntrl.is() )
    {
        xTabCntrl->addTabListener( this );
        if ( nActiveTab && nCtrls ) // Ensure peer is initialise with correct activated tab
        {
            xTabCntrl->activateTab( nActiveTab );
            ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( nActiveTab ), true );
        }
    }
}

void    UnoMultiPageControl::impl_createControlPeerIfNecessary( const uno::Reference< awt::XControl >& _rxControl)
{
    OSL_PRECOND( _rxControl.is(), "UnoMultiPageControl::impl_createControlPeerIfNecessary: invalid control, this will crash!" );

    // if the container already has a peer, then also create a peer for the control
    uno::Reference< awt::XWindowPeer > xMyPeer( getPeer() );

    if( xMyPeer.is() )
    {
        _rxControl->createPeer( nullptr, xMyPeer );
        bindPage( _rxControl );
        ImplActivateTabControllers();
    }

}

// ------------- UnoMultiPageModel -----------------

UnoMultiPageModel::UnoMultiPageModel( const Reference< XComponentContext >& rxContext ) : ControlModelContainerBase( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );

    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    //ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );
    ImplRegisterProperty( BASEPROPERTY_MULTIPAGEVALUE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );

    Any aBool;
    aBool <<= true;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_DECORATION, aBool );
    // MultiPage Control has the tab stop property. And the default value is True.
    ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );

    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >;
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoMultiPageModel::~UnoMultiPageModel()
{
}

rtl::Reference<UnoControlModel> UnoMultiPageModel::Clone() const
{
    // clone the container itself
    UnoMultiPageModel* pClone = new UnoMultiPageModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

OUString UnoMultiPageModel::getServiceName()
{
    return OUString( "com.sun.star.awt.UnoMultiPageModel" );
}

uno::Any UnoMultiPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        return uno::Any( OUString( "com.sun.star.awt.UnoControlMultiPage" ) );
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoMultiPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoMultiPageModel::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoMultiPageModel::insertByName( const OUString& aName, const Any& aElement )
{
    Reference< XServiceInfo > xInfo;
    aElement >>= xInfo;

    if ( !xInfo.is() )
        throw IllegalArgumentException();

    // Only a Page model can be inserted into the multipage
    if ( !xInfo->supportsService( "com.sun.star.awt.UnoPageModel" ) )
        throw IllegalArgumentException();

    return ControlModelContainerBase::insertByName( aName, aElement );
}


sal_Bool SAL_CALL UnoMultiPageModel::getGroupControl(  )
{
    return true;
}


//  class UnoPageControl

UnoPageControl::UnoPageControl( const uno::Reference< uno::XComponentContext >& rxContext ) : ControlContainerBase(rxContext)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoPageControl::~UnoPageControl()
{
}

OUString UnoPageControl::GetComponentServiceName()
{
    return OUString("tabpage");
}


// ------------- UnoPageModel -----------------

UnoPageModel::UnoPageModel( const Reference< XComponentContext >& rxContext ) : ControlModelContainerBase( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );

    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );
//    ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );

    Any aBool;
    aBool <<= true;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    //ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );

    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >;
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoPageModel::~UnoPageModel()
{
}

rtl::Reference<UnoControlModel> UnoPageModel::Clone() const
{
    // clone the container itself
    UnoPageModel* pClone = new UnoPageModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

OUString UnoPageModel::getServiceName()
{
    return OUString( "com.sun.star.awt.UnoPageModel" );
}

uno::Any UnoPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        return uno::Any( OUString( "com.sun.star.awt.UnoControlPage" ) );
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoPageModel::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


sal_Bool SAL_CALL UnoPageModel::getGroupControl(  )
{
    return false;
}

// Frame control


//  class UnoFrameControl

UnoFrameControl::UnoFrameControl( const uno::Reference< uno::XComponentContext >& rxContext ) : ControlContainerBase(rxContext)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoFrameControl::~UnoFrameControl()
{
}

OUString UnoFrameControl::GetComponentServiceName()
{
    return OUString("frame");
}

void UnoFrameControl::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    bool bOwnCtrl = false;
    OUString sTitle;
    if ( rxCtrl.get() == Reference<XControl>( this ).get() )
        bOwnCtrl = true;
    Reference< XPropertySet > xProps( getModel(), UNO_QUERY );
    //xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ) ) >>= sTitle;
    xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ) ) >>= sTitle;

    ControlContainerBase::ImplSetPosSize( rxCtrl );
    Reference < XWindow > xW( rxCtrl, UNO_QUERY );
    if ( !bOwnCtrl && xW.is() && !sTitle.isEmpty() )
    {
        awt::Rectangle aSizePos = xW->getPosSize();

        sal_Int32 nX = aSizePos.X, nY = aSizePos.Y, nWidth = aSizePos.Width, nHeight = aSizePos.Height;
        // Retrieve the values set by the base class
        OutputDevice*pOutDev = Application::GetDefaultDevice();
        if ( pOutDev )
        {
            // Adjust Y based on height of Title
            ::tools::Rectangle aRect;
            aRect = pOutDev->GetTextRect( aRect, sTitle );
            nY = nY + ( aRect.GetHeight() / 2 );
        }
        else
        {
            Reference< XWindowPeer > xPeer = ImplGetCompatiblePeer();
            Reference< XDevice > xD( xPeer, UNO_QUERY );

            SimpleFontMetric aFM;
            FontDescriptor aFD;
            Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ) );

            aVal >>= aFD;
            if ( !aFD.StyleName.isEmpty() )
            {
                Reference< XFont > xFont = xD->getFont( aFD );
                aFM = xFont->getFontMetric();
            }
            else
            {
                Reference< XGraphics > xG = xD->createGraphics();
                aFM = xG->getFontMetric();
            }

            sal_Int16 nH = aFM.Ascent + aFM.Descent;
            // offset y based on height of font ( not sure if my guess at the correct calculation is correct here )
            nY = nY + ( nH / 8); // how do I test this
        }
        xW->setPosSize( nX, nY, nWidth, nHeight, PosSize::POSSIZE );
    }
}

// ------------- UnoFrameModel -----------------

UnoFrameModel::UnoFrameModel(  const Reference< XComponentContext >& rxContext ) : ControlModelContainerBase( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
    ImplRegisterProperty( BASEPROPERTY_SCROLLWIDTH );
    ImplRegisterProperty( BASEPROPERTY_SCROLLHEIGHT );
    ImplRegisterProperty( BASEPROPERTY_SCROLLTOP );
    ImplRegisterProperty( BASEPROPERTY_SCROLLLEFT );


    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >;
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoFrameModel::~UnoFrameModel()
{
}

rtl::Reference<UnoControlModel> UnoFrameModel::Clone() const
{
    // clone the container itself
    UnoFrameModel* pClone = new UnoFrameModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

OUString UnoFrameModel::getServiceName()
{
    return OUString( "com.sun.star.awt.UnoFrameModel" );
}

uno::Any UnoFrameModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
        {
            return uno::Any( OUString( "com.sun.star.awt.UnoControlFrame" ) );
        }
        case BASEPROPERTY_SCROLLWIDTH:
        case BASEPROPERTY_SCROLLHEIGHT:
        case BASEPROPERTY_SCROLLTOP:
        case BASEPROPERTY_SCROLLLEFT:
            return uno::Any( sal_Int32(0) );
        case BASEPROPERTY_USERFORMCONTAINEES:
        {
            uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >;
            return makeAny( xNameCont );
        }
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoFrameModel::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoFrameModel::getPropertySetInfo(  )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoControlDialogModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OGeometryControlModel<UnoControlDialogModel>(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoDialogControl_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoDialogControl(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoMultiPageControl_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoMultiPageControl(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoMultiPageModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoMultiPageModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoPageControl_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoPageControl(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoPageModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoPageModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoFrameControl_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoFrameControl(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_UnoFrameModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UnoFrameModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
