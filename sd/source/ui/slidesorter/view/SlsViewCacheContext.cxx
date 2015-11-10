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

#include "SlsViewCacheContext.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "tools/IdleDetection.hxx"
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <vcl/window.hxx>
#include <svx/sdr/contact/objectcontact.hxx>

namespace sd { namespace slidesorter { namespace view {

ViewCacheContext::ViewCacheContext (SlideSorter& rSlideSorter)
    : mrModel(rSlideSorter.GetModel()),
      mrSlideSorter(rSlideSorter)
{
}

ViewCacheContext::~ViewCacheContext()
{
}

void ViewCacheContext::NotifyPreviewCreation (
    cache::CacheKey aKey,
    const Bitmap&)
{
    const model::SharedPageDescriptor pDescriptor (GetDescriptor(aKey));
    if (pDescriptor.get() != nullptr)
    {
        // Force a repaint that will trigger their re-creation.
        mrSlideSorter.GetView().RequestRepaint(pDescriptor);
    }
    else
    {
        // It is OK when a preview was created for a page that is not
        // currently displayed because both normal and master pages are
        // kept in the same cache.
    }
}

bool ViewCacheContext::IsIdle()
{
    sal_Int32 nIdleState (tools::IdleDetection::GetIdleState(mrSlideSorter.GetContentWindow()));
    if (nIdleState == tools::IdleDetection::IDET_IDLE)
        return true;
    else
        return false;
}

bool ViewCacheContext::IsVisible (cache::CacheKey aKey)
{
    const model::SharedPageDescriptor pDescriptor (GetDescriptor(aKey));
    return pDescriptor && pDescriptor->HasState(model::PageDescriptor::ST_Visible);
}

const SdrPage* ViewCacheContext::GetPage (cache::CacheKey aKey)
{
    return static_cast<const SdrPage*>(aKey);
}

std::shared_ptr<std::vector<cache::CacheKey> > ViewCacheContext::GetEntryList (bool bVisible)
{
    std::shared_ptr<std::vector<cache::CacheKey> > pKeys (new std::vector<cache::CacheKey>());

    model::PageEnumeration aPageEnumeration (
        bVisible
            ? model::PageEnumerationProvider::CreateVisiblePagesEnumeration(mrModel)
            : model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));

    while (aPageEnumeration.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
        pKeys->push_back(pDescriptor->GetPage());
    }

    return pKeys;
}

sal_Int32 ViewCacheContext::GetPriority (cache::CacheKey aKey)
{
    return - (static_cast<const SdrPage*>(aKey)->GetPageNum()-1) / 2;
}

model::SharedPageDescriptor ViewCacheContext::GetDescriptor (cache::CacheKey aKey)
{
    sal_uInt16 nPageIndex ((static_cast<const SdrPage*>(aKey)->GetPageNum() - 1) / 2);
    return mrModel.GetPageDescriptor(nPageIndex);
}

css::uno::Reference<css::uno::XInterface> ViewCacheContext::GetModel()
{
    if (mrModel.GetDocument() == nullptr)
        return nullptr;
    else
        return mrModel.GetDocument()->getUnoModel();
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
