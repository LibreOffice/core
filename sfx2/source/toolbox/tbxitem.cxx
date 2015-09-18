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

#include <string>
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
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
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
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include "statcach.hxx"
#include <sfx2/viewfrm.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/module.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include "helpid.hrc"
#include "workwin.hxx"
#include <sfx2/imgmgr.hxx>
#include "virtmenu.hxx"
#include <sfx2/imagemgr.hxx>
#include <ctrlfactoryimpl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;



SFX_IMPL_TOOLBOX_CONTROL_ARG(SfxToolBoxControl, SfxStringItem, true);
SFX_IMPL_TOOLBOX_CONTROL(SfxRecentFilesToolBoxControl, SfxStringItem);
SFX_IMPL_TOOLBOX_CONTROL(SfxSaveAsToolBoxControl, SfxStringItem);

static vcl::Window* GetTopMostParentSystemWindow( vcl::Window* pWindow )
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
                pTopMostSysWin = static_cast<SystemWindow*>(pWindow);
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
    VclPtr<ToolBox>         pBox;
    bool                    bShowString;
    SfxTbxCtrlFactory*      pFact;
    sal_uInt16              nTbxId;
    sal_uInt16              nSlotId;
    VclPtr<SfxPopupWindow>  mpFloatingWindow;
    VclPtr<SfxPopupWindow>  mpPopupWindow;
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );
};

IMPL_LINK_TYPED( SfxToolBoxControl_Impl, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if ( ( rEvent.GetId() == VCLEVENT_WINDOW_MOVE ) ||
         ( rEvent.GetId() == VCLEVENT_WINDOW_ACTIVATE ))
    {
        vcl::Window* pWindow( rEvent.GetWindow() );
        if (( pWindow == mpFloatingWindow ) &&
            ( mpPopupWindow != nullptr ))
        {
            mpPopupWindow.disposeAndClear();
        }
    }
}



SfxToolBoxControl::SfxToolBoxControl(
    sal_uInt16      nSlotID,
    sal_uInt16      nID,
    ToolBox&        rBox,
    bool            bShowStringItems     )
{
    pImpl = new SfxToolBoxControl_Impl;

    pImpl->pBox = &rBox;
    pImpl->bShowString = bShowStringItems;
    pImpl->pFact = 0;
    pImpl->nTbxId = nID;
    pImpl->nSlotId = nSlotID;
    pImpl->mpFloatingWindow = 0;
    pImpl->mpPopupWindow = 0;
}



SfxToolBoxControl::~SfxToolBoxControl()
{
    delete pImpl;
}



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



void SAL_CALL SfxToolBoxControl::dispose() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( m_bDisposed )
        return;

    svt::ToolboxController::dispose();

    // Remove and destroy our item window at our toolbox
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = pImpl->pBox->GetItemWindow( pImpl->nTbxId );
    pImpl->pBox->SetItemWindow( pImpl->nTbxId, 0 );
    pWindow.disposeAndClear();

    // Delete my popup windows
    pImpl->mpFloatingWindow.disposeAndClear();
    pImpl->mpPopupWindow.disposeAndClear();
}


void SfxToolBoxControl::RegisterToolBoxControl( SfxModule* pMod, SfxTbxCtrlFactory* pFact)
{
    SfxGetpApp()->RegisterToolBoxControl_Impl( pMod, pFact );
}

SfxToolBoxControl* SfxToolBoxControl::CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule* pMod  )
{
    SolarMutexGuard aGuard;

    SfxApplication *pApp = SfxGetpApp();

    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &SfxSlotPool::GetSlotPool();
    TypeId aSlotType = pSlotPool->GetSlotType( nSlotId );
    if ( aSlotType )
    {
        SfxToolBoxControl *pCtrl;
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
                    if( (rFactories[nFactory].nTypeId == aSlotType) && (rFactories[nFactory].nSlotId == nSlotId) )
                        break;

                if( nFactory == nCount )
                {
                    // if no factory exists for the given slot id, see if we
                    // have a generic factory with the correct slot type and slot id == 0
                    for ( nFactory = 0; nFactory < nCount; ++nFactory )
                        if( (rFactories[nFactory].nTypeId == aSlotType) && (rFactories[nFactory].nSlotId == 0) )
                            break;
                }

                if( nFactory < nCount )
                {
                    pCtrl = rFactories[nFactory].pCtor( nSlotId, nTbxId, *pBox );
                    pCtrl->pImpl->pFact = &rFactories[nFactory];
                    return pCtrl;
                }
            }
        }

        SfxTbxCtrlFactArr_Impl &rFactories = pApp->GetTbxCtrlFactories_Impl();
        sal_uInt16 nFactory;
        const sal_uInt16 nCount = rFactories.size();

        for( nFactory = 0; nFactory < nCount; ++nFactory )
            if( (rFactories[nFactory].nTypeId == aSlotType) && (rFactories[nFactory].nSlotId == nSlotId) )
                break;

        if( nFactory == nCount )
        {
            // if no factory exists for the given slot id, see if we
            // have a generic factory with the correct slot type and slot id == 0
            for( nFactory = 0; nFactory < nCount; ++nFactory )
                if( (rFactories[nFactory].nTypeId == aSlotType) && (rFactories[nFactory].nSlotId == 0) )
                    break;
        }

        if( nFactory < nCount )
        {
            pCtrl = rFactories[nFactory].pCtor( nSlotId, nTbxId, *pBox );
            pCtrl->pImpl->pFact = &rFactories[nFactory];
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

    SfxItemState        SfxItemState::UNKNOWN
                        Enabled, however no further status information is available.
                        Typical for <Slot>s, which are temporarily disabled a
                        anyway but other than that do not change their appearance.

                        SfxItemState::DISABLED
                        Disabled, no further status information is available.
                        All other displayed values should be reset to the default
                        if possible.

                        SfxItemState::DONTCARE
                        Enabled but there were only ambiguous values available
                        (i.e. none that could be queried).

                        SfxItemState::DEFAULT
                        Enabled and with available values which can be queried
                        through'pState'. The type is thus by the Slot clearly
                        defined in the entire Program.
*/

{
    return !pState
                ? SfxItemState::DISABLED
                : IsInvalidItem(pState)
                    ? SfxItemState::DONTCARE
                    : pState->ISA(SfxVoidItem) && !pState->Which()
                        ? SfxItemState::UNKNOWN
                        : SfxItemState::DEFAULT;
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

void SAL_CALL SfxToolBoxControl::disposing( const ::com::sun::star::lang::EventObject& aEvent )
throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::disposing( aEvent );
}

// XStatusListener
void SAL_CALL SfxToolBoxControl::statusChanged( const FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException, std::exception )
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
            SfxItemState eState = SfxItemState::DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SfxItemState::DEFAULT;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == cppu::UnoType<void>::get() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SfxItemState::UNKNOWN;
                }
                else if ( pType == cppu::UnoType<bool>::get() )
                {
                    bool bTemp = false;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get())
                {
                    sal_uInt16 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == cppu::UnoType<sal_uInt32>::get() )
                {
                    sal_uInt32 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == cppu::UnoType<OUString>::get() )
                {
                    OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == cppu::UnoType< ::com::sun::star::frame::status::ItemStatus>::get() )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    SfxItemState tmpState = (SfxItemState) aItemStatus.State;
                    // make sure no-one tries to send us a combination of states
                    if (tmpState != SfxItemState::UNKNOWN && tmpState != SfxItemState::DISABLED &&
                        tmpState != SfxItemState::READONLY && tmpState != SfxItemState::DONTCARE &&
                        tmpState != SfxItemState::DEFAULT && tmpState != SfxItemState::SET)
                        throw ::com::sun::star::uno::RuntimeException("unknown status");
                    eState = tmpState;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else if ( pType == cppu::UnoType< ::com::sun::star::frame::status::Visibility>::get() )
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
                        pItem->PutValue( rEvent.State, 0 );
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
sal_Bool SAL_CALL SfxToolBoxControl::opensSubToolbar() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return sal_False;
}

OUString SAL_CALL SfxToolBoxControl::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString();
}

void SAL_CALL SfxToolBoxControl::functionSelected( const OUString& /*aCommand*/ ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    // must be implemented by sub-class
}

void SAL_CALL SfxToolBoxControl::updateImage() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    // must be implemented by sub-class
}

// XToolbarController
void SAL_CALL SfxToolBoxControl::execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Select( (sal_uInt16)KeyModifier );
}

void SAL_CALL SfxToolBoxControl::click() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Click();
}

void SAL_CALL SfxToolBoxControl::doubleClick() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    DoubleClick();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createPopupWindow() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    VclPtr<vcl::Window> pWindow = CreatePopupWindow();
    if ( pWindow )
        return VCLUnoHelper::GetInterface( pWindow );
    else
        return Reference< ::com::sun::star::awt::XWindow >();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createItemWindow( const Reference< ::com::sun::star::awt::XWindow >& rParent ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return VCLUnoHelper::GetInterface( CreateItemWindow( VCLUnoHelper::GetWindow( rParent )));
}

bool SfxToolBoxControl::hasBigImages() const
{
    return (GetToolBox().GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE);
}

void SfxToolBoxControl::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pImpl->mpPopupWindow = pWindow;
    pImpl->mpPopupWindow->SetPopupModeEndHdl( LINK( this, SfxToolBoxControl, PopupModeEndHdl ));
    pImpl->mpPopupWindow->SetDeleteLink_Impl( LINK( this, SfxToolBoxControl, ClosePopupWindow ));
}



IMPL_LINK_NOARG_TYPED(SfxToolBoxControl, PopupModeEndHdl, FloatingWindow*, void)
{
    if ( pImpl->mpPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        pImpl->mpFloatingWindow.disposeAndClear();
        pImpl->mpFloatingWindow = pImpl->mpPopupWindow;
        pImpl->mpPopupWindow.clear();
        // We also need to know when the user tries to use the
        // floating window.
        pImpl->mpFloatingWindow->AddEventListener( LINK( pImpl, SfxToolBoxControl_Impl, WindowEventListener ));
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pImpl->mpPopupWindow.clear();
    }
}


IMPL_LINK_TYPED( SfxToolBoxControl, ClosePopupWindow, SfxPopupWindow *, pWindow, void )
{
    if ( pWindow == pImpl->mpFloatingWindow )
        pImpl->mpFloatingWindow = 0;
    else
        pImpl->mpPopupWindow = 0;
}



void SfxToolBoxControl::StateChanged
(
    sal_uInt16              nId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_ASSERT( pImpl->pBox != nullptr, "setting state to dangling ToolBox" );

    if ( GetId() >= SID_OBJECTMENU0 && GetId() <= SID_OBJECTMENU_LAST )
        return;

    // enabled/disabled-Flag correcting the lump sum
    pImpl->pBox->EnableItem( GetId(), eState != SfxItemState::DISABLED );

    ToolBoxItemBits nItemBits = pImpl->pBox->GetItemBits( GetId() );
    nItemBits &= ~ToolBoxItemBits::CHECKABLE;
    ::TriState eTri = TRISTATE_FALSE;
    switch ( eState )
    {
        case SfxItemState::DEFAULT:
        if ( pState )
        {
            if ( pState->ISA(SfxBoolItem) )
            {
                // BoolItem for checking
                if ( static_cast<const SfxBoolItem*>(pState)->GetValue() )
                    eTri = TRISTATE_TRUE;
                nItemBits |= ToolBoxItemBits::CHECKABLE;
            }
            else if ( pState->ISA(SfxEnumItemInterface) &&
                static_cast<const SfxEnumItemInterface *>(pState)->HasBoolValue())
            {
                // EnumItem is handled as Bool
                if ( static_cast<const SfxEnumItemInterface *>(pState)->GetBoolValue() )
                    eTri = TRISTATE_TRUE;
                nItemBits |= ToolBoxItemBits::CHECKABLE;
            }
            else if ( pImpl->bShowString && pState->ISA(SfxStringItem) )
                pImpl->pBox->SetItemText(nId, static_cast<const SfxStringItem*>(pState)->GetValue() );
        }
        break;

        case SfxItemState::DONTCARE:
        {
            eTri = TRISTATE_INDET;
            nItemBits |= ToolBoxItemBits::CHECKABLE;
        }
        break;

        default: break; // do nothing
    }

    pImpl->pBox->SetItemState( GetId(), eTri );
    pImpl->pBox->SetItemBits( GetId(), nItemBits );
}



void SfxToolBoxControl::Select( sal_uInt16 nSelectModifier )
{
    svt::ToolboxController::execute( nSelectModifier );
}



void SfxToolBoxControl::DoubleClick()
{
}



void SfxToolBoxControl::Click()
{
}



VclPtr<SfxPopupWindow> SfxToolBoxControl::CreatePopupWindow()
{
    return 0;
}

VclPtr<vcl::Window> SfxToolBoxControl::CreateItemWindow( vcl::Window * )
{
    return VclPtr<vcl::Window>();
}

SfxFrameStatusListener::SfxFrameStatusListener(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame,
    SfxStatusListenerInterface* pCallee ) :
    svt::FrameStatusListener( rxContext, xFrame ),
    m_pCallee( pCallee )
{
}



SfxFrameStatusListener::~SfxFrameStatusListener()
{
}



// XStatusListener
void SAL_CALL SfxFrameStatusListener::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException, std::exception )
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
            SfxItemState eState = SfxItemState::DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SfxItemState::DEFAULT;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == cppu::UnoType<void>::get() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SfxItemState::UNKNOWN;
                }
                else if ( pType == cppu::UnoType<bool>::get() )
                {
                    bool bTemp = false;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get())
                {
                    sal_uInt16 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == cppu::UnoType<sal_uInt32>::get() )
                {
                    sal_uInt32 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == cppu::UnoType<OUString>::get() )
                {
                    OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == cppu::UnoType< ::com::sun::star::frame::status::ItemStatus>::get() )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    SfxItemState tmpState = (SfxItemState) aItemStatus.State;
                    // make sure no-one tries to send us a combination of states
                    if (tmpState != SfxItemState::UNKNOWN && tmpState != SfxItemState::DISABLED &&
                        tmpState != SfxItemState::READONLY && tmpState != SfxItemState::DONTCARE &&
                        tmpState != SfxItemState::DEFAULT && tmpState != SfxItemState::SET)
                        throw ::com::sun::star::uno::RuntimeException("unknown status");
                    eState = tmpState;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else if ( pType == cppu::UnoType< ::com::sun::star::frame::status::Visibility>::get() )
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
                        pItem->PutValue( rEvent.State, 0 );
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

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    FloatingWindow( SfxGetpApp()->GetTopWindow(), nBits )
    , m_bFloating( false )
    , m_bCascading( false )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

SfxPopupWindow::SfxPopupWindow(sal_uInt16 nId, const OString& rID, const OUString& rUIXMLDescription,
                               const css::uno::Reference<css::frame::XFrame> &rFrame)
    : FloatingWindow(SfxGetpApp()->GetTopWindow(), rID, rUIXMLDescription, rFrame)
    , m_bFloating( false )
    , m_bCascading( false )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    const Reference< XFrame >& rFrame,
    vcl::Window* pParentWindow,
    WinBits nBits ) :
    FloatingWindow( pParentWindow, nBits )
    , m_bFloating( false )
    , m_bCascading( false )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    vcl::Window* pParentWindow,
    const OString& rID, const OUString& rUIXMLDescription,
    const Reference< XFrame >& rFrame ) :
    FloatingWindow( pParentWindow, rID, rUIXMLDescription, rFrame)
    , m_bFloating( false )
    , m_bCascading( false )
    , m_nId( nId )
    , m_xFrame( rFrame )
    , m_pStatusListener( 0 )
{
    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->AddWindow( this );
}

SfxPopupWindow::~SfxPopupWindow()
{
    disposeOnce();
}

void SfxPopupWindow::dispose()
{
    if ( m_xStatusListener.is() )
    {
        m_xStatusListener->dispose();
        m_xStatusListener.clear();
    }

    vcl::Window* pWindow = GetTopMostParentSystemWindow( this );
    if ( pWindow )
        static_cast<SystemWindow *>(pWindow)->GetTaskPaneList()->RemoveWindow( this );
    FloatingWindow::dispose();
}



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



void SfxPopupWindow::BindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->bindListener();
}



void SfxPopupWindow::UnbindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->unbindListener();
}



void SfxPopupWindow::AddStatusListener( const OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->addStatusListener( rCommandURL );
}



bool SfxPopupWindow::Close()
{
    m_bFloating = false;
    FloatingWindow::Close();

    Delete();
    return true;
}



void SfxPopupWindow::PopupModeEnd()
{
    //! to allow PopupModeEndHdl to be called
    FloatingWindow::PopupModeEnd();

    if ( IsVisible() )
    {
        // was teared-off
        DeleteFloatingWindow();
        m_bFloating = true;
    }
    else
        Close();
}



void SfxPopupWindow::DeleteFloatingWindow()
{
    if ( m_bFloating )
    {
        Hide();
        Delete();
    }
}



void SfxPopupWindow::MouseMove( const ::MouseEvent& rMEvt )
{
    if ( !m_bCascading )
        FloatingWindow::MouseMove( rMEvt );
    else
    {
        // Forward MouseMove-Event to Children
        ::Point       aPos = rMEvt.GetPosPixel();
        ::Point       aScrPos = OutputToScreenPixel( aPos );
        sal_uInt16 i = 0;
        vcl::Window* pWindow = GetChild( i );
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



void SfxPopupWindow::StartCascading()
{
    m_bCascading = true;
}



void SfxPopupWindow::StateChanged(
    sal_uInt16 /*nSID*/,
    SfxItemState eState,
    const SfxPoolItem* /*pState*/ )
/*  [Description]

    See also <SfxControllerItem::StateChanged()>. In addition the Popup
    will become hidden when eState==SfxItemState::DISABLED and in all other
    cases it will be shown again if it is floating. In general this requires
    to call the Base class.

    Due to the parent the presentation mode is handled in a special way.
*/

{
    if ( SfxItemState::DISABLED == eState )
    {
        Hide();
    }
    else if ( m_bFloating )
    {
        Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
    }
}



void SfxPopupWindow::Delete()
{
    if ( m_aDeleteLink.IsSet() )
        m_aDeleteLink.Call( this );
    disposeOnce();
}



SfxRecentFilesToolBoxControl::SfxRecentFilesToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
{
    rBox.SetItemBits( nId, rBox.GetItemBits( nId ) | ToolBoxItemBits::DROPDOWN);
}

SfxRecentFilesToolBoxControl::~SfxRecentFilesToolBoxControl()
{
}

VclPtr<SfxPopupWindow> SfxRecentFilesToolBoxControl::CreatePopupWindow()
{
    ToolBox& rBox = GetToolBox();
    sal_uInt16 nItemId = GetId();
    ::Rectangle aRect( rBox.GetItemRect( nItemId ) );

    Sequence< Any > aArgs( 3 );
    PropertyValue aPropValue;

    aPropValue.Name = "CommandURL";
    aPropValue.Value <<= OUString( ".uno:RecentFileList" );
    aArgs[0] <<= aPropValue;

    aPropValue.Name = "Frame";
    aPropValue.Value <<= m_xFrame;
    aArgs[1] <<= aPropValue;

    aPropValue.Name = "ShowRemote";
    aPropValue.Value <<= true;
    aArgs[2] <<= aPropValue;

    uno::Reference< frame::XPopupMenuController > xPopupController( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.framework.RecentFilesMenuController", aArgs, m_xContext ), UNO_QUERY );

    uno::Reference< awt::XPopupMenu > xPopupMenu( m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.awt.PopupMenu", m_xContext ), uno::UNO_QUERY );

    if ( xPopupController.is() && xPopupMenu.is() )
    {
        xPopupController->setPopupMenu( xPopupMenu );

        rBox.SetItemDown( nItemId, true );
        Reference< awt::XWindowPeer > xPeer( getParent(), uno::UNO_QUERY );

        if ( xPeer.is() )
            xPopupMenu->execute( xPeer, VCLUnoHelper::ConvertToAWTRect( aRect ), 0 );

        rBox.SetItemDown( nItemId, false );
    }

    return 0;
}

SfxSaveAsToolBoxControl::SfxSaveAsToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
{
    rBox.SetItemBits( nId, rBox.GetItemBits( nId ) | ToolBoxItemBits::DROPDOWN);
}

SfxSaveAsToolBoxControl::~SfxSaveAsToolBoxControl()
{
}

VclPtr<SfxPopupWindow> SfxSaveAsToolBoxControl::CreatePopupWindow()
{
    ToolBox& rBox = GetToolBox();
    sal_uInt16 nItemId = GetId();
    ::Rectangle aRect( rBox.GetItemRect( nItemId ) );

    Sequence< Any > aArgs( 2 );
    PropertyValue aPropValue;

    aPropValue.Name = "CommandURL";
    aPropValue.Value <<= OUString( ".uno:SaveAsMenu" );
    aArgs[0] <<= aPropValue;

    aPropValue.Name = "Frame";
    aPropValue.Value <<= m_xFrame;
    aArgs[1] <<= aPropValue;

    uno::Reference< frame::XPopupMenuController > xPopupController( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.framework.SaveAsMenuController", aArgs, m_xContext ), UNO_QUERY );

    uno::Reference< awt::XPopupMenu > xPopupMenu( m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.awt.PopupMenu", m_xContext ), uno::UNO_QUERY );

    if ( xPopupController.is() && xPopupMenu.is() )
    {
        xPopupController->setPopupMenu( xPopupMenu );

        rBox.SetItemDown( nItemId, true );
        Reference< awt::XWindowPeer > xPeer( getParent(), uno::UNO_QUERY );

        if ( xPeer.is() )
            xPopupMenu->execute( xPeer, VCLUnoHelper::ConvertToAWTRect( aRect ), 0 );

        rBox.SetItemDown( nItemId, false );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
