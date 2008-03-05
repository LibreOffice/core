/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texteffectprimitive2d.cxx,v $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE_TEXTEFFECTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_GEOMETRY_VIEWINFORMATION2D_HXX
#include <drawinglayer/geometry/viewinformation2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
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
        static double fDiscreteSize(1.1);

        Primitive2DSequence TextEffectPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence aRetval;

            // get the distance of one discrete units from target display. Use between 1.0 and sqrt(2) to
            // have good results on rotated objects, too
            const basegfx::B2DVector aDistance(rViewInformation.getInverseViewTransformation() * basegfx::B2DVector(fDiscreteSize, fDiscreteSize));
            const basegfx::B2DVector aDiagonalDistance(aDistance * (1.0 / 1.44));

            switch(getTextEffectStyle2D())
            {
                case TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED:
                case TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED:
                case TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT:
                case TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT:
                {
                    // prepare transform of sub-group back to (0,0) and align to X-Axis
                    basegfx::B2DHomMatrix aBackTransform;
                    aBackTransform.translate(-getRotationCenter().getX(), -getRotationCenter().getY());
                    aBackTransform.rotate(-getDirection());

                    // prepare transform of sub-group back to it's position and rotation
                    basegfx::B2DHomMatrix aForwardTransform;
                    aForwardTransform.rotate(getDirection());
                    aForwardTransform.translate(getRotationCenter().getX(), getRotationCenter().getY());

                    // create transformation for one discrete unit
                    const bool bEmbossed(
                        TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED  == getTextEffectStyle2D()
                        || TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT == getTextEffectStyle2D());
                    const bool bDefaultTextColor(
                        TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT == getTextEffectStyle2D()
                        || TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT == getTextEffectStyle2D());
                    basegfx::B2DHomMatrix aTransform(aBackTransform);
                    aRetval.realloc(2);

                    if(bEmbossed)
                    {
                        // to bottom-right
                        aTransform.translate(aDiagonalDistance.getX(), aDiagonalDistance.getY());
                    }
                    else
                    {
                        // to top-left
                        aTransform.translate(-aDiagonalDistance.getX(), -aDiagonalDistance.getY());
                    }

                    aTransform *= aForwardTransform;

                    if(bDefaultTextColor)
                    {
                        // emboss/engrave in black, original forced to white
                        const basegfx::BColorModifier aBColorModifierToGray(basegfx::BColor(0.0));
                        const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(getChildren(), aBColorModifierToGray));
                        aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, Primitive2DSequence(&xModifiedColor, 1)));

                        // add original, too
                        const basegfx::BColorModifier aBColorModifierToWhite(basegfx::BColor(1.0));
                        aRetval[1] = Primitive2DReference(new ModifiedColorPrimitive2D(getChildren(), aBColorModifierToWhite));
                    }
                    else
                    {
                        // emboss/engrave in gray, keep original's color
                        const basegfx::BColorModifier aBColorModifierToGray(basegfx::BColor(0.75)); // 192
                        const Primitive2DReference xModifiedColor(new ModifiedColorPrimitive2D(getChildren(), aBColorModifierToGray));
                        aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, Primitive2DSequence(&xModifiedColor, 1)));

                        // add original, too
                        aRetval[1] = Primitive2DReference(new GroupPrimitive2D(getChildren()));
                    }

                    break;
                }
                case TEXTEFFECTSTYLE2D_OUTLINE:
                {
                    // create transform primitives in all directions
                    basegfx::B2DHomMatrix aTransform;
                    aRetval.realloc(9);

                    aTransform.set(0, 2, aDistance.getX());
                    aTransform.set(1, 2, 0.0);
                    aRetval[0] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, aDiagonalDistance.getX());
                    aTransform.set(1, 2, aDiagonalDistance.getY());
                    aRetval[1] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, 0.0);
                    aTransform.set(1, 2, aDistance.getY());
                    aRetval[2] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, -aDiagonalDistance.getX());
                    aTransform.set(1, 2, aDiagonalDistance.getY());
                    aRetval[3] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, -aDistance.getX());
                    aTransform.set(1, 2, 0.0);
                    aRetval[4] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, -aDiagonalDistance.getX());
                    aTransform.set(1, 2, -aDiagonalDistance.getY());
                    aRetval[5] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, 0.0);
                    aTransform.set(1, 2, -aDistance.getY());
                    aRetval[6] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    aTransform.set(0, 2, aDiagonalDistance.getX());
                    aTransform.set(1, 2, -aDiagonalDistance.getY());
                    aRetval[7] = Primitive2DReference(new TransformPrimitive2D(aTransform, getChildren()));

                    // at last, place original over it, but force to white
                    const basegfx::BColorModifier aBColorModifierToWhite(basegfx::BColor(1.0, 1.0, 1.0));
                    aRetval[8] = Primitive2DReference(new ModifiedColorPrimitive2D(getChildren(), aBColorModifierToWhite));

                    break;
                }
            }

            return aRetval;
        }

        TextEffectPrimitive2D::TextEffectPrimitive2D(
            const Primitive2DSequence& rChildren,
            const basegfx::B2DPoint& rRotationCenter,
            double fDirection,
            TextEffectStyle2D eTextEffectStyle2D)
        :   GroupPrimitive2D(rChildren),
            maRotationCenter(rRotationCenter),
            mfDirection(fDirection),
            meTextEffectStyle2D(eTextEffectStyle2D)
        {
        }

        bool TextEffectPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const TextEffectPrimitive2D& rCompare = (TextEffectPrimitive2D&)rPrimitive;

                return (getRotationCenter() == rCompare.getRotationCenter()
                    && getDirection() == rCompare.getDirection()
                    && getTextEffectStyle2D() == rCompare.getTextEffectStyle2D());
            }

            return false;
        }

        basegfx::B2DRange TextEffectPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // get range of content and grow by used fDiscreteSize. That way it is not necessary to ask
            // the whole decomposition for it's ranges (which may be expensive with outline mode which
            // then will ask 9 times at nearly the same content. This may even be refined here using the
            // TextEffectStyle information, e.g. for TEXTEFFECTSTYLE2D_RELIEF the grow needs only to
            // be in two directions
            basegfx::B2DRange aRetval(getB2DRangeFromPrimitive2DSequence(getChildren(), rViewInformation));
            aRetval.grow(fDiscreteSize);

            return aRetval;
        }

        Primitive2DSequence TextEffectPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getLocalDecomposition().hasElements())
            {
                if(maLastViewTransformation != rViewInformation.getViewTransformation())
                {
                    // conditions of last local decomposition have changed, delete
                    const_cast< TextEffectPrimitive2D* >(this)->setLocalDecomposition(Primitive2DSequence());
                }
            }

            if(!getLocalDecomposition().hasElements())
            {
                // remember ViewRange and ViewTransformation
                const_cast< TextEffectPrimitive2D* >(this)->maLastViewTransformation = rViewInformation.getViewTransformation();
            }

            // use parent implementation
            return BasePrimitive2D::get2DDecomposition(rViewInformation);
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextEffectPrimitive2D, PRIMITIVE2D_ID_TEXTEFFECTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
