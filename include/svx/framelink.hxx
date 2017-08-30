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
namespace svx { namespace frame { class Cell; }}

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
private:
    class implStyle
    {
    private:
        friend class Style;

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
        const Cell*         mpUsingCell;

    public:
        /** Constructs an invisible frame style. */
        explicit implStyle() :
            maColorPrim(),
            maColorSecn(),
            maColorGap(),
            mbUseGapColor(false),
            meRefMode(RefMode::Centered),
            mfPrim(0.0),
            mfDist(0.0),
            mfSecn(0.0),
            mfPatternScale(1.0),
            mnType(SvxBorderLineStyle::SOLID),
            mpUsingCell(nullptr)
        {}
    };

    std::shared_ptr< implStyle >        maImplStyle;

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

    RefMode      GetRefMode() const { return maImplStyle->meRefMode; }
    const Color& GetColorPrim() const { return maImplStyle->maColorPrim; }
    const Color& GetColorSecn() const { return maImplStyle->maColorSecn; }
    const Color& GetColorGap() const { return maImplStyle->maColorGap; }
    bool         UseGapColor() const { return maImplStyle->mbUseGapColor; }
    double       Prim() const { return maImplStyle->mfPrim; }
    double       Dist() const { return maImplStyle->mfDist; }
    double       Secn() const { return maImplStyle->mfSecn; }
    double PatternScale() const { return maImplStyle->mfPatternScale;}
    void SetPatternScale( double fScale ) { maImplStyle->mfPatternScale = fScale; }
    SvxBorderLineStyle Type() const { return maImplStyle->mnType; }

    /** Returns the total width of this frame style. */
    double       GetWidth() const;

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
    void         SetRefMode( RefMode eRefMode ) { maImplStyle->meRefMode = eRefMode; }
    /** Sets a new color, does not modify other settings. */
    void         SetColorPrim( const Color& rColor ) { maImplStyle->maColorPrim = rColor; }
    void         SetColorSecn( const Color& rColor ) { maImplStyle->maColorSecn = rColor; }
    /** Sets whether to use dotted style for single hair lines. */
    void         SetType( SvxBorderLineStyle nType ) { maImplStyle->mnType = nType; }

    /** Mirrors this style (exchanges primary and secondary), if it is a double frame style. */
    Style&              MirrorSelf();

    /** return the Cell using this style (if set) */
    const Cell* GetUsingCell() const;

private:
    /// need information which cell this style info comes from due to needed
    /// rotation info (which is in the cell). Rotation depends on the cell.
    friend class Cell;
    void SetUsingCell(const Cell* pCell);
};

bool operator==( const Style& rL, const Style& rR );
SVX_DLLPUBLIC bool operator<( const Style& rL, const Style& rR );

inline bool operator>( const Style& rL, const Style& rR ) { return rR < rL; }

// Various helper functions

/** Checks whether two horizontal frame borders are "connectable".

    Two borders are "connectable" in terms of this function, if both can be
    drawn with only one call of a border drawing function. This means, the two
    frame borders must have equal style and color, and none of the other
    vertical and diagonal frame borders break the lines of the two borders in
    any way (i.e. two vertical double frame borders would break the horizonal
    frame borders). Of course this function can be used for vertical frame
    borders as well.

    The following picture shows the meaning of all passed parameters:

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

    The following picture shows the meaning of all passed parameters:

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
SVX_DLLPUBLIC void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer&    rTarget,        /// target for created primitives

    const basegfx::B2DPoint&    rOrigin,    /// start point of borderline
    const basegfx::B2DVector&   rX,         /// X-Axis with length
    const basegfx::B2DVector&   rY,         /// Y-Axis for perpendicular, normalized. Does *not* need to be perpendicular, but may express a rotation

    const Style&        rBorder,        /// Style of the processed frame border.

    const Style&        rLFromTR,       /// Diagonal frame border from top-right to left end of rBorder.
    const Style&        rLFromT,        /// Vertical frame border from top to left end of rBorder.
    const Style&        rLFromL,        /// Horizontal frame border from left to left end of rBorder.
    const Style&        rLFromB,        /// Vertical frame border from bottom to left end of rBorder.
    const Style&        rLFromBR,       /// Diagonal frame border from bottom-right to left end of rBorder.

    const Style&        rRFromTL,       /// Diagonal frame border from top-left to right end of rBorder.
    const Style&        rRFromT,        /// Vertical frame border from top to right end of rBorder.
    const Style&        rRFromR,        /// Horizontal frame border from right to right end of rBorder.
    const Style&        rRFromB,        /// Vertical frame border from bottom to right end of rBorder.
    const Style&        rRFromBL,       /// Diagonal frame border from bottom-left to right end of rBorder.

    const Color*        pForceColor     /// If specified, overrides frame border color.
);

SVX_DLLPUBLIC void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer&    rTarget,        /// target for created primitives

    const basegfx::B2DPoint&    rOrigin,    /// start point of borderline
    const basegfx::B2DVector&   rX,         /// X-Axis with length
    const basegfx::B2DVector&   rY,         /// Y-Axis for perpendicular, normalized. Does *not* need to be perpendicular, but may express a rotation

    const Style&        rBorder,        /// Style of the processed frame border.

    const Style&        rLFromT,        /// Vertical frame border from top to left end of rBorder.
    const Style&        rLFromL,        /// Horizontal frame border from left to left end of rBorder.
    const Style&        rLFromB,        /// Vertical frame border from bottom to left end of rBorder.

    const Style&        rRFromT,        /// Vertical frame border from top to right end of rBorder.
    const Style&        rRFromR,        /// Horizontal frame border from right to right end of rBorder.
    const Style&        rRFromB,        /// Vertical frame border from bottom to right end of rBorder.

    const Color*        pForceColor     /// If specified, overrides frame border color.
);

/** Draws both diagonal frame borders, regards all connected frame styles.

One or both passed diagonal frame styles may be invisible.

The function preserves all settings of the passed output device.
*/
SVX_DLLPUBLIC void CreateDiagFrameBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer&    rTarget,        /// target for created primitives

    const basegfx::B2DPoint&                            rOrigin,        /// Origin of the coordinate system spawning the cell
    const basegfx::B2DVector&                           rXAxis,         /// X-Axis of the coordinate system spawning the cell
    const basegfx::B2DVector&                           rYAxis,         /// Y-Axis of the coordinate system spawning the cell

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

    const Color*        pForceColor     /// If specified, overrides frame border color.
);


}
}

/* Yes, I love ASCII art. :-) -DR- */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
