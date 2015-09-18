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

CanvasUpdateRequester::RequesterMap CanvasUpdateRequester::maRequesterMap;

std::shared_ptr<CanvasUpdateRequester> CanvasUpdateRequester::Instance (
    const Reference<rendering::XSpriteCanvas>& rxSharedCanvas)
{
    RequesterMap::const_iterator iRequester;
    for (iRequester=maRequesterMap.begin(); iRequester!=maRequesterMap.end(); ++iRequester)
    {
        if (iRequester->first == rxSharedCanvas)
            return iRequester->second;
    }

    // No requester for the given canvas found.  Create a new one.
    std::shared_ptr<CanvasUpdateRequester> pRequester (
        new CanvasUpdateRequester(rxSharedCanvas), Deleter());
    maRequesterMap.push_back(RequesterMap::value_type(rxSharedCanvas,pRequester));
    return pRequester;
}

CanvasUpdateRequester::CanvasUpdateRequester (
    const Reference<rendering::XSpriteCanvas>& rxCanvas)
    : mxCanvas(rxCanvas),
      mnUserEventId(0),
      mbUpdateFlag(false)
{
    Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
    if (xComponent.is())
    {
        //xComponent->addEventListener(this);
    }
}

CanvasUpdateRequester::~CanvasUpdateRequester()
{
    if (mnUserEventId != 0)
        Application::RemoveUserEvent(mnUserEventId);
}

void CanvasUpdateRequester::RequestUpdate (const bool bUpdateAll)
{
    if (mnUserEventId == 0)
    {
        mbUpdateFlag = bUpdateAll;
        mnUserEventId = Application::PostUserEvent(LINK(this, CanvasUpdateRequester, Callback));
    }
    else
    {
        mbUpdateFlag |= bUpdateAll;
    }
}

IMPL_LINK_NOARG_TYPED(CanvasUpdateRequester, Callback, void*, void)
{
    mnUserEventId = 0;
    if (mxCanvas.is())
    {
        mxCanvas->updateScreen(mbUpdateFlag);
        mbUpdateFlag = false;
    }
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
