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


#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <osl/mutex.hxx>
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
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
#include <boost/unordered_map.hpp>
#include <cppuhelper/implbase1.hxx>
#include <algorithm>
#include <functional>
#include "osl/file.hxx"

#include <vcl/tabctrl.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include "toolkit/controls/unocontrols.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

#define PROPERTY_DIALOGSOURCEURL ::rtl::OUString( "DialogSourceURL" )
#define PROPERTY_IMAGEURL ::rtl::OUString( "ImageURL" )
#define PROPERTY_GRAPHIC ::rtl::OUString( "Graphic" )
//

// we probably will need both a hash of control models and hash of controls
// => use some template magic

typedef ::cppu::WeakImplHelper1< container::XNameContainer > SimpleNameContainer_BASE;

template< typename T >
class SimpleNamedThingContainer : public SimpleNameContainer_BASE
{
    typedef boost::unordered_map< rtl::OUString, Reference< T >, ::rtl::OUStringHash,
       ::std::equal_to< ::rtl::OUString > > NamedThingsHash;
    NamedThingsHash things;
    ::osl::Mutex m_aMutex;
public:
    // ::com::sun::star::container::XNameContainer, XNameReplace, XNameAccess
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        return uno::makeAny( things[ aName ] );
    }
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Sequence< ::rtl::OUString > aResult( things.size() );
        typename NamedThingsHash::iterator it = things.begin();
        typename NamedThingsHash::iterator it_end = things.end();
        rtl::OUString* pName = aResult.getArray();
        for (; it != it_end; ++it, ++pName )
            *pName = it->first;
        return aResult;
    }
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( things.find( aName ) != things.end() );
    }
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw ElementExistException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        things.erase( things.find( aName ) );
    }
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException)
    {
        return T::static_type( NULL );
    }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( things.size() > 0 );
    }
};

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
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    // #TODO separate class for 'UserForm' ( instead of re-using Dialog ? )
    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >();
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    : ControlModelContainerBase( rModel )
{
    // need to clone BASEPROPERTY_USERFORMCONTAINEES too
    Reference< XNameContainer > xSrcNameCont( const_cast< UnoControlDialogModel& >(rModel).getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );
    Reference<XNameContainer > xNameCont( new SimpleNamedThingContainer< XControlModel >() );

    uno::Sequence< rtl::OUString > sNames = xSrcNameCont->getElementNames();
    rtl::OUString* pName = sNames.getArray();
    rtl::OUString* pNamesEnd = pName + sNames.getLength();
    for ( ; pName != pNamesEnd; ++pName )
    {
        if ( xSrcNameCont->hasByName( *pName ) )
            xNameCont->insertByName( *pName, xSrcNameCont->getByName( *pName ) );
    }
    setFastPropertyValue_NoBroadcast( BASEPROPERTY_USERFORMCONTAINEES, makeAny( xNameCont ) );
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

void SAL_CALL UnoControlDialogModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    ControlModelContainerBase::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    try
    {
        if ( nHandle == BASEPROPERTY_IMAGEURL && ImplHasProperty( BASEPROPERTY_GRAPHIC ) )
        {
            ::rtl::OUString sImageURL;
            OSL_VERIFY( rValue >>= sImageURL );
            setPropertyValue( GetPropertyName( BASEPROPERTY_GRAPHIC ), uno::makeAny( ImageHelper::getGraphicAndGraphicObjectFromURL_nothrow( mxGrfObj, sImageURL ) ) );
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        OSL_ENSURE( sal_False, "UnoControlDialogModel::setFastPropertyValue_NoBroadcast: caught an exception while setting ImageURL properties!" );
    }
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
        return ::rtl::OUString("Dialog");
    else
        return ::rtl::OUString("TabPage");
}

void UnoDialogControl::dispose() throw(RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;
        sal_Bool bRet = ControlContainerBase::setModel( rxModel );
    ImplStartListingForResourceEvents();
    return bRet;
}

void UnoDialogControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
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
        ( !aImageURL.isEmpty() ))
    {
        ::rtl::OUString absoluteUrl = aImageURL;
        if ( aImageURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) != 0 )
            absoluteUrl = getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                 uno::makeAny( aImageURL ) );

        xGraphic = ImageHelper::getGraphicFromURL_nothrow( absoluteUrl );
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
    SolarMutexGuard aGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar ) throw (RuntimeException)
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

        // #i87592 In design mode the drawing layer works with sizes with decoration.
        // Therefore we have to substract them before writing back to the properties (model).
        if ( xDialogDevice.is() && mbDesignMode )
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
        aProps[0] = rtl::OUString( "Height" );
        aProps[1] = rtl::OUString( "Width" );
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
        aProps[0] = rtl::OUString( "PositionX" );
        aProps[1] = rtl::OUString( "PositionY" );
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
    SolarMutexGuard aGuard;
    Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(RuntimeException)
{
    SolarMutexGuard aGuard;
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
    SolarMutexGuard aGuard;
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
                ( !aImageURL.isEmpty() ))
            {
                ::rtl::OUString absoluteUrl = aImageURL;
                if ( aImageURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) != 0 )

                    absoluteUrl = getPhysicalLocation( ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL )),
                                         uno::makeAny(aImageURL));

                xGraphic = ImageHelper::getGraphicFromURL_nothrow( absoluteUrl );
            }
            ImplSetPropertyValue(  GetPropertyName( BASEPROPERTY_GRAPHIC), uno::makeAny( xGraphic ), sal_True );
            break;
        }
    }
    ControlContainerBase::ImplModelPropertiesChanged(rEvents);
}

//  ----------------------------------------------------
//  class MultiPageControl
//  ----------------------------------------------------
UnoMultiPageControl::UnoMultiPageControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory) : ControlContainerBase( i_factory ), maTabListeners( *this )
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoMultiPageControl::~UnoMultiPageControl()
{
}
// XTabListener

void SAL_CALL UnoMultiPageControl::inserted( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::removed( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::changed( ::sal_Int32 /*ID*/, const Sequence< NamedValue >& /*Properties*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::activated( ::sal_Int32 ID ) throw (RuntimeException)
{
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), sal_False );

}
void SAL_CALL UnoMultiPageControl::deactivated( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::disposing(const EventObject&) throw (RuntimeException)
{
}

void SAL_CALL UnoMultiPageControl::dispose() throw (RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maTabListeners.disposeAndClear( aEvt );
    ControlContainerBase::dispose();
}

// com::sun::star::awt::XSimpleTabController
::sal_Int32 SAL_CALL UnoMultiPageControl::insertTab() throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->insertTab();
}

void SAL_CALL UnoMultiPageControl::removeTab( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->removeTab( ID );
}

void SAL_CALL UnoMultiPageControl::setTabProps( ::sal_Int32 ID, const Sequence< NamedValue >& Properties ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->setTabProps( ID, Properties );
}

Sequence< NamedValue > SAL_CALL UnoMultiPageControl::getTabProps( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->getTabProps( ID );
}

void SAL_CALL UnoMultiPageControl::activateTab( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->activateTab( ID );
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), sal_True );

}

::sal_Int32 SAL_CALL UnoMultiPageControl::getActiveTabID() throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->getActiveTabID();
}

void SAL_CALL UnoMultiPageControl::addTabListener( const Reference< XTabListener >& Listener ) throw (RuntimeException)
{
    maTabListeners.addInterface( Listener );
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->addTabListener( &maTabListeners );
}

void SAL_CALL UnoMultiPageControl::removeTabListener( const Reference< XTabListener >& Listener ) throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->removeTabListener( &maTabListeners );
    maTabListeners.removeInterface( Listener );
}


// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoMultiPageControl )
    getCppuType( ( uno::Reference< awt::XSimpleTabController>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XTabListener>* ) NULL ),
    ControlContainerBase::getTypes()
IMPL_XTYPEPROVIDER_END

// uno::XInterface
uno::Any UnoMultiPageControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XTabListener* >(this)), (static_cast< awt::XSimpleTabController* >(this)) );
    return (aRet.hasValue() ? aRet : ControlContainerBase::queryAggregation( rType ));
}

::rtl::OUString UnoMultiPageControl::GetComponentServiceName()
{
    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return ::rtl::OUString("tabcontrol");
    // Hopefully we can tweak the tabcontrol to display without tabs
    return ::rtl::OUString("tabcontrolnotabs");
}

void UnoMultiPageControl::bindPage( const uno::Reference< awt::XControl >& _rxControl )
{
    uno::Reference< awt::XWindowPeer > xPage( _rxControl->getPeer() );
    uno::Reference< awt::XSimpleTabController > xTabCntrl( getPeer(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProps( _rxControl->getModel(), uno::UNO_QUERY );

   VCLXTabPage* pXPage = dynamic_cast< VCLXTabPage* >( xPage.get() );
   TabPage* pPage = pXPage ? pXPage->getTabPage() : NULL;
    if ( xTabCntrl.is() && pPage )
    {
        VCLXMultiPage* pXTab = dynamic_cast< VCLXMultiPage* >( xTabCntrl.get() );
        if ( pXTab )
        {
            rtl::OUString sTitle;
            xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ) ) >>= sTitle;
            pXTab->insertTab( pPage, sTitle);
        }
    }

}

void UnoMultiPageControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
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
            ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( nActiveTab ), sal_True );
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
        _rxControl->createPeer( NULL, xMyPeer );
        bindPage( _rxControl );
        ImplActivateTabControllers();
    }

}

// ------------- UnoMultiPageModel -----------------

UnoMultiPageModel::UnoMultiPageModel( const Reference< XMultiServiceFactory >& i_factory ) : ControlModelContainerBase( i_factory )
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
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_DECORATION, aBool );
    // MultiPage Control has the tab stop property. And the default value is True.
    ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );

    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >();
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoMultiPageModel::UnoMultiPageModel( const UnoMultiPageModel& rModel )
    : ControlModelContainerBase( rModel )
{
}

UnoMultiPageModel::~UnoMultiPageModel()
{
}

UnoControlModel*
UnoMultiPageModel::Clone() const
{
    // clone the container itself
    UnoMultiPageModel* pClone = new UnoMultiPageModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

::rtl::OUString UnoMultiPageModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageModel );
}

uno::Any UnoMultiPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageControl );
        return aAny;
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoMultiPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoMultiPageModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoMultiPageModel::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    Reference< XServiceInfo > xInfo;
    aElement >>= xInfo;

    if ( !xInfo.is() )
        throw IllegalArgumentException();

    // Only a Page model can be inserted into the multipage
    if ( !xInfo->supportsService( rtl::OUString::createFromAscii( szServiceName_UnoPageModel ) ) )
        throw IllegalArgumentException();

    return ControlModelContainerBase::insertByName( aName, aElement );
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoMultiPageModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_True;
}

//  ----------------------------------------------------
//  class UnoPageControl
//  ----------------------------------------------------
UnoPageControl::UnoPageControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory ) : ControlContainerBase( i_factory )
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoPageControl::~UnoPageControl()
{
}

::rtl::OUString UnoPageControl::GetComponentServiceName()
{
    return ::rtl::OUString("tabpage");
}


// ------------- UnoPageModel -----------------

UnoPageModel::UnoPageModel( const Reference< XMultiServiceFactory >& i_factory ) : ControlModelContainerBase( i_factory )
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
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    //ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );

    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >();
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoPageModel::UnoPageModel( const UnoPageModel& rModel )
    : ControlModelContainerBase( rModel )
{
}

UnoPageModel::~UnoPageModel()
{
}

UnoControlModel*
UnoPageModel::Clone() const
{
    // clone the container itself
    UnoPageModel* pClone = new UnoPageModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

::rtl::OUString UnoPageModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoPageModel );
}

uno::Any UnoPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoPageControl );
        return aAny;
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoPageModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoPageModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_False;
}

// Frame control

//  ----------------------------------------------------
//  class UnoFrameControl
//  ----------------------------------------------------
UnoFrameControl::UnoFrameControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory ) : ControlContainerBase( i_factory )
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoFrameControl::~UnoFrameControl()
{
}

::rtl::OUString UnoFrameControl::GetComponentServiceName()
{
    return ::rtl::OUString("frame");
}

void UnoFrameControl::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    bool bOwnCtrl = false;
    rtl::OUString sTitle;
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
            if ( !bOwnCtrl && !sTitle.isEmpty() )
            {
                // Adjust Y based on height of Title
                ::Rectangle aRect;
                aRect = pOutDev->GetTextRect( aRect, sTitle );
                nY = nY + ( aRect.GetHeight() / 2 );
            }
        }
        else
        {
            Reference< XWindowPeer > xPeer = ImplGetCompatiblePeer( sal_True );
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
            if ( !bOwnCtrl && !sTitle.isEmpty() )
                // offset y based on height of font ( not sure if my guess at the correct calculation is correct here )
                nY = nY + ( nH / 8); // how do I test this
        }
        xW->setPosSize( nX, nY, nWidth, nHeight, PosSize::POSSIZE );
    }
}

// ------------- UnoFrameModel -----------------

UnoFrameModel::UnoFrameModel(  const Reference< XMultiServiceFactory >& i_factory ) : ControlModelContainerBase( i_factory )
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

    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >();
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoFrameModel::UnoFrameModel( const UnoFrameModel& rModel )
    : ControlModelContainerBase( rModel )
{
}

UnoFrameModel::~UnoFrameModel()
{
}

UnoControlModel*
UnoFrameModel::Clone() const
{
    // clone the container itself
    UnoFrameModel* pClone = new UnoFrameModel( *this );
    Clone_Impl( *pClone );
    return pClone;
}

::rtl::OUString UnoFrameModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoFrameModel );
}

uno::Any UnoFrameModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoFrameControl );
        return aAny;
    }
    return ControlModelContainerBase::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoFrameModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoFrameModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
