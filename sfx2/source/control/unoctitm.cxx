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
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Template.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <officecfg/Office/Common.hxx>
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
#include <sfx2/viewsh.hxx>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <unotools/pathoptions.hxx>
#include <osl/time.h>

#include <iostream>
#include <map>
#include <memory>

#include <sal/log.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#define USAGE "file:///~/.config/libreofficedev/4/user/usage.txt"

using namespace ::com::sun::star;
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

SfxUnoControllerItem::SfxUnoControllerItem( SfxControllerItem *pItem, SfxBindings& rBind, const OUString& rCmd )
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
    pCtrlItem = nullptr;
    css::uno::Reference< css::frame::XStatusListener >  aRef( static_cast<cppu::OWeakObject*>(this), css::uno::UNO_QUERY );
    ReleaseDispatch();
}

void SAL_CALL SfxUnoControllerItem::statusChanged(const css::frame::FeatureStateEvent& rEvent) throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    DBG_ASSERT( pCtrlItem, "dispatch implementation didn't respect our previous removeStatusListener call!" );

    if ( rEvent.Requery )
    {
        // Error can only happen if the old Dispatch is implemented incorrectly
        // i.e. removeStatusListener did not work. But such things can happen...
        // So protect before ReleaseDispatch from release!
        css::uno::Reference< css::frame::XStatusListener >  aRef( static_cast<cppu::OWeakObject*>(this), css::uno::UNO_QUERY  );
        ReleaseDispatch();
        if ( pCtrlItem )
            GetNewDispatch();           // asynchronous ??
    }
    else if ( pCtrlItem )
    {
        SfxItemState eState = SfxItemState::DISABLED;
        SfxPoolItem* pItem = nullptr;
        if ( rEvent.IsEnabled )
        {
            eState = SfxItemState::DEFAULT;
            css::uno::Type aType = rEvent.State.getValueType();

            if ( aType == cppu::UnoType< bool >::get() )
            {
                bool bTemp = false;
                rEvent.State >>= bTemp ;
                pItem = new SfxBoolItem( pCtrlItem->GetId(), bTemp );
            }
            else if ( aType == cppu::UnoType< ::cppu::UnoUnsignedShortType >::get() )
            {
                sal_uInt16 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem = new SfxUInt16Item( pCtrlItem->GetId(), nTemp );
            }
            else if ( aType == cppu::UnoType<sal_uInt32>::get() )
            {
                sal_uInt32 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem = new SfxUInt32Item( pCtrlItem->GetId(), nTemp );
            }
            else if ( aType == cppu::UnoType<OUString>::get() )
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

void  SAL_CALL SfxUnoControllerItem::disposing( const css::lang::EventObject& ) throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::frame::XStatusListener >  aRef( static_cast<cppu::OWeakObject*>(this), css::uno::UNO_QUERY );
    ReleaseDispatch();
}

void SfxUnoControllerItem::ReleaseDispatch()
{
    if ( xDispatch.is() )
    {
        xDispatch->removeStatusListener( static_cast<css::frame::XStatusListener*>(this), aCommand );
        xDispatch.clear();
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
    xDispatch.clear();

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
        css::uno::Reference< css::frame::XFrame >  xFrame = rFrame.GetFrameInterface();
        css::uno::Reference< css::frame::XDispatchProvider >  xProv( xFrame, css::uno::UNO_QUERY );
        if ( xProv.is() )
            xDispatch = xProv->queryDispatch( aCommand, OUString(), 0 );
    }

    if ( xDispatch.is() )
        xDispatch->addStatusListener( static_cast<css::frame::XStatusListener*>(this), aCommand );
    else if ( pCtrlItem )
        pCtrlItem->StateChanged( pCtrlItem->GetId(), SfxItemState::DISABLED, nullptr );
}

css::uno::Reference< css::frame::XDispatch >  SfxUnoControllerItem::TryGetDispatch( SfxFrame *pFrame )
{
    css::uno::Reference< css::frame::XDispatch >  xDisp;
    SfxFrame *pParent = pFrame->GetParentFrame();
    if ( pParent )
        // parent may intercept
        xDisp = TryGetDispatch( pParent );

    return xDisp;
}

void SfxUnoControllerItem::ReleaseBindings()
{
    // connection to binding is lost; so forget the binding and the dispatch
    css::uno::Reference< css::frame::XStatusListener >  aRef( static_cast<cppu::OWeakObject*>(this), css::uno::UNO_QUERY );
    ReleaseDispatch();
    if ( pBindings )
        pBindings->ReleaseUnoController_Impl( this );
    pBindings = nullptr;
}

static void InterceptLOKStateChangeEvent(const SfxViewFrame* pViewFrame, const css::frame::FeatureStateEvent& aEvent);

void SfxStatusDispatcher::ReleaseAll()
{
    css::lang::EventObject aObject;
    aObject.Source = static_cast<cppu::OWeakObject*>(this);
    aListeners.disposeAndClear( aObject );
}

void SAL_CALL SfxStatusDispatcher::dispatch( const css::util::URL&, const css::uno::Sequence< css::beans::PropertyValue >& ) throw ( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL SfxStatusDispatcher::dispatchWithNotification(
    const css::util::URL&,
    const css::uno::Sequence< css::beans::PropertyValue >&,
    const css::uno::Reference< css::frame::XDispatchResultListener >& ) throw( css::uno::RuntimeException, std::exception )
{
}

SfxStatusDispatcher::SfxStatusDispatcher()
    : aListeners( aMutex )
{
}

void SAL_CALL SfxStatusDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) throw ( css::uno::RuntimeException, std::exception )
{
    aListeners.addInterface( aURL.Complete, aListener );
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

void SAL_CALL SfxStatusDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL ) throw ( css::uno::RuntimeException, std::exception )
{
    aListeners.removeInterface( aURL.Complete, aListener );
}


// XUnoTunnel
sal_Int64 SAL_CALL SfxOfficeDispatch::getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception)
{
    if ( aIdentifier == impl_getStaticIdentifier() )
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    else
        return 0;
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
    }
}

const css::uno::Sequence< sal_Int8 >& SfxOfficeDispatch::impl_getStaticIdentifier()
{
    // {38 57 CA 80 09 36 11 d4 83 FE 00 50 04 52 6B 21}
    static const sal_uInt8 pGUID[16] = { 0x38, 0x57, 0xCA, 0x80, 0x09, 0x36, 0x11, 0xd4, 0x83, 0xFE, 0x00, 0x50, 0x04, 0x52, 0x6B, 0x21 };
    static css::uno::Sequence< sal_Int8 > seqID(reinterpret_cast<const sal_Int8*>(pGUID), 16) ;
    return seqID ;
}


void SAL_CALL SfxOfficeDispatch::dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) throw ( css::uno::RuntimeException, std::exception )
{
    // ControllerItem is the Impl class
    if ( pImpl )
    {
#if HAVE_FEATURE_JAVA
        // The JavaContext contains an interaction handler which is used when
        // the creation of a Java Virtual Machine fails. The second parameter
        // indicates, that there shall only be one user notification (message box)
        // even if the same error (interaction) reoccurs. The effect is, that if a
        // user selects a menu entry than they may get only one notification that
        // a JRE is not selected.
        css::uno::ContextLayer layer(
            new svt::JavaContext( css::uno::getCurrentContext(),
                                  true) );
#endif
        pImpl->dispatch( aURL, aArgs, css::uno::Reference < css::frame::XDispatchResultListener >() );
    }
}

void SAL_CALL SfxOfficeDispatch::dispatchWithNotification( const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
        const css::uno::Reference< css::frame::XDispatchResultListener >& rListener ) throw( css::uno::RuntimeException, std::exception )
{
    // ControllerItem is the Impl class
    if ( pImpl )
    {
#if HAVE_FEATURE_JAVA
        // see comment for SfxOfficeDispatch::dispatch
        css::uno::ContextLayer layer( new svt::JavaContext( css::uno::getCurrentContext(), true) );
#endif
        pImpl->dispatch( aURL, aArgs, rListener );
    }
}

void SAL_CALL SfxOfficeDispatch::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) throw ( css::uno::RuntimeException, std::exception )
{
    GetListeners().addInterface( aURL.Complete, aListener );
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
    const css::util::URL&              rURL )
    : aDispatchURL( rURL )
    , pDispatcher( pDispat )
    , pBindings( pBind )
    , pLastState( nullptr )
    , nSlot( pSlot->GetSlotId() )
    , pDispatch( pDisp )
    , bMasterSlave( false )
    , bVisible( true )
    , pUnoName( pSlot->pUnoName )
{
    if ( aDispatchURL.Protocol == "slot:" && pUnoName )
    {
        OStringBuffer aTmp(".uno:");
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
        pDispatch->pImpl = nullptr;

        // force all listeners to release the dispatch object
        css::lang::EventObject aObject;
        aObject.Source = static_cast<cppu::OWeakObject*>(pDispatch);
        pDispatch->GetListeners().disposeAndClear( aObject );
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
                aParamName = aToken.getToken( 0, ':', nParmIndex );
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
                rArgs[nLen].Value <<= OUString( INetURLObject::decode( aValue, INetURLObject::DECODE_WITH_CHARSET ));
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

OUString SfxDispatchController_Impl::getSlaveCommand( const css::util::URL& rURL )
{
    OUString   aSlaveCommand;
    sal_Int32       nIndex = rURL.Path.indexOf( '.' );
    if (( nIndex > 0 ) && ( nIndex < rURL.Path.getLength() ))
        aSlaveCommand = rURL.Path.copy( nIndex+1 );
    return aSlaveCommand;
}

namespace {

/// Class that collects the usage information - how many times what .uno: command was used.
class UsageInfo {

    typedef std::map<OUString, int> UsageMap;

    /// Are we collecting the info?  We cache the value because the call to save can happen very late.
    bool mbIsCollecting;

    /// Command vs. how many times it was used
    UsageMap maUsage;

public:
    UsageInfo() : mbIsCollecting(false)
    {
    }

    ~UsageInfo()
    {
        save();
    }

    /// Increment command's use.
    void increment(const OUString &rCommand);

    /// Save the usage data for the next session.
    void save();

    /// Modify the flag whether we are collecting.
    void setCollecting(bool bIsCollecting) { mbIsCollecting = bIsCollecting; }
};

void UsageInfo::increment(const OUString &rCommand)
{
    UsageMap::iterator it = maUsage.find(rCommand);

    if (it != maUsage.end())
        ++(it->second);
    else
        maUsage[rCommand] = 1;
}

void UsageInfo::save()
{
    if (!mbIsCollecting)
        return;

    OUString path(SvtPathOptions().GetConfigPath());
    path += "usage/";
    osl::Directory::createPath(path);

    //get system time information.
    TimeValue systemTime;
    TimeValue localTime;
    oslDateTime localDateTime;
    osl_getSystemTime( &systemTime );
    osl_getLocalTimeFromSystemTime( &systemTime, &localTime );
    osl_getDateTimeFromTimeValue( &localTime, &localDateTime );

    sal_Char time[1024];
    sprintf(time,"%4i-%02i-%02iT%02i_%02i_%02i", localDateTime.Year, localDateTime.Month, localDateTime.Day, localDateTime.Hours, localDateTime.Minutes, localDateTime.Seconds);

    //filename type: usage-YYYY-MM-DDTHH_MM_SS.csv
    OUString filename = "usage-" + OUString::createFromAscii(time) + ".csv";
    path += filename;

    osl::File file(path);

    if( file.open(osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create) == osl::File::E_None )
    {
        OString aUsageInfoMsg = "Document Type;Command;Count";

        for (UsageMap::const_iterator it = maUsage.begin(); it != maUsage.end(); ++it)
            aUsageInfoMsg += "\n" + it->first.toUtf8() + ";" + OString::number(it->second);

        sal_uInt64 written = 0;
        file.write(aUsageInfoMsg.pData->buffer, aUsageInfoMsg.getLength(), written);
        file.close();
    }
}

class theUsageInfo : public rtl::Static<UsageInfo, theUsageInfo> {};

/// Extracts information about the command + args, and stores that.
void collectUsageInformation(const util::URL& rURL, const uno::Sequence<beans::PropertyValue>& rArgs)
{
    bool bCollecting = getenv("LO_COLLECT_USAGE") || officecfg::Office::Common::Misc::CollectUsageInformation::get();
    theUsageInfo::get().setCollecting(bCollecting);
    if (!bCollecting)
        return;

    OUStringBuffer aBuffer;

    // app identification [uh, several UNO calls :-(]
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XModuleManager2> xModuleManager(frame::ModuleManager::create(xContext));
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);
    uno::Reference<frame::XFrame> xFrame = xDesktop->getCurrentFrame();

    OUString aModule(xModuleManager->identify(xFrame));
    sal_Int32 nLastDot = aModule.lastIndexOf('.');
    if (nLastDot >= 0)
        aModule = aModule.copy(nLastDot + 1);

    aBuffer.append(aModule);
    aBuffer.append(';');

    // command
    aBuffer.append(rURL.Protocol);
    aBuffer.append(rURL.Path);
    sal_Int32 nCount = rArgs.getLength();

    // parameters - only their names, not the values (could be sensitive!)
    if (nCount > 0)
    {
        aBuffer.append('(');
        for (sal_Int32 n = 0; n < nCount; n++)
        {
            const css::beans::PropertyValue& rProp = rArgs[n];
            if (n > 0)
                aBuffer.append(',');
            aBuffer.append(rProp.Name);
        }
        aBuffer.append(')');
    }

    OUString aCommand(aBuffer.makeStringAndClear());

    // store
    theUsageInfo::get().increment(aCommand);
}

}

void SAL_CALL SfxDispatchController_Impl::dispatch( const css::util::URL& aURL,
        const css::uno::Sequence< css::beans::PropertyValue >& aArgs,
        const css::uno::Reference< css::frame::XDispatchResultListener >& rListener )
    throw (css::uno::RuntimeException, std::exception)
{
    collectUsageInformation(aURL, aArgs);

    SolarMutexGuard aGuard;
    if (
        pDispatch &&
        (
         (aURL.Protocol == ".uno:" && aURL.Path == aDispatchURL.Path) ||
         (aURL.Protocol == "slot:" && aURL.Path.toInt32() == GetId())
        )
       )
    {
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

            lNewArgs.realloc( lNewArgs.getLength()+aAddArgs.size() );
            for ( sal_uInt32 i = 0; i < nAddArgs; i++ )
                lNewArgs[nIndex++] = aAddArgs[i];
        }

        // Overwrite possible detected synchron argument, if real listener exists (currently no other way)
        if ( rListener.is() )
            nCall = SfxCallMode::SYNCHRON;

        if( GetId() == SID_JUMPTOMARK && nMarkArg == - 1 )
        {
            // we offer dispatches for SID_JUMPTOMARK if the URL points to a bookmark inside the document
            // so we must retrieve this as an argument from the parsed URL
            lNewArgs.realloc( lNewArgs.getLength()+1 );
            nMarkArg = lNewArgs.getLength()-1;
            lNewArgs[nMarkArg].Name = "Bookmark";
            lNewArgs[nMarkArg].Value <<= aURL.Mark;
        }

        css::uno::Reference< css::frame::XFrame > xFrameRef(xFrame.get(), css::uno::UNO_QUERY);
        if (! xFrameRef.is() && pDispatcher)
        {
            SfxViewFrame* pViewFrame = pDispatcher->GetFrame();
            if (pViewFrame)
                xFrameRef = pViewFrame->GetFrame().GetFrameInterface();
        }

        bool bSuccess = false;
        const SfxPoolItem* pItem = nullptr;
        SfxMapUnit eMapUnit( SFX_MAPUNIT_100TH_MM );

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
                if ( !pDispatcher->IsLocked( GetId() ) )
                {
                    const SfxSlot *pSlot = nullptr;
                    if ( pDispatcher->GetShellAndSlot_Impl( GetId(), &pShell, &pSlot, false,
                                                            SfxCallMode::MODAL==(nCall&SfxCallMode::MODAL), false ) )
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
                        std::unique_ptr<SfxAllItemSet> xSet(new SfxAllItemSet(pShell->GetPool()));
                        TransformParameters(GetId(), lNewArgs, *xSet, pSlot);
                        if (xSet->Count())
                        {
                            // execute with arguments - call directly
                            pItem = pDispatcher->Execute(GetId(), nCall, xSet.get(), &aInternalSet, nModifier);
                            if ( pItem != nullptr )
                            {
                                if ( dynamic_cast< const SfxBoolItem *>( pItem ) !=  nullptr )
                                    bSuccess = dynamic_cast< const SfxBoolItem *>( pItem )->GetValue();
                                else if ( dynamic_cast< const SfxVoidItem *>( pItem ) ==  nullptr )
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
                            pItem = aReq.GetReturnValue();
                            bSuccess = aReq.IsDone() || pItem != nullptr;
                        }
                    }
#ifdef DBG_UTIL
                    else
                        SAL_INFO("sfx.control", "MacroPlayer: Unknown slot dispatched!");
#endif
                }
            }
            else
            {
                eMapUnit = GetCoreMetric( SfxGetpApp()->GetPool(), GetId() );
                // AppDispatcher
                SfxAllItemSet aSet( SfxGetpApp()->GetPool() );
                TransformParameters( GetId(), lNewArgs, aSet );

                if ( aSet.Count() )
                    pItem = pDispatcher->Execute( GetId(), nCall, &aSet, &aInternalSet, nModifier );
                else
                    // SfxRequests take empty sets as argument sets, GetArgs() returning non-zero!
                    pItem = pDispatcher->Execute( GetId(), nCall, nullptr, &aInternalSet, nModifier );

                // no bindings, no invalidate ( usually done in SfxDispatcher::Call_Impl()! )
                if ( SfxApplication::Get() )
                {
                    SfxDispatcher* pAppDispat = SfxGetpApp()->GetAppDispatcher_Impl();
                    if ( pAppDispat )
                    {
                        const SfxPoolItem* pState=nullptr;
                        SfxItemState eState = pDispatcher->QueryState( GetId(), pState );
                        StateChanged( GetId(), eState, pState );
                    }
                }

                bSuccess = (pItem != nullptr);
            }
        }

        if ( rListener.is() )
        {
            css::frame::DispatchResultEvent aEvent;
            if ( bSuccess )
                aEvent.State = css::frame::DispatchResultState::SUCCESS;
            else
                aEvent.State = css::frame::DispatchResultState::FAILURE;

            aEvent.Source = static_cast<css::frame::XDispatch*>(pDispatch);
            if ( bSuccess && pItem && dynamic_cast< const SfxVoidItem *>( pItem ) ==  nullptr )
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

void SAL_CALL SfxDispatchController_Impl::addStatusListener(const css::uno::Reference< css::frame::XStatusListener > & aListener, const css::util::URL& aURL) throw ( css::uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !pDispatch )
        return;

    // Use alternative QueryState call to have a valid UNO representation of the state.
    css::uno::Any aState;
    if ( !pDispatcher && pBindings )
        pDispatcher = GetBindings().GetDispatcher_Impl();
    SfxItemState eState = pDispatcher ? pDispatcher->QueryState( GetId(), aState ) : SfxItemState::DONTCARE;

    if ( eState == SfxItemState::DONTCARE )
    {
        // Use special uno struct to transport don't care state
        css::frame::status::ItemStatus aItemStatus;
        aItemStatus.State = css::frame::status::ItemState::DONT_CARE;
        aState = makeAny( aItemStatus );
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
    bool bNotify = true;
    if ( pState && !IsInvalidItem( pState ) )
    {
        if ( dynamic_cast< const SfxVisibilityItem *>( pState ) ==  nullptr )
        {
            if (pLastState && !IsInvalidItem(pLastState))
            {
                bNotify = typeid(*pState) != typeid(*pLastState) || *pState != *pLastState;
                delete pLastState;
            }
            pLastState = !IsInvalidItem(pState) ? pState->Clone() : pState;
            bVisible = true;
        }
        else
            bVisible = static_cast<const SfxVisibilityItem *>(pState)->GetValue();
    }
    else
    {
        if ( pLastState && !IsInvalidItem( pLastState ) )
            delete pLastState;
        pLastState = pState;
    }

    if (bNotify)
    {
        css::uno::Any aState;
        if ( ( eState >= SfxItemState::DEFAULT ) && pState && !IsInvalidItem( pState ) && dynamic_cast< const SfxVoidItem *>( pState ) ==  nullptr )
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
        else if ( eState == SfxItemState::DONTCARE )
        {
            // Use special uno struct to transport don't care state
            css::frame::status::ItemStatus aItemStatus;
            aItemStatus.State = css::frame::status::ItemState::DONT_CARE;
            aState = makeAny( aItemStatus );
        }

        css::frame::FeatureStateEvent aEvent;
        aEvent.FeatureURL = aDispatchURL;
        aEvent.Source = static_cast<css::frame::XDispatch*>(pDispatch);
        aEvent.IsEnabled = eState != SfxItemState::DISABLED;
        aEvent.Requery = false;
        aEvent.State = aState;

        if (pDispatcher && pDispatcher->GetFrame())
        {
            InterceptLOKStateChangeEvent(pDispatcher->GetFrame(), aEvent);
        }

        ::cppu::OInterfaceContainerHelper* pContnr = pDispatch->GetListeners().getContainer ( aDispatchURL.Complete );
        if (pContnr) {
            ::cppu::OInterfaceIteratorHelper aIt( *pContnr );
            while( aIt.hasMoreElements() )
            {
                try
                {
                    static_cast< css::frame::XStatusListener *>(aIt.next())->statusChanged( aEvent );
                }
                catch (const css::uno::RuntimeException&)
                {
                    aIt.remove();
                }
            }
        }
    }
}

void SfxDispatchController_Impl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    StateChanged( nSID, eState, pState, nullptr );
}

static void InterceptLOKStateChangeEvent(const SfxViewFrame* pViewFrame, const css::frame::FeatureStateEvent& aEvent)
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    OUStringBuffer aBuffer;
    aBuffer.append(aEvent.FeatureURL.Complete);
    aBuffer.append(static_cast<sal_Unicode>('='));

    if (aEvent.FeatureURL.Path == "Bold" ||
        aEvent.FeatureURL.Path == "CenterPara" ||
        aEvent.FeatureURL.Path == "CharBackgroundExt" ||
        aEvent.FeatureURL.Path == "DefaultBullet" ||
        aEvent.FeatureURL.Path == "DefaultNumbering" ||
        aEvent.FeatureURL.Path == "Italic" ||
        aEvent.FeatureURL.Path == "JustifyPara" ||
        aEvent.FeatureURL.Path == "LeftPara" ||
        aEvent.FeatureURL.Path == "OutlineFont" ||
        aEvent.FeatureURL.Path == "RightPara" ||
        aEvent.FeatureURL.Path == "Shadowed" ||
        aEvent.FeatureURL.Path == "SubScript" ||
        aEvent.FeatureURL.Path == "SuperScript" ||
        aEvent.FeatureURL.Path == "Strikeout" ||
        aEvent.FeatureURL.Path == "Underline" ||
        aEvent.FeatureURL.Path == "ModifiedStatus")
    {
        bool bTemp = false;
        aEvent.State >>= bTemp;
        aBuffer.append(bTemp);
    }
    else if (aEvent.FeatureURL.Path == "CharFontName")
    {
        css::awt::FontDescriptor aFontDesc;
        aEvent.State >>= aFontDesc;
        aBuffer.append(aFontDesc.Name);
    }
    else if (aEvent.FeatureURL.Path == "FontHeight")
    {
        css::frame::status::FontHeight aFontHeight;
        aEvent.State >>= aFontHeight;
        aBuffer.append(aFontHeight.Height);
    }
    else if (aEvent.FeatureURL.Path == "StyleApply")
    {
        css::frame::status::Template aTemplate;
        aEvent.State >>= aTemplate;
        aBuffer.append(aTemplate.StyleName);
    }
    else if (aEvent.FeatureURL.Path == "BackColor" ||
             aEvent.FeatureURL.Path == "BackgroundColor" ||
             aEvent.FeatureURL.Path == "CharBackColor" ||
             aEvent.FeatureURL.Path == "Color" ||
             aEvent.FeatureURL.Path == "FontColor")
    {
        sal_Int32 nColor = -1;
        aEvent.State >>= nColor;
        aBuffer.append(nColor);
    }
    else if (aEvent.FeatureURL.Path == "Undo" ||
             aEvent.FeatureURL.Path == "Redo" ||
             aEvent.FeatureURL.Path == "Cut" ||
             aEvent.FeatureURL.Path == "Copy" ||
             aEvent.FeatureURL.Path == "Paste" ||
             aEvent.FeatureURL.Path == "SelectAll" ||
             aEvent.FeatureURL.Path == "InsertAnnotation" ||
             aEvent.FeatureURL.Path == "InsertRowsBefore" ||
             aEvent.FeatureURL.Path == "InsertRowsAfter" ||
             aEvent.FeatureURL.Path == "InsertColumnsBefore" ||
             aEvent.FeatureURL.Path == "InsertColumnsAfter" ||
             aEvent.FeatureURL.Path == "DeleteRows" ||
             aEvent.FeatureURL.Path == "DeleteColumns" ||
             aEvent.FeatureURL.Path == "DeleteTable" ||
             aEvent.FeatureURL.Path == "SelectTable" ||
             aEvent.FeatureURL.Path == "EntireRow" ||
             aEvent.FeatureURL.Path == "EntireColumn" ||
             aEvent.FeatureURL.Path == "EntireCell" ||
             aEvent.FeatureURL.Path == "MergeCells")
    {
        aBuffer.append(aEvent.IsEnabled ? OUString("enabled") : OUString("disabled"));
    }
    else if (aEvent.FeatureURL.Path == "InsertPage" ||
             aEvent.FeatureURL.Path == "DeletePage" ||
             aEvent.FeatureURL.Path == "DuplicatePage" ||
             aEvent.FeatureURL.Path == "SortAscending" ||
             aEvent.FeatureURL.Path == "SortDescending")
    {
        aBuffer.append(OUString::boolean(aEvent.IsEnabled));
    }
    else if (aEvent.FeatureURL.Path == "AssignLayout" ||
             aEvent.FeatureURL.Path == "StatusSelectionMode" ||
             aEvent.FeatureURL.Path == "Signature" ||
             aEvent.FeatureURL.Path == "SelectionMode" ||
             aEvent.FeatureURL.Path == "StatusBarFunc")
    {
        sal_Int32 aInt32;

        if (aEvent.IsEnabled && (aEvent.State >>= aInt32))
        {
            aBuffer.append(OUString::number(aInt32));
        }
    }
    else if (aEvent.FeatureURL.Path == "StatusDocPos" ||
             aEvent.FeatureURL.Path == "RowColSelCount" ||
             aEvent.FeatureURL.Path == "StatusPageStyle" ||
             aEvent.FeatureURL.Path == "StateTableCell" ||
             aEvent.FeatureURL.Path == "StatePageNumber" ||
             aEvent.FeatureURL.Path == "StateWordCount" ||
             aEvent.FeatureURL.Path == "PageStyleName" ||
             aEvent.FeatureURL.Path == "PageStatus" ||
             aEvent.FeatureURL.Path == "LayoutStatus" ||
             aEvent.FeatureURL.Path == "Context")
    {
        OUString aString;

        if (aEvent.IsEnabled && (aEvent.State >>= aString))
        {
            aBuffer.append(aString);
        }
    }
    else if (aEvent.FeatureURL.Path == "InsertMode" ||
             aEvent.FeatureURL.Path == "WrapText" ||
             aEvent.FeatureURL.Path == "ToggleMergeCells" ||
             aEvent.FeatureURL.Path == "NumberFormatCurrency" ||
             aEvent.FeatureURL.Path == "NumberFormatPercent" ||
             aEvent.FeatureURL.Path == "NumberFormatDate")
    {
        sal_Bool aBool;

        if (aEvent.IsEnabled && (aEvent.State >>= aBool))
        {
            aBuffer.append(OUString::boolean(aBool));
        }
    }
    else if (aEvent.FeatureURL.Path == "Position")
    {
        css::awt::Point aPoint;

        if (aEvent.IsEnabled && (aEvent.State >>= aPoint))
        {
            aBuffer.append(OUString::number(aPoint.X) + OUString(" / ") + OUString::number(aPoint.Y));
        }
    }
    else if (aEvent.FeatureURL.Path == "Size")
    {
        css::awt::Size aSize;

        if (aEvent.IsEnabled && (aEvent.State >>= aSize))
        {
            aBuffer.append(OUString::number(aSize.Width) + OUString(" x ") + OUString::number(aSize.Height));
        }
    }
    else
    {
        return;
    }

    OUString payload = aBuffer.makeStringAndClear();
    if (const SfxViewShell* pViewShell = pViewFrame->GetViewShell())
        pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, payload.toUtf8().getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
