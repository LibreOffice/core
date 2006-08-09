/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrattribute.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:36:38 $
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

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#define _DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX

#ifndef _BGFX_POLYGON_B2DLINEGEOMETRY_HXX
#include <basegfx/polygon/b2dlinegeometry.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

#include <vector>

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrTextObj;

namespace drawinglayer { namespace attribute {
    class sdrFillBitmapAttribute;
    class fillHatchAttribute;
    class fillGradientAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class sdrLineAttribute
        {
            // line definitions
            basegfx::tools::B2DLineJoin         meJoin;             // B2DLINEJOIN_* defines
            double                                  mfWidth;            // 1/100th mm, 0.0==hair
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;            // color of line
            ::std::vector< double >                 maDotDashArray;     // array of double which defines the dot-dash pattern
            double                                  mfFullDotDashLen;   // sum of maDotDashArray (for convenience)

        public:
            sdrLineAttribute(
                basegfx::tools::B2DLineJoin eJoin, double fWidth, double fTransparence, const basegfx::BColor& rColor,
                const ::std::vector< double >& rDotDashArray, double fFullDotDashLen);
            ~sdrLineAttribute();

            // compare operator
            bool operator==(const sdrLineAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (1.0 != mfTransparence); }
            bool isDashed() const { return (0L != maDotDashArray.size()); }

            // data access
            basegfx::tools::B2DLineJoin getJoin() const { return meJoin; }
            double getWidth() const { return mfWidth; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class sdrLineStartEndAttribute
        {
            // line arrow definitions
            basegfx::B2DPolyPolygon             maStartPolyPolygon;     // start Line PolyPolygon
            basegfx::B2DPolyPolygon             maEndPolyPolygon;       // end Line PolyPolygon
            double                                  mfStartWidth;           // 1/100th mm
            double                                  mfEndWidth;             // 1/100th mm

            // bitfield
            unsigned                                mbStartActive : 1L;     // start of Line is active
            unsigned                                mbEndActive : 1L;       // end of Line is active
            unsigned                                mbStartCentered : 1L;   // Line is centered on line start point
            unsigned                                mbEndCentered : 1L;     // Line is centered on line end point

        public:
            sdrLineStartEndAttribute(
                const basegfx::B2DPolyPolygon& rStartPolyPolygon, const basegfx::B2DPolyPolygon& rEndPolyPolygon,
                double fStartWidth, double fEndWidth, bool bStartActive, bool bEndActive, bool bStartCentered, bool bEndCentered);
            ~sdrLineStartEndAttribute();

            // compare operator
            bool operator==(const sdrLineStartEndAttribute& rCandidate) const;

            // bool access
            bool isStartActive() const { return mbStartActive; }
            bool isEndActive() const { return mbEndActive; }
            bool isStartCentered() const { return mbStartCentered; }
            bool isEndCentered() const { return mbEndCentered; }
            bool isVisible() const { return (isStartActive() || isEndActive()); }

            // data access
            const basegfx::B2DPolyPolygon& getStartPolyPolygon() const { return maStartPolyPolygon; }
            const basegfx::B2DPolyPolygon& getEndPolyPolygon() const { return maEndPolyPolygon; }
            double getStartWidth() const { return mfStartWidth; }
            double getEndWidth() const { return mfEndWidth; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class sdrShadowAttribute
        {
            // shadow definitions
            basegfx::B2DVector                  maOffset;                   // shadow offset 1/100th mm
            double                                  mfTransparence;             // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;                    // color of shadow

        public:
            sdrShadowAttribute(
                const basegfx::B2DVector& rOffset, double fTransparence, const basegfx::BColor& rColor);
            ~sdrShadowAttribute();

            // compare operator
            bool operator==(const sdrShadowAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (1.0 != mfTransparence); }

            // data access
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class sdrFillAttribute
        {
            // fill definitions
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;            // fill color
            fillGradientAttribute*                  mpGradient;         // fill gradient (if used)
            fillHatchAttribute*                     mpHatch;            // fill hatch (if used)
            sdrFillBitmapAttribute*                 mpBitmap;           // fill bitmap (if used)

        public:
            sdrFillAttribute(
                double fTransparence, const basegfx::BColor& rColor, fillGradientAttribute* pGradient = 0L,
                fillHatchAttribute* pHatch = 0L, sdrFillBitmapAttribute* pBitmap = 0L);
            ~sdrFillAttribute();

            // copy constructor and assigment operator
            sdrFillAttribute(const sdrFillAttribute& rCandidate);
            sdrFillAttribute& operator=(const sdrFillAttribute& rCandidate);

            // compare operator
            bool operator==(const sdrFillAttribute& rCandidate) const;

            // bool access
            bool isGradient() const { return (0L != mpGradient); }
            bool isHatch() const { return (0L != mpHatch); }
            bool isBitmap() const { return (0L != mpBitmap); }
            bool isColor() const { return (!mpGradient && !mpHatch && !mpBitmap); }
            bool isVisible() const { return (1.0 != mfTransparence); }

            // data access
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const fillGradientAttribute* getGradient() const { return mpGradient; }
            const fillHatchAttribute* getHatch() const { return mpHatch; }
            const sdrFillBitmapAttribute* getBitmap() const { return mpBitmap; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX

// eof
