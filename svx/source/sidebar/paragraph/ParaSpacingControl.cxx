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

#include "ParaSpacingWindow.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/editids.hrc>
#include <svx/ParaSpacingControl.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/intitem.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>

using namespace svx;

SFX_IMPL_TOOLBOX_CONTROL(ParaAboveSpacingControl, SvxULSpaceItem);
SFX_IMPL_TOOLBOX_CONTROL(ParaBelowSpacingControl, SvxULSpaceItem);

SFX_IMPL_TOOLBOX_CONTROL(ParaLeftSpacingControl, SvxLRSpaceItem);
SFX_IMPL_TOOLBOX_CONTROL(ParaRightSpacingControl, SvxLRSpaceItem);
SFX_IMPL_TOOLBOX_CONTROL(ParaFirstLineSpacingControl, SvxLRSpaceItem);

ParaULSpacingControl::ParaULSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
    addStatusListener(u".uno:MetricUnit"_ustr);
}

ParaULSpacingControl::~ParaULSpacingControl()
{
}

void ParaULSpacingControl::StateChangedAtToolBoxControl(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    ToolBoxItemId nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaULSpacingWindow* pWindow = static_cast<ParaULSpacingWindow*>(rTbx.GetItemWindow(nId));

    DBG_ASSERT( pWindow, "Control not found!" );

    if(SfxItemState::DISABLED == eState)
        pWindow->Disable();
    else
        pWindow->Enable();

    rTbx.EnableItem(nId, SfxItemState::DISABLED != eState);

    if(nSID == SID_ATTR_METRIC && pState && eState >= SfxItemState::DEFAULT)
    {
        const SfxUInt16Item* pMetricItem = static_cast<const SfxUInt16Item*>(pState);
        pWindow->SetUnit(static_cast<FieldUnit>(pMetricItem->GetValue()));
    }
    else if((nSID == SID_ATTR_PARA_ULSPACE
        || nSID == SID_ATTR_PARA_ABOVESPACE
        || nSID == SID_ATTR_PARA_BELOWSPACE )
        && pState && eState >= SfxItemState::DEFAULT)
        pWindow->SetValue(static_cast<const SvxULSpaceItem*>(pState));
}

// ParaAboveSpacingControl

ParaAboveSpacingControl::ParaAboveSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : ParaULSpacingControl(nSlotId, nId, rTbx)
{
}

VclPtr<InterimItemWindow> ParaAboveSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaAboveSpacingWindow> pWindow = VclPtr<ParaAboveSpacingWindow>::Create(pParent);
    pWindow->Show();

    return pWindow;
}

// ParaBelowSpacingControl

ParaBelowSpacingControl::ParaBelowSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : ParaULSpacingControl(nSlotId, nId, rTbx)
{
}

VclPtr<InterimItemWindow> ParaBelowSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaBelowSpacingWindow> pWindow = VclPtr<ParaBelowSpacingWindow>::Create(pParent);
    pWindow->Show();

    return pWindow;
}

// ParaLRSpacingControl

ParaLRSpacingControl::ParaLRSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
    addStatusListener(u".uno:MetricUnit"_ustr);
}

ParaLRSpacingControl::~ParaLRSpacingControl()
{
}

void SAL_CALL ParaLRSpacingControl::dispose()
{
    if(m_xMultiplexer.is())
    {
        m_xMultiplexer->removeAllContextChangeEventListeners(this);
        m_xMultiplexer.clear();
    }

    SfxToolBoxControl::dispose();
}

void ParaLRSpacingControl::StateChangedAtToolBoxControl(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    ToolBoxItemId nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaLRSpacingWindow* pWindow = static_cast<ParaLRSpacingWindow*>(rTbx.GetItemWindow(nId));

    DBG_ASSERT( pWindow, "Control not found!" );

    if(SfxItemState::DISABLED == eState)
        pWindow->Disable();
    else
        pWindow->Enable();

    if(!m_xMultiplexer.is() && m_xFrame.is())
    {
        m_xMultiplexer = css::ui::ContextChangeEventMultiplexer::get(
                                    ::comphelper::getProcessComponentContext());

        m_xMultiplexer->addContextChangeEventListener(this, m_xFrame->getController());
    }

    if(nSID == SID_ATTR_METRIC && pState && eState >= SfxItemState::DEFAULT)
    {
        const SfxUInt16Item* pMetricItem = static_cast<const SfxUInt16Item*>(pState);
        pWindow->SetUnit(static_cast<FieldUnit>(pMetricItem->GetValue()));
    }
    else if(nSID == SID_ATTR_PARA_LRSPACE
         || nSID == SID_ATTR_PARA_LEFTSPACE
         || nSID == SID_ATTR_PARA_RIGHTSPACE
         || nSID == SID_ATTR_PARA_FIRSTLINESPACE
    )
    {
        pWindow->SetValue(eState, pState);
    }
}

void SAL_CALL ParaLRSpacingControl::notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
{
    ToolBoxItemId nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaLRSpacingWindow* pWindow = static_cast<ParaLRSpacingWindow*>(rTbx.GetItemWindow(nId));

    if(pWindow)
    {
        vcl::EnumContext eContext(
                vcl::EnumContext::GetApplicationEnum(rEvent.ApplicationName),
                vcl::EnumContext::GetContextEnum(rEvent.ContextName));
        pWindow->SetContext(eContext);
    }
}

::css::uno::Any SAL_CALL ParaLRSpacingControl::queryInterface(const ::css::uno::Type& aType)
{
    ::css::uno::Any a(SfxToolBoxControl::queryInterface(aType));
    if (a.hasValue())
        return a;

    return ::cppu::queryInterface(aType, static_cast<css::ui::XContextChangeEventListener*>(this));
}

void SAL_CALL ParaLRSpacingControl::acquire() noexcept
{
    SfxToolBoxControl::acquire();
}

void SAL_CALL ParaLRSpacingControl::disposing(const ::css::lang::EventObject&)
{
    SfxToolBoxControl::disposing();
}

void SAL_CALL ParaLRSpacingControl::release() noexcept
{
    SfxToolBoxControl::release();
}

// ParaLeftSpacingControl

ParaLeftSpacingControl::ParaLeftSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
: ParaLRSpacingControl(nSlotId, nId, rTbx)
{
}

VclPtr<InterimItemWindow> ParaLeftSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaLeftSpacingWindow> pWindow = VclPtr<ParaLeftSpacingWindow>::Create(pParent);
    pWindow->Show();

    return pWindow;
}

// ParaRightSpacingControl

ParaRightSpacingControl::ParaRightSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
: ParaLRSpacingControl(nSlotId, nId, rTbx)
{
}

VclPtr<InterimItemWindow> ParaRightSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaRightSpacingWindow> pWindow = VclPtr<ParaRightSpacingWindow>::Create(pParent);
    pWindow->Show();

    return pWindow;
}

// ParaFirstLineSpacingControl

ParaFirstLineSpacingControl::ParaFirstLineSpacingControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
: ParaLRSpacingControl(nSlotId, nId, rTbx)
{
}

VclPtr<InterimItemWindow> ParaFirstLineSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaFirstLineSpacingWindow> pWindow = VclPtr<ParaFirstLineSpacingWindow>::Create(pParent);
    pWindow->Show();

    return pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
