/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backgroundcolorprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence BackgroundColorPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(!rViewInformation.getViewport().isEmpty())
            {
                const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(rViewInformation.getViewport()));
                const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aOutline), getBColor()));
                return Primitive2DSequence(&xRef, 1L);
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        BackgroundColorPrimitive2D::BackgroundColorPrimitive2D(
            const basegfx::BColor& rBColor)
        :   BufferedDecompositionPrimitive2D(),
            maBColor(rBColor),
            maLastViewport()
        {
        }

        bool BackgroundColorPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BackgroundColorPrimitive2D& rCompare = (BackgroundColorPrimitive2D&)rPrimitive;

                return (getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange BackgroundColorPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // always as big as the view
            return rViewInformation.getViewport();
        }

        Primitive2DSequence BackgroundColorPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getBuffered2DDecomposition().hasElements() && (maLastViewport != rViewInformation.getViewport()))
            {
                // conditions of last local decomposition have changed, delete
                const_cast< BackgroundColorPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DSequence());
            }

            if(!getBuffered2DDecomposition().hasElements())
            {
                // remember ViewRange
                const_cast< BackgroundColorPrimitive2D* >(this)->maLastViewport = rViewInformation.getViewport();
            }

            // use parent implementation
            return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(BackgroundColorPrimitive2D, PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
