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

#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/bitmapex.hxx>

enum class Disposal
{
    Not,
    Back,
    Previous
};

struct VCL_DLLPUBLIC AnimationBitmap
{
    BitmapEx aBmpEx;
    Point aPosPix;
    Size aSizePix;
    long nWait;
    Disposal eDisposal;
    bool bUserInput;

    AnimationBitmap()
        : nWait(0)
        , eDisposal(Disposal::Not)
        , bUserInput(false)
    {
    }

    AnimationBitmap(const BitmapEx& rBmpEx, const Point& rPosPix, const Size& rSizePix,
                    long _nWait = 0, Disposal _eDisposal = Disposal::Not)
        : aBmpEx(rBmpEx)
        , aPosPix(rPosPix)
        , aSizePix(rSizePix)
        , nWait(_nWait)
        , eDisposal(_eDisposal)
        , bUserInput(false)
    {
    }

    bool operator==(const AnimationBitmap& rAnimBmp) const
    {
        return (rAnimBmp.aBmpEx == aBmpEx && rAnimBmp.aPosPix == aPosPix
                && rAnimBmp.aSizePix == aSizePix && rAnimBmp.nWait == nWait
                && rAnimBmp.eDisposal == eDisposal && rAnimBmp.bUserInput == bUserInput);
    }

    bool operator!=(const AnimationBitmap& rAnimBmp) const { return !(*this == rAnimBmp); }

    BitmapChecksum GetChecksum() const;
};

#endif // INCLUDED_VCL_ANIMATE_ANIMATIONBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
