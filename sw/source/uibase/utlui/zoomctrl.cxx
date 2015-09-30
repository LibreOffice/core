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

#include "hintids.hxx"

#include <vcl/status.hxx>
#include <sfx2/app.hxx>
#include <svl/stritem.hxx>
#include <sfx2/zoomitem.hxx>

#include "swtypes.hxx"
#include "zoomctrl.hxx"

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

void SwZoomControl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    if(SfxItemState::DEFAULT == eState && dynamic_cast< const SfxStringItem *>( pState ) !=  nullptr)
    {
        sPreviewZoom = static_cast<const SfxStringItem*>(pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
    }
    else
    {
        sPreviewZoom.clear();
        SvxZoomStatusBarControl::StateChanged(nSID, eState, pState);
    }
}

void SwZoomControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if(sPreviewZoom.isEmpty())
        SvxZoomStatusBarControl::Paint(rUsrEvt);
    else
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
}

void SwZoomControl::Command( const CommandEvent& rCEvt )
{
    if(sPreviewZoom.isEmpty())
        SvxZoomStatusBarControl::Command(rCEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
