/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
