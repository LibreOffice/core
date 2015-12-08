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
#include <sfx2/sidebar/ControllerItem.hxx>

#include <sfx2/msgpool.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/bindings.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/help.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <cppuhelper/compbase.hxx>

using namespace css;
using namespace css::uno;

namespace
{
    typedef ::cppu::WeakComponentImplHelper <
        css::frame::XFrameActionListener
        > FrameActionListenerInterfaceBase;

    class FrameActionListener
        : public ::cppu::BaseMutex,
          public FrameActionListenerInterfaceBase
    {
    public:
        FrameActionListener (
            sfx2::sidebar::ControllerItem& rControllerItem,
            const Reference<frame::XFrame>& rxFrame)
            : FrameActionListenerInterfaceBase(m_aMutex),
              mrControllerItem(rControllerItem),
              mxFrame(rxFrame)
        {
            if (mxFrame.is())
                mxFrame->addFrameActionListener(this);
        }
        virtual ~FrameActionListener()
        {
        }
        virtual void SAL_CALL disposing() override
        {
            SolarMutexGuard g;
            if (mxFrame.is())
                mxFrame->removeFrameActionListener(this);
        }
        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw (css::uno::RuntimeException, std::exception) override
        {
            (void)rEvent;

            SolarMutexGuard g;
            mrControllerItem.ResetFrame();
            mxFrame = nullptr;
        }
        virtual void SAL_CALL frameAction (const css::frame::FrameActionEvent& rEvent)
            throw (css::uno::RuntimeException, std::exception) override
        {
            SolarMutexGuard g;
            if (rEvent.Action == frame::FrameAction_CONTEXT_CHANGED)
                mrControllerItem.NotifyFrameContextChange();
        }

    private:
        sfx2::sidebar::ControllerItem& mrControllerItem;
        Reference<frame::XFrame> mxFrame;
    };
}

namespace sfx2 { namespace sidebar {

ControllerItem::ControllerItem (
    const sal_uInt16 nSlotId,
    SfxBindings &rBindings,
    ItemUpdateReceiverInterface& rItemUpdateReceiver)
    : SfxControllerItem(nSlotId, rBindings),
      mrItemUpdateReceiver(rItemUpdateReceiver),
      mxFrame(),
      mxFrameActionListener(),
      msCommandName()
{
}

ControllerItem::ControllerItem (
    const sal_uInt16 nSlotId,
    SfxBindings &rBindings,
    ItemUpdateReceiverInterface& rItemUpdateReceiver,
    const ::rtl::OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
    : SfxControllerItem(nSlotId, rBindings),
      mrItemUpdateReceiver(rItemUpdateReceiver),
      mxFrame(rxFrame),
      mxFrameActionListener(new FrameActionListener(*this, mxFrame)),
      msCommandName(rsCommandName)
{
}

ControllerItem::~ControllerItem()
{
    dispose();
}

void ControllerItem::dispose()
{
    if (mxFrameActionListener.is())
        mxFrameActionListener->dispose();
    mxFrameActionListener.clear();

    SfxControllerItem::dispose();
}

void ControllerItem::StateChanged (
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    mrItemUpdateReceiver.NotifyItemUpdate(nSID, eState, pState, IsEnabled(eState));
}

bool ControllerItem::IsEnabled (SfxItemState eState) const
{
    if (eState == SfxItemState::DISABLED)
        return false;
    else if ( ! SvtCommandOptions().HasEntries(SvtCommandOptions::CMDOPTION_DISABLED))
    {
        // There are no disabled commands.
        return true;
    }
    else if (msCommandName.getLength() == 0)
    {
        // We were not given a command name at construction and can
        // not check the state now.  Assume the best and return true.
        return true;
    }
    else if (SvtCommandOptions().Lookup(SvtCommandOptions::CMDOPTION_DISABLED, msCommandName))
    {
        // The command is part of a list of disabled commands.
        return false;
    }
    else
        return true;
}

void ControllerItem::RequestUpdate()
{
    std::unique_ptr<SfxPoolItem> pState;
    const SfxItemState eState (GetBindings().QueryState(GetId(), pState));
    mrItemUpdateReceiver.NotifyItemUpdate(GetId(), eState, pState.get(), IsEnabled(eState));
}

void ControllerItem::NotifyFrameContextChange()
{
    RequestUpdate();
}

void ControllerItem::ResetFrame()
{
    mxFrame = nullptr;
}

::rtl::OUString ControllerItem::GetHelpText() const
{
    Help* pHelp = Application::GetHelp();
    if (pHelp != nullptr)
    {
        if (msCommandName.getLength() > 0)
        {
            const ::rtl::OUString sHelp (pHelp->GetHelpText(".uno:" + msCommandName, nullptr));
            return sHelp;
        }
    }
    return ::rtl::OUString();
}

Image ControllerItem::GetIcon() const
{
    return GetImage(mxFrame, ".uno:" + msCommandName, false);
}

ControllerItem::ItemUpdateReceiverInterface::~ItemUpdateReceiverInterface()
{
}

void ControllerItem::SetupToolBoxItem (ToolBox& rToolBox, const sal_uInt16 nIndex)
{
    rToolBox.SetQuickHelpText(nIndex, vcl::CommandInfoProvider::Instance().GetTooltipForCommand(".uno:" + msCommandName, mxFrame));
    rToolBox.SetHelpText(nIndex, GetHelpText());
    rToolBox.SetItemImage(nIndex, GetIcon());
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
