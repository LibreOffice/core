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

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#define INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/tools/gradienttools.hxx>
#include <vector>


namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvx
        {
        public:
            GeoTexSvx();
            virtual ~GeoTexSvx();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;
            bool operator!=(const GeoTexSvx& rGeoTexSvx) const { return !operator==(rGeoTexSvx); }

            // virtual base methods
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        /// helper class for processing equal number of matrices and colors
        /// for texture processing
        struct B2DHomMatrixAndBColor
        {
            basegfx::B2DHomMatrix   maB2DHomMatrix;
            basegfx::BColor         maBColor;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradient : public GeoTexSvx
        {
        protected:
            basegfx::ODFGradientInfo            maGradientInfo;
            basegfx::B2DRange                   maDefinitionRange;
            basegfx::BColor                     maStart;
            basegfx::BColor                     maEnd;
            double                              mfBorder;

        public:
            GeoTexSvxGradient(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder);
            virtual ~GeoTexSvxGradient();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const SAL_OVERRIDE;

            // virtual base methods
            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) = 0;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientLinear : public GeoTexSvxGradient
        {
        protected:
            double                  mfUnitMinX;
            double                  mfUnitWidth;
            double                  mfUnitMaxY;

        public:
            GeoTexSvxGradientLinear(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::B2DRange& rOutputRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fAngle);
            virtual ~GeoTexSvxGradientLinear();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientAxial : public GeoTexSvxGradient
        {
        protected:
            double                  mfUnitMinX;
            double                  mfUnitWidth;

        public:
            GeoTexSvxGradientAxial(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::B2DRange& rOutputRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fAngle);
            virtual ~GeoTexSvxGradientAxial();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientRadial : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientRadial(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fOffsetX,
                double fOffsetY);
            virtual ~GeoTexSvxGradientRadial();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientElliptical : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientElliptical(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle);
            virtual ~GeoTexSvxGradientElliptical();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientSquare : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientSquare(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle);
            virtual ~GeoTexSvxGradientSquare();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientRect : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientRect(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rStart,
                const basegfx::BColor& rEnd,
                sal_uInt32 nSteps,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle);
            virtual ~GeoTexSvxGradientRect();

            virtual void appendTransformationsAndColors(
                std::vector< B2DHomMatrixAndBColor >& rEntries,
                basegfx::BColor& rOuterColor) SAL_OVERRIDE;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const SAL_OVERRIDE;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxHatch : public GeoTexSvx
        {
        protected:
            basegfx::B2DRange                   maOutputRange;
            basegfx::B2DHomMatrix               maTextureTransform;
            basegfx::B2DHomMatrix               maBackTextureTransform;
            double                              mfDistance;
            double                              mfAngle;
            sal_uInt32                          mnSteps;

            /// bitfield
            bool                                mbDefinitionRangeEqualsOutputRange : 1;

        public:
            GeoTexSvxHatch(
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::B2DRange& rOutputRange,
                double fDistance,
                double fAngle);
            virtual ~GeoTexSvxHatch();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const SAL_OVERRIDE;

            void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            double getDistanceToHatch(const basegfx::B2DPoint& rUV) const;
            const basegfx::B2DHomMatrix& getBackTextureTransform() const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer



namespace drawinglayer
{
    namespace texture
    {
        // This class applies a tiling to the unit range. The given range
        // will be repeated inside the unit range in X and Y and for each
        // tile a matrix will be created (by appendTransformations) that
        // represents the needed transformation to map a filling in unit
        // coordinates to that tile.
        // When offsetX is given, every 2nd line will be offsetted by the
        // given percentage value (offsetX has to be 0.0 <= offsetX <= 1.0).
        // Accordingly to offsetY. If both are given, offsetX is preferred
        // and offsetY is ignored.
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxTiled : public GeoTexSvx
        {
        protected:
            basegfx::B2DRange               maRange;
            double                          mfOffsetX;
            double                          mfOffsetY;

        public:
            GeoTexSvxTiled(
                const basegfx::B2DRange& rRange,
                double fOffsetX = 0.0,
                double fOffsetY = 0.0);
            virtual ~GeoTexSvxTiled();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const SAL_OVERRIDE;

            void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
        };
    } // end of namespace texture
} // end of namespace drawinglayer



#endif //_DRAWINGLAYER_TEXTURE_TEXTURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
