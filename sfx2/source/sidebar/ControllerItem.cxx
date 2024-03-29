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

#include <sfx2/bindings.hxx>
#include <sfx2/sfxsids.hrc>
#include <comphelper/lok.hxx>
#include <unotools/localedatawrapper.hxx>
#include <tools/fldunit.hxx>
#include <svl/intitem.hxx>

using namespace css;

namespace sfx2::sidebar {

ControllerItem::ControllerItem (
    const sal_uInt16 nSlotId,
    SfxBindings &rBindings,
    ItemUpdateReceiverInterface& rItemUpdateReceiver)
    : SfxControllerItem(nSlotId, rBindings),
      mrItemUpdateReceiver(rItemUpdateReceiver)
{
}

ControllerItem::~ControllerItem()
{
    dispose();
}

void ControllerItem::StateChangedAtToolBoxControl (
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    mrItemUpdateReceiver.NotifyItemUpdate(nSID, eState, pState);
}

void ControllerItem::GetControlState (
    sal_uInt16 nSID,
    boost::property_tree::ptree& rState)
{
    mrItemUpdateReceiver.GetControlState(nSID, rState);
}

void ControllerItem::RequestUpdate()
{
    std::unique_ptr<SfxPoolItem> pState;
    const SfxItemState eState (GetBindings().QueryState(GetId(), pState));
    if (GetId() == SID_ATTR_METRIC && pState && comphelper::LibreOfficeKit::isActive())
    {
        MeasurementSystem eSystem
            = LocaleDataWrapper(comphelper::LibreOfficeKit::getLocale()).getMeasurementSystemEnum();
        FieldUnit eUnit = MeasurementSystem::Metric == eSystem ? FieldUnit::CM : FieldUnit::INCH;
        static_cast<SfxUInt16Item*>(pState.get())->SetValue(static_cast<sal_uInt16>(eUnit));
    }
    mrItemUpdateReceiver.NotifyItemUpdate(GetId(), eState, pState.get());
}

ControllerItem::ItemUpdateReceiverInterface::~ItemUpdateReceiverInterface()
{
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
