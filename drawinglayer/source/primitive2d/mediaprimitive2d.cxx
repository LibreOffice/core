/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediaprimitive2d.cxx,v $
 *
 *  $Revision: 1.2 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef _AVMEDIA_MEDIAWINDOW_HXX
#include <avmedia/mediawindow.hxx>
#endif

#ifndef _GRFMGR_HXX //autogen
#include <goodies/grfmgr.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence MediaPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval(1);

            // create background object
            basegfx::B2DPolygon aBackgroundPolygon(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 1.0, 1.0)));
            aBackgroundPolygon.transform(getTransform());
            const Primitive2DReference xRefBackground(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aBackgroundPolygon), getBackgroundColor()));
            xRetval[0] = xRefBackground;

            // try to get graphic snapshot
            const Graphic aGraphic(avmedia::MediaWindow::grabFrame(getURL(), true));

            if(GRAPHIC_BITMAP == aGraphic.GetType() || GRAPHIC_GDIMETAFILE == aGraphic.GetType())
            {
                const GraphicObject aGraphicObject(aGraphic);
                const GraphicAttr aGraphicAttr;
                xRetval.realloc(2);
                xRetval[0] = xRefBackground;
                xRetval[1] = Primitive2DReference(new GraphicPrimitive2D(getTransform(), aGraphicObject, aGraphicAttr));
            }

            if(getDiscreteBorder())
            {
                const basegfx::B2DVector aDiscreteInLogic(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector((double)getDiscreteBorder(), (double)getDiscreteBorder()));
                const double fDiscreteSize(aDiscreteInLogic.getX() + aDiscreteInLogic.getY());

                basegfx::B2DRange aSourceRange(0.0, 0.0, 1.0, 1.0);
                aSourceRange.transform(getTransform());

                basegfx::B2DRange aDestRange(aSourceRange);
                aDestRange.grow(-0.5 * fDiscreteSize);

                if(::basegfx::fTools::equalZero(aDestRange.getWidth()) || ::basegfx::fTools::equalZero(aDestRange.getHeight()))
                {
                    // shrunk primitive has no content (zero size in X or Y), nothing to display, nothing to return
                    xRetval = Primitive2DSequence();
                }
                else
                {
                    // create transformation matrix from original range to shrunk range
                    basegfx::B2DHomMatrix aTransform;
                    aTransform.translate(-aSourceRange.getMinX(), -aSourceRange.getMinY());
                    aTransform.scale(aDestRange.getWidth() / aSourceRange.getWidth(), aDestRange.getHeight() / aSourceRange.getHeight());
                    aTransform.translate(aDestRange.getMinX(), aDestRange.getMinY());

                    // add transform primitive
                    const Primitive2DReference aScaled(new TransformPrimitive2D(aTransform, xRetval));
                    xRetval = Primitive2DSequence(&aScaled, 1L);
                }
            }

            return xRetval;
        }

        MediaPrimitive2D::MediaPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const rtl::OUString& rURL,
            const basegfx::BColor& rBackgroundColor,
            sal_uInt32 nDiscreteBorder)
        :   BasePrimitive2D(),
            maTransform(rTransform),
            maURL(rURL),
            maBackgroundColor(rBackgroundColor),
            mnDiscreteBorder(nDiscreteBorder)
        {
        }

        bool MediaPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const MediaPrimitive2D& rCompare = (MediaPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getURL() == rCompare.getURL()
                    && getBackgroundColor() == rCompare.getBackgroundColor()
                    && getDiscreteBorder() == rCompare.getDiscreteBorder());
            }

            return false;
        }

        basegfx::B2DRange MediaPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());

            if(getDiscreteBorder())
            {
                const basegfx::B2DVector aDiscreteInLogic(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector((double)getDiscreteBorder(), (double)getDiscreteBorder()));
                const double fDiscreteSize(aDiscreteInLogic.getX() + aDiscreteInLogic.getY());

                aRetval.grow(-0.5 * fDiscreteSize);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MediaPrimitive2D, PRIMITIVE2D_ID_MEDIAPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
