/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helperwrongspellrenderer.cxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: aw $ $Date: 2008/09/24 14:28:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
