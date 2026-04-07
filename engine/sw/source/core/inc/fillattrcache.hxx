/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_FILLATTRCACHE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FILLATTRCACHE_HXX

#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>
#include <vcl/graph.hxx>

class SfxItemSet;

// Returns true if the fill bitmap in rSet has an unresolved remote graphic
// (GraphicType::Default with a non-empty originURL). When true, callers
// should not cache SdrAllFillAttributesHelper built from this set, because
// the pool item will be replaced when link updates are allowed and the
// cache would become stale.
inline bool hasDeferredFillBitmap(const SfxItemSet& rSet)
{
    const XFillBitmapItem* pBmpItem = rSet.GetItemIfSet(XATTR_FILLBITMAP, false);
    if (!pBmpItem)
        return false;
    const Graphic& rGrf = pBmpItem->GetGraphicObject().GetGraphic();
    return rGrf.GetType() == GraphicType::Default && !rGrf.getOriginURL().isEmpty();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
