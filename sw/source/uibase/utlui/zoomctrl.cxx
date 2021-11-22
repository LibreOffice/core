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

#include <vcl/status.hxx>
#include <svl/stritem.hxx>
#include <sfx2/zoomitem.hxx>

#include <zoomctrl.hxx>

SFX_IMPL_STATUSBAR_CONTROL( SwZoomControl, SvxZoomItem );

SwZoomControl::SwZoomControl( sal_uInt16 _nSlotId,
                              sal_uInt16 _nId,
                              StatusBar& rStb ) :
    SvxZoomStatusBarControl( _nSlotId, _nId, rStb )
{
}

SwZoomControl::~SwZoomControl()
{
}

void SwZoomControl::StateChangedAtStatusBarControl( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    const SfxStringItem* pItem = nullptr;
    if (SfxItemState::DEFAULT == eState && (pItem = dynamic_cast<const SfxStringItem*>(pState)))
    {
        m_sPreviewZoom = pItem->GetValue();
        GetStatusBar().SetItemText(GetId(), m_sPreviewZoom);
    }
    else
    {
        m_sPreviewZoom.clear();
        SvxZoomStatusBarControl::StateChangedAtStatusBarControl(nSID, eState, pState);
    }
}

void SwZoomControl::Paint( const UserDrawEvent& )
{
}

void SwZoomControl::Command( const CommandEvent& rCEvt )
{
    if (m_sPreviewZoom.isEmpty())
        SvxZoomStatusBarControl::Command(rCEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
