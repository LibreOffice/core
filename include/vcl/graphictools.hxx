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

#ifndef INCLUDED_VCL_GRAPHICTOOLS_HXX
#define INCLUDED_VCL_GRAPHICTOOLS_HXX

#include <vcl/dllapi.h>
#include <sal/types.h>
#include <rtl/string.hxx>
#include <tools/color.hxx>
#include <tools/poly.hxx>
#include <vcl/graph.hxx>

#include <memory>
#include <vector>

class SvStream;

/** Encapsulates geometry and associated attributes of a graphical 'pen stroke'

    @attention Widespread use is deprecated. See declarations above
    for the way to go. Especially the copied enums from svx/xenum.hxx
    are troublesome.

    Use this class to store geometry and attributes of a graphical
    'pen stroke', such as pen width, dashing etc. The geometry is the
    so-called 'path' along which the stroke is traced, with the given
    pen width. The cap type determines how the open ends of the path
    should be drawn. If the geometry consists of more than one
    segment, the join type determines in which way the segments are
    joined.
 */
class VCL_DLLPUBLIC SvtGraphicStroke
{
public:
    /// Style for open stroke ends
    enum CapType
    {
        /// No additional cap
        capButt=0,
        /// Half-round cap at the line end, the center lying at the end point
        capRound,
        /// Half-square cap at the line end, the center lying at the end point
        capSquare
    };
    /// Style for joins of individual stroke segments
    enum JoinType
    {
        /// Extend segment edges, until they cross
        joinMiter=0,
        /// Connect segments by a filled round arc
        joinRound,
        /// Connect segments by a direct straight line
        joinBevel,
        /// Perform no join, leads to visible gaps between thick line segments
        joinNone
    };
    typedef ::std::vector< double > DashArray;

    SvtGraphicStroke();
    /** All in one constructor

        See accessor method descriptions for argument description
     */
    SvtGraphicStroke( const tools::Polygon& rPath,
                      const tools::PolyPolygon& rStartArrow,
                      const tools::PolyPolygon& rEndArrow,
                      double                fTransparency,
                      double                fStrokeWidth,
                      CapType               aCap,
                      JoinType              aJoin,
                      double                fMiterLimit,
                      const DashArray&      rDashArray  );      // TODO: Dash array offset (position where to start, see PS)

    // accessors
    /// Query path to stroke
    void            getPath             ( tools::Polygon& ) const;
    /** Get the polygon that is put at the start of the line

        The polygon is in a special normalized position: the center of
        the stroked path will meet the given polygon at (0,0) from
        negative y values. Thus, an arrow would have its baseline on
        the x axis, going upwards to positive y values. Furthermore,
        the polygon is also scaled in a special way: the width of the
        joining stroke is defined to be
        SvtGraphicStroke::normalizedArrowWidth (0x10000), i.e. ranging
        from x=-0x8000 to x=0x8000. So, if the arrow does have this
        width, it has to fit every stroke with every stroke width
        exactly.
     */
    void            getStartArrow       ( tools::PolyPolygon& ) const;
    /** Get the polygon that is put at the end of the line

        The polygon is in a special normalized position, and already
        scaled to the desired size: the center of the stroked path
        will meet the given polygon at (0,0) from negative y
        values. Thus, an arrow would have its baseline on the x axis,
        going upwards to positive y values. Furthermore, the polygon
        is also scaled in a special way: the width of the joining
        stroke is defined to be SvtGraphicStroke::normalizedArrowWidth
        (0x10000), i.e. ranging from x=-0x8000 to x=0x8000. So, if the
        arrow does have this width, it has to fit every stroke with
        every stroke width exactly.
     */
    void            getEndArrow         ( tools::PolyPolygon& ) const;
    /** Get stroke transparency

        @return the transparency, ranging from 0.0 (opaque) to 1.0 (fully translucent)
     */
    double          getTransparency     () const { return mfTransparency;}
    /// Get width of the stroke
    double          getStrokeWidth      () const { return mfStrokeWidth;}
    /// Get the style in which open stroke ends are drawn
    CapType         getCapType          () const { return maCapType;}
    /// Get the style in which the stroke segments are joined
    JoinType        getJoinType         () const { return maJoinType;}
    /// Get the maximum length of mitered joins
    double          getMiterLimit       () const { return mfMiterLimit;}
    /// Get an array of "on" and "off" lengths for stroke dashing
    void            getDashArray        ( DashArray& ) const;

    // mutators
    /// Set path to stroke
    void    setPath             ( const tools::Polygon& );
    /** Set the polygon that is put at the start of the line

        The polygon has to be in a special normalized position, and
        already scaled to the desired size: the center of the stroked
        path will meet the given polygon at (0,0) from negative y
        values. Thus, an arrow would have its baseline on the x axis,
        going upwards to positive y values. Furthermore, the polygon
        also has to be scaled appropriately: the width of the joining
        stroke is defined to be SvtGraphicStroke::normalizedArrowWidth
        (0x10000), i.e. ranging from x=-0x8000 to x=0x8000. If your
        arrow does have this width, it will fit every stroke with
        every stroke width exactly.
     */
    void    setStartArrow       ( const tools::PolyPolygon& );
    /** Set the polygon that is put at the end of the line

        The polygon has to be in a special normalized position, and
        already scaled to the desired size: the center of the stroked
        path will meet the given polygon at (0,0) from negative y
        values. Thus, an arrow would have its baseline on the x axis,
        going upwards to positive y values. Furthermore, the polygon
        also has to be scaled appropriately: the width of the joining
        stroke is defined to be SvtGraphicStroke::normalizedArrowWidth
        (0x10000), i.e. ranging from x=-0x8000 to x=0x8000. If your
        arrow does have this width, it will fit every stroke with
        every stroke width exactly.
     */
    void    setEndArrow         ( const tools::PolyPolygon& );
    /// Affine scaling in both X and Y dimensions
    void    scale               ( double fScaleX, double fScaleY );

private:
    // friends
    VCL_DLLPUBLIC friend SvStream& WriteSvtGraphicStroke( SvStream& rOStm, const SvtGraphicStroke& rClass );
    VCL_DLLPUBLIC friend SvStream& ReadSvtGraphicStroke( SvStream& rIStm, SvtGraphicStroke& rClass );

    tools::Polygon  maPath;
    tools::PolyPolygon maStartArrow;
    tools::PolyPolygon maEndArrow;
    double          mfTransparency;
    double          mfStrokeWidth;
    CapType         maCapType;
    JoinType        maJoinType;
    double          mfMiterLimit;
    DashArray       maDashArray;
};

/** Encapsulates geometry and associated attributes of a filled area

    @attention Widespread use is deprecated. See declarations above
    for the way to go. Especially the copied enums from svx/xenum.hxx
    is troublesome.

    Use this class to store geometry and attributes of a filled area,
    such as fill color, transparency, texture or hatch.  The geometry
    is the so-called 'path', whose inner area will get filled
    according to the attributes set. If the path is intersecting, or
    one part of the path is lying fully within another part, then the
    fill rule determines which parts are filled and which are not.
 */
class VCL_DLLPUBLIC SvtGraphicFill
{
public:
    /// Type of fill algorithm used
    enum FillRule
    {
        /** Non-zero winding rule

            Fill shape scanline-wise. Starting at the left, determine
            the winding number as follows: every segment crossed that
            runs counter-clockwise adds one to the winding number,
            every segment crossed that runs clockwise subtracts
            one. The part of the scanline where the winding number is
            non-zero gets filled.
         */
        fillNonZero=0,
        /** Even-odd fill rule

            Fill shape scanline-wise. Starting at the left, count the
            number of segments crossed. If this number is odd, the
            part of the scanline is filled, otherwise not.
         */
        fillEvenOdd
    };
    /// Type of filling used
    enum FillType
    {
        /// Fill with a specified solid color
        fillSolid=0,
        /// Fill with the specified gradient
        fillGradient,
        /// Fill with the specified hatch
        fillHatch,
        /// Fill with the specified texture (a Graphic object)
        fillTexture
    };
    /// Type of hatching used
    enum HatchType
    {
        /// horizontal parallel lines, one unit apart
        hatchSingle=0,
        /// horizontal and verticall orthogonally crossing lines, one unit apart
        hatchDouble,
        /// three crossing lines, like HatchType::hatchDouble, but
        /// with an additional diagonal line, rising to the upper
        /// right corner. The first diagonal line goes through the
        /// upper left corner, the other are each spaced a unit apart.
        hatchTriple
    };
    /// Type of gradient used
    enum GradientType {gradientLinear=0, gradientRadial, gradientRectangular};
    /// Special values for gradient step count
    enum { gradientStepsInfinite=0 };
    /** Homogeneous 2D transformation matrix

        This is a 2x3 matrix representing an affine transformation on
        the R^2, in the usual C/C++ row major form. It is structured as follows:
        <pre>
        a b t_x
        c d t_y
        0 0 1
        </pre>
        where the lowest line is not stored in the matrix, since it is
        constant. Variables t_x and t_y contain translational
        components, a to d rotation, scale and shear (for details,
        look up your favorite linear algebra/computer graphics book).
     */
    struct VCL_DLLPUBLIC Transform
    {
        enum { MatrixSize=6 };
        Transform();
        double matrix[MatrixSize];
    };

    SvtGraphicFill();
    /** All in one constructor

        See accessor method descriptions for argument description
     */
    SvtGraphicFill( const tools::PolyPolygon&  rPath,
                    Color               aFillColor,
                    double              fTransparency,
                    FillRule            aFillRule,
                    FillType            aFillType,              // TODO: Multitexturing
                    const Transform&    aFillTransform,
                    bool                bTiling,
                    HatchType           aHatchType,             // TODO: vector of directions and start points
                    Color               aHatchColor,
                    GradientType        aGradientType,          // TODO: Transparent gradients (orthogonal to normal ones)
                    Color               aGradient1stColor,      // TODO: vector of colors and offsets
                    Color               aGradient2ndColor,
                    sal_Int32           aGradientStepCount,     // numbers of steps to render the gradient. gradientStepsInfinite means infinitely many.
                    const Graphic&      aFillGraphic );

    // accessors
    /// Query path to fill
    void            getPath             ( tools::PolyPolygon& ) const;
    /// Get color used for solid fills
    const Color&    getFillColor        () const { return maFillColor;}
    /** Get stroke transparency

        @return the transparency, ranging from 0.0 (opaque) to 1.0 (fully translucent)
     */
    double          getTransparency     () const { return mfTransparency;}
    /// Get fill rule used
    FillRule        getFillRule         () const { return maFillRule;}
    /** Get fill type used

        Currently, only one of the fill types can be used
        simultaneously. If you specify e.g. FillRule::fillGradient,
        hatching, texture and solid fill color are ignored.
     */
    FillType        getFillType         () const { return maFillType;}
    /** Get transformation applied to hatch, gradient or texture during fill

        A fill operation generally starts at the top left position of
        the object's bounding box. At that position (if tiling is on,
        also all successive positions), the specified fill graphic is
        rendered, after applying the fill transformation to it. For
        example, if the fill transformation contains a translation,
        the fill graphic is rendered at the object's bounding box's
        top left corner plus the translation components.

     */
    void            getTransform        ( Transform& ) const;
    /// deprecated
    bool            IsTiling            () const { return mbTiling;}
    /** Query state of texture tiling

        @return true, if texture is tiled, false, if output only once.
     */
    bool            isTiling            () const { return mbTiling;}
    /// Get type of gradient used
    GradientType    getGradientType     () const { return maGradientType;}

    /** Get the texture graphic used

        The Graphic object returned is used to fill the geometry, if
        the FillType is fillTexture. The Graphic object is always
        assumed to be of size 1x1, the transformation is used to scale
        it to the appropriate size.
     */
    void            getGraphic          ( Graphic& ) const;

    // mutators
    /// Set path to fill
    void    setPath             ( const tools::PolyPolygon& rPath );

private:
    // friends
    VCL_DLLPUBLIC friend SvStream& WriteSvtGraphicFill( SvStream& rOStm, const SvtGraphicFill& rClass );
    VCL_DLLPUBLIC friend SvStream& ReadSvtGraphicFill( SvStream& rIStm, SvtGraphicFill& rClass );

    tools::PolyPolygon     maPath;
    Color           maFillColor;
    double          mfTransparency;
    FillRule        maFillRule;
    FillType        maFillType;
    Transform       maFillTransform;
    bool            mbTiling;
    HatchType       maHatchType;
    Color           maHatchColor;
    GradientType    maGradientType;
    Color           maGradient1stColor;
    Color           maGradient2ndColor;
    sal_Int32       maGradientStepCount;
    Graphic         maFillGraphic;
};

#endif // INCLUDED_VCL_GRAPHICTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
