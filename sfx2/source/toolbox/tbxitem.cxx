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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/visitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/InterimItemWindow.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <sidebar/ControllerFactory.hxx>
#include <unoctitm.hxx>
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

rtl::Reference<svt::ToolboxController> SfxToolBoxControllerFactory( const Reference< XFrame >& rFrame, ToolBox* pToolbox, ToolBoxItemId nID, const OUString& aCommandURL )
{
    SolarMutexGuard aGuard;

    URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    Reference < XURLTransformer > xTrans( URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );
    if ( !aTargetURL.Arguments.isEmpty() )
        return nullptr;

    Reference < XController > xController;
    Reference < XModel > xModel;
    if ( rFrame.is() )
    {
        xController = rFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    SfxObjectShell* pObjShell = SfxObjectShell::GetShellFromComponent(xModel);
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
    ToolBoxItemId           nTbxId;
    sal_uInt16              nSlotId;
};

SfxToolBoxControl::SfxToolBoxControl(
    sal_uInt16      nSlotID,
    ToolBoxItemId   nID,
    ToolBox&        rBox,
    bool            bShowStringItems     )
    : pImpl( new SfxToolBoxControl_Impl )
{
    pImpl->pBox = &rBox;
    pImpl->bShowString = bShowStringItems;
    pImpl->nTbxId = nID;
    pImpl->nSlotId = nSlotID;
}


SfxToolBoxControl::~SfxToolBoxControl()
{
}


ToolBox& SfxToolBoxControl::GetToolBox() const
{
    return *pImpl->pBox;
}
ToolBoxItemId SfxToolBoxControl::GetId() const
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
}


void SfxToolBoxControl::RegisterToolBoxControl( SfxModule* pMod, const SfxTbxCtrlFactory& rFact)
{
    SfxGetpApp()->RegisterToolBoxControl_Impl( pMod, rFact );
}

rtl::Reference<SfxToolBoxControl> SfxToolBoxControl::CreateControl( sal_uInt16 nSlotId, ToolBoxItemId nTbxId, ToolBox *pBox, SfxModule const * pMod  )
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
        rtl::Reference<SfxToolBoxControl> pCtrl;
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
    CreatePopupWindow();
    return nullptr;
}

Reference< css::awt::XWindow > SAL_CALL SfxToolBoxControl::createItemWindow( const Reference< css::awt::XWindow >& rParent )
{
    SolarMutexGuard aGuard;
    return VCLUnoHelper::GetInterface( CreateItemWindow( VCLUnoHelper::GetWindow( rParent )));
}

void SfxToolBoxControl::StateChanged
(
    sal_uInt16          /*nSlotId*/,
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
            else if ( auto pEnumItem = dynamic_cast< const SfxEnumItemInterface *>( pState ) )
            {
                if (pEnumItem->HasBoolValue())
                {
                    // EnumItem is handled as Bool
                    if (pEnumItem->GetBoolValue())
                        eTri = TRISTATE_TRUE;
                    nItemBits |= ToolBoxItemBits::CHECKABLE;
                }
            }
            else if ( pImpl->bShowString )
            {
                if (auto pStringItem = dynamic_cast< const SfxStringItem *>( pState ) )
                    pImpl->pBox->SetItemText(GetId(), pStringItem->GetValue() );
            }
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

void SfxToolBoxControl::CreatePopupWindow()
{
}

VclPtr<InterimItemWindow> SfxToolBoxControl::CreateItemWindow(vcl::Window*)
{
    return VclPtr<InterimItemWindow>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
