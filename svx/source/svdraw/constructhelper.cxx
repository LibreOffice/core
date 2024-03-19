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

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svl/itemset.hxx>
#include <svx/constructhelper.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/svxids.hrc>
#include <svx/xdef.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnwtit.hxx>

//using namespace ::com::sun::star;

::basegfx::B2DPolyPolygon ConstructHelper::GetLineEndPoly(TranslateId pResId,
                                                          const SdrModel& rModel)
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList(rModel.GetLineEndList());

    if (pLineEndList.is())
    {
        OUString aArrowName(SvxResId(pResId));
        tools::Long nCount = pLineEndList->Count();
        tools::Long nIndex;
        for (nIndex = 0; nIndex < nCount; nIndex++)
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if (pEntry->GetName() == aArrowName)
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

void ConstructHelper::SetLineEnds(SfxItemSet& rAttr, const SdrObject& rObj, sal_uInt16 nSlotId,
                                  tools::Long nWidth)
{
    SdrModel& rModel(rObj.getSdrModelFromSdrObject());

    if (!(nSlotId == SID_LINE_ARROW_START || nSlotId == SID_LINE_ARROW_END
          || nSlotId == SID_LINE_ARROWS || nSlotId == SID_LINE_ARROW_CIRCLE
          || nSlotId == SID_LINE_CIRCLE_ARROW || nSlotId == SID_LINE_ARROW_SQUARE
          || nSlotId == SID_LINE_SQUARE_ARROW || nSlotId == SID_DRAW_MEASURELINE))
        return;

    // set attributes of line start and ends

    // arrowhead
    ::basegfx::B2DPolyPolygon aArrow(GetLineEndPoly(RID_SVXSTR_ARROW, rModel));
    if (!aArrow.count())
    {
        ::basegfx::B2DPolygon aNewArrow;
        aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aNewArrow.setClosed(true);
        aArrow.append(aNewArrow);
    }

    // Circles
    ::basegfx::B2DPolyPolygon aCircle(GetLineEndPoly(RID_SVXSTR_CIRCLE, rModel));
    if (!aCircle.count())
    {
        ::basegfx::B2DPolygon aNewCircle = ::basegfx::utils::createPolygonFromEllipse(
            ::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
        aNewCircle.setClosed(true);
        aCircle.append(aNewCircle);
    }

    // Square
    ::basegfx::B2DPolyPolygon aSquare(GetLineEndPoly(RID_SVXSTR_SQUARE, rModel));
    if (!aSquare.count())
    {
        ::basegfx::B2DPolygon aNewSquare;
        aNewSquare.append(::basegfx::B2DPoint(0.0, 0.0));
        aNewSquare.append(::basegfx::B2DPoint(10.0, 0.0));
        aNewSquare.append(::basegfx::B2DPoint(10.0, 10.0));
        aNewSquare.append(::basegfx::B2DPoint(0.0, 10.0));
        aNewSquare.setClosed(true);
        aSquare.append(aNewSquare);
    }

    SfxItemSet aSet(rModel.GetItemPool());

    // determine line width and calculate with it the line end width
    if (aSet.GetItemState(XATTR_LINEWIDTH) != SfxItemState::INVALID)
    {
        tools::Long nValue = aSet.Get(XATTR_LINEWIDTH).GetValue();
        if (nValue > 0)
            nWidth = nValue * 3;
    }

    switch (nSlotId)
    {
        case SID_LINE_ARROWS:
        case SID_DRAW_MEASURELINE:
        {
            // connector with arrow ends
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineStartWidthItem(nWidth));
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        {
            // connector with arrow start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_END:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        {
            // connector with arrow end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;
    }

    // and again, for the still missing ends
    switch (nSlotId)
    {
        case SID_LINE_ARROW_CIRCLE:
        {
            // circle end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_CIRCLE_ARROW:
        {
            // circle start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;

        case SID_LINE_ARROW_SQUARE:
        {
            // square end
            rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineEndWidthItem(nWidth));
        }
        break;

        case SID_LINE_SQUARE_ARROW:
        {
            // square start
            rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
            rAttr.Put(XLineStartWidthItem(nWidth));
        }
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
