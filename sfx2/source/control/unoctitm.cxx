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

#include <config_features.h>

#include <tools/debug.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svl/visitem.hxx>
#include <svtools/javacontext.hxx>
#include <svl/itempool.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <osl/mutex.hxx>
#include <uno/current_context.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include "statcach.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/objsh.hxx>

#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

enum URLTypeId
{
    URLType_BOOL,
    URLType_BYTE,
    URLType_SHORT,
    URLType_LONG,
    URLType_HYPER,
    URLType_STRING,
    URLType_FLOAT,
    URLType_DOUBLE,
    URLType_COUNT
};

const char* URLTypeNames[URLType_COUNT] =
{
    "bool",
    "byte",
    "short",
    "long",
    "hyper",
    "string",
    "float",
    "double"
};

SfxUnoControllerItem::SfxUnoControllerItem( SfxControllerItem *pItem, SfxBindings& rBind, const String& rCmd )
    : pCtrlItem( pItem )
    , pBindings( &rBind )
{
    DBG_ASSERT( !pCtrlItem || !pCtrlItem->IsBound(), "ControllerItem is incorrect!" );

    aCommand.Complete = rCmd;
    Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aCommand );
    pBindings->RegisterUnoController_Impl( this );
}

SfxUnoControllerItem::~SfxUnoControllerItem()
{
    // tell bindings to forget this controller ( if still connected )
    if ( pBindings )
        pBindings->ReleaseUnoController_Impl( this );
}

void SfxUnoControllerItem::UnBind()
{
    // connection to SfxControllerItem is lost
    pCtrlItem = NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  aRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    ReleaseDispatch();
}

void SAL_CALL SfxUnoControllerItem::statusChanged(const ::com::sun::star::frame::FeatureStateEvent& rEvent) throw ( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    DBG_ASSERT( pCtrlItem, "dispatch implementation didn't respect our previous removeStatusListener call!" );

    if ( rEvent.Requery )
    {
        // Error can only happen if the old Dispatch is implemented incorrectly
        // i.e. removeStatusListener did not work. But such things can happen...
        // So protect before ReleaseDispatch from release!
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  aRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY  );
        ReleaseDispatch();
        if ( pCtrlItem )
            GetNewDispatch();           // asynchronous ??
    }
    else if ( pCtrlItem )
    {
        SfxItemState eState = SFX_ITEM_DISABLED;
        SfxPoolItem* pItem = NULL;
        if ( rEvent.IsEnabled )
        {
            eState = SFX_ITEM_AVAILABLE;
            ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

            if ( pType == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = false;
                rEvent.State >>= bTemp ;
                pItem = new SfxBoolItem( pCtrlItem->GetId(), bTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt16*)0) )
            {
                sal_uInt16 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem = new SfxUInt16Item( pCtrlItem->GetId(), nTemp );
            }
            else if ( pType == ::getCppuType((const sal_uInt32*)0) )
            {
                sal_uInt32 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem = new SfxUInt32Item( pCtrlItem->GetId(), nTemp );
            }
            else if ( pType == ::getCppuType((const OUString*)0) )
            {
                OUString sTemp ;
                rEvent.State >>= sTemp ;
                pItem = new SfxStringItem( pCtrlItem->GetId(), sTemp );
            }
            else
                pItem = new SfxVoidItem( pCtrlItem->GetId() );
        }

        pCtrlItem->StateChanged( pCtrlItem->GetId(), eState, pItem );
        delete pItem;
    }
}

void  SAL_CALL SfxUnoControllerItem::disposing( const ::com::sun::star::lang::EventObject& ) throw ( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  aRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    ReleaseDispatch();
}

void SfxUnoControllerItem::ReleaseDispatch()
{
    if ( xDispatch.is() )
    {
        xDispatch->removeStatusListener( (::com::sun::star::frame::XStatusListener*) this, aCommand );
        xDispatch = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();
    }
}

void SfxUnoControllerItem::GetNewDispatch()
{
    if ( !pBindings )
    {
        // Bindings released
        OSL_FAIL( "Tried to get dispatch, but no Bindings!" );
        return;
    }

    // forget old dispatch
    xDispatch = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > ();

    // no arms, no cookies !
    if ( !pBindings->GetDispatcher_Impl() || !pBindings->GetDispatcher_Impl()->GetFrame() )
        return;

    SfxFrame& rFrame = pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame();
    SfxFrame *pParent = rFrame.GetParentFrame();
    if ( pParent )
        // parent may intercept
        xDispatch = TryGetDispatch( pParent );

    if ( !xDispatch.is() )
    {
        // no interception
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  xFrame = rFrame.GetFrameInterface();
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
        if ( xProv.is() )
            xDispatch = xProv->queryDispatch( aCommand, OUString(), 0 );
    }

    if ( xDispatch.is() )
        xDispatch->addStatusListener( (::com::sun::star::frame::XStatusListener*) this, aCommand );
    else if ( pCtrlItem )
        pCtrlItem->StateChanged( pCtrlItem->GetId(), SFX_ITEM_DISABLED, NULL );
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SfxUnoControllerItem::TryGetDispatch( SfxFrame *pFrame )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  xDisp;
    SfxFrame *pParent = pFrame->GetParentFrame();
    if ( pParent )
        // parent may intercept
        xDisp = TryGetDispatch( pParent );

    // only components may intercept
    if ( !xDisp.is() && pFrame->HasComponent() )
    {
        // no interception
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = pFrame->GetFrameInterface();
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >  xProv( xFrame, ::com::sun::star::uno::UNO_QUERY );
        if ( xProv.is() )
            xDisp = xProv->queryDispatch( aCommand, OUString(), 0 );
    }

    return xDisp;
}

void SfxUnoControllerItem::ReleaseBindings()
{
    // connection to binding is lost; so forget the binding and the dispatch
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >  aRef( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    ReleaseDispatch();
    if ( pBindings )
        pBindings->ReleaseUnoController_Impl( this );
    pBindings = NULL;
}

void SfxStatusDispatcher::ReleaseAll()
{
    ::com::sun::star::lang::EventObject aObject;
    aObject.Source = (::cppu::OWeakObject*) this;
    aListeners.disposeAndClear( aObject );
}

void SAL_CALL SfxStatusDispatcher::dispatch( const ::com::sun::star::util::URL&, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& ) throw ( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL SfxStatusDispatcher::dispatchWithNotification(
    const ::com::sun::star::util::URL&,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& ) throw( ::com::sun::star::uno::RuntimeException )
{
}

SfxStatusDispatcher::SfxStatusDispatcher()
    : aListeners( aMutex )
{
}

void SAL_CALL SfxStatusDispatcher::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL) throw ( ::com::sun::star::uno::RuntimeException )
{
    aListeners.addInterface( aURL.Complete, aListener );
    if ( aURL.Complete.compareToAscii(".uno:LifeTime")==0 )
    {
        ::com::sun::star::frame::FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.Source = (::com::sun::star::frame::XDispatch*) this;
        aEvent.IsEnabled = sal_True;
        aEvent.Requery = sal_False;
        aListener->statusChanged( aEvent );
    }
}

void SAL_CALL SfxStatusDispatcher::removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL ) throw ( ::com::sun::star::uno::RuntimeException )
{
    aListeners.removeInterface( aURL.Complete, aListener );
}

//-------------------------------------------------------------------------
// XUnoTunnel
sal_Int64 SAL_CALL SfxOfficeDispatch::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( aIdentifier == impl_getStaticIdentifier() )
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    else
        return 0;
}

SfxOfficeDispatch::SfxOfficeDispatch( SfxBindings& rBindings, SfxDispatcher* pDispat, const SfxSlot* pSlot, const ::com::sun::star::util::URL& rURL )
{
    // this object is an adapter that shows a ::com::sun::star::frame::XDispatch-Interface to the outside and uses a SfxControllerItem to monitor a state
    pControllerItem = new SfxDispatchController_Impl( this, &rBindings, pDispat, pSlot, rURL );
}

SfxOfficeDispatch::SfxOfficeDispatch( SfxDispatcher* pDispat, const SfxSlot* pSlot, const ::com::sun::star::util::URL& rURL )
{
    // this object is an adapter that shows a ::com::sun::star::frame::XDispatch-Interface to the outside and uses a SfxControllerItem to monitor a state
    pControllerItem = new SfxDispatchController_Impl( this, NULL, pDispat, pSlot, rURL );
}

SfxOfficeDispatch::~SfxOfficeDispatch()
{
    if ( pControllerItem )
    {
        // when dispatch object is released, destroy its connection to this object and destroy it
        pControllerItem->UnBindController();
        delete pControllerItem;
    }
}

const ::com::sun::star::uno::Sequence< sal_Int8 >& SfxOfficeDispatch::impl_getStaticIdentifier()
{
    // {38 57 CA 80 09 36 11 d4 83 FE 00 50 04 52 6B 21}
    static const sal_uInt8 pGUID[16] = { 0x38, 0x57, 0xCA, 0x80, 0x09, 0x36, 0x11, 0xd4, 0x83, 0xFE, 0x00, 0x50, 0x04, 0x52, 0x6B, 0x21 };
    static ::com::sun::star::uno::Sequence< sal_Int8 > seqID((const sal_Int8*)pGUID,16) ;
    return seqID ;
}


void SAL_CALL SfxOfficeDispatch::dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw ( ::com::sun::star::uno::RuntimeException )
{
    // ControllerItem is the Impl class
    if ( pControllerItem )
    {
#if HAVE_FEATURE_JAVA
        // The JavaContext contains an interaction handler which is used when
        // the creation of a Java Virtual Machine fails. The second parameter
        // indicates, that there shall only be one user notification (message box)
        // even if the same error (interaction) reoccurs. The effect is, that if a
        // user selects a menu entry than they may get only one notification that
        // a JRE is not selected.
        com::sun::star::uno::ContextLayer layer(
            new svt::JavaContext( com::sun::star::uno::getCurrentContext(),
                                  true) );
#endif
        pControllerItem->dispatch( aURL, aArgs, ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatchResultListener >() );
    }
}

void SAL_CALL SfxOfficeDispatch::dispatchWithNotification( const ::com::sun::star::util::URL& aURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& rListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    // ControllerItem is the Impl class
    if ( pControllerItem )
    {
#if HAVE_FEATURE_JAVA
        // see comment for SfxOfficeDispatch::dispatch
        com::sun::star::uno::ContextLayer layer(
            new svt::JavaContext( com::sun::star::uno::getCurrentContext(),
                                  true) );
#endif
        pControllerItem->dispatch( aURL, aArgs, rListener );
    }
}

void SAL_CALL SfxOfficeDispatch::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL) throw ( ::com::sun::star::uno::RuntimeException )
{
    GetListeners().addInterface( aURL.Complete, aListener );
    if ( pControllerItem )
    {
        // ControllerItem is the Impl class
        pControllerItem->addStatusListener( aListener, aURL );
    }
}

SfxDispatcher* SfxOfficeDispatch::GetDispatcher_Impl()
{
    return pControllerItem->GetDispatcher();
}

void SfxOfficeDispatch::SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame)
{
    if ( pControllerItem )
        pControllerItem->SetFrame( xFrame );
}

void SfxOfficeDispatch::SetMasterUnoCommand( sal_Bool bSet )
{
    if ( pControllerItem )
        pControllerItem->setMasterSlaveCommand( bSet );
}

// Determine if URL contains a master/slave command which must be handled a little bit different
sal_Bool SfxOfficeDispatch::IsMasterUnoCommand( const ::com::sun::star::util::URL& aURL )
{
    if ( aURL.Protocol == ".uno:" && ( aURL.Path.indexOf( '.' ) > 0 ))
        return sal_True;

    return sal_False;
}

OUString SfxOfficeDispatch::GetMasterUnoCommand( const ::com::sun::star::util::URL& aURL )
{
    OUString aMasterCommand;
    if ( IsMasterUnoCommand( aURL ))
    {
        sal_Int32 nIndex = aURL.Path.indexOf( '.' );
        if ( nIndex > 0 )
            aMasterCommand = aURL.Path.copy( 0, nIndex );
    }

    return aMasterCommand;
}

SfxDispatchController_Impl::SfxDispatchController_Impl(
    SfxOfficeDispatch*                 pDisp,
    SfxBindings*                       pBind,
    SfxDispatcher*                     pDispat,
    const SfxSlot*                     pSlot,
    const ::com::sun::star::util::URL& rURL )
    : aDispatchURL( rURL )
    , pDispatcher( pDispat )
    , pBindings( pBind )
    , pLastState( 0 )
    , nSlot( pSlot->GetSlotId() )
    , pDispatch( pDisp )
    , bMasterSlave( sal_False )
    , bVisible( sal_True )
    , pUnoName( pSlot->pUnoName )
{
    if ( aDispatchURL.Protocol == "slot:" && pUnoName )
    {
        OStringBuffer aTmp(RTL_CONSTASCII_STRINGPARAM(".uno:"));
        aTmp.append(pUnoName);
        aDispatchURL.Complete = OStringToOUString(aTmp.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aDispatchURL );
    }

    SetId( nSlot );
    if ( pBindings )
    {
        // Bind immediately to enable the cache to recycle dispatches when asked for the same command
        // a command in "slot" or in ".uno" notation must be treated as identical commands!
        pBindings->ENTERREGISTRATIONS();
        BindInternal_Impl( nSlot, pBindings );
        pBindings->LEAVEREGISTRATIONS();
    }
}

SfxDispatchController_Impl::~SfxDispatchController_Impl()
{
    if ( pLastState && !IsInvalidItem( pLastState ) )
        delete pLastState;

    if ( pDispatch )
    {
        // disconnect
        pDispatch->pControllerItem = NULL;

        // force all listeners to release the dispatch object
        ::com::sun::star::lang::EventObject aObject;
        aObject.Source = (::cppu::OWeakObject*) pDispatch;
        pDispatch->GetListeners().disposeAndClear( aObject );
    }
}

void SfxDispatchController_Impl::SetFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame)
{
    xFrame = _xFrame;
}

void SfxDispatchController_Impl::setMasterSlaveCommand( sal_Bool bSet )
{
    bMasterSlave = bSet;
}

void SfxDispatchController_Impl::UnBindController()
{
    pDispatch = NULL;
    if ( IsBound() )
    {
        GetBindings().ENTERREGISTRATIONS();
        SfxControllerItem::UnBind();
        GetBindings().LEAVEREGISTRATIONS();
    }
}

void SfxDispatchController_Impl::addParametersToArgs( const com::sun::star::util::URL& aURL, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs ) const
{
    // Extract the parameter from the URL and put them into the property value sequence
    sal_Int32 nQueryIndex = aURL.Complete.indexOf( '?' );
    if ( nQueryIndex > 0 )
    {
        OUString aParamString( aURL.Complete.copy( nQueryIndex+1 ));
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aParamString.getToken( 0, '&', nIndex );

            sal_Int32 nParmIndex = 0;
            OUString aParamType;
            OUString aParamName = aToken.getToken( 0, '=', nParmIndex );
            OUString aValue     = (nParmIndex!=-1) ? aToken.getToken( 0, '=', nParmIndex ) : OUString();

            if ( !aParamName.isEmpty() )
            {
                nParmIndex = 0;
                aToken = aParamName;
                aParamName = (nParmIndex!=-1) ? aToken.getToken( 0, ':', nParmIndex ) : OUString();
                aParamType = (nParmIndex!=-1) ? aToken.getToken( 0, ':', nParmIndex ) : OUString();
            }

            sal_Int32 nLen = rArgs.getLength();
            rArgs.realloc( nLen+1 );
            rArgs[nLen].Name = aParamName;

            if ( aParamType.isEmpty() )
            {
                // Default: LONG
                rArgs[nLen].Value <<= aValue.toInt32();
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_BOOL], 4 ))
            {
                // sal_Bool support
                rArgs[nLen].Value <<= aValue.toBoolean();
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_BYTE], 4 ))
            {
                // sal_uInt8 support
                rArgs[nLen].Value <<= sal_Int8( aValue.toInt32() );
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_LONG], 4 ))
            {
                // LONG support
                rArgs[nLen].Value <<= aValue.toInt32();
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_SHORT], 5 ))
            {
                // SHORT support
                rArgs[nLen].Value <<= sal_Int8( aValue.toInt32() );
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_HYPER], 5 ))
            {
                // HYPER support
                rArgs[nLen].Value <<= aValue.toInt64();
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_FLOAT], 5 ))
            {
                // FLOAT support
                rArgs[nLen].Value <<= aValue.toFloat();
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_STRING], 6 ))
            {
                // STRING support
                rArgs[nLen].Value <<= OUString( INetURLObject::decode( aValue, '%', INetURLObject::DECODE_WITH_CHARSET ));
            }
            else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_DOUBLE], 6))
            {
                // DOUBLE support
                rArgs[nLen].Value <<= aValue.toDouble();
            }
        }
        while ( nIndex >= 0 );
    }
}

SfxMapUnit SfxDispatchController_Impl::GetCoreMetric( SfxItemPool& rPool, sal_uInt16 nSlotId )
{
    sal_uInt16 nWhich = rPool.GetWhich( nSlotId );
    return rPool.GetMetric( nWhich );
}

OUString SfxDispatchController_Impl::getSlaveCommand( const ::com::sun::star::util::URL& rURL )
{
    OUString   aSlaveCommand;
    sal_Int32       nIndex = rURL.Path.indexOf( '.' );
    if (( nIndex > 0 ) && ( nIndex < rURL.Path.getLength() ))
        aSlaveCommand = rURL.Path.copy( nIndex+1 );
    return aSlaveCommand;
}

void SAL_CALL SfxDispatchController_Impl::dispatch( const ::com::sun::star::util::URL& aURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& rListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if (
        pDispatch &&
        (
         (aURL.Protocol.equalsAsciiL( ".uno:", 5 ) && aURL.Path == aDispatchURL.Path) ||
         (aURL.Protocol.equalsAsciiL( "slot:", 5 ) && aURL.Path.toInt32() == GetId())
        )
       )
    {
        if ( !pDispatcher && pBindings )
            pDispatcher = GetBindings().GetDispatcher_Impl();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lNewArgs;
        sal_Int32 nCount = aArgs.getLength();

        // Support for URL based arguments
        INetURLObject aURLObj( aURL.Complete );
        if ( aURLObj.HasParam() )
            addParametersToArgs( aURL, lNewArgs );

        // Try to find call mode and frame name inside given arguments...
        SfxCallMode nCall = SFX_CALLMODE_STANDARD;
        sal_Int32   nMarkArg = -1;

        // Filter arguments which shouldn't be part of the sequence property value
        sal_Bool    bTemp = sal_Bool();
        sal_uInt16  nModifier(0);
        std::vector< ::com::sun::star::beans::PropertyValue > aAddArgs;
        for( sal_Int32 n=0; n<nCount; n++ )
        {
            const ::com::sun::star::beans::PropertyValue& rProp = aArgs[n];
            if( rProp.Name.equalsAsciiL("SynchronMode",12))
            {
                if( rProp.Value >>=bTemp )
                    nCall = bTemp ? SFX_CALLMODE_SYNCHRON : SFX_CALLMODE_ASYNCHRON;
            }
            else if( rProp.Name.equalsAsciiL("Bookmark",8))
            {
                nMarkArg = n;
                aAddArgs.push_back( aArgs[n] );
            }
            else if( rProp.Name.equalsAsciiL("KeyModifier",11))
                rProp.Value >>= nModifier;
            else
                aAddArgs.push_back( aArgs[n] );
        }

        // Add needed arguments to sequence property value
        sal_uInt32 nAddArgs = aAddArgs.size();
        if ( nAddArgs > 0 )
        {
            sal_uInt32 nIndex( lNewArgs.getLength() );

            lNewArgs.realloc( lNewArgs.getLength()+aAddArgs.size() );
            for ( sal_uInt32 i = 0; i < nAddArgs; i++ )
                lNewArgs[nIndex++] = aAddArgs[i];
        }

        // Overwrite possible detected sychron argument, if real listener exists (currently no other way)
        if ( rListener.is() )
            nCall = SFX_CALLMODE_SYNCHRON;

        if( GetId() == SID_JUMPTOMARK && nMarkArg == - 1 )
        {
            // we offer dispatches for SID_JUMPTOMARK if the URL points to a bookmark inside the document
            // so we must retrieve this as an argument from the parsed URL
            lNewArgs.realloc( lNewArgs.getLength()+1 );
            nMarkArg = lNewArgs.getLength()-1;
            lNewArgs[nMarkArg].Name = OUString("Bookmark");
            lNewArgs[nMarkArg].Value <<= aURL.Mark;
        }

        css::uno::Reference< css::frame::XFrame > xFrameRef(xFrame.get(), css::uno::UNO_QUERY);
        if (! xFrameRef.is() && pDispatcher)
        {
            SfxViewFrame* pViewFrame = pDispatcher->GetFrame();
            if (pViewFrame)
                xFrameRef = pViewFrame->GetFrame().GetFrameInterface();
        }
        SfxAllItemSet aInternalSet( SFX_APP()->GetPool() );
        if (xFrameRef.is()) // an empty set is no problem ... but an empty frame reference can be a problem !
            aInternalSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrameRef ) );

        sal_Bool bSuccess = sal_False;
        const SfxPoolItem* pItem = NULL;
        SfxShell* pShell( 0 );
        // #i102619# Retrieve metric from shell before execution - the shell could be destroyed after execution
        SfxMapUnit eMapUnit( SFX_MAPUNIT_100TH_MM );
        if ( pDispatcher->GetBindings() )
        {
            if ( !pDispatcher->IsLocked( GetId() ) )
            {
                const SfxSlot *pSlot = 0;
                if ( pDispatcher->GetShellAndSlot_Impl( GetId(), &pShell, &pSlot, sal_False,
                        SFX_CALLMODE_MODAL==(nCall&SFX_CALLMODE_MODAL), sal_False ) )
                {
                    if ( bMasterSlave )
                    {
                        // Extract slave command and add argument to the args list. Master slot MUST
                        // have a argument that has the same name as the master slot and type is SfxStringItem.
                        sal_Int32 nIndex = lNewArgs.getLength();
                        lNewArgs.realloc( nIndex+1 );
                        lNewArgs[nIndex].Name   = OUString::createFromAscii( pSlot->pUnoName );
                        lNewArgs[nIndex].Value  = makeAny( SfxDispatchController_Impl::getSlaveCommand( aDispatchURL ));
                    }

                    eMapUnit = GetCoreMetric( pShell->GetPool(), GetId() );
                    boost::scoped_ptr<SfxAllItemSet> xSet(new SfxAllItemSet(pShell->GetPool()));
                    TransformParameters(GetId(), lNewArgs, *xSet, pSlot);
                    if (xSet->Count())
                    {
                        // execute with arguments - call directly
                        pItem = pDispatcher->Execute(GetId(), nCall, xSet.get(), &aInternalSet, nModifier);
                        bSuccess = (pItem != NULL);
                    }
                    else
                    {
                        // Be sure to delete this before we send a dispatch
                        // request, which will destroy the current shell.
                        xSet.reset();

                        // execute using bindings, enables support for toggle/enum etc.
                        SfxRequest aReq( GetId(), nCall, pShell->GetPool() );
                        aReq.SetModifier( nModifier );
                        aReq.SetInternalArgs_Impl(aInternalSet);
                        pDispatcher->GetBindings()->Execute_Impl( aReq, pSlot, pShell );
                        pItem = aReq.GetReturnValue();
                        bSuccess = aReq.IsDone() || pItem != NULL;
                    }
                }
#ifdef DBG_UTIL
                else
                    DBG_WARNING("MacroPlayer: Unknown slot dispatched!");
#endif
            }
        }
        else
        {
            eMapUnit = GetCoreMetric( SFX_APP()->GetPool(), GetId() );
            // AppDispatcher
            SfxAllItemSet aSet( SFX_APP()->GetPool() );
            TransformParameters( GetId(), lNewArgs, aSet );

            if ( aSet.Count() )
                pItem = pDispatcher->Execute( GetId(), nCall, &aSet, &aInternalSet, nModifier );
            else
                // SfxRequests take empty sets as argument sets, GetArgs() returning non-zero!
                pItem = pDispatcher->Execute( GetId(), nCall, 0, &aInternalSet, nModifier );

            // no bindings, no invalidate ( usually done in SfxDispatcher::Call_Impl()! )
            if ( SfxApplication::Get() )
            {
                SfxDispatcher* pAppDispat = SFX_APP()->GetAppDispatcher_Impl();
                if ( pAppDispat )
                {
                    const SfxPoolItem* pState=0;
                     SfxItemState eState = pDispatcher->QueryState( GetId(), pState );
                    StateChanged( GetId(), eState, pState );
                }
            }

            bSuccess = (pItem != NULL);
        }

        if ( rListener.is() )
        {
            ::com::sun::star::frame::DispatchResultEvent aEvent;
            if ( bSuccess )
                aEvent.State = com::sun::star::frame::DispatchResultState::SUCCESS;
            else
                aEvent.State = com::sun::star::frame::DispatchResultState::FAILURE;

            aEvent.Source = (::com::sun::star::frame::XDispatch*) pDispatch;
            if ( bSuccess && pItem && !pItem->ISA(SfxVoidItem) )
            {
                sal_uInt16 nSubId( 0 );
                if ( eMapUnit == SFX_MAPUNIT_TWIP )
                    nSubId |= CONVERT_TWIPS;
                pItem->QueryValue( aEvent.Result, (sal_uInt8)nSubId );
            }

            rListener->dispatchFinished( aEvent );
        }
    }
}

SfxDispatcher* SfxDispatchController_Impl::GetDispatcher()
{
    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();
    return pDispatcher;
}

void SAL_CALL SfxDispatchController_Impl::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL) throw ( ::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !pDispatch )
        return;

    // Use alternative QueryState call to have a valid UNO representation of the state.
    ::com::sun::star::uno::Any aState;
    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();
    SfxItemState eState = pDispatcher->QueryState( GetId(), aState );

    if ( eState == SFX_ITEM_DONTCARE )
    {
        // Use special uno struct to transport don't care state
        ::com::sun::star::frame::status::ItemStatus aItemStatus;
        aItemStatus.State = ::com::sun::star::frame::status::ItemState::dont_care;
        aState = makeAny( aItemStatus );
    }

    ::com::sun::star::frame::FeatureStateEvent  aEvent;
    aEvent.FeatureURL = aURL;
    aEvent.Source     = (::com::sun::star::frame::XDispatch*) pDispatch;
    aEvent.Requery    = sal_False;
    if ( bVisible )
    {
        aEvent.IsEnabled  = eState != SFX_ITEM_DISABLED;
        aEvent.State      = aState;
    }
    else
    {
        ::com::sun::star::frame::status::Visibility aVisibilityStatus;
        aVisibilityStatus.bVisible = sal_False;

        // MBA: we might decide to *not* disable "invisible" slots, but this would be
        // a change that needs to adjust at least the testtool
        aEvent.IsEnabled           = sal_False;
        aEvent.State               = makeAny( aVisibilityStatus );
    }

    aListener->statusChanged( aEvent );
}

void SfxDispatchController_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState, SfxSlotServer* pSlotServ )
{
    if ( !pDispatch )
        return;

    // Bindings instance notifies controller about a state change, listeners must be notified also
    // Don't cache visibility state changes as they are volatile. We need our real state to send it
    // to our controllers after visibility is set to true.
    sal_Bool bNotify = sal_True;
    if ( pState && !IsInvalidItem( pState ) )
    {
        if ( !pState->ISA( SfxVisibilityItem ) )
        {
            sal_Bool bBothAvailable = pLastState && !IsInvalidItem(pLastState);
            if ( bBothAvailable )
                bNotify = pState->Type() != pLastState->Type() || *pState != *pLastState;
            if ( pLastState && !IsInvalidItem( pLastState ) )
                delete pLastState;
            pLastState = !IsInvalidItem(pState) ? pState->Clone() : pState;
            bVisible = sal_True;
        }
        else
            bVisible = ((SfxVisibilityItem *)pState)->GetValue();
    }
    else
    {
        if ( pLastState && !IsInvalidItem( pLastState ) )
            delete pLastState;
        pLastState = pState;
    }

    ::cppu::OInterfaceContainerHelper* pContnr = pDispatch->GetListeners().getContainer ( aDispatchURL.Complete );
    if ( bNotify && pContnr )
    {
        ::com::sun::star::uno::Any aState;
        if ( ( eState >= SFX_ITEM_AVAILABLE ) && pState && !IsInvalidItem( pState ) && !pState->ISA(SfxVoidItem) )
        {
            // Retrieve metric from pool to have correct sub ID when calling QueryValue
            sal_uInt16     nSubId( 0 );
            SfxMapUnit eMapUnit( SFX_MAPUNIT_100TH_MM );

            // retrieve the core metric
            // it's enough to check the objectshell, the only shell that does not use the pool of the document
            // is SfxViewFrame, but it hasn't any metric parameters
            // TODO/LATER: what about the FormShell? Does it use any metric data?! Perhaps it should use the Pool of the document!
            if ( pSlotServ && pDispatcher )
            {
                SfxShell* pShell = pDispatcher->GetShell( pSlotServ->GetShellLevel() );
                DBG_ASSERT( pShell, "Can't get core metric without shell!" );
                if ( pShell )
                    eMapUnit = GetCoreMetric( pShell->GetPool(), nSID );
            }

            if ( eMapUnit == SFX_MAPUNIT_TWIP )
                nSubId |= CONVERT_TWIPS;

            pState->QueryValue( aState, (sal_uInt8)nSubId );
        }
        else if ( eState == SFX_ITEM_DONTCARE )
        {
            // Use special uno struct to transport don't care state
            ::com::sun::star::frame::status::ItemStatus aItemStatus;
            aItemStatus.State = ::com::sun::star::frame::status::ItemState::dont_care;
            aState = makeAny( aItemStatus );
        }

        ::com::sun::star::frame::FeatureStateEvent aEvent;
        aEvent.FeatureURL = aDispatchURL;
        aEvent.Source = (::com::sun::star::frame::XDispatch*) pDispatch;
        aEvent.IsEnabled = eState != SFX_ITEM_DISABLED;
        aEvent.Requery = sal_False;
        aEvent.State = aState;

        ::cppu::OInterfaceIteratorHelper aIt( *pContnr );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((::com::sun::star::frame::XStatusListener *)aIt.next())->statusChanged( aEvent );
            }
            catch (const ::com::sun::star::uno::RuntimeException&)
            {
                aIt.remove();
            }
        }
    }
}

void SfxDispatchController_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    StateChanged( nSID, eState, pState, 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
