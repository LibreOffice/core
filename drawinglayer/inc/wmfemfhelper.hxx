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

#ifndef INCLUDED_DRAWINGLAYER_INC_WMFEMFHELPER_HXX
#define INCLUDED_DRAWINGLAYER_INC_WMFEMFHELPER_HXX

#include <sal/config.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/font.hxx>
#include <vcl/outdevstate.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

// predefines
namespace drawinglayer { namespace geometry { class ViewInformation2D; }}
class GDIMetaFile;
namespace wmfemfhelper { class PropertyHolder; }

namespace wmfemfhelper
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
        TargetHolder();
        ~TargetHolder();
        sal_uInt32 size() const;
        void append(drawinglayer::primitive2d::BasePrimitive2D* pCandidate);
        drawinglayer::primitive2d::Primitive2DContainer getPrimitive2DSequence(const PropertyHolder& rPropertyHolder);
    };
}

namespace wmfemfhelper
{
    /** Helper class which builds a stack on the TargetHolder class */
    class TargetHolders
    {
    private:
        std::vector< TargetHolder* >          maTargetHolders;

    public:
        TargetHolders();
        sal_uInt32 size() const;
        void Push();
        void Pop();
        TargetHolder& Current();
        ~TargetHolders();
    };
}

namespace wmfemfhelper
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
        ComplexTextLayoutFlags   mnLayoutMode;
        LanguageType            maLanguageType;
        PushFlags               mnPushFlags;

        /// contains all active markers
        bool                    mbLineColor : 1;
        bool                    mbFillColor : 1;
        bool                    mbTextColor : 1;
        bool                    mbTextFillColor : 1;
        bool                    mbTextLineColor : 1;
        bool                    mbOverlineColor : 1;
        bool                    mbClipPolyPolygonActive : 1;

    public:
        PropertyHolder();

        /// read/write accesses
        const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
        void setTransformation(const basegfx::B2DHomMatrix& rNew) { if (rNew != maTransformation) maTransformation = rNew; }

        MapUnit getMapUnit() const { return maMapUnit; }
        void setMapUnit(MapUnit eNew) { if (eNew != maMapUnit) maMapUnit = eNew; }

        const basegfx::BColor& getLineColor() const { return maLineColor; }
        void setLineColor(const basegfx::BColor& rNew) { if (rNew != maLineColor) maLineColor = rNew; }
        bool getLineColorActive() const { return mbLineColor; }
        void setLineColorActive(bool bNew) { if (bNew != mbLineColor) mbLineColor = bNew; }

        const basegfx::BColor& getFillColor() const { return maFillColor; }
        void setFillColor(const basegfx::BColor& rNew) { if (rNew != maFillColor) maFillColor = rNew; }
        bool getFillColorActive() const { return mbFillColor; }
        void setFillColorActive(bool bNew) { if (bNew != mbFillColor) mbFillColor = bNew; }

        const basegfx::BColor& getTextColor() const { return maTextColor; }
        void setTextColor(const basegfx::BColor& rNew) { if (rNew != maTextColor) maTextColor = rNew; }
        bool getTextColorActive() const { return mbTextColor; }
        void setTextColorActive(bool bNew) { if (bNew != mbTextColor) mbTextColor = bNew; }

        const basegfx::BColor& getTextFillColor() const { return maTextFillColor; }
        void setTextFillColor(const basegfx::BColor& rNew) { if (rNew != maTextFillColor) maTextFillColor = rNew; }
        bool getTextFillColorActive() const { return mbTextFillColor; }
        void setTextFillColorActive(bool bNew) { if (bNew != mbTextFillColor) mbTextFillColor = bNew; }

        const basegfx::BColor& getTextLineColor() const { return maTextLineColor; }
        void setTextLineColor(const basegfx::BColor& rNew) { if (rNew != maTextLineColor) maTextLineColor = rNew; }
        bool getTextLineColorActive() const { return mbTextLineColor; }
        void setTextLineColorActive(bool bNew) { if (bNew != mbTextLineColor) mbTextLineColor = bNew; }

        const basegfx::BColor& getOverlineColor() const { return maOverlineColor; }
        void setOverlineColor(const basegfx::BColor& rNew) { if (rNew != maOverlineColor) maOverlineColor = rNew; }
        bool getOverlineColorActive() const { return mbOverlineColor; }
        void setOverlineColorActive(bool bNew) { if (bNew != mbOverlineColor) mbOverlineColor = bNew; }

        const basegfx::B2DPolyPolygon& getClipPolyPolygon() const { return maClipPolyPoygon; }
        void setClipPolyPolygon(const basegfx::B2DPolyPolygon& rNew) { if (rNew != maClipPolyPoygon) maClipPolyPoygon = rNew; }
        bool getClipPolyPolygonActive() const { return mbClipPolyPolygonActive; }
        void setClipPolyPolygonActive(bool bNew) { if (bNew != mbClipPolyPolygonActive) mbClipPolyPolygonActive = bNew; }

        const vcl::Font& getFont() const { return maFont; }
        void setFont(const vcl::Font& rFont) { if (rFont != maFont) maFont = rFont; }

        const RasterOp& getRasterOp() const { return maRasterOp; }
        void setRasterOp(const RasterOp& rRasterOp) { if (rRasterOp != maRasterOp) maRasterOp = rRasterOp; }
        bool isRasterOpInvert() const { return (RasterOp::Xor == maRasterOp || RasterOp::Invert == maRasterOp); }
        bool isRasterOpForceBlack() const { return RasterOp::N0 == maRasterOp; }
        bool isRasterOpActive() const { return isRasterOpInvert() || isRasterOpForceBlack(); }

        ComplexTextLayoutFlags getLayoutMode() const { return mnLayoutMode; }
        void setLayoutMode(ComplexTextLayoutFlags nNew) { if (nNew != mnLayoutMode) mnLayoutMode = nNew; }

        LanguageType getLanguageType() const { return maLanguageType; }
        void setLanguageType(LanguageType aNew) { if (aNew != maLanguageType) maLanguageType = aNew; }

        PushFlags getPushFlags() const { return mnPushFlags; }
        void setPushFlags(PushFlags nNew) { if (nNew != mnPushFlags) mnPushFlags = nNew; }

        bool getLineOrFillActive() const { return (mbLineColor || mbFillColor); }
    };
}

namespace wmfemfhelper
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
        PropertyHolders();
        void PushDefault();
        void Push(PushFlags nPushFlags);
        void Pop();
        PropertyHolder& Current();
        ~PropertyHolders();
    };
}

namespace wmfemfhelper
{
    drawinglayer::primitive2d::Primitive2DContainer interpretMetafile(
        const GDIMetaFile& rMetaFile,
        const drawinglayer::geometry::ViewInformation2D& rViewInformation);

    void HandleNewClipRegion(
        const basegfx::B2DPolyPolygon& rClipPolyPolygon,
        TargetHolders& rTargetHolders,
        PropertyHolders& rPropertyHolders);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
