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
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/wallpaperprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <drawinglayer/primitive2d/textlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/textstrikeoutprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <tools/fract.hxx>
#include <numeric>



using namespace com::sun::star;



namespace
{
    /** helper class for graphic context

        This class allows to hold a complete representation of classic
        VCL OutputDevice state. This data is needed for correct
        interpretation of the MetaFile action flow.
    */
    class PropertyHolder
    {
    private:
        /// current transformation (aka MapMode)
        basegfx::B2DHomMatrix   maTransformation;
        MapUnit                 maMapUnit;

        /// current colors
        basegfx::BColor         maLineColor;
        basegfx::BColor         maFillColor;
        basegfx::BColor         maTextColor;
        basegfx::BColor         maTextFillColor;
        basegfx::BColor         maTextLineColor;
        basegfx::BColor         maOverlineColor;

        /// clipping
        basegfx::B2DPolyPolygon maClipPolyPoygon;

        /// font, etc.
        vcl::Font               maFont;
        RasterOp                maRasterOp;
        ComplexTextLayoutMode   mnLayoutMode;
        LanguageType            maLanguageType;
        PushFlags               mnPushFlags;

        /// bitfield
        /// contains all active markers
        bool                    mbLineColor : 1;
        bool                    mbFillColor : 1;
        bool                    mbTextColor : 1;
        bool                    mbTextFillColor : 1;
        bool                    mbTextLineColor : 1;
        bool                    mbOverlineColor : 1;
        bool                    mbClipPolyPolygonActive : 1;

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
            maClipPolyPoygon(),
            maFont(),
            maRasterOp(ROP_OVERPAINT),
            mnLayoutMode(TEXT_LAYOUT_DEFAULT),
            maLanguageType(0),
            mnPushFlags(PushFlags::NONE),
            mbLineColor(false),
            mbFillColor(false),
            mbTextColor(true),
            mbTextFillColor(false),
            mbTextLineColor(false),
            mbOverlineColor(false),
            mbClipPolyPolygonActive(false)
        {
        }

        ~PropertyHolder()
        {
        }

        /// read/write accesses
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

        const basegfx::B2DPolyPolygon& getClipPolyPolygon() const { return maClipPolyPoygon; }
        void setClipPolyPolygon(const basegfx::B2DPolyPolygon& rNew) { if(rNew != maClipPolyPoygon) maClipPolyPoygon = rNew; }
        bool getClipPolyPolygonActive() const { return mbClipPolyPolygonActive; }
        void setClipPolyPolygonActive(bool bNew) { if(bNew != mbClipPolyPolygonActive) mbClipPolyPolygonActive = bNew; }

        const vcl::Font& getFont() const { return maFont; }
        void setFont(const vcl::Font& rFont) { if(rFont != maFont) maFont = rFont; }

        const RasterOp& getRasterOp() const { return maRasterOp; }
        void setRasterOp(const RasterOp& rRasterOp) { if(rRasterOp != maRasterOp) maRasterOp = rRasterOp; }
        bool isRasterOpInvert() const { return (ROP_XOR == maRasterOp || ROP_INVERT == maRasterOp); }
        bool isRasterOpForceBlack() const { return ROP_0 == maRasterOp; }
        bool isRasterOpActive() const { return isRasterOpInvert() || isRasterOpForceBlack(); }

        ComplexTextLayoutMode getLayoutMode() const { return mnLayoutMode; }
        void setLayoutMode(ComplexTextLayoutMode nNew) { if(nNew != mnLayoutMode) mnLayoutMode = nNew; }

        LanguageType getLanguageType() const { return maLanguageType; }
        void setLanguageType(LanguageType aNew) { if(aNew != maLanguageType) maLanguageType = aNew; }

        PushFlags getPushFlags() const { return mnPushFlags; }
        void setPushFlags(PushFlags nNew) { if(nNew != mnPushFlags) mnPushFlags = nNew; }

        bool getLineOrFillActive() const { return (mbLineColor || mbFillColor); }
    };
} // end of anonymous namespace



namespace
{
    /** stack for properites

        This class builds a stack based on the PropertyHolder
        class. It encapsulates the pointer/new/delete usage to
        make it safe and implements the push/pop as needed by a
        VCL Metafile interpreter. The critical part here are the
        flag values VCL OutputDevice uses here; not all stuff is
        pushed and thus needs to be copied at pop.
    */
    class PropertyHolders
    {
    private:
        std::vector< PropertyHolder* >          maPropertyHolders;

    public:
        PropertyHolders()
        {
            maPropertyHolders.push_back(new PropertyHolder());
        }

        void PushDefault()
        {
            PropertyHolder* pNew = new PropertyHolder();
            maPropertyHolders.push_back(pNew);
        }

        void Push(PushFlags nPushFlags)
        {
            if(bool(nPushFlags))
            {
                OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: PUSH with no property holders (!)");
                if ( !maPropertyHolders.empty() )
                {
                    PropertyHolder* pNew = new PropertyHolder(*maPropertyHolders.back());
                    pNew->setPushFlags(nPushFlags);
                    maPropertyHolders.push_back(pNew);
                }
            }
        }

        void Pop()
        {
            OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: POP with no property holders (!)");
            const sal_uInt32 nSize(maPropertyHolders.size());

            if(nSize)
            {
                const PropertyHolder* pTip = maPropertyHolders.back();
                const PushFlags nPushFlags(pTip->getPushFlags());

                if(nPushFlags != PushFlags::NONE)
                {
                    if(nSize > 1)
                    {
                        // copy back content for all non-set flags
                        PropertyHolder* pLast = maPropertyHolders[nSize - 2];

                        if(PushFlags::ALL != nPushFlags)
                        {
                            if(!(nPushFlags & PushFlags::LINECOLOR      ))
                            {
                                pLast->setLineColor(pTip->getLineColor());
                                pLast->setLineColorActive(pTip->getLineColorActive());
                            }
                            if(!(nPushFlags & PushFlags::FILLCOLOR      ))
                            {
                                pLast->setFillColor(pTip->getFillColor());
                                pLast->setFillColorActive(pTip->getFillColorActive());
                            }
                            if(!(nPushFlags & PushFlags::FONT           ))
                            {
                                pLast->setFont(pTip->getFont());
                            }
                            if(!(nPushFlags & PushFlags::TEXTCOLOR      ))
                            {
                                pLast->setTextColor(pTip->getTextColor());
                                pLast->setTextColorActive(pTip->getTextColorActive());
                            }
                            if(!(nPushFlags & PushFlags::MAPMODE        ))
                            {
                                pLast->setTransformation(pTip->getTransformation());
                                pLast->setMapUnit(pTip->getMapUnit());
                            }
                            if(!(nPushFlags & PushFlags::CLIPREGION     ))
                            {
                                pLast->setClipPolyPolygon(pTip->getClipPolyPolygon());
                                pLast->setClipPolyPolygonActive(pTip->getClipPolyPolygonActive());
                            }
                            if(!(nPushFlags & PushFlags::RASTEROP       ))
                            {
                                pLast->setRasterOp(pTip->getRasterOp());
                            }
                            if(!(nPushFlags & PushFlags::TEXTFILLCOLOR  ))
                            {
                                pLast->setTextFillColor(pTip->getTextFillColor());
                                pLast->setTextFillColorActive(pTip->getTextFillColorActive());
                            }
                            if(!(nPushFlags & PushFlags::TEXTALIGN      ))
                            {
                                if(pLast->getFont().GetAlign() != pTip->getFont().GetAlign())
                                {
                                    vcl::Font aFont(pLast->getFont());
                                    aFont.SetAlign(pTip->getFont().GetAlign());
                                    pLast->setFont(aFont);
                                }
                            }
                            if(!(nPushFlags & PushFlags::REFPOINT       ))
                            {
                                // not supported
                            }
                            if(!(nPushFlags & PushFlags::TEXTLINECOLOR  ))
                            {
                                pLast->setTextLineColor(pTip->getTextLineColor());
                                pLast->setTextLineColorActive(pTip->getTextLineColorActive());
                            }
                            if(!(nPushFlags & PushFlags::TEXTLAYOUTMODE ))
                            {
                                pLast->setLayoutMode(pTip->getLayoutMode());
                            }
                            if(!(nPushFlags & PushFlags::TEXTLANGUAGE   ))
                            {
                                pLast->setLanguageType(pTip->getLanguageType());
                            }
                            if(!(nPushFlags & PushFlags::OVERLINECOLOR  ))
                            {
                                pLast->setOverlineColor(pTip->getOverlineColor());
                                pLast->setOverlineColorActive(pTip->getOverlineColorActive());
                            }
                        }
                    }
                }

                // execute the pop
                delete maPropertyHolders.back();
                maPropertyHolders.pop_back();
            }
        }

        PropertyHolder& Current()
        {
            static PropertyHolder aDummy;
            OSL_ENSURE(maPropertyHolders.size(), "PropertyHolders: CURRENT with no property holders (!)");
            return maPropertyHolders.empty() ? aDummy : *maPropertyHolders.back();
        }

        ~PropertyHolders()
        {
            while(!maPropertyHolders.empty())
            {
                delete maPropertyHolders.back();
                maPropertyHolders.pop_back();
            }
        }
    };
} // end of anonymous namespace



namespace
{
    /** helper to convert a vcl::Region to a B2DPolyPolygon
        when it does not yet contain one. In the future
        this may be expanded to merge the polygons created
        from rectangles or use a special algo to directly turn
        the spans of regions to a single, already merged
        PolyPolygon.
     */
    basegfx::B2DPolyPolygon getB2DPolyPolygonFromRegion(const vcl::Region& rRegion)
    {
        basegfx::B2DPolyPolygon aRetval;

        if(!rRegion.IsEmpty())
        {
            vcl::Region aRegion(rRegion);

            aRetval = aRegion.GetAsB2DPolyPolygon();
        }

        return aRetval;
    }
} // end of anonymous namespace



namespace
{
    /** Helper class to buffer and hold a Primitive target vector. It
        encapsulates the new/delete functionality and allows to work
        on pointers of the implementation classes. All data will
        be converted to uno sequences of uno references when accessing the
        data.
    */
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

        sal_uInt32 size() const
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

        drawinglayer::primitive2d::Primitive2DContainer getPrimitive2DSequence(const PropertyHolder& rPropertyHolder)
        {
            const sal_uInt32 nCount(aTargets.size());
            drawinglayer::primitive2d::Primitive2DContainer xRetval(nCount);

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                xRetval[a] = aTargets[a];
            }

            // All Targets were pointers, but do not need to be deleted since they
            // were converted to UNO API references now, so they stay as long as
            // referenced. Do NOT delete the C++ implementation classes here, but clear
            // the buffer to not delete them in the destructor.
            aTargets.clear();

            if(!xRetval.empty() && rPropertyHolder.getClipPolyPolygonActive())
            {
                const basegfx::B2DPolyPolygon& rClipPolyPolygon = rPropertyHolder.getClipPolyPolygon();

                if(rClipPolyPolygon.count())
                {
                    const drawinglayer::primitive2d::Primitive2DReference xMask(
                        new drawinglayer::primitive2d::MaskPrimitive2D(
                            rClipPolyPolygon,
                            xRetval));

                    xRetval = drawinglayer::primitive2d::Primitive2DContainer { xMask };
                }
            }

            return xRetval;
        }
    };
} // end of anonymous namespace



namespace
{
    /** Helper class which builds a stack on the TargetHolder class */
    class TargetHolders
    {
    private:
        std::vector< TargetHolder* >          maTargetHolders;

    public:
        TargetHolders()
        {
            maTargetHolders.push_back(new TargetHolder());
        }

        sal_uInt32 size() const
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
            if(!maTargetHolders.empty())
            {
                delete maTargetHolders.back();
                maTargetHolders.pop_back();
            }
        }

        TargetHolder& Current()
        {
            static TargetHolder aDummy;
            OSL_ENSURE(maTargetHolders.size(), "TargetHolders: CURRENT with no property holders (!)");
            return maTargetHolders.empty() ? aDummy : *maTargetHolders.back();
        }

        ~TargetHolders()
        {
            while(!maTargetHolders.empty())
            {
                delete maTargetHolders.back();
                maTargetHolders.pop_back();
            }
        }
    };
} // end of anonymous namespace



namespace drawinglayer
{
    namespace primitive2d
    {
        /** NonOverlappingFillGradientPrimitive2D class

            This is a special version of the FillGradientPrimitive2D which decomposes
            to a non-overlapping geometry version of the gradient. This needs to be
            used to support the old XOR paint-'trick'.

            It does not need an own identifier since a renderer who wants to interpret
            it itself may do so. It just overrides the decomposition of the C++
            implementation class to do an alternative decomposition.
         */
        class NonOverlappingFillGradientPrimitive2D : public FillGradientPrimitive2D
        {
        protected:
            /// local decomposition.
            virtual Primitive2DContainer create2DDecomposition(
                const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            NonOverlappingFillGradientPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const attribute::FillGradientAttribute& rFillGradient)
            :   FillGradientPrimitive2D(rObjectRange, rFillGradient)
            {
            }
        };

        Primitive2DContainer NonOverlappingFillGradientPrimitive2D::create2DDecomposition(
            const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(!getFillGradient().isDefault())
            {
                return createFill(false);
            }
            else
            {
                return Primitive2DContainer();
            }
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer



namespace
{
    /** helper to convert a MapMode to a transformation */
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

    /** helper to create a PointArrayPrimitive2D based on current context */
    void createPointArrayPrimitive(
        const std::vector< basegfx::B2DPoint >& rPositions,
        TargetHolder& rTarget,
        PropertyHolder& rProperties,
        const basegfx::BColor& rBColor)
    {
        if(!rPositions.empty())
        {
            if(rProperties.getTransformation().isIdentity())
            {
                rTarget.append(
                    new drawinglayer::primitive2d::PointArrayPrimitive2D(
                        rPositions,
                        rBColor));
            }
            else
            {
                std::vector< basegfx::B2DPoint > aPositions(rPositions);

                for(size_t a(0); a < aPositions.size(); a++)
                {
                    aPositions[a] = rProperties.getTransformation() * aPositions[a];
                }

                rTarget.append(
                    new drawinglayer::primitive2d::PointArrayPrimitive2D(
                        aPositions,
                        rBColor));
            }
        }
    }

    /** helper to create a PolygonHairlinePrimitive2D based on current context */
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

    /** helper to create a PolyPolygonColorPrimitive2D based on current context */
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

    /** helper to create a PolygonStrokePrimitive2D based on current context */
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
                    rLineInfo.GetLineJoin(),
                    rLineInfo.GetLineCap());

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

    /** helper to create needed line and fill primitives based on current context */
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

    /** helper to create needed line and fill primitives based on current context */
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

    /** helper to create DiscreteBitmapPrimitive2D based on current context.
        The DiscreteBitmapPrimitive2D is especially created for this usage
        since no other usage defines a bitmap visualisation based on top-left
        position and size in pixels. At the end it will create a view-dependent
        transformed embedding of a BitmapPrimitive2D.
    */
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

    /** helper to create BitmapPrimitive2D based on current context */
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

    /** helper to create a regular BotmapEx from a MaskAction (definitions
        which use a bitmap without transparence but define one of the colors as
        transparent)
     */
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

    /** helper to convert from a VCL Gradient definition to the corresponding
        data for primitive representation
     */
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
            case GradientStyle_LINEAR :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_LINEAR;
                break;
            }
            case GradientStyle_AXIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_AXIAL;
                break;
            }
            case GradientStyle_RADIAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RADIAL;
                break;
            }
            case GradientStyle_ELLIPTICAL :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_ELLIPTICAL;
                break;
            }
            case GradientStyle_SQUARE :
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_SQUARE;
                break;
            }
            default : // GradientStyle_RECT
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

    /** helper to convert from a VCL Hatch definition to the corresponding
        data for primitive representation
     */
    drawinglayer::attribute::FillHatchAttribute createFillHatchAttribute(const Hatch& rHatch)
    {
        drawinglayer::attribute::HatchStyle aHatchStyle(drawinglayer::attribute::HATCHSTYLE_SINGLE);

        switch(rHatch.GetStyle())
        {
            default : // case HATCH_SINGLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_SINGLE;
                break;
            }
            case HATCH_DOUBLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_DOUBLE;
                break;
            }
            case HATCH_TRIPLE :
            {
                aHatchStyle = drawinglayer::attribute::HATCHSTYLE_TRIPLE;
                break;
            }
        }

        return drawinglayer::attribute::FillHatchAttribute(
            aHatchStyle,
            (double)rHatch.GetDistance(),
            (double)rHatch.GetAngle() * F_PI1800,
            rHatch.GetColor().getBColor(),
            3, // same default as VCL, a minimum of three discrete units (pixels) offset
            false);
    }

    /** helper to take needed action on ClipRegion change. This method needs to be called
        on any vcl::Region change, e.g. at the obvious actions doing this, but also at pop-calls
        which change the vcl::Region of the current context. It takes care of creating the
        current embedded context, set the new vcl::Region at the context and possibly prepare
        a new target for including new geometry into the current region
     */
    void HandleNewClipRegion(
        const basegfx::B2DPolyPolygon& rClipPolyPolygon,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders)
    {
        const bool bNewActive(rClipPolyPolygon.count());

        // #i108636# The handling of new ClipPolyPolygons was not done as good as possible
        // in the first version of this interpreter; e.g. when a ClipPolyPolygon was set
        // initially and then using a lot of push/pop actions, the pop always leads
        // to setting a 'new' ClipPolyPolygon which indeed is the return to the ClipPolyPolygon
        // of the properties next on the stack.

        // This ClipPolyPolygon is identical to the current one, so there is no need to
        // create a MaskPrimitive2D containing the up-to-now created primitives, but
        // this was done before. While this does not lead to wrong primitive
        // representations of the metafile data, it creates unnecessarily expensive
        // representations. Just detecting when no really 'new' ClipPolyPolygon gets set
        // solves the problem.

        if(!rPropertyHolders.Current().getClipPolyPolygonActive() && !bNewActive)
        {
            // no active ClipPolyPolygon exchanged by no new one, done
            return;
        }

        if(rPropertyHolders.Current().getClipPolyPolygonActive() && bNewActive)
        {
            // active ClipPolyPolygon and new active ClipPolyPolygon
            if(rPropertyHolders.Current().getClipPolyPolygon() == rClipPolyPolygon)
            {
                // new is the same as old, done
                return;
            }
        }

        // Here the old and the new are definitively different, maybe
        // old one and/or new one is not active.

        // Handle deletion of old ClipPolyPolygon. The process evtl. created primitives which
        // belong to this active ClipPolyPolygon. These need to be embedded to a
        // MaskPrimitive2D accordingly.
        if(rPropertyHolders.Current().getClipPolyPolygonActive() && rTargetHolders.size() > 1)
        {
            drawinglayer::primitive2d::Primitive2DContainer aSubContent;

            if(rPropertyHolders.Current().getClipPolyPolygon().count()
                && rTargetHolders.Current().size())
            {
                aSubContent = rTargetHolders.Current().getPrimitive2DSequence(
                    rPropertyHolders.Current());
            }

            rTargetHolders.Pop();

            if(!aSubContent.empty())
            {
                rTargetHolders.Current().append(
                    new drawinglayer::primitive2d::GroupPrimitive2D(
                        aSubContent));
            }
        }

        // apply new settings to current properties by setting
        // the new region now
        rPropertyHolders.Current().setClipPolyPolygonActive(bNewActive);

        if(bNewActive)
        {
            rPropertyHolders.Current().setClipPolyPolygon(rClipPolyPolygon);

            // prepare new content holder for new active region
            rTargetHolders.Push();
        }
    }

    /** helper to handle the change of RasterOp. It takes care of encapsulating all current
        geometry to the current RasterOp (if changed) and needs to be called on any RasterOp
        change. It will also start a new geometry target to embrace to the new RasterOp if
        a changing RasterOp is used. Currently, ROP_XOR and ROP_INVERT are supported using
        InvertPrimitive2D, and ROP_0 by using a ModifiedColorPrimitive2D to force to black paint
     */
    void HandleNewRasterOp(
        RasterOp aRasterOp,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders)
    {
        // check if currently active
        if(rPropertyHolders.Current().isRasterOpActive() && rTargetHolders.size() > 1)
        {
            drawinglayer::primitive2d::Primitive2DContainer aSubContent;

            if(rTargetHolders.Current().size())
            {
                aSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
            }

            rTargetHolders.Pop();

            if(!aSubContent.empty())
            {
                if(rPropertyHolders.Current().isRasterOpForceBlack())
                {
                    // force content to black
                    rTargetHolders.Current().append(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                            aSubContent,
                            basegfx::BColorModifierSharedPtr(
                                new basegfx::BColorModifier_replace(
                                    basegfx::BColor(0.0, 0.0, 0.0)))));
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

    /** helper to create needed data to emulate the VCL Wallpaper Metafile action.
        It is a quite mighty action. This helper is for simple color filled background.
     */
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

    /** helper to create needed data to emulate the VCL Wallpaper Metafile action.
        It is a quite mighty action. This helper is for gradient filled background.
     */
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
                const drawinglayer::primitive2d::Primitive2DContainer xSeq { xPrim };

                pRetval = new drawinglayer::primitive2d::TransformPrimitive2D(
                    rPropertyHolder.getTransformation(),
                    xSeq);
            }

            return pRetval;
        }
    }

    /** helper to create needed data to emulate the VCL Wallpaper Metafile action.
        It is a quite mighty action. This helper decides if color and/or gradient
        background is needed for the wanted bitmap fill and then creates the needed
        WallpaperBitmapPrimitive2D. This primitive was created for this purpose and
        takes over all needed logic of orientations and tiling.
     */
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
            || (WallpaperStyle::Tile != eWallpaperStyle && WallpaperStyle::Scale != eWallpaperStyle))
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
                    drawinglayer::primitive2d::Primitive2DContainer { xPrim }));
        }
    }

    /** helper to decide UnderlineAbove for text primitives */
    bool isUnderlineAbove(const vcl::Font& rFont)
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

    void createFontAttributeTransformAndAlignment(
        drawinglayer::attribute::FontAttribute& rFontAttribute,
        basegfx::B2DHomMatrix& rTextTransform,
        basegfx::B2DVector& rAlignmentOffset,
        PropertyHolder& rProperty)
    {
        const vcl::Font& rFont = rProperty.getFont();
        basegfx::B2DVector aFontScaling;

        rFontAttribute = drawinglayer::attribute::FontAttribute(
            drawinglayer::primitive2d::getFontAttributeFromVclFont(
                aFontScaling,
                rFont,
                bool(rProperty.getLayoutMode() & TEXT_LAYOUT_BIDI_RTL),
                bool(rProperty.getLayoutMode() & TEXT_LAYOUT_BIDI_STRONG)));

        // add FontScaling
        rTextTransform.scale(aFontScaling.getX(), aFontScaling.getY());

        // take text align into account
        if(ALIGN_BASELINE != rFont.GetAlign())
        {
            drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
            aTextLayouterDevice.setFont(rFont);

            if(ALIGN_TOP == rFont.GetAlign())
            {
                rAlignmentOffset.setY(aTextLayouterDevice.getFontAscent());
            }
            else // ALIGN_BOTTOM
            {
                rAlignmentOffset.setY(-aTextLayouterDevice.getFontDescent());
            }

            rTextTransform.translate(rAlignmentOffset.getX(), rAlignmentOffset.getY());
        }

        // add FontRotation (if used)
        if(rFont.GetOrientation())
        {
            rTextTransform.rotate(-rFont.GetOrientation() * F_PI1800);
        }
    }

    /** helper which takes complete care for creating the needed text primitives. It
        takes care of decorated stuff and all the geometry adaptions needed
     */
    void processMetaTextAction(
        const Point& rTextStartPosition,
        const OUString& rText,
        sal_uInt16 nTextStart,
        sal_uInt16 nTextLength,
        const ::std::vector< double >& rDXArray,
        TargetHolder& rTarget,
        PropertyHolder& rProperty)
    {
        drawinglayer::primitive2d::BasePrimitive2D* pResult = nullptr;
        const vcl::Font& rFont = rProperty.getFont();
        basegfx::B2DVector aAlignmentOffset(0.0, 0.0);

        if(nTextLength)
        {
            drawinglayer::attribute::FontAttribute aFontAttribute;
            basegfx::B2DHomMatrix aTextTransform;

            // fill parameters derived from current font
            createFontAttributeTransformAndAlignment(
                aFontAttribute,
                aTextTransform,
                aAlignmentOffset,
                rProperty);

            // add TextStartPosition
            aTextTransform.translate(rTextStartPosition.X(), rTextStartPosition.Y());

            // prepare FontColor and Locale
            const basegfx::BColor aFontColor(rProperty.getTextColor());
            const Color aFillColor(rFont.GetFillColor());
            const css::lang::Locale aLocale(LanguageTag(rProperty.getLanguageType()).getLocale());
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
                // prepare overline, underline and strikeout data
                const drawinglayer::primitive2d::TextLine eFontOverline(drawinglayer::primitive2d::mapFontUnderlineToTextLine(rFont.GetOverline()));
                const drawinglayer::primitive2d::TextLine eFontUnderline(drawinglayer::primitive2d::mapFontUnderlineToTextLine(rFont.GetUnderline()));
                const drawinglayer::primitive2d::TextStrikeout eTextStrikeout(drawinglayer::primitive2d::mapFontStrikeoutToTextStrikeout(rFont.GetStrikeout()));

                // check UndelineAbove
                const bool bUnderlineAbove(drawinglayer::primitive2d::TEXT_LINE_NONE != eFontUnderline && isUnderlineAbove(rFont));

                // prepare emphasis mark data
                drawinglayer::primitive2d::TextEmphasisMark eTextEmphasisMark(drawinglayer::primitive2d::TEXT_EMPHASISMARK_NONE);

                switch(rFont.GetEmphasisMark() & EMPHASISMARK_STYLE)
                {
                    case EMPHASISMARK_DOT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DOT; break;
                    case EMPHASISMARK_CIRCLE : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_CIRCLE; break;
                    case EMPHASISMARK_DISC : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_DISC; break;
                    case EMPHASISMARK_ACCENT : eTextEmphasisMark = drawinglayer::primitive2d::TEXT_EMPHASISMARK_ACCENT; break;
                }

                const bool bEmphasisMarkAbove(rFont.GetEmphasisMark() & EMPHASISMARK_POS_ABOVE);
                const bool bEmphasisMarkBelow(rFont.GetEmphasisMark() & EMPHASISMARK_POS_BELOW);

                // prepare font relief data
                drawinglayer::primitive2d::TextRelief eTextRelief(drawinglayer::primitive2d::TEXT_RELIEF_NONE);

                switch(rFont.GetRelief())
                {
                    case RELIEF_EMBOSSED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_EMBOSSED; break;
                    case RELIEF_ENGRAVED : eTextRelief = drawinglayer::primitive2d::TEXT_RELIEF_ENGRAVED; break;
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
                    rDXArray,
                    aFontAttribute,
                    aLocale,
                    aFontColor,
                    aFillColor,

                    // attributes for TextDecoratedPortionPrimitive2D
                    rProperty.getOverlineColorActive() ? rProperty.getOverlineColor() : aFontColor,
                    rProperty.getTextLineColorActive() ? rProperty.getTextLineColor() : aFontColor,
                    eFontOverline,
                    eFontUnderline,
                    bUnderlineAbove,
                    eTextStrikeout,
                    bWordLineMode,
                    eTextEmphasisMark,
                    bEmphasisMarkAbove,
                    bEmphasisMarkBelow,
                    eTextRelief,
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
                    rDXArray,
                    aFontAttribute,
                    aLocale,
                    aFontColor);
            }
        }

        if(pResult && rProperty.getTextFillColorActive())
        {
            // text background is requested, add and encapsulate both to new primitive
            drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
            aTextLayouterDevice.setFont(rFont);

            // get text width
            double fTextWidth(0.0);

            if(rDXArray.empty())
            {
                fTextWidth = aTextLayouterDevice.getTextWidth(rText, nTextStart, nTextLength);
            }
            else
            {
                fTextWidth = rDXArray.back();
            }

            if(basegfx::fTools::more(fTextWidth, 0.0))
            {
                // build text range
                const basegfx::B2DRange aTextRange(
                    0.0, -aTextLayouterDevice.getFontAscent(),
                    fTextWidth, aTextLayouterDevice.getFontDescent());

                // create Transform
                basegfx::B2DHomMatrix aTextTransform;

                aTextTransform.translate(aAlignmentOffset.getX(), aAlignmentOffset.getY());

                if(rFont.GetOrientation())
                {
                    aTextTransform.rotate(-rFont.GetOrientation() * F_PI1800);
                }

                aTextTransform.translate(rTextStartPosition.X(), rTextStartPosition.Y());

                // prepare Primitive2DSequence, put text in foreground
                drawinglayer::primitive2d::Primitive2DContainer aSequence(2);
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
                        drawinglayer::primitive2d::Primitive2DContainer { aReference }));
            }
        }
    }

    /** helper which takes complete care for creating the needed textLine primitives */
    void proccessMetaTextLineAction(
        const MetaTextLineAction& rAction,
        TargetHolder& rTarget,
        PropertyHolder& rProperty)
    {
        const double fLineWidth(fabs((double)rAction.GetWidth()));

        if(fLineWidth > 0.0)
        {
            const drawinglayer::primitive2d::TextLine aOverlineMode(drawinglayer::primitive2d::mapFontUnderlineToTextLine(rAction.GetOverline()));
            const drawinglayer::primitive2d::TextLine aUnderlineMode(drawinglayer::primitive2d::mapFontUnderlineToTextLine(rAction.GetUnderline()));
            const drawinglayer::primitive2d::TextStrikeout aTextStrikeout(drawinglayer::primitive2d::mapFontStrikeoutToTextStrikeout(rAction.GetStrikeout()));

            const bool bOverlineUsed(drawinglayer::primitive2d::TEXT_LINE_NONE != aOverlineMode);
            const bool bUnderlineUsed(drawinglayer::primitive2d::TEXT_LINE_NONE != aUnderlineMode);
            const bool bStrikeoutUsed(drawinglayer::primitive2d::TEXT_STRIKEOUT_NONE != aTextStrikeout);

            if(bUnderlineUsed || bStrikeoutUsed || bOverlineUsed)
            {
                std::vector< drawinglayer::primitive2d::BasePrimitive2D* > aTargetVector;
                basegfx::B2DVector aAlignmentOffset(0.0, 0.0);
                drawinglayer::attribute::FontAttribute aFontAttribute;
                basegfx::B2DHomMatrix aTextTransform;

                // fill parameters derived from current font
                createFontAttributeTransformAndAlignment(
                    aFontAttribute,
                    aTextTransform,
                    aAlignmentOffset,
                    rProperty);

                // add TextStartPosition
                aTextTransform.translate(rAction.GetStartPoint().X(), rAction.GetStartPoint().Y());

                // prepare TextLayouter (used in most cases)
                drawinglayer::primitive2d::TextLayouterDevice aTextLayouter;
                aTextLayouter.setFont(rProperty.getFont());

                if(bOverlineUsed)
                {
                    // create primitive geometry for overline
                    aTargetVector.push_back(
                        new drawinglayer::primitive2d::TextLinePrimitive2D(
                            aTextTransform,
                            fLineWidth,
                            aTextLayouter.getOverlineOffset(),
                            aTextLayouter.getOverlineHeight(),
                            aOverlineMode,
                            rProperty.getOverlineColor()));
                }

                if(bUnderlineUsed)
                {
                    // create primitive geometry for underline
                    aTargetVector.push_back(
                        new drawinglayer::primitive2d::TextLinePrimitive2D(
                            aTextTransform,
                            fLineWidth,
                            aTextLayouter.getUnderlineOffset(),
                            aTextLayouter.getUnderlineHeight(),
                            aUnderlineMode,
                            rProperty.getTextLineColor()));
                }

                if(bStrikeoutUsed)
                {
                    // create primitive geometry for strikeout
                    if(drawinglayer::primitive2d::TEXT_STRIKEOUT_SLASH == aTextStrikeout
                        || drawinglayer::primitive2d::TEXT_STRIKEOUT_X == aTextStrikeout)
                    {
                        // strikeout with character
                        const sal_Unicode aStrikeoutChar(
                            drawinglayer::primitive2d::TEXT_STRIKEOUT_SLASH == aTextStrikeout ? '/' : 'X');
                        const css::lang::Locale aLocale(LanguageTag(
                            rProperty.getLanguageType()).getLocale());

                        aTargetVector.push_back(
                            new drawinglayer::primitive2d::TextCharacterStrikeoutPrimitive2D(
                                aTextTransform,
                                fLineWidth,
                                rProperty.getTextColor(),
                                aStrikeoutChar,
                                aFontAttribute,
                                aLocale));
                    }
                    else
                    {
                        // strikeout with geometry
                        aTargetVector.push_back(
                            new drawinglayer::primitive2d::TextGeometryStrikeoutPrimitive2D(
                                aTextTransform,
                                fLineWidth,
                                rProperty.getTextColor(),
                                aTextLayouter.getUnderlineHeight(),
                                aTextLayouter.getStrikeoutOffset(),
                                aTextStrikeout));
                    }
                }

                if(!aTargetVector.empty())
                {
                    // add created text primitive to target
                    if(rProperty.getTransformation().isIdentity())
                    {
                        for(size_t a(0); a < aTargetVector.size(); a++)
                        {
                            rTarget.append(aTargetVector[a]);
                        }
                    }
                    else
                    {
                        // when a transformation is set, embed to it
                        drawinglayer::primitive2d::Primitive2DContainer xTargets(aTargetVector.size());

                        for(size_t a(0); a < aTargetVector.size(); a++)
                        {
                            xTargets[a] = drawinglayer::primitive2d::Primitive2DReference(aTargetVector[a]);
                        }

                        rTarget.append(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                rProperty.getTransformation(),
                                xTargets));
                    }
                }
            }
        }

    }

    /** This is the main interpreter method. It is designed to handle the given Metafile
        completely inside the given context and target. It may use and modify the context and
        target. This design allows to call itself recursively which adapted contexts and
        targets as e.g. needed for the MetaActionType::FLOATTRANSPARENT where the content is expressed
        as a metafile as sub-content.

        This interpreter is as free of VCL functionality as possible. It uses VCL data classes
        (else reading the data would not be possible), but e.g. does NOT use a local OutputDevice
        as most other MetaFile interpreters/exporters do to hold and work with the current context.
        This is necessary to be able to get away from the strong internal VCL-binding.

        It tries to combine e.g. pixel and/or point actions and to stitch together single line primitives
        where possible (which is not trivial with the possible line geometry definitions).

        It tries to handle clipping no longer as Regions and spans of Rectangles, but as PolyPolygon
        ClipRegions with (where possible) high precision by using the best possible data quality
        from the Region. The vcl::Region is unavoidable as data container, but nowadays allows the transport
        of Polygon-based clip regions. Where this is not used, a Polygon is constructed from the
        vcl::Region ranges. All primitive clipping uses the MaskPrimitive2D with Polygon-based clipping.

        I have marked the single MetaActions with:

        SIMPLE, DONE:
        Simple, e.g nothing to do or value setting in the context

        CHECKED, WORKS WELL:
        Thoroughly tested with extra written test code which created a replacement
        Metafile just to test this action in various combinations

        NEEDS IMPLEMENTATION:
        Not implemented and asserted, but also no usage found, neither in own Metafile
        creations, nor in EMF/WMF imports (checked with a whole bunch of critical EMF/WMF
        bugdocs)

        For more comments, see the single action implementations.
    */
    void interpretMetafile(
        const GDIMetaFile& rMetaFile,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders,
        const drawinglayer::geometry::ViewInformation2D& rViewInformation)
    {
        const size_t nCount(rMetaFile.GetActionSize());

        for(size_t nAction(0); nAction < nCount; nAction++)
        {
            MetaAction* pAction = rMetaFile.GetAction(nAction);

            switch(pAction->GetType())
            {
                case MetaActionType::NONE :
                {
                    /** SIMPLE, DONE */
                    break;
                }
                case MetaActionType::PIXEL :
                {
                    /** CHECKED, WORKS WELL */
                    std::vector< basegfx::B2DPoint > aPositions;
                    Color aLastColor(COL_BLACK);

                    while(MetaActionType::PIXEL == pAction->GetType() && nAction < nCount)
                    {
                        const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pAction);

                        if(pA->GetColor() != aLastColor)
                        {
                            if(!aPositions.empty())
                            {
                                createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), aLastColor.getBColor());
                                aPositions.clear();
                            }

                            aLastColor = pA->GetColor();
                        }

                        const Point& rPoint = pA->GetPoint();
                        aPositions.push_back(basegfx::B2DPoint(rPoint.X(), rPoint.Y()));
                        nAction++; if(nAction < nCount) pAction = rMetaFile.GetAction(nAction);
                    }

                    nAction--;

                    if(!aPositions.empty())
                    {
                        createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), aLastColor.getBColor());
                    }

                    break;
                }
                case MetaActionType::POINT :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        std::vector< basegfx::B2DPoint > aPositions;

                        while(MetaActionType::POINT == pAction->GetType() && nAction < nCount)
                        {
                            const MetaPointAction* pA = static_cast<const MetaPointAction*>(pAction);
                            const Point& rPoint = pA->GetPoint();
                            aPositions.push_back(basegfx::B2DPoint(rPoint.X(), rPoint.Y()));
                            nAction++; if(nAction < nCount) pAction = rMetaFile.GetAction(nAction);
                        }

                        nAction--;

                        if(!aPositions.empty())
                        {
                            createPointArrayPrimitive(aPositions, rTargetHolders.Current(), rPropertyHolders.Current(), rPropertyHolders.Current().getLineColor());
                        }
                    }

                    break;
                }
                case MetaActionType::LINE :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        basegfx::B2DPolygon aLinePolygon;
                        LineInfo aLineInfo;

                        while(MetaActionType::LINE == pAction->GetType() && nAction < nCount)
                        {
                            const MetaLineAction* pA = static_cast<const MetaLineAction*>(pAction);
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
                                    aLineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE); // It were lines; force to NONE
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

                            nAction++; if(nAction < nCount) pAction = rMetaFile.GetAction(nAction);
                        }

                        nAction--;

                        if(aLinePolygon.count())
                        {
                            aLineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE); // It were lines; force to NONE
                            createLinePrimitive(aLinePolygon, aLineInfo, rTargetHolders.Current(), rPropertyHolders.Current());
                        }
                    }

                    break;
                }
                case MetaActionType::RECT :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaRectAction* pA = static_cast<const MetaRectAction*>(pAction);
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
                case MetaActionType::ROUNDRECT :
                {
                    /** CHECKED, WORKS WELL */
                    /** The original OutputDevice::DrawRect paints nothing when nHor or nVer is zero; but just
                        because the tools::Polygon operator creating the rounding does produce nonsense. I assume
                        this an error and create an unrounded rectangle in that case (implicit in
                        createPolygonFromRect)
                     */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pAction);
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
                case MetaActionType::ELLIPSE :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaEllipseAction* pA = static_cast<const MetaEllipseAction*>(pAction);
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
                case MetaActionType::ARC :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                        const tools::Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_ARC);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlinePrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::PIE :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPieAction* pA = static_cast<const MetaPieAction*>(pAction);
                        const tools::Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_PIE);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::CHORD :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaChordAction* pA = static_cast<const MetaChordAction*>(pAction);
                        const tools::Polygon aToolsPoly(pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint(), POLY_CHORD);
                        const basegfx::B2DPolygon aOutline(aToolsPoly.getB2DPolygon());

                        createHairlineAndFillPrimitive(aOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::POLYLINE :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineColorActive())
                    {
                        const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pAction);
                        createLinePrimitive(pA->GetPolygon().getB2DPolygon(), pA->GetLineInfo(), rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::POLYGON :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPolygonAction* pA = static_cast<const MetaPolygonAction*>(pAction);
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
                case MetaActionType::POLYPOLYGON :
                {
                    /** CHECKED, WORKS WELL */
                    if(rPropertyHolders.Current().getLineOrFillActive())
                    {
                        const MetaPolyPolygonAction* pA = static_cast<const MetaPolyPolygonAction*>(pAction);
                        basegfx::B2DPolyPolygon aPolyPolygonOutline(pA->GetPolyPolygon().getB2DPolyPolygon());

                        // the metafile play interprets the single polygons from MetaPolyPolygonAction
                        // always as closed and always paints an edge from last to first point,
                        // so force to closed here to emulate that
                        for(sal_uInt32 b(0); b < aPolyPolygonOutline.count(); b++)
                        {
                            basegfx::B2DPolygon aPolygonOutline(aPolyPolygonOutline.getB2DPolygon(b));

                            if(aPolygonOutline.count() > 1 && !aPolygonOutline.isClosed())
                            {
                                aPolygonOutline.setClosed(true);
                                aPolyPolygonOutline.setB2DPolygon(b, aPolygonOutline);
                            }
                        }

                        createHairlineAndFillPrimitive(aPolyPolygonOutline, rTargetHolders.Current(), rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::TEXT :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                    sal_uInt32 nTextLength(pA->GetLen());
                    const sal_uInt32 nTextIndex(pA->GetIndex());
                    const sal_uInt32 nStringLength(pA->GetText().getLength());

                    if(nTextLength + nTextIndex > nStringLength)
                    {
                        nTextLength = nStringLength - nTextIndex;
                    }

                    if(nTextLength && rPropertyHolders.Current().getTextColorActive())
                    {
                        const std::vector< double > aDXArray{};
                        processMetaTextAction(
                            pA->GetPoint(),
                            pA->GetText(),
                            nTextIndex,
                            nTextLength,
                            aDXArray,
                            rTargetHolders.Current(),
                            rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::TEXTARRAY :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTextArrayAction* pA = static_cast<const MetaTextArrayAction*>(pAction);
                    sal_uInt32 nTextLength(pA->GetLen());
                    const sal_uInt32 nTextIndex(pA->GetIndex());
                    const sal_uInt32 nStringLength(pA->GetText().getLength());

                    if(nTextLength + nTextIndex > nStringLength)
                    {
                        nTextLength = nTextIndex > nStringLength ? 0 : nStringLength - nTextIndex;
                    }

                    if(nTextLength && rPropertyHolders.Current().getTextColorActive())
                    {
                        // preapare DXArray (if used)
                        std::vector< double > aDXArray;
                        long* pDXArray = pA->GetDXArray();

                        if(pDXArray)
                        {
                            aDXArray.reserve(nTextLength);

                            for(sal_uInt32 a(0); a < nTextLength; a++)
                            {
                                aDXArray.push_back((double)(*(pDXArray + a)));
                            }
                        }

                        processMetaTextAction(
                            pA->GetPoint(),
                            pA->GetText(),
                            nTextIndex,
                            nTextLength,
                            aDXArray,
                            rTargetHolders.Current(),
                            rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::STRETCHTEXT :
                {
                    // #i108440# StarMath uses MetaStretchTextAction, thus support is needed.
                    // It looks as if it pretty never really uses a width different from
                    // the default text-layout width, but it's not possible to be sure.
                    // Implemented getting the DXArray and checking for scale at all. If
                    // scale is more than 3.5% different, scale the DXArray before usage.
                    // New status:

                    /** CHECKED, WORKS WELL */
                    const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pAction);
                    sal_uInt32 nTextLength(pA->GetLen());
                    const sal_uInt32 nTextIndex(pA->GetIndex());
                    const sal_uInt32 nStringLength(pA->GetText().getLength());

                    if(nTextLength + nTextIndex > nStringLength)
                    {
                        nTextLength = nStringLength - nTextIndex;
                    }

                    if(nTextLength && rPropertyHolders.Current().getTextColorActive())
                    {
                        drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                        aTextLayouterDevice.setFont(rPropertyHolders.Current().getFont());

                        ::std::vector< double > aTextArray(
                            aTextLayouterDevice.getTextArray(
                                pA->GetText(),
                                nTextIndex,
                                nTextLength));

                        if(!aTextArray.empty())
                        {
                            const double fTextLength(aTextArray.back());

                            if(0.0 != fTextLength && pA->GetWidth())
                            {
                                const double fRelative(pA->GetWidth() / fTextLength);

                                if(fabs(fRelative - 1.0) >= 0.035)
                                {
                                    // when derivation is more than 3,5% from default text size,
                                    // scale the DXArray
                                    for(size_t a(0); a < aTextArray.size(); a++)
                                    {
                                        aTextArray[a] *= fRelative;
                                    }
                                }
                            }
                        }

                        processMetaTextAction(
                            pA->GetPoint(),
                            pA->GetText(),
                            nTextIndex,
                            nTextLength,
                            aTextArray,
                            rTargetHolders.Current(),
                            rPropertyHolders.Current());
                    }

                    break;
                }
                case MetaActionType::TEXTRECT :
                {
                    /** CHECKED, WORKS WELL */
                    // OSL_FAIL("MetaActionType::TEXTRECT requested (!)");
                    const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                    const Rectangle& rRectangle = pA->GetRect();
                    const sal_uInt32 nStringLength(pA->GetText().getLength());

                    if(!rRectangle.IsEmpty() && 0 != nStringLength)
                    {
                        // The problem with this action is that it describes unlayouted text
                        // and the layout capabilities are in EditEngine/Outliner in SVX. The
                        // same problem is true for VCL which internally has implementations
                        // to layout text in this case. There exists even a call
                        // OutputDevice::AddTextRectActions(...) to create the needed actions
                        // as 'sub-content' of a Metafile. Unfortunately i do not have an
                        // OutputDevice here since this interpreter tries to work without
                        // VCL AFAP.
                        // Since AddTextRectActions is the only way as long as we do not have
                        // a simple text layouter available, i will try to add it to the
                        // TextLayouterDevice isolation.
                        drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
                        aTextLayouterDevice.setFont(rPropertyHolders.Current().getFont());
                        GDIMetaFile aGDIMetaFile;

                        aTextLayouterDevice.addTextRectActions(
                            rRectangle, pA->GetText(), pA->GetStyle(), aGDIMetaFile);

                        if(aGDIMetaFile.GetActionSize())
                        {
                            // create sub-content
                            drawinglayer::primitive2d::Primitive2DContainer xSubContent;
                            {
                                rTargetHolders.Push();

                                // for sub-Mteafile contents, do start with new, default render state
                                // #i124686# ...but copy font, this is already set accordingly
                                vcl::Font aTargetFont = rPropertyHolders.Current().getFont();
                                rPropertyHolders.PushDefault();
                                rPropertyHolders.Current().setFont(aTargetFont);

                                interpretMetafile(aGDIMetaFile, rTargetHolders, rPropertyHolders, rViewInformation);
                                xSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
                                rPropertyHolders.Pop();
                                rTargetHolders.Pop();
                            }

                            if(!xSubContent.empty())
                            {
                                // add with transformation
                                rTargetHolders.Current().append(
                                    new drawinglayer::primitive2d::TransformPrimitive2D(
                                        rPropertyHolders.Current().getTransformation(),
                                        xSubContent));
                            }
                        }
                    }

                    break;
                }
                case MetaActionType::BMP :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpAction* pA = static_cast<const MetaBmpAction*>(pAction);
                    const BitmapEx aBitmapEx(pA->GetBitmap());

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::BMPSCALE :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
                    const Bitmap aBitmapEx(pA->GetBitmap());

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::BMPSCALEPART :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaBmpScalePartAction* pA = static_cast<const MetaBmpScalePartAction*>(pAction);
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
                case MetaActionType::BMPEX :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMP */
                    const MetaBmpExAction* pA = static_cast<const MetaBmpExAction*>(pAction);
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    createBitmapExPrimitive(rBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::BMPEXSCALE :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMPSCALE */
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
                    const BitmapEx& rBitmapEx = pA->GetBitmapEx();

                    createBitmapExPrimitive(rBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::BMPEXSCALEPART :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMPSCALEPART */
                    const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);
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
                case MetaActionType::MASK :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMP */
                    /** Huh, no it isn't!? */
                    const MetaMaskAction* pA = static_cast<const MetaMaskAction*>(pAction);
                    const BitmapEx aBitmapEx(createMaskBmpEx(pA->GetBitmap(), pA->GetColor()));

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::MASKSCALE :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMPSCALE */
                    const MetaMaskScaleAction* pA = static_cast<const MetaMaskScaleAction*>(pAction);
                    const BitmapEx aBitmapEx(createMaskBmpEx(pA->GetBitmap(), pA->GetColor()));

                    createBitmapExPrimitive(aBitmapEx, pA->GetPoint(), pA->GetSize(), rTargetHolders.Current(), rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::MASKSCALEPART :
                {
                    /** CHECKED, WORKS WELL: Simply same as MetaActionType::BMPSCALEPART */
                    const MetaMaskScalePartAction* pA = static_cast<const MetaMaskScalePartAction*>(pAction);
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
                case MetaActionType::GRADIENT :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaGradientAction* pA = static_cast<const MetaGradientAction*>(pAction);
                    const Rectangle& rRectangle = pA->GetRect();

                    if(!rRectangle.IsEmpty())
                    {
                        basegfx::B2DRange aRange(rRectangle.Left(), rRectangle.Top(), rRectangle.Right(), rRectangle.Bottom());

                        if(!aRange.isEmpty())
                        {
                            const Gradient& rGradient = pA->GetGradient();
                            const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));
                            basegfx::B2DPolyPolygon aOutline(basegfx::tools::createPolygonFromRect(aRange));

                            if(aAttribute.getStartColor() == aAttribute.getEndColor())
                            {
                                // not really a gradient. Create filled rectangle
                                createFillPrimitive(
                                    aOutline,
                                    rTargetHolders.Current(),
                                    rPropertyHolders.Current());
                            }
                            else
                            {
                                // really a gradient
                                aRange.transform(rPropertyHolders.Current().getTransformation());
                                drawinglayer::primitive2d::Primitive2DContainer xGradient(1);

                                if(rPropertyHolders.Current().isRasterOpInvert())
                                {
                                    // use a special version of FillGradientPrimitive2D which creates
                                    // non-overlapping geometry on decomposition to make the old XOR
                                    // paint 'trick' work.
                                    xGradient[0] = drawinglayer::primitive2d::Primitive2DReference(
                                        new drawinglayer::primitive2d::NonOverlappingFillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));
                                }
                                else
                                {
                                    xGradient[0] = drawinglayer::primitive2d::Primitive2DReference(
                                        new drawinglayer::primitive2d::FillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));
                                }

                                // #i112300# clip against polygon representing the rectangle from
                                // the action. This is implicitly done using a temp Clipping in VCL
                                // when a MetaGradientAction is executed
                                aOutline.transform(rPropertyHolders.Current().getTransformation());
                                rTargetHolders.Current().append(
                                    new drawinglayer::primitive2d::MaskPrimitive2D(
                                        aOutline,
                                        xGradient));
                            }
                        }
                    }

                    break;
                }
                case MetaActionType::HATCH :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaHatchAction* pA = static_cast<const MetaHatchAction*>(pAction);
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
                                drawinglayer::primitive2d::Primitive2DContainer { aFillHatch }));
                    }

                    break;
                }
                case MetaActionType::WALLPAPER :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaWallpaperAction* pA = static_cast<const MetaWallpaperAction*>(pAction);
                    Rectangle aWallpaperRectangle(pA->GetRect());

                    if(!aWallpaperRectangle.IsEmpty())
                    {
                        const Wallpaper& rWallpaper = pA->GetWallpaper();
                           const WallpaperStyle eWallpaperStyle(rWallpaper.GetStyle());
                        basegfx::B2DRange aWallpaperRange(
                            aWallpaperRectangle.Left(), aWallpaperRectangle.Top(),
                            aWallpaperRectangle.Right(), aWallpaperRectangle.Bottom());

                        if(WallpaperStyle::NONE != eWallpaperStyle)
                        {
                            if(rWallpaper.IsBitmap())
                            {
                                // create bitmap background. Caution: This
                                // also will create gradient/color background(s)
                                // when the bitmap is transparent or not tiled
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
                case MetaActionType::CLIPREGION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaClipRegionAction* pA = static_cast<const MetaClipRegionAction*>(pAction);

                    if(pA->IsClipping())
                    {
                        // new clipping. Get tools::PolyPolygon and transform with current transformation
                        basegfx::B2DPolyPolygon aNewClipPolyPolygon(getB2DPolyPolygonFromRegion(pA->GetRegion()));

                        aNewClipPolyPolygon.transform(rPropertyHolders.Current().getTransformation());
                        HandleNewClipRegion(aNewClipPolyPolygon, rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        // end clipping
                        const basegfx::B2DPolyPolygon aEmptyPolyPolygon;

                        HandleNewClipRegion(aEmptyPolyPolygon, rTargetHolders, rPropertyHolders);
                    }

                    break;
                }
                case MetaActionType::ISECTRECTCLIPREGION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pAction);
                    const Rectangle& rRectangle = pA->GetRect();

                    if(rRectangle.IsEmpty())
                    {
                        // intersect with empty rectangle will always give empty
                        // ClipPolyPolygon; start new clipping with empty PolyPolygon
                        const basegfx::B2DPolyPolygon aEmptyPolyPolygon;

                        HandleNewClipRegion(aEmptyPolyPolygon, rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        // create transformed ClipRange
                        basegfx::B2DRange aClipRange(
                            rRectangle.Left(), rRectangle.Top(),
                            rRectangle.Right(), rRectangle.Bottom());

                        aClipRange.transform(rPropertyHolders.Current().getTransformation());

                        if(rPropertyHolders.Current().getClipPolyPolygonActive())
                        {
                            if(0 == rPropertyHolders.Current().getClipPolyPolygon().count())
                            {
                                // nothing to do, empty active clipPolyPolygon will stay
                                // empty when intersecting
                            }
                            else
                            {
                                // AND existing region and new ClipRange
                                const basegfx::B2DPolyPolygon aOriginalPolyPolygon(
                                    rPropertyHolders.Current().getClipPolyPolygon());
                                basegfx::B2DPolyPolygon aClippedPolyPolygon;

                                if(aOriginalPolyPolygon.count())
                                {
                                    aClippedPolyPolygon = basegfx::tools::clipPolyPolygonOnRange(
                                        aOriginalPolyPolygon,
                                        aClipRange,
                                        true,
                                        false);
                                }

                                if(aClippedPolyPolygon != aOriginalPolyPolygon)
                                {
                                    // start new clipping with intersected region
                                    HandleNewClipRegion(
                                        aClippedPolyPolygon,
                                        rTargetHolders,
                                        rPropertyHolders);
                                }
                            }
                        }
                        else
                        {
                            // start new clipping with ClipRange
                            const basegfx::B2DPolyPolygon aNewClipPolyPolygon(
                                basegfx::tools::createPolygonFromRect(aClipRange));

                            HandleNewClipRegion(aNewClipPolyPolygon, rTargetHolders, rPropertyHolders);
                        }
                    }

                    break;
                }
                case MetaActionType::ISECTREGIONCLIPREGION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaISectRegionClipRegionAction* pA = static_cast<const MetaISectRegionClipRegionAction*>(pAction);
                    const vcl::Region& rNewRegion = pA->GetRegion();

                    if(rNewRegion.IsEmpty())
                    {
                        // intersect with empty region will always give empty
                        // region; start new clipping with empty PolyPolygon
                        const basegfx::B2DPolyPolygon aEmptyPolyPolygon;

                        HandleNewClipRegion(aEmptyPolyPolygon, rTargetHolders, rPropertyHolders);
                    }
                    else
                    {
                        // get new ClipPolyPolygon, transform it with current transformation
                        basegfx::B2DPolyPolygon aNewClipPolyPolygon(getB2DPolyPolygonFromRegion(rNewRegion));
                        aNewClipPolyPolygon.transform(rPropertyHolders.Current().getTransformation());

                        if(rPropertyHolders.Current().getClipPolyPolygonActive())
                        {
                            if(0 == rPropertyHolders.Current().getClipPolyPolygon().count())
                            {
                                // nothing to do, empty active clipPolyPolygon will stay empty
                                // when intersecting with any region
                            }
                            else
                            {
                                // AND existing and new region
                                const basegfx::B2DPolyPolygon aOriginalPolyPolygon(
                                    rPropertyHolders.Current().getClipPolyPolygon());
                                basegfx::B2DPolyPolygon aClippedPolyPolygon;

                                if(aOriginalPolyPolygon.count())
                                {
                                    aClippedPolyPolygon = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                                        aOriginalPolyPolygon, aNewClipPolyPolygon, true, false);
                                }

                                if(aClippedPolyPolygon != aOriginalPolyPolygon)
                                {
                                    // start new clipping with intersected ClipPolyPolygon
                                    HandleNewClipRegion(aClippedPolyPolygon, rTargetHolders, rPropertyHolders);
                                }
                            }
                        }
                        else
                        {
                            // start new clipping with new ClipPolyPolygon
                            HandleNewClipRegion(aNewClipPolyPolygon, rTargetHolders, rPropertyHolders);
                        }
                    }

                    break;
                }
                case MetaActionType::MOVECLIPREGION :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaMoveClipRegionAction* pA = static_cast<const MetaMoveClipRegionAction*>(pAction);

                    if(rPropertyHolders.Current().getClipPolyPolygonActive())
                    {
                        if(0 == rPropertyHolders.Current().getClipPolyPolygon().count())
                        {
                            // nothing to do
                        }
                        else
                        {
                            const sal_Int32 nHor(pA->GetHorzMove());
                            const sal_Int32 nVer(pA->GetVertMove());

                            if(0 != nHor || 0 != nVer)
                            {
                                // prepare translation, add current transformation
                                basegfx::B2DVector aVector(pA->GetHorzMove(), pA->GetVertMove());
                                aVector *= rPropertyHolders.Current().getTransformation();
                                basegfx::B2DHomMatrix aTransform(
                                    basegfx::tools::createTranslateB2DHomMatrix(aVector));

                                // transform existing region
                                basegfx::B2DPolyPolygon aClipPolyPolygon(
                                    rPropertyHolders.Current().getClipPolyPolygon());

                                aClipPolyPolygon.transform(aTransform);
                                HandleNewClipRegion(aClipPolyPolygon, rTargetHolders, rPropertyHolders);
                            }
                        }
                    }

                    break;
                }
                case MetaActionType::LINECOLOR :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaLineColorAction* pA = static_cast<const MetaLineColorAction*>(pAction);
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setLineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setLineColor(pA->GetColor().getBColor());

                    break;
                }
                case MetaActionType::FILLCOLOR :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaFillColorAction* pA = static_cast<const MetaFillColorAction*>(pAction);
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setFillColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setFillColor(pA->GetColor().getBColor());

                    break;
                }
                case MetaActionType::TEXTCOLOR :
                {
                    /** SIMPLE, DONE */
                    const MetaTextColorAction* pA = static_cast<const MetaTextColorAction*>(pAction);
                    const bool bActivate(COL_TRANSPARENT != pA->GetColor().GetColor());

                    rPropertyHolders.Current().setTextColorActive(bActivate);
                    rPropertyHolders.Current().setTextColor(pA->GetColor().getBColor());

                    break;
                }
                case MetaActionType::TEXTFILLCOLOR :
                {
                    /** SIMPLE, DONE */
                    const MetaTextFillColorAction* pA = static_cast<const MetaTextFillColorAction*>(pAction);
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
                case MetaActionType::TEXTALIGN :
                {
                    /** SIMPLE, DONE */
                    const MetaTextAlignAction* pA = static_cast<const MetaTextAlignAction*>(pAction);
                    const TextAlign aNewTextAlign = pA->GetTextAlign();

                    // TextAlign is applied to the current font (as in
                    // OutputDevice::SetTextAlign which would be used when
                    // playing the Metafile)
                    if(rPropertyHolders.Current().getFont().GetAlign() != aNewTextAlign)
                    {
                        vcl::Font aNewFont(rPropertyHolders.Current().getFont());
                        aNewFont.SetAlign(aNewTextAlign);
                        rPropertyHolders.Current().setFont(aNewFont);
                    }

                    break;
                }
                case MetaActionType::MAPMODE :
                {
                    /** CHECKED, WORKS WELL */
                    // the most necessary MapMode to be interpreted is MAP_RELATIVE,
                    // but also the others may occur. Even not yet supported ones
                    // may need to be added here later
                    const MetaMapModeAction* pA = static_cast<const MetaMapModeAction*>(pAction);
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
                                OSL_FAIL("interpretMetafile: MetaActionType::MAPMODE with unsupported MapUnit (!)");
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
                case MetaActionType::FONT :
                {
                    /** SIMPLE, DONE */
                    const MetaFontAction* pA = static_cast<const MetaFontAction*>(pAction);
                    rPropertyHolders.Current().setFont(pA->GetFont());
                    Size aFontSize(pA->GetFont().GetSize());

                    if(0 == aFontSize.Height())
                    {
                        // this should not happen but i got Metafiles where this was the
                        // case. A height needs to be guessed (similar to OutputDevice::ImplNewFont())
                        vcl::Font aCorrectedFont(pA->GetFont());

                        // guess 16 pixel (as in VCL)
                        aFontSize = Size(0, 16);

                        // convert to target MapUnit if not pixels
                        aFontSize = OutputDevice::LogicToLogic(
                            aFontSize, MAP_PIXEL, rPropertyHolders.Current().getMapUnit());

                        aCorrectedFont.SetSize(aFontSize);
                        rPropertyHolders.Current().setFont(aCorrectedFont);
                    }

                    // older Metafiles have no MetaActionType::TEXTCOLOR which defines
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
                case MetaActionType::PUSH :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaPushAction* pA = static_cast<const MetaPushAction*>(pAction);
                    rPropertyHolders.Push(pA->GetFlags());

                    break;
                }
                case MetaActionType::POP :
                {
                    /** CHECKED, WORKS WELL */
                    const bool bRegionMayChange(rPropertyHolders.Current().getPushFlags() & PushFlags::CLIPREGION);
                    const bool bRasterOpMayChange(rPropertyHolders.Current().getPushFlags() & PushFlags::RASTEROP);

                    if(bRegionMayChange && rPropertyHolders.Current().getClipPolyPolygonActive())
                    {
                        // end evtl. clipping
                        const basegfx::B2DPolyPolygon aEmptyPolyPolygon;

                        HandleNewClipRegion(aEmptyPolyPolygon, rTargetHolders, rPropertyHolders);
                    }

                    if(bRasterOpMayChange && rPropertyHolders.Current().isRasterOpActive())
                    {
                        // end evtl. RasterOp
                        HandleNewRasterOp(ROP_OVERPAINT, rTargetHolders, rPropertyHolders);
                    }

                    rPropertyHolders.Pop();

                    if(bRasterOpMayChange && rPropertyHolders.Current().isRasterOpActive())
                    {
                        // start evtl. RasterOp
                        HandleNewRasterOp(rPropertyHolders.Current().getRasterOp(), rTargetHolders, rPropertyHolders);
                    }

                    if(bRegionMayChange && rPropertyHolders.Current().getClipPolyPolygonActive())
                    {
                        // start evtl. clipping
                        HandleNewClipRegion(
                            rPropertyHolders.Current().getClipPolyPolygon(), rTargetHolders, rPropertyHolders);
                    }

                    break;
                }
                case MetaActionType::RASTEROP :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaRasterOpAction* pA = static_cast<const MetaRasterOpAction*>(pAction);
                    const RasterOp aRasterOp = pA->GetRasterOp();

                    HandleNewRasterOp(aRasterOp, rTargetHolders, rPropertyHolders);

                    break;
                }
                case MetaActionType::Transparent :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaTransparentAction* pA = static_cast<const MetaTransparentAction*>(pAction);
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
                            const drawinglayer::primitive2d::Primitive2DContainer aSubContent(
                                rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current()));

                            // back to old target
                            rTargetHolders.Pop();

                            if(!aSubContent.empty())
                            {
                                rTargetHolders.Current().append(
                                    new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                        aSubContent,
                                        nTransparence * 0.01));
                            }
                        }
                    }

                    break;
                }
                case MetaActionType::EPS :
                {
                    /** CHECKED, WORKS WELL */
                    // To support this action, I have added a EpsPrimitive2D which will
                    // by default decompose to the Metafile replacement data. To support
                    // this EPS on screen, the renderer visualizing this has to support
                    // that primitive and visualize the Eps file (e.g. printing)
                    const MetaEPSAction* pA = static_cast<const MetaEPSAction*>(pAction);
                    const Rectangle aRectangle(pA->GetPoint(), pA->GetSize());

                    if(!aRectangle.IsEmpty())
                    {
                        // create object transform
                        basegfx::B2DHomMatrix aObjectTransform;

                        aObjectTransform.set(0, 0, aRectangle.GetWidth());
                        aObjectTransform.set(1, 1, aRectangle.GetHeight());
                        aObjectTransform.set(0, 2, aRectangle.Left());
                        aObjectTransform.set(1, 2, aRectangle.Top());

                        // add current transformation
                        aObjectTransform = rPropertyHolders.Current().getTransformation() * aObjectTransform;

                        // embed using EpsPrimitive
                        rTargetHolders.Current().append(
                            new drawinglayer::primitive2d::EpsPrimitive2D(
                                aObjectTransform,
                                pA->GetLink(),
                                pA->GetSubstitute()));
                    }

                    break;
                }
                case MetaActionType::REFPOINT :
                {
                    /** SIMPLE, DONE */
                    // only used for hatch and line pattern offsets, pretty much no longer
                    // supported today
                    // const MetaRefPointAction* pA = (const MetaRefPointAction*)pAction;
                    break;
                }
                case MetaActionType::TEXTLINECOLOR :
                {
                    /** SIMPLE, DONE */
                    const MetaTextLineColorAction* pA = static_cast<const MetaTextLineColorAction*>(pAction);
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setTextLineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setTextLineColor(pA->GetColor().getBColor());

                    break;
                }
                case MetaActionType::TEXTLINE :
                {
                    /** CHECKED, WORKS WELL */
                    // actually creates overline, underline and strikeouts, so
                    // these should be isolated from TextDecoratedPortionPrimitive2D
                    // to own primitives. Done, available now.
                    //
                    // This Metaaction seems not to be used (was not used in any
                    // checked files). It's used in combination with the current
                    // Font.
                    const MetaTextLineAction* pA = static_cast<const MetaTextLineAction*>(pAction);

                    proccessMetaTextLineAction(
                        *pA,
                        rTargetHolders.Current(),
                        rPropertyHolders.Current());

                    break;
                }
                case MetaActionType::FLOATTRANSPARENT :
                {
                    /** CHECKED, WORKS WELL */
                    const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pAction);
                    const basegfx::B2DRange aTargetRange(
                        pA->GetPoint().X(),
                        pA->GetPoint().Y(),
                        pA->GetPoint().X() + pA->GetSize().Width(),
                        pA->GetPoint().Y() + pA->GetSize().Height());

                    if(!aTargetRange.isEmpty())
                    {
                        const GDIMetaFile& rContent = pA->GetGDIMetaFile();

                        if(rContent.GetActionSize())
                        {
                            // create the sub-content with no embedding specific to the
                            // sub-metafile, this seems not to be used.
                            drawinglayer::primitive2d::Primitive2DContainer xSubContent;
                            {
                                rTargetHolders.Push();
                                // #i# for sub-Mteafile contents, do start with new, default render state
                                rPropertyHolders.PushDefault();
                                interpretMetafile(rContent, rTargetHolders, rPropertyHolders, rViewInformation);
                                xSubContent = rTargetHolders.Current().getPrimitive2DSequence(rPropertyHolders.Current());
                                rPropertyHolders.Pop();
                                rTargetHolders.Pop();
                            }

                            if(!xSubContent.empty())
                            {
                                // prepare sub-content transform
                                basegfx::B2DHomMatrix aSubTransform;

                                // create SourceRange
                                const basegfx::B2DRange aSourceRange(
                                    rContent.GetPrefMapMode().GetOrigin().X(),
                                    rContent.GetPrefMapMode().GetOrigin().Y(),
                                    rContent.GetPrefMapMode().GetOrigin().X() + rContent.GetPrefSize().Width(),
                                    rContent.GetPrefMapMode().GetOrigin().Y() + rContent.GetPrefSize().Height());

                                // apply mapping if aTargetRange and aSourceRange are not equal
                                if(!aSourceRange.equal(aTargetRange))
                                {
                                    aSubTransform.translate(-aSourceRange.getMinX(), -aSourceRange.getMinY());
                                    aSubTransform.scale(
                                        aTargetRange.getWidth() / (basegfx::fTools::equalZero(aSourceRange.getWidth()) ? 1.0 : aSourceRange.getWidth()),
                                        aTargetRange.getHeight() / (basegfx::fTools::equalZero(aSourceRange.getHeight()) ? 1.0 : aSourceRange.getHeight()));
                                    aSubTransform.translate(aTargetRange.getMinX(), aTargetRange.getMinY());
                                }

                                // apply general current transformation
                                aSubTransform = rPropertyHolders.Current().getTransformation() * aSubTransform;

                                // evtl. embed sub-content to its transformation
                                if(!aSubTransform.isIdentity())
                                {
                                    const drawinglayer::primitive2d::Primitive2DReference aEmbeddedTransform(
                                        new drawinglayer::primitive2d::TransformPrimitive2D(
                                            aSubTransform,
                                            xSubContent));

                                    xSubContent = drawinglayer::primitive2d::Primitive2DContainer { aEmbeddedTransform };
                                }

                                // check if gradient is a real gradient
                                const Gradient& rGradient = pA->GetGradient();
                                const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));

                                if(aAttribute.getStartColor() == aAttribute.getEndColor())
                                {
                                    // not really a gradient; create UnifiedTransparencePrimitive2D
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
                                            xSubContent,
                                            aAttribute.getStartColor().luminance()));
                                }
                                else
                                {
                                    // really a gradient. Create gradient sub-content (with correct scaling)
                                    basegfx::B2DRange aRange(aTargetRange);
                                    aRange.transform(rPropertyHolders.Current().getTransformation());

                                    // prepare gradient for transparent content
                                    const drawinglayer::primitive2d::Primitive2DReference xTransparence(
                                        new drawinglayer::primitive2d::FillGradientPrimitive2D(
                                            aRange,
                                            aAttribute));

                                    // create transparence primitive
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::TransparencePrimitive2D(
                                            xSubContent,
                                            drawinglayer::primitive2d::Primitive2DContainer { xTransparence }));
                                }
                            }
                        }
                    }

                    break;
                }
                case MetaActionType::GRADIENTEX :
                {
                    /** SIMPLE, DONE */
                    // This is only a data holder which is interpreted inside comment actions,
                    // see MetaActionType::COMMENT for more info
                    // const MetaGradientExAction* pA = (const MetaGradientExAction*)pAction;
                    break;
                }
                case MetaActionType::LAYOUTMODE :
                {
                    /** SIMPLE, DONE */
                    const MetaLayoutModeAction* pA = static_cast<const MetaLayoutModeAction*>(pAction);
                    rPropertyHolders.Current().setLayoutMode(pA->GetLayoutMode());
                    break;
                }
                case MetaActionType::TEXTLANGUAGE :
                {
                    /** SIMPLE, DONE */
                    const MetaTextLanguageAction* pA = static_cast<const MetaTextLanguageAction*>(pAction);
                    rPropertyHolders.Current().setLanguageType(pA->GetTextLanguage());
                    break;
                }
                case MetaActionType::OVERLINECOLOR :
                {
                    /** SIMPLE, DONE */
                    const MetaOverlineColorAction* pA = static_cast<const MetaOverlineColorAction*>(pAction);
                    const bool bActive(pA->IsSetting());

                    rPropertyHolders.Current().setOverlineColorActive(bActive);
                    if(bActive)
                        rPropertyHolders.Current().setOverlineColor(pA->GetColor().getBColor());

                    break;
                }
                case MetaActionType::COMMENT :
                {
                    /** CHECKED, WORKS WELL */
                    // I already implemented
                    //     XPATHFILL_SEQ_BEGIN, XPATHFILL_SEQ_END
                    //     XPATHSTROKE_SEQ_BEGIN, XPATHSTROKE_SEQ_END,
                    // but opted to remove these again; it works well without them
                    // and makes the code less dependent from those Metafile Add-Ons
                    const MetaCommentAction* pA = static_cast<const MetaCommentAction*>(pAction);

                    if (pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                    {
                        // XGRAD_SEQ_BEGIN, XGRAD_SEQ_END should be supported since the
                        // pure recorded paint of the gradients uses the XOR paint functionality
                        // ('trick'). This is (and will be) problematic with AntAliasing, so it's
                        // better to use this info
                        const MetaGradientExAction* pMetaGradientExAction = nullptr;
                        bool bDone(false);
                        sal_uInt32 b(nAction + 1);

                        for(; !bDone && b < nCount; b++)
                        {
                            pAction = rMetaFile.GetAction(b);

                            if(MetaActionType::GRADIENTEX == pAction->GetType())
                            {
                                pMetaGradientExAction = static_cast<const MetaGradientExAction*>(pAction);
                            }
                            else if(MetaActionType::COMMENT == pAction->GetType())
                            {
                                if (static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END"))
                                {
                                    bDone = true;
                                }
                            }
                        }

                        if(bDone && pMetaGradientExAction)
                        {
                            // consume actions and skip forward
                            nAction = b - 1;

                            // get geometry data
                            basegfx::B2DPolyPolygon aPolyPolygon(pMetaGradientExAction->GetPolyPolygon().getB2DPolyPolygon());

                            if(aPolyPolygon.count())
                            {
                                // transform geometry
                                aPolyPolygon.transform(rPropertyHolders.Current().getTransformation());

                                // get and check if gradient is a real gradient
                                const Gradient& rGradient = pMetaGradientExAction->GetGradient();
                                const drawinglayer::attribute::FillGradientAttribute aAttribute(createFillGradientAttribute(rGradient));

                                if(aAttribute.getStartColor() == aAttribute.getEndColor())
                                {
                                    // not really a gradient
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                            aPolyPolygon,
                                            aAttribute.getStartColor()));
                                }
                                else
                                {
                                    // really a gradient
                                    rTargetHolders.Current().append(
                                        new drawinglayer::primitive2d::PolyPolygonGradientPrimitive2D(
                                            aPolyPolygon,
                                            aAttribute));
                                }
                            }
                        }
                    }

                    break;
                }
                default:
                {
                    OSL_FAIL("Unknown MetaFile Action (!)");
                    break;
                }
            }
        }
    }
} // end of anonymous namespace



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DContainer MetafilePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // prepare target and porperties; each will have one default entry
            TargetHolders aTargetHolders;
            PropertyHolders aPropertyHolders;

            // set target MapUnit at Properties
            aPropertyHolders.Current().setMapUnit(getMetaFile().GetPrefMapMode().GetMapUnit());

            // interpret the Metafile
            interpretMetafile(getMetaFile(), aTargetHolders, aPropertyHolders, rViewInformation);

            // get the content. There should be only one target, as in the start condition,
            // but iterating will be the right thing to do when some push/pop is not closed
            Primitive2DContainer xRetval;

            while(aTargetHolders.size() > 1)
            {
                xRetval.append(
                    aTargetHolders.Current().getPrimitive2DSequence(aPropertyHolders.Current()));
                aTargetHolders.Pop();
            }

            xRetval.append(
                aTargetHolders.Current().getPrimitive2DSequence(aPropertyHolders.Current()));

            if(!xRetval.empty())
            {
                // get target size
                const Rectangle aMtfTarget(getMetaFile().GetPrefMapMode().GetOrigin(), getMetaFile().GetPrefSize());

                // create transformation
                basegfx::B2DHomMatrix aAdaptedTransform;

                aAdaptedTransform.translate(-aMtfTarget.Left(), -aMtfTarget.Top());
                aAdaptedTransform.scale(
                    aMtfTarget.getWidth() ? 1.0 / aMtfTarget.getWidth() : 1.0,
                    aMtfTarget.getHeight() ? 1.0 / aMtfTarget.getHeight() : 1.0);
                aAdaptedTransform = getTransform() * aAdaptedTransform;

                // embed to target transformation
                const Primitive2DReference aEmbeddedTransform(
                    new TransformPrimitive2D(
                        aAdaptedTransform,
                        xRetval));

                xRetval = Primitive2DContainer { aEmbeddedTransform };
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
                const MetafilePrimitive2D& rCompare = static_cast<const MetafilePrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getMetaFile() == rCompare.getMetaFile());
            }

            return false;
        }

        basegfx::B2DRange MetafilePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // use own implementation to quickly answer the getB2DRange question. The
            // MetafilePrimitive2D assumes that all geometry is inside of the shape. If
            // this is not the case (i have already seen some wrong Metafiles) it should
            // be embedded to a MaskPrimitive2D
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());

            return aRetval;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(MetafilePrimitive2D, PRIMITIVE2D_ID_METAFILEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
