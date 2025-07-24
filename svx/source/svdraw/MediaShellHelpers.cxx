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

#include <config_features.h>
#include <svx/MediaShellHelpers.hxx>
#include <avmedia/mediaitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/whiter.hxx>
#include <sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdmrkv.hxx>

namespace svx::MediaShellHelpers
{
void GetState(const SdrMarkView* pSdrView, SfxItemSet& rSet)
{
    if (!pSdrView)
        return;

    SfxWhichIter aIter(rSet);

    for (sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich())
    {
        if (SID_AVMEDIA_TOOLBOX != nWhich)
            continue;

#if HAVE_FEATURE_AVMEDIA
        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        bool bDisable = true;

        if (1 == rMarkList.GetMarkCount())
        {
            SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

            if (dynamic_cast<SdrMediaObj*>(pObj))
            {
                ::avmedia::MediaItem aItem(SID_AVMEDIA_TOOLBOX);

                static_cast<sdr::contact::ViewContactOfSdrMediaObj&>(pObj->GetViewContact())
                    .updateMediaItem(aItem);
                rSet.Put(aItem);
                bDisable = false;
            }
        }

        if (bDisable)
#endif
            rSet.DisableItem(SID_AVMEDIA_TOOLBOX);
    }
}

const ::avmedia::MediaItem* Execute(const SdrMarkView* pSdrView, SfxRequest const& rReq)
{
#if !HAVE_FEATURE_AVMEDIA
    (void)pSdrView;
    (void)rReq;
    return nullptr;
#else
    if (!pSdrView)
        return nullptr;

    if (SID_AVMEDIA_TOOLBOX != rReq.GetSlot())
        return nullptr;

    const SfxItemSet* pArgs = rReq.GetArgs();
    if (!pArgs)
        return nullptr;

    const ::avmedia::MediaItem* pMediaItem = pArgs->GetItemIfSet(SID_AVMEDIA_TOOLBOX, false);
    if (!pMediaItem)
        return nullptr;

    const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();

    if (1 != rMarkList.GetMarkCount())
        return nullptr;

    SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    if (!dynamic_cast<SdrMediaObj*>(pObj))
        return nullptr;

    static_cast<sdr::contact::ViewContactOfSdrMediaObj&>(pObj->GetViewContact())
        .executeMediaItem(*pMediaItem);

    return pMediaItem;
#endif
}

} // end of namespace svx::MediaShellHelpers

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
