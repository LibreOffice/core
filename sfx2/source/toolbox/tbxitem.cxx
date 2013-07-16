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


#ifdef SOLARIS
#include <ctime>
#endif

#include <string>           // prevent conflict with STL includes
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>
#include <com/sun/star/ui/UIElementFactoryManager.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
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
#include <sfx2/genlink.hxx>
#include "sfx2/sfxresid.hxx"
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
#include "sfx2/imgmgr.hxx"
#include "virtmenu.hxx"
#include "sfx2/imagemgr.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

//====================================================================

SFX_IMPL_TOOLBOX_CONTROL_ARG(SfxToolBoxControl, SfxStringItem, sal_True);
SFX_IMPL_TOOLBOX_CONTROL(SfxRecentFilesToolBoxControl, SfxStringItem);

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

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL )
{
    SolarMutexGuard aGuard;

    URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );
    if ( !aTargetURL.Arguments.isEmpty() )
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
        sal_uInt16 nSlotId = pSlot->GetSlotId();
        if ( nSlotId > 0 )
            return SfxToolBoxControl::CreateControl( nSlotId, nID, pToolbox, pModule );
    }

    return NULL;
}

struct SfxToolBoxControl_Impl
{
    ToolBox*                pBox;
    sal_Bool                    bShowString;
    sal_uInt16                  nSelectModifier;
    SfxTbxCtrlFactory*      pFact;
    sal_uInt16                  nTbxId;
    sal_uInt16                  nSlotId;
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
    sal_uInt16          nSlotID,
    sal_uInt16          nID,
    ToolBox&        rBox,
    sal_Bool            bShowStringItems     )
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
    // parent toolbar!
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

SfxToolBoxControl* SfxToolBoxControl::CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule* pMod  )
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
                sal_uInt16 nFactory;
                const sal_uInt16 nCount = rFactories.size();

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
        sal_uInt16 nFactory;
        const sal_uInt16 nCount = rFactories.size();

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
/*  [Description]

    Static method for determining the status of the SfxPoolItem-pointer,
    used in the method <SfxControllerItem::StateChanged(const SfxPoolItem*)>.

    [Return value]

    SfxItemState        SFX_ITEM_UNKNOWN
                        Enabled, however no further status information is available.
                        Typical for <Slot>s, which are temporarily disabled a
                        anyway but other than that do not change their appearance.

                        SFX_ITEM_DISABLED
                        Disabled, no further status information is available.
                        All other displayed values should be reset to the default
                        if possible.

                        SFX_ITEM_DONTCARE
                        Enabled but there were only ambiguous values available
                        (i.e. none that could be queried).

                        SFX_ITEM_AVAILABLE
                        Enabled and with available values which can be queried
                        through'pState'. The type is thus by the Slot clearly
                        defined in the entire Program.
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
    const OUString& rCommand,
    Sequence< ::PropertyValue >& aArgs )
{
    if ( rProvider.is() )
    {
        ::com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = rCommand;
        Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = rProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

void SfxToolBoxControl::Dispatch( const OUString& aCommand, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
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

        Reference < XDispatch > xDispatch = xProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

// XInterface
Any SAL_CALL SfxToolBoxControl::queryInterface( const Type & rType )
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                           (static_cast< ::com::sun::star::awt::XDockableWindowListener* >(this)),
                                        (static_cast< ::com::sun::star::frame::XSubToolbarController* >(this)));
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
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, OUString(), 0 );
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

    sal_uInt16 nSlotId = 0;
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
                else if ( pType == ::getCppuType((const OUString*)0) )
                {
                    OUString sTemp ;
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

OUString SAL_CALL SfxToolBoxControl::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException)
{
    return OUString();
}

void SAL_CALL SfxToolBoxControl::functionSelected( const OUString& /*aCommand*/ ) throw (::com::sun::star::uno::RuntimeException)
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
    Select( (sal_uInt16)KeyModifier );
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

    OUString aSubToolBarResName;
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XPropertySet > xPropSet( pImpl->mxUIElement, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                xPropSet->getPropertyValue("ResourceURL") >>= aSubToolBarResName;
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
                OUString aPersistentString( "Persistent" );
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

                        xProp->setPropertyValue("Persistent", a );
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

::Size  SfxToolBoxControl::getPersistentFloatingSize( const Reference< XFrame >& /*xFrame*/, const OUString& /*rSubToolBarResName*/ )
{
    ::Size  aToolboxSize;
    return aToolboxSize;
}

bool SfxToolBoxControl::hasBigImages() const
{
    return (GetToolBox().GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE);
}

void SfxToolBoxControl::createAndPositionSubToolBar( const OUString& rSubToolBarResName )
{
    SolarMutexGuard aGuard;

    if ( pImpl->pBox )
    {
        static WeakReference< XUIElementFactoryManager > xWeakUIElementFactory;

        sal_uInt16 nItemId = pImpl->pBox->GetDownItemId();

        if ( !nItemId )
            return;

        // create element with factory
        Reference< XFrame >                 xFrame          = getFrameInterface();
        Reference< XUIElement >             xUIElement;
        Reference< XUIElementFactoryManager >  xUIElementFactory;

        xUIElementFactory = xWeakUIElementFactory;
        if ( !xUIElementFactory.is() )
        {
            xUIElementFactory = UIElementFactoryManager::create( m_xContext );
            xWeakUIElementFactory = xUIElementFactory;
        }

        Sequence< PropertyValue > aPropSeq( 3 );
        aPropSeq[0].Name = OUString( "Frame" );
        aPropSeq[0].Value <<= xFrame;
        aPropSeq[1].Name = OUString( "Persistent" );
        aPropSeq[1].Value <<= sal_False;
        aPropSeq[2].Name = OUString( "PopupMode" );
        aPropSeq[2].Value <<= sal_True;

        try
        {
            xUIElement = xUIElementFactory->createUIElement( rSubToolBarResName, aPropSeq );
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

IMPL_LINK_NOARG(SfxToolBoxControl, PopupModeEndHdl)
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
    sal_uInt16              nId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_ASSERT( pImpl->pBox != 0, "setting state to dangling ToolBox" );

    if ( GetId() >= SID_OBJECTMENU0 && GetId() <= SID_OBJECTMENU_LAST )
        return;

    // enabled/disabled-Flag correcting the lump sum
    pImpl->pBox->EnableItem( GetId(), eState != SFX_ITEM_DISABLED );

    sal_uInt16 nItemBits = pImpl->pBox->GetItemBits( GetId() );
    nItemBits &= ~TIB_CHECKABLE;
    ::TriState eTri = STATE_NOCHECK;
    switch ( eState )
    {
        case SFX_ITEM_AVAILABLE:
        {
            if ( pState->ISA(SfxBoolItem) )
            {
                // BoolItem for checking
                if ( ((const SfxBoolItem*)pState)->GetValue() )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else if ( pState->ISA(SfxEnumItemInterface) &&
                ((SfxEnumItemInterface *)pState)->HasBoolValue())
            {
                // EnumItem is handled as Bool
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

void SfxToolBoxControl::Select( sal_uInt16 nModifier )
{
    pImpl->nSelectModifier = nModifier;
    Select( sal_Bool((nModifier & KEY_MOD1)!=0) );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Select( sal_Bool /*bMod1*/ )
{
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
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame,
    SfxStatusListenerInterface* pCallee ) :
    svt::FrameStatusListener( rxContext, xFrame ),
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
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, OUString(), 0 );
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

    sal_uInt16 nSlotId = 0;
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
                else if ( pType == ::getCppuType((const OUString*)0) )
                {
                    OUString sTemp ;
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
    sal_uInt16 nId,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), nBits )
    , m_bFloating(sal_False)
    , m_bCascading( sal_False )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    const Reference< XFrame >& rFrame,
    const ResId &rId ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), rId )
    , m_bFloating(sal_False)
    , m_bCascading( sal_False )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    const Reference< XFrame >& rFrame,
    Window* pParentWindow,
    WinBits nBits ) :
    FloatingWindow( pParentWindow, nBits )
    , m_bFloating(sal_False)
    , m_bCascading( sal_False )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
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
                                    ::comphelper::getProcessComponentContext(),
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

void SfxPopupWindow::AddStatusListener( const OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->addStatusListener( rCommandURL );
}

//--------------------------------------------------------------------

sal_Bool SfxPopupWindow::Close()
{
    m_bFloating = sal_False;
    FloatingWindow::Close();

    Delete();
    return sal_True;
}

//--------------------------------------------------------------------

void SfxPopupWindow::PopupModeEnd()
{
    //! to allow PopupModeEndHdl to be called
    FloatingWindow::PopupModeEnd();

    if ( IsVisible() )
    {
        // was teared-off
        DeleteFloatingWindow();
        m_bFloating = sal_True;
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
        Delete();
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::MouseMove( const ::MouseEvent& rMEvt )
{
    if ( m_bCascading == sal_False )
        FloatingWindow::MouseMove( rMEvt );
    else
    {
        // Forward MouseMove-Event to Children
        ::Point       aPos = rMEvt.GetPosPixel();
        ::Point       aScrPos = OutputToScreenPixel( aPos );
        sal_uInt16 i = 0;
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
    m_bCascading= sal_True;
}

//--------------------------------------------------------------------

SfxPopupWindow* SfxPopupWindow::Clone() const

/*  [Description]

    This method must be overloaded to show this Popup also in the
    Presentation-mode. It is called when a Show() would be meaningless
    since the parent is no presentation window.
    When create a new window the bew Top-Window will be used automatically,
    so that the Parent becomes the presentation window and and that the new
    Popup therefore becomes visible.
*/

{
    return 0;
}

//--------------------------------------------------------------------

void SfxPopupWindow::StateChanged(
    sal_uInt16 /*nSID*/,
    SfxItemState eState,
    const SfxPoolItem* /*pState*/ )
/*  [Description]

    See also <SfxControllerItem::StateChanged()>. In addition the Popup
    will become hidden when eState==SFX_ITEM_DISABLED and in all other
    cases it will be shown again if it is floating. In general this requires
    to call the Base class.

    Due to the parent the presentation mode is handled in a special way.
*/

{
    if ( SFX_ITEM_DISABLED == eState )
    {
        Hide();
    }
    else if ( m_bFloating )
    {
        Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::Delete()
{
    if ( m_aDeleteLink.IsSet() )
        m_aDeleteLink.Call( this );
    delete this;
}

//--------------------------------------------------------------------

SfxRecentFilesToolBoxControl::SfxRecentFilesToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
{
    rBox.SetItemBits( nId, rBox.GetItemBits( nId ) | TIB_DROPDOWN);
}

SfxRecentFilesToolBoxControl::~SfxRecentFilesToolBoxControl()
{
}

SfxPopupWindow* SfxRecentFilesToolBoxControl::CreatePopupWindow()
{
    ToolBox& rBox = GetToolBox();
    sal_uInt16 nItemId = GetId();
    ::Rectangle aRect( rBox.GetItemRect( nItemId ) );

    Sequence< Any > aArgs( 2 );
    PropertyValue aPropValue;

    aPropValue.Name = "CommandURL";
    aPropValue.Value <<= OUString( ".uno:RecentFileList" );
    aArgs[0] <<= aPropValue;

    aPropValue.Name = "Frame";
    aPropValue.Value <<= m_xFrame;
    aArgs[1] <<= aPropValue;

    uno::Reference< frame::XPopupMenuController > xPopupController( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.framework.RecentFilesMenuController", aArgs, m_xContext ), UNO_QUERY );

    uno::Reference< awt::XPopupMenu > xPopupMenu( m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.awt.PopupMenu", m_xContext ), uno::UNO_QUERY );

    if ( xPopupController.is() && xPopupMenu.is() )
    {
        xPopupController->setPopupMenu( xPopupMenu );

        rBox.SetItemDown( nItemId, sal_True );
        Reference< awt::XWindowPeer > xPeer( getParent(), uno::UNO_QUERY );

        if ( xPeer.is() )
            xPopupMenu->execute( xPeer, VCLUnoHelper::ConvertToAWTRect( aRect ), 0 );

        rBox.SetItemDown( nItemId, sal_False );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
