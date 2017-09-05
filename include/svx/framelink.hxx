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
            mnType(SvxBorderLineStyle::SOLID)
        {}
    };

    /// the impl class holding the data
    std::shared_ptr< implStyle >        maImplStyle;

    /// pointer to Cell using this style. Not member of the
    /// impl class since multiple Cells may use the same style
    const Cell*                         mpUsingCell;

    /// call to set maImplStyle on demand
    void implEnsureImplStyle();

    /// need information which cell this style info comes from due to needed
    /// rotation info (which is in the cell). Rotation depends on the cell.
    friend class Cell;
    void SetUsingCell(const Cell* pCell) { mpUsingCell = pCell; }

public:
    /** Constructs an invisible frame style. */
    explicit Style();
    /** Constructs a frame style with passed line widths. */
    explicit Style( double nP, double nD, double nS, SvxBorderLineStyle nType );
    /** Constructs a frame style with passed color and line widths. */
    explicit Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType );
    /** Constructs a frame style from the passed SvxBorderLine struct. */
    explicit Style( const editeng::SvxBorderLine* pBorder, double fScale = 1.0 );

    RefMode GetRefMode() const { if(!maImplStyle) return RefMode::Centered; return maImplStyle->meRefMode; }
    const Color GetColorPrim() const { if(!maImplStyle) return Color(); return maImplStyle->maColorPrim; }
    const Color GetColorSecn() const { if(!maImplStyle) return Color(); return maImplStyle->maColorSecn; }
    const Color GetColorGap() const { if(!maImplStyle) return Color(); return maImplStyle->maColorGap; }
    bool UseGapColor() const { if(!maImplStyle) return false; return maImplStyle->mbUseGapColor; }
    double Prim() const { if(!maImplStyle) return 0.0; return maImplStyle->mfPrim; }
    double Dist() const { if(!maImplStyle) return 0.0; return maImplStyle->mfDist; }
    double Secn() const { if(!maImplStyle) return 0.0; return maImplStyle->mfSecn; }
    double PatternScale() const { if(!maImplStyle) return 1.0; return maImplStyle->mfPatternScale;}
    void SetPatternScale( double fScale );
    SvxBorderLineStyle Type() const { if(!maImplStyle) return SvxBorderLineStyle::SOLID; return maImplStyle->mnType; }

    /// Check if this style is used - this depends on it having any width definition.
    /// As can be seen in the definition comment above, Prim() *must* be non zero to have a width
    bool IsUsed() const { if(!maImplStyle) return false; return 0.0 != maImplStyle->mfPrim; }

    /** Returns the total width of this frame style. */
    double GetWidth() const { if(!maImplStyle) return 0.0; implStyle* pTarget = maImplStyle.get(); return pTarget->mfPrim + pTarget->mfDist + pTarget->mfSecn; }

    /** Sets the frame style to invisible state. */
    void Clear();
    /** Sets the frame style to the passed line widths. */
    void Set( double nP, double nD, double nS );
    /** Sets the frame style to the passed line widths. */
    void Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS );
    /** Sets the frame style to the passed SvxBorderLine struct. If nullptr, resets the style */
    void Set( const editeng::SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth = SAL_MAX_UINT16 );

    /** Sets a new reference point handling mode, does not modify other settings. */
    void SetRefMode( RefMode eRefMode );
    /** Sets a new color, does not modify other settings. */
    void SetColorPrim( const Color& rColor );
    void SetColorSecn( const Color& rColor );
    /** Sets whether to use dotted style for single hair lines. */
    void SetType( SvxBorderLineStyle nType );

    /** Mirrors this style (exchanges primary and secondary), if it is a double frame style. */
    Style& MirrorSelf();

    /** return the Cell using this style (if set) */
    const Cell* GetUsingCell() const { return mpUsingCell; }

    bool operator==( const Style& rOther) const;
    bool operator<( const Style& rOther) const;
};

inline bool operator>( const Style& rL, const Style& rR ) { return rR.operator<(rL); }

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

class SAL_WARN_UNUSED SVX_DLLPUBLIC StyleVectorCombination
{
private:
    const Style&                mrStyle;
    const basegfx::B2DVector&   mrB2DVector;

public:
    StyleVectorCombination(const Style& rStyle, const basegfx::B2DVector& rB2DVector) :
        mrStyle(rStyle),
        mrB2DVector(rB2DVector)
    {}

    const Style& getStyle() const { return mrStyle; }
    const basegfx::B2DVector& getB2DVector() const { return mrB2DVector; }
};

typedef std::vector< StyleVectorCombination > StyleVectorTable;

SVX_DLLPUBLIC void CreateBorderPrimitives(
    drawinglayer::primitive2d::Primitive2DContainer&    rTarget,        /// target for created primitives
    const basegfx::B2DPoint&    rOrigin,                /// start point of borderline
    const basegfx::B2DVector&   rX,                     /// X-Axis of borderline with length
    const Style&                rBorder,                /// Style of borderline
    const StyleVectorTable&     rStartStyleVectorTable, /// Styles and vectors (pointing away) at borderline start, ccw
    const StyleVectorTable&     rEndStyleVectorTable,   /// Styles and vectors (pointing away) at borderline end, cw
    const Color*                pForceColor             /// If specified, overrides frame border color.
);

}
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
