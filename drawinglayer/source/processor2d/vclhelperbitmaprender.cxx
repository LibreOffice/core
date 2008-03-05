/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclhelperbitmaprender.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:45 $
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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPRENDER_HXX
#include <vclhelperbitmaprender.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBITMAPTRANSFORM_HXX
#include <vclhelperbitmaptransform.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// support for different kinds of bitmap rendering using vcl

namespace drawinglayer
{
    void RenderBitmapPrimitive2D_GraphicManager(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform)
    {
        // prepare attributes
        GraphicAttr aAttributes;

        // decompose matrix to check for shear, rotate and mirroring
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

        // mirror flags
        aAttributes.SetMirrorFlags(
            (basegfx::fTools::less(aScale.getX(), 0.0) ? BMP_MIRROR_HORZ : 0)|
            (basegfx::fTools::less(aScale.getY(), 0.0) ? BMP_MIRROR_VERT : 0));

        // rotation
        if(!basegfx::fTools::equalZero(fRotate))
        {
            double fRotation(fmod(3600.0 - (fRotate * (10.0 / F_PI180)), 3600.0));
            aAttributes.SetRotation((sal_uInt16)(fRotation));
        }

        // prepare Bitmap
        basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);

        if(basegfx::fTools::equalZero(fRotate))
        {
            aOutlineRange.transform(rTransform);
        }
        else
        {
            // if rotated, create the unrotated output rectangle for the GraphicManager paint
            basegfx::B2DHomMatrix aSimpleObjectMatrix;

            aSimpleObjectMatrix.scale(fabs(aScale.getX()), fabs(aScale.getY()));
            aSimpleObjectMatrix.translate(aTranslate.getX(), aTranslate.getY());

            aOutlineRange.transform(aSimpleObjectMatrix);
        }

        // prepare dest coor
        const Rectangle aDestRectPixel(
            basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
            basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));

        // paint it using GraphicManager
        Graphic aGraphic(rBitmapEx);
        GraphicObject aGraphicObject(aGraphic);
        aGraphicObject.Draw(&rOutDev, aDestRectPixel.TopLeft(), aDestRectPixel.GetSize(), &aAttributes);
    }

    void RenderBitmapPrimitive2D_BitmapEx(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform)
    {
        // only translate and scale, use vcl's DrawBitmapEx().
        BitmapEx aContent(rBitmapEx);

        // prepare dest coor. Necessary to expand since vcl's DrawBitmapEx draws one pix less
        basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);
        aOutlineRange.transform(rTransform);
        const Rectangle aDestRectPixel(
            basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
            basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));

        // decompose matrix to check for shear, rotate and mirroring
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

        // Check mirroring.
        sal_uInt32 nMirrorFlags(BMP_MIRROR_NONE);

        if(basegfx::fTools::less(aScale.getX(), 0.0))
        {
            nMirrorFlags |= BMP_MIRROR_HORZ;
        }

        if(basegfx::fTools::less(aScale.getY(), 0.0))
        {
            nMirrorFlags |= BMP_MIRROR_VERT;
        }

        if(BMP_MIRROR_NONE != nMirrorFlags)
        {
            aContent.Mirror(nMirrorFlags);
        }

        // draw bitmap
        rOutDev.DrawBitmapEx(aDestRectPixel.TopLeft(), aDestRectPixel.GetSize(), aContent);
    }

    void RenderBitmapPrimitive2D_self(
        OutputDevice& rOutDev,
        const BitmapEx& rBitmapEx,
        const basegfx::B2DHomMatrix& rTransform)
    {
        // process self with free transformation (containing shear and rotate). Get dest rect in pixels.
        basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);
        aOutlineRange.transform(rTransform);
        const Rectangle aDestRectLogic(
            basegfx::fround(aOutlineRange.getMinX()), basegfx::fround(aOutlineRange.getMinY()),
            basegfx::fround(aOutlineRange.getMaxX()), basegfx::fround(aOutlineRange.getMaxY()));
        const Rectangle aDestRectPixel(rOutDev.LogicToPixel(aDestRectLogic));

        // intersect with output pixel size
        const Rectangle aOutputRectPixel(Point(), rOutDev.GetOutputSizePixel());
        const Rectangle aCroppedRectPixel(aDestRectPixel.GetIntersection(aOutputRectPixel));

        if(!aCroppedRectPixel.IsEmpty())
        {
            // build transform from pixel in aDestination to pixel in rBitmapEx
            basegfx::B2DHomMatrix aTransform;

            // from relative in aCroppedRectPixel to relative in aDestRectPixel
            aTransform.translate(aCroppedRectPixel.Left() - aDestRectPixel.Left(), aCroppedRectPixel.Top() - aDestRectPixel.Top());

            // from relative in aDestRectPixel to absolute Logic
            aTransform.scale((double)aDestRectLogic.getWidth() / (double)aDestRectPixel.getWidth(), (double)aDestRectLogic.getHeight() / (double)aDestRectPixel.getHeight());
            aTransform.translate(aDestRectLogic.Left(), aDestRectLogic.Top());

            // from absolute in Logic to unified object coordinates (0.0 .. 1.0 in x and y)
            basegfx::B2DHomMatrix aInvBitmapTransform(rTransform);
            aInvBitmapTransform.invert();
            aTransform = aInvBitmapTransform * aTransform;

            // from unit object coordinates to rBitmapEx pixel coordintes
            const Size aSourceSizePixel(rBitmapEx.GetSizePixel());
            aTransform.scale(aSourceSizePixel.getWidth() - 1L, aSourceSizePixel.getHeight() - 1L);

            // create bitmap using source, destination and linear back-transformation
            BitmapEx aDestination = impTransformBitmapEx(rBitmapEx, aCroppedRectPixel, aTransform);

            // paint
            const bool bWasEnabled(rOutDev.IsMapModeEnabled());
            rOutDev.EnableMapMode(false);
            rOutDev.DrawBitmapEx(aCroppedRectPixel.TopLeft(), aDestination);
            rOutDev.EnableMapMode(bWasEnabled);
        }
    }
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
