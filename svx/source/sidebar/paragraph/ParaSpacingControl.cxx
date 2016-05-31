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

#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/editids.hrc>
#include <svx/ParaSpacingControl.hxx>
#include <vcl/toolbox.hxx>
#include <svl/itempool.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/intitem.hxx>

using namespace svx;

SFX_IMPL_TOOLBOX_CONTROL(ParaULSpacingControl, SvxULSpaceItem);
SFX_IMPL_TOOLBOX_CONTROL(ParaLRSpacingControl, SvxLRSpaceItem);

ParaULSpacingControl::ParaULSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
    addStatusListener(".uno:MetricUnit");
}

ParaULSpacingControl::~ParaULSpacingControl()
{
}

void ParaULSpacingControl::StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    sal_uInt16 nId = GetId();
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
        pWindow->SetUnit((FieldUnit)pMetricItem->GetValue());
    }
    else if(nSID == SID_ATTR_PARA_ULSPACE && pState && eState >= SfxItemState::DEFAULT)
        pWindow->SetValue(static_cast<const SvxULSpaceItem*>(pState));
}

VclPtr<vcl::Window> ParaULSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaULSpacingWindow> pWindow = VclPtr<ParaULSpacingWindow>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

// ParaLRSpacingControl

ParaLRSpacingControl::ParaLRSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
    addStatusListener(".uno:MetricUnit");
}

ParaLRSpacingControl::~ParaLRSpacingControl()
{
}

void SAL_CALL ParaLRSpacingControl::dispose() throw (css::uno::RuntimeException, std::exception)
{
    if(m_xMultiplexer.is())
    {
        m_xMultiplexer->removeAllContextChangeEventListeners(this);
        m_xMultiplexer.clear();
    }

    SfxToolBoxControl::dispose();
}

void ParaLRSpacingControl::StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaLRSpacingWindow* pWindow = static_cast<ParaLRSpacingWindow*>(rTbx.GetItemWindow(nId));

    DBG_ASSERT( pWindow, "Control not found!" );

    if(!m_xMultiplexer.is())
    {
        m_xMultiplexer = css::ui::ContextChangeEventMultiplexer::get(
                                    ::comphelper::getProcessComponentContext());

        if(m_xFrame.is() && m_xMultiplexer.is())
            m_xMultiplexer->addContextChangeEventListener(this, m_xFrame->getController());
    }

    if(nSID == SID_ATTR_METRIC && pState && eState >= SfxItemState::DEFAULT)
    {
        const SfxUInt16Item* pMetricItem = static_cast<const SfxUInt16Item*>(pState);
        pWindow->SetUnit((FieldUnit)pMetricItem->GetValue());
    }
    else if(nSID == SID_ATTR_PARA_LRSPACE)
    {
        pWindow->SetValue(eState, pState);
    }
}

void SAL_CALL ParaLRSpacingControl::notifyContextChangeEvent(const css::ui::ContextChangeEventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception)
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaLRSpacingWindow* pWindow = static_cast<ParaLRSpacingWindow*>(rTbx.GetItemWindow(nId));

    if(pWindow)
    {
        ::sfx2::sidebar::EnumContext eContext = ::sfx2::sidebar::EnumContext(
                ::sfx2::sidebar::EnumContext::GetApplicationEnum(rEvent.ApplicationName),
                ::sfx2::sidebar::EnumContext::GetContextEnum(rEvent.ContextName));
        pWindow->SetContext(eContext);
    }
}

::css::uno::Any SAL_CALL ParaLRSpacingControl::queryInterface(const ::css::uno::Type& aType)
        throw (::css::uno::RuntimeException, ::std::exception)
{
    ::css::uno::Any a(SfxToolBoxControl::queryInterface(aType));
    if (a.hasValue())
        return a;

    return ::cppu::queryInterface(aType, static_cast<css::ui::XContextChangeEventListener*>(this));
}

void SAL_CALL ParaLRSpacingControl::acquire() throw ()
{
    SfxToolBoxControl::acquire();
}

void SAL_CALL ParaLRSpacingControl::disposing(const ::css::lang::EventObject&)
    throw (::css::uno::RuntimeException, ::std::exception)
{
    SfxToolBoxControl::disposing();
}

void SAL_CALL ParaLRSpacingControl::release() throw ()
{
    SfxToolBoxControl::release();
}

VclPtr<vcl::Window> ParaLRSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaLRSpacingWindow> pWindow = VclPtr<ParaLRSpacingWindow>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
