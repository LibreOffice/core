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

#include <MediaObjectBar.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/objface.hxx>
#include <svx/MediaShellHelpers.hxx>

#include <strings.hrc>
#include <DrawDocShell.hxx>
#include <ViewShell.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>

using namespace sd;
using namespace svx;

#define ShellClass_MediaObjectBar
#include <sdslots.hxx>

namespace sd
{
SFX_IMPL_INTERFACE(MediaObjectBar, SfxShell)

void MediaObjectBar::InitInterface_Impl() {}

MediaObjectBar::MediaObjectBar(ViewShell& rSdViewShell, ::sd::View* pSdView)
    : SfxShell(rSdViewShell.GetViewShell())
    , mpView(pSdView)
{
    DrawDocShell* pDocShell = rSdViewShell.GetDocSh();

    SetPool(&pDocShell->GetPool());
    SetUndoManager(pDocShell->GetUndoManager());
    SetRepeatTarget(mpView);
    SetName(SdResId(RID_DRAW_MEDIA_TOOLBOX));
}

MediaObjectBar::~MediaObjectBar() { SetRepeatTarget(nullptr); }

void MediaObjectBar::GetState(SfxItemSet& rSet) { MediaShellHelpers::GetState(mpView, rSet); }

void MediaObjectBar::Execute(SfxRequest const& rReq)
{
    const ::avmedia::MediaItem* pMediaItem = MediaShellHelpers::Execute(mpView, rReq);
    if (!pMediaItem)
        return;

    //if only changing state then don't set modified flag (e.g. playing a video)
    if (!(pMediaItem->getMaskSet() & AVMediaSetMask::STATE))
    {
        //fdo #32598: after changing playback opts, set document's modified flag
        SdDrawDocument& rDoc = mpView->GetDoc();
        rDoc.SetChanged();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
