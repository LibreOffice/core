/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/animate/Animation.hxx>

class SAL_DLLPUBLIC_RTTI AnimationContainer final
{
public:
    Animation maAnimation;

    AnimationContainer() = default;

    AnimationContainer(Animation const& rAnimation)
        : maAnimation(rAnimation)
    {
    }

    bool operator==(const AnimationContainer& rOther) const
    {
        return maAnimation == rOther.maAnimation;
    }

    void createSwapInfo(ImpSwapInfo& rSwapInfo);

    bool isTransparent() const { return maAnimation.IsTransparent(); }

    sal_uInt64 getSizeBytes() { return maAnimation.GetSizeBytes(); }

    Size getPrefSize() const
    {
        Size aSize = maAnimation.GetBitmapEx().GetPrefSize();
        if (!aSize.Width() || !aSize.Height())
            aSize = maAnimation.GetBitmapEx().GetSizePixel();
        return aSize;
    }

    MapMode getPrefMapMode() const
    {
        const Size aSize = maAnimation.GetBitmapEx().GetPrefSize();
        if (aSize.Width() && aSize.Height())
            return maAnimation.GetBitmapEx().GetPrefMapMode();
        return {};
    }

    sal_uInt32 getLoopCount() { return maAnimation.GetLoopCount(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
