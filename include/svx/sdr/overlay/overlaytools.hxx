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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
