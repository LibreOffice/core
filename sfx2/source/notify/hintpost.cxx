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

#include <hintpost.hxx>

#include <sfx2/request.hxx>
#include <sfx2/lokhelper.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>

SfxHintPoster::SfxHintPoster(SfxDispatcher* aLink)
    : m_Link(aLink)
{
}

SfxHintPoster::~SfxHintPoster() {}

void SfxHintPoster::Post(std::unique_ptr<SfxRequest> pHintToPost)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        // Store the LOK view at the time of posting, so we can restore it later.
        if (SfxLokHelper::isSettingView())
        {
            // This would be bad, setView() should not trigger new posted hints, otherwise this will
            // never end if the view doesn't match when we execute the posted hint.
            SAL_WARN("sfx.notify", "SfxHintPoster::Post: posting new hint during setting a view");
        }

        pHintToPost->SetLokViewId(SfxLokHelper::getView());
    }

    Application::PostUserEvent((LINK(this, SfxHintPoster, DoEvent_Impl)), pHintToPost.release());
    AddFirstRef();
}

IMPL_LINK(SfxHintPoster, DoEvent_Impl, void*, pPostedHint, void)
{
    auto pRequest = static_cast<SfxRequest*>(pPostedHint);
    if (m_Link)
    {
        bool bSetView = false;
        int nOldId = -1;
        if (comphelper::LibreOfficeKit::isActive())
        {
            int nNewId = pRequest->GetLokViewId();
            nOldId = SfxLokHelper::getView();
            if (nNewId != -1 && nNewId != nOldId)
            {
                // The current view ID is not the one that was active when posting the hint, switch
                // to it.
                SfxLokHelper::setView(nNewId);
                bSetView = true;
            }
        }

        m_Link->PostMsgHandler(std::unique_ptr<SfxRequest>(pRequest));

        if (bSetView)
        {
            SfxLokHelper::setView(nOldId);
        }
    }
    else
        delete pRequest;
    ReleaseRef();
}

void SfxHintPoster::ClearLink() { m_Link = nullptr; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
