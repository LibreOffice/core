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


#ifdef __sun
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
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
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
#include <tools/globname.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>

#include <sfx2/tbxctrl.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <statcach.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfxtypes.hxx>
#include <sfx2/module.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/frmhtmlw.hxx>
#include <sfx2/app.hxx>
#include <sfx2/unoctitm.hxx>
#include <workwin.hxx>
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

static vcl::Window* GetTopMostParentSystemWindow( vcl::Window* pWindow )
{
    OSL_ASSERT( pWindow );
    if ( pWindow )
    {
        // ->manually search topmost system window
        // required because their might be another system window between this and the top window
        pWindow = pWindow->GetParent();
        SystemWindow* pTopMostSysWin = nullptr;
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

    return nullptr;
}

svt::ToolboxController* SfxToolBoxControllerFactory( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const OUString& aCommandURL )
{
    SolarMutexGuard aGuard;

    URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );
    if ( !aTargetURL.Arguments.isEmpty() )
        return nullptr;

    SfxObjectShell* pObjShell = nullptr;
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
        css::uno::Reference < css::lang::XUnoTunnel > xObj( xModel, UNO_QUERY );
        if ( xObj.is() )
        {
            css::uno::Sequence < sal_Int8 > aSeq = SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence();
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
                pObjShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
        }
    }

    SfxModule*     pModule   = pObjShell ? pObjShell->GetModule() : nullptr;
    SfxSlotPool*   pSlotPool = nullptr;

    if ( pModule )
        pSlotPool = pModule->GetSlotPool();
    else
        pSlotPool = &(SfxSlotPool::GetSlotPool());

    const SfxSlot* pSlot = pSlotPool->GetUnoSlot( aTargetURL.Path );
    if ( pSlot )
    {
        sal_uInt16 nSlotId = pSlot->GetSlotId();
        if ( nSlotId > 0 )
            return SfxToolBoxControl::CreateControl( nSlotId, nID, pToolbox, pModule );
    }

    return nullptr;
}

struct SfxToolBoxControl_Impl
{
    VclPtr<ToolBox>         pBox;
    bool                    bShowString;
    sal_uInt16              nTbxId;
    sal_uInt16              nSlotId;
    VclPtr<SfxPopupWindow>  mpFloatingWindow;
    VclPtr<SfxPopupWindow>  mpPopupWindow;
    DECL_LINK( WindowEventListener, VclWindowEvent&, void );
};

IMPL_LINK( SfxToolBoxControl_Impl, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if ( ( rEvent.GetId() == VclEventId::WindowMove ) ||
         ( rEvent.GetId() == VclEventId::WindowActivate ))
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
    : pImpl( new SfxToolBoxControl_Impl )
{
    pImpl->pBox = &rBox;
    pImpl->bShowString = bShowStringItems;
    pImpl->nTbxId = nID;
    pImpl->nSlotId = nSlotID;
    pImpl->mpFloatingWindow = nullptr;
    pImpl->mpPopupWindow = nullptr;
}


SfxToolBoxControl::~SfxToolBoxControl()
{
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


void SAL_CALL SfxToolBoxControl::dispose()
{
    if ( m_bDisposed )
        return;

    svt::ToolboxController::dispose();

    // Remove and destroy our item window at our toolbox
    SolarMutexGuard aGuard;
    VclPtr< vcl::Window > pWindow = pImpl->pBox->GetItemWindow( pImpl->nTbxId );
    pImpl->pBox->SetItemWindow( pImpl->nTbxId, nullptr );
    pWindow.disposeAndClear();

    // Delete my popup windows
    pImpl->mpFloatingWindow.disposeAndClear();
    pImpl->mpPopupWindow.disposeAndClear();
}


void SfxToolBoxControl::RegisterToolBoxControl( SfxModule* pMod, const SfxTbxCtrlFactory& rFact)
{
    SfxGetpApp()->RegisterToolBoxControl_Impl( pMod, rFact );
}

SfxToolBoxControl* SfxToolBoxControl::CreateControl( sal_uInt16 nSlotId, sal_uInt16 nTbxId, ToolBox *pBox, SfxModule const * pMod  )
{
    SolarMutexGuard aGuard;

    SfxApplication *pApp = SfxGetpApp();

    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &SfxSlotPool::GetSlotPool();
    const std::type_info* aSlotType = pSlotPool->GetSlotType( nSlotId );
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
                    if( (rFactories[nFactory].nTypeId == *aSlotType) && (rFactories[nFactory].nSlotId == nSlotId) )
                        break;

                if( nFactory == nCount )
                {
                    // if no factory exists for the given slot id, see if we
                    // have a generic factory with the correct slot type and slot id == 0
                    for ( nFactory = 0; nFactory < nCount; ++nFactory )
                        if( (rFactories[nFactory].nTypeId == *aSlotType) && (rFactories[nFactory].nSlotId == 0) )
                            break;
                }

                if( nFactory < nCount )
                {
                    pCtrl = rFactories[nFactory].pCtor( nSlotId, nTbxId, *pBox );
                    return pCtrl;
                }
            }
        }

        SfxTbxCtrlFactArr_Impl &rFactories = pApp->GetTbxCtrlFactories_Impl();
        sal_uInt16 nFactory;
        const sal_uInt16 nCount = rFactories.size();

        for( nFactory = 0; nFactory < nCount; ++nFactory )
            if( (rFactories[nFactory].nTypeId == *aSlotType) && (rFactories[nFactory].nSlotId == nSlotId) )
                break;

        if( nFactory == nCount )
        {
            // if no factory exists for the given slot id, see if we
            // have a generic factory with the correct slot type and slot id == 0
            for( nFactory = 0; nFactory < nCount; ++nFactory )
                if( (rFactories[nFactory].nTypeId == *aSlotType) && (rFactories[nFactory].nSlotId == 0) )
                    break;
        }

        if( nFactory < nCount )
        {
            pCtrl = rFactories[nFactory].pCtor( nSlotId, nTbxId, *pBox );
            return pCtrl;
        }
    }

    return nullptr;
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
                    : pState->IsVoidItem() && !pState->Which()
                        ? SfxItemState::UNKNOWN
                        : SfxItemState::DEFAULT;
}

void SfxToolBoxControl::Dispatch(
    const Reference< XDispatchProvider >& rProvider,
    const OUString& rCommand,
    Sequence< ::PropertyValue > const & aArgs )
{
    if ( rProvider.is() )
    {
        css::util::URL aTargetURL;
        aTargetURL.Complete = rCommand;
        Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = rProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

void SfxToolBoxControl::Dispatch( const OUString& aCommand, css::uno::Sequence< css::beans::PropertyValue > const & aArgs )
{
    Reference < XController > xController;

    SolarMutexGuard aGuard;
    if ( getFrameInterface().is() )
        xController = getFrameInterface()->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        css::util::URL aTargetURL;
        aTargetURL.Complete = aCommand;
        getURLTransformer()->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = xProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

// XStatusListener
void SAL_CALL SfxToolBoxControl::statusChanged( const FeatureStateEvent& rEvent )
{
    SfxViewFrame* pViewFrame = nullptr;
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
            SfxOfficeDispatch* pDisp = nullptr;
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

    if ( nSlotId <= 0 )
        return;

    if ( rEvent.Requery )
        svt::ToolboxController::statusChanged( rEvent );
    else
    {
        SfxItemState eState = SfxItemState::DISABLED;
        std::unique_ptr<SfxPoolItem> pItem;
        if ( rEvent.IsEnabled )
        {
            eState = SfxItemState::DEFAULT;
            css::uno::Type aType = rEvent.State.getValueType();

            if ( aType == cppu::UnoType<void>::get() )
            {
                pItem.reset(new SfxVoidItem( nSlotId ));
                eState = SfxItemState::UNKNOWN;
            }
            else if ( aType == cppu::UnoType<bool>::get() )
            {
                bool bTemp = false;
                rEvent.State >>= bTemp ;
                pItem.reset(new SfxBoolItem( nSlotId, bTemp ));
            }
            else if ( aType == ::cppu::UnoType< ::cppu::UnoUnsignedShortType >::get())
            {
                sal_uInt16 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem.reset(new SfxUInt16Item( nSlotId, nTemp ));
            }
            else if ( aType == cppu::UnoType<sal_uInt32>::get() )
            {
                sal_uInt32 nTemp = 0;
                rEvent.State >>= nTemp ;
                pItem.reset(new SfxUInt32Item( nSlotId, nTemp ));
            }
            else if ( aType == cppu::UnoType<OUString>::get() )
            {
                OUString sTemp ;
                rEvent.State >>= sTemp ;
                pItem.reset(new SfxStringItem( nSlotId, sTemp ));
            }
            else if ( aType == cppu::UnoType< css::frame::status::ItemStatus>::get() )
            {
                ItemStatus aItemStatus;
                rEvent.State >>= aItemStatus;
                SfxItemState tmpState = static_cast<SfxItemState>(aItemStatus.State);
                // make sure no-one tries to send us a combination of states
                if (tmpState != SfxItemState::UNKNOWN && tmpState != SfxItemState::DISABLED &&
                    tmpState != SfxItemState::READONLY && tmpState != SfxItemState::DONTCARE &&
                    tmpState != SfxItemState::DEFAULT && tmpState != SfxItemState::SET)
                    throw css::uno::RuntimeException("unknown status");
                eState = tmpState;
                pItem.reset(new SfxVoidItem( nSlotId ));
            }
            else if ( aType == cppu::UnoType< css::frame::status::Visibility>::get() )
            {
                Visibility aVisibilityStatus;
                rEvent.State >>= aVisibilityStatus;
                pItem.reset(new SfxVisibilityItem( nSlotId, aVisibilityStatus.bVisible ));
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
                    pItem.reset(new SfxVoidItem( nSlotId ));
            }
        }

        StateChanged( nSlotId, eState, pItem.get() );
    }
}

// XToolbarController
void SAL_CALL SfxToolBoxControl::execute( sal_Int16 KeyModifier )
{
    SolarMutexGuard aGuard;
    Select( static_cast<sal_uInt16>(KeyModifier) );
}

void SAL_CALL SfxToolBoxControl::click()
{
    SolarMutexGuard aGuard;
    Click();
}

void SAL_CALL SfxToolBoxControl::doubleClick()
{
    SolarMutexGuard aGuard;
    DoubleClick();
}

Reference< css::awt::XWindow > SAL_CALL SfxToolBoxControl::createPopupWindow()
{
    SolarMutexGuard aGuard;
    VclPtr<vcl::Window> pWindow = CreatePopupWindow();
    if ( pWindow )
        return VCLUnoHelper::GetInterface( pWindow );
    else
        return Reference< css::awt::XWindow >();
}

Reference< css::awt::XWindow > SAL_CALL SfxToolBoxControl::createItemWindow( const Reference< css::awt::XWindow >& rParent )
{
    SolarMutexGuard aGuard;
    return VCLUnoHelper::GetInterface( CreateItemWindow( VCLUnoHelper::GetWindow( rParent )));
}

void SfxToolBoxControl::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pImpl->mpPopupWindow = pWindow;
    pImpl->mpPopupWindow->SetPopupModeEndHdl( LINK( this, SfxToolBoxControl, PopupModeEndHdl ));
    pImpl->mpPopupWindow->SetDeleteLink_Impl( LINK( this, SfxToolBoxControl, ClosePopupWindow ));
}


IMPL_LINK_NOARG(SfxToolBoxControl, PopupModeEndHdl, FloatingWindow*, void)
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
        pImpl->mpFloatingWindow->AddEventListener( LINK( pImpl.get(), SfxToolBoxControl_Impl, WindowEventListener ));
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pImpl->mpPopupWindow.clear();
    }
}


IMPL_LINK( SfxToolBoxControl, ClosePopupWindow, SfxPopupWindow *, pWindow, void )
{
    if ( pWindow == pImpl->mpFloatingWindow )
        pImpl->mpFloatingWindow = nullptr;
    else
        pImpl->mpPopupWindow = nullptr;
}


void SfxToolBoxControl::StateChanged
(
    sal_uInt16              nId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_ASSERT( pImpl->pBox != nullptr, "setting state to dangling ToolBox" );

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
            if ( auto pBoolItem = dynamic_cast< const SfxBoolItem* >(pState) )
            {
                // BoolItem for checking
                if ( pBoolItem->GetValue() )
                    eTri = TRISTATE_TRUE;
                nItemBits |= ToolBoxItemBits::CHECKABLE;
            }
            else if ( dynamic_cast< const SfxEnumItemInterface *>( pState ) !=  nullptr &&
                static_cast<const SfxEnumItemInterface *>(pState)->HasBoolValue())
            {
                // EnumItem is handled as Bool
                if ( static_cast<const SfxEnumItemInterface *>(pState)->GetBoolValue() )
                    eTri = TRISTATE_TRUE;
                nItemBits |= ToolBoxItemBits::CHECKABLE;
            }
            else if ( pImpl->bShowString && dynamic_cast< const SfxStringItem *>( pState ) !=  nullptr )
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
    return nullptr;
}

VclPtr<vcl::Window> SfxToolBoxControl::CreateItemWindow( vcl::Window * )
{
    return VclPtr<vcl::Window>();
}

class SfxFrameStatusListener : public svt::FrameStatusListener
{
    public:
        SfxFrameStatusListener( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                const css::uno::Reference< css::frame::XFrame >& xFrame,
                                SfxPopupWindow* pCallee );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

    private:
        VclPtr<SfxPopupWindow> m_pCallee;
};
SfxFrameStatusListener::SfxFrameStatusListener(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame,
    SfxPopupWindow* pCallee ) :
    svt::FrameStatusListener( rxContext, xFrame ),
    m_pCallee( pCallee )
{
}

// XStatusListener
void SAL_CALL SfxFrameStatusListener::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    m_pCallee->statusChanged( rEvent );
}

SfxPopupWindow::SfxPopupWindow(
    sal_uInt16 nId,
    vcl::Window* pParentWindow,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    FloatingWindow( pParentWindow, nBits )
    , m_bFloating( false )
    , m_bCascading( false )
    , m_nId( nId )
    , m_xFrame( rFrame )
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


void SfxPopupWindow::AddStatusListener( const OUString& rCommandURL )
{
    if ( !m_xStatusListener.is() )
    {
        m_xStatusListener = new SfxFrameStatusListener(
                                    ::comphelper::getProcessComponentContext(),
                                    m_xFrame,
                                    this );
    }
    if ( m_xStatusListener.is() )
        m_xStatusListener->addStatusListener( rCommandURL );
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
        if ( m_bFloating )
        {
            Hide();
            Delete();
        }
        m_bFloating = true;
    }
    else
        Close();
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


void SfxPopupWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( !rEvent.IsEnabled )
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
    VclPtr<SfxPopupWindow> xThis(this);
    m_aDeleteLink.Call( this );
    disposeOnce();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
