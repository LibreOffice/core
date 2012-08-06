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

#include <helperwrongspellrenderer.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <tools/gen.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/color/bcolormodifier.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    bool renderWrongSpellPrimitive2D(
        const primitive2d::WrongSpellPrimitive2D& rWrongSpellCandidate,
        OutputDevice& rOutputDevice,
        const basegfx::B2DHomMatrix& rObjectToViewTransformation,
        const basegfx::BColorModifierStack& rBColorModifierStack)
    {
        const basegfx::B2DHomMatrix aLocalTransform(rObjectToViewTransformation * rWrongSpellCandidate.getTransformation());
        const basegfx::B2DVector aFontVectorPixel(aLocalTransform * basegfx::B2DVector(0.0, 1.0));
        const sal_uInt32 nFontPixelHeight(basegfx::fround(aFontVectorPixel.getLength()));

        static const sal_uInt32 nMinimumFontHeight(5); // #define WRONG_SHOW_MIN         5
        static const sal_uInt32 nSmallFontHeight(11);  // #define WRONG_SHOW_SMALL      11
        static const sal_uInt32 nMediumFontHeight(15); // #define WRONG_SHOW_MEDIUM     15

        if(nFontPixelHeight > nMinimumFontHeight)
        {
            const basegfx::B2DPoint aStart(aLocalTransform * basegfx::B2DPoint(rWrongSpellCandidate.getStart(), 0.0));
            const basegfx::B2DPoint aStop(aLocalTransform * basegfx::B2DPoint(rWrongSpellCandidate.getStop(), 0.0));
            const Point aVclStart(basegfx::fround(aStart.getX()), basegfx::fround(aStart.getY()));
            const Point aVclStop(basegfx::fround(aStop.getX()), basegfx::fround(aStop.getY()));
            sal_uInt16 nWaveStyle(WAVE_FLAT);

            if(nFontPixelHeight > nMediumFontHeight)
            {
                nWaveStyle = WAVE_NORMAL;
            }
            else if(nFontPixelHeight > nSmallFontHeight)
            {
                nWaveStyle = WAVE_SMALL;
            }

            // #i101075# draw it. Do not forget to use the evtl. offsetted origin of the target device,
            // e.g. when used with mask/transparence buffer device
            const Point aOrigin(rOutputDevice.GetMapMode().GetOrigin());

            const basegfx::BColor aProcessedColor(rBColorModifierStack.getModifiedColor(rWrongSpellCandidate.getColor()));
            const bool bMapModeEnabledState(rOutputDevice.IsMapModeEnabled());

            rOutputDevice.EnableMapMode(false);
            rOutputDevice.SetLineColor(Color(aProcessedColor));
            rOutputDevice.SetFillColor();
            rOutputDevice.DrawWaveLine(aOrigin + aVclStart, aOrigin + aVclStop, nWaveStyle);
            rOutputDevice.EnableMapMode(bMapModeEnabledState);
        }

        // cannot really go wrong
        return true;
    }
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
