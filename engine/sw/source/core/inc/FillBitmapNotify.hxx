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
// Adaptor for SwFormat/SwContentNode SwClientNotify to the document's
// fill-bitmap link tracker. Fires onFillBitmapURLChanged on rHost when
// XATTR_FILLBITMAP appears in either side of the change and the change
// originated on the host's own attribute set (pOwnSet).
template <typename Host>
inline void notifyFillBitmapIfChanged(Host& rHost, const SwAttrSet* pOwnSet,
                                      const SwAttrSetChg* pOld, const SwAttrSetChg* pNew)
{
    if (!pOld || !pNew || !pOwnSet)
        return;
    if (pNew->GetTheChgdSet() != pOwnSet)
        return;
    if (!pNew->GetChgSet()->GetItemIfSet(XATTR_FILLBITMAP, false)
        && !pOld->GetChgSet()->GetItemIfSet(XATTR_FILLBITMAP, false))
        return;
    OUString aNewURL;
    if (const XFillBitmapItem* pItem = pOwnSet->GetItemIfSet(XATTR_FILLBITMAP, false))
        aNewURL = getDeferredOriginURL(*pItem);
    rHost.GetDoc().getIDocumentLinksAdministration().onFillBitmapURLChanged(rHost, aNewURL);
}

// Adaptor for SwFormat/SwContentNode SwClientNotify(SwAttrSetChange) for use
// in code paths that set/overwrite rHost's attribute set without firing
// SwClientNotify
template <typename Host>
inline void notifyFillBitmapForPutSet(Host& rHost, const SfxItemSet& rPutSet,
                                      const SwAttrSet* pOwnSet)
{
    if (!rPutSet.GetItemIfSet(XATTR_FILLBITMAP, false))
        return;
    OUString aNewURL;
    if (pOwnSet)
        if (const XFillBitmapItem* pItem = pOwnSet->GetItemIfSet(XATTR_FILLBITMAP, false))
            aNewURL = getDeferredOriginURL(*pItem);
    rHost.GetDoc().getIDocumentLinksAdministration().onFillBitmapURLChanged(rHost, aNewURL);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
