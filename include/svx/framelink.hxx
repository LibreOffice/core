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

#ifndef INCLUDED_SVX_FRAMELINK_HXX
#define INCLUDED_SVX_FRAMELINK_HXX

#include <algorithm>
#include <sal/types.h>
#include <tools/color.hxx>
#include <svx/svxdllapi.h>
#include <vcl/outdev.hxx>
#include <editeng/borderline.hxx>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

class Point;
namespace tools { class Rectangle; }
class OutputDevice;

namespace svx {
namespace frame {


// Enums


/** Specifies how the reference points for frame borders are used.
 */
enum class RefMode
{
    /** Frame borders are drawn centered to the reference points. */
    Centered,

    /** The reference points specify the begin of the frame border width.

        The result is that horizontal lines are drawn below, and vertical lines
        are drawn right of the reference points.
     */
    Begin,

    /** The reference points specify the end of the frame border width.

        The result is that horizontal lines are drawn above, and vertical lines
        are drawn left of the reference points.
     */
    End
};


// Classes


/** Contains the widths of primary and secondary line of a frame style.

    In the following, "frame style" is a complete style of one frame border,
    i.e. the double line at the left side of the frame. A "line" is always a
    trivial single line, i.e. the first line of a double frame style.

    The following states of the members of this struct are valid:

    mnPrim      mnDist      mnSecn      frame style
    -------------------------------------------------
    0           0           0           invisible
    >0          0           0           single
    >0          >0          >0          double

    The behaviour of the member functions for other states is not defined.

    Per definition the primary line in double frame styles is:
    -   The top line for horizontal frame borders.
    -   The left line for vertical frame borders.
    -   The bottom-left line for top-left to bottom-right diagonal frame borders.
    -   The top-left line for bottom-left to top-right diagonal frame borders.

    The following picture shows the upper end of a vertical double frame
    border.

        |<---------------- GetWidth() ----------------->|
        |                                               |
        |<----- mnPrim ----->||<- mnDist ->||<- mnSecn >|
        |                    ||            ||           |
        ######################              #############
        ######################              #############
        ######################              #############
        ######################              #############
        ######################  |           #############
        ######################  |           #############
                                |
                                |<- middle of the frame border
 */
class SAL_WARN_UNUSED SVX_DLLPUBLIC Style
{
public:
    /** Constructs an invisible frame style. */
    explicit Style();
    /** Constructs a frame style with passed line widths. */
    explicit Style( double nP, double nD, double nS, SvxBorderLineStyle nType );
    /** Constructs a frame style with passed color and line widths. */
    explicit Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor,
                    double nP, double nD, double nS, SvxBorderLineStyle nType );
    /** Constructs a frame style from the passed SvxBorderLine struct. Clears the style, if pBorder is 0. */
    explicit Style( const editeng::SvxBorderLine* pBorder, double fScale = 1.0 );

    RefMode      GetRefMode() const { return meRefMode; }
    const Color& GetColorPrim() const { return maColorPrim; }
    const Color& GetColorSecn() const { return maColorSecn; }
    const Color& GetColorGap() const { return maColorGap; }
    bool         UseGapColor() const { return mbUseGapColor; }
    double       Prim() const { return mfPrim; }
    double       Dist() const { return mfDist; }
    double       Secn() const { return mfSecn; }
    double PatternScale() const { return mfPatternScale;}
    void SetPatternScale( double fScale );
    SvxBorderLineStyle Type() const { return mnType; }

    /** Returns the total width of this frame style. */
    double       GetWidth() const { return mfPrim + mfDist + mfSecn; }

    /** Sets the frame style to invisible state. */
    void                Clear();
    /** Sets the frame style to the passed line widths. */
    void                Set( double nP, double nD, double nS );
    /** Sets the frame style to the passed line widths. */
    void                Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor,
                            double nP, double nD, double nS );
    /** Sets the frame style to the passed SvxBorderLine struct. */
    void                Set( const editeng::SvxBorderLine& rBorder, double fScale, sal_uInt16 nMaxWidth = SAL_MAX_UINT16 );
    /** Sets the frame style to the passed SvxBorderLine struct. Clears the style, if pBorder is 0. */
    void                Set( const editeng::SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth = SAL_MAX_UINT16 );

    /** Sets a new reference point handling mode, does not modify other settings. */
    void         SetRefMode( RefMode eRefMode ) { meRefMode = eRefMode; }
    /** Sets a new color, does not modify other settings. */
    void         SetColorPrim( const Color& rColor ) { maColorPrim = rColor; }
    void         SetColorSecn( const Color& rColor ) { maColorSecn = rColor; }
    /** Sets whether to use dotted style for single hair lines. */
    void         SetType( SvxBorderLineStyle nType ) { mnType = nType; }

    /** Mirrors this style (exchanges primary and secondary), if it is a double frame style. */
    Style&              MirrorSelf();
    /** Returns this style mirrored, if it is a double frame style, otherwise a simple copy. */
    Style               Mirror() const;

private:
    Color               maColorPrim;
    Color               maColorSecn;
    Color               maColorGap;
    bool                mbUseGapColor;
    RefMode             meRefMode;  /// Reference point handling for this frame border.
    double              mfPrim;     /// Width of primary (single, left, or top) line.
    double              mfDist;     /// Distance between primary and secondary line.
    double              mfSecn;     /// Width of secondary (right or bottom) line.
    double              mfPatternScale; /// Scale used for line pattern spacing.
    SvxBorderLineStyle  mnType;
};

bool operator==( const Style& rL, const Style& rR );
SVX_DLLPUBLIC bool operator<( const Style& rL, const Style& rR );

inline bool operator>( const Style& rL, const Style& rR ) { return rR < rL; }
inline bool operator<=( const Style& rL, const Style& rR ) { return !(rR < rL); }


/** Extends the Style struct with an angle for diagonal frame borders.

    The angle is specified in radian (a full circle is equivalent to 2*PI).
    It is dependent on the context, how the value is interpreted, i.e. it may
    specify the angle to a horizontal or vertical frame border.
 */
class SAL_WARN_UNUSED DiagStyle : public Style
{
public:
    /** Constructs an invisible diagonal frame style. */
    explicit     DiagStyle() : mfAngle( 0.0 ) {}
    /** Constructs a diagonal frame style passed style and angle. */
    explicit     DiagStyle( const Style& rStyle, double fAngle ) :
                            Style( rStyle ), mfAngle( fAngle ) {}

    double       GetAngle() const { return mfAngle; }

    /** Returns this style mirrored, if it is a double frame style, otherwise a simple copy. */
    DiagStyle    Mirror() const { return DiagStyle( Style::Mirror(), mfAngle ); }

private:
    double              mfAngle;    /// Angle between this and hor. or vert. border.
};


// Various helper functions


/** Returns the angle between horizontal border of a rectangle and its diagonal.

    The returned values represents the inner angle between the diagonals and
    horizontal borders, and is therefore in the range [0,PI/2] (inclusive). The
    passed sizes may be negative, calculation is done with absolute values.
 */
SVX_DLLPUBLIC double GetHorDiagAngle( long nWidth, long nHeight );

/** Returns the angle between horizontal border of a rectangle and its diagonal.

    The returned values represents the inner angle between the diagonals and
    horizontal borders, and is therefore in the range [0,PI/2] (inclusive).
 */
inline double GetHorDiagAngle( const tools::Rectangle& rRect )
{ return GetHorDiagAngle( rRect.GetWidth(), rRect.GetHeight() ); }


/** Returns the angle between vertical border of a rectangle and its diagonal.

    The returned values represents the inner angle between the diagonals and
    vertical borders, and is therefore in the range [0,PI/2] (inclusive). The
    passed sizes may be negative, calculation is done with absolute values.
 */
inline double GetVerDiagAngle( long nWidth, long nHeight )
{ return GetHorDiagAngle( nHeight, nWidth ); }

/** Returns the angle between vertical border of a rectangle and its diagonal.

    The returned values represents the inner angle between the diagonals and
    vertical borders, and is therefore in the range [0,PI/2] (inclusive).
 */
inline double GetVerDiagAngle( const tools::Rectangle& rRect )
{ return GetVerDiagAngle( rRect.GetWidth(), rRect.GetHeight() ); }


/** Returns an X coordinate for a diagonal frame border in the specified height.

    This function is for usage with the top-left end of a top-left to
    bottom-right diagonal frame border, connected to the left end of a
    horizontal frame border.

    The function returns the relative X position (i.e. for a polygon) of the
    diagonal frame border according to the specified relative Y position. The
    mentioned positions are relative to the reference point of both frame
    borders.

                +----------------------------------------------------------
                |               The horizontal frame border.
                |    |
    - - - - - - |  --+--  <---- Reference point for horizontal and diagonal frame borders.
      ^         | \  |  \
     nVerOffs   |  \     \ <--- The diagonal frame border.
      v         +---\     \------------------------------------------------
    - - - - - - - - -\- - -X <----- The function calculates the X position of i.e.
                      \     \       this point (relative from X of reference point).
                       \     \
             Primary -->\     \<-- Secondary

    @param nVerOffs
        The vertical position of the point to be calculated, relative to the Y
        coordinate of the reference point.
    @param nDiagOffs
        The width offset across the diagonal frame border (0 = middle),
        regardless of the gradient of the diagonal frame border (always
        vertical to the direction of the diagonal frame border). This value is
        not related in any way to the reference point. For details about
        relative width offsets, see description of class Style.
    @param fAngle
        Inner (right) angle between diagonal and horizontal frame border.
 */
SVX_DLLPUBLIC long GetTLDiagOffset( long nVerOffs, long nDiagOffs, double fAngle );

/** Returns an X coordinate for a diagonal frame border in the specified height.

    This function is for usage with the bottom-left end of a bottom-left to
    top-right diagonal frame border, connected to the left end of a horizontal
    frame border.

    The function returns the relative X position (i.e. for a polygon) of the
    diagonal frame border according to the specified relative Y position. The
    mentioned positions are relative to the reference point of both frame
    borders.

             Primary -->/     /<--- Secondary
                       /     /
                      /     /       The function calculates the X position of i.e.
    - - - - - - - - -/- - -X <----- this point (relative from X of reference point).
      ^         +---/     /------------------------------------------------
     nVerOffs   |  /     / <--- The diagonal frame border.
      v         | /  |  /
    - - - - - - |  --+--  <---- Reference point for horizontal and diagonal frame borders.
                |    |
                |               The horizontal frame border.
                +----------------------------------------------------------

    @param nVerOffs
        The vertical position of the point to be calculated, relative to the Y
        coordinate of the reference point.
    @param nDiagOffs
        The width offset across the diagonal frame border (0 = middle),
        regardless of the gradient of the diagonal frame border (always
        vertical to the direction of the diagonal frame border). This value is
        not related in any way to the reference point. For details about
        relative width offsets, see description of class Style.
    @param fAngle
        Inner (right) angle between diagonal and horizontal frame border.
 */
long GetBLDiagOffset( long nVerOffs, long nDiagOffs, double fAngle );

/** Returns an X coordinate for a diagonal frame border in the specified height.

    This function is for usage with the bottom-right end of a top-left to
    bottom-right diagonal frame border, connected to the right end of a
    horizontal frame border.

    @param nDiagOffs
        The width offset across the diagonal frame border (0 = middle),
        regardless of the gradient of the diagonal frame border (always
        vertical to the direction of the diagonal frame border). This value is
        not related in any way to the reference point. For details about
        relative width offsets, see description of class Style.
    @param fAngle
        Inner (left) angle between diagonal and horizontal frame border.
 */
long GetBRDiagOffset( long nDiagOffs, double fAngle );

/** Returns an X coordinate for a diagonal frame border in the specified height.

    This function is for usage with the top-right end of a bottom-left to
    top-right diagonal frame border, connected to the right end of a horizontal
    frame border.

    @param nDiagOffs
        The width offset across the diagonal frame border (0 = middle),
        regardless of the gradient of the diagonal frame border (always
        vertical to the direction of the diagonal frame border). This value is
        not related in any way to the reference point. For details about
        relative width offsets, see description of class Style.
    @param fAngle
        Inner (left) angle between diagonal and horizontal frame border.
 */
long GetTRDiagOffset( long nDiagOffs, double fAngle );


/** Checks whether two horizontal frame borders are "connectable".

    Two borders are "connectable" in terms of this function, if both can be
    drawn with only one call of a border drawing function. This means, the two
    frame borders must have equal style and color, and none of the other
    vertical and diagonal frame borders break the lines of the two borders in
    any way (i.e. two vertical double frame borders would break the horizonal
    frame borders). Of course this function can be used for vertical frame
    borders as well.

    The follong picture shows the meaning of all passed parameters:

                      \      rTFromT      /
                        \       |       /
                   rTFromTL     |   rTFromTR
                            \   |   /
                              \ | /
    ======== rLBorder =========   ========== rRBorder =======
                              / | \
                            /   |   \
                   rBFromBL     |   rBFromBR
                        /       |       \
                      /      rBFromB      \

    @return
        True, if rLBorder and rRBorder can be drawn in one step without
        interruption at their connection point.
 */
SVX_DLLPUBLIC bool CheckFrameBorderConnectable(
    const Style&        rLBorder,       /// Style of the left frame border to connect.
    const Style&        rRBorder,       /// Style of the right frame border to connect.

    const Style&        rTFromTL,       /// Diagonal frame border from top-left to connection point.
    const Style&        rTFromT,        /// Vertical frame border from top to connection point.
    const Style&        rTFromTR,       /// Horizontal frame border from top-right to connection point.

    const Style&        rBFromBL,       /// Diagonal frame border from bottom-left to connection point.
    const Style&        rBFromB,        /// Vertical frame border from bottom to connection point.
    const Style&        rBFromBR        /// Horizontal frame border from bottom-right to connection point.
);


// Drawing functions


/** Draws a horizontal frame border, regards all connected frame styles.

    The frame style to draw is passed as parameter rBorder. The function
    calculates the adjustment in X direction for left and right end of primary
    and secondary line of the frame border (the style may present a double
    line). The line ends may differ according to the connected frame styles
    coming from top, bottom, left, right, and/or diagonal.

    Thick frame styles are always drawn centered (in width) to the passed
    reference points. The Y coordinates of both reference points must be equal
    (the line cannot be drawn slanted).

    The function preserves all settings of the passed output device.

    All parameters starting with "rL" refer to the left end of the processed
    frame border, all parameters starting with "rR" refer to the right end.
    The following part of the parameter name starting with "From" specifies
    where the frame border comes from. Example: "rLFromTR" means the frame
    border coming from top-right, connected to the left end of rBorder (and
    therefore a diagonal frame border).

    The follong picture shows the meaning of all passed parameters:

                 rLFromT      /                   \      rRFromT
                    |       /                       \       |
                    |   rLFromTR               rRFromTL     |
                    |   /                               \   |
                    | /                                   \ |
    --- rLFromL ---   ============== rBorder ==============   --- rRFromR ---
                    | \                                   / |
                    |   \                               /   |
                    |   rLFromBR               rRFromBL     |
                    |       \                       /       |
                 rLFromB      \                   /      rRFromB
 */
SVX_DLLPUBLIC drawinglayer::primitive2d::Primitive2DReference CreateBorderPrimitives(
    const Point&        rLPos,          /// Reference point for left end of the processed frame border.
    const Point&        rRPos,          /// Reference point for right end of the processed frame border.
    const Style&        rBorder,        /// Style of the processed frame border.

    const DiagStyle&    rLFromTR,       /// Diagonal frame border from top-right to left end of rBorder.
    const Style&        rLFromT,        /// Vertical frame border from top to left end of rBorder.
    const Style&        rLFromL,        /// Horizontal frame border from left to left end of rBorder.
    const Style&        rLFromB,        /// Vertical frame border from bottom to left end of rBorder.
    const DiagStyle&    rLFromBR,       /// Diagonal frame border from bottom-right to left end of rBorder.

    const DiagStyle&    rRFromTL,       /// Diagonal frame border from top-left to right end of rBorder.
    const Style&        rRFromT,        /// Vertical frame border from top to right end of rBorder.
    const Style&        rRFromR,        /// Horizontal frame border from right to right end of rBorder.
    const Style&        rRFromB,        /// Vertical frame border from bottom to right end of rBorder.
    const DiagStyle&    rRFromBL,       /// Diagonal frame border from bottom-left to right end of rBorder.

    const Color*        pForceColor,    /// If specified, overrides frame border color.
    const long          rRotationT = 9000, /// Angle of the top slanted frames in 100th of degree
    const long          rRotationB = 9000  /// Angle of the bottom slanted frames in 100th of degree
);

SVX_DLLPUBLIC drawinglayer::primitive2d::Primitive2DReference CreateBorderPrimitives(
    const Point&        rLPos,          /// Reference point for left end of the processed frame border.
    const Point&        rRPos,          /// Reference point for right end of the processed frame border.
    const Style&        rBorder,        /// Style of the processed frame border.

    const Style&        rLFromT,        /// Vertical frame border from top to left end of rBorder.
    const Style&        rLFromL,        /// Horizontal frame border from left to left end of rBorder.
    const Style&        rLFromB,        /// Vertical frame border from bottom to left end of rBorder.

    const Style&        rRFromT,        /// Vertical frame border from top to right end of rBorder.
    const Style&        rRFromR,        /// Horizontal frame border from right to right end of rBorder.
    const Style&        rRFromB,        /// Vertical frame border from bottom to right end of rBorder.

    const Color*        pForceColor,    /// If specified, overrides frame border color.
    const long          rRotationT = 9000, /// Angle of the top slanted frame in 100th of degrees
    const long          rRotationB = 9000  /// Angle of the bottom slanted frame in 100th of degrees
);

/** Draws a horizontal frame border, regards all connected frame styles.

    The frame style to draw is passed as parameter rBorder. The function
    calculates the adjustment in X direction for left and right end of primary
    and secondary line of the frame border (the style may present a double
    line). The line ends may differ according to the connected frame styles
    coming from top, bottom, left, right, and/or diagonal.

    Thick frame styles are always drawn centered (in width) to the passed
    reference points. The Y coordinates of both reference points must be equal
    (the line cannot be drawn slanted).

    The function preserves all settings of the passed output device.

    All parameters starting with "rL" refer to the left end of the processed
    frame border, all parameters starting with "rR" refer to the right end.
    The following part of the parameter name starting with "From" specifies
    where the frame border comes from. Example: "rLFromTR" means the frame
    border coming from top-right, connected to the left end of rBorder (and
    therefore a diagonal frame border).

    The follong picture shows the meaning of all passed parameters:

                 rLFromT      /                   \      rRFromT
                    |       /                       \       |
                    |   rLFromTR               rRFromTL     |
                    |   /                               \   |
                    | /                                   \ |
    --- rLFromL ---   ============== rBorder ==============   --- rRFromR ---
                    | \                                   / |
                    |   \                               /   |
                    |   rLFromBR               rRFromBL     |
                    |       \                       /       |
                 rLFromB      \                   /      rRFromB
 */
SVX_DLLPUBLIC void DrawHorFrameBorder(
    OutputDevice&       rDev,           /// The output device used to draw the frame border.

    const Point&        rLPos,          /// Reference point for left end of the processed frame border.
    const Point&        rRPos,          /// Reference point for right end of the processed frame border.
    const Style&        rBorder,        /// Style of the processed frame border.

    const DiagStyle&    rLFromTR,       /// Diagonal frame border from top-right to left end of rBorder.
    const Style&        rLFromT,        /// Vertical frame border from top to left end of rBorder.
    const Style&        rLFromL,        /// Horizontal frame border from left to left end of rBorder.
    const Style&        rLFromB,        /// Vertical frame border from bottom to left end of rBorder.
    const DiagStyle&    rLFromBR,       /// Diagonal frame border from bottom-right to left end of rBorder.

    const DiagStyle&    rRFromTL,       /// Diagonal frame border from top-left to right end of rBorder.
    const Style&        rRFromT,        /// Vertical frame border from top to right end of rBorder.
    const Style&        rRFromR,        /// Horizontal frame border from right to right end of rBorder.
    const Style&        rRFromB,        /// Vertical frame border from bottom to right end of rBorder.
    const DiagStyle&    rRFromBL,       /// Diagonal frame border from bottom-left to right end of rBorder.

    const Color*        pForceColor = nullptr /// If specified, overrides frame border color.
);


/** Draws a vertical frame border, regards all connected frame styles.

    The frame style to draw is passed as parameter rBorder. The function
    calculates the adjustment in Y direction for top and bottom end of primary
    and secondary line of the frame border (the style may present a double
    line). The line ends may differ according to the connected frame styles
    coming from left, right, top, bottom, and/or diagonal.

    Thick frame styles are always drawn centered (in width) to the passed
    reference points. The X coordinates of both reference points must be equal
    (the line cannot be drawn slanted).

    The function preserves all settings of the passed output device.

    All parameters starting with "rT" refer to the top end of the processed
    frame border, all parameters starting with "rB" refer to the bottom end.
    The following part of the parameter name starting with "From" specifies
    where the frame border comes from. Example: "rTFromBL" means the frame
    border coming from bottom-left, connected to the top end of rBorder (and
    therefore a diagonal frame border).

    The follong picture shows the meaning of all passed parameters:

                    |
                 rTFromT
                    |
                    |
    --- rTFromL ---   --- rTFromR ---
                  / # \
                /   #   \
        rTFromBL    #   rTFromBR
            /       #       \
          /         #         \
                    #
                 rBorder
                    #
          \         #         /
            \       #       /
        rBFromTL    #   rBFromTR
                \   #   /
                  \ # /
    --- rBFromL ---   --- rBFromR ---
                    |
                    |
                 rBFromB
                    |
 */
SVX_DLLPUBLIC void DrawVerFrameBorder(
    OutputDevice&       rDev,           /// The output device used to draw the frame border.

    const Point&        rTPos,          /// Reference point for top end of the processed frame border.
    const Point&        rBPos,          /// Reference point for bottom end of the processed frame border.
    const Style&        rBorder,        /// Style of the processed frame border.

    const DiagStyle&    rTFromBL,       /// Diagonal frame border from bottom-right to top end of rBorder.
    const Style&        rTFromL,        /// Horizontal frame border from left to top end of rBorder.
    const Style&        rTFromT,        /// Vertical frame border from top to top end of rBorder.
    const Style&        rTFromR,        /// Horizontal frame border from right to top end of rBorder.
    const DiagStyle&    rTFromBR,       /// Diagonal frame border from bottom-right to top end of rBorder.

    const DiagStyle&    rBFromTL,       /// Diagonal frame border from top-left to bottom end of rBorder.
    const Style&        rBFromL,        /// Horizontal frame border from left to bottom end of rBorder.
    const Style&        rBFromB,        /// Vertical frame border from bottom to bottom end of rBorder.
    const Style&        rBFromR,        /// Horizontal frame border from right to bottom end of rBorder.
    const DiagStyle&    rBFromTR,       /// Diagonal frame border from top-right to bottom end of rBorder.

    const Color*        pForceColor = nullptr /// If specified, overrides frame border color.
);


/** Draws both diagonal frame borders, regards all connected frame styles.

    One or both passed diagonal frame styles may be invisible.

    The function preserves all settings of the passed output device.
 */
SVX_DLLPUBLIC void DrawDiagFrameBorders(
    OutputDevice&       rDev,           /// The output device used to draw the frame border.

    const tools::Rectangle&    rRect,          /// Rectangle for both diagonal frame borders.
    const Style&        rTLBR,          /// Style of the processed top-left to bottom-right diagonal frame border.
    const Style&        rBLTR,          /// Style of the processed bottom-left to top-right diagonal frame border.

    const Style&        rTLFromB,       /// Vertical frame border from bottom to top-left end of rTLBR.
    const Style&        rTLFromR,       /// Horizontal frame border from right to top-left end of rTLBR.
    const Style&        rBRFromT,       /// Vertical frame border from top to bottom-right end of rTLBR.
    const Style&        rBRFromL,       /// Horizontal frame border from left to bottom-right end of rTLBR.

    const Style&        rBLFromT,       /// Vertical frame border from top to bottom-left end of rBLTR.
    const Style&        rBLFromR,       /// Horizontal frame border from right to bottom-left end of rBLTR.
    const Style&        rTRFromB,       /// Vertical frame border from bottom to top-right end of rBLTR.
    const Style&        rTRFromL,       /// Horizontal frame border from left to top-right end of rBLTR.

    const Color*        pForceColor,    /// If specified, overrides frame border color.
    bool                bDiagDblClip    /// true = Use clipping for crossing double frame borders.
);


}
}

/* Yes, I love ASCII art. :-) -DR- */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
