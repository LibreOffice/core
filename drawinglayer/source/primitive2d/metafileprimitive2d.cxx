/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: metafileprimitive2d.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:20 $
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

#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <vcl/lineinfo.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <vcl/metaact.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <vcl/salbtype.hxx>
#include <drawinglayer/primitive2d/unifiedalphaprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/primitive2d/alphaprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/wallpaperprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <i18npool/mslangid.hxx>
#include <numeric>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    class PropertyHolder
    {
    private:
        basegfx::B2DHomMatrix   maTransformation;
        MapUnit                 maMapUnit;

        basegfx::BColor         maLineColor;
        basegfx::BColor         maFillColor;
        basegfx::BColor         maTextColor;
        basegfx::BColor         maTextFillColor;
        basegfx::BColor         maTextLineColor;
        basegfx::BColor         maOverlineColor;

        Region                  maRegion;
        Font                    maFont;
        RasterOp                maRasterOp;
        sal_uInt32              mnLayoutMode;
        LanguageType            maLanguageType;

        /// bitfield
        bool                    mbLineColor : 1;
        bool                    mbFillColor : 1;
        bool                    mbTextColor : 1;
        bool                    mbTextFillColor : 1;
        bool                    mbTextLineColor : 1;
        bool                    mbOverlineColor : 1;
        bool                    mbRegion : 1;

    public:
        PropertyHolder()
        :   maTransformation(),
            maMapUnit(MAP_100TH_MM),
            maLineColor(),
            maFillColor(),
            maTextColor(COL_BLACK),
            maTextFillColor(),
            maTextLineColor(),
            maOverlineColor(),
            maRegion(),
            maFont(),
            maRasterOp(ROP_OVERPAINT),
            mnLayoutMode(0),
            maLanguageType(0),
            mbLineColor(false),
            mbFillColor(false),
            mbTextColor(true),
            mbTextFillColor(false),
            mbTextLineColor(false),
            mbOverlineColor(false),
            mbRegion(false)
        {
        }

        ~PropertyHolder()
        {
        }

        const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
        void setTransformation(const basegfx::B2DHomMatrix& rNew) { if(rNew != maTransformation) maTransformation = rNew; }

        MapUnit getMapUnit() const { return maMapUnit; }
        void setMapUnit(MapUnit eNew) { if(eNew != maMapUnit) maMapUnit = eNew; }

        const basegfx::BColor& getLineColor() const { return maLineColor; }
        void setLineColor(const basegfx::BColor& rNew) { if(rNew != maLineColor) maLineColor = rNew; }
        bool getLineColorActive() const { return mbLineColor; }
        void setLineColorActive(bool bNew) { if(bNew != mbLineColor) mbLineColor = bNew; }

        const basegfx::BColor& getFillColor() const { return maFillColor; }
        void setFillColor(const basegfx::BColor& rNew) { if(rNew != maFillColor) maFillColor = rNew; }
        bool getFillColorActive() const { return mbFillColor; }
        void setFillColorActive(bool bNew) { if(bNew != mbFillColor) mbFillColor = bNew; }

        const basegfx::BColor& getTextColor() const { return maTextColor; }
        void setTextColor(const basegfx::BColor& rNew) { if(rNew != maTextColor) maTextColor = rNew; }
        bool getTextColorActive() const { return mbTextColor; }
        void setTextColorActive(bool bNew) { if(bNew != mbTextColor) mbTextColor = bNew; }

        const basegfx::BColor& getTextFillColor() const { return maTextFillColor; }
        void setTextFillColor(const basegfx::BColor& rNew) { if(rNew != maTextFillColor) maTextFillColor = rNew; }
        bool getTextFillColorActive() const { return mbTextFillColor; }
        void setTextFillColorActive(bool bNew) { if(bNew != mbTextFillColor) mbTextFillColor = bNew; }

        const basegfx::BColor& getTextLineColor() const { return maTextLineColor; }
        void setTextLineColor(const basegfx::BColor& rNew) { if(rNew != maTextLineColor) maTextLineColor = rNew; }
        bool getTextLineColorActive() const { return mbTextLineColor; }
        void setTextLineColorActive(bool bNew) { if(bNew != mbTextLineColor) mbTextLineColor = bNew; }

        const basegfx::BColor& getOverlineColor() const { return maOverlineColor; }
        void setOverlineColor(const basegfx::BColor& rNew) { if(rNew != maOverlineColor) maOverlineColor = rNew; }
        bool getOverlineColorActive() const { return mbOverlineColor; }
        void setOverlineColorActive(bool bNew) { if(bNew != mbOverlineColor) mbOverlineColor = bNew; }

        const Region& getRegion() const { return maRegion; }
        void setRegion(const Region& rRegion) { if(rRegion != maRegion) maRegion = rRegion; }
        bool getRegionActive() const { return mbRegion; }
        void setRegionActive(bool bNew) { if(bNew != mbRegion) mbRegion = bNew; }

        const Font& getFont() const { return maFont; }
        void setFont(const Font& rFont) { if(rFont != maFont) maFont = rFont; }

        const RasterOp& getRasterOp() const { return maRasterOp; }
        void setRasterOp(const RasterOp& rRasterOp) { if(rRasterOp != maRasterOp) maRasterOp = rRasterOp; }
        bool isRasterOpInvert() const { return (ROP_XOR == maRasterOp || ROP_INVERT == maRasterOp); }
        bool isRasterOpForceBlack() const { return ROP_0 == maRasterOp; }
        bool isRasterOpActive() const { return isRasterOpInvert() || isRasterOpForceBlack(); }

        sal_uInt32 getLayoutMode() const { return mnLayoutMode; }
        void setLayoutMode(sal_uInt32 nNew) { if(nNew != mnLayoutMode) mnLayoutMode = nNew; }

        LanguageType getLanguageType() const { return maLanguageType; }
        void setLanguageType(LanguageType aNew) { if(aNew != maLanguageType) maLanguageType = aNew; }

        bool getLineOrFillActive() const { return (mbLineColor || mbFillColor); }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace
{
    class PropertyHolders
    {
    private:
        std::vector< PropertyHolder* >          maPropertyHolders;

    public:
        PropertyHolders()
        {
            maPropertyHolders.push_back(new PropertyHolder());
        }

        sal_uInt32 size()
        {
            return maPropertyHolders.size();
        }

        void Push()
        {
            OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: PUSH with no property holders (!)");
            maPropertyHolders.push_back(new PropertyHolder(*maPropertyHolders[maPropertyHolders.size() - 1]));
        }

        void Pop()
        {
            OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: POP with no property holders (!)");
            if(maPropertyHolders.size())
            {
                delete maPropertyHolders[maPropertyHolders.size() - 1];
                maPropertyHolders.pop_back();
            }
        }

        PropertyHolder& Current()
        {
            OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: CURRENT with no property holders (!)");
            return *maPropertyHolders[maPropertyHolders.size() - 1];
        }

        ~PropertyHolders()
        {
            while(maPropertyHolders.size())
            {
                Pop();
            }
        }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace
{
    basegfx::B2DPolyPolygon getB2DPolyPolygonFromRegion(const Region& rRegion)
    {
        basegfx::B2DPolyPolygon aRetval;

        if(!rRegion.IsEmpty())
        {
            Region aRegion(rRegion);
            aRetval = aRegion.GetB2DPolyPolygon();

            if(!aRetval.count())
            {
                RegionHandle aRegionHandle(aRegion.BeginEnumRects());
                Rectangle aRegionRectangle;

                while(aRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                {
                    if(!aRegionRectangle.IsEmpty())
                    {
                        const basegfx::B2DRange aRegionRange(
                            aRegionRectangle.Left(), aRegionRectangle.Top(),
                            aRegionRectangle.Right(), aRegionRectangle.Bottom());
                        aRetval.append(basegfx::tools::createPolygonFromRect(aRegionRange));
                    }
                }

                aRegion.EndEnumRects(aRegionHandle);
            }
        }

        return aRetval;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace
{
    class TargetHolder
    {
    private:
        std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aTargets;

    public:
        TargetHolder()
        :   aTargets()
        {
        }

        ~TargetHolder()
        {
            const sal_uInt32 nCount(aTargets.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                delete aTargets[a];
            }
        }

        sal_uInt32 size()
        {
            return aTargets.size();
        }

        void append(drawinglayer::primitive2d::BasePrimitive2D* pCandidate)
        {
            if(pCandidate)
            {
                aTargets.push_back(pCandidate);
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequence(const PropertyHolder& rPropertyHolder)
        {
            const sal_uInt32 nCount(aTargets.size());
            drawinglayer::primitive2d::Primitive2DSequence xRetval(nCount);

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                xRetval[a] = aTargets[a];
            }

            // All Targets were pointers, but do not need to be deleted since they
            // were converted to UNO API references now, so they stay as long as
            // referenced. Do NOT delete the C++ implementation classes here, but clear
            // the buffer to not delete them in the destructor.
            aTargets.clear();

            if(xRetval.hasElements() && rPropertyHolder.getRegionActive())
            {
                const Region& rRegion = rPropertyHolder.getRegion();

                if(!rRegion.IsEmpty())
                {
                    basegfx::B2DPolyPolygon aClipPolyPolygon(getB2DPolyPolygonFromRegion(rRegion));

                    if(aClipPolyPolygon.count())
                    {
                        aClipPolyPolygon.transform(rPropertyHolder.getTransformation());

                        const drawinglayer::primitive2d::Primitive2DReference xMask(
                            new drawinglayer::primitive2d::MaskPrimitive2D(
                                aClipPolyPolygon,
                                xRetval));

                        xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xMask, 1);
                    }
                }
            }

            return xRetval;
        }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace
{
    class TargetHolders
    {
    private:
        std::vector< TargetHolder* >          maTargetHolders;

    public:
        TargetHolders()
        {
            maTargetHolders.push_back(new TargetHolder());
        }

        sal_uInt32 size()
        {
            return maTargetHolders.size();
        }

        void Push()
        {
            maTargetHolders.push_back(new TargetHolder());
        }

        void Pop()
        {
            OSL_ENSURE(maTargetHolders.size(), "TargetHolders: POP with no property holders (!)");
            if(maTargetHolders.size())
            {
                delete maTargetHolders[maTargetHolders.size() - 1];
                maTargetHolders.pop_back();
            }
        }

        TargetHolder& Current()
        {
            OSL_ENSURE(maTargetHolders.size(), "TargetHolders: CURRENT with no property holders (!)");
            return *maTargetHolders[maTargetHolders.size() - 1];
        }

        ~TargetHolders()
        {
            while(maTargetHolders.size())
            {
                Pop();
            }
        }
    };
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** NonOverlappingFillGradientPrimitive2D class

            This is a special version of the FillGradientPrimitive2D which decomposes
            to a non-overlapping geometry version of the gradient. This needs to be
            used to support the old XOR paint-'trick'.

            It does not need an own identifier since a renderer who wants to interpret
            it itself may do so. It just overloads the decomposition of the C++
            implementation class to do an alternative decomposition.
         */
        class NonOverlappingFillGradientPrimitive2D : public FillGradientPrimitive2D
        {
        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(
                const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            NonOverlappingFillGradientPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const attribute::FillGradientAttribute& rFillGradient)
            :   FillGradientPrimitive2D(rObjectRange, rFillGradient)
            {
            }
        };

        Primitive2DSequence NonOverlappingFillGradientPrimitive2D::create2DDecomposition(
            const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return createFill(false);
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace
{
    basegfx::B2DHomMatrix getTransformFromMapMode(const MapMode& rMapMode)
    {
        basegfx::B2DHomMatrix aMapping;
        const Fraction aNoScale(1, 1);
        const Point& rOrigin(rMapMode.GetOrigin());

        if(0 != rOrigin.X() || 0 != rOrigin.Y())
        {
            aMapping.translate(rOrigin.X(), rOrigin.Y());
        }

        if(rMapMode.GetScaleX() != aNoScale || rMapMode.GetScaleY() != aNoScale)
        {
            aMapping.scale(
                double(rMapMode.GetScaleX()),
                double(rMapMode.GetScaleY()));
        }

        return aMapping;
    }

    void createPointArrayPrimitive(
        const std::vector< basegfx::B2DPoint >& rPositions,
        TargetHolder& rTarget,
        PropertyHolder& rProperties,
        basegfx::BColor aBColor)
    {
        if(rPositions.size())
        {
            if(rProperties.getTransformation().isIdentity())
            {
                rTarget.append(
                    new drawinglayer::primitive2d::PointArrayPrimitive2D(
                        rPositions,
                        aBColor));
            }
            else
            {
                std::vector< basegfx::B2DPoint > aPositions(rPositions);

                for(sal_uInt32 a(0); a < aPositions.size(); a++)
                {
                    aPositions[a] = rProperties.getTransformation() * aPositions[a];
                }

                rTarget.append(
                    new drawinglayer::primitive2d::PointArrayPrimitive2D(
                        aPositions,
                        aBColor));
            }
        }
    }

    void createHairlinePrimitive(
        const basegfx::B2DPolygon& rLinePolygon,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(rLinePolygon.count())
        {
            basegfx::B2DPolygon aLinePolygon(rLinePolygon);
            aLinePolygon.transform(rProperties.getTransformation());
            rTarget.append(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aLinePolygon,
                    rProperties.getLineColor()));
        }
    }

    void createFillPrimitive(
        const basegfx::B2DPolyPolygon& rFillPolyPolygon,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(rFillPolyPolygon.count())
        {
            basegfx::B2DPolyPolygon aFillPolyPolygon(rFillPolyPolygon);
            aFillPolyPolygon.transform(rProperties.getTransformation());
            rTarget.append(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    aFillPolyPolygon,
                    rProperties.getFillColor()));
        }
    }

    void createLinePrimitive(
        const basegfx::B2DPolygon& rLinePolygon,
        const LineInfo& rLineInfo,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(rLinePolygon.count())
        {
            const bool bDashDotUsed(LINE_DASH == rLineInfo.GetStyle());
            const bool bWidthUsed(rLineInfo.GetWidth() > 1);

            if(bDashDotUsed || bWidthUsed)
            {
                basegfx::B2DPolygon aLinePolygon(rLinePolygon);
                aLinePolygon.transform(rProperties.getTransformation());
                const drawinglayer::attribute::LineAttribute aLineAttribute(
                    rProperties.getLineColor(),
                    bWidthUsed ? rLineInfo.GetWidth() : 0.0,
                    rLineInfo.GetLineJoin());

                if(bDashDotUsed)
                {
                    ::std::vector< double > fDotDashArray;
                    const double fDashLen(rLineInfo.GetDashLen());
                    const double fDotLen(rLineInfo.GetDotLen());
                    const double fDistance(rLineInfo.GetDistance());

                    for(sal_uInt16 a(0); a < rLineInfo.GetDashCount(); a++)
                    {
                        fDotDashArray.push_back(fDashLen);
                        fDotDashArray.push_back(fDistance);
                    }

                    for(sal_uInt16 b(0); b < rLineInfo.GetDotCount(); b++)
                    {
                        fDotDashArray.push_back(fDotLen);
                        fDotDashArray.push_back(fDistance);
                    }

                    const double fAccumulated(::std::accumulate(fDotDashArray.begin(), fDotDashArray.end(), 0.0));
                    const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
                        fDotDashArray,
                        fAccumulated);

                    rTarget.append(
                        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                            aLinePolygon,
                            aLineAttribute,
                            aStrokeAttribute));
                }
                else
                {
                    rTarget.append(
                        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
                            aLinePolygon,
                            aLineAttribute));
                }
            }
            else
            {
                createHairlinePrimitive(rLinePolygon, rTarget, rProperties);
            }
        }
    }

    void createHairlineAndFillPrimitive(
        const basegfx::B2DPolygon& rPolygon,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(rProperties.getFillColorActive())
        {
            createFillPrimitive(basegfx::B2DPolyPolygon(rPolygon), rTarget, rProperties);
        }

        if(rProperties.getLineColorActive())
        {
            createHairlinePrimitive(rPolygon, rTarget, rProperties);
        }
    }

    void createHairlineAndFillPrimitive(
        const basegfx::B2DPolyPolygon& rPolyPolygon,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(rProperties.getFillColorActive())
        {
            createFillPrimitive(rPolyPolygon, rTarget, rProperties);
        }

        if(rProperties.getLineColorActive())
        {
            for(sal_uInt32 a(0); a < rPolyPolygon.count(); a++)
            {
                createHairlinePrimitive(rPolyPolygon.getB2DPolygon(a), rTarget, rProperties);
            }
        }
    }

    void createBitmapExPrimitive(
        const BitmapEx& rBitmapEx,
        const Point& rPoint,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(!rBitmapEx.IsEmpty())
        {
            basegfx::B2DPoint aPoint(rPoint.X(), rPoint.Y());
            aPoint = rProperties.getTransformation() * aPoint;

            rTarget.append(
                new drawinglayer::primitive2d::DiscreteBitmapPrimitive2D(
                    rBitmapEx,
                    aPoint));
        }
    }

    void createBitmapExPrimitive(
        const BitmapEx& rBitmapEx,
        const Point& rPoint,
        const Size& rSize,
        TargetHolder& rTarget,
        PropertyHolder& rProperties)
    {
        if(!rBitmapEx.IsEmpty())
        {
            basegfx::B2DHomMatrix aObjectTransform;

            aObjectTransform.set(0, 0, rSize.Width());
            aObjectTransform.set(1, 1, rSize.Height());
            aObjectTransform.set(0, 2, rPoint.X());
            aObjectTransform.set(1, 2, rPoint.Y());

            aObjectTransform = rProperties.getTransformation() * aObjectTransform;

            rTarget.append(
                new drawinglayer::primitive2d::BitmapPrimitive2D(
                    rBitmapEx,
                    aObjectTransform));
        }
    }

    BitmapEx createMaskBmpEx(const Bitmap& rBitmap, const Color& rMaskColor)
    {
        const Color aWhite(COL_WHITE);
        BitmapPalette aBiLevelPalette(2);

        aBiLevelPalette[0] = aWhite;
        aBiLevelPalette[1] = rMaskColor;

        Bitmap aMask(rBitmap.CreateMask(aWhite));
        Bitmap aSolid(rBitmap.GetSizePixel(), 1, &aBiLevelPalette);

        aSolid.Erase(rMaskColor);

        return BitmapEx(aSolid, aMask);
    }

    drawinglayer::attribute::FillGradientAttribute createFillGradientAttribute(const Gradient& rGradient)
    {
        const Color aStartColor(rGradient.GetStartColor());
        const sal_uInt16 nStartIntens(rGradient.GetStartIntensity());
        basegfx::BColor aStart(aStartColor.getBColor());

        if(nStartIntens != 100)
        {
            const basegfx::BColor aBlack;
            aStart = interpolate(aBlack, aStart, (double)nStartIntens * 0.01);
        }

        const Color aEndColor(rGradient.GetEndColor());
        const sal_uInt16 nEndIntens(rGradient.GetEndIntensity());
        basegfx::BColor aEnd(aEndColor.getBColor());

        if(nEndIntens != 100)
        {
            const basegfx::BColor aBlack;
            aEnd = interpolate(aBlack, aEnd, (double)nEndIntens * 0.01);
        }

        drawinglayer::attribute::GradientStyle aGradientStyle(drawinglayer::attribute::GRADIENTSTYLE_RECT);

        switch(rGradient.GetStyle())
        {
            case GRADIENT_LINEAR :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_LINEAR;
                break;
            }
            case GRADIENT_AXIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_AXIAL;
                break;
            }
            case GRADIENT_RADIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RADIAL;
                break;
            }
            case GRADIENT_ELLIPTICAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_ELLIPTICAL;
                break;
            }
            case GRADIENT_SQUARE :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_SQUARE;
                break;
            }
            default : // GRADIENT_RECT
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RECT;
                break;
            }
        }

        return drawinglayer::attribute::FillGradientAttribute(
            aGradientStyle,
            (double)rGradient.GetBorder() * 0.01,
            (double)rGradient.GetOfsX() * 0.01,
            (double)rGradient.GetOfsY() * 0.01,
            (double)rGradient.GetAngle() * F_PI1800,
            aStart,
            aEnd,
            rGradient.GetSteps());
    }

    drawinglayer::attribute::FillHatchAttribute createFillHatchAttribute(const Hatch& rHatch)
    {
        drawinglayer::attribute::HatchStyle aHatchStyle(drawinglayer::attribute::HATCHSTYLE_SINGLE);

        switch(rHatch.GetStyle())
        {
            default : // case HATCH_SINGLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_SINGLE;
            }
            case HATCH_DOUBLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_DOUBLE;
            }
            case HATCH_TRIPLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_TRIPLE;
            }
        }

        return drawinglayer::attribute::FillHatchAttribute(
            aHatchStyle,
            (double)rHatch.GetDistance(),
            (double)rHatch.GetAngle() * F_PI1800,
            rHatch.GetColor().getBColor(),
            false);
    }

    void HandleNewClipRegion(
        const Region* pRegion,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders)
    {
        // process evtl. created primitives which belong to the current region settings
        if(rPropertyHolders.Current().getRegionActive() && rTargetHolders.size() > 1)
        {
            drawinglayer::primitive2d::Primitive2DSequence aSubContent;

            if(!rPropertyHolders.Current().getRegion().IsEmpty() && rTargetHolders.Current().size())
            {
                aSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
            }

            rTargetHolders.Pop();

            if(aSubContent.hasElements())
            {
                rTargetHolders.Current().append(
                    new drawinglayer::primitive2d::GroupPrimitive2D(
                        aSubContent));
            }
        }

        // apply new settings
        const bool bNewActive(pRegion);
        rPropertyHolders.Current().setRegionActive(bNewActive);

        if(bNewActive)
        {
            rPropertyHolders.Current().setRegion(*pRegion);

            // prepare new content holder for new active region
            rTargetHolders.Push();
        }
    }

    void HandleNewRasterOp(
        RasterOp aRasterOp,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders)
    {
        // check if currently active
        if(rPropertyHolders.Current().isRasterOpActive() && rTargetHolders.size() > 1)
        {
            drawinglayer::primitive2d::Primitive2DSequence aSubContent;

            if(rTargetHolders.Current().size())
            {
                aSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
            }

            rTargetHolders.Pop();

            if(aSubContent.hasElements())
            {
                if(rPropertyHolders.Current().isRasterOpForceBlack())
                {
                    // force content to black
                    rTargetHolders.Current().append(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                            aSubContent,
                            basegfx::BColorModifier(basegfx::BColor(0.0, 0.0, 0.0))));
                }
                else // if(rPropertyHolders.Current().isRasterOpInvert())
                {
                    // invert content
                    rTargetHolders.Current().append(
                        new drawinglayer::primitive2d::InvertPrimitive2D(
                            aSubContent));
                }
            }
        }

        // apply new settings
        rPropertyHolders.Current().setRasterOp(aRasterOp);

        // check if now active
        if(rPropertyHolders.Current().isRasterOpActive())
        {
            // prepare new content holder for new invert
            rTargetHolders.Push();
        }
    }

    drawinglayer::primitive2d::BasePrimitive2D* CreateColorWallpaper(
        const basegfx::B2DRange& rRange,
        const basegfx::BColor& rColor,
        PropertyHolder& rPropertyHolder)
    {
        basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(rRange));
        aOutline.transform(rPropertyHolder.getTransformation());

        return new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
            basegfx::B2DPolyPolygon(aOutline),
            rColor);
    }

    drawinglayer::primitive2d::BasePrimitive2D* CreateGradientWallpaper(
        const basegfx::B2DRange& rRange,
        const Gradient& rGradient,
        PropertyHolder& rPropertyHolder)
    {
        const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));

        if(aAttribute.getStartColor() == aAttribute.getEndColor())
        {
            // not really a gradient. Create filled rectangle
            return CreateColorWallpaper(rRange, aAttribute.getStartColor(), rPropertyHolder);
        }
        else
        {
            // really a gradient
            drawinglayer::primitive2d::BasePrimitive2D* pRetval =
                new drawinglayer::primitive2d::FillGradientPrimitive2D(
                    rRange,
                    aAttribute);

            if(!rPropertyHolder.getTransformation().isIdentity())
            {
                const drawinglayer::primitive2d::Primitive2DReference xPrim(pRetval);
                const drawinglayer::primitive2d::Primitive2DSequence xSeq(&xPrim, 1);

                pRetval = new drawinglayer::primitive2d::TransformPrimitive2D(
                    rPropertyHolder.getTransformation(),
                    xSeq);
            }

            return pRetval;
        }
    }

    void CreateAndAppendBitmapWallpaper(
        basegfx::B2DRange aWallpaperRange,
        const Wallpaper& rWallpaper,
        TargetHolder& rTarget,
        PropertyHolder& rProperty)
    {
        const BitmapEx aBitmapEx(rWallpaper.GetBitmap());
        const WallpaperStyle eWallpaperStyle(rWallpaper.GetStyle());

        // if bitmap visualisation is transparent, maybe background
        // needs to be filled. Create background
        if(aBitmapEx.IsTransparent()
            || (WALLPAPER_TILE != eWallpaperStyle && WALLPAPER_SCALE != eWallpaperStyle))
        {
            if(rWallpaper.IsGradient())
            {
                rTarget.append(
                    CreateGradientWallpaper(
                        aWallpaperRange,
                        rWallpaper.GetGradient(),
                        rProperty));
            }
            else if(!rWallpaper.GetColor().GetTransparency())
            {
                rTarget.append(
                    CreateColorWallpaper(
                        aWallpaperRange,
                        rWallpaper.GetColor().getBColor(),
                        rProperty));
            }
        }

        // use wallpaper rect if set
        if(rWallpaper.IsRect() && !rWallpaper.GetRect().IsEmpty())
        {
            aWallpaperRange = basegfx::B2DRange(
                rWallpaper.GetRect().Left(), rWallpaper.GetRect().Top(),
                rWallpaper.GetRect().Right(), rWallpaper.GetRect().Bottom());
        }

        drawinglayer::primitive2d::BasePrimitive2D* pBitmapWallpaperFill =
            new drawinglayer::primitive2d::WallpaperBitmapPrimitive2D(
                aWallpaperRange,
                aBitmapEx,
                eWallpaperStyle);

        if(rProperty.getTransformation().isIdentity())
        {
            // add directly
            rTarget.append(pBitmapWallpaperFill);
        }
        else
        {
            // when a transformation is set, embed to it
            const drawinglayer::primitive2d::Primitive2DReference xPrim(pBitmapWallpaperFill);

            rTarget.append(
                new drawinglayer::primitive2d::TransformPrimitive2D(
                    rProperty.getTransformation(),
                    drawinglayer::primitive2d::Primitive2DSequence(&xPrim, 1)));
        }
    }

    drawinglayer::primitive2d::TextLine mapTextLineStyle(FontUnderline eLineStyle)
    {
        switch(eLineStyle)
        {
            case UNDERLINE_SINGLE:          return drawinglayer::primitive2d::TEXT_LINE_SINGLE;
            case UNDERLINE_DOUBLE:          return drawinglayer::primitive2d::TEXT_LINE_DOUBLE;
            case UNDERLINE_DOTTED:          return drawinglayer::primitive2d::TEXT_LINE_DOTTED;
            case UNDERLINE_DASH:            return drawinglayer::primitive2d::TEXT_LINE_DASH;
            case UNDERLINE_LONGDASH:        return drawinglayer::primitive2d::TEXT_LINE_LONGDASH;
            case UNDERLINE_DASHDOT:         return drawinglayer::primitive2d::TEXT_LINE_DASHDOT;
            case UNDERLINE_DASHDOTDOT:      return drawinglayer::primitive2d::TEXT_LINE_DASHDOTDOT;
            case UNDERLINE_SMALLWAVE:       return drawinglayer::primitive2d::TEXT_LINE_SMALLWAVE;
            case UNDERLINE_WAVE:            return drawinglayer::primitive2d::TEXT_LINE_WAVE;
            case UNDERLINE_DOUBLEWAVE:      return drawinglayer::primitive2d::TEXT_LINE_DOUBLEWAVE;
            case UNDERLINE_BOLD:            return drawinglayer::primitive2d::TEXT_LINE_BOLD;
            case UNDERLINE_BOLDDOTTED:      return drawinglayer::primitive2d::TEXT_LINE_BOLDDOTTED;
            case UNDERLINE_BOLDDASH:        return drawinglayer::primitive2d::TEXT_LINE_BOLDDASH;
            case UNDERLINE_BOLDLONGDASH:    return drawinglayer::primitive2d::TEXT_LINE_BOLDLONGDASH;
            case UNDERLINE_BOLDDASHDOT:     return drawinglayer::primitive2d::TEXT_LINE_BOLDDASHDOT;
            case UNDERLINE_BOLDDASHDOTDOT:  return drawinglayer::primitive2d::TEXT_LINE_BOLDDASHDOTDOT;
            case UNDERLINE_BOLDWAVE:        return drawinglayer::primitive2d::TEXT_LINE_BOLDWAVE;
            // FontUnderline_FORCE_EQUAL_SIZE, UNDERLINE_DONTKNOW, UNDERLINE_NONE
            default:                        return drawinglayer::primitive2d::TEXT_LINE_NONE;
        }
    }

    bool isUnderlineAbove(const Font& rFont)
    {
        if(!rFont.IsVertical())
        {
            return false;
        }

        if((LANGUAGE_JAPANESE == rFont.GetLanguage()) || (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()))
        {
            // the underline is right for Japanese only
            return true;
        }

        return false;
    }

    void proccessMetaTextAction(
        const Point& rTextStartPosition,
        const XubString& rText,
        sal_uInt16 nTextStart,
        sal_uInt16 nTextLength,
        sal_Int32* pDXArray,
        TargetHolder& rTarget,
        PropertyHolder& rProperty)
    {
        drawinglayer::primitive2d::BasePrimitive2D* pResult = 0;
        const Font& rFont = rProperty.getFont();
        std::vector< double > aDXArray;

        if(nTextLength)
        {
            // get current font and create FontScaling and FontAttribute
            basegfx::B2DVector aFontScaling;
            const drawinglayer::attribute::FontAttribute aFontAttribute(
                drawinglayer::primitive2d::getFontAttributeFromVclFont(
                    aFontScaling,
                    rFont,
                    0 != (rProperty.getLayoutMode() & TEXT_LAYOUT_BIDI_RTL),
                    0 != (rProperty.getLayoutMode() & TEXT_LAYOUT_BIDI_STRONG)));

            // create TextTransform
            basegfx::B2DHomMatrix aTextTransform;

            // add FontScaling
            aTextTransform.scale(aFontScaling.getX(), aFontScaling.getY());

            // take text align into account
            if(ALIGN_BASELINE != rFont.GetAlign())
            {
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                aTextLayouterDevice.setFont(rFont);

                if(ALIGN_TOP == rFont.GetAlign())
                {
                    aTextTransform.translate(
                        0.0,
                        aTextLayouterDevice.getFontAscent());
                }
                else // ALIGN_BOTTOM
                {
                    aTextTransform.translate(
                        0.0,
                        -aTextLayouterDevice.getFontDescent());
                }
            }

            // add FontRotation (if used)
            if(rFont.GetOrientation())
            {
                aTextTransform.rotate(-rFont.GetOrientation() * F_PI1800);
            }

            // add TextStartPosition
            aTextTransform.translate(rTextStartPosition.X(), rTextStartPosition.Y());

            // preapare DXArray (if used)
            if(pDXArray && nTextLength)
            {
                aDXArray.reserve(nTextLength);

                for(xub_StrLen a(0); a < nTextLength; a++)
                {
                    aDXArray.push_back((double)(*(pDXArray + a)));
                }
            }

            // prepare FontColor and Locale
            const basegfx::BColor aFontColor(rProperty.getTextColor());
            const com::sun::star::lang::Locale aLocale(MsLangId::convertLanguageToLocale(rProperty.getLanguageType()));
            const bool bWordLineMode(rFont.IsWordLineMode());

            const bool bDecoratedIsNeeded(
                   UNDERLINE_NONE != rFont.GetOverline()
                || UNDERLINE_NONE != rFont.GetUnderline()
                || STRIKEOUT_NONE != rFont.GetStrikeout()
                || EMPHASISMARK_NONE != (rFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                || RELIEF_NONE != rFont.GetRelief()
                || rFont.IsShadow()
                || bWordLineMode);

            if(bDecoratedIsNeeded)
            {
                // prepare overline and underline data
                const drawinglayer::primitive2d::TextLine eFontOverline(mapTextLineStyle(rFont.GetOverline()));
                const drawinglayer::primitive2d::TextLine eFontUnderline(mapTextLineStyle(rFont.GetUnderline()));

                // check UndelineAbove
                const bool bUnderlineAbove(drawinglayer::primitive2d::TEXT_LINE_NONE != eFontUnderline && isUnderlineAbove(rFont));

                // prepare strikeout data
                drawinglayer::primitive2d::FontStrikeout eFontStrikeout(drawinglayer::primitive2d::FONT_STRIKEOUT_NONE);

                switch(rFont.GetStrikeout())
                {
                    case STRIKEOUT_SINGLE:  eFontStrikeout = drawinglayer::primitive2d::FONT_STRIKEOUT_SINGLE; break;
                    case STRIKEOUT_DOUBLE:  eFontStrikeout = drawinglayer::primitive2d::FONT_STRIKEOUT_DOUBLE; break;
                    case STRIKEOUT_BOLD:    eFontStrikeout = drawinglayer::primitive2d::FONT_STRIKEOUT_BOLD; break;
                    case STRIKEOUT_SLASH:   eFontStrikeout = drawinglayer::primitive2d::FONT_STRIKEOUT_SLASH; break;
                    case STRIKEOUT_X:       eFontStrikeout = drawinglayer::primitive2d::FONT_STRIKEOUT_X; break;
                    default : break; // FontStrikeout_FORCE_EQUAL_SIZE, STRIKEOUT_NONE, STRIKEOUT_DONTKNOW
                }

                // prepare emphasis mark data
                drawinglayer::primitive2d::FontEmphasisMark eFontEmphasisMark(drawinglayer::primitive2d::FONT_EMPHASISMARK_NONE);

                switch(rFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                {
                    case EMPHASISMARK_DOT : eFontEmphasisMark = drawinglayer::primitive2d::FONT_EMPHASISMARK_DOT; break;
                    case EMPHASISMARK_CIRCLE : eFontEmphasisMark = drawinglayer::primitive2d::FONT_EMPHASISMARK_CIRCLE; break;
                    case EMPHASISMARK_DISC : eFontEmphasisMark = drawinglayer::primitive2d::FONT_EMPHASISMARK_DISC; break;
                    case EMPHASISMARK_ACCENT : eFontEmphasisMark = drawinglayer::primitive2d::FONT_EMPHASISMARK_ACCENT; break;
                }

                const bool bEmphasisMarkAbove(rFont.GetEmphasisMark() & EMPHASISMARK_POS_ABOVE);
                const bool bEmphasisMarkBelow(rFont.GetEmphasisMark() & EMPHASISMARK_POS_BELOW);

                // prepare font relief data
                drawinglayer::primitive2d::FontRelief eFontRelief(drawinglayer::primitive2d::FONT_RELIEF_NONE);

                switch(rFont.GetRelief())
                {
                    case RELIEF_EMBOSSED : eFontRelief = drawinglayer::primitive2d::FONT_RELIEF_EMBOSSED; break;
                    case RELIEF_ENGRAVED : eFontRelief = drawinglayer::primitive2d::FONT_RELIEF_ENGRAVED; break;
                    default : break; // RELIEF_NONE, FontRelief_FORCE_EQUAL_SIZE
                }

                // prepare shadow/outline data
                const bool bShadow(rFont.IsShadow());

                // TextDecoratedPortionPrimitive2D is needed, create one
                pResult = new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(

                    // attributes for TextSimplePortionPrimitive2D
                    aTextTransform,
                    rText,
                    nTextStart,
                    nTextLength,
                    aDXArray,
                    aFontAttribute,
                    aLocale,
                    aFontColor,

                    // attributes for TextDecoratedPortionPrimitive2D
                    rProperty.getOverlineColorActive() ? rProperty.getOverlineColor() : aFontColor,
                    rProperty.getTextLineColorActive() ? rProperty.getTextLineColor() : aFontColor,
                    eFontOverline,
                    eFontUnderline,
                    bUnderlineAbove,
                    eFontStrikeout,
                    bWordLineMode,
                    eFontEmphasisMark,
                    bEmphasisMarkAbove,
                    bEmphasisMarkBelow,
                    eFontRelief,
                    bShadow);
            }
            else
            {
                // TextSimplePortionPrimitive2D is enough
                pResult = new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                    aTextTransform,
                    rText,
                    nTextStart,
                    nTextLength,
                    aDXArray,
                    aFontAttribute,
                    aLocale,
                    aFontColor);
            }
        }

        if(pResult && rProperty.getTextFillColorActive())
        {
            drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
            const bool bRotated(rFont.GetOrientation());

            if(bRotated)
            {
                // use unrotated font
                Font aUnrotatedFont(rFont);
                aUnrotatedFont.SetOrientation(0);
                aTextLayouterDevice.setFont(aUnrotatedFont);
            }
            else
            {
                aTextLayouterDevice.setFont(rFont);
            }

            // get base range
            basegfx::B2DRange aTextRange(
                aTextLayouterDevice.getTextBoundRect(
                    rText, nTextStart, nTextLength));

            if(aDXArray.size())
            {
                // use the last entry in DXArray to correct the width
                aTextRange = basegfx::B2DRange(
                    aTextRange.getMinX(),
                    aTextRange.getMinY(),
                    aTextRange.getMinX() + aDXArray[aDXArray.size() - 1],
                    aTextRange.getMaxY());
            }

            // create Transform. Scale and Alignment are already applied.
            basegfx::B2DHomMatrix aTextTransform;

            if(bRotated)
            {
                aTextTransform.rotate(-rFont.GetOrientation() * F_PI1800);
            }

            aTextTransform.translate(rTextStartPosition.X(), rTextStartPosition.Y());

            // prepare Primitive2DSequence, put text in foreground
            drawinglayer::primitive2d::Primitive2DSequence aSequence(2);
            aSequence[1] = drawinglayer::primitive2d::Primitive2DReference(pResult);

            // prepare filled polygon
            basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aTextRange));
            aOutline.transform(aTextTransform);

            aSequence[0] = drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aOutline),
                    rProperty.getTextFillColor()));

            // set as group at pResult
            pResult = new drawinglayer::primitive2d::GroupPrimitive2D(aSequence);
        }

        if(pResult)
        {
            // add created text primitive to target
            if(rProperty.getTransformation().isIdentity())
            {
                rTarget.append(pResult);
            }
            else
            {
                // when a transformation is set, embed to it
                const drawinglayer::primitive2d::Primitive2DReference aReference(pResult);

                rTarget.append(
                    new drawinglayer::primitive2d::TransformPrimitive2D(
                        rProperty.getTransformation(),
                        drawinglayer::primitive2d::Primitive2DSequence(&aReference, 1)));
            }
        }
    }

    void interpretMetafile(
        const GDIMetaFile& rMetaFile,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders,
        const drawinglayer::geometry::ViewInformation2D& rViewInformation)
    {
        const sal_uInt32 nCount(rMetaFile.GetActionCount());

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            MetaAction* pAction = rMetaFile.GetAction(a);

            switch(pAction->GetType())
            {
                case META_NULL_ACTION :
                {
                    /** SIMPLE, DONE */
                    break;
                }
                case META_PIXEL_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    std::vector< basegfx::B2DPoint > aPositions;
                    Color aLastColor(COL_BLACK);

                    while(META_PIXEL_ACTION == pAction->GetType() && a < nCount)
                    {
                        const MetaPixelAction* pA = (const MetaPixelAction*)pAction;

                        if(pA->GetColor() != aLastColor)
                        {
                            if(aPositions.size())
                            {
                                createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), aLastColor.getBColor());
                                aPositions.clear();
                            }

                            aLastColor = pA->GetColor();
                        }

                        const Point& rPoint = pA->GetPoint();
                        aPositions.push_back(basegfx::B2DPoint(rPoint.X(), rPoint.Y()));
                        a++; if(a < nCount) pAction = rMetaFile.GetAction(a);
                    }

                    a--;

                    if(aPositions.size())
                    {
                        createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), aLastColor.getBColor());
                    }

                    break;
                }
                case META_POINT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        std::vector< basegfx::B2DPoint > aPositions;

                        while(META_POINT_ACTION == pAction->GetType() && a < nCount)
                        {
                            const MetaPointAction* pA = (const MetaPointAction*)pAction;
                            const Point& rPoint = pA->GetPoint();
                            aPositions.push_back(basegfx::B2DPoint(rPoint.X(), rPoint.Y()));
                            a++; if(a < nCount) pAction = rMetaFile.GetAction(a);
                        }

                        a--;

                        if(aPositions.size())
                        {
                            createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), rPropertyHolders.Current().getLineColor());
                        }
                    }

                    break;
                }
                case META_LINE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        basegfx::B2DPolygon aLinePolygon;
                        LineInfo aLineInfo;

                        while(META_LINE_ACTION == pAction->GetType() && a < nCount)
                        {
                            const MetaLineAction* pA = (const MetaLineAction*)pAction;
                            const Point& rStartPoint = pA->GetStartPoint();
                            const Point& rEndPoint = pA->GetEndPoint();
                            const basegfx::B2DPoint aStart(rStartPoint.X(), rStartPoint.Y());
                            const basegfx::B2DPoint aEnd(rEndPoint.X(), rEndPoint.Y());

                            if(aLinePolygon.count())
                            {
                                if(pA->GetLineInfo() == aLineInfo
                                    && aStart == aLinePolygon.getB2DPoint(aLinePolygon.count() - 1))
                                {
                                    aLinePolygon.append(aEnd);
                                }
                                else
                                {
                                    aLineInfo.SetLineJoin(basegfx::B2DLINEJOIN_NONE); // It were lines; force to NONE
                                    createLinePrimitive(aLinePolygon, aLineInfo, rTargetHolders.Current(), rPropertyHolders.Current());
                                    aLinePolygon.clear();
                                    aLineInfo = pA->GetLineInfo();
                                    aLinePolygon.append(aStart);
                                    aLinePolygon.append(aEnd);
                                }
                            }
                            else
                            {
                                aLineInfo = pA->GetLineInfo();
                                aLinePolygon.append(aStart);
                                aLinePolygon.append(aEnd);
                            }

                            a++; if(a < nCount) pAction = rMetaFile.GetAction(a);
                        }

                        a--;

                        if(aLinePolygon.count())
                        {
                            aLineInfo.SetLineJoin(basegfx::B2DLINEJOIN_NONE); // It were lines; force to NONE
                            createLinePrimitive(aLinePolygon, aLineInfo, rTargetHolders.Current(), rPropertyHolders.Current());
                        }
                    }

                    break;
                }
                case META_RECT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaRectAction* pA = (const MetaRectAction*)pAction;
                        const Rectangle& rRectangle = pA->GetRect();

                        if(!rRectangle.IsEmpty())
                        {
                            const basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                            if(!aRange.isEmpty())
                            {
                                const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                                createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                            }
                        }
                    }

                    break;
                }
                case META_ROUNDRECT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    /** The original OutputDevice::DrawRect paints nothing when nHor or nVer is zero; but just
                        because the tools::Polygon operator creating the rounding does produce nonsense. I assume
                        this an error and create an unrounded rectangle in that case (implicit in
                        createPolygonFromRect)
                     */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaRoundRectAction* pA = (const MetaRoundRectAction*)pAction;
                        const Rectangle& rRectangle = pA->GetRect();

                        if(!rRectangle.IsEmpty())
                        {
                            const basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                            if(!aRange.isEmpty())
                            {
                                const sal_uInt32 nHor(pA->GetHorzRound());
                                const sal_uInt32 nVer(pA->GetVertRound());
                                basegfx::B2DPolygon aOutline;

                                if(nHor || nVer)
                                {
                                    double fRadiusX((nHor * 2.0) / (aRange.getWidth() > 0.0 ? aRange.getWidth() : 1.0));
                                    double fRadiusY((nVer * 2.0) / (aRange.getHeight() > 0.0 ? aRange.getHeight() : 1.0));
                                    fRadiusX = std::max(0.0, std::min(1.0, fRadiusX));
                                    fRadiusY = std::max(0.0, std::min(1.0, fRadiusY));

                                    aOutline = basegfx::tools::createPolygonFromRect(aRange, fRadiusX, fRadiusY);
                                }
                                else
                                {
                                    aOutline = basegfx::tools::createPolygonFromRect(aRange);
                                }

                                createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                            }
                        }
                    }

                    break;
                }
                case META_ELLIPSE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaEllipseAction* pA = (const MetaEllipseAction*)pAction;
                        const Rectangle& rRectangle = pA->GetRect();

                        if(!rRectangle.IsEmpty())
                        {
                            const basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                            if(!aRange.isEmpty())
                            {
                                const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromEllipse(
                                    aRange.getCenter(), aRange.getWidth() * 0.5, aRange.getHeight() * 0.5));

                                createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                            }
                        }
                    }

                    break;
                }
                case META_ARC_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        const MetaArcAction* pA = (const MetaArcAction*)pAction;
                        const Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlinePrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_PIE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPieAction* pA = (const MetaPieAction*)pAction;
                        const Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_CHORD_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaChordAction* pA = (const MetaChordAction*)pAction;
                        const Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_POLYLINE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        const MetaPolyLineAction* pA = (const MetaPolyLineAction*)pAction;
                        createLinePrimitive(pA->GetPolygon().getB2DPolygon(), pA->GetLineInfo(), rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_POLYGON_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPolygonAction* pA = (const MetaPolygonAction*)pAction;
                        basegfx::B2DPolygon aOutline(pA->GetPolygon().getB2DPolygon());

                        // the metafile play interprets the polygons from MetaPolygonAction
                        // always as closed and always paints an edge from last to first point,
                        // so force to closed here to emulate that
                        if(aOutline.count() > 1 && !aOutline.isClosed())
                        {
                            aOutline.setClosed(true);
                        }

                        createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_POLYPOLYGON_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*)pAction;
                        basegfx::B2DPolyPolygon aPolyPolygonOutline(pA->GetPolyPolygon().getB2DPolyPolygon());

                        // the metafile play interprets the single polygons from MetaPolyPolygonAction
                        // always as closed and always paints an edge from last to first point,
                        // so force to closed here to emulate that
                        for(sal_uInt32 a(0); a < aPolyPolygonOutline.count(); a++)
                        {
                            basegfx::B2DPolygon aPolygonOutline(aPolyPolygonOutline.getB2DPolygon(a));

                            if(aPolygonOutline.count() > 1 && !aPolygonOutline.isClosed())
                            {
                                aPolygonOutline.setClosed(true);
                                aPolyPolygonOutline.setB2DPolygon(a, aPolygonOutline);
                            }
                        }

                        createHairlineAndFillPrimitive(aPolyPolygonOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_TEXT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTextAction* pA = (const MetaTextAction*)pAction;

                    if(pA->GetLen() && rPropertyHolders.Current().getTextColorActive())
                    {
                        proccessMetaTextAction(
                            pA->GetPoint(),
                            pA->GetText(),
                            pA->GetIndex(),
                            pA->GetLen(),
                            0,
                            rTargetHolders.Current(),
                            rPropertyHolders.Current());
                    }

                    break;
                }
                case META_TEXTARRAY_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*)pAction;

                    if(pA->GetLen() && rPropertyHolders.Current().getTextColorActive())
                    {
                        proccessMetaTextAction(
                            pA->GetPoint(),
                            pA->GetText(),
                            pA->GetIndex(),
                            pA->GetLen(),
                            pA->GetDXArray(),
                            rTargetHolders.Current(),
                            rPropertyHolders.Current());
                    }

                    break;
                }
                case META_STRETCHTEXT_ACTION :
                {
                    /** NEEDS IMPLEMENTATION */
                    OSL_ENSURE(false, "META_STRETCHTEXT_ACTION requested (!)");
                    // use OutputDevice::GetTextArray() to map the...
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction*)pAction;
                    break;
                }
                case META_TEXTRECT_ACTION :
                {
                    /** NEEDS IMPLEMENTATION */
                    OSL_ENSURE(false, "META_TEXTRECT_ACTION requested (!)");
                    const MetaTextRectAction* pA = (const MetaTextRectAction*)pAction;
                    break;
                }
                case META_BMP_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpAction* pA = (const MetaBmpAction*)pAction;
                    const BitmapEx aBitmapEx(pA->GetBitmap());

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_BMPSCALE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*)pAction;
                    const Bitmap aBitmapEx(pA->GetBitmap());

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_BMPSCALEPART_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*)pAction;
                    const Bitmap& rBitmap = pA->GetBitmap();

                    if(!rBitmap.IsEmpty())
                    {
                        Bitmap aCroppedBitmap(rBitmap);
                        const Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

                        if(!aCropRectangle.IsEmpty())
                        {
                            aCroppedBitmap.Crop(aCropRectangle);
                        }

                        const BitmapEx aCroppedBitmapEx(aCroppedBitmap);
                        createBitmapExPrimitive(aCroppedBitmapEx, pA->GetDestPoint(), pA->GetDestSize(), rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_BMPEX_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMP_ACTION */
                    const MetaBmpExAction* pA = (const MetaBmpExAction*)pAction;
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    createBitmapExPrimitive(rBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_BMPEXSCALE_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMPSCALE_ACTION */
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*)pAction;
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    createBitmapExPrimitive(rBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_BMPEXSCALEPART_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMPSCALEPART_ACTION */
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*)pAction;
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    if(!rBitmapEx.IsEmpty())
                    {
                        BitmapEx aCroppedBitmapEx(rBitmapEx);
                        const Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

                        if(!aCropRectangle.IsEmpty())
                        {
                            aCroppedBitmapEx.Crop(aCropRectangle);
                        }

                        createBitmapExPrimitive(aCroppedBitmapEx, pA->GetDestPoint(), pA->GetDestSize(), rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_MASK_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMP_ACTION */
                    const MetaMaskAction* pA = (const MetaMaskAction*)pAction;
                    const BitmapEx aBitmapEx(createMaskBmpEx(pA->GetBitmap(), pA->GetColor()));

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_MASKSCALE_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMPSCALE_ACTION */
                    const MetaMaskScaleAction* pA = (const MetaMaskScaleAction*)pAction;
                    const BitmapEx aBitmapEx(createMaskBmpEx(pA->GetBitmap(), pA->GetColor()));

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case META_MASKSCALEPART_ACTION :
                {
                    /** CHECKED, WORKS WELL: Simply same as META_BMPSCALEPART_ACTION */
                    const MetaMaskScalePartAction* pA = (const MetaMaskScalePartAction*)pAction;
                    const Bitmap& rBitmap = pA->GetBitmap();

                    if(!rBitmap.IsEmpty())
                    {
                        Bitmap aCroppedBitmap(rBitmap);
                        const Rectangle aCropRectangle(pA->GetSrcPoint(), pA->GetSrcSize());

                        if(!aCropRectangle.IsEmpty())
                        {
                            aCroppedBitmap.Crop(aCropRectangle);
                        }

                        const BitmapEx aCroppedBitmapEx(createMaskBmpEx(aCroppedBitmap, pA->GetColor()));
                        createBitmapExPrimitive(aCroppedBitmapEx, pA->GetDestPoint(), pA->GetDestSize(), rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case META_GRADIENT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaGradientAction* pA = (const MetaGradientAction*)pAction;
                    const Rectangle& rRectangle = pA->GetRect();

                    if(!rRectangle.IsEmpty())
                    {
                        basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                        if(!aRange.isEmpty())
                        {
                            const Gradient& rGradient = pA->GetGradient();
                            const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));

                            if(aAttribute.getStartColor() == aAttribute.getEndColor())
                            {
                                // not really a gradient. Create filled rectangle
                                const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));
                                createFillPrimitive(basegfx::B2DPolyPolygon(aOutline), rTargetHolders.Current(), rPropertyHolders.Current());
                            }
                            else
                            {
                                // really a gradient
                                aRange.transform(rPropertyHolders.Current().getTransformation());

                                if(rPropertyHolders.Current().isRasterOpInvert())
                                {
                                    // use a special version of FillGradientPrimitive2D which creates
                                    // non-overlapping geometry on decomposition to makethe old XOR
                                    // paint 'trick' work.
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::NonOverlappingFillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));
                                }
                                else
                                {
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::FillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));
                                }
                            }
                        }
                    }

                    break;
                }
                case META_HATCH_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaHatchAction* pA = (const MetaHatchAction*)pAction;
                    basegfx::B2DPolyPolygon aOutline(pA->GetPolyPolygon().getB2DPolyPolygon());

                    if(aOutline.count())
                    {
                        const Hatch& rHatch = pA->GetHatch();
                        const drawinglayer::attribute::FillHatchAttribute aAttribute(createFillHatchAttribute(rHatch));

                        aOutline.transform(rPropertyHolders.Current().getTransformation());

                        const basegfx::B2DRange aObjectRange(aOutline.getB2DRange());
                        const drawinglayer::primitive2d::Primitive2DReference aFillHatch(
                            new drawinglayer::primitive2d::FillHatchPrimitive2D(
                                aObjectRange,
                                basegfx::BColor(),
                                aAttribute));

                        rTargetHolders.Current().append(
                            new drawinglayer::primitive2d::MaskPrimitive2D(
                                aOutline,
                                drawinglayer::primitive2d::Primitive2DSequence(&aFillHatch, 1)));
                    }

                    break;
                }
                case META_WALLPAPER_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaWallpaperAction* pA = (const MetaWallpaperAction*)pAction;
                    Rectangle aWallpaperRectangle(pA->GetRect());

                    if(!aWallpaperRectangle.IsEmpty())
                    {
                        const Wallpaper& rWallpaper = pA->GetWallpaper();
                           const WallpaperStyle eWallpaperStyle(rWallpaper.GetStyle());
                        basegfx::B2DRange aWallpaperRange(
                            aWallpaperRectangle.Left(), aWallpaperRectangle.Top(),
                            aWallpaperRectangle.Right(), aWallpaperRectangle.Bottom());

                        if(WALLPAPER_NULL != eWallpaperStyle)
                        {
                            if(rWallpaper.IsBitmap())
                            {
                                // create bitmap background
                                CreateAndAppendBitmapWallpaper(
                                    aWallpaperRange,
                                    rWallpaper,
                                    rTargetHolders.Current(),
                                    rPropertyHolders.Current());
                            }
                            else if(rWallpaper.IsGradient())
                            {
                                // create gradient background
                                rTargetHolders.Current().append(
                                    CreateGradientWallpaper(
                                        aWallpaperRange,
                                        rWallpaper.GetGradient(),
                                        rPropertyHolders.Current()));
                            }
                            else if(!rWallpaper.GetColor().GetTransparency())
                            {
                                // create color background
                                rTargetHolders.Current().append(
                                    CreateColorWallpaper(
                                        aWallpaperRange,
                                        rWallpaper.GetColor().getBColor(),
                                        rPropertyHolders.Current()));
                            }
                        }
                    }

                    break;
                }
                case META_CLIPREGION_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaClipRegionAction* pA = (const MetaClipRegionAction*)pAction;

                    if(pA->IsClipping())
                    {
                        // new clipping
                        HandleNewClipRegion(&pA->GetRegion(), rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        // end clipping
                        HandleNewClipRegion(0, rTargetHolders, rPropertyHolders);
                    }

                    break;
                }
                case META_ISECTRECTCLIPREGION_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*)pAction;
                    const Rectangle& rRectangle = pA->GetRect();

                    if(rRectangle.IsEmpty())
                    {
                        // intersect with empty rectangle will always give empty
                        // region; start new clipping with empty region
                        const Region aNewRegion;
                        HandleNewClipRegion(&aNewRegion, rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        if(rPropertyHolders.Current().getRegionActive())
                        {
                            if(rPropertyHolders.Current().getRegion().IsEmpty())
                            {
                                // nothing to do, empty active clip region will stay
                                // empty when intersecting
                            }
                            else
                            {
                                // AND existing region and new rectangle
                                const basegfx::B2DPolyPolygon aOriginalPolyPolygon(
                                    getB2DPolyPolygonFromRegion(rPropertyHolders.Current().getRegion()));
                                basegfx::B2DPolyPolygon aClippedPolyPolygon;

                                if(aOriginalPolyPolygon.count())
                                {
                                    const basegfx::B2DRange aIntersectRange(
                                        rRectangle.Left(), rRectangle.Top(),
                                        rRectangle.Right(), rRectangle.Bottom());

                                    aClippedPolyPolygon = basegfx::tools::clipPolyPolygonOnRange(
                                        aOriginalPolyPolygon, aIntersectRange, true, false);
                                }

                                if(aClippedPolyPolygon != aOriginalPolyPolygon)
                                {
                                    // start new clipping with intersected region
                                    const Region aNewRegion(aClippedPolyPolygon);
                                    HandleNewClipRegion(&aNewRegion, rTargetHolders, rPropertyHolders);
                                }
                            }
                        }
                        else
                        {
                            // start new clipping with rectangle
                            const Region aNewRegion(rRectangle);
                            HandleNewClipRegion(&aNewRegion, rTargetHolders, rPropertyHolders);
                        }
                    }

                    break;
                }
                case META_ISECTREGIONCLIPREGION_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*)pAction;
                    const Region& rNewRegion = pA->GetRegion();

                    if(rNewRegion.IsEmpty())
                    {
                        // intersect with empty region will always give empty
                        // region; start new clipping with empty region
                        const Region aNewRegion;
                        HandleNewClipRegion(&aNewRegion, rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        if(rPropertyHolders.Current().getRegionActive())
                        {
                            if(rPropertyHolders.Current().getRegion().IsEmpty())
                            {
                                // nothing to do, empty active clip region will stay empty
                                // when intersecting with any region
                            }
                            else
                            {
                                // AND existing and new region
                                const basegfx::B2DPolyPolygon aOriginalPolyPolygon(
                                    getB2DPolyPolygonFromRegion(rPropertyHolders.Current().getRegion()));
                                basegfx::B2DPolyPolygon aClippedPolyPolygon;

                                if(aOriginalPolyPolygon.count())
                                {
                                    const basegfx::B2DPolyPolygon aClipPolyPolygon(
                                        getB2DPolyPolygonFromRegion(rNewRegion));

                                    if(aClipPolyPolygon.count())
                                    {
                                        aClippedPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                                            aOriginalPolyPolygon, aClipPolyPolygon, true, false);
                                    }
                                }

                                if(aClippedPolyPolygon != aOriginalPolyPolygon)
                                {
                                    // start new clipping with intersected region
                                    const Region aNewRegion(aClippedPolyPolygon);
                                    HandleNewClipRegion(&aNewRegion, rTargetHolders, rPropertyHolders);
                                }
                            }
                        }
                        else
                        {
                            // start new clipping with new region
                            HandleNewClipRegion(&rNewRegion, rTargetHolders, rPropertyHolders);
                        }
                    }

                    break;
                }
                case META_MOVECLIPREGION_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*)pAction;

                    if(rPropertyHolders.Current().getRegionActive())
                    {
                        if(rPropertyHolders.Current().getRegion().IsEmpty())
                        {
                            // nothing to do
                        }
                        else
                        {
                            // move using old interface
                            Region aRegion(rPropertyHolders.Current().getRegion());

                            const sal_Int32 nHor(pA->GetHorzMove());
                            const sal_Int32 nVer(pA->GetVertMove());

                            if(0 != nHor || 0 != nVer)
                            {
                                aRegion.Move(nHor, nVer);
                                HandleNewClipRegion(&aRegion, rTargetHolders, rPropertyHolders);
                            }
                        }
                    }

                    break;
                }
                case META_LINECOLOR_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaLineColorAction* pA = (const MetaLineColorAction*)pAction;
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setLineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setLineColor(pA->GetColor().getBColor());

                    break;
                }
                case META_FILLCOLOR_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaFillColorAction* pA = (const MetaFillColorAction*)pAction;
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setFillColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setFillColor(pA->GetColor().getBColor());

                    break;
                }
                case META_TEXTCOLOR_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaTextColorAction* pA = (const MetaTextColorAction*)pAction;
                    const bool bActivate(COL_TRANSPARENT != pA->GetColor().GetColor());

                    rPropertyHolders.Current().setTextColorActive(bActivate);
                    rPropertyHolders.Current().setTextColor(pA->GetColor().getBColor());

                    break;
                }
                case META_TEXTFILLCOLOR_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*)pAction;
                    const bool bWithColorArgument(pA->IsSetting());

                    if(bWithColorArgument)
                    {
                        // emulate OutputDevice::SetTextFillColor(...) WITH argument
                        const Color& rFontFillColor = pA->GetColor();
                        rPropertyHolders.Current().setTextFillColor(rFontFillColor.getBColor());
                        rPropertyHolders.Current().setTextFillColorActive(COL_TRANSPARENT != rFontFillColor.GetColor());
                    }
                    else
                    {
                        // emulate SetFillColor() <- NO argument (!)
                        rPropertyHolders.Current().setTextFillColorActive(false);
                    }

                    break;
                }
                case META_TEXTALIGN_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*)pAction;
                    const TextAlign aNewTextAlign = pA->GetTextAlign();

                    // TextAlign is applied to the current font (as in
                    // OutputDevice::SetTextAlign which would be used when
                    // playing the Metafile)
                    if(rPropertyHolders.Current().getFont().GetAlign() != aNewTextAlign)
                    {
                        Font aNewFont(rPropertyHolders.Current().getFont());
                        aNewFont.SetAlign(aNewTextAlign);
                        rPropertyHolders.Current().setFont(aNewFont);
                    }

                    break;
                }
                case META_MAPMODE_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaMapModeAction* pA = (const MetaMapModeAction*)pAction;
                    const MapMode& rMapMode = pA->GetMapMode();
                    basegfx::B2DHomMatrix aMapping;

                    if(MAP_RELATIVE == rMapMode.GetMapUnit())
                    {
                        aMapping = getTransformFromMapMode(rMapMode);
                    }
                    else
                    {
                        switch(rMapMode.GetMapUnit())
                        {
                            case MAP_100TH_MM :
                            {
                                if(MAP_TWIP == rPropertyHolders.Current().getMapUnit())
                                {
                                    // MAP_TWIP -> MAP_100TH_MM
                                    const double fTwipTo100thMm(127.0 / 72.0);
                                    aMapping.scale(fTwipTo100thMm, fTwipTo100thMm);
                                }
                                break;
                            }
                            case MAP_TWIP :
                            {
                                if(MAP_100TH_MM == rPropertyHolders.Current().getMapUnit())
                                {
                                    // MAP_100TH_MM -> MAP_TWIP
                                    const double f100thMmToTwip(72.0 / 127.0);
                                    aMapping.scale(f100thMmToTwip, f100thMmToTwip);
                                }
                                break;
                            }
                            default :
                            {
                                OSL_ENSURE(false, "interpretMetafile: META_MAPMODE_ACTION with unsupported MapUnit (!)");
                                break;
                            }
                        }

                        aMapping = getTransformFromMapMode(rMapMode) * aMapping;
                        rPropertyHolders.Current().setMapUnit(rMapMode.GetMapUnit());
                    }

                    if(!aMapping.isIdentity())
                    {
                        aMapping = aMapping * rPropertyHolders.Current().getTransformation();
                        rPropertyHolders.Current().setTransformation(aMapping);
                    }

                    break;
                }
                case META_FONT_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaFontAction* pA = (const MetaFontAction*)pAction;
                    rPropertyHolders.Current().setFont(pA->GetFont());

                    // older Metafiles have no META_TEXTCOLOR_ACTION which defines
                    // the FontColor now, so use the Font's color when not transparent
                    const Color& rFontColor = pA->GetFont().GetColor();
                    const bool bActivate(COL_TRANSPARENT != rFontColor.GetColor());

                    if(bActivate)
                    {
                        rPropertyHolders.Current().setTextColor(rFontColor.getBColor());
                    }

                    // caution: do NOT decativate here on transparet, see
                    // OutputDevice::SetFont(..) for more info
                    // rPropertyHolders.Current().setTextColorActive(bActivate);

                    // for fill color emulate a MetaTextFillColorAction with !transparent as bool,
                    // see OutputDevice::SetFont(..) the if(mpMetaFile) case
                    if(bActivate)
                    {
                        const Color& rFontFillColor = pA->GetFont().GetFillColor();
                        rPropertyHolders.Current().setTextFillColor(rFontFillColor.getBColor());
                        rPropertyHolders.Current().setTextFillColorActive(COL_TRANSPARENT != rFontFillColor.GetColor());
                    }
                    else
                    {
                        rPropertyHolders.Current().setTextFillColorActive(false);
                    }

                    break;
                }
                case META_PUSH_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    rPropertyHolders.Push();

                    break;
                }
                case META_POP_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getRegionActive())
                    {
                        // end clipping
                        HandleNewClipRegion(0, rTargetHolders, rPropertyHolders);
                    }

                    if(rPropertyHolders.Current().isRasterOpActive())
                    {
                        // end RasterOp
                        HandleNewRasterOp(ROP_OVERPAINT, rTargetHolders, rPropertyHolders);
                    }

                    rPropertyHolders.Pop();

                    if(rPropertyHolders.Current().isRasterOpActive())
                    {
                        // start RasterOp
                        HandleNewRasterOp(rPropertyHolders.Current().getRasterOp(), rTargetHolders, rPropertyHolders);
                    }

                    if(rPropertyHolders.Current().getRegionActive())
                    {
                        // start clipping
                        HandleNewClipRegion(&rPropertyHolders.Current().getRegion(), rTargetHolders, rPropertyHolders);
                    }

                    break;
                }
                case META_RASTEROP_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaRasterOpAction* pA = (const MetaRasterOpAction*)pAction;
                    const RasterOp aRasterOp = pA->GetRasterOp();

                    HandleNewRasterOp(pA->GetRasterOp(), rTargetHolders, rPropertyHolders);

                    break;
                }
                case META_TRANSPARENT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTransparentAction* pA = (const MetaTransparentAction*)pAction;
                    const basegfx::B2DPolyPolygon aOutline(pA->GetPolyPolygon().getB2DPolyPolygon());

                    if(aOutline.count())
                    {
                        const sal_uInt16 nTransparence(pA->GetTransparence());

                        if(0 == nTransparence)
                        {
                            // not transparent
                            createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                        }
                        else if(nTransparence >= 100)
                        {
                            // fully or more than transparent
                        }
                        else
                        {
                            // transparent. Create new target
                            rTargetHolders.Push();

                            // create primitives there and get them
                            createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                            const drawinglayer::primitive2d::Primitive2DSequence aSubContent(
                                rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current()));

                            // back to old target
                            rTargetHolders.Pop();

                            if(aSubContent.hasElements())
                            {
                                rTargetHolders.Current().append(
                                    new drawinglayer::primitive2d::UnifiedAlphaPrimitive2D(
                                        aSubContent,
                                        nTransparence * 0.01));
                            }
                        }
                    }

                    break;
                }
                case META_EPS_ACTION :
                {
                    /** NEEDS IMPLEMENTATION */
                    OSL_ENSURE(false, "META_EPS_ACTION requested (!)");
                    const MetaEPSAction* pA = (const MetaEPSAction*)pAction;
                    break;
                }
                case META_REFPOINT_ACTION :
                {
                    /** SIMPLE, DONE */
                    // only used for hatch and line pattern offsets
                    // const MetaRefPointAction* pA = (const MetaRefPointAction*)pAction;
                    break;
                }
                case META_TEXTLINECOLOR_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaTextLineColorAction* pA = (const MetaTextLineColorAction*)pAction;
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setTextLineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setTextLineColor(pA->GetColor().getBColor());

                    break;
                }
                case META_TEXTLINE_ACTION :
                {
                    /** NEEDS IMPLEMENTATION */
                    OSL_ENSURE(false, "META_TEXTLINE_ACTION requested (!)");
                    // actually creates overline, underline and strikeouts, so
                    // these should be isolated from TextDecoratedPortionPrimitive2D
                    // to own primitives...
                    const MetaTextLineAction* pA = (const MetaTextLineAction*)pAction;
                    break;
                }
                case META_FLOATTRANSPARENT_ACTION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*)pAction;
                    const Rectangle aTargetRectangle(pA->GetPoint(), pA->GetSize());

                    if(!aTargetRectangle.IsEmpty())
                    {
                        const GDIMetaFile& rContent = pA->GetGDIMetaFile();

                        if(rContent.GetActionCount())
                        {
                            // create the sub-content
                            drawinglayer::primitive2d::Primitive2DSequence xSubContent;
                            {
                                rTargetHolders.Push();
                                interpretMetafile(rContent, rTargetHolders, rPropertyHolders, rViewInformation);
                                xSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
                                rTargetHolders.Pop();
                            }

                            if(xSubContent.hasElements())
                            {
                                // check if gradient is a real gradient
                                const Gradient& rGradient = pA->GetGradient();
                                const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));

                                if(aAttribute.getStartColor() == aAttribute.getEndColor())
                                {
                                    // not really a gradient; create UnifiedAlphaPrimitive2D
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::UnifiedAlphaPrimitive2D(
                                            xSubContent,
                                            aAttribute.getStartColor().luminance()));
                                }
                                else
                                {
                                    // really a gradient. Create gradient sub-content (with correct scaling)
                                    basegfx::B2DRange aRange(
                                        aTargetRectangle.Left(), aTargetRectangle.Top(),
                                        aTargetRectangle.Right(), aTargetRectangle.Bottom());
                                    aRange.transform(rPropertyHolders.Current().getTransformation());

                                    // prepare gradient for alpha content
                                    const drawinglayer::primitive2d::Primitive2DReference xAlpha(
                                        new drawinglayer::primitive2d::FillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));

                                    // create alpha primitive
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::AlphaPrimitive2D(
                                            xSubContent,
                                            drawinglayer::primitive2d::Primitive2DSequence(&xAlpha, 1)));
                                }
                            }
                        }
                    }

                    break;
                }
                case META_GRADIENTEX_ACTION :
                {
                    /** SIMPLE, DONE */
                    // This is only a data holder which is interpreted inside comment actions
                    // const MetaGradientExAction* pA = (const MetaGradientExAction*)pAction;
                    break;
                }
                case META_LAYOUTMODE_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaLayoutModeAction* pA = (const MetaLayoutModeAction*)pAction;
                    rPropertyHolders.Current().setLayoutMode(pA->GetLayoutMode());
                    break;
                }
                case META_TEXTLANGUAGE_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaTextLanguageAction* pA = (const MetaTextLanguageAction*)pAction;
                    rPropertyHolders.Current().setLanguageType(pA->GetTextLanguage());
                    break;
                }
                case META_OVERLINECOLOR_ACTION :
                {
                    /** SIMPLE, DONE */
                    const MetaOverlineColorAction* pA = (const MetaOverlineColorAction*)pAction;
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setOverlineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setOverlineColor(pA->GetColor().getBColor());

                    break;
                }
                case META_COMMENT_ACTION :
                {
                    /** NEEDS IMPLEMENTATION */
                    OSL_ENSURE(false, "META_COMMENT_ACTION requested (!)");
                    const MetaCommentAction* pA = (const MetaCommentAction*)pAction;
                    break;
                }
                default:
                {
                    OSL_ENSURE(false, "Unknown MetaFile Action (!)");
                    break;
                }
            }
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence MetafilePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            TargetHolders aTargetHolders;
            PropertyHolders aPropertyHolders;

            interpretMetafile(getMetaFile(), aTargetHolders, aPropertyHolders, rViewInformation);
            Primitive2DSequence xRetval = aTargetHolders.Current().getPrimitive2DSequence(aPropertyHolders.Current());

            if(xRetval.hasElements())
            {
                Rectangle aMtfTarget(getMetaFile().GetPrefMapMode().GetOrigin(), getMetaFile().GetPrefSize());

                if(MAP_PIXEL == getMetaFile().GetPrefMapMode().GetMapUnit())
                {
                    aMtfTarget = Application::GetDefaultDevice()->PixelToLogic(aMtfTarget, MAP_100TH_MM);
                }
                else
                {
                    aMtfTarget = Application::GetDefaultDevice()->LogicToLogic(aMtfTarget, getMetaFile().GetPrefMapMode(), MAP_100TH_MM);
                }

                basegfx::B2DHomMatrix aAdaptedTransform;

                aAdaptedTransform.translate(-aMtfTarget.Left(), -aMtfTarget.Top());
                aAdaptedTransform.scale(
                    aMtfTarget.getWidth() ? 1.0 / aMtfTarget.getWidth() : 1.0,
                    aMtfTarget.getHeight() ? 1.0 / aMtfTarget.getHeight() : 1.0);
                aAdaptedTransform = getTransform() * aAdaptedTransform;

                const Primitive2DReference aEmbeddedTransform(
                    new TransformPrimitive2D(
                        aAdaptedTransform,
                        xRetval));

                xRetval = Primitive2DSequence(&aEmbeddedTransform, 1);
            }

            return xRetval;
        }

        MetafilePrimitive2D::MetafilePrimitive2D(
            const basegfx::B2DHomMatrix& rMetaFileTransform,
            const GDIMetaFile& rMetaFile)
        :   BufferedDecompositionPrimitive2D(),
            maMetaFileTransform(rMetaFileTransform),
            maMetaFile(rMetaFile)
        {
        }

        bool MetafilePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MetafilePrimitive2D& rCompare = (MetafilePrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getMetaFile() == rCompare.getMetaFile());
            }

            return false;
        }

        basegfx::B2DRange MetafilePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MetafilePrimitive2D, PRIMITIVE2D_ID_METAFILEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
