/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_GFXTYPES_HXX
#define INCLUDED_GFXTYPES_HXX

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <rtl/ustring.hxx>

namespace svgi
{

struct ARGBColor
{
    double toDoubleColor( sal_uInt8 val ) const { return val/255.0; }

    ARGBColor() : a(1.0), r(0.0), g(0.0), b(0.0)
    {}
    explicit ARGBColor(double fGrey) : a(1.0), r(fGrey), g(fGrey), b(fGrey)
    {}
    ARGBColor( double r_, double g_, double b_ ) :
        a(1.0), r(r_), g(g_), b(b_)
    {}
    ARGBColor( double a_, double r_, double g_, double b_ ) :
        a(a_), r(r_), g(g_), b(b_)
    {}
    ARGBColor( int r_, int g_, int b_ ) :
        a(1.0),
        r(toDoubleColor(sal::static_int_cast<sal_uInt8>(r_))),
        g(toDoubleColor(sal::static_int_cast<sal_uInt8>(g_))),
        b(toDoubleColor(sal::static_int_cast<sal_uInt8>(b_)))
    {}
    ARGBColor( int a_, int r_, int g_, int b_ ) :
        a(toDoubleColor(sal::static_int_cast<sal_uInt8>(a_))),
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
inline bool operator==( const GradientStop& rLHS, const GradientStop& rRHS )
{ return rLHS.mnStopPosition==rRHS.mnStopPosition && rLHS.maStopColor==rRHS.maStopColor; }

struct Gradient
{
    enum GradientType { LINEAR, RADIAL};
    std::vector<sal_Size> maStops;
    basegfx::B2DHomMatrix maTransform;
    GradientType          meType;
    union
    {
        double test;
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
    bool                  mbLinearBoundingBoxUnits;

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
        maFontStyle("normal"),
        maFontVariant("normal"),
        mnFontWeight(400.0),
        meTextAnchor(BEFORE),
        meTextDisplayAlign(BEFORE),
        mnTextLineIncrement(0.0),
        maCurrentColor(1.0),
        mbVisibility(true),
        meFillType(SOLID),
        mnFillOpacity(1.0),
        mnOpacity(1.0),
        meStrokeType(NONE),
        mnStrokeOpacity(1.0),
        meViewportFillType(NONE),
        mnViewportFillOpacity(1.0),
        maFillColor(0.0),
        maFillGradient(Gradient::LINEAR),
        meFillRule(NON_ZERO),
        maStrokeColor(0.0),
        maStrokeGradient(Gradient::LINEAR),
        maDashArray(),
        mnDashOffset(0.0),
        meLineCap(BUTT),
        meLineJoin(basegfx::B2DLINEJOIN_MITER),
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
    OUString               maFontFamily;
    /** Absolute: xx-small=6.94 | x-small=8.33 | small=10 | medium=12 | large=14.4 | x-large=17.28 | xx-large=20.736

        Relative(to parent): larger (enlarge by 1.2)
                             smaller (shrink by 1.2)

     */
    double                      mnFontSize;
    OUString               maFontStyle;
    OUString               maFontVariant;
    double                      mnFontWeight;

    TextAlign                   meTextAnchor; // text-anchor
    TextAlign                   meTextDisplayAlign; // display-align
    double                      mnTextLineIncrement; // 0.0 means auto

    ARGBColor                   maCurrentColor;
    bool                        mbVisibility;

    PaintType                   meFillType;
    double                      mnFillOpacity;
    double                      mnOpacity;
    PaintType                   meStrokeType;
    double                      mnStrokeOpacity;
    PaintType                   meViewportFillType;
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
        rLHS.maFontStyle==rRHS.maFontStyle &&
        rLHS.maFontVariant==rRHS.maFontVariant &&
        rLHS.mnFontWeight==rRHS.mnFontWeight &&
        rLHS.meTextAnchor==rRHS.meTextAnchor &&
        rLHS.meTextDisplayAlign==rRHS.meTextDisplayAlign &&
        rLHS.mnTextLineIncrement==rRHS.mnTextLineIncrement &&
        rLHS.maCurrentColor==rRHS.maCurrentColor &&
        rLHS.mbVisibility==rRHS.mbVisibility &&
        rLHS.meFillType==rRHS.meFillType &&
        rLHS.mnFillOpacity==rRHS.mnFillOpacity &&
        rLHS.mnOpacity==rRHS.mnOpacity &&
        rLHS.meStrokeType==rRHS.meStrokeType &&
        rLHS.mnStrokeOpacity==rRHS.mnStrokeOpacity &&
        rLHS.meViewportFillType==rRHS.meViewportFillType &&
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

struct StateHash
{
    size_t operator()(const State& rState ) const
    {
        return size_t(rState.maCTM.get( 0, 0 ))
            ^  size_t(rState.maCTM.get( 1, 0 ))
            ^  size_t(rState.maCTM.get( 0, 1 ))
            ^  size_t(rState.maCTM.get( 1, 1 ))
            ^  size_t(rState.maCTM.get( 0, 2 ))
            ^  size_t(rState.maCTM.get( 1, 2 ))
            ^  size_t(rState.maViewport.getWidth())
            ^  size_t(rState.maViewport.getHeight())
            ^  size_t(rState.maViewBox.getWidth())
            ^  size_t(rState.maViewBox.getHeight())
            ^  size_t(rState.mbIsText)
            ^  size_t(rState.maFontFamily.hashCode())
            ^  size_t(rState.mnFontSize)
            ^  size_t(rState.maFontStyle.hashCode())
            ^  size_t(rState.maFontVariant.hashCode())
            ^  size_t(rState.mnFontWeight)
            ^  size_t(rState.meTextAnchor)
            ^  size_t(rState.meTextDisplayAlign)
            ^  size_t(rState.mnTextLineIncrement)
            ^  size_t(rState.mbVisibility)
            ^  size_t(rState.meFillType)
            ^  size_t(rState.mnFillOpacity)
            ^  size_t(rState.mnOpacity)
            ^  size_t(rState.meStrokeType)
            ^  size_t(rState.mnStrokeOpacity)
            ^  size_t(rState.meViewportFillType)
            ^  size_t(rState.mnViewportFillOpacity)
            ^  size_t(rState.maFillColor.a)
            ^  size_t(rState.maFillColor.r)
            ^  size_t(rState.maFillColor.g)
            ^  size_t(rState.maFillColor.b)
            ^  size_t(rState.maFillGradient.maStops.size())
            ^  size_t(rState.meFillRule)
            ^  size_t(rState.maStrokeColor.a)
            ^  size_t(rState.maStrokeColor.r)
            ^  size_t(rState.maStrokeColor.g)
            ^  size_t(rState.maStrokeColor.b)
            ^  size_t(rState.maStrokeGradient.maStops.size())
            ^  size_t(rState.maDashArray.size())
            ^  size_t(rState.mnDashOffset)
            ^  size_t(rState.meLineCap)
            ^  size_t(rState.meLineJoin)
            ^  size_t(rState.mnMiterLimit)
            ^  size_t(rState.mnStrokeWidth)
            ^  size_t(rState.maViewportFillColor.a)
            ^  size_t(rState.maViewportFillColor.r)
            ^  size_t(rState.maViewportFillColor.g)
            ^  size_t(rState.maViewportFillColor.b)
            ^  size_t(rState.maViewportFillGradient.maStops.size());
    }
};

typedef boost::unordered_set<State, StateHash> StatePool;
typedef boost::unordered_map<sal_Int32, State> StateMap;

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
