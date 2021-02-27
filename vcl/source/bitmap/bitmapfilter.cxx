/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapFilter.hxx>
#include <vcl/animate/Animation.hxx>

#include <sal/log.hxx>

BitmapFilter::BitmapFilter() {}

BitmapFilter::~BitmapFilter() {}

bool BitmapFilter::Filter(BitmapEx& rBmpEx, BitmapFilter const& rFilter)
{
    BitmapEx aTmpBmpEx(rFilter.execute(rBmpEx));

    if (aTmpBmpEx.IsEmpty())
    {
        SAL_WARN("vcl.gdi", "Bitmap filter failed " << typeid(rFilter).name());
        return false;
    }

    rBmpEx = aTmpBmpEx;
    return true;
}

bool BitmapFilter::Filter(Animation& rAnimation, BitmapFilter const& rFilter)
{
    SAL_WARN_IF(rAnimation.IsInAnimation(), "vcl", "Animation modified while it is animated");

    bool bRet = false;

    if (!rAnimation.IsInAnimation() && !rAnimation.Count())
    {
        bRet = true;

        std::vector<std::unique_ptr<AnimationBitmap>>& aList = rAnimation.GetAnimationFrames();
        for (size_t i = 0, n = aList.size(); (i < n) && bRet; ++i)
        {
            bRet = BitmapFilter::Filter(aList[i]->maBitmapEx, rFilter);
        }

        BitmapEx aBmpEx(rAnimation.GetBitmapEx());
        BitmapFilter::Filter(aBmpEx, rFilter);
        rAnimation.SetBitmapEx(aBmpEx);
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
