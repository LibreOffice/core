/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texture.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:46:03 $
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

#ifndef _DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#define _DRAWINGLAYER_TEXTURE_TEXTURE_HXX

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////
namespace drawinglayer
{
    namespace texture
    {
        class geoTexSvx
        {
        public:
            geoTexSvx();
            virtual ~geoTexSvx();

            // compare operator
            virtual bool operator==(const geoTexSvx& rGeoTexSvx) const;
            bool operator!=(const geoTexSvx& rGeoTexSvx) const { return !operator==(rGeoTexSvx); }

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
        class geoTexSvxGradient : public geoTexSvx
        {
        protected:
            basegfx::B2DHomMatrix               maTextureTransform;
            basegfx::B2DHomMatrix               maBackTextureTransform;
            basegfx::B2DRange                   maTargetRange;
            basegfx::BColor                 maStart;
            basegfx::BColor                 maEnd;
            sal_uInt32                          mnSteps;
            double                              mfAspect;
            double                              mfBorder;

            // helpers
            void impAppendMatrix(::std::vector< basegfx::B2DHomMatrix >& rMatrices, const basegfx::B2DRange& rRange);
            void impAppendColorsRadial(::std::vector< basegfx::BColor >& rColors);

        public:
            geoTexSvxGradient(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder);
            virtual ~geoTexSvxGradient();

            // compare operator
            virtual bool operator==(const geoTexSvx& rGeoTexSvx) const;

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
        class geoTexSvxGradientLinear : public geoTexSvxGradient
        {
        public:
            geoTexSvxGradientLinear(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle);
            virtual ~geoTexSvxGradientLinear();

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
        class geoTexSvxGradientAxial : public geoTexSvxGradient
        {
        protected:
            double                              mfInternalSteps;

        public:
            geoTexSvxGradientAxial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fAngle);
            virtual ~geoTexSvxGradientAxial();

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
        class geoTexSvxGradientRadial : public geoTexSvxGradient
        {
        public:
            geoTexSvxGradientRadial(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY);
            virtual ~geoTexSvxGradientRadial();

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
        class geoTexSvxGradientElliptical : public geoTexSvxGradient
        {
        public:
            geoTexSvxGradientElliptical(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~geoTexSvxGradientElliptical();

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
        class geoTexSvxGradientSquare : public geoTexSvxGradient
        {
        public:
            geoTexSvxGradientSquare(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~geoTexSvxGradientSquare();

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
        class geoTexSvxGradientRect : public geoTexSvxGradient
        {
        public:
            geoTexSvxGradientRect(const basegfx::B2DRange& rTargetRange, const basegfx::BColor& rStart, const basegfx::BColor& rEnd, sal_uInt32 nSteps, double fBorder, double fOffsetX, double fOffsetY, double fAngle);
            virtual ~geoTexSvxGradientRect();

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
        class geoTexSvxHatch : public geoTexSvx
        {
        protected:
            basegfx::B2DHomMatrix               maTextureTransform;
            basegfx::B2DHomMatrix               maBackTextureTransform;
            double                              mfDistance;
            double                              mfAngle;
            sal_uInt32                          mnSteps;

        public:
            geoTexSvxHatch(const basegfx::B2DRange& rTargetRange, double fDistance, double fAngle);
            virtual ~geoTexSvxHatch();

            // compare operator
            virtual bool operator==(const geoTexSvx& rGeoTexSvx) const;

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
        class geoTexSvxTiled : public geoTexSvx
        {
        protected:
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                      maSize;

        public:
            geoTexSvxTiled(const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual ~geoTexSvxTiled();

            // compare operator
            virtual bool operator==(const geoTexSvx& rGeoTexSvx) const;

            virtual void appendTransformations(::std::vector< basegfx::B2DHomMatrix >& rMatrices);
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_TEXTURE_TEXTURE_HXX

// eof
