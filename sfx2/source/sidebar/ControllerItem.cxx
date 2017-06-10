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
#include <memory>
#include <sfx2/sidebar/ControllerItem.hxx>

#include <sfx2/msgpool.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/help.hxx>

#include <cppuhelper/compbase.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

ControllerItem::ControllerItem (
    const sal_uInt16 nSlotId,
    SfxBindings &rBindings,
    ItemUpdateReceiverInterface& rItemUpdateReceiver)
    : SfxControllerItem(nSlotId, rBindings),
      mrItemUpdateReceiver(rItemUpdateReceiver),
      mxFrameActionListener(),
      msCommandName()
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

ControllerItem::ItemUpdateReceiverInterface::~ItemUpdateReceiverInterface()
{
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
