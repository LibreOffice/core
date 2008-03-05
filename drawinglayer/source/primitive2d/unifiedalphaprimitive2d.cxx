/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unifiedalphaprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_ALPHAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence UnifiedAlphaPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(0.0 == getAlpha())
            {
                // no transparence used, so just use the content
                return getChildren();
            }
            else if(getAlpha() > 0.0 && getAlpha() < 1.0)
            {
                // create fill polygon for TransparenceList
                const basegfx::B2DPolygon aPolygon(basegfx::tools::createPolygonFromRect(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation)));
                const basegfx::BColor aGray(getAlpha(), getAlpha(), getAlpha());
                const Primitive2DReference xRefA(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon), aGray));
                const Primitive2DSequence aAlphaContent(&xRefA, 1L);

                // create sub-transparence group with a gray-colored rectangular fill polygon
                const Primitive2DReference xRefB(new AlphaPrimitive2D(getChildren(), aAlphaContent));
                return Primitive2DSequence(&xRefB, 1L);
            }
            else
            {
                // completely transparent or invalid definition, add nothing
                return Primitive2DSequence();
            }
        }

        UnifiedAlphaPrimitive2D::UnifiedAlphaPrimitive2D(
            const Primitive2DSequence& rChildren,
            double fAlpha)
        :   GroupPrimitive2D(rChildren),
            mfAlpha(fAlpha)
        {
        }

        bool UnifiedAlphaPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const UnifiedAlphaPrimitive2D& rCompare = (UnifiedAlphaPrimitive2D&)rPrimitive;

                return (getAlpha() == rCompare.getAlpha());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(UnifiedAlphaPrimitive2D, PRIMITIVE2D_ID_UNIFIEDALPHAPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
