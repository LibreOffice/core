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
#include "SimpleToolBoxController.hxx"

#include <comphelper/processfactory.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>


using namespace ::com::sun::star;

namespace svx { namespace sidebar {

SimpleToolBoxController::SimpleToolBoxController(
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    ToolBox& rToolBox,
    const sal_uInt16 nItemId,
    const rtl::OUString& rsCommand)
    : svt::ToolboxController(comphelper::getComponentContext(comphelper::getProcessServiceFactory()),
        rxFrame, rsCommand),
      mrToolbox(rToolBox),
      mnItemId(nItemId)
{
}




SimpleToolBoxController::~SimpleToolBoxController (void)
{
}




void SAL_CALL SimpleToolBoxController::statusChanged (const css::frame::FeatureStateEvent& rEvent)
    throw (cssu::RuntimeException)
{
    SolarMutexGuard aSolarMutexGuard;

    if (m_bDisposed)
        return;

    mrToolbox.EnableItem(mnItemId, rEvent.IsEnabled);

    sal_uInt16 nItemBits = mrToolbox.GetItemBits(mnItemId);
    nItemBits &= ~TIB_CHECKABLE;
    TriState eState = STATE_NOCHECK;

    sal_Bool bValue;
    if (rEvent.State >>= bValue)
    {
        // Boolean, treat it as checked/unchecked
        mrToolbox.CheckItem(mnItemId, bValue);
        if ( bValue )
            eState = STATE_CHECK;
        nItemBits |= TIB_CHECKABLE;
    }

    mrToolbox.SetItemState(mnItemId, eState);
    mrToolbox.SetItemBits(mnItemId, nItemBits);
}

} } // end of namespace svx::sidebar
