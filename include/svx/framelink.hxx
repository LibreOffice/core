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

#include <sal/types.h>
#include <tools/color.hxx>
#include <svx/svxdllapi.h>
#include <editeng/borderline.hxx>

namespace svx::frame {


// Enums


/** Specifies how the reference points for frame borders are used.
 */
enum class RefMode : sal_uInt8
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
class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC Style
{
private:
    Color               maColorPrim;
    Color               maColorSecn;
    Color               maColorGap;
    double              mfPrim;     /// Width of primary (single, left, or top) line.
    double              mfDist;     /// Distance between primary and secondary line.
    double              mfSecn;     /// Width of secondary (right or bottom) line.
    double              mfPatternScale; /// Scale used for line pattern spacing.
    RefMode             meRefMode;  /// Reference point handling for this frame border.
    SvxBorderLineStyle  mnType;
    bool                mbWordTableCell : 1;
    bool                mbUseGapColor : 1;

public:
    /** Constructs an invisible frame style. */
    explicit Style();
    /** Constructs a frame style with passed line widths. */
    explicit Style( double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale );
    /** Constructs a frame style with passed color and line widths. */
    explicit Style( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS, SvxBorderLineStyle nType, double fScale );
    /** Constructs a frame style from the passed SvxBorderLine struct. */
    explicit Style( const editeng::SvxBorderLine* pBorder, double fScale );

    RefMode GetRefMode() const { return meRefMode; }
    Color GetColorPrim() const { return maColorPrim; }
    Color GetColorSecn() const { return maColorSecn; }
    Color GetColorGap() const { return maColorGap; }
    bool UseGapColor() const { return mbUseGapColor; }
    double Prim() const { return mfPrim; }
    double Dist() const { return mfDist; }
    double Secn() const { return mfSecn; }
    double PatternScale() const { return mfPatternScale;}
    SvxBorderLineStyle Type() const { return mnType; }

    /// Check if this style is used - this depends on it having any width definition.
    /// As can be seen in the definition comment above, Prim() *must* be non zero to have a width
    bool IsUsed() const { return 0.0 != mfPrim; }

    /** Returns the total width of this frame style. */
    double GetWidth() const { return mfPrim + mfDist + mfSecn; }

    /** Sets the frame style to invisible state. */
    void Clear();
    /** Sets the frame style to the passed line widths. */
    void Set( double nP, double nD, double nS );
    /** Sets the frame style to the passed line widths. */
    void Set( const Color& rColorPrim, const Color& rColorSecn, const Color& rColorGap, bool bUseGapColor, double nP, double nD, double nS );
    /** Sets the frame style to the passed SvxBorderLine struct. If nullptr, resets the style */
    void Set( const editeng::SvxBorderLine* pBorder, double fScale, sal_uInt16 nMaxWidth = SAL_MAX_UINT16 );

    /** Sets a new reference point handling mode, does not modify other settings. */
    void SetRefMode( RefMode eRefMode ) { meRefMode = eRefMode; }
    /** Sets a new color, does not modify other settings. */
    void SetColorPrim( const Color& rColor ) { maColorPrim = rColor; }
    void SetColorSecn( const Color& rColor ) { maColorSecn = rColor; }
    /** Sets whether to use dotted style for single hair lines. */
    void SetType( SvxBorderLineStyle nType ) { mnType = nType; }

    /** Mirrors this style (exchanges primary and secondary), if it is a double frame style. */
    Style& MirrorSelf();

    /** Enables the Word-compatible Style comparison code. */
    void SetWordTableCell(bool bWordTableCell) { mbWordTableCell = bWordTableCell; }

    bool operator==( const Style& rOther) const;
    bool operator<( const Style& rOther) const;
};

inline bool operator>( const Style& rL, const Style& rR ) { return rR.operator<(rL); }

}


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
