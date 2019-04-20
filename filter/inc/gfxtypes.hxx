/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <rtl/ustring.hxx>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace svgi
{

struct ARGBColor
{
    static double toDoubleColor( sal_uInt8 val ) { return val/255.0; }

    ARGBColor() : a(1.0), r(0.0), g(0.0), b(0.0)
    {}
    explicit ARGBColor(double fGrey) : a(1.0), r(fGrey), g(fGrey), b(fGrey)
    {}
    ARGBColor( int r_, int g_, int b_ ) :
        a(1.0),
        r(toDoubleColor(sal::static_int_cast<sal_uInt8>(r_))),
        g(toDoubleColor(sal::static_int_cast<sal_uInt8>(g_))),
        b(toDoubleColor(sal::static_int_cast<sal_uInt8>(b_)))
    {}
    double a;
    double r;
    double g;
    double b;
};
inline bool operator==( const ARGBColor& rLHS, const ARGBColor& rRHS )
{ return rLHS.a==rRHS.a && rLHS.r==rRHS.r && rLHS.g==rRHS.g && rLHS.b==rRHS.b; }
inline bool operator!=( const ARGBColor& rLHS, const ARGBColor& rRHS )
{ return !(rLHS==rRHS); }

struct GradientStop
{
    GradientStop() : maStopColor(), mnStopPosition(0.0)
    {}
    ARGBColor     maStopColor;
    double        mnStopPosition;
};

struct Gradient
{
    enum GradientType { LINEAR, RADIAL};
    std::vector<std::size_t> maStops;
    basegfx::B2DHomMatrix maTransform;
    GradientType          meType;
    union
    {
        struct
        {
            double                mfX1;
            double                mfX2;
            double                mfY1;
            double                mfY2;
        } linear;
        struct
        {
            double                mfCX;
            double                mfCY;
            double                mfFX;
            double                mfFY;
            double                mfR;
        } radial;
    } maCoords;
    sal_Int32             mnId;
    bool                  mbBoundingBoxUnits;

//    explicit Gradient(GradientType eType) : maStops(), maTransform(), meType(eType), maCoords.mfCX(0.0), maCoords.mfCY(0.0), maCoords.mfFX(0.0), maCoords.mfFY(0.0), maCoords.mfR(0.0), mnId(0), mbBoundingBoxUnits(false)
    explicit Gradient(GradientType eType) : maStops(), maTransform(), meType(eType), mnId(0), mbBoundingBoxUnits(false)
    {
        maCoords.radial.mfCX = 0.0;
        maCoords.radial.mfCY = 0.0;
        maCoords.radial.mfFX = 0.0;
        maCoords.radial.mfFY = 0.0;
        maCoords.radial.mfR  = 0.0;
    }
};

inline bool operator==( const Gradient& rLHS, const Gradient& rRHS )
{
    if( rLHS.meType != rRHS.meType )
        return false;
    if( rLHS.meType == Gradient::LINEAR )
        return rLHS.mbBoundingBoxUnits==rRHS.mbBoundingBoxUnits && rLHS.maStops==rRHS.maStops &&
            rLHS.maCoords.linear.mfX1 == rRHS.maCoords.linear.mfX1 && rLHS.maCoords.linear.mfX2 == rRHS.maCoords.linear.mfX2 &&
            rLHS.maCoords.linear.mfY1 == rRHS.maCoords.linear.mfY1 && rLHS.maCoords.linear.mfY2 == rRHS.maCoords.linear.mfY2;
    else
        return rLHS.mbBoundingBoxUnits==rRHS.mbBoundingBoxUnits && rLHS.maStops==rRHS.maStops &&
            rLHS.maCoords.radial.mfCX == rRHS.maCoords.radial.mfCX && rLHS.maCoords.radial.mfCY == rRHS.maCoords.radial.mfCY &&
            rLHS.maCoords.radial.mfFX == rRHS.maCoords.radial.mfFX && rLHS.maCoords.radial.mfFY == rRHS.maCoords.radial.mfFY &&
            rLHS.maCoords.radial.mfR == rRHS.maCoords.radial.mfR;
}

enum PaintType
{
    NONE,
    SOLID,
    GRADIENT,
    DASH
};

enum FillRule
{
    NON_ZERO,
    EVEN_ODD
};

enum TextAlign
{
    BEFORE,
    CENTER,
    AFTER
};

enum CapStyle
{
    BUTT,
    RECT,
    ROUND
};

struct State
{
    State() :
        maCTM(),
        maTransform(),
        maViewport(),
        maViewBox(),
        mbIsText(false),
        maFontFamily(), // app-default
        mnFontSize(12),
        mnParentFontSize(12),
        maFontStyle("normal"),
        maFontVariant("normal"),
        mnFontWeight(400.0),
        meTextAnchor(BEFORE),
        maCurrentColor(0.0),
        mbVisibility(true),
        meFillType(SOLID),
        mnFillOpacity(1.0),
        mnOpacity(1.0),
        meStrokeType(NONE),
        mnStrokeOpacity(1.0),
        mnViewportFillOpacity(1.0),
        maFillColor(0.0),
        maFillGradient(Gradient::LINEAR),
        meFillRule(NON_ZERO),
        maStrokeColor(0.0),
        maStrokeGradient(Gradient::LINEAR),
        maDashArray(),
        mnDashOffset(0.0),
        meLineCap(BUTT),
        meLineJoin(basegfx::B2DLineJoin::Miter),
        mnMiterLimit(4.0),
        mnStrokeWidth(1.0),
        maViewportFillColor(1.0),
        maViewportFillGradient(Gradient::LINEAR),
        mnStyleId(0)
    {}

    basegfx::B2DHomMatrix       maCTM;
    basegfx::B2DHomMatrix       maTransform;
    basegfx::B2DRange           maViewport;
    basegfx::B2DRange           maViewBox;

    bool                        mbIsText;
    OUString                    maFontFamily;

    double                      mnFontSize;
    double                      mnParentFontSize;
    OUString                    maFontStyle;
    OUString                    maFontVariant;
    double                      mnFontWeight;

    TextAlign                   meTextAnchor; // text-anchor

    ARGBColor                   maCurrentColor;
    bool                        mbVisibility;

    PaintType                   meFillType;
    double                      mnFillOpacity;
    double                      mnOpacity;
    PaintType                   meStrokeType;
    double                      mnStrokeOpacity;
    double                      mnViewportFillOpacity;

    ARGBColor                   maFillColor;
    Gradient                    maFillGradient;
    FillRule                    meFillRule;

    ARGBColor                   maStrokeColor;
    Gradient                    maStrokeGradient;
    std::vector<double>         maDashArray;
    double                      mnDashOffset;
    CapStyle                    meLineCap;
    basegfx::B2DLineJoin        meLineJoin;
    double                      mnMiterLimit;
    double                      mnStrokeWidth;

    ARGBColor                   maViewportFillColor;
    Gradient                    maViewportFillGradient;

    sal_Int32                   mnStyleId;
};

inline bool operator==(const State& rLHS, const State& rRHS )
{
    return rLHS.maCTM==rRHS.maCTM &&
        rLHS.maTransform==rRHS.maTransform &&
        rLHS.maViewport==rRHS.maViewport &&
        rLHS.maViewBox==rRHS.maViewBox &&
        rLHS.mbIsText==rRHS.mbIsText &&
        rLHS.maFontFamily==rRHS.maFontFamily &&
        rLHS.mnFontSize==rRHS.mnFontSize &&
        rLHS.mnParentFontSize==rRHS.mnParentFontSize &&
        rLHS.maFontStyle==rRHS.maFontStyle &&
        rLHS.maFontVariant==rRHS.maFontVariant &&
        rLHS.mnFontWeight==rRHS.mnFontWeight &&
        rLHS.meTextAnchor==rRHS.meTextAnchor &&
        rLHS.maCurrentColor==rRHS.maCurrentColor &&
        rLHS.mbVisibility==rRHS.mbVisibility &&
        rLHS.meFillType==rRHS.meFillType &&
        rLHS.mnFillOpacity==rRHS.mnFillOpacity &&
        rLHS.mnOpacity==rRHS.mnOpacity &&
        rLHS.meStrokeType==rRHS.meStrokeType &&
        rLHS.mnStrokeOpacity==rRHS.mnStrokeOpacity &&
        rLHS.mnViewportFillOpacity==rRHS.mnViewportFillOpacity &&
        rLHS.maFillColor==rRHS.maFillColor &&
        rLHS.maFillGradient==rRHS.maFillGradient &&
        rLHS.meFillRule==rRHS.meFillRule &&
        rLHS.maStrokeColor==rRHS.maStrokeColor &&
        rLHS.maStrokeGradient==rRHS.maStrokeGradient &&
        rLHS.maDashArray==rRHS.maDashArray &&
        rLHS.mnDashOffset==rRHS.mnDashOffset &&
        rLHS.meLineCap==rRHS.meLineCap &&
        rLHS.meLineJoin==rRHS.meLineJoin &&
        rLHS.mnMiterLimit==rRHS.mnMiterLimit &&
        rLHS.mnStrokeWidth==rRHS.mnStrokeWidth &&
        rLHS.maViewportFillColor==rRHS.maViewportFillColor &&
        rLHS.maViewportFillGradient==rRHS.maViewportFillGradient;
}

} // namespace svgi

namespace std
{
    template<> struct hash<svgi::State>
    {
        using result_type = std::size_t;
        using argument_type = svgi::State;
        std::size_t operator()(const svgi::State& rState ) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, rState.maCTM.get( 0, 0 ));
            boost::hash_combine(seed, rState.maCTM.get( 1, 0 ));
            boost::hash_combine(seed, rState.maCTM.get( 0, 1 ));
            boost::hash_combine(seed, rState.maCTM.get( 1, 1 ));
            boost::hash_combine(seed, rState.maCTM.get( 0, 2 ));
            boost::hash_combine(seed, rState.maCTM.get( 1, 2 ));
            boost::hash_combine(seed, rState.maViewport.getWidth());
            boost::hash_combine(seed, rState.maViewport.getHeight());
            boost::hash_combine(seed, rState.maViewBox.getWidth());
            boost::hash_combine(seed, rState.maViewBox.getHeight());
            boost::hash_combine(seed, rState.mbIsText);
            boost::hash_combine(seed, rState.maFontFamily);
            boost::hash_combine(seed, rState.mnFontSize);
            boost::hash_combine(seed, rState.mnParentFontSize);
            boost::hash_combine(seed, rState.maFontStyle);
            boost::hash_combine(seed, rState.maFontVariant);
            boost::hash_combine(seed, rState.mnFontWeight);
            boost::hash_combine(seed, rState.meTextAnchor);
            boost::hash_combine(seed, rState.mbVisibility);
            boost::hash_combine(seed, rState.meFillType)
            boost::hash_combine(seed, rState.mnFillOpacity);
            boost::hash_combine(seed, rState.mnOpacity);
            boost::hash_combine(seed, rState.meStrokeType);
            boost::hash_combine(seed, rState.mnStrokeOpacity);
            boost::hash_combine(seed, rState.mnViewportFillOpacity);
            boost::hash_combine(seed, rState.maFillColor.a);
            boost::hash_combine(seed, rState.maFillColor.r);
            boost::hash_combine(seed, rState.maFillColor.g);
            boost::hash_combine(seed, rState.maFillColor.b);
            boost::hash_combine(seed, rState.maFillGradient.maStops.size());
            boost::hash_combine(seed, rState.meFillRule);
            boost::hash_combine(seed, rState.maStrokeColor.a);
            boost::hash_combine(seed, rState.maStrokeColor.r);
            boost::hash_combine(seed, rState.maStrokeColor.g);
            boost::hash_combine(seed, rState.maStrokeColor.b);
            boost::hash_combine(seed, rState.maStrokeGradient.maStops.size());
            boost::hash_combine(seed, rState.maDashArray.size());
            boost::hash_combine(seed, rState.mnDashOffset);
            boost::hash_combine(seed, rState.meLineCap);
            boost::hash_combine(seed, rState.meLineJoin);
            boost::hash_combine(seed, rState.mnMiterLimit);
            boost::hash_combine(seed, rState.mnStrokeWidth);
            boost::hash_combine(seed, rState.maViewportFillColor.a);
            boost::hash_combine(seed, rState.maViewportFillColor.r);
            boost::hash_combine(seed, rState.maViewportFillColor.g);
            boost::hash_combine(seed, rState.maViewportFillColor.b);
            boost::hash_combine(seed, rState.maViewportFillGradient.maStops.size());
            return seed;
        }
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
