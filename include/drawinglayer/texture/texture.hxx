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

//////////////////////////////////////////////////////////////////////////////
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
            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradient : public GeoTexSvx
        {
        protected:
            basegfx::ODFGradientInfo            maGradientInfo;
            basegfx::B2DRange                   maTargetRange;
            basegfx::BColor                     maStart;
            basegfx::BColor                     maEnd;
            double                              mfBorder;

            // helpers
            void impAppendMatrix(::std::vector< basegfx::B2DHomMatrix >& rMatrices, const basegfx::B2DRange& rRange);
            void impAppendColorsRadial(::std::vector< basegfx::BColor >& rColors);

        public:
            GeoTexSvxGradient(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder);
            virtual ~GeoTexSvxGradient();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;

            // virtual base methods
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors) = 0;

            // data access
            const basegfx::BColor& getStart() const { return maStart; }
            const basegfx::BColor& getEnd() const { return maEnd; }
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientLinear : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientLinear(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle);
            virtual ~GeoTexSvxGradientLinear();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientAxial : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientAxial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle);
            virtual ~GeoTexSvxGradientAxial();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientRadial : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientRadial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY);
            virtual ~GeoTexSvxGradientRadial();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientElliptical : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientElliptical(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~GeoTexSvxGradientElliptical();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientSquare : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientSquare(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~GeoTexSvxGradientSquare();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxGradientRect : public GeoTexSvxGradient
        {
        public:
            GeoTexSvxGradientRect(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~GeoTexSvxGradientRect();

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            virtual void appendColors(::std::vector< basegfx::BColor >& rColors);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxHatch : public GeoTexSvx
        {
        protected:
            basegfx::B2DHomMatrix               maTextureTransform;
            basegfx::B2DHomMatrix               maBackTextureTransform;
            double                              mfDistance;
            double                              mfAngle;
            sal_uInt32                          mnSteps;

        public:
            GeoTexSvxHatch(const basegfx::B2DRange& rTargetRange, double fDistance, double fAngle);
            virtual ~GeoTexSvxHatch();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
            double getDistanceToHatch(const basegfx::B2DPoint& rUV) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxTiled : public GeoTexSvx
        {
        protected:
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;

        public:
            GeoTexSvxTiled(const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual ~GeoTexSvxTiled();

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_TEXTURE_TEXTURE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
