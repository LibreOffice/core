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

#include <sal/config.h>

#include <cstdlib>

#include <svx/framelink.hxx>

#include <math.h>
#include <vcl/outdev.hxx>
#include <tools/gen.hxx>
#include <editeng/borderline.hxx>
#include <svtools/borderhelper.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>


using namespace ::com::sun::star;
using namespace editeng;

namespace svx {
namespace frame {


namespace {

typedef std::vector< Point > PointVec;


// Link result structs for horizontal and vertical lines and borders.

/** Result struct used by the horizontal/vertical frame link functions.

    This struct is used to return coordinate offsets for one end of a single
    line in a frame border, i.e. the left end of the primary line of a
    horizontal frame border.

    1) Usage for horizontal lines

    If this struct is returned by the lclLinkHorFrameBorder() function, each
    member refers to the X coordinate of one edge of a single line end in a
    horizontal frame border. They specify an offset to modify this coordinate
    when the line is painted. The values in this struct may change a
    rectangular line shape into a line with slanted left or right border, which
    is used to connect the line with diagonal lines.

    Usage for a left line end:          Usage for a right line end:
                mnOffs1                         mnOffs1
                <------->                       <------->
                    +-------------------------------+
                    | the original horizontal line  |
                    +-------------------------------+
                <------->                       <------->
                mnOffs2                         mnOffs2

    2) Usage for vertical lines

    If this struct is returned by the lclLinkVerFrameBorder() function, each
    member refers to the Y coordinate of one edge of a single line end in a
    vertical frame border. They specify an offset to modify this coordinate
    when the line is painted. The values in this struct may change a
    rectangular line shape into a line with slanted top or bottom border, which
    is used to connect the line with diagonal lines.

    Usage for a top line end:       mnOffs1 ^               ^ mnOffs2
                                            |   +-------+   |
                                            v   |       |   v
                                                |       |
                                                |       |
                the original vertical line ---> |       |
                                                |       |
                                                |       |
                                            ^   |       |   ^
                                            |   +-------+   |
    Usage for a bottom line end:    mnOffs1 v               v mnOffs2
 */
struct LineEndResult
{
    long                mnOffs1;    /// Offset for top or left edge, dependent of context.
    long                mnOffs2;    /// Offset for bottom or right edge, dependent of context

    explicit     LineEndResult() : mnOffs1( 0 ), mnOffs2( 0 ) {}

    void         Swap() { std::swap( mnOffs1, mnOffs2 ); }
    void         Negate() { mnOffs1 = -mnOffs1; mnOffs2 = -mnOffs2; }
};

/** Result struct used by the horizontal/vertical frame link functions.

    This struct contains the linking results for one end of a frame border,
    including both the primary and secondary line ends.
 */
struct BorderEndResult
{
    LineEndResult       maPrim;     /// Result for primary line.
    LineEndResult       maSecn;     /// Result for secondary line.
    LineEndResult       maGap;      /// Result for gap line.

    void         Negate() { maPrim.Negate(); maSecn.Negate(); maGap.Negate(); }
};

/** Result struct used by the horizontal/vertical frame link functions.

    This struct contains the linking results for both frame border ends, and
    therefore for the complete frame border.
 */
struct BorderResult
{
    BorderEndResult     maBeg;      /// Result for begin of border line (left or top end).
    BorderEndResult     maEnd;      /// Result for end of border line (right or bottom end).
};


// Link result structs for diagonal lines and borders.

/** Result struct used by the diagonal frame link functions.

    This struct contains the linking results for one line of a diagonal frame
    border.
 */
struct DiagLineResult
{
    long                mnLClip;    /// Offset for left border of clipping rectangle.
    long                mnRClip;    /// Offset for right border of clipping rectangle.
    long                mnTClip;    /// Offset for top border of clipping rectangle.
    long                mnBClip;    /// Offset for bottom border of clipping rectangle.

    explicit     DiagLineResult() : mnLClip( 0 ), mnRClip( 0 ), mnTClip( 0 ), mnBClip( 0 ) {}
};

/** Result struct used by the diagonal frame link functions.

    This struct contains the linking results for one diagonal frame border.
 */
struct DiagBorderResult
{
    DiagLineResult      maPrim;     /// Result for primary line.
    DiagLineResult      maSecn;     /// Result for secondary line.
};

/** Result struct used by the diagonal frame link functions.

    This struct contains the linking results for both diagonal frame borders.
 */
struct DiagBordersResult
{
    DiagBorderResult    maTLBR;     /// Result for top-left to bottom-right frame border.
    DiagBorderResult    maBLTR;     /// Result for bottom-left to top-right frame border.
};


/** A helper struct containing two points of a line.
 */
struct LinePoints
{
    Point               maBeg;      /// Start position of the line.
    Point               maEnd;      /// End position of the line.

    explicit            LinePoints( const Point& rBeg, const Point& rEnd ) :
                            maBeg( rBeg ), maEnd( rEnd ) {}
    explicit            LinePoints( const tools::Rectangle& rRect, bool bTLBR ) :
                            maBeg( bTLBR ? rRect.TopLeft() : rRect.TopRight() ),
                            maEnd( bTLBR ? rRect.BottomRight() : rRect.BottomLeft() ) {}
};


/** Rounds and casts a double value to a long value. */
inline long lclD2L( double fValue )
{
    return static_cast< long >( (fValue < 0.0) ? (fValue - 0.5) : (fValue + 0.5) );
}

/** Converts a width in twips to a width in another map unit (specified by fScale). */
double lclScaleValue( double nValue, double fScale, sal_uInt16 nMaxWidth )
{
    return std::min<double>(nValue * fScale, nMaxWidth);
}


// Line width offset calculation.

/** Returns the start offset of the single/primary line across the frame border.

    All following lclGet*Beg() and lclGet*End() functions return sub units to
    increase the computational accuracy, where 256 sub units are equal to
    1 map unit of the used OutputDevice.

    The following pictures show the upper end of a vertical frame border and
    illustrates the return values of all following lclGet*Beg() and lclGet*End()
    functions. The first picture shows a single frame border, the second picture
    shows a double frame border.

    The functions regard the reference point handling mode of the passed border
    style.
    RefMode::Centered:
        All returned offsets are relative to the middle position of the frame
        border (offsets left of the middle are returned negative, offsets right
        of the middle are returned positive).
    RefMode::Begin:
        All returned offsets are relative to the begin of the frame border
        (lclGetBeg() always returns 0).
    RefMode::End:
        All returned offsets are relative to the end of the frame border
        (lclGetEnd() always returns 0).

                                                        |<- lclGetEnd()
                       |<- lclGetBeforeBeg()            |<- lclGetPrimEnd()
                       |                                |
                       ||<- lclGetBeg()                 ||<- lclGetBehindEnd()
                       ||                               ||
                       |#################################|
       direction of |   #################################
          the frame |   #################################
          border is |   #################################
           vertical |   #################################
                    v   #################################
                                        |
                                        |<- middle of the frame border

                                         lclGetDistEnd() ->||<- lclGetSecnBeg()
                                                           ||
          lclGetBeg() ->|   lclGetDistBeg() ->|            ||           |<- lclGetEnd()
                        |                     |            ||           |
    lclGetBeforeBeg()->||  lclGetPrimEnd() ->||            ||           ||<- lclGetBehindEnd()
                       ||                    ||            ||           ||
                       |######################|            |#############|
       direction of |   ######################              #############
          the frame |   ######################              #############
          border is |   ######################              #############
           vertical |   ######################  |           #############
                    v   ######################  |           #############
                        primary line            |           secondary line
                                                |
                                                |<- middle of the frame border

    @return
        The start offset of the single/primary line relative to the reference
        position of the frame border (sub units; 0 for invisible or one pixel
        wide single frame styles).
 */
long lclGetBeg( const Style& rBorder )
{
    long nPos = 0;
    switch( rBorder.GetRefMode() )
    {
        case RefMode::Centered:  if( rBorder.Prim() ) nPos = -128 * (rBorder.GetWidth() - 1); break;
        case RefMode::End:       if( rBorder.Prim() ) nPos = -256 * (rBorder.GetWidth() - 1); break;
        case RefMode::Begin:     break;
    }
    return nPos;
}

/** Returns the end offset of the single/secondary line across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The end offset of the single/secondary line relative to the
    reference position of the frame border (sub units; 0 for invisible or one
    pixel wide single frame styles). */
long lclGetEnd( const Style& rBorder )
{
    long nPos = 0;
    switch( rBorder.GetRefMode() )
    {
        case RefMode::Centered:  if( rBorder.Prim() ) nPos = 128 * (rBorder.GetWidth() - 1); break;
        case RefMode::Begin:     if( rBorder.Prim() ) nPos = 256 * (rBorder.GetWidth() - 1); break;
        case RefMode::End:     break;
    }
    return nPos;
}

/** Returns the end offset of the primary line across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The end offset of the primary line relative to the reference
    position of the frame border (sub units; the end of the primary line in a
    double frame style, otherwise the same as lclGetEnd()). */
inline long lclGetPrimEnd( const Style& rBorder )
{ return rBorder.Prim() ? (lclGetBeg( rBorder ) + 256 * (rBorder.Prim() - 1)) : 0; }

/** Returns the start offset of the secondary line across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The start offset of the secondary line relative to the reference
    position of the frame border (sub units; 0 for single/invisible border
    styles). */
inline long lclGetSecnBeg( const Style& rBorder )
{ return rBorder.Secn() ? (lclGetEnd( rBorder ) - 256 * (rBorder.Secn() - 1)) : 0; }

/** Returns the start offset of the distance space across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The start offset of the distance space relative to the reference
    position of the frame border (sub units; 0 for single/invisible border
    styles). */
inline long lclGetDistBeg( const Style& rBorder )
{ return rBorder.Secn() ? (lclGetBeg( rBorder ) + 256 * rBorder.Prim()) : 0; }

/** Returns the end offset of the distance space across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The end offset of the distance space relative to the reference
    position of the frame border (sub units; 0 for single/invisible border
    styles). */
inline long lclGetDistEnd( const Style& rBorder )
{ return rBorder.Secn() ? (lclGetEnd( rBorder ) - 256 * rBorder.Secn()) : 0; }

/** Returns the offset before start of single/primary line across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The offset directly before start of single/primary line relative
    to the reference position of the frame border (sub units; a value one less
    than lclGetBeg() for visible frame styles, or 0 for invisible frame style). */
inline long lclGetBeforeBeg( const Style& rBorder )
{ return rBorder.Prim() ? (lclGetBeg( rBorder ) - 256) : 0; }

/** Returns the offset behind end of single/secondary line across the frame border.
    @descr  See description of lclGetBeg() for an illustration.
    @return  The offset directly behind end of single/secondary line relative
    to the reference position of the frame border (sub units; a value one
    greater than lclGetEnd() for visible frame styles, or 0 for invisible frame
    style). */
inline long lclGetBehindEnd( const Style& rBorder )
{ return rBorder.Prim() ? (lclGetEnd( rBorder ) + 256) : 0; }


// Linking functions


// Linking of single horizontal line ends.

/** Calculates X offsets for the left end of a single horizontal frame border.

    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of the left line end.
 */
void lclLinkLeftEnd_Single(
        LineEndResult& rResult, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& rLFromBR )
{
    // both vertical and diagonal frame borders are double
    if( rLFromT.Secn() && rLFromB.Secn() && rLFromTR.Secn() && rLFromBR.Secn() )
    {
        // take left position of upper and lower secondary start
        rResult.mnOffs1 = GetBLDiagOffset( lclGetBeg( rBorder ), lclGetSecnBeg( rLFromTR ), rLFromTR.GetAngle() );
        rResult.mnOffs2 = GetTLDiagOffset( lclGetEnd( rBorder ), lclGetSecnBeg( rLFromBR ), rLFromBR.GetAngle() );
    }
    else
    {
        // both vertical frame borders are double
        if( rLFromT.Secn() && rLFromB.Secn() )
        {
            rResult.mnOffs1 = (!rLFromTR.Secn() && !rLFromBR.Secn() && rtl::math::approxEqual(rLFromT.GetWidth(), rLFromB.GetWidth())) ?
                // don't overdraw vertical borders with equal width
                lclGetBehindEnd( rLFromT ) :
                // take leftmost start of both secondary lines (#46488#)
                std::min( lclGetSecnBeg( rLFromT ), lclGetSecnBeg( rLFromB ) );
        }

        // single border with equal width coming from left
        else if( !rLFromL.Secn() && rtl::math::approxEqual(rLFromL.Prim(), rBorder.Prim()) )
            // draw to connection point
            rResult.mnOffs1 = 0;

        // single border coming from left
        else if( !rLFromL.Secn() && rLFromL.Prim() )
        {
            if( rtl::math::approxEqual(rLFromL.Prim(), rBorder.Prim()) )
                // draw to reference position, if from left has equal width
                rResult.mnOffs1 = 0;
            else
                rResult.mnOffs1 = (rLFromL < rBorder) ?
                    // take leftmost start of both frame borders, if from left is thinner
                    std::min( lclGetBeg( rLFromT ), lclGetBeg( rLFromB ) ) :
                    // do not overdraw vertical, if from left is thicker
                    std::max( lclGetBehindEnd( rLFromT ), lclGetBehindEnd( rLFromB ) );
        }

        // no border coming from left
        else if( !rLFromL.Prim() )
            // don't overdraw vertical borders with equal width
            rResult.mnOffs1 = rtl::math::approxEqual(rLFromT.GetWidth(), rLFromB.GetWidth()) ?
                lclGetBehindEnd( rLFromT ) :
                std::min( lclGetBeg( rLFromT ), lclGetBeg( rLFromB ) );

        // double frame border coming from left and from top
        else if( rLFromT.Secn() )
            // do not overdraw the vertical double frame border
            rResult.mnOffs1 = lclGetBehindEnd( rLFromT );

        // double frame border coming from left and from bottom
        else if( rLFromB.Secn() )
            // do not overdraw the vertical double frame border
            rResult.mnOffs1 = lclGetBehindEnd( rLFromB );

        // double frame border coming from left, both vertical frame borders are single or off
        else
            // draw from leftmost start of both frame borders, if from left is not thicker
            rResult.mnOffs1 = (rLFromL <= rBorder) ?
                std::min( lclGetBeg( rLFromT ), lclGetBeg( rLFromB ) ) :
                std::max( lclGetBehindEnd( rLFromT ), lclGetBehindEnd( rLFromB ) );

        // bottom-left point is equal to top-left point (results in rectangle)
        rResult.mnOffs2 = rResult.mnOffs1;
    }
}

/** Calculates X offsets for the left end of a primary horizontal line.

    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of the left end of the primary line.
 */
void lclLinkLeftEnd_Prim(
        LineEndResult& rResult, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& /*rLFromBR*/ )
{
    // double diagonal frame border coming from top right
    if( rLFromTR.Secn() )
    {
        // draw from where secondary diagonal line meets the own primary
        rResult.mnOffs1 = GetBLDiagOffset( lclGetBeg( rBorder ), lclGetSecnBeg( rLFromTR ), rLFromTR.GetAngle() );
        rResult.mnOffs2 = GetBLDiagOffset( lclGetPrimEnd( rBorder ), lclGetSecnBeg( rLFromTR ), rLFromTR.GetAngle() );
    }

    // no or single diagonal frame border - ignore it
    else
    {
        // double frame border coming from top
        if( rLFromT.Secn() )
            // draw from left edge of secondary vertical
            rResult.mnOffs1 = lclGetSecnBeg( rLFromT );

        // double frame border coming from left (from top is not double)
        else if( rLFromL.Secn() )
            // do not overdraw single frame border coming from top
            rResult.mnOffs1 = rtl::math::approxEqual(rLFromL.GetWidth(), rBorder.GetWidth()) ?
                0 : lclGetBehindEnd( rLFromT );

        // double frame border coming from bottom (from top and from left are not double)
        else if( rLFromB.Secn() )
            // draw from left edge of primary vertical from bottom
            rResult.mnOffs1 = lclGetBeg( rLFromB );

        // no other frame border is double
        else
            // do not overdraw vertical frame borders
            rResult.mnOffs1 = std::max( lclGetBehindEnd( rLFromT ), lclGetBehindEnd( rLFromB ) );

        // bottom-left point is equal to top-left point (results in rectangle)
        rResult.mnOffs2 = rResult.mnOffs1;
    }
}

/** Calculates X offsets for the left end of a secondary horizontal line.

    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of the left end of the secondary line.
 */
void lclLinkLeftEnd_Secn(
        LineEndResult& rResult, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& rLFromBR )
{
    /*  Recycle lclLinkLeftEnd_Prim() function with mirrored horizontal borders. */
    lclLinkLeftEnd_Prim( rResult, rBorder.Mirror(), rLFromBR, rLFromB, rLFromL.Mirror(), rLFromT, rLFromTR );
    rResult.Swap();
}

void lclLinkLeftEnd_Gap(
        LineEndResult& rResult, const Style& rBorder,
        const DiagStyle& /*rLFromTR*/, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& /*rLFromBR*/ )

{
    if ( rLFromT.Secn() )
        rResult.mnOffs1 = lclGetDistBeg( rLFromT );
    else if ( rLFromL.Secn( ) )
        rResult.mnOffs1 = rtl::math::approxEqual( rLFromL.GetWidth(), rBorder.GetWidth() ) ?
            0 : lclGetBehindEnd( rLFromT );
    else if ( rLFromB.Secn( ) )
        rResult.mnOffs1 = lclGetDistBeg( rLFromB );
    else
        rResult.mnOffs1 = std::max( lclGetBehindEnd( rLFromT ), lclGetBehindEnd( rLFromB ) );

    rResult.mnOffs2 = rResult.mnOffs1;
}

// Linking of horizontal frame border ends.

/** Calculates X offsets for the left end of a horizontal frame border.

    This function can be used for single and double frame borders.
    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of the left end of the line(s) in the frame border.
 */
void lclLinkLeftEnd(
        BorderEndResult& rResult, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& rLFromBR )
{
    if( rBorder.Secn() )
    {
        // current frame border is double
        lclLinkLeftEnd_Prim( rResult.maPrim, rBorder, rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR );
        lclLinkLeftEnd_Secn( rResult.maSecn, rBorder, rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR );
        lclLinkLeftEnd_Gap( rResult.maGap, rBorder, rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR );
    }
    else if( rBorder.Prim() )
    {
        // current frame border is single
        lclLinkLeftEnd_Single( rResult.maPrim, rBorder, rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR );
    }
    else
    {
        SAL_WARN( "svx.dialog", "lclLinkLeftEnd - called for invisible frame style" );
    }
}

/** Calculates X offsets for the right end of a horizontal frame border.

    This function can be used for single and double frame borders.
    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of the right end of the line(s) in the frame border.
 */
void lclLinkRightEnd(
        BorderEndResult& rResult, const Style& rBorder,
        const DiagStyle& rRFromTL, const Style& rRFromT, const Style& rRFromR, const Style& rRFromB, const DiagStyle& rRFromBL )
{
    /*  Recycle lclLinkLeftEnd() function with mirrored vertical borders. */
    lclLinkLeftEnd( rResult, rBorder, rRFromTL.Mirror(), rRFromT.Mirror(), rRFromR, rRFromB.Mirror(), rRFromBL.Mirror() );
    rResult.Negate();
}


// Linking of horizontal and vertical frame borders.

/** Calculates X offsets for all line ends of a horizontal frame border.

    This function can be used for single and double frame borders.
    See DrawHorFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        X coordinates of both ends of the line(s) in the frame border. To get
        the actual X coordinates to draw the lines, these offsets have to be
        added to the X coordinates of the reference points of the frame border
        (the offsets may be negative).
 */
void lclLinkHorFrameBorder(
        BorderResult& rResult, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& rLFromBR,
        const DiagStyle& rRFromTL, const Style& rRFromT, const Style& rRFromR, const Style& rRFromB, const DiagStyle& rRFromBL )
{
    lclLinkLeftEnd( rResult.maBeg, rBorder, rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR );
    lclLinkRightEnd( rResult.maEnd, rBorder, rRFromTL, rRFromT, rRFromR, rRFromB, rRFromBL );
}

/** Calculates Y offsets for all line ends of a vertical frame border.

    This function can be used for single and double frame borders.
    See DrawVerFrameBorder() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for the
        Y coordinates of both ends of the line(s) in the frame border. To get
        the actual Y coordinates to draw the lines, these offsets have to be
        added to the Y coordinates of the reference points of the frame border
        (the offsets may be negative).
 */
void lclLinkVerFrameBorder(
        BorderResult& rResult, const Style& rBorder,
        const DiagStyle& rTFromBL, const Style& rTFromL, const Style& rTFromT, const Style& rTFromR, const DiagStyle& rTFromBR,
        const DiagStyle& rBFromTL, const Style& rBFromL, const Style& rBFromB, const Style& rBFromR, const DiagStyle& rBFromTR )
{
    /*  Recycle lclLinkHorFrameBorder() function with correct parameters. The
        frame border is virtually mirrored at the top-left to bottom-right
        diagonal. rTFromBR and rBFromTL are mirrored to process their primary
        and secondary lines correctly. */
    lclLinkHorFrameBorder( rResult, rBorder,
        rTFromBL, rTFromL, rTFromT, rTFromR, rTFromBR.Mirror(),
        rBFromTL.Mirror(), rBFromL, rBFromB, rBFromR, rBFromTR );
}


// Linking of diagonal frame borders.

/** Calculates clipping offsets for a top-left to bottom-right frame border.

    This function can be used for single and double frame borders.
    See DrawDiagFrameBorders() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for all
        borders of the reference rectangle containing the diagonal frame border.
 */
void lclLinkTLBRFrameBorder(
        DiagBorderResult& rResult, const Style& rBorder,
        const Style& rTLFromB, const Style& rTLFromR, const Style& rBRFromT, const Style& rBRFromL )
{
    bool bIsDbl = rBorder.Secn() != 0;

    rResult.maPrim.mnLClip = lclGetBehindEnd( rTLFromB );
    rResult.maPrim.mnRClip = (bIsDbl && rBRFromT.Secn()) ? lclGetEnd( rBRFromT ) : lclGetBeforeBeg( rBRFromT );
    rResult.maPrim.mnTClip = (bIsDbl && rTLFromR.Secn()) ? lclGetBeg( rTLFromR ) : lclGetBehindEnd( rTLFromR );
    rResult.maPrim.mnBClip = lclGetBeforeBeg( rBRFromL );

    if( bIsDbl )
    {
        rResult.maSecn.mnLClip = rTLFromB.Secn() ? lclGetBeg( rTLFromB ) : lclGetBehindEnd( rTLFromB );
        rResult.maSecn.mnRClip = lclGetBeforeBeg( rBRFromT );
        rResult.maSecn.mnTClip = lclGetBehindEnd( rTLFromR );
        rResult.maSecn.mnBClip = rBRFromL.Secn() ? lclGetEnd( rBRFromL ) : lclGetBeforeBeg( rBRFromL );
    }
}

/** Calculates clipping offsets for a bottom-left to top-right frame border.

    This function can be used for single and double frame borders.
    See DrawDiagFrameBorders() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for all
        borders of the reference rectangle containing the diagonal frame border.
 */
void lclLinkBLTRFrameBorder(
        DiagBorderResult& rResult, const Style& rBorder,
        const Style& rBLFromT, const Style& rBLFromR, const Style& rTRFromB, const Style& rTRFromL )
{
    bool bIsDbl = rBorder.Secn() != 0;

    rResult.maPrim.mnLClip = lclGetBehindEnd( rBLFromT );
    rResult.maPrim.mnRClip = (bIsDbl && rTRFromB.Secn()) ? lclGetEnd( rTRFromB ) : lclGetBeforeBeg( rTRFromB );
    rResult.maPrim.mnTClip = lclGetBehindEnd( rTRFromL );
    rResult.maPrim.mnBClip = (bIsDbl && rBLFromR.Secn()) ? lclGetEnd( rBLFromR ) : lclGetBeforeBeg( rBLFromR );

    if( bIsDbl )
    {
        rResult.maSecn.mnLClip = rBLFromT.Secn() ? lclGetBeg( rBLFromT ) : lclGetBehindEnd( rBLFromT );
        rResult.maSecn.mnRClip = lclGetBeforeBeg( rTRFromB );
        rResult.maSecn.mnTClip = rTRFromL.Secn() ? lclGetBeg( rTRFromL ) : lclGetBehindEnd( rTRFromL );
        rResult.maSecn.mnBClip = lclGetBeforeBeg( rBLFromR );
    }
}

/** Calculates clipping offsets for both diagonal frame borders.

    This function can be used for single and double frame borders.
    See DrawDiagFrameBorders() function for a description of all parameters.

    @param rResult
        (out-param) The contained values (sub units) specify offsets for all
        borders of the reference rectangle containing the diagonal frame
        borders.
 */
void lclLinkDiagFrameBorders(
        DiagBordersResult& rResult, const Style& rTLBR, const Style& rBLTR,
        const Style& rTLFromB, const Style& rTLFromR, const Style& rBRFromT, const Style& rBRFromL,
        const Style& rBLFromT, const Style& rBLFromR, const Style& rTRFromB, const Style& rTRFromL )
{
    lclLinkTLBRFrameBorder( rResult.maTLBR, rTLBR, rTLFromB, rTLFromR, rBRFromT, rBRFromL );
    lclLinkBLTRFrameBorder( rResult.maBLTR, rBLTR, rBLFromT, rBLFromR, rTRFromB, rTRFromL );
}


// Drawing functions


// Simple helper functions

/** Converts sub units to OutputDevice map units. */
inline long lclToMapUnit( long nSubUnits )
{
    return ((nSubUnits < 0) ? (nSubUnits - 127) : (nSubUnits + 128)) / 256;
}

/** Converts a point in sub units to an OutputDevice point. */
inline Point lclToMapUnit( long nSubXPos, long nSubYPos )
{
    return Point( lclToMapUnit( nSubXPos ), lclToMapUnit( nSubYPos ) );
}

/** Returns a polygon constructed from a vector of points. */
inline tools::Polygon lclCreatePolygon( const PointVec& rPoints )
{
    return tools::Polygon( static_cast< sal_uInt16 >( rPoints.size() ), &rPoints[ 0 ] );
}

/** Returns a polygon constructed from the five passed points. */
vcl::Region lclCreatePolygon( const Point& rP1, const Point& rP2, const Point& rP3, const Point& rP4, const Point& rP5 )
{
    PointVec aPoints;
    aPoints.reserve( 5 );
    aPoints.push_back( rP1 );
    aPoints.push_back( rP2 );
    aPoints.push_back( rP3 );
    aPoints.push_back( rP4 );
    aPoints.push_back( rP5 );
    return vcl::Region(lclCreatePolygon(aPoints));
}

/** Sets the color of the passed frame style to the output device.

    Sets the line color and fill color in the output device.

    @param rDev
        The output device the color has to be set to. The old colors are pushed
        onto the device's stack and can be restored with a call to
        OutputDevice::Pop(). Please take care about the correct calling order
        of Pop() if this function is used with other functions pushing
        something onto the stack.
    @param rStyle
        The border style that contains the line color to be set to the device.
 */
void lclSetColorToOutDev( OutputDevice& rDev, const Color& rColor, const Color* pForceColor )
{
    rDev.Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
    rDev.SetLineColor( pForceColor ? *pForceColor : rColor );
    rDev.SetFillColor( pForceColor ? *pForceColor : rColor );
}


// Drawing of horizontal frame borders.

/** Draws a horizontal thin or thick line into the passed output device.

    The X coordinates of the edges of the line are adjusted according to the
    passed LineEndResult structs. A one pixel wide line can be drawn dotted.
 */
void lclDrawHorLine(
        OutputDevice& rDev,
        const Point& rLPos, const LineEndResult& rLRes,
        const Point& rRPos, const LineEndResult& rRRes,
        long nTOffs, long nBOffs, SvxBorderLineStyle nDashing )
{
    LinePoints aTPoints( rLPos + lclToMapUnit( rLRes.mnOffs1, nTOffs ), rRPos + lclToMapUnit( rRRes.mnOffs1, nTOffs ) );
    LinePoints aBPoints( rLPos + lclToMapUnit( rLRes.mnOffs2, nBOffs ), rRPos + lclToMapUnit( rRRes.mnOffs2, nBOffs ) );

    sal_uInt32 nWidth = lclToMapUnit( std::abs( nTOffs ) ) + lclToMapUnit( std::abs( nBOffs ) );
    if ( ( nTOffs >= 0 && nBOffs >= 0 ) || ( nTOffs <= 0 && nBOffs <= 0 ) )
        nWidth = std::abs( lclToMapUnit( nTOffs ) - lclToMapUnit( nBOffs ) ) + 1;
    Point rLMid = ( aTPoints.maBeg + aBPoints.maBeg ) / 2;
    Point rRMid = ( aTPoints.maEnd + aBPoints.maEnd ) / 2;

    ::svtools::DrawLine( rDev, rLMid, rRMid, nWidth, nDashing );
}

/** Draws a horizontal frame border into the passed output device.

    @param rLPos
        The top-left or bottom-left reference point of the diagonal frame border.
    @param rRPos
        The top-right or bottom-right reference point of the diagonal frame border.
    @param rBorder
        The frame style used to draw the border.
    @param rResult
        The X coordinates of the edges of all lines of the frame border are
        adjusted according to the offsets contained here.
 */
void lclDrawHorFrameBorder(
        OutputDevice& rDev, const Point& rLPos, const Point& rRPos,
        const Style& rBorder, const BorderResult& rResult, const Color* pForceColor )
{
    DBG_ASSERT( rBorder.Prim(), "svx::frame::lclDrawHorFrameBorder - line not visible" );
    DBG_ASSERT( rLPos.X() <= rRPos.X(), "svx::frame::lclDrawHorFrameBorder - wrong order of line ends" );
    DBG_ASSERT( rLPos.Y() == rRPos.Y(), "svx::frame::lclDrawHorFrameBorder - line not horizontal" );
    if( rLPos.X() <= rRPos.X() )
    {
        if ( rBorder.UseGapColor( ) )
        {
            lclSetColorToOutDev( rDev, rBorder.GetColorGap(), pForceColor );
            lclDrawHorLine( rDev, rLPos, rResult.maBeg.maGap, rRPos, rResult.maEnd.maGap,
                   lclGetPrimEnd( rBorder ), lclGetSecnBeg( rBorder ), rBorder.Type() );
            rDev.Pop(); // Gap color
        }

        lclSetColorToOutDev( rDev, rBorder.GetColorPrim(), pForceColor );
        lclDrawHorLine( rDev, rLPos, rResult.maBeg.maPrim, rRPos, rResult.maEnd.maPrim,
            lclGetBeg( rBorder ), lclGetPrimEnd( rBorder ), rBorder.Type() );
        rDev.Pop(); // colors

        if( rBorder.Secn() )
        {
            lclSetColorToOutDev( rDev, rBorder.GetColorSecn(), pForceColor );
            lclDrawHorLine( rDev, rLPos, rResult.maBeg.maSecn, rRPos, rResult.maEnd.maSecn,
                lclGetSecnBeg( rBorder ), lclGetEnd( rBorder ), rBorder.Type() );
            rDev.Pop(); // colors
        }
    }
}


// Drawing of vertical frame borders.

/** Draws a vertical thin or thick line into the passed output device.

    The Y coordinates of the edges of the line are adjusted according to the
    passed LineEndResult structs. A one pixel wide line can be drawn dotted.
 */
void lclDrawVerLine(
        OutputDevice& rDev,
        const Point& rTPos, const LineEndResult& rTRes,
        const Point& rBPos, const LineEndResult& rBRes,
        long nLOffs, long nROffs, SvxBorderLineStyle nDashing )
{
    LinePoints aLPoints( rTPos + lclToMapUnit( nLOffs, rTRes.mnOffs1 ), rBPos + lclToMapUnit( nLOffs, rBRes.mnOffs1 ) );
    LinePoints aRPoints( rTPos + lclToMapUnit( nROffs, rTRes.mnOffs2 ), rBPos + lclToMapUnit( nROffs, rBRes.mnOffs2 ) );

    sal_uInt32 nWidth = lclToMapUnit( std::abs( nLOffs ) ) + lclToMapUnit( std::abs( nROffs ) );
    if ( ( nLOffs >= 0 && nROffs >= 0 ) || ( nLOffs <= 0 && nROffs <= 0 ) )
        nWidth = std::abs( lclToMapUnit( nLOffs ) - lclToMapUnit( nROffs ) ) + 1;
    Point rTMid = ( aLPoints.maBeg + aRPoints.maBeg ) / 2;
    Point rBMid = ( aLPoints.maEnd + aRPoints.maEnd ) / 2;

    ::svtools::DrawLine( rDev, rTMid, rBMid, nWidth, nDashing );
}

/** Draws a vertical frame border into the passed output device.

    @param rTPos
        The top-left or top-right reference point of the diagonal frame border.
    @param rBPos
        The bottom-left or bottom-right reference point of the diagonal frame border.
    @param rBorder
        The frame style used to draw the border.
    @param rResult
        The Y coordinates of the edges of all lines of the frame border are
        adjusted according to the offsets contained here.
 */
void lclDrawVerFrameBorder(
        OutputDevice& rDev, const Point& rTPos, const Point& rBPos,
        const Style& rBorder, const BorderResult& rResult, const Color* pForceColor )
{
    DBG_ASSERT( rBorder.Prim(), "svx::frame::lclDrawVerFrameBorder - line not visible" );
    DBG_ASSERT( rTPos.Y() <= rBPos.Y(), "svx::frame::lclDrawVerFrameBorder - wrong order of line ends" );
    DBG_ASSERT( rTPos.X() == rBPos.X(), "svx::frame::lclDrawVerFrameBorder - line not vertical" );
    if( rTPos.Y() <= rBPos.Y() )
    {
        if ( rBorder.UseGapColor( ) )
        {
            lclSetColorToOutDev( rDev, rBorder.GetColorGap(), pForceColor );
            lclDrawVerLine( rDev, rTPos, rResult.maBeg.maGap, rBPos, rResult.maEnd.maGap,
                   lclGetPrimEnd( rBorder ), lclGetSecnBeg( rBorder ), rBorder.Type() );
            rDev.Pop(); // Gap color
        }

        lclSetColorToOutDev( rDev, rBorder.GetColorPrim(), pForceColor );
        lclDrawVerLine( rDev, rTPos, rResult.maBeg.maPrim, rBPos, rResult.maEnd.maPrim,
            lclGetBeg( rBorder ), lclGetPrimEnd( rBorder ), rBorder.Type() );
        rDev.Pop(); // colors
        if( rBorder.Secn() )
        {
            lclSetColorToOutDev( rDev, rBorder.GetColorSecn(), pForceColor );
            lclDrawVerLine( rDev, rTPos, rResult.maBeg.maSecn, rBPos, rResult.maEnd.maSecn,
                lclGetSecnBeg( rBorder ), lclGetEnd( rBorder ), rBorder.Type() );
            rDev.Pop(); // colors
        }
    }
}


// Drawing of diagonal frame borders, includes clipping functions.

/** Returns the drawing coordinates for a diagonal thin line.

    This function can be used for top-left to bottom-right and for bottom-left
    to top-right lines.

    @param rRect
        The reference rectangle of the diagonal frame border.
    @param bTLBR
        true = top-left to bottom-right; false = bottom-left to top-right.
    @param nDiagOffs
        Width offset (sub units) across the diagonal frame border.
    @return
        A struct containg start and end position of the diagonal line.
 */
LinePoints lclGetDiagLineEnds( const tools::Rectangle& rRect, bool bTLBR, long nDiagOffs )
{
    LinePoints aPoints( rRect, bTLBR );
    bool bVert = rRect.GetWidth() < rRect.GetHeight();
    double fAngle = bVert ? GetVerDiagAngle( rRect ) : GetHorDiagAngle( rRect );
    // vertical top-left to bottom-right borders are handled mirrored
    if( bVert && bTLBR )
        nDiagOffs = -nDiagOffs;
    long nTOffs = bTLBR ? GetTLDiagOffset( 0, nDiagOffs, fAngle ) : GetTRDiagOffset( nDiagOffs, fAngle );
    long nBOffs = bTLBR ? GetBRDiagOffset( nDiagOffs, fAngle ) : GetBLDiagOffset( 0, nDiagOffs, fAngle );
    // vertical bottom-left to top-right borders are handled with exchanged end points
    if( bVert && !bTLBR )
        std::swap( nTOffs, nBOffs );
    (bVert ? aPoints.maBeg.Y() : aPoints.maBeg.X()) += lclToMapUnit( nTOffs );
    (bVert ? aPoints.maEnd.Y() : aPoints.maEnd.X()) += lclToMapUnit( nBOffs );
    return aPoints;
}


// Clipping functions for diagonal frame borders.

/** Limits the clipping region to the inner area of a rectangle.

    Takes the values from the passed DiagLineResult struct into account. They
    may specify to not clip one or more borders of a rectangle.

    @param rDev
        The output device with the clipping region to be modified. The old
        clipping region is pushed onto the device's stack and can be restored
        with a call to OutputDevice::Pop(). Please take care about the correct
        calling order of Pop() if this function is used with other functions
        pushing something onto the stack.
    @param rRect
        The reference rectangle of the diagonal frame borders.
    @param rResult
        The result struct containing modifies for each border of the reference
        rectangle.
 */
void lclPushDiagClipRect( OutputDevice& rDev, const tools::Rectangle& rRect, const DiagLineResult& rResult )
{
    // PixelToLogic() regards internal offset of the output device
    tools::Rectangle aClipRect( rRect );
    aClipRect.Left()   += lclToMapUnit( rResult.mnLClip );
    aClipRect.Top()    += lclToMapUnit( rResult.mnTClip );
    aClipRect.Right()  += lclToMapUnit( rResult.mnRClip );
    aClipRect.Bottom() += lclToMapUnit( rResult.mnBClip );
    // output device would adjust the rectangle -> invalidate it before
    if( (aClipRect.GetWidth() < 1) ||(aClipRect.GetHeight() < 1) )
        aClipRect.SetEmpty();

    rDev.Push( PushFlags::CLIPREGION );
    rDev.IntersectClipRegion( aClipRect );
}

/** Excludes inner area of a crossing double frame border from clipping region.

    This function is used to modify the clipping region so that it excludes the
    inner free area of a double diagonal frame border. This makes it possible
    to draw a diagonal frame border in one step without taking care of the
    crossing double frame border.

    @param rDev
        The output device with the clipping region to be modified. The old
        clipping region is pushed onto the device's stack and can be restored
        with a call to OutputDevice::Pop(). Please take care about the correct
        calling order of Pop() if this function is used with other functions
        pushing something onto the stack.
    @param rRect
        The reference rectangle of the diagonal frame borders.
    @param bTLBR
        The orientation of the processed frame border (not the orientation of
        the crossing frame border).
    @param bCrossStyle
        The style of the crossing frame border. Must be a double frame style.
 */
void lclPushCrossingClipRegion( OutputDevice& rDev, const tools::Rectangle& rRect, bool bTLBR, const Style& rCrossStyle )
{
    DBG_ASSERT( rCrossStyle.Secn(), "lclGetCrossingClipRegion - use only for double styles" );
    LinePoints aLPoints( lclGetDiagLineEnds( rRect, !bTLBR, lclGetPrimEnd( rCrossStyle ) ) );
    LinePoints aRPoints( lclGetDiagLineEnds( rRect, !bTLBR, lclGetSecnBeg( rCrossStyle ) ) );

    vcl::Region aClipReg;
    if( bTLBR )
    {
        aClipReg = lclCreatePolygon(
            aLPoints.maBeg, aLPoints.maEnd, rRect.BottomRight(), rRect.BottomLeft(), rRect.TopLeft() );
        aClipReg.Union( lclCreatePolygon(
            aRPoints.maBeg, aRPoints.maEnd, rRect.BottomRight(), rRect.TopRight(), rRect.TopLeft() ) );
    }
    else
    {
        aClipReg = lclCreatePolygon(
            aLPoints.maBeg, aLPoints.maEnd, rRect.BottomLeft(), rRect.TopLeft(), rRect.TopRight() );
        aClipReg.Union( lclCreatePolygon(
            aRPoints.maBeg, aRPoints.maEnd, rRect.BottomLeft(), rRect.BottomRight(), rRect.TopRight() ) );
    }

    rDev.Push( PushFlags::CLIPREGION );
    rDev.IntersectClipRegion( aClipReg );
}


// Drawing functions for diagonal frame borders.

/** Draws a diagonal thin or thick line into the passed output device.

    The clipping region of the output device is modified according to the
    passed DiagLineResult struct. A one pixel wide line can be drawn dotted.
 */
void lclDrawDiagLine(
        OutputDevice& rDev, const tools::Rectangle& rRect, bool bTLBR,
        const DiagLineResult& rResult, long nDiagOffs1, long nDiagOffs2, SvxBorderLineStyle nDashing )
{
    lclPushDiagClipRect( rDev, rRect, rResult );
    LinePoints aLPoints( lclGetDiagLineEnds( rRect, bTLBR, nDiagOffs1 ) );
    LinePoints aL2Points( lclGetDiagLineEnds( rRect, bTLBR, nDiagOffs2 ) );
    Point aSMid( ( aLPoints.maBeg + aL2Points.maBeg ) / 2 );
    Point aEMid( ( aLPoints.maEnd + aL2Points.maEnd ) / 2 );

    sal_uInt32 nWidth = lclToMapUnit( std::abs( nDiagOffs1 ) ) + lclToMapUnit( std::abs( nDiagOffs2 ) );
    if ( ( nDiagOffs1 <= 0 && nDiagOffs2 <= 0 ) || ( nDiagOffs1 >=0 && nDiagOffs2 >=0 ) )
        nWidth = lclToMapUnit( std::abs( nDiagOffs1 - nDiagOffs2 ) );

    svtools::DrawLine( rDev, aSMid, aEMid, nWidth, nDashing );
    rDev.Pop(); // clipping region
}

/** Draws a diagonal frame border into the passed output device.

    The lines of the frame border are drawn interrupted, if the style of the
    crossing frame border is double.

    @param rRect
        The reference rectangle of the diagonal frame border.
    @param bTLBR
        The orientation of the diagonal frame border.
    @param rBorder
        The frame style used to draw the border.
    @param rResult
        Offsets (sub units) to modify the clipping region of the output device.
    @param rCrossStyle
        Style of the crossing diagonal frame border.
 */
void lclDrawDiagFrameBorder(
        OutputDevice& rDev, const tools::Rectangle& rRect, bool bTLBR,
        const Style& rBorder, const DiagBorderResult& rResult, const Style& rCrossStyle,
        const Color* pForceColor, bool bDiagDblClip )
{
    DBG_ASSERT( rBorder.Prim(), "svx::frame::lclDrawDiagFrameBorder - line not visible" );

    bool bClip = bDiagDblClip && rCrossStyle.Secn();
    if( bClip )
        lclPushCrossingClipRegion( rDev, rRect, bTLBR, rCrossStyle );

    lclSetColorToOutDev( rDev, rBorder.GetColorPrim(), pForceColor );
    lclDrawDiagLine( rDev, rRect, bTLBR, rResult.maPrim, lclGetBeg( rBorder ), lclGetPrimEnd( rBorder ), rBorder.Type() );
    rDev.Pop(); // colors
    if( rBorder.Secn() )
    {
        if ( rBorder.UseGapColor( ) )
        {
            lclSetColorToOutDev( rDev, rBorder.GetColorGap(), pForceColor );
            lclDrawDiagLine( rDev, rRect, bTLBR, rResult.maSecn, lclGetDistBeg( rBorder ), lclGetDistEnd( rBorder ), rBorder.Type() );
            rDev.Pop(); // colors
        }

        lclSetColorToOutDev( rDev, rBorder.GetColorSecn(), pForceColor );
        lclDrawDiagLine( rDev, rRect, bTLBR, rResult.maSecn, lclGetSecnBeg( rBorder ), lclGetEnd( rBorder ), rBorder.Type() );
        rDev.Pop(); // colors
    }

    if( bClip )
        rDev.Pop(); // clipping region
}

/** Draws both diagonal frame borders into the passed output device.

    The lines of each frame border is drawn interrupted, if the style of the
    other crossing frame border is double.

    @param rRect
        The reference rectangle of the diagonal frame borders.
    @param rTLBR
        The frame style of the top-left to bottom-right frame border.
    @param rBLTR
        The frame style of the bottom-left to top-right frame border.
    @param rResult
        Offsets (sub units) to modify the clipping region of the output device.
 */
void lclDrawDiagFrameBorders(
        OutputDevice& rDev, const tools::Rectangle& rRect,
        const Style& rTLBR, const Style& rBLTR, const DiagBordersResult& rResult,
        const Color* pForceColor, bool bDiagDblClip )
{
    DBG_ASSERT( (rRect.GetWidth() > 1) && (rRect.GetHeight() > 1), "svx::frame::lclDrawDiagFrameBorders - rectangle too small" );
    if( (rRect.GetWidth() > 1) && (rRect.GetHeight() > 1) )
    {
        bool bDrawTLBR = rTLBR.Prim() != 0;
        bool bDrawBLTR = rBLTR.Prim() != 0;
        bool bFirstDrawBLTR = rTLBR.Secn() != 0;

        if( bDrawBLTR && bFirstDrawBLTR )
            lclDrawDiagFrameBorder( rDev, rRect, false, rBLTR, rResult.maBLTR, rTLBR, pForceColor, bDiagDblClip );
        if( bDrawTLBR )
            lclDrawDiagFrameBorder( rDev, rRect, true, rTLBR, rResult.maTLBR, rBLTR, pForceColor, bDiagDblClip );
        if( bDrawBLTR && !bFirstDrawBLTR )
            lclDrawDiagFrameBorder( rDev, rRect, false, rBLTR, rResult.maBLTR, rTLBR, pForceColor, bDiagDblClip );
    }
}


} // namespace


// Classes


#define SCALEVALUE( value ) lclScaleValue( value, fScale, nMaxWidth )

Style::Style() :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(SvxBorderLineStyle::SOLID)
{
    Clear();
}

Style::Style( double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(nType)
{
    Clear();
    Set( nP, nD, nS );
}

Style::Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor,
              double nP, double nD, double nS, SvxBorderLineStyle nType ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(1.0),
    mnType(nType)
{
    Set( rColorPrim, rColorSecn, rColorGap, bUseGapColor, nP, nD, nS );
}

Style::Style( const editeng::SvxBorderLine* pBorder, double fScale ) :
    meRefMode(RefMode::Centered),
    mfPatternScale(fScale)
{
    Set( pBorder, fScale );
}


void Style::SetPatternScale( double fScale )
{
    mfPatternScale = fScale;
}

void Style::Clear()
{
    Set( Color(), Color(), Color(), false, 0, 0, 0 );
}

void Style::Set( double nP, double nD, double nS )
{
    /*  nP  nD  nS  ->  mfPrim  mfDist  mfSecn
        --------------------------------------
        any any 0       nP      0       0
        0   any >0      nS      0       0
        >0  0   >0      nP      0       0
        >0  >0  >0      nP      nD      nS
     */
    mfPrim = rtl::math::round(nP ? nP : nS, 2);
    mfDist = rtl::math::round((nP && nS) ? nD : 0, 2);
    mfSecn = rtl::math::round((nP && nD) ? nS : 0, 2);
}

void Style::Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS )
{
    maColorPrim = rColorPrim;
    maColorSecn = rColorSecn;
    maColorGap = rColorGap;
    mbUseGapColor = bUseGapColor;
    Set( nP, nD, nS );
}

void Style::Set( const SvxBorderLine& rBorder, double fScale, sal_uInt16 nMaxWidth )
{
    maColorPrim = rBorder.GetColorOut();
    maColorSecn = rBorder.GetColorIn();
    maColorGap = rBorder.GetColorGap();
    mbUseGapColor = rBorder.HasGapColor();

    sal_uInt16 nPrim = rBorder.GetOutWidth();
    sal_uInt16 nDist = rBorder.GetDistance();
    sal_uInt16 nSecn = rBorder.GetInWidth();

    mnType = rBorder.GetBorderLineStyle();
    if( !nSecn )    // no or single frame border
    {
        Set( SCALEVALUE( nPrim ), 0, 0 );
    }
    else
    {
        Set( SCALEVALUE( nPrim ), SCALEVALUE( nDist ), SCALEVALUE( nSecn ) );
        // Enlarge the style if distance is too small due to rounding losses.
        double nPixWidth = SCALEVALUE( nPrim + nDist + nSecn );
        if( nPixWidth > GetWidth() )
            mfDist = nPixWidth - mfPrim - mfSecn;
        // Shrink the style if it is too thick for the control.
        while( GetWidth() > nMaxWidth )
        {
            // First decrease space between lines.
            if (mfDist)
                --mfDist;
            // Still too thick? Decrease the line widths.
            if( GetWidth() > nMaxWidth )
            {
                if (mfPrim != 0.0 && rtl::math::approxEqual(mfPrim, mfSecn))
                {
                    // Both lines equal - decrease both to keep symmetry.
                    --mfPrim;
                    --mfSecn;
                }
                else
                {
                    // Decrease each line for itself
                    if (mfPrim)
                        --mfPrim;
                    if ((GetWidth() > nMaxWidth) && mfSecn != 0.0)
                        --mfSecn;
                }
            }
        }
    }
}

void Style::Set( const SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth )
{
    if( pBorder )
        Set( *pBorder, fScale, nMaxWidth );
    else
    {
        Clear();
        mnType = SvxBorderLineStyle::SOLID;
    }
}

Style& Style::MirrorSelf()
{
    if (mfSecn)
        std::swap( mfPrim, mfSecn );
    if( meRefMode != RefMode::Centered )
        meRefMode = (meRefMode == RefMode::Begin) ? RefMode::End : RefMode::Begin;
    return *this;
}

Style Style::Mirror() const
{
    return Style( *this ).MirrorSelf();
}

bool operator==( const Style& rL, const Style& rR )
{
    return (rL.Prim() == rR.Prim()) && (rL.Dist() == rR.Dist()) && (rL.Secn() == rR.Secn()) &&
        (rL.GetColorPrim() == rR.GetColorPrim()) && (rL.GetColorSecn() == rR.GetColorSecn()) &&
        (rL.GetColorGap() == rR.GetColorGap()) && (rL.GetRefMode() == rR.GetRefMode()) &&
        (rL.UseGapColor() == rR.UseGapColor() ) && (rL.Type() == rR.Type());
}

bool operator<( const Style& rL, const Style& rR )
{
    // different total widths -> rL<rR, if rL is thinner
    double nLW = rL.GetWidth();
    double nRW = rR.GetWidth();
    if( !rtl::math::approxEqual(nLW, nRW) ) return nLW < nRW;

    // one line double, the other single -> rL<rR, if rL is single
    if( (rL.Secn() == 0) != (rR.Secn() == 0) ) return rL.Secn() == 0;

    // both lines double with different distances -> rL<rR, if distance of rL greater
    if( (rL.Secn() && rR.Secn()) && !rtl::math::approxEqual(rL.Dist(), rR.Dist()) ) return rL.Dist() > rR.Dist();

    // both lines single and 1 unit thick, only one is dotted -> rL<rR, if rL is dotted
    if( (nLW == 1) && (rL.Type() != rR.Type()) ) return rL.Type() != SvxBorderLineStyle::SOLID;

    // seem to be equal
    return false;
}

#undef SCALEVALUE


// Various helper functions


double GetHorDiagAngle( long nWidth, long nHeight )
{
    return atan2( static_cast< double >( std::abs( nHeight ) ), static_cast< double >( std::abs( nWidth ) ) );
}


long GetTLDiagOffset( long nVerOffs, long nDiagOffs, double fAngle )
{
    return lclD2L( nVerOffs / tan( fAngle ) + nDiagOffs / sin( fAngle ) );
}

long GetBLDiagOffset( long nVerOffs, long nDiagOffs, double fAngle )
{
    return lclD2L( -nVerOffs / tan( fAngle ) + nDiagOffs / sin( fAngle ) );
}

long GetBRDiagOffset( long nDiagOffs, double fAngle )
{
    return -lclD2L( - nDiagOffs / sin( fAngle ) );
}

long GetTRDiagOffset( long nDiagOffs, double fAngle )
{
    return -lclD2L( - nDiagOffs / sin( fAngle ) );
}


bool CheckFrameBorderConnectable( const Style& rLBorder, const Style& rRBorder,
        const Style& rTFromTL, const Style& rTFromT, const Style& rTFromTR,
        const Style& rBFromBL, const Style& rBFromB, const Style& rBFromBR )
{
    return      // returns 1 AND (2a OR 2b)
        // 1) only, if both frame borders are equal
        (rLBorder == rRBorder)
        &&
        (
            (
                // 2a) if the borders are not double, at least one of the vertical must not be double
                !rLBorder.Secn() && (!rTFromT.Secn() || !rBFromB.Secn())
            )
            ||
            (
                // 2b) if the borders are double, all other borders must not be double
                rLBorder.Secn() &&
                !rTFromTL.Secn() && !rTFromT.Secn() && !rTFromTR.Secn() &&
                !rBFromBL.Secn() && !rBFromB.Secn() && !rBFromBR.Secn()
            )
        );
}


// Drawing functions


double lcl_GetExtent( const Style& rBorder, const Style& rSide, const Style& rOpposite,
                      long nAngleSide, long nAngleOpposite )
{
    Style aOtherBorder = rSide;
    long nOtherAngle = nAngleSide;
    if ( rSide.GetWidth() == 0 && rOpposite.GetWidth() > 0 )
    {
        nOtherAngle = nAngleOpposite;
        aOtherBorder = rOpposite;
    }
    else if ( rSide.GetWidth() == 0 && rOpposite.GetWidth() == 0 )
    {
        if ( ( nAngleOpposite % 18000 ) == 0 )
            nOtherAngle = nAngleSide;
        else if ( ( nAngleSide % 18000 ) == 0 )
            nOtherAngle = nAngleOpposite;
    }

    // Let's assume the border we are drawing is horizontal and compute all the angles / distances from this
    basegfx::B2DVector aBaseVector( 1.0, 0.0 );
    basegfx::B2DPoint aBasePoint( 0.0, static_cast<double>( rBorder.GetWidth() / 2 ) );

    basegfx::B2DHomMatrix aRotation;
    aRotation.rotate( double( nOtherAngle ) * M_PI / 18000.0 );

    basegfx::B2DVector aOtherVector = aRotation * aBaseVector;
    // Compute a line shifted by half the width of the other border
    basegfx::B2DVector aPerpendicular = basegfx::getNormalizedPerpendicular( aOtherVector );
    basegfx::B2DPoint aOtherPoint = basegfx::B2DPoint() + aPerpendicular * aOtherBorder.GetWidth() / 2;

    // Find the cut between the two lines
    double nCut = 0.0;
    basegfx::tools::findCut(
            aBasePoint, aBaseVector, aOtherPoint, aOtherVector,
            CutFlagValue::ALL, &nCut );

    return nCut;
}

drawinglayer::primitive2d::Primitive2DReference CreateBorderPrimitives(
        const Point& rLPos, const Point& rRPos, const Style& rBorder,
        const DiagStyle& /*rLFromTR*/, const Style& rLFromT, const Style& /*rLFromL*/, const Style& rLFromB, const DiagStyle& /*rLFromBR*/,
        const DiagStyle& /*rRFromTL*/, const Style& rRFromT, const Style& /*rRFromR*/, const Style& rRFromB, const DiagStyle& /*rRFromBL*/,
        const Color* /*pForceColor*/, long nRotateT, long nRotateB )
{
    basegfx::B2DPoint aStart( rLPos.getX(), rLPos.getY() );
    basegfx::B2DPoint aEnd( rRPos.getX(), rRPos.getY() );

    return drawinglayer::primitive2d::Primitive2DReference(
        new drawinglayer::primitive2d::BorderLinePrimitive2D(
            aStart, aEnd,
            rBorder.Prim(),
            rBorder.Dist(),
            rBorder.Secn(),
            lcl_GetExtent( rBorder, rLFromT, rLFromB, nRotateT, - nRotateB ),
            lcl_GetExtent( rBorder, rRFromT, rRFromB, 18000 - nRotateT, nRotateB - 18000 ),
            lcl_GetExtent( rBorder, rLFromB, rLFromT, nRotateB, - nRotateT ),
            lcl_GetExtent( rBorder, rRFromB, rRFromT, 18000 - nRotateB, nRotateT - 18000 ),
            rBorder.GetColorSecn().getBColor(),
            rBorder.GetColorPrim().getBColor(),
            rBorder.GetColorGap().getBColor(),
            rBorder.UseGapColor(), rBorder.Type(), rBorder.PatternScale()));
}

drawinglayer::primitive2d::Primitive2DReference CreateBorderPrimitives(
        const Point& rLPos, const Point& rRPos, const Style& rBorder,
        const Style& rLFromT, const Style& rLFromL, const Style& rLFromB,
        const Style& rRFromT, const Style& rRFromR, const Style& rRFromB,
        const Color* pForceColor, long nRotateT, long nRotateB )
{
    return CreateBorderPrimitives( rLPos, rRPos, rBorder,
            DiagStyle(), rLFromT, rLFromL, rLFromB, DiagStyle(),
            DiagStyle(), rRFromT, rRFromR, rRFromB, DiagStyle(),
            pForceColor, nRotateT, nRotateB );
}

void DrawHorFrameBorder( OutputDevice& rDev,
        const Point& rLPos, const Point& rRPos, const Style& rBorder,
        const DiagStyle& rLFromTR, const Style& rLFromT, const Style& rLFromL, const Style& rLFromB, const DiagStyle& rLFromBR,
        const DiagStyle& rRFromTL, const Style& rRFromT, const Style& rRFromR, const Style& rRFromB, const DiagStyle& rRFromBL,
        const Color* pForceColor )
{
    if( rBorder.Prim() )
    {
        BorderResult aResult;
        lclLinkHorFrameBorder( aResult, rBorder,
            rLFromTR, rLFromT, rLFromL, rLFromB, rLFromBR,
            rRFromTL, rRFromT, rRFromR, rRFromB, rRFromBL );
        lclDrawHorFrameBorder( rDev, rLPos, rRPos, rBorder, aResult, pForceColor );
    }
}


void DrawVerFrameBorder( OutputDevice& rDev,
        const Point& rTPos, const Point& rBPos, const Style& rBorder,
        const DiagStyle& rTFromBL, const Style& rTFromL, const Style& rTFromT, const Style& rTFromR, const DiagStyle& rTFromBR,
        const DiagStyle& rBFromTL, const Style& rBFromL, const Style& rBFromB, const Style& rBFromR, const DiagStyle& rBFromTR,
        const Color* pForceColor )
{
    if( rBorder.Prim() )
    {
        BorderResult aResult;
        lclLinkVerFrameBorder( aResult, rBorder,
            rTFromBL, rTFromL, rTFromT, rTFromR, rTFromBR,
            rBFromTL, rBFromL, rBFromB, rBFromR, rBFromTR );
        lclDrawVerFrameBorder( rDev, rTPos, rBPos, rBorder, aResult, pForceColor );
    }
}


void DrawDiagFrameBorders(
        OutputDevice& rDev, const tools::Rectangle& rRect, const Style& rTLBR, const Style& rBLTR,
        const Style& rTLFromB, const Style& rTLFromR, const Style& rBRFromT, const Style& rBRFromL,
        const Style& rBLFromT, const Style& rBLFromR, const Style& rTRFromB, const Style& rTRFromL,
        const Color* pForceColor, bool bDiagDblClip )
{
    if( rTLBR.Prim() || rBLTR.Prim() )
    {
        DiagBordersResult aResult;
        lclLinkDiagFrameBorders( aResult, rTLBR, rBLTR,
            rTLFromB, rTLFromR, rBRFromT, rBRFromL, rBLFromT, rBLFromR, rTRFromB, rTRFromL );
        lclDrawDiagFrameBorders( rDev, rRect, rTLBR, rBLTR, aResult, pForceColor, bDiagDblClip );
    }
}


}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
