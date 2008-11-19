/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrattribute.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX

#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vector>
#include <basegfx/vector/b2dvector.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class SdrTextObj;

namespace drawinglayer { namespace attribute {
    class SdrFillBitmapAttribute;
    class FillHatchAttribute;
    class FillGradientAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineAttribute
        {
            // line definitions
            basegfx::B2DLineJoin                    meJoin;             // B2DLINEJOIN_* defines
            double                                  mfWidth;            // 1/100th mm, 0.0==hair
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                         maColor;            // color of line
            ::std::vector< double >                 maDotDashArray;     // array of double which defines the dot-dash pattern
            double                                  mfFullDotDashLen;   // sum of maDotDashArray (for convenience)

        public:
            SdrLineAttribute(
                basegfx::B2DLineJoin eJoin, double fWidth, double fTransparence, const basegfx::BColor& rColor,
                const ::std::vector< double >& rDotDashArray, double fFullDotDashLen);
            ~SdrLineAttribute();

            // constructor to create a line attribute for a colored hairline
            SdrLineAttribute(const basegfx::BColor& rColor);

            // compare operator
            bool operator==(const SdrLineAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (1.0 != mfTransparence); }
            bool isDashed() const { return (0L != maDotDashArray.size()); }

            // data access
            basegfx::B2DLineJoin getJoin() const { return meJoin; }
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
        class SdrLineStartEndAttribute
        {
            // line arrow definitions
            basegfx::B2DPolyPolygon                 maStartPolyPolygon;     // start Line PolyPolygon
            basegfx::B2DPolyPolygon                 maEndPolyPolygon;       // end Line PolyPolygon
            double                                  mfStartWidth;           // 1/100th mm
            double                                  mfEndWidth;             // 1/100th mm

            // bitfield
            unsigned                                mbStartActive : 1L;     // start of Line is active
            unsigned                                mbEndActive : 1L;       // end of Line is active
            unsigned                                mbStartCentered : 1L;   // Line is centered on line start point
            unsigned                                mbEndCentered : 1L;     // Line is centered on line end point

        public:
            SdrLineStartEndAttribute(
                const basegfx::B2DPolyPolygon& rStartPolyPolygon, const basegfx::B2DPolyPolygon& rEndPolyPolygon,
                double fStartWidth, double fEndWidth, bool bStartActive, bool bEndActive, bool bStartCentered, bool bEndCentered);
            ~SdrLineStartEndAttribute();

            // compare operator
            bool operator==(const SdrLineStartEndAttribute& rCandidate) const;

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
        class SdrShadowAttribute
        {
            // shadow definitions
            basegfx::B2DVector                  maOffset;                   // shadow offset 1/100th mm
            double                              mfTransparence;             // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;                    // color of shadow

        public:
            SdrShadowAttribute(
                const basegfx::B2DVector& rOffset, double fTransparence, const basegfx::BColor& rColor);
            ~SdrShadowAttribute();

            // compare operator
            bool operator==(const SdrShadowAttribute& rCandidate) const;

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
        class SdrFillAttribute
        {
            // fill definitions
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                         maColor;            // fill color
            FillGradientAttribute*                  mpGradient;         // fill gradient (if used)
            FillHatchAttribute*                     mpHatch;            // fill hatch (if used)
            SdrFillBitmapAttribute*                 mpBitmap;           // fill bitmap (if used)

        public:
            SdrFillAttribute(
                double fTransparence, const basegfx::BColor& rColor, FillGradientAttribute* pGradient = 0L,
                FillHatchAttribute* pHatch = 0L, SdrFillBitmapAttribute* pBitmap = 0L);
            ~SdrFillAttribute();

            // copy constructor and assigment operator
            SdrFillAttribute(const SdrFillAttribute& rCandidate);
            SdrFillAttribute& operator=(const SdrFillAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrFillAttribute& rCandidate) const;

            // bool access
            bool isGradient() const { return (0L != mpGradient); }
            bool isHatch() const { return (0L != mpHatch); }
            bool isBitmap() const { return (0L != mpBitmap); }
            bool isColor() const { return (!mpGradient && !mpHatch && !mpBitmap); }
            bool isVisible() const { return (1.0 != mfTransparence); }

            // data access
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const FillGradientAttribute* getGradient() const { return mpGradient; }
            const FillHatchAttribute* getHatch() const { return mpHatch; }
            const SdrFillBitmapAttribute* getBitmap() const { return mpBitmap; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRATTRIBUTE_HXX

// eof
