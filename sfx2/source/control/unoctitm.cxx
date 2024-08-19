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

#include <config_java.h>

#include <tools/debug.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <svtools/javacontext.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Template.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <comphelper/processfactory.hxx>
#include <uno/current_context.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/uitest/logger.hxx>
#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>

#include <sfx2/app.hxx>
#include <unoctitm.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/request.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/viewsh.hxx>
#include <slotserv.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/lokhelper.hxx>

#include <memory>
#include <string_view>

#include <sal/log.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>

#include <desktop/crashreport.hxx>
#include <vcl/threadex.hxx>
#include <unotools/mediadescriptor.hxx>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

namespace {

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

}

const char* const URLTypeNames[URLType_COUNT] =
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

static void InterceptLOKStateChangeEvent( sal_uInt16 nSID, SfxViewFrame* pViewFrame, const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem* pState );

void SfxStatusDispatcher::ReleaseAll()
{
    css::lang::EventObject aObject;
    aObject.Source = getXWeak();
    std::unique_lock aGuard(maMutex);
    maListeners.disposeAndClear( aGuard, aObject );
}

void SfxStatusDispatcher::sendStatusChanged(const OUString& rURL, const css::frame::FeatureStateEvent& rEvent)
{
    std::unique_lock aGuard(maMutex);
    ::comphelper::OInterfaceContainerHelper4<css::frame::XStatusListener>* pContnr = maListeners.getContainer(aGuard, rURL);
    if (!pContnr)
        return;
    pContnr->forEach(aGuard,
        [&rEvent](const css::uno::Reference<css::frame::XStatusListener>& xListener)
        {
            xListener->statusChanged(rEvent);
        }
    );
}

void SAL_CALL SfxStatusDispatcher::dispatch( const css::util::URL&, const css::uno::Sequence< css::beans::PropertyValue >& )
{
}

void SAL_CALL SfxStatusDispatcher::dispatchWithNotification(
    const css::util::URL&,
    const css::uno::Sequence< css::beans::PropertyValue >&,
    const css::uno::Reference< css::frame::XDispatchResultListener >& )
{
}

SfxStatusDispatcher::SfxStatusDispatcher()
{
}

void SAL_CALL SfxStatusDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL)
{
    {
        std::unique_lock aGuard(maMutex);
        maListeners.addInterface( aGuard, aURL.Complete, aListener );
    }
    if ( aURL.Complete == ".uno:LifeTime" )
    {
        css::frame::FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.Source = static_cast<css::frame::XDispatch*>(this);
        aEvent.IsEnabled = true;
        aEvent.Requery = false;
        aListener->statusChanged( aEvent );
    }
}

void SAL_CALL SfxStatusDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL )
{
    std::unique_lock aGuard(maMutex);
    maListeners.removeInterface( aGuard, aURL.Complete, aListener );
}


SfxOfficeDispatch::SfxOfficeDispatch( SfxBindings& rBindings, SfxDispatcher* pDispat, const SfxSlot* pSlot, const css::util::URL& rURL )
    : pImpl( new SfxDispatchController_Impl( this, &rBindings, pDispat, pSlot, rURL ))
{
    // pImpl is an adapter that shows a css::frame::XDispatch-Interface to the outside and uses a SfxControllerItem to monitor a state

}

SfxOfficeDispatch::SfxOfficeDispatch( SfxDispatcher* pDispat, const SfxSlot* pSlot, const css::util::URL& rURL )
    : pImpl( new SfxDispatchController_Impl( this, nullptr, pDispat, pSlot, rURL ))
{
    // pImpl is an adapter that shows a css::frame::XDispatch-Interface to the outside and uses a SfxControllerItem to monitor a state
}

SfxOfficeDispatch::~SfxOfficeDispatch()
{
    if ( pImpl )
    {
        // when dispatch object is released, destroy its connection to this object and destroy it
        pImpl->UnBindController();

        // Ensure that SfxDispatchController_Impl is deleted while the solar mutex is locked, since
        // that derives from SfxListener.
        SolarMutexGuard aGuard;
        pImpl.reset();
    }
}

#if HAVE_FEATURE_JAVA
// The JavaContext contains an interaction handler which is used when
// the creation of a Java Virtual Machine fails. There shall only be one
// user notification (message box) even if the same error (interaction)
// reoccurs. The effect is, that if a user selects a menu entry than they
// may get only one notification that a JRE is not selected.
// This function checks if a JavaContext is already available (typically
// created by Desktop::Main() in app.cxx), and creates new one if not.
namespace {
std::unique_ptr< css::uno::ContextLayer > EnsureJavaContext()
{
    css::uno::Reference< css::uno::XCurrentContext > xContext(css::uno::getCurrentContext());
    if (xContext.is())
    {
        css::uno::Reference< css::task::XInteractionHandler > xHandler;
        xContext->getValueByName(JAVA_INTERACTION_HANDLER_NAME) >>= xHandler;
        if (xHandler.is())
            return nullptr; // No need to add new layer: JavaContext already present
    }
    return std::make_unique< css::uno::ContextLayer >(new svt::JavaContext(xContext));
}
}
#endif

void SAL_CALL SfxOfficeDispatch::dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs )
{
    // ControllerItem is the Impl class
    if ( pImpl )
    {
#if HAVE_FEATURE_JAVA
        std::unique_ptr< css::uno::ContextLayer > layer(EnsureJavaContext());
#endif
        utl::MediaDescriptor aDescriptor(aArgs);
        bool bOnMainThread = aDescriptor.getUnpackedValueOrDefault(u"OnMainThread"_ustr, false);
        if (bOnMainThread)
        {
            // Make sure that we own the solar mutex, otherwise later
            // vcl::SolarThreadExecutor::execute() will release the solar mutex, even if it's owned by
            // another thread, leading to an std::abort() at the end.
            SolarMutexGuard aGuard;
            vcl::solarthread::syncExecute([this, &aURL, &aArgs]() {
                pImpl->dispatch(aURL, aArgs,
                                css::uno::Reference<css::frame::XDispatchResultListener>());
            });
        }
        else
        {
            pImpl->dispatch(aURL, aArgs,
                            css::uno::Reference<css::frame::XDispatchResultListener>());
        }
    }
}

void SAL_CALL SfxOfficeDispatch::dispatchWithNotification( const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
        const css::uno::Reference< css::frame::XDispatchResultListener >& rListener )
{
    // ControllerItem is the Impl class
    if ( pImpl )
    {
#if HAVE_FEATURE_JAVA
        std::unique_ptr< css::uno::ContextLayer > layer(EnsureJavaContext());
#endif
        pImpl->dispatch( aURL, aArgs, rListener );
    }
}

void SAL_CALL SfxOfficeDispatch::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL)
{
    {
        std::unique_lock aGuard(maMutex);
        maListeners.addInterface( aGuard, aURL.Complete, aListener );
    }
    if ( pImpl )
    {
        // ControllerItem is the Impl class
        pImpl->addStatusListener( aListener, aURL );
    }
}

SfxDispatcher* SfxOfficeDispatch::GetDispatcher_Impl()
{
    return pImpl->GetDispatcher();
}

sal_uInt16 SfxOfficeDispatch::GetId() const
{
    return pImpl ? pImpl->GetId() : 0;
}

void SfxOfficeDispatch::SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    if ( pImpl )
        pImpl->SetFrame( xFrame );
}

void SfxOfficeDispatch::SetMasterUnoCommand( bool bSet )
{
    if ( pImpl )
        pImpl->setMasterSlaveCommand( bSet );
}

// Determine if URL contains a master/slave command which must be handled a little bit different
bool SfxOfficeDispatch::IsMasterUnoCommand( const css::util::URL& aURL )
{
    return aURL.Protocol == ".uno:" && ( aURL.Path.indexOf( '.' ) > 0 );
}

OUString SfxOfficeDispatch::GetMasterUnoCommand( const css::util::URL& aURL )
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
    css::util::URL               aURL )
    : aDispatchURL(std::move( aURL ))
    , pDispatcher( pDispat )
    , pBindings( pBind )
    , pLastState( nullptr )
    , pDispatch( pDisp )
    , bMasterSlave( false )
    , bVisible( true )
{
    if ( aDispatchURL.Protocol == "slot:" && !pSlot->aUnoName.isEmpty() )
    {
        aDispatchURL.Complete = pSlot->GetCommand();
        Reference< XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aDispatchURL );
    }

    sal_uInt16 nSlot = pSlot->GetSlotId();
    SetId( nSlot );
    if ( pBindings )
    {
        // Bind immediately to enable the cache to recycle dispatches when asked for the same command
        // a command in "slot" or in ".uno" notation must be treated as identical commands!
        pBindings->ENTERREGISTRATIONS();
        BindInternal_Impl( nSlot, pBindings );
        pBindings->LEAVEREGISTRATIONS();
    }
    assert(pDispatcher);
    assert(SfxApplication::Get()->GetAppDispatcher_Impl() == pDispatcher
        || pDispatcher->GetFrame() != nullptr);
    if (pDispatcher->GetFrame())
    {
        StartListening(*pDispatcher->GetFrame());
    }
    else
    {
        StartListening(*SfxApplication::Get());
    }
}

void SfxDispatchController_Impl::Notify(SfxBroadcaster& rBC, SfxHint const& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {   // both pBindings and pDispatcher are dead if SfxViewFrame is dead
        pBindings = nullptr;
        pDispatcher = nullptr;
        EndListening(rBC);
    }
}

SfxDispatchController_Impl::~SfxDispatchController_Impl()
{
    if ( pLastState && !IsInvalidItem( pLastState ) )
        delete pLastState;

    if ( pDispatch )
    {
        // disconnect
        pDispatch->pImpl = nullptr;

        // force all listeners to release the dispatch object
        pDispatch->ReleaseAll();
    }
}

void SfxDispatchController_Impl::SetFrame(const css::uno::Reference< css::frame::XFrame >& _xFrame)
{
    xFrame = _xFrame;
}

void SfxDispatchController_Impl::setMasterSlaveCommand( bool bSet )
{
    bMasterSlave = bSet;
}

void SfxDispatchController_Impl::UnBindController()
{
    pDispatch = nullptr;
    if ( IsBound() )
    {
        GetBindings().ENTERREGISTRATIONS();
        SfxControllerItem::UnBind();
        GetBindings().LEAVEREGISTRATIONS();
    }
}

void SfxDispatchController_Impl::addParametersToArgs( const css::util::URL& aURL, css::uno::Sequence< css::beans::PropertyValue >& rArgs )
{
    // Extract the parameter from the URL and put them into the property value sequence
    sal_Int32 nQueryIndex = aURL.Complete.indexOf( '?' );
    if ( nQueryIndex <= 0 )
        return;

    OUString aParamString( aURL.Complete.copy( nQueryIndex+1 ));
    sal_Int32 nIndex = 0;
    do
    {
        OUString aToken = aParamString.getToken( 0, '&', nIndex );

        sal_Int32 nParmIndex = 0;
        OUString aParamType;
        OUString aParamName = aToken.getToken( 0, '=', nParmIndex );
        OUString aValue     = aToken.getToken( 0, '=', nParmIndex );

        if ( !aParamName.isEmpty() )
        {
            nParmIndex = 0;
            aToken = aParamName;
            aParamName = aToken.getToken( 0, ':', nParmIndex );
            aParamType = aToken.getToken( 0, ':', nParmIndex );
        }

        sal_Int32 nLen = rArgs.getLength();
        rArgs.realloc( nLen+1 );
        auto pArgs = rArgs.getArray();
        pArgs[nLen].Name = aParamName;

        if ( aParamType.isEmpty() )
        {
            // Default: LONG
            pArgs[nLen].Value <<= aValue.toInt32();
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_BOOL], 4 ))
        {
            // sal_Bool support
            pArgs[nLen].Value <<= aValue.toBoolean();
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_BYTE], 4 ))
        {
            // sal_uInt8 support
            pArgs[nLen].Value <<= sal_Int8( aValue.toInt32() );
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_LONG], 4 ))
        {
            // LONG support
            pArgs[nLen].Value <<= aValue.toInt32();
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_SHORT], 5 ))
        {
            // SHORT support
            pArgs[nLen].Value <<= sal_Int16( aValue.toInt32() );
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_HYPER], 5 ))
        {
            // HYPER support
            pArgs[nLen].Value <<= aValue.toInt64();
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_FLOAT], 5 ))
        {
            // FLOAT support
            pArgs[nLen].Value <<= aValue.toFloat();
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_STRING], 6 ))
        {
            // STRING support
            pArgs[nLen].Value <<= INetURLObject::decode( aValue, INetURLObject::DecodeMechanism::WithCharset );
        }
        else if ( aParamType.equalsAsciiL( URLTypeNames[URLType_DOUBLE], 6))
        {
            // DOUBLE support
            pArgs[nLen].Value <<= aValue.toDouble();
        }
    }
    while ( nIndex >= 0 );
}

MapUnit SfxDispatchController_Impl::GetCoreMetric( SfxItemPool const & rPool, sal_uInt16 nSlotId )
{
    sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( nSlotId );
    return rPool.GetMetric( nWhich );
}

OUString SfxDispatchController_Impl::getSlaveCommand( const css::util::URL& rURL )
{
    OUString   aSlaveCommand;
    sal_Int32       nIndex = rURL.Path.indexOf( '.' );
    if (( nIndex > 0 ) && ( nIndex < rURL.Path.getLength() ))
        aSlaveCommand = rURL.Path.copy( nIndex+1 );
    return aSlaveCommand;
}

namespace {

void collectUIInformation(const util::URL& rURL, const css::uno::Sequence< css::beans::PropertyValue >& rArgs)
{
    static const char* pFile = std::getenv("LO_COLLECT_UIINFO");
    if (!pFile)
        return;

    UITestLogger::getInstance().logCommand(
        Concat2View("Send UNO Command (\"" + rURL.Complete + "\") "), rArgs);
}

}

void SfxDispatchController_Impl::dispatch( const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
        const css::uno::Reference< css::frame::XDispatchResultListener >& rListener )
{
    if ( aURL.Protocol == ".uno:")
    {
        CrashReporter::logUnoCommand(aURL.Path);
    }
    collectUIInformation(aURL, aArgs);

    SolarMutexGuard aGuard;

    if (comphelper::LibreOfficeKit::isActive())
    {
        const SfxViewShell* pViewShell = SfxViewShell::Current();
        if (pViewShell && pViewShell->isBlockedCommand(aURL.Complete))
        {
            tools::JsonWriter aTree;
            aTree.put("code", "");
            aTree.put("kind", "BlockedCommand");
            aTree.put("cmd", aURL.Complete);
            aTree.put("message", "Blocked feature");
            aTree.put("viewID", pViewShell->GetViewShellId().get());

            pViewShell->libreOfficeKitViewCallback(LOK_COMMAND_BLOCKED, aTree.finishAndGetAsOString());
            return;
        }
    }

    if (
        !(pDispatch &&
        (
         (aURL.Protocol == ".uno:" && aURL.Path == aDispatchURL.Path) ||
         (aURL.Protocol == "slot:" && aURL.Path.toInt32() == GetId())
        ))
       )
        return;

    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();

    css::uno::Sequence< css::beans::PropertyValue > lNewArgs;
    sal_Int32 nCount = aArgs.getLength();

    // Support for URL based arguments
    INetURLObject aURLObj( aURL.Complete );
    if ( aURLObj.HasParam() )
        addParametersToArgs( aURL, lNewArgs );

    // Try to find call mode and frame name inside given arguments...
    SfxCallMode nCall = SfxCallMode::RECORD;
    sal_Int32   nMarkArg = -1;

    // Filter arguments which shouldn't be part of the sequence property value
    sal_uInt16  nModifier(0);
    std::vector< css::beans::PropertyValue > aAddArgs;
    for( sal_Int32 n=0; n<nCount; n++ )
    {
        const css::beans::PropertyValue& rProp = aArgs[n];
        if( rProp.Name == "SynchronMode" )
        {
            bool    bTemp;
            if( rProp.Value >>= bTemp )
                nCall = bTemp ? SfxCallMode::SYNCHRON : SfxCallMode::ASYNCHRON;
        }
        else if( rProp.Name == "Bookmark" )
        {
            nMarkArg = n;
            aAddArgs.push_back( aArgs[n] );
        }
        else if( rProp.Name == "KeyModifier" )
            rProp.Value >>= nModifier;
        else
            aAddArgs.push_back( aArgs[n] );
    }

    // Add needed arguments to sequence property value
    sal_uInt32 nAddArgs = aAddArgs.size();
    if ( nAddArgs > 0 )
    {
        sal_uInt32 nIndex( lNewArgs.getLength() );

        lNewArgs.realloc( nIndex + nAddArgs );
        std::copy(aAddArgs.begin(), aAddArgs.end(), std::next(lNewArgs.getArray(), nIndex));
    }

    // Overwrite possible detected synchron argument, if real listener exists (currently no other way)
    if ( rListener.is() )
        nCall = SfxCallMode::SYNCHRON;

    if( GetId() == SID_JUMPTOMARK && nMarkArg == - 1 )
    {
        // we offer dispatches for SID_JUMPTOMARK if the URL points to a bookmark inside the document
        // so we must retrieve this as an argument from the parsed URL
        lNewArgs.realloc( lNewArgs.getLength()+1 );
        auto& el = lNewArgs.getArray()[lNewArgs.getLength()-1];
        el.Name = "Bookmark";
        el.Value <<= aURL.Mark;
    }

    css::uno::Reference< css::frame::XFrame > xFrameRef(xFrame.get(), css::uno::UNO_QUERY);
    if (! xFrameRef.is() && pDispatcher)
    {
        SfxViewFrame* pViewFrame = pDispatcher->GetFrame();
        if (pViewFrame)
            xFrameRef = pViewFrame->GetFrame().GetFrameInterface();
    }

    bool bSuccess = false;
    SfxPoolItemHolder aItem;
    MapUnit eMapUnit( MapUnit::Map100thMM );

    // Extra scope so that aInternalSet is destroyed before
    // rListener->dispatchFinished potentially calls
    // framework::Desktop::terminate -> SfxApplication::Deinitialize ->
    // ~CntItemPool:
    if (pDispatcher)
    {
        SfxAllItemSet aInternalSet( SfxGetpApp()->GetPool() );
        if (xFrameRef.is()) // an empty set is no problem ... but an empty frame reference can be a problem !
            aInternalSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrameRef ) );

        SfxShell* pShell( nullptr );
        // #i102619# Retrieve metric from shell before execution - the shell could be destroyed after execution
        if ( pDispatcher->GetBindings() )
        {
            if ( !pDispatcher->IsLocked() )
            {
                const SfxSlot *pSlot = nullptr;
                if ( pDispatcher->GetShellAndSlot_Impl( GetId(), &pShell, &pSlot, false, false ) )
                {
                    if ( bMasterSlave )
                    {
                        // Extract slave command and add argument to the args list. Master slot MUST
                        // have an argument that has the same name as the master slot and type is SfxStringItem.
                        sal_Int32 nIndex = lNewArgs.getLength();
                        lNewArgs.realloc( nIndex+1 );
                        auto plNewArgs = lNewArgs.getArray();
                        plNewArgs[nIndex].Name   = pSlot->aUnoName;
                        plNewArgs[nIndex].Value  <<= SfxDispatchController_Impl::getSlaveCommand( aDispatchURL );
                    }

                    eMapUnit = GetCoreMetric( pShell->GetPool(), GetId() );
                    std::optional<SfxAllItemSet> xSet(pShell->GetPool());
                    TransformParameters(GetId(), lNewArgs, *xSet, pSlot);
                    if (xSet->Count())
                    {
                        // execute with arguments - call directly
                        aItem = pDispatcher->Execute(GetId(), nCall, &*xSet, &aInternalSet, nModifier);
                        if (aItem)
                        {
                            if (const SfxBoolItem* pBoolItem = dynamic_cast<const SfxBoolItem*>(aItem.getItem()))
                                bSuccess = pBoolItem->GetValue();
                            else if ( !IsDisabledItem(aItem.getItem()) )
                                bSuccess = true;  // all other types are true
                        }
                        // else bSuccess = false look to line 664 it is false
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
                        aItem = aReq.GetReturnValue();
                        bSuccess = aReq.IsDone() || aItem;
                    }
                }
                else
                    SAL_INFO("sfx.control", "MacroPlayer: Unknown slot dispatched!");
            }
        }
        else
        {
            eMapUnit = GetCoreMetric( SfxGetpApp()->GetPool(), GetId() );
            // AppDispatcher
            SfxAllItemSet aSet( SfxGetpApp()->GetPool() );
            TransformParameters( GetId(), lNewArgs, aSet );

            if ( aSet.Count() )
                aItem = pDispatcher->Execute(GetId(), nCall, &aSet, &aInternalSet, nModifier);
            else
                // SfxRequests take empty sets as argument sets, GetArgs() returning non-zero!
                aItem = pDispatcher->Execute(GetId(), nCall, nullptr, &aInternalSet, nModifier);

            // no bindings, no invalidate ( usually done in SfxDispatcher::Call_Impl()! )
            if (SfxApplication* pApp = SfxApplication::Get())
            {
                SfxDispatcher* pAppDispat = pApp->GetAppDispatcher_Impl();
                if ( pAppDispat )
                {
                    SfxPoolItemHolder aResult;
                    SfxItemState eState(pDispatcher->QueryState(GetId(), aResult));
                    StateChangedAtToolBoxControl(GetId(), eState, aResult.getItem());
                }
            }

            bSuccess = aItem.is();
        }
    }

    if ( !rListener.is() )
        return;

    css::frame::DispatchResultEvent aEvent;
    if ( bSuccess )
        aEvent.State = css::frame::DispatchResultState::SUCCESS;
    else
        aEvent.State = css::frame::DispatchResultState::FAILURE;

    aEvent.Source = static_cast<css::frame::XDispatch*>(pDispatch);
    if ( bSuccess && aItem && !IsDisabledItem(aItem.getItem()) )
    {
        sal_uInt16 nSubId( 0 );
        if ( eMapUnit == MapUnit::MapTwip )
            nSubId |= CONVERT_TWIPS;
        aItem.getItem()->QueryValue( aEvent.Result, static_cast<sal_uInt8>(nSubId) );
    }

    rListener->dispatchFinished( aEvent );
}

SfxDispatcher* SfxDispatchController_Impl::GetDispatcher()
{
    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();
    return pDispatcher;
}

void SfxDispatchController_Impl::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL)
{
    SolarMutexGuard aGuard;
    if ( !pDispatch )
        return;

    // Use alternative QueryState call to have a valid UNO representation of the state.
    css::uno::Any aState;
    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();
    SfxItemState eState = pDispatcher ? pDispatcher->QueryState( GetId(), aState ) : SfxItemState::INVALID;

    if ( eState == SfxItemState::INVALID )
    {
        // Use special uno struct to transport don't care state
        css::frame::status::ItemStatus aItemStatus;
        aItemStatus.State = css::frame::status::ItemState::DONT_CARE;
        aState <<= aItemStatus;
    }

    css::frame::FeatureStateEvent  aEvent;
    aEvent.FeatureURL = aURL;
    aEvent.Source     = static_cast<css::frame::XDispatch*>(pDispatch);
    aEvent.Requery    = false;
    if ( bVisible )
    {
        aEvent.IsEnabled  = eState != SfxItemState::DISABLED;
        aEvent.State      = aState;
    }
    else
    {
        css::frame::status::Visibility aVisibilityStatus;
        aVisibilityStatus.bVisible = false;

        // MBA: we might decide to *not* disable "invisible" slots, but this would be
        // a change that needs to adjust at least the testtool
        aEvent.IsEnabled           = false;
        aEvent.State               <<= aVisibilityStatus;
    }

    aListener->statusChanged( aEvent );
}

void SfxDispatchController_Impl::sendStatusChanged(const OUString& rURL, const css::frame::FeatureStateEvent& rEvent)
{
    pDispatch->sendStatusChanged(rURL, rEvent);
}

void SfxDispatchController_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState, SfxSlotServer const * pSlotServ )
{
    if ( !pDispatch )
        return;

    // Bindings instance notifies controller about a state change, listeners must be notified also
    // Don't cache visibility state changes as they are volatile. We need our real state to send it
    // to our controllers after visibility is set to true.
    bool bNotify = true;
    if ( pState && !IsInvalidItem( pState ) )
    {
        if ( auto pVisibilityItem = dynamic_cast< const SfxVisibilityItem *>( pState ) )
            bVisible = pVisibilityItem->GetValue();
        else
        {
            if (pLastState && !IsInvalidItem(pLastState))
            {
                bNotify = typeid(*pState) != typeid(*pLastState) || *pState != *pLastState;
                delete pLastState;
            }
            pLastState = !IsInvalidItem(pState) ? pState->Clone() : pState;
            bVisible = true;
        }
    }
    else
    {
        if ( pLastState && !IsInvalidItem( pLastState ) )
            delete pLastState;
        pLastState = pState;
    }

    if (!bNotify)
        return;

    css::uno::Any aState;
    if ( ( eState >= SfxItemState::DEFAULT ) && pState && !IsInvalidItem( pState ) && !IsDisabledItem(pState) )
    {
        // Retrieve metric from pool to have correct sub ID when calling QueryValue
        sal_uInt16     nSubId( 0 );
        MapUnit eMapUnit( MapUnit::Map100thMM );

        // retrieve the core metric
        // it's enough to check the objectshell, the only shell that does not use the pool of the document
        // is SfxViewFrame, but it hasn't any metric parameters
        // TODO/LATER: what about the FormShell? Does it use any metric data?! Perhaps it should use the Pool of the document!
        if ( pSlotServ && pDispatcher )
        {
            if (SfxShell* pShell = pDispatcher->GetShell( pSlotServ->GetShellLevel() ))
                eMapUnit = GetCoreMetric( pShell->GetPool(), nSID );
        }

        if ( eMapUnit == MapUnit::MapTwip )
            nSubId |= CONVERT_TWIPS;

        pState->QueryValue( aState, static_cast<sal_uInt8>(nSubId) );
    }
    else if ( eState == SfxItemState::INVALID )
    {
        // Use special uno struct to transport don't care state
        css::frame::status::ItemStatus aItemStatus;
        aItemStatus.State = css::frame::status::ItemState::DONT_CARE;
        aState <<= aItemStatus;
    }

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = aDispatchURL;
    aEvent.Source = static_cast<css::frame::XDispatch*>(pDispatch);
    aEvent.IsEnabled = eState != SfxItemState::DISABLED;
    aEvent.Requery = false;
    aEvent.State = aState;

    if (pDispatcher && pDispatcher->GetFrame())
    {
        InterceptLOKStateChangeEvent(nSID, pDispatcher->GetFrame(), aEvent, pState);
    }

    const std::vector<OUString> aContainedTypes = pDispatch->getContainedTypes();
    for (const OUString& rName: aContainedTypes)
    {
        if (rName == aDispatchURL.Main || rName == aDispatchURL.Complete)
            sendStatusChanged(rName, aEvent);
    }
}

void SfxDispatchController_Impl::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    StateChanged( nSID, eState, pState, nullptr );
}

namespace
{
using PayloadGetter_t = OString (*)(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent&,
                                   const SfxPoolItem*);

OString IsActivePayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                        const SfxPoolItem*)
{
    bool bTemp = false;
    aEvent.State >>= bTemp;
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + OString::boolean(bTemp);
}

OString FontNamePayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                        const SfxPoolItem*)
{
    css::awt::FontDescriptor aFontDesc;
    aEvent.State >>= aFontDesc;
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + aFontDesc.Name.toUtf8();
}

OString FontHeightPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                          const SfxPoolItem*)
{
    css::frame::status::FontHeight aFontHeight;
    aEvent.State >>= aFontHeight;
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + OString::number(aFontHeight.Height);
}

OString StyleApplyPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                          const SfxPoolItem*)
{
    css::frame::status::Template aTemplate;
    aEvent.State >>= aTemplate;
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + aTemplate.StyleName.toUtf8();
}

OString ColorPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                     const SfxPoolItem*)
{
    sal_Int32 nColor = -1;
    aEvent.State >>= nColor;
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + OString::number(nColor);
}

OString UndoRedoPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                        const SfxPoolItem* pState)
{
    if (aEvent.IsEnabled)
        if (auto pUndoConflict = dynamic_cast<const SfxUInt32Item*>(pState);
            !pUndoConflict || pUndoConflict->GetValue() == 0)
            return aEvent.FeatureURL.Complete.toUtf8() + "=enabled";
    return aEvent.FeatureURL.Complete.toUtf8() + "=disabled";
}

OString EnabledPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                       const SfxPoolItem*)
{
    return aEvent.FeatureURL.Complete.toUtf8()
           + (aEvent.IsEnabled ? std::string_view("=enabled") : std::string_view("=disabled"));
}

OString ParaDirectionPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                             const SfxPoolItem*)
{
    tools::JsonWriter aTree;
    bool bTemp = false;
    aEvent.State >>= bTemp;
    aTree.put("commandName", aEvent.FeatureURL.Complete);
    aTree.put("disabled", !aEvent.IsEnabled);
    aTree.put("state", bTemp ? std::string_view("true") : std::string_view("false"));
    return aTree.finishAndGetAsOString();
}

OString Int32Payload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                     const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (sal_Int32 aInt32; aEvent.IsEnabled && (aEvent.State >>= aInt32))
        aBuffer.append(aInt32);
    return aBuffer.makeStringAndClear();
}

OString TransformPayload(sal_uInt16 nSID, SfxViewFrame* pViewFrame,
                         const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem*)
{
    if (aEvent.IsEnabled && pViewFrame->GetViewShell()->isLOKMobilePhone())
    {
        boost::property_tree::ptree aTree;
        boost::property_tree::ptree aState;

        aTree.put("commandName", aEvent.FeatureURL.Complete);
        pViewFrame->GetBindings().QueryControlState(nSID, aState);
        aTree.add_child("state", aState);

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        return OString(aStream.str());
    }
    return aEvent.FeatureURL.Complete.toUtf8() + "="; // Should an empty string be returned?
}

OString StringPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                      const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (OUString aString; aEvent.IsEnabled && (aEvent.State >>= aString))
        aBuffer.append(aString.toUtf8());
    return aBuffer.makeStringAndClear();
}

OString RowColSelCountPayload(sal_uInt16, SfxViewFrame*,
                              const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem*)
{
    OUString aString;
    if (aEvent.IsEnabled)
        aEvent.State >>= aString;
    tools::JsonWriter aTree;
    aTree.put("commandName", aEvent.FeatureURL.Complete);
    aTree.put("locale", comphelper::LibreOfficeKit::getLocale().getBcp47());
    aTree.put("state", aString);
    return aTree.finishAndGetAsOString();
}

OString StateTableCellPayload(sal_uInt16, SfxViewFrame*,
                              const css::frame::FeatureStateEvent& aEvent,
                              const SfxPoolItem* pState)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (aEvent.IsEnabled)
        if (const SfxStringItem* pSvxStatusItem = dynamic_cast<const SfxStringItem*>(pState))
            aBuffer.append(pSvxStatusItem->GetValue().toUtf8());
    return aBuffer.makeStringAndClear();
}

OString BooleanPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                       const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (bool aBool; aEvent.IsEnabled && (aEvent.State >>= aBool))
        aBuffer.append(aBool);
    return aBuffer.makeStringAndClear();
}

OString BooleanOrDisabledPayload(sal_uInt16, SfxViewFrame*,
                                 const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (bool aBool; aEvent.IsEnabled && (aEvent.State >>= aBool))
        aBuffer.append(aBool);
    else
        aBuffer.append("disabled");
    return aBuffer.makeStringAndClear();
}

OString PointPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                     const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (css::awt::Point aPoint; aEvent.IsEnabled && (aEvent.State >>= aPoint))
        aBuffer.append(OString::number(aPoint.X) + " / " + OString::number(aPoint.Y));
    return aBuffer.makeStringAndClear();
}

OString SizePayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                    const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (css::awt::Size aSize; aEvent.IsEnabled && (aEvent.State >>= aSize))
        aBuffer.append(OString::number(aSize.Width) + " x " + OString::number(aSize.Height));
    return aBuffer.makeStringAndClear();
}

OString StringOrStrSeqPayload(sal_uInt16, SfxViewFrame*,
                              const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (aEvent.IsEnabled)
    {
        if (OUString sValue; aEvent.State >>= sValue)
            aBuffer.append(sValue.toUtf8());
        else if (css::uno::Sequence<OUString> aSeq; aEvent.State >>= aSeq)
            aBuffer.append(aSeq[0].toUtf8());
    }
    return aBuffer.makeStringAndClear();
}

OString StrSeqPayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                      const SfxPoolItem*)
{
    OString json;
    if (aEvent.IsEnabled)
    {
        if (css::uno::Sequence<OUString> aSeq; aEvent.State >>= aSeq)
        {
            tools::JsonWriter aTree;
            for (const auto& s : aSeq)
                aTree.put(s.toUtf8(), "true");
            json = aTree.finishAndGetAsOString();
        }
    }
    return aEvent.FeatureURL.Complete.toUtf8() + "=" + json;
}

OString TableSizePayload(sal_uInt16, SfxViewFrame*, const css::frame::FeatureStateEvent& aEvent,
                         const SfxPoolItem*)
{
    OStringBuffer aBuffer(aEvent.FeatureURL.Complete.toUtf8() + "=");
    if (sal_Int32 nValue; aEvent.State >>= nValue)
        aBuffer.append(o3tl::convert<double>(nValue, o3tl::Length::twip, o3tl::Length::in));
    return aBuffer.makeStringAndClear();
}

constexpr auto handlers = frozen::make_unordered_map<std::u16string_view, PayloadGetter_t>({
    { u"Bold", IsActivePayload },
    { u"CenterPara", IsActivePayload },
    { u"CharBackgroundExt", IsActivePayload },
    { u"ControlCodes", IsActivePayload },
    { u"DefaultBullet", IsActivePayload },
    { u"DefaultNumbering", IsActivePayload },
    { u"Italic", IsActivePayload },
    { u"JustifyPara", IsActivePayload },
    { u"LeftPara", IsActivePayload },
    { u"OutlineFont", IsActivePayload },
    { u"RightPara", IsActivePayload },
    { u"Shadowed", IsActivePayload },
    { u"SpellOnline", IsActivePayload },
    { u"OnlineAutoFormat", IsActivePayload },
    { u"SubScript", IsActivePayload },
    { u"SuperScript", IsActivePayload },
    { u"Strikeout", IsActivePayload },
    { u"Underline", IsActivePayload },
    { u"ModifiedStatus", IsActivePayload },
    { u"TrackChanges", IsActivePayload },
    { u"ShowTrackedChanges", IsActivePayload },
    { u"AlignLeft", IsActivePayload },
    { u"AlignHorizontalCenter", IsActivePayload },
    { u"AlignRight", IsActivePayload },
    { u"DocumentRepair", IsActivePayload },
    { u"ObjectAlignLeft", IsActivePayload },
    { u"ObjectAlignRight", IsActivePayload },
    { u"AlignCenter", IsActivePayload },
    { u"AlignUp", IsActivePayload },
    { u"AlignMiddle", IsActivePayload },
    { u"AlignDown", IsActivePayload },
    { u"TraceChangeMode", IsActivePayload },
    { u"FormatPaintbrush", IsActivePayload },
    { u"FreezePanes", IsActivePayload },
    { u"Sidebar", IsActivePayload },
    { u"SpacePara1", IsActivePayload },
    { u"SpacePara15", IsActivePayload },
    { u"SpacePara2", IsActivePayload },
    { u"DataFilterAutoFilter", IsActivePayload },
    { u"CellProtection", IsActivePayload },
    { u"NormalMultiPaneGUI", IsActivePayload },
    { u"NotesMode", IsActivePayload },
    { u"SlideMasterPage", IsActivePayload },

    { u"CharFontName", FontNamePayload },

    { u"FontHeight", FontHeightPayload },

    { u"StyleApply", StyleApplyPayload },

    { u"BackColor", ColorPayload },
    { u"BackgroundColor", ColorPayload },
    { u"CharBackColor", ColorPayload },
    { u"Color", ColorPayload },
    { u"FontColor", ColorPayload },
    { u"FrameLineColor", ColorPayload },
    { u"GlowColor", ColorPayload },

    { u"Undo", UndoRedoPayload },
    { u"Redo", UndoRedoPayload },

    { u"Cut", EnabledPayload },
    { u"Copy", EnabledPayload },
    { u"Paste", EnabledPayload },
    { u"SelectAll", EnabledPayload },
    { u"InsertAnnotation", EnabledPayload },
    { u"DeleteAnnotation", EnabledPayload },
    { u"ResolveAnnotation", EnabledPayload },
    { u"ResolveAnnotationThread", EnabledPayload },
    { u"InsertRowsBefore", EnabledPayload },
    { u"InsertRowsAfter", EnabledPayload },
    { u"InsertColumnsBefore", EnabledPayload },
    { u"InsertColumnsAfter", EnabledPayload },
    { u"NameGroup", EnabledPayload },
    { u"ObjectTitleDescription", EnabledPayload },
    { u"MergeCells", EnabledPayload },
    { u"InsertObjectChart", EnabledPayload },
    { u"InsertSection", EnabledPayload },
    { u"InsertPagebreak", EnabledPayload },
    { u"InsertColumnBreak", EnabledPayload },
    { u"HyperlinkDialog", EnabledPayload },
    { u"InsertSymbol", EnabledPayload },
    { u"InsertPage", EnabledPayload },
    { u"DeletePage", EnabledPayload },
    { u"DuplicatePage", EnabledPayload },
    { u"DeleteRows", EnabledPayload },
    { u"DeleteColumns", EnabledPayload },
    { u"DeleteTable", EnabledPayload },
    { u"SelectTable", EnabledPayload },
    { u"EntireRow", EnabledPayload },
    { u"EntireColumn", EnabledPayload },
    { u"EntireCell", EnabledPayload },
    { u"SortAscending", EnabledPayload },
    { u"SortDescending", EnabledPayload },
    { u"AcceptAllTrackedChanges", EnabledPayload },
    { u"RejectAllTrackedChanges", EnabledPayload },
    { u"AcceptTrackedChange", EnabledPayload },
    { u"RejectTrackedChange", EnabledPayload },
    { u"AcceptTrackedChangeToNext", EnabledPayload },
    { u"RejectTrackedChangeToNext", EnabledPayload },
    { u"NextTrackedChange", EnabledPayload },
    { u"PreviousTrackedChange", EnabledPayload },
    { u"FormatGroup", EnabledPayload },
    { u"ObjectBackOne", EnabledPayload },
    { u"SendToBack", EnabledPayload },
    { u"ObjectForwardOne", EnabledPayload },
    { u"BringToFront", EnabledPayload },
    { u"WrapRight", EnabledPayload },
    { u"WrapThrough", EnabledPayload },
    { u"WrapLeft", EnabledPayload },
    { u"WrapIdeal", EnabledPayload },
    { u"WrapOn", EnabledPayload },
    { u"WrapOff", EnabledPayload },
    { u"UpdateCurIndex", EnabledPayload },
    { u"InsertCaptionDialog", EnabledPayload },
    { u"SplitTable", EnabledPayload },
    { u"SplitCell", EnabledPayload },
    { u"DeleteNote", EnabledPayload },
    { u"AcceptChanges", EnabledPayload },
    { u"SetDefault", EnabledPayload },
    { u"ParaspaceIncrease", EnabledPayload },
    { u"ParaspaceDecrease", EnabledPayload },
    { u"TableDialog", EnabledPayload },
    { u"FormatCellDialog", EnabledPayload },
    { u"FontDialog", EnabledPayload },
    { u"ParagraphDialog", EnabledPayload },
    { u"OutlineBullet", EnabledPayload },
    { u"InsertIndexesEntry", EnabledPayload },
    { u"TransformDialog", EnabledPayload },
    { u"EditRegion", EnabledPayload },
    { u"ThesaurusDialog", EnabledPayload },
    { u"OutlineRight", EnabledPayload },
    { u"OutlineLeft", EnabledPayload },
    { u"OutlineDown", EnabledPayload },
    { u"OutlineUp", EnabledPayload },
    { u"FormatArea", EnabledPayload },
    { u"FormatLine", EnabledPayload },
    { u"FormatColumns", EnabledPayload },
    { u"Watermark", EnabledPayload },
    { u"InsertBreak", EnabledPayload },
    { u"InsertEndnote", EnabledPayload },
    { u"InsertFootnote", EnabledPayload },
    { u"InsertReferenceField", EnabledPayload },
    { u"InsertBookmark", EnabledPayload },
    { u"InsertAuthoritiesEntry", EnabledPayload },
    { u"InsertMultiIndex", EnabledPayload },
    { u"InsertField", EnabledPayload },
    { u"PageNumberWizard", EnabledPayload },
    { u"InsertPageNumberField", EnabledPayload },
    { u"InsertPageCountField", EnabledPayload },
    { u"InsertDateField", EnabledPayload },
    { u"InsertTitleField", EnabledPayload },
    { u"InsertFieldCtrl", EnabledPayload },
    { u"CharmapControl", EnabledPayload },
    { u"EnterGroup", EnabledPayload },
    { u"LeaveGroup", EnabledPayload },
    { u"Combine", EnabledPayload },
    { u"Merge", EnabledPayload },
    { u"Dismantle", EnabledPayload },
    { u"Substract", EnabledPayload },
    { u"DistributeSelection", EnabledPayload },
    { u"Intersect", EnabledPayload },
    { u"ResetAttributes", EnabledPayload },
    { u"IncrementIndent", EnabledPayload },
    { u"DecrementIndent", EnabledPayload },
    { u"EditHeaderAndFooter", EnabledPayload },
    { u"InsertSparkline", EnabledPayload },
    { u"DeleteSparkline", EnabledPayload },
    { u"DeleteSparklineGroup", EnabledPayload },
    { u"EditSparklineGroup", EnabledPayload },
    { u"EditSparkline", EnabledPayload },
    { u"GroupSparklines", EnabledPayload },
    { u"UngroupSparklines", EnabledPayload },
    { u"FormatSparklineMenu", EnabledPayload },
    { u"DataDataPilotRun", EnabledPayload },
    { u"RecalcPivotTable", EnabledPayload },
    { u"DeletePivotTable", EnabledPayload },
    { u"NumberFormatDecDecimals", EnabledPayload },
    { u"NumberFormatIncDecimals", EnabledPayload },
    { u"Protect", EnabledPayload },
    { u"UnsetCellsReadOnly", EnabledPayload },
    { u"ContentControlProperties", EnabledPayload },
    { u"InsertCheckboxContentControl", EnabledPayload },
    { u"InsertContentControl", EnabledPayload },
    { u"InsertDateContentControl", EnabledPayload },
    { u"InsertDropdownContentControl", EnabledPayload },
    { u"InsertPlainTextContentControl", EnabledPayload },
    { u"InsertPictureContentControl", EnabledPayload },
    { u"ChangeBezier", EnabledPayload },
    { u"DistributeHorzCenter", EnabledPayload },
    { u"DistributeHorzDistance", EnabledPayload },
    { u"DistributeHorzLeft", EnabledPayload },
    { u"DistributeHorzRight", EnabledPayload },
    { u"DistributeVertBottom", EnabledPayload },
    { u"DistributeVertCenter", EnabledPayload },
    { u"DistributeVertDistance", EnabledPayload },
    { u"DistributeVertTop", EnabledPayload },
    { u"AnimationEffects", EnabledPayload },
    { u"ExecuteAnimationEffect", EnabledPayload },

    { u"ParaLeftToRight", ParaDirectionPayload },
    { u"ParaRightToLeft", ParaDirectionPayload },

    { u"AssignLayout", Int32Payload },
    { u"StatusSelectionMode", Int32Payload },
    { u"Signature", Int32Payload },
    { u"SelectionMode", Int32Payload },
    { u"StatusBarFunc", Int32Payload },

    { u"TransformPosX", TransformPayload },
    { u"TransformPosY", TransformPayload },
    { u"TransformWidth", TransformPayload },
    { u"TransformHeight", TransformPayload },

    { u"StatusDocPos", StringPayload },
    { u"StatusPageStyle", StringPayload },
    { u"StateWordCount", StringPayload },
    { u"PageStyleName", StringPayload },
    { u"PageStatus", StringPayload },
    { u"LayoutStatus", StringPayload },
    { u"Scale", StringPayload },
    { u"Context", StringPayload },

    { u"RowColSelCount", RowColSelCountPayload },

    { u"StateTableCell", StateTableCellPayload },

    { u"InsertMode", BooleanPayload },
    { u"WrapText", BooleanPayload },
    { u"NumberFormatCurrency", BooleanPayload },
    { u"NumberFormatPercent", BooleanPayload },
    { u"NumberFormatDecimal", BooleanPayload },
    { u"NumberFormatDate", BooleanPayload },
    { u"ShowResolvedAnnotations", BooleanPayload },

    { u"ToggleMergeCells", BooleanOrDisabledPayload },
    { u"SheetRightToLeft", BooleanOrDisabledPayload },
    { u"ToggleSheetGrid", BooleanOrDisabledPayload },

    { u"Position", PointPayload },
    { u"FreezePanesColumn", PointPayload },
    { u"FreezePanesRow", PointPayload },

    { u"Size", SizePayload },

    { u"LanguageStatus", StringOrStrSeqPayload },
    { u"StatePageNumber", StringOrStrSeqPayload },

    { u"InsertPageHeader", StrSeqPayload },
    { u"InsertPageFooter", StrSeqPayload },

    { u"TableColumWidth", TableSizePayload },
    { u"TableRowHeight", TableSizePayload },
});
}

static void InterceptLOKStateChangeEvent(sal_uInt16 nSID, SfxViewFrame* pViewFrame, const css::frame::FeatureStateEvent& aEvent, const SfxPoolItem* pState)
{
    const SfxViewShell* pViewShell = pViewFrame->GetViewShell();
    if (!comphelper::LibreOfficeKit::isActive() || !pViewShell)
        return;

    auto handler = handlers.find(aEvent.FeatureURL.Path);
    if (handler == handlers.end())
    {
        // Try to send JSON state version
        SfxLokHelper::sendUnoStatus(pViewShell, pState);

        return;
    }

    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                           handler->second(nSID, pViewFrame, aEvent, pState));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
