/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texture.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:19 $
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

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#define INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX

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
        class GeoTexSvx
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
        class GeoTexSvxGradient : public GeoTexSvx
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
        class GeoTexSvxGradientLinear : public GeoTexSvxGradient
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
        class GeoTexSvxGradientAxial : public GeoTexSvxGradient
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
        class GeoTexSvxGradientRadial : public GeoTexSvxGradient
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
        class GeoTexSvxGradientElliptical : public GeoTexSvxGradient
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
        class GeoTexSvxGradientSquare : public GeoTexSvxGradient
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
        class GeoTexSvxGradientRect : public GeoTexSvxGradient
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
        class GeoTexSvxHatch : public GeoTexSvx
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
        class GeoTexSvxTiled : public GeoTexSvx
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

// eof
