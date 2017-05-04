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

#include "CanvasUpdateRequester.hxx"
#include <vcl/svapp.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace presenter {

//===== CanvasUpdateRequester::Deleter ========================================

class CanvasUpdateRequester::Deleter
{
public:
    void operator() (CanvasUpdateRequester* pObject) { delete pObject; }
};

//===== CanvasUpdateRequester =================================================

std::shared_ptr<CanvasUpdateRequester> CanvasUpdateRequester::Instance (
    const Reference<rendering::XSpriteCanvas>& rxSharedCanvas)
{
    // this global must not own anything or we crash on shutdown
    static std::vector<std::pair<
        uno::WeakReference<rendering::XSpriteCanvas>,
        std::weak_ptr<CanvasUpdateRequester>>> s_RequesterMap;
    for (auto it = s_RequesterMap.begin(); it != s_RequesterMap.end(); )
    {
        uno::Reference<rendering::XSpriteCanvas> const xCanvas(it->first);
        if (!xCanvas.is())
        {
            it = s_RequesterMap.erase(it); // remove stale entry
        }
        else if (xCanvas == rxSharedCanvas)
        {
            std::shared_ptr<CanvasUpdateRequester> pRequester(it->second);
            if (pRequester)
            {
                return pRequester;
            }
            else
            {
                std::shared_ptr<CanvasUpdateRequester> const pNew(
                        new CanvasUpdateRequester(rxSharedCanvas), Deleter());
                it->second = pNew;
                return pNew;
            }
        }
        else
        {
            ++it;
        }
    }

    // No requester for the given canvas found.  Create a new one.
    std::shared_ptr<CanvasUpdateRequester> pRequester (
        new CanvasUpdateRequester(rxSharedCanvas), Deleter());
    s_RequesterMap.push_back(std::make_pair(rxSharedCanvas, pRequester));
    return pRequester;
}


CanvasUpdateRequester::CanvasUpdateRequester (
        const Reference<rendering::XSpriteCanvas>& rxCanvas)
    : mxCanvas(rxCanvas)
    , m_pUserEventId(nullptr)
    , mbUpdateFlag(false)
{
    Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
    if (xComponent.is())
    {
        //xComponent->addEventListener(this);
    }
}

CanvasUpdateRequester::~CanvasUpdateRequester()
{
    assert(m_pUserEventId == nullptr);
}

void CanvasUpdateRequester::RequestUpdate (const bool bUpdateAll)
{
    if (m_pUserEventId == nullptr)
    {
        m_pThis = shared_from_this(); // keep instance alive until dispatch
        mbUpdateFlag = bUpdateAll;
        m_pUserEventId = Application::PostUserEvent(LINK(this, CanvasUpdateRequester, Callback));
    }
    else
    {
        mbUpdateFlag |= bUpdateAll;
    }
}

IMPL_LINK_NOARG(CanvasUpdateRequester, Callback, void*, void)
{
    m_pUserEventId = nullptr;
    if (mxCanvas.is())
    {
        mxCanvas->updateScreen(mbUpdateFlag);
        mbUpdateFlag = false;
    }
    assert(m_pThis);
    m_pThis.reset(); // possibly delete "this"
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
