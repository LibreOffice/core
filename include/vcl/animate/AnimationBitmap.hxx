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

#ifndef INCLUDED_VCL_ANIMATE_ANIMATIONBITMAP_HXX
#define INCLUDED_VCL_ANIMATE_ANIMATIONBITMAP_HXX

#include <vcl/bitmapex.hxx>

enum class Disposal
{
    Not,
    Back,
    Previous
};

struct AnimationBitmap
{
    BitmapEx maBitmapEx;
    Point maPositionPixel;
    Size maSizePixel;
    tools::Long mnWait;
    Disposal meDisposal;
    bool mbUserInput;

    AnimationBitmap()
        : mnWait(0)
        , meDisposal(Disposal::Not)
        , mbUserInput(false)
    {
    }

    AnimationBitmap(const BitmapEx& rBitmapEx, const Point& rPositionPixel, const Size& rSizePixel,
                    tools::Long nWait = 0, Disposal eDisposal = Disposal::Not)
        : maBitmapEx(rBitmapEx)
        , maPositionPixel(rPositionPixel)
        , maSizePixel(rSizePixel)
        , mnWait(nWait)
        , meDisposal(eDisposal)
        , mbUserInput(false)
    {
    }

    bool operator==(const AnimationBitmap& rAnimationBitmap) const
    {
        return (rAnimationBitmap.maBitmapEx == maBitmapEx
                && rAnimationBitmap.maPositionPixel == maPositionPixel
                && rAnimationBitmap.maSizePixel == maSizePixel && rAnimationBitmap.mnWait == mnWait
                && rAnimationBitmap.meDisposal == meDisposal
                && rAnimationBitmap.mbUserInput == mbUserInput);
    }

    bool operator!=(const AnimationBitmap& rAnimationBitmap) const
    {
        return !(*this == rAnimationBitmap);
    }

    BitmapChecksum GetChecksum() const;
};

#endif // INCLUDED_VCL_ANIMATE_ANIMATIONBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
