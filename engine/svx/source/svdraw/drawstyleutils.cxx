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

#include <svx/drawstyleutils.hxx>

#include <basegfx/utils/bgradient.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svx/chrtitem.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

namespace svx
{
void convertDrawStyleArguments(SfxItemSet& rArgs)
{
    // Non-fully transparent fill color should force solid color fill.
    // It wouldn't make sense to preserve another fill type.
    if (const SfxPoolItem* pColorItem = rArgs.GetItem(SID_ATTR_FILL_COLOR, false))
    {
        const Color aColor = static_cast<const XFillColorItem*>(pColorItem)->GetColorValue();
        if (aColor.IsFullyTransparent())
            rArgs.Put(XFillStyleItem(css::drawing::FillStyle_NONE));
        else
        {
            const SfxPoolItem* pStyleItem = rArgs.GetItem(SID_ATTR_FILL_STYLE, false);
            if (!pStyleItem
                || static_cast<const XFillStyleItem*>(pStyleItem)->GetValue()
                       == css::drawing::FillStyle_NONE)
                rArgs.Put(XFillStyleItem(css::drawing::FillStyle_SOLID));
        }
    }

    // A fully transparent line color means "no line". A non-transparent line
    // color is left as-is here: it is visible alongside any style, and forcing
    // solid is done per object by applyBareLineColorToMarked().
    if (const SfxPoolItem* pColorItem = rArgs.GetItem(SID_ATTR_LINE_COLOR, false))
    {
        const Color aColor = static_cast<const XLineColorItem*>(pColorItem)->GetColorValue();
        if (aColor.IsFullyTransparent())
            rArgs.Put(XLineStyleItem(css::drawing::LineStyle_NONE));
    }

    if (const SvxDoubleItem* pWidthItem = rArgs.GetItemIfSet(SID_ATTR_LINE_WIDTH_ARG, false))
    {
        double fValue = pWidthItem->GetValue();
        // FIXME: different units...
        int nPow = 100;
        int nValue = fValue * nPow;

        XLineWidthItem aItem(nValue);
        rArgs.Put(aItem);
    }

    if (const SfxStringItem* pJSON = rArgs.GetItemIfSet(SID_FILL_GRADIENT_JSON, false))
    {
        basegfx::BGradient aGradient = basegfx::BGradient::fromJSON(pJSON->GetValue());
        XFillGradientItem aItem(aGradient);
        rArgs.Put(aItem);
    }
}

void applyBareLineColorToMarked(SdrView& rView, const SfxItemSet& rArgs)
{
    // A bare line color is a non-transparent line color with no explicit,
    // non-None line style in the request.
    const SfxPoolItem* pLineColor = rArgs.GetItem(SID_ATTR_LINE_COLOR, false);
    const SfxPoolItem* pLineStyle = rArgs.GetItem(SID_ATTR_LINE_STYLE, false);
    const bool bBareLineColor
        = pLineColor
          && !static_cast<const XLineColorItem*>(pLineColor)->GetColorValue().IsFullyTransparent()
          && (!pLineStyle
              || static_cast<const XLineStyleItem*>(pLineStyle)->GetValue()
                     == css::drawing::LineStyle_NONE);
    if (!bBareLineColor)
        return;

    const bool bUndo = rView.IsUndoEnabled();
    const SdrMarkList& rMarkList = rView.GetMarkedObjectList();
    const size_t nMarkCount = rMarkList.GetMarkCount();
    for (size_t i = 0; i < nMarkCount; ++i)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

        if (pObj->GetMergedItemSet().Get(XATTR_LINESTYLE).GetValue() != css::drawing::LineStyle_NONE)
            continue; // already has a line, keep its style

        if (bUndo)
            rView.AddUndo(rView.GetModel().GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
        pObj->SetMergedItem(XLineStyleItem(css::drawing::LineStyle_SOLID));
        pObj->BroadcastObjectChange();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
