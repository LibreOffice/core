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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORLIN_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORLIN_HXX

#include "possiz.hxx"
#include <txttypes.hxx>
#include <TextFrameIndex.hxx>

#ifdef DBG_UTIL
#include <libxml/xmlwriter.h>
#endif

class SwTextSizeInfo;
class SwTextPaintInfo;
class SwTextFormatInfo;
class SwPortionHandler;

// Portion groups
#define PORGRP_TXT      0x8000
#define PORGRP_EXP      0x4000
#define PORGRP_FLD      0x2000
#define PORGRP_HYPH     0x1000
#define PORGRP_NUMBER   0x0800
#define PORGRP_GLUE     0x0400
#define PORGRP_FIX      0x0200
#define PORGRP_TAB      0x0100
// Small special groups
#define PORGRP_FIXMARG  0x0040
//#define PORGRP_?  0x0020
#define PORGRP_TABNOTLFT 0x0010
#define PORGRP_TOXREF   0x0008

/// Base class for anything that can be part of a line in the Writer layout.
/// Typically owned by SwLineLayout.
class SwLinePortion: public SwPosSize
{
protected:
    // Here we have areas with different attributes
    SwLinePortion *mpNextPortion;
    // Count of chars and spaces on the line
    TextFrameIndex nLineLength;
    sal_uInt16 nAscent;      // Maximum ascender

    SwLinePortion();
private:
    PortionType nWhichPor;       // Who's who?
    bool m_bJoinBorderWithPrev;
    bool m_bJoinBorderWithNext;

    void Truncate_();

public:
    explicit inline SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Access methods
    SwLinePortion *GetNextPortion() const { return mpNextPortion; }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    TextFrameIndex GetLen() const { return nLineLength; }
    void SetLen(TextFrameIndex const nLen) { nLineLength = nLen; }
    void SetNextPortion( SwLinePortion *pNew ){ mpNextPortion = pNew; }
    sal_uInt16 &GetAscent() { return nAscent; }
    sal_uInt16 GetAscent() const { return nAscent; }
    void SetAscent( const sal_uInt16 nNewAsc ) { nAscent = nNewAsc; }
    void  PrtWidth( sal_uInt16 nNewWidth ) { Width( nNewWidth ); }
    sal_uInt16 PrtWidth() const { return Width(); }
    void AddPrtWidth( const sal_uInt16 nNew ) { Width( Width() + nNew ); }
    void SubPrtWidth( const sal_uInt16 nNew ) { Width( Width() - nNew ); }

    // Insert methods
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
            SwLinePortion *Cut( SwLinePortion *pVictim );
    inline  void Truncate();

    // Returns 0, if there's no payload
    virtual SwLinePortion *Compress();

    void SetWhichPor( const PortionType nNew )    { nWhichPor = nNew; }
    PortionType GetWhichPor( ) const        { return nWhichPor; }

// Group queries
    bool InTextGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_TXT) != 0; }
    bool InGlueGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_GLUE) != 0; }
    bool InTabGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_TAB) != 0; }
    bool InHyphGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_HYPH) != 0; }
    bool InNumberGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_NUMBER) != 0; }
    bool InFixGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_FIX) != 0; }
    bool InFieldGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_FLD) != 0; }
    bool InToxRefGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_TOXREF) != 0; }
    bool InToxRefOrFieldGrp() const { return (sal_uInt16(nWhichPor) & ( PORGRP_FLD | PORGRP_TOXREF )) != 0; }
    bool InExpGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_EXP) != 0; }
    bool InFixMargGrp() const { return (sal_uInt16(nWhichPor) & PORGRP_FIXMARG) != 0; }
    bool InSpaceGrp() const { return InTextGrp() || IsMultiPortion(); }
// Individual queries
    bool IsGrfNumPortion() const { return nWhichPor == PortionType::GrfNum; }
    bool IsFlyCntPortion() const { return nWhichPor == PortionType::FlyCnt; }
    bool IsBlankPortion() const { return nWhichPor == PortionType::Blank; }
    bool IsBreakPortion() const { return nWhichPor == PortionType::Break; }
    bool IsErgoSumPortion() const { return nWhichPor == PortionType::ErgoSum; }
    bool IsQuoVadisPortion() const { return nWhichPor == PortionType::QuoVadis; }
    bool IsTabLeftPortion() const { return nWhichPor == PortionType::TabLeft; }
    bool IsTabRightPortion() const { return nWhichPor == PortionType::TabRight; }
    bool IsFootnoteNumPortion() const { return nWhichPor == PortionType::FootnoteNum; }
    bool IsFootnotePortion() const { return nWhichPor == PortionType::Footnote; }
    bool IsDropPortion() const { return nWhichPor == PortionType::Drop; }
    bool IsLayPortion() const { return nWhichPor == PortionType::Lay; }
    bool IsParaPortion() const { return nWhichPor == PortionType::Para; }
    bool IsMarginPortion() const { return nWhichPor == PortionType::Margin; }
    bool IsFlyPortion() const { return nWhichPor == PortionType::Fly; }
    bool IsHolePortion() const { return nWhichPor == PortionType::Hole; }
    bool IsSoftHyphPortion() const { return nWhichPor == PortionType::SoftHyphen; }
    bool IsPostItsPortion() const { return nWhichPor == PortionType::PostIts; }
    bool IsCombinedPortion() const { return nWhichPor == PortionType::Combined; }
    bool IsTextPortion() const { return nWhichPor == PortionType::Text; }
    bool IsHangingPortion() const { return nWhichPor == PortionType::Hanging; }
    bool IsKernPortion() const { return nWhichPor == PortionType::Kern; }
    bool IsArrowPortion() const { return nWhichPor == PortionType::Arrow; }
    bool IsMultiPortion() const { return nWhichPor == PortionType::Multi; }
    bool IsNumberPortion() const { return nWhichPor == PortionType::Number; } // #i23726#
    bool IsControlCharPortion() const { return nWhichPor == PortionType::ControlChar; }

    // Positioning
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    /// the parameter is actually SwTwips apparently?
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const;
    void CalcTextSize( const SwTextSizeInfo &rInfo );

    // Output
    virtual void Paint( const SwTextPaintInfo &rInf ) const = 0;
    void PrePaint( const SwTextPaintInfo &rInf, const SwLinePortion *pLast ) const;

    virtual bool Format( SwTextFormatInfo &rInf );
    // Is called for the line's last portion
    virtual void FormatEOL( SwTextFormatInfo &rInf );
            void Move( SwTextPaintInfo &rInf );

    // For SwTextSlot
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const;

    // For SwFieldPortion, SwSoftHyphPortion
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const;

    // for text- and multi-portions
    virtual long CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    bool GetJoinBorderWithPrev() const { return m_bJoinBorderWithPrev; }
    bool GetJoinBorderWithNext() const { return m_bJoinBorderWithNext; }
    void SetJoinBorderWithPrev( const bool bJoinPrev ) { m_bJoinBorderWithPrev = bJoinPrev; }
    void SetJoinBorderWithNext( const bool bJoinNext ) { m_bJoinBorderWithNext = bJoinNext; }
};

inline SwLinePortion &SwLinePortion::operator=(const SwLinePortion &rPortion)
{
    *static_cast<SwPosSize*>(this) = rPortion;
    nLineLength = rPortion.nLineLength;
    nAscent = rPortion.nAscent;
    nWhichPor = rPortion.nWhichPor;
    m_bJoinBorderWithPrev = rPortion.m_bJoinBorderWithPrev;
    m_bJoinBorderWithNext = rPortion.m_bJoinBorderWithNext;
    return *this;
}

inline SwLinePortion::SwLinePortion(const SwLinePortion &rPortion) :
    SwPosSize( rPortion ),
    mpNextPortion( nullptr ),
    nLineLength( rPortion.nLineLength ),
    nAscent( rPortion.nAscent ),
    nWhichPor( rPortion.nWhichPor ),
    m_bJoinBorderWithPrev( rPortion.m_bJoinBorderWithPrev ),
    m_bJoinBorderWithNext( rPortion.m_bJoinBorderWithNext )
{
}

inline void SwLinePortion::Truncate()
{
    if ( mpNextPortion )
        Truncate_();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
