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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORRST_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORRST_HXX
#include "porlay.hxx"
#include "porexp.hxx"

#define LINE_BREAK_WIDTH        150
#define SPECIAL_FONT_HEIGHT     200

class SwTextFormatInfo;

class SwTmpEndPortion : public SwLinePortion
{
public:
    explicit SwTmpEndPortion( const SwLinePortion &rPortion );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
};

class SwBreakPortion : public SwLinePortion
{
public:
    explicit SwBreakPortion( const SwLinePortion &rPortion );
    // Returns 0 if we have no usable data
    virtual SwLinePortion *Compress() override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

class SwKernPortion : public SwLinePortion
{
    short const nKern;
    bool const bBackground;
    bool const bGridKern;

public:

    // This constructor automatically appends the portion to rPortion
    // bBG indicates, that the background of the kerning portion has to
    // be painted, e.g., if the portion if positioned between to fields.
    // bGridKern indicates, that the kerning portion is used to provide
    // additional space in grid mode.
    SwKernPortion( SwLinePortion &rPortion, short nKrn,
                   bool bBG = false, bool bGridKern = false );

    // This constructor only sets the height and ascent to the values
    // of rPortion. It is only used for kerning portions for grid mode
    explicit SwKernPortion( const SwLinePortion &rPortion );

    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
};

class SwArrowPortion : public SwLinePortion
{
    Point aPos;
    bool const bLeft;
public:
    explicit SwArrowPortion( const SwLinePortion &rPortion );
    explicit SwArrowPortion( const SwTextPaintInfo &rInf );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual SwLinePortion *Compress() override;
    bool IsLeft() const { return bLeft; }
    const Point& GetPos() const { return aPos; }
};

// The characters which are forbidden at the start of a line like the dot and
// other punctuation marks are allowed to display in the margin of the page
// by a user option.
// The SwHangingPortion is the corresponding textportion to do that.
class SwHangingPortion : public SwTextPortion
{
    sal_uInt16 const nInnerWidth;
public:
    explicit SwHangingPortion( SwPosSize aSize ) : nInnerWidth( aSize.Width() )
    {
        SetWhichPor( PortionType::Hanging );
        SetLen(TextFrameIndex(1));
        Height( aSize.Height() );
    }

    sal_uInt16 GetInnerWidth() const { return nInnerWidth; }
};

// Used to hide text
class SwHiddenTextPortion : public SwLinePortion
{
public:
    explicit SwHiddenTextPortion(TextFrameIndex const nLen)
    {
        SetWhichPor( PortionType::HiddenText );  SetLen( nLen );
    }

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
};

class SwControlCharPortion : public SwLinePortion
{

private:
    mutable sal_uInt16 mnViewWidth;            // used to cache a calculated value
    mutable sal_uInt16 mnHalfCharWidth;        // used to cache a calculated value
    sal_Unicode const mcChar;

public:

    explicit SwControlCharPortion( sal_Unicode cChar )
        : mnViewWidth( 0 ), mnHalfCharWidth( 0 ), mcChar( cChar )
    {
        SetWhichPor( PortionType::ControlChar ); SetLen( TextFrameIndex(1) );
    }

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo& rInf ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
