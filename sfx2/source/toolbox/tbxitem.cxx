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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifdef SOLARIS
#include <ctime>
#endif

#include <string>           // prevent conflict with STL includes
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/imageitm.hxx>
#include <svl/visitem.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/toolbox.hxx>
#include <unotools/moduleoptions.hxx>

#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <framework/addonmenu.hxx>
#include <framework/addonsoptions.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <vcl/taskpanelist.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>

#include <sfx2/tbxctrl.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/dispatch.hxx>
#include "fltfnc.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include "statcach.hxx"
#include <sfx2/viewfrm.hxx>
#include "arrdecl.hxx"
#include "sfxtypes.hxx"
#include <sfx2/macrconf.hxx>
#include <sfx2/genlink.hxx>
#include "sfxresid.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/module.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include "referers.hxx"
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include "helpid.hrc"
#include "workwin.hxx"
#include "imgmgr.hxx"
#include "virtmenu.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include "imagemgr.hxx"

#include <comphelper/uieventslogger.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

//====================================================================

SFX_IMPL_TOOLBOX_CONTROL_ARG(SfxToolBoxControl, SfxStringItem, TRUE);
SFX_IMPL_TOOLBOX_CONTROL(SfxAppToolBoxControl_Impl, SfxStringItem);

static Window* GetTopMostParentSystemWindow( Window* pWindow )
{
    OSL_ASSERT( pWindow );
    if ( pWindow )
    {
        // ->manually search topmost system window
        // required because their might be another system window between this and the top window
        pWindow = pWindow->GetParent();
        SystemWindow* pTopMostSysWin = NULL;
        while ( pWindow )
        {
            if ( pWindow->IsSystemWindow() )
                pTopMostSysWin = (SystemWindow*)pWindow;
            pWindow = pWindow->GetParent();
        }
        pWindow = pTopMostSysWin;
        OSL_ASSERT( pWindow );
        return pWindow;
    }

    return NULL;
}

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL )
{
    SolarMutexGuard aGuard;

    URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );
    if ( aTargetURL.Arguments.getLength() )
        return NULL;

    SfxObjectShell* pObjShell = NULL;
    Reference < XController > xController;
    Reference < XModel > xModel;
    if ( rFrame.is() )
    {
        xController = rFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    if ( xModel.is() )
    {
        // Get tunnel from model to retrieve the SfxObjectShell pointer from it
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xModel, UNO_QUERY );
        if ( xObj.is() )
        {
            ::com::sun::star::uno::Sequence < sal_Int8 > aSeq = SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence();
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
                pObjShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
        }
    }

    SfxModule*     pModule   = pObjShell ? pObjShell->GetModule() : NULL;
    SfxSlotPool*   pSlotPool = 0;

    if ( pModule )
        pSlotPool = pModule->GetSlotPool();
    else
        pSlotPool = &(SfxSlotPool::GetSlotPool( NULL ));

    const SfxSlot* pSlot = pSlotPool->GetUnoSlot( aTargetURL.Path );
    if ( pSlot )
    {
        USHORT nSlotId = pSlot->GetSlotId();
        if ( nSlotId > 0 )
            return SfxToolBoxControl::CreateControl( nSlotId, nID, pToolbox, pModule );
    }

    return NULL;
}

struct SfxToolBoxControl_Impl
{
    ToolBox*                pBox;
    BOOL                    bShowString;
    USHORT                  nSelectModifier;
    SfxTbxCtrlFactory*      pFact;
    USHORT                  nTbxId;
    USHORT                  nSlotId;
    SfxPopupWindow*         mpFloatingWindow;
    SfxPopupWindow*         mpPopupWindow;
    Reference< XUIElement > mxUIElement;

    DECL_LINK( WindowEventListener, VclSimpleEvent* );
};

IMPL_LINK( SfxToolBoxControl_Impl, WindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( pEvent &&
         pEvent->ISA( VclWindowEvent ) &&
         (( pEvent->GetId() == VCLEVENT_WINDOW_MOVE ) ||
          ( pEvent->GetId() == VCLEVENT_WINDOW_ACTIVATE )))
    {
        Window* pWindow( ((VclWindowEvent*)pEvent)->GetWindow() );
        if (( pWindow == mpFloatingWindow ) &&
            ( mpPopupWindow != 0 ))
        {
            delete mpPopupWindow;
            mpPopupWindow = 0;
        }
    }

    return 1;
}

//--------------------------------------------------------------------

SfxToolBoxControl::SfxToolBoxControl(
    USHORT          nSlotID,
    USHORT          nID,
    ToolBox&        rBox,
    BOOL            bShowStringItems     )
:   svt::ToolboxController()
{
    pImpl = new SfxToolBoxControl_Impl;

    pImpl->pBox = &rBox;
    pImpl->bShowString = bShowStringItems;
    pImpl->nSelectModifier = 0;
    pImpl->pFact = 0;
    pImpl->nTbxId = nID;
    pImpl->nSlotId = nSlotID;
    pImpl->mpFloatingWindow = 0;
    pImpl->mpPopupWindow = 0;
}

//--------------------------------------------------------------------

SfxToolBoxControl::~SfxToolBoxControl()
{
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
        xComponent->dispose();
    }
    pImpl->mxUIElement = 0;
    delete pImpl;
}

//--------------------------------------------------------------------

ToolBox& SfxToolBoxControl::GetToolBox() const
{
    return *pImpl->pBox;
}
unsigned short SfxToolBoxControl::GetId() const
{
    return pImpl->nTbxId;
}
unsigned short SfxToolBoxControl::GetSlotId() const
{
    return pImpl->nSlotId;
}

//--------------------------------------------------------------------

void SAL_CALL SfxToolBoxControl::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_bDisposed )
        return;

    svt::ToolboxController::dispose();

    // Remove and destroy our item window at our toolbox
    SolarMutexGuard aGuard;
    Window* pWindow = pImpl->pBox->GetItemWindow( pImpl->nTbxId );
    pImpl->pBox->SetItemWindow( pImpl->nTbxId, 0 );
    delete pWindow;

    // Dispose an open sub toolbar. It's possible that we have an open
    // sub toolbar while we get disposed. Therefore we have to dispose
    // it now! Not doing so would result in a crash. The sub toolbar
    // gets destroyed asynchronously and would access a non-existing
    // parent toolbar! See #126569#
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
        xComponent->dispose();
    }
    pImpl->mxUIElement = 0;

    // Delete my popup windows
    delete pImpl->mpFloatingWindow;
    delete pImpl->mpPopupWindow;

    pImpl->mpFloatingWindow = 0;
    pImpl->mpPopupWindow = 0;
}

//--------------------------------------------------------------------
void SfxToolBoxControl::RegisterToolBoxControl( SfxModule* pMod, SfxTbxCtrlFactory* pFact)
{
    SFX_APP()->RegisterToolBoxControl_Impl( pMod, pFact );
}

SfxToolBoxControl* SfxToolBoxControl::CreateControl( USHORT nSlotId, USHORT nTbxId, ToolBox *pBox, SfxModule* pMod  )
{
    SolarMutexGuard aGuard;

    SfxToolBoxControl *pCtrl;
    SfxApplication *pApp = SFX_APP();

    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &SfxSlotPool::GetSlotPool();
    TypeId aSlotType = pSlotPool->GetSlotType( nSlotId );
    if ( aSlotType )
    {
        if ( pMod )
        {
            SfxTbxCtrlFactArr_Impl *pFactories = pMod->GetTbxCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxTbxCtrlFactArr_Impl &rFactories = *pFactories;
                USHORT nFactory;
                const USHORT nCount = rFactories.Count();

                // search for a factory with the given slot id
                for( nFactory = 0; nFactory < nCount; ++nFactory )
                    if( (rFactories[nFactory]->nTypeId == aSlotType) && (rFactories[nFactory]->nSlotId == nSlotId) )
                        break;

                if( nFactory == nCount )
                {
                    // if no factory exists for the given slot id, see if we
                    // have a generic factory with the correct slot type and slot id == 0
                    for ( nFactory = 0; nFactory < nCount; ++nFactory )
                        if( (rFactories[nFactory]->nTypeId == aSlotType) && (rFactories[nFactory]->nSlotId == 0) )
                            break;
                }

                if( nFactory < nCount )
                {
                    pCtrl = rFactories[nFactory]->pCtor( nSlotId, nTbxId, *pBox );
                    pCtrl->pImpl->pFact = rFactories[nFactory];
                    return pCtrl;
                }
            }
        }

        SfxTbxCtrlFactArr_Impl &rFactories = pApp->GetTbxCtrlFactories_Impl();
        USHORT nFactory;
        const USHORT nCount = rFactories.Count();

        for( nFactory = 0; nFactory < nCount; ++nFactory )
            if( (rFactories[nFactory]->nTypeId == aSlotType) && (rFactories[nFactory]->nSlotId == nSlotId) )
                break;

        if( nFactory == nCount )
        {
            // if no factory exists for the given slot id, see if we
            // have a generic factory with the correct slot type and slot id == 0
            for( nFactory = 0; nFactory < nCount; ++nFactory )
                if( (rFactories[nFactory]->nTypeId == aSlotType) && (rFactories[nFactory]->nSlotId == 0) )
                    break;
        }

        if( nFactory < nCount )
        {
            pCtrl = rFactories[nFactory]->pCtor( nSlotId, nTbxId, *pBox );
            pCtrl->pImpl->pFact = rFactories[nFactory];
            return pCtrl;
        }
    }

    return NULL;
}

SfxItemState SfxToolBoxControl::GetItemState(
    const SfxPoolItem* pState )
/*  [Beschreibung]

    Statische Methode zum Ermitteln des Status des SfxPoolItem-Pointers,
    in der Methode <SfxControllerItem::StateChanged(const SfxPoolItem*)>
    zu verwenden.

    [R"uckgabewert]

    SfxItemState        SFX_ITEM_UNKNOWN
                        Enabled, aber keine weitere Statusinformation
                        verf"ugbar. Typisch f"ur <Slot>s, die allenfalls
                        zeitweise disabled sind, aber ihre Darstellung sonst
                        nicht "andern.

                        SFX_ITEM_DISABLED
                        Disabled und keine weiter Statusinformation
                        verf"ugbar. Alle anderen ggf. angezeigten Werte sollten
                        auf den Default zur"uckgesetzt werden.

                        SFX_ITEM_DONTCARE
                        Enabled aber es waren nur uneindeutige Werte
                        verf"ugbar (also keine, die abgefragt werden k"onnen).

                        SFX_ITEM_AVAILABLE
                        Enabled und mit verf"ugbarem Wert, der von 'pState'
                        erfragbar ist. Der Typ ist dabei im gesamten
                        Programm eindeutig und durch den Slot festgelegt.
*/

{
    return !pState
                ? SFX_ITEM_DISABLED
                : IsInvalidItem(pState)
                    ? SFX_ITEM_DONTCARE
                    : pState->ISA(SfxVoidItem) && !pState->Which()
                        ? SFX_ITEM_UNKNOWN
                        : SFX_ITEM_AVAILABLE;
}

void SfxToolBoxControl::Dispatch(
    const Reference< XDispatchProvider >& rProvider,
    const rtl::OUString& rCommand,
    Sequence< ::PropertyValue >& aArgs )
{
    if ( rProvider.is() )
    {
        ::com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = rCommand;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
                                          UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = rProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

void SfxToolBoxControl::Dispatch( const ::rtl::OUString& aCommand, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
{
    Reference < XController > xController;

    SolarMutexGuard aGuard;
    if ( getFrameInterface().is() )
        xController = getFrameInterface()->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        ::com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = aCommand;
        getURLTransformer()->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = xProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
        {
            if(::comphelper::UiEventsLogger::isEnabled()) //#i88653#
            {
                ::rtl::OUString sAppName;
                try
                {
                    static ::rtl::OUString our_aModuleManagerName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ModuleManager"));
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager =
                        ::comphelper::getProcessServiceFactory();
                    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager(
                        xServiceManager->createInstance(our_aModuleManagerName)
                        , ::com::sun::star::uno::UNO_QUERY_THROW);
                    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame(
                        getFrameInterface(), UNO_QUERY_THROW);
                    sAppName = xModuleManager->identify(xFrame);
                } catch(::com::sun::star::uno::Exception&) {}
                Sequence<PropertyValue> source;
                ::comphelper::UiEventsLogger::appendDispatchOrigin(source, sAppName, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SfxToolBoxControl")));
                ::comphelper::UiEventsLogger::logDispatch(aTargetURL, source);
            }
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

// XInterface
Any SAL_CALL SfxToolBoxControl::queryInterface( const Type & rType )
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                           SAL_STATIC_CAST( ::com::sun::star::awt::XDockableWindowListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::frame::XSubToolbarController*, this ));
    return (aRet.hasValue() ? aRet : svt::ToolboxController::queryInterface( rType ));
}

void SAL_CALL SfxToolBoxControl::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SfxToolBoxControl::release() throw()
{
    OWeakObject::release();
}

void SAL_CALL SfxToolBoxControl::disposing( const ::com::sun::star::lang::EventObject& aEvent )
throw( ::com::sun::star::uno::RuntimeException )
{
    svt::ToolboxController::disposing( aEvent );
}

// XStatusListener
void SAL_CALL SfxToolBoxControl::statusChanged( const FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxViewFrame* pViewFrame = NULL;
    Reference < XController > xController;

    SolarMutexGuard aGuard;
    if ( getFrameInterface().is() )
        xController = getFrameInterface()->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = reinterpret_cast< SfxOfficeDispatch* >( sal::static_int_cast< sal_IntPtr >( nImplementation ));
            }

            if ( pDisp )
                pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
        }
    }

    USHORT nSlotId = 0;
    SfxSlotPool& rPool = SfxSlotPool::GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetUnoSlot( rEvent.FeatureURL.Path );
    if ( pSlot )
        nSlotId = pSlot->GetSlotId();
    else if ( m_aCommandURL == rEvent.FeatureURL.Path )
        nSlotId = GetSlotId();

    if ( nSlotId > 0 )
    {
        if ( rEvent.Requery )
            svt::ToolboxController::statusChanged( rEvent );
        else
        {
            SfxItemState eState = SFX_ITEM_DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SFX_ITEM_AVAILABLE;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == ::getVoidCppuType() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SFX_ITEM_UNKNOWN;
                }
                else if ( pType == ::getBooleanCppuType() )
                {
                    sal_Bool bTemp = false;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    eState = aItemStatus.State;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::Visibility*)0) )
                {
                    Visibility aVisibilityStatus;
                    rEvent.State >>= aVisibilityStatus;
                    pItem = new SfxVisibilityItem( nSlotId, aVisibilityStatus.bVisible );
                }
                else
                {
                    if ( pSlot )
                        pItem = pSlot->GetType()->CreateItem();
                    if ( pItem )
                    {
                        pItem->SetWhich( nSlotId );
                        pItem->PutValue( rEvent.State );
                    }
                    else
                        pItem = new SfxVoidItem( nSlotId );
                }
            }

            StateChanged( nSlotId, eState, pItem );
            delete pItem;
        }
    }
}

// XSubToolbarController
::sal_Bool SAL_CALL SfxToolBoxControl::opensSubToolbar() throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::rtl::OUString SAL_CALL SfxToolBoxControl::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException)
{
    return rtl::OUString();
}

void SAL_CALL SfxToolBoxControl::functionSelected( const ::rtl::OUString& /*aCommand*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    // must be implemented by sub-class
}

void SAL_CALL SfxToolBoxControl::updateImage() throw (::com::sun::star::uno::RuntimeException)
{
    // must be implemented by sub-class
}

// XToolbarController
void SAL_CALL SfxToolBoxControl::execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Select( (USHORT)KeyModifier );
}
void SAL_CALL SfxToolBoxControl::click() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Click();
}

void SAL_CALL SfxToolBoxControl::doubleClick() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    DoubleClick();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createPopupWindow() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window* pWindow = CreatePopupWindow();
    if ( pWindow )
        return VCLUnoHelper::GetInterface( pWindow );
    else
        return Reference< ::com::sun::star::awt::XWindow >();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createItemWindow( const Reference< ::com::sun::star::awt::XWindow >& rParent ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return VCLUnoHelper::GetInterface( CreateItemWindow( VCLUnoHelper::GetWindow( rParent )));
}

// XDockableWindowListener
void SAL_CALL SfxToolBoxControl::startDocking( const ::com::sun::star::awt::DockingEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
}
::com::sun::star::awt::DockingData SAL_CALL SfxToolBoxControl::docking( const ::com::sun::star::awt::DockingEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return ::com::sun::star::awt::DockingData();
}

void SAL_CALL SfxToolBoxControl::endDocking( const ::com::sun::star::awt::EndDockingEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
}

sal_Bool SAL_CALL SfxToolBoxControl::prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SfxToolBoxControl::toggleFloatingMode( const ::com::sun::star::lang::EventObject& )
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SfxToolBoxControl::closed( const ::com::sun::star::lang::EventObject& )
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SfxToolBoxControl::endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& aEvent )
throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aSubToolBarResName;
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XPropertySet > xPropSet( pImpl->mxUIElement, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ))) >>= aSubToolBarResName;
            }
            catch ( com::sun::star::beans::UnknownPropertyException& )
            {
            }
            catch ( com::sun::star::lang::WrappedTargetException& )
            {
            }
        }

        Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
        xComponent->dispose();
    }
    pImpl->mxUIElement = 0;

    // if the toolbar was teared-off recreate it and place it at the given position
    if( aEvent.bTearoff )
    {
        Reference< XUIElement >     xUIElement;
        Reference< XLayoutManager > xLayoutManager = getLayoutManager();

        if ( !xLayoutManager.is() )
            return;

        xLayoutManager->createElement( aSubToolBarResName );
        xUIElement = xLayoutManager->getElement( aSubToolBarResName );
        if ( xUIElement.is() )
        {
            Reference< ::com::sun::star::awt::XWindow > xParent = getFrameInterface()->getContainerWindow();

            Reference< ::com::sun::star::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), UNO_QUERY );
            Reference< ::com::sun::star::beans::XPropertySet > xProp( xUIElement, UNO_QUERY );
            if ( xSubToolBar.is() && xProp.is() )
            {
                rtl::OUString aPersistentString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
                try
                {
                    Window*  pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                    if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        Any a;
                        a = xProp->getPropertyValue( aPersistentString );
                        xProp->setPropertyValue( aPersistentString, makeAny( sal_False ));

                        xLayoutManager->hideElement( aSubToolBarResName );
                        xLayoutManager->floatWindow( aSubToolBarResName );

                        xLayoutManager->setElementPos( aSubToolBarResName, aEvent.FloatingPosition );
                        xLayoutManager->showElement( aSubToolBarResName );

                        xProp->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" )), a );
                    }
                }
                catch ( ::com::sun::star::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( ::com::sun::star::uno::Exception& )
                {
                }
            }
        }
    }
}

::Size  SfxToolBoxControl::getPersistentFloatingSize( const Reference< XFrame >& /*xFrame*/, const ::rtl::OUString& /*rSubToolBarResName*/ )
{
    ::Size  aToolboxSize;
    return aToolboxSize;
}

void SfxToolBoxControl::createAndPositionSubToolBar( const ::rtl::OUString& rSubToolBarResName )
{
    SolarMutexGuard aGuard;

    if ( pImpl->pBox )
    {
        static WeakReference< XUIElementFactory > xWeakUIElementFactory;

        USHORT nItemId = pImpl->pBox->GetDownItemId();

        if ( !nItemId )
            return;

        // create element with factory
        Reference< XMultiServiceFactory >   xServiceManager = getServiceManager();
        Reference< XFrame >                 xFrame          = getFrameInterface();
        Reference< XUIElement >             xUIElement;
        Reference< XUIElementFactory >      xUIEementFactory;

        xUIEementFactory = xWeakUIElementFactory;
        if ( !xUIEementFactory.is() )
        {
            xUIEementFactory = Reference< XUIElementFactory >(
                xServiceManager->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.ui.UIElementFactoryManager" ))),
                UNO_QUERY );
            xWeakUIElementFactory = xUIEementFactory;
        }

        Sequence< PropertyValue > aPropSeq( 3 );
        aPropSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
        aPropSeq[0].Value <<= xFrame;
        aPropSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
        aPropSeq[1].Value <<= sal_False;
        aPropSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PopupMode" ));
        aPropSeq[2].Value <<= sal_True;

        try
        {
            xUIElement = xUIEementFactory->createUIElement( rSubToolBarResName, aPropSeq );
        }
        catch ( ::com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( IllegalArgumentException& )
        {
        }

        if ( xUIElement.is() )
        {
            Reference< ::com::sun::star::awt::XWindow > xParent = getFrameInterface()->getContainerWindow();

            Reference< ::com::sun::star::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xSubToolBar.is() )
            {
                Reference< ::com::sun::star::awt::XDockableWindow > xDockWindow( xSubToolBar, UNO_QUERY );
                xDockWindow->addDockableWindowListener( Reference< ::com::sun::star::awt::XDockableWindowListener >(
                    static_cast< OWeakObject * >( this ), UNO_QUERY ));
                xDockWindow->enableDocking( sal_True );

                // keep refererence to UIElement to avoid its destruction
                if ( pImpl->mxUIElement.is() )
                {
                    Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
                    xComponent->dispose();
                }
                pImpl->mxUIElement = xUIElement;

                Window*  pParentTbxWindow( pImpl->pBox );
                Window*  pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                ToolBox* pToolBar( 0 );
                if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBar = (ToolBox *)pTbxWindow;

                if ( pToolBar )
                {
                    pToolBar->SetParent( pParentTbxWindow );
                    ::Size aSize = getPersistentFloatingSize( xFrame, rSubToolBarResName );
                    if ( aSize.Width() == 0 || aSize.Height() == 0 )
                    {
                        // calc and set size for popup mode
                        aSize = pToolBar->CalcPopupWindowSizePixel();
                    }
                    pToolBar->SetSizePixel( aSize );

                    // open subtoolbox in popup mode
                    Window::GetDockingManager()->StartPopupMode( pImpl->pBox, pToolBar );
                }
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxToolBoxControl::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pImpl->mpPopupWindow = pWindow;
    pImpl->mpPopupWindow->SetPopupModeEndHdl( LINK( this, SfxToolBoxControl, PopupModeEndHdl ));
    pImpl->mpPopupWindow->SetDeleteLink_Impl( LINK( this, SfxToolBoxControl, ClosePopupWindow ));
}

//--------------------------------------------------------------------

IMPL_LINK( SfxToolBoxControl, PopupModeEndHdl, void *, EMPTYARG )
{
    if ( pImpl->mpPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        delete pImpl->mpFloatingWindow;
        pImpl->mpFloatingWindow = pImpl->mpPopupWindow;
        pImpl->mpPopupWindow    = 0;
        // We also need to know when the user tries to use the
        // floating window.
        pImpl->mpFloatingWindow->AddEventListener( LINK( pImpl, SfxToolBoxControl_Impl, WindowEventListener ));
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pImpl->mpPopupWindow = 0;
    }

    return 1;
}

//--------------------------------------------------------------------
IMPL_LINK( SfxToolBoxControl, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pImpl->mpFloatingWindow )
        pImpl->mpFloatingWindow = 0;
    else
        pImpl->mpPopupWindow = 0;

    return 1;
}

//--------------------------------------------------------------------

void SfxToolBoxControl::StateChanged
(
    USHORT              nId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_MEMTEST();
    DBG_ASSERT( pImpl->pBox != 0, "setting state to dangling ToolBox" );

    if ( GetId() >= SID_OBJECTMENU0 && GetId() <= SID_OBJECTMENU_LAST )
        return;

    // enabled/disabled-Flag pauschal korrigieren
    pImpl->pBox->EnableItem( GetId(), eState != SFX_ITEM_DISABLED );

    USHORT nItemBits = pImpl->pBox->GetItemBits( GetId() );
    nItemBits &= ~TIB_CHECKABLE;
    TriState eTri = STATE_NOCHECK;
    switch ( eState )
    {
        case SFX_ITEM_AVAILABLE:
        {
            if ( pState->ISA(SfxBoolItem) )
            {
                // BoolItem fuer checken
                if ( ((const SfxBoolItem*)pState)->GetValue() )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else if ( pState->ISA(SfxEnumItemInterface) &&
                ((SfxEnumItemInterface *)pState)->HasBoolValue())
            {
                // EnumItem wie Bool behandeln
                if ( ((const SfxEnumItemInterface *)pState)->GetBoolValue() )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else if ( pImpl->bShowString && pState->ISA(SfxStringItem) )
                pImpl->pBox->SetItemText(nId, ((const SfxStringItem*)pState)->GetValue() );
            break;
        }

        case SFX_ITEM_DONTCARE:
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
        }
    }

    pImpl->pBox->SetItemState( GetId(), eTri );
    pImpl->pBox->SetItemBits( GetId(), nItemBits );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Select( USHORT nModifier )
{
    pImpl->nSelectModifier = nModifier;
    Select( BOOL((nModifier & KEY_MOD1)!=0) );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Select( BOOL /*bMod1*/ )
{
    if(::comphelper::UiEventsLogger::isEnabled()) //#i88653# #i102805#
    {
        ::rtl::OUString sAppName;
        try
        {
            static ::rtl::OUString our_aModuleManagerName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ModuleManager"));
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager =
                ::comphelper::getProcessServiceFactory();
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager(
                xServiceManager->createInstance(our_aModuleManagerName)
                , ::com::sun::star::uno::UNO_QUERY_THROW);
            sAppName = xModuleManager->identify(m_xFrame);
        } catch(::com::sun::star::uno::Exception&) {}
        Sequence<PropertyValue> vSource;
        ::comphelper::UiEventsLogger::appendDispatchOrigin(vSource, sAppName, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SfxToolBoxControl")));
        URL aURL;
        aURL.Complete = m_aCommandURL;
        ::comphelper::UiEventsLogger::logDispatch(aURL, vSource);
    }
    svt::ToolboxController::execute( pImpl->nSelectModifier );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::DoubleClick()
{
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Click()
{
}

//--------------------------------------------------------------------

SfxPopupWindowType SfxToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_NONE;
}

//--------------------------------------------------------------------

SfxPopupWindow* SfxToolBoxControl::CreatePopupWindow()
{
    return 0;
}

SfxPopupWindow* SfxToolBoxControl::CreatePopupWindowCascading()
{
    return 0;
}

//--------------------------------------------------------------------

Window* SfxToolBoxControl::CreateItemWindow( Window * )
{
    return 0;
}

//--------------------------------------------------------------------

SfxFrameStatusListener::SfxFrameStatusListener(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >& xFrame,
    SfxStatusListenerInterface* pCallee ) :
    svt::FrameStatusListener( rServiceManager, xFrame ),
    m_pCallee( pCallee )
{
}

//--------------------------------------------------------------------

SfxFrameStatusListener::~SfxFrameStatusListener()
{
}

//--------------------------------------------------------------------

// XStatusListener
void SAL_CALL SfxFrameStatusListener::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxViewFrame* pViewFrame = NULL;
    Reference < XController > xController;

    SolarMutexGuard aGuard;
    if ( m_xFrame.is() )
        xController = m_xFrame->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = reinterpret_cast< SfxOfficeDispatch* >( sal::static_int_cast< sal_IntPtr >( nImplementation ));
            }

            if ( pDisp )
                pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
        }
    }

    USHORT nSlotId = 0;
    SfxSlotPool& rPool = SfxSlotPool::GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetUnoSlot( rEvent.FeatureURL.Path );
    if ( pSlot )
        nSlotId = pSlot->GetSlotId();

    if ( nSlotId > 0 )
    {
        if ( rEvent.Requery )
        {
            // requery for the notified state
            addStatusListener( rEvent.FeatureURL.Complete );
        }
        else
        {
            SfxItemState eState = SFX_ITEM_DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SFX_ITEM_AVAILABLE;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == ::getVoidCppuType() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SFX_ITEM_UNKNOWN;
                }
                else if ( pType == ::getBooleanCppuType() )
                {
                    sal_Bool bTemp = false;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    eState = aItemStatus.State;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::Visibility*)0) )
                {
                    Visibility aVisibilityStatus;
                    rEvent.State >>= aVisibilityStatus;
                    pItem = new SfxVisibilityItem( nSlotId, aVisibilityStatus.bVisible );
                }
                else
                {
                    if ( pSlot )
                        pItem = pSlot->GetType()->CreateItem();
                    if ( pItem )
                    {
                        pItem->SetWhich( nSlotId );
                        pItem->PutValue( rEvent.State );
                    }
                    else
                        pItem = new SfxVoidItem( nSlotId );
                }
            }

            if ( m_pCallee )
                m_pCallee->StateChanged( nSlotId, eState, pItem );
            delete pItem;
        }
    }
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), nBits )
    , m_bFloating(FALSE)
    , m_bCascading( FALSE )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const Reference< XFrame >& rFrame,
    const ResId &rId ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), rId )
    , m_bFloating(FALSE)
    , m_bCascading( FALSE )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const Reference< XFrame >& rFrame,
    Window* pParentWindow,
    WinBits nBits ) :
    FloatingWindow( pParentWindow, nBits )
    , m_bFloating(FALSE)
    , m_bCascading( FALSE )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    Window* pParentWindow,
    const ResId &rId ) :
    FloatingWindow( pParentWindow, rId )
    , m_bFloating(FALSE)
    , m_bCascading( FALSE )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::~SfxPopupWindow()
{
    if ( m_xStatusListener.is() )
    {
        m_xStatusListener->dispose();
        m_xStatusListener.clear();
    }

    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( this );
}

//--------------------------------------------------------------------

SfxFrameStatusListener* SfxPopupWindow::GetOrCreateStatusListener()
{
    if ( !m_xStatusListener.is() )
    {
        m_pStatusListener = new SfxFrameStatusListener(
                                    m_xServiceManager,
                                    m_xFrame,
                                    this );
        m_xStatusListener = Reference< XComponent >( static_cast< cppu::OWeakObject* >(
                                                        m_pStatusListener ), UNO_QUERY );
    }

    return m_pStatusListener;
}

//--------------------------------------------------------------------

void SfxPopupWindow::BindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->bindListener();
}

//--------------------------------------------------------------------

void SfxPopupWindow::UnbindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->unbindListener();
}

//--------------------------------------------------------------------

void SfxPopupWindow::AddStatusListener( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->addStatusListener( rCommandURL );
}

//--------------------------------------------------------------------

void SfxPopupWindow::RemoveStatusListener( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->removeStatusListener( rCommandURL );
}

//--------------------------------------------------------------------

void SfxPopupWindow::UpdateStatus( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->updateStatus( rCommandURL );
}

//--------------------------------------------------------------------

BOOL SfxPopupWindow::Close()
{
    m_bFloating = FALSE;
    FloatingWindow::Close();

    Delete(0);
    return TRUE;
}

//--------------------------------------------------------------------

void SfxPopupWindow::PopupModeEnd()
{
    //! to allow PopupModeEndHdl to be called
    FloatingWindow::PopupModeEnd();

    if ( IsVisible() )
    {
        // wurde abgerissen
        DeleteFloatingWindow();
        m_bFloating = TRUE;
    }
    else
        Close();
}

//--------------------------------------------------------------------

void SfxPopupWindow::DeleteFloatingWindow()
{
    if ( m_bFloating )
    {
        Hide();
        Delete(0);
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::MouseMove( const ::MouseEvent& rMEvt )
{
    if ( m_bCascading == FALSE )
        FloatingWindow::MouseMove( rMEvt );
    else
    {
        // MouseMove-Event an die Children forwarden
        ::Point       aPos = rMEvt.GetPosPixel();
        ::Point       aScrPos = OutputToScreenPixel( aPos );
        USHORT i = 0;
        Window* pWindow = GetChild( i );
        while ( pWindow )
        {
            ::MouseEvent aMEvt( pWindow->ScreenToOutputPixel( aScrPos ),
                              rMEvt.GetClicks(), rMEvt.GetMode(),
                              rMEvt.GetButtons(), rMEvt.GetModifier() );
            pWindow->MouseMove( rMEvt );
            pWindow->Update();
            i++;
            pWindow = GetChild( i );
        }
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::StartCascading()
{
    m_bCascading= TRUE;
}

void SfxPopupWindow::EndCascading()
{
    m_bCascading = FALSE;
}

//--------------------------------------------------------------------

SfxPopupWindow* SfxPopupWindow::Clone() const

/*  [Beschreibung]

    Diese Methode mu\s "uberladen werden, um dieses Popup auch im
    Presentations-Modus anzuzeigen. Sie wird gerufen, wenn ein Show()
    sinnlos w"are, da der Parent nicht das Presentations-Window ist.
    Beim neu erzeugen wird automatisch das neue Top-Window verwendet, so
    da\s der Parent das Presentations-Window ist und das neue Popup somit
    sichtbar ist.
*/

{
    return 0;
}

//--------------------------------------------------------------------

void SfxPopupWindow::StateChanged(
    USHORT /*nSID*/,
    SfxItemState eState,
    const SfxPoolItem* /*pState*/ )
/*  [Bescheibung]

    Siehe auch <SfxControllerItem::StateChanged()>. Au\serdem wird
    bei eState==SFX_ITEM_DISABLED das Popup gehided und in allen anderen
    F"allen, falls es floating ist, wieder angezeigt. Daher mu\s die
    Basisklasse i.d.R. gerufen werden.

    Es findet wegen des Parents eine Sonderbehandlung f"ur den
    Presentationsmodus statt.

*/

{
    if ( SFX_ITEM_DISABLED == eState )
    {
        Hide();
    }
    else if ( m_bFloating )
    {
        Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    }
}

//--------------------------------------------------------------------

IMPL_LINK( SfxPopupWindow, Delete, void *, EMPTYARG )
{
    if ( m_aDeleteLink.IsSet() )
        m_aDeleteLink.Call( this );
    delete this;
    return 0;
}

//--------------------------------------------------------------------

SfxAppToolBoxControl_Impl::SfxAppToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
    , bBigImages( FALSE )
    , pMenu( 0 )
{
    rBox.SetHelpId( nId, HID_TBXCONTROL_FILENEW );
    rBox.SetItemBits( nId,  rBox.GetItemBits( nId ) | TIB_DROPDOWN);

    // Determine the current background color of the menus
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_nSymbolsStyle         = rSettings.GetSymbolsStyle();
    m_bShowMenuImages       = rSettings.GetUseImagesInMenus();

    SetImage( String() );
}

SfxAppToolBoxControl_Impl::~SfxAppToolBoxControl_Impl()
{
    delete pMenu;
}

//_____________________________________________________
/*
    it return the existing state of the given URL in the popupmenu of this toolbox control.

    If the given URL can be located as an action command of one menu item of the
    popup menu of this control, we return TRUE. Otherwhise we return FALSE.
    Further we return a fallback URL, in case we have to return FALSE. Because
    the outside code must select a valid item of the popup menu everytime ...
    and we define it here. By the way this m ethod was written to handle
    error situations gracefully. E.g. it can be called during creation time
    but then we have no valid menu. For this case we know another fallback URL.
    Then we return the private:factory/ URL of the default factory.

    @param  *pMenu
                pounts to the popup menu, on which item we try to locate the given URL
                Can be NULL! Search will be supressed then.

    @param  sURL
                the URL for searching

    @param  pFallback
                contains the fallback URL in case we return FALSE
                Must point to valid memory!

    @param  pImage
                contains the image of the menu for the URL.

    @return TRUE - if URL could be located as an item of the popup menu.
            FALSE - otherwhise.
*/
BOOL Impl_ExistURLInMenu( const PopupMenu *pMenu     ,
                                String    &sURL      ,
                                String    *pFallback ,
                                Image     *pImage    )
{
    BOOL bValidFallback = FALSE;
    if (pMenu && sURL.Len())
    {
        USHORT c = pMenu->GetItemCount();
        for (USHORT p=0; p<c; ++p)
        {
            USHORT nId = pMenu->GetItemId(p);
            String aCmd( pMenu->GetItemCommand(nId) );

            if (!bValidFallback && aCmd.Len())
            {
                *pFallback = aCmd;
                bValidFallback = TRUE;
            }

            if (aCmd.Search(sURL)==0)//match even if the menu command is more detailed (maybe an additional query) #i28667#
            {
                sURL = aCmd;
                *pImage = pMenu->GetItemImage( nId );
                return TRUE;
            }
        }
    }

    if (!bValidFallback)
    {
        *pFallback  = DEFINE_CONST_UNICODE("private:factory/");
        *pFallback += String(SvtModuleOptions().GetDefaultModuleName());
    }

    return FALSE;
}

long Select_Impl( void* pHdl, void* pVoid );

SfxPopupWindow* SfxAppToolBoxControl_Impl::CreatePopupWindow()
{
    ToolBox& rBox = GetToolBox();
    ::Rectangle aRect( rBox.GetItemRect( GetId() ) );

    if ( !pMenu )
    {
        ::framework::MenuConfiguration aConf( m_xServiceManager );
        if ( m_aCommandURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( ".uno:AddDirect" ) ))
            pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
        else
            pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );
    }

    if ( pMenu )
    {
        pMenu->SetSelectHdl( Link( NULL, Select_Impl ) );
        pMenu->SetActivateHdl( LINK( this, SfxAppToolBoxControl_Impl, Activate ));
        rBox.SetItemDown( GetId(), TRUE );
        USHORT nSelected = pMenu->Execute( &rBox, aRect, POPUPMENU_EXECUTE_DOWN );
        if ( nSelected )
        {
            aLastURL = pMenu->GetItemCommand( nSelected );
            SetImage( pMenu->GetItemCommand( nSelected ) );
        }

        rBox.SetItemDown( GetId(), FALSE );
    }

    return 0;
}

void SfxAppToolBoxControl_Impl::SetImage( const String &rURL )
{
    /* We accept URL's here only, which exist as items of our internal popup menu.
       All other ones will be ignored and a fallback is used ... */
    String aURL = rURL;
    String sFallback;
    Image aMenuImage;
    BOOL bValid = Impl_ExistURLInMenu(pMenu,aURL,&sFallback,&aMenuImage);
    if (!bValid)
        aURL = sFallback;

    BOOL bBig = SvtMiscOptions().AreCurrentSymbolsLarge();
    Image aImage = SvFileInformationManager::GetImageNoDefault( INetURLObject( aURL ), bBig );
    if ( !aImage )
        aImage = !!aMenuImage ? aMenuImage :
            SvFileInformationManager::GetImage( INetURLObject( aURL ), bBig );
    Size aBigSize( GetToolBox().GetDefaultImageSize() );
    if ( bBig && aImage.GetSizePixel() != aBigSize )
    {
        BitmapEx aScaleBmpEx( aImage.GetBitmapEx() );
        aScaleBmpEx.Scale( aBigSize, BMP_SCALE_INTERPOLATE );
        GetToolBox().SetItemImage( GetId(), Image( aScaleBmpEx ) );
    }
    else
        GetToolBox().SetItemImage( GetId(), aImage );
    aLastURL = aURL;
}

void SfxAppToolBoxControl_Impl::StateChanged
(
    USHORT              nSlotId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( pState && pState->ISA(SfxStringItem) )
    {
        // Important step for following SetImage() call!
        // It needs the valid pMenu item to fullfill it's specification
        // to check for supported URLs ...
        if ( !pMenu )
        {
            ::framework::MenuConfiguration aConf( m_xServiceManager );
            // This toolbox controller is used for two popup menus (new documents and wizards!). Create the correct
            // popup menu according to the slot ID our controller has been initialized with.
            if ( nSlotId == SID_NEWDOCDIRECT )
                pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
            else
                pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );
        }

        GetToolBox().EnableItem( GetId(), eState != SFX_ITEM_DISABLED );
        SetImage(((const SfxStringItem*)pState)->GetValue());
    }
    else
        SfxToolBoxControl::StateChanged( nSlotId, eState, pState );
}

//--------------------------------------------------------------------

void SfxAppToolBoxControl_Impl::Select( BOOL bMod1 )
{
    if( aLastURL.Len() )
    {
        URL                            aTargetURL;
        Reference< XDispatch >         xDispatch;
        Reference< XDispatchProvider > xDispatchProvider( getFrameInterface(), UNO_QUERY );

        if ( xDispatchProvider.is() )
        {
            aTargetURL.Complete = aLastURL;
            getURLTransformer()->parseStrict( aTargetURL );

            ::rtl::OUString aTarget( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default")));
            if ( pMenu )
            {
                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

                if ( pMenuAttributes )
                    aTarget = pMenuAttributes->aTargetFrame;
            }

            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTarget, 0 );

            if ( xDispatch.is() )
            {
                Sequence< PropertyValue > aArgs( 1 );

                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer"));
                aArgs[0].Value = makeAny( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER )) );

                ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                pExecuteInfo->xDispatch     = xDispatch;
                pExecuteInfo->aTargetURL    = aTargetURL;
                pExecuteInfo->aArgs         = aArgs;
                Application::PostUserEvent( STATIC_LINK(0, SfxAppToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
            }
        }
    }
    else
        SfxToolBoxControl::Select( bMod1 );
}

//--------------------------------------------------------------------
long Select_Impl( void* /*pHdl*/, void* pVoid )
{
    Menu* pMenu = (Menu*)pVoid;
    String aURL( pMenu->GetItemCommand( pMenu->GetCurItemId() ) );

    if( !aURL.Len() )
        return 0;

    Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop =
            Reference < ::com::sun::star::frame::XFramesSupplier >( ::comphelper::getProcessServiceFactory()->createInstance(
                                                                        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop, UNO_QUERY );

    URL aTargetURL;
    aTargetURL.Complete = aURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
                                          UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < XDispatch > xDisp;
    if ( xProv.is() )
    {
        if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
            xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        else
        {
            ::rtl::OUString aTargetFrame( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")) );
            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

            if ( pMenuAttributes )
                aTargetFrame = pMenuAttributes->aTargetFrame;

            xDisp = xProv->queryDispatch( aTargetURL, aTargetFrame , 0 );
        }
    }

    if ( xDisp.is() )
    {
        SfxAppToolBoxControl_Impl::ExecuteInfo* pExecuteInfo = new SfxAppToolBoxControl_Impl::ExecuteInfo;
        pExecuteInfo->xDispatch     = xDisp;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = Sequence< PropertyValue >();
        Application::PostUserEvent( STATIC_LINK( 0, SfxAppToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
    }

    return TRUE;
}

IMPL_LINK( SfxAppToolBoxControl_Impl, Activate, Menu *, pActMenu )
{
    if ( pActMenu )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        ULONG nSymbolsStyle     = rSettings.GetSymbolsStyle();
        BOOL bShowMenuImages    = rSettings.GetUseImagesInMenus();

        if (( nSymbolsStyle != m_nSymbolsStyle ) ||
            ( bShowMenuImages != m_bShowMenuImages ))
        {
            m_nSymbolsStyle      = nSymbolsStyle;
            m_bShowMenuImages    = bShowMenuImages;

            USHORT nCount = pActMenu->GetItemCount();
            for ( USHORT nSVPos = 0; nSVPos < nCount; nSVPos++ )
            {
                USHORT nId = pActMenu->GetItemId( nSVPos );
                if ( pActMenu->GetItemType( nSVPos ) != MENUITEM_SEPARATOR )
                {
                    if ( bShowMenuImages )
                    {
                        sal_Bool        bImageSet = sal_False;
                        ::rtl::OUString aImageId;
                        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nId );

                        if ( pMenuAttributes )
                            aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                        if ( aImageId.getLength() > 0 )
                        {
                            Reference< ::com::sun::star::frame::XFrame > xFrame;
                            Image aImage = GetImage( xFrame, aImageId, FALSE );
                            if ( !!aImage )
                            {
                                bImageSet = sal_True;
                                pActMenu->SetItemImage( nId, aImage );
                            }
                        }

                        String aCmd( pActMenu->GetItemCommand( nId ) );
                        if ( !bImageSet && aCmd.Len() )
                        {
                            Image aImage = SvFileInformationManager::GetImage(
                                INetURLObject(aCmd), FALSE );
                            if ( !!aImage )
                                pActMenu->SetItemImage( nId, aImage );
                        }
                    }
                    else
                        pActMenu->SetItemImage( nId, Image() );
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( SfxAppToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
/*  i62706: Don't catch all exceptions. We hide all problems here and are not able
            to handle them on higher levels.
    try
    {
*/
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
/*
}
    catch (const ::com::sun::star::document::CorruptedFilterConfigurationException& exFilters)
    {
        throw exFilters;
    }
    catch (const Exception& )
    {
    }
*/
    delete pExecuteInfo;
    return 0;
}

//--------------------------------------------------------------------

void SfxAppToolBoxControl_Impl::Click( )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
