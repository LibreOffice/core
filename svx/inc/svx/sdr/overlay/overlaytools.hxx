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

#ifndef _SDR_OVERLAY_OVERLAYTOOLS_HXX
#define _SDR_OVERLAY_OVERLAYTOOLS_HXX

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// Overlay helper class which holds a BotmapEx which is to be visualized
// at the given logic position with the Bitmap's pixel size, unscaled and
// unrotated (like a marker). The discrete pixel on the bitmap assocciated
// with the target position is given in discrete X,Y coordinates
namespace drawinglayer
{
    namespace primitive2d
    {
        class OverlayBitmapExPrimitive : public DiscreteMetricDependentPrimitive2D
        {
        private:
            // The BitmapEx to use, PixelSize is used
            BitmapEx                                maBitmapEx;

            // The logic position
            basegfx::B2DPoint                       maBasePosition;

            // The pixel inside the BitmapEx which is assocciated with
            // the target position (offset in the bitmap)
            sal_uInt16                              mnCenterX;
            sal_uInt16                              mnCenterY;

        protected:
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            OverlayBitmapExPrimitive(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rBasePosition,
                sal_uInt16 nCenterX,
                sal_uInt16 nCenterY);

            // data access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
            sal_uInt16 getCenterX() const { return mnCenterX; }
            sal_uInt16 getCenterY() const { return mnCenterY; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// Overlay helper class for a crosshair
namespace drawinglayer
{
    namespace primitive2d
    {
        class OverlayCrosshairPrimitive : public ViewportDependentPrimitive2D
        {
        private:
            // The logic position
            basegfx::B2DPoint                       maBasePosition;

            // The stripe colors and legth
            basegfx::BColor                         maRGBColorA;
            basegfx::BColor                         maRGBColorB;
            double                                  mfDiscreteDashLength;

        protected:
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            OverlayCrosshairPrimitive(
                const basegfx::B2DPoint& rBasePosition,
                const basegfx::BColor& rRGBColorA,
                const basegfx::BColor& rRGBColorB,
                double fDiscreteDashLength);

            // data access
            const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
            const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
            const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// Overlay helper class for a hatch rectangle as used e.g. for text object
// selection hilighting
namespace drawinglayer
{
    namespace primitive2d
    {
        class OverlayHatchRectanglePrimitive : public DiscreteMetricDependentPrimitive2D
        {
        private:
            // the logic rectangle definition
            basegfx::B2DRange               maObjectRange;

            // the hatch definition
            double                          mfDiscreteHatchDistance;
            double                          mfHatchRotation;
            basegfx::BColor                 maHatchColor;

            // the dscrete grow and shrink of the box
            double                          mfDiscreteGrow;
            double                          mfDiscreteShrink;

            // the rotation of the primitive itself
            double                          mfRotation;

        protected:
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            OverlayHatchRectanglePrimitive(
                const basegfx::B2DRange& rObjectRange,
                double fDiscreteHatchDistance,
                double fHatchRotation,
                const basegfx::BColor& rHatchColor,
                double fDiscreteGrow,
                double fDiscreteShrink,
                double fRotation);

            // data access
            const basegfx::B2DRange& getObjectRange() const { return maObjectRange; }
            double getDiscreteHatchDistance() const { return mfDiscreteHatchDistance; }
            double getHatchRotation() const { return mfHatchRotation; }
            const basegfx::BColor& getHatchColor() const { return maHatchColor; }
            double getDiscreteGrow() const { return mfDiscreteGrow; }
            double getDiscreteShrink() const { return mfDiscreteShrink; }
            double getRotation() const { return mfRotation; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// Overlay helper class for a striped helpline

namespace drawinglayer
{
    namespace primitive2d
    {
        enum HelplineStyle
        {
            HELPLINESTYLE_POINT,
            HELPLINESTYLE_VERTICAL,
            HELPLINESTYLE_HORIZONTAL
        };

        class OverlayHelplineStripedPrimitive : public ViewportDependentPrimitive2D
        {
        private:
            // The logic position
            basegfx::B2DPoint                       maBasePosition;

            // the style
            HelplineStyle                           meStyle;

            // The stripe colors and legth
            basegfx::BColor                         maRGBColorA;
            basegfx::BColor                         maRGBColorB;
            double                                  mfDiscreteDashLength;

        protected:
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            OverlayHelplineStripedPrimitive(
                const basegfx::B2DPoint& rBasePosition,
                HelplineStyle eStyle,
                const basegfx::BColor& rRGBColorA,
                const basegfx::BColor& rRGBColorB,
                double fDiscreteDashLength);

            // data access
            const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
            HelplineStyle getStyle() const { return meStyle; }
            const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
            const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// Overlay helper class for rolling rectangle helplines. This primitive is
// only for the extended lines to the ends of the view

namespace drawinglayer
{
    namespace primitive2d
    {
        class OverlayRollingRectanglePrimitive : public ViewportDependentPrimitive2D
        {
        private:
            // The logic range
            basegfx::B2DRange                       maRollingRectangle;

            // The stripe colors and legth
            basegfx::BColor                         maRGBColorA;
            basegfx::BColor                         maRGBColorB;
            double                                  mfDiscreteDashLength;

        protected:
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            OverlayRollingRectanglePrimitive(
                const basegfx::B2DRange& aRollingRectangle,
                const basegfx::BColor& rRGBColorA,
                const basegfx::BColor& rRGBColorB,
                double fDiscreteDashLength);

            // data access
            const basegfx::B2DRange& getRollingRectangle() const { return maRollingRectangle; }
            const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
            const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            // compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYTOOLS_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
