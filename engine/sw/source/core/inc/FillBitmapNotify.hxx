/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <IDocumentLinksAdministration.hxx>
#include <doc.hxx>
#include <hintids.hxx>
#include <swatrset.hxx>

#include <editeng/brushitem.hxx>
#include <svx/fillbitmaplink.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xdef.hxx>

namespace sw
{
class AttrSetChangeHint;
}
class SwAttrSetChg;

namespace sw
{
// The URL of a brush picture that is still waiting for a link update. A brush with a graphic link
// holds no picture yet, so it has none.
inline OUString getDeferredBrushURL(const SvxBrushItem& rBrush)
{
    if (!rBrush.GetGraphicLink().isEmpty())
        return OUString();
    const GraphicObject* pObj = rBrush.GetGraphicObject();
    if (!pObj || pObj->GetGraphic().GetType() != GraphicType::Default)
        return OUString();
    return pObj->GetGraphic().getOriginURL();
}

// The URL of a fill bitmap or brush picture in rSet that is still waiting for a link update.
inline OUString getDeferredHostURL(const SwAttrSet& rSet)
{
    if (const XFillBitmapItem* pItem = rSet.GetItemIfSet(XATTR_FILLBITMAP, false))
        return getDeferredOriginURL(*pItem);
    const SvxBrushItem* pBrush = rSet.GetItemIfSet(RES_BACKGROUND, false);
    return pBrush ? getDeferredBrushURL(*pBrush) : OUString();
}

inline bool hasLinkablePicture(const SfxItemSet& rSet)
{
    return rSet.GetItemIfSet(XATTR_FILLBITMAP, false) || rSet.GetItemIfSet(RES_BACKGROUND, false);
}

// Adaptor for SwFormat/SwContentNode SwClientNotify to the document's
// fill-bitmap link tracker. Fires onFillBitmapURLChanged on rHost when
// XATTR_FILLBITMAP or RES_BACKGROUND appears in either side of the change and the change
// originated on the host's own attribute set (pOwnSet).
template <typename Host>
inline void notifyFillBitmapIfChanged(Host& rHost, const SwAttrSet* pOwnSet,
                                      const SwAttrSetChg* pOld, const SwAttrSetChg* pNew)
{
    if (!pOld || !pNew || !pOwnSet)
        return;
    if (pNew->GetTheChgdSet() != pOwnSet)
        return;
    if (!hasLinkablePicture(*pNew->GetChgSet()) && !hasLinkablePicture(*pOld->GetChgSet()))
        return;
    const OUString aNewURL = getDeferredHostURL(*pOwnSet);
    rHost.GetDoc().getIDocumentLinksAdministration().onFillBitmapURLChanged(rHost, aNewURL);
}

// Adaptor for SwFormat/SwContentNode SwClientNotify(SwAttrSetChange) for use
// in code paths that set/overwrite rHost's attribute set without firing
// SwClientNotify
template <typename Host>
inline void notifyFillBitmapForPutSet(Host& rHost, const SfxItemSet& rPutSet,
                                      const SwAttrSet* pOwnSet)
{
    if (!hasLinkablePicture(rPutSet))
        return;
    const OUString aNewURL = pOwnSet ? getDeferredHostURL(*pOwnSet) : OUString();
    rHost.GetDoc().getIDocumentLinksAdministration().onFillBitmapURLChanged(rHost, aNewURL);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
