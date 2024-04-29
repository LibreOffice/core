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
#pragma once

#include <libxml/xmlwriter.h>

#include "possiz.hxx"
#include <txttypes.hxx>
#include <TextFrameIndex.hxx>
#include <rtl/ustring.hxx>
#include <swporlayoutcontext.hxx>

class SwTextSizeInfo;
class SwTextPaintInfo;
class SwTextFormatInfo;
class SwPortionHandler;

/// Base class for anything that can be part of a line in the Writer layout.
/// Typically owned by SwLineLayout.
class SAL_DLLPUBLIC_RTTI SwLinePortion: public SwPosSize
{
protected:
    // Here we have areas with different attributes
    SwLinePortion *mpNextPortion;
    // Count of chars and spaces on the line
    TextFrameIndex mnLineLength;
    SwTwips mnAscent;      // Maximum ascender
    SwTwips mnHangingBaseline;

    SwLinePortion();
private:
    PortionType mnWhichPor;       // Who's who?
    bool m_bJoinBorderWithPrev;
    bool m_bJoinBorderWithNext;
    SwTwips m_nExtraBlankWidth = 0;    // width of spaces after the break
    std::optional<SwLinePortionLayoutContext> m_nLayoutContext;

    void Truncate_();

public:
    explicit inline SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Access methods
    SwLinePortion *GetNextPortion() const { return mpNextPortion; }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    TextFrameIndex GetLen() const { return mnLineLength; }
    void SetLen(TextFrameIndex const nLen) { mnLineLength = nLen; }
    void SetNextPortion( SwLinePortion *pNew ){ mpNextPortion = pNew; }
    SwTwips &GetAscent() { return mnAscent; }
    SwTwips GetAscent() const { return mnAscent; }
    void SetAscent( const SwTwips nNewAsc ) { mnAscent = nNewAsc; }
    void  PrtWidth( SwTwips nNewWidth ) { Width( nNewWidth ); }
    SwTwips PrtWidth() const { return Width(); }
    void AddPrtWidth( const SwTwips nNew ) { Width( Width() + nNew ); }
    void SubPrtWidth( const SwTwips nNew ) { Width( Width() - nNew ); }
    SwTwips ExtraBlankWidth() const { return m_nExtraBlankWidth; }
    void ExtraBlankWidth(const SwTwips nNew) { m_nExtraBlankWidth = nNew; }
    SwTwips GetHangingBaseline() const { return mnHangingBaseline; }
    void SetHangingBaseline( const SwTwips nNewBaseline ) { mnHangingBaseline = nNewBaseline; }
    std::optional<SwLinePortionLayoutContext> GetLayoutContext() const { return m_nLayoutContext; }
    void SetLayoutContext(std::optional<SwLinePortionLayoutContext> nNew)
    {
        m_nLayoutContext = nNew;
    }

    // Insert methods
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
            SwLinePortion *Cut( SwLinePortion *pVictim );
    inline  void Truncate();

    // Returns 0, if there's no payload
    virtual SwLinePortion *Compress();

    void SetWhichPor( const PortionType nNew )    { mnWhichPor = nNew; }
    PortionType GetWhichPor( ) const        { return mnWhichPor; }

// Group queries
    bool InTextGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_TXT) != 0; }
    bool InGlueGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_GLUE) != 0; }
    bool InTabGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_TAB) != 0; }
    bool InHyphGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_HYPH) != 0; }
    bool InNumberGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_NUMBER) != 0; }
    bool InFixGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_FIX) != 0; }
    bool InFieldGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_FLD) != 0; }
    bool InToxRefGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_TOXREF) != 0; }
    bool InToxRefOrFieldGrp() const { return (sal_uInt16(mnWhichPor) & ( PORGRP_FLD | PORGRP_TOXREF )) != 0; }
    bool InExpGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_EXP) != 0; }
    bool InFixMargGrp() const { return (sal_uInt16(mnWhichPor) & PORGRP_FIXMARG) != 0; }
    bool InSpaceGrp() const { return InTextGrp() || IsMultiPortion(); }
// Individual queries
    bool IsGrfNumPortion() const { return mnWhichPor == PortionType::GrfNum; }
    bool IsFlyCntPortion() const { return mnWhichPor == PortionType::FlyCnt; }
    bool IsBlankPortion() const { return mnWhichPor == PortionType::Blank; }
    bool IsBreakPortion() const { return mnWhichPor == PortionType::Break; }
    bool IsErgoSumPortion() const { return mnWhichPor == PortionType::ErgoSum; }
    bool IsQuoVadisPortion() const { return mnWhichPor == PortionType::QuoVadis; }
    bool IsTabLeftPortion() const { return mnWhichPor == PortionType::TabLeft; }
    bool IsTabRightPortion() const { return mnWhichPor == PortionType::TabRight; }
    bool IsTabCenterPortion() const { return mnWhichPor == PortionType::TabCenter; }
    bool IsTabDecimalPortion() const { return mnWhichPor == PortionType::TabDecimal; }
    bool IsFootnoteNumPortion() const { return mnWhichPor == PortionType::FootnoteNum; }
    bool IsFootnotePortion() const { return mnWhichPor == PortionType::Footnote; }
    bool IsDropPortion() const { return mnWhichPor == PortionType::Drop; }
    bool IsLayPortion() const { return mnWhichPor == PortionType::Lay; }
    bool IsParaPortion() const { return mnWhichPor == PortionType::Para; }
    bool IsMarginPortion() const { return mnWhichPor == PortionType::Margin; }
    bool IsFlyPortion() const { return mnWhichPor == PortionType::Fly; }
    bool IsHolePortion() const { return mnWhichPor == PortionType::Hole; }
    bool IsSoftHyphPortion() const { return mnWhichPor == PortionType::SoftHyphen; }
    bool IsPostItsPortion() const { return mnWhichPor == PortionType::PostIts; }
    bool IsCombinedPortion() const { return mnWhichPor == PortionType::Combined; }
    bool IsTextPortion() const { return mnWhichPor == PortionType::Text; }
    bool IsHangingPortion() const { return mnWhichPor == PortionType::Hanging; }
    bool IsKernPortion() const { return mnWhichPor == PortionType::Kern; }
    bool IsArrowPortion() const { return mnWhichPor == PortionType::Arrow; }
    bool IsMultiPortion() const { return mnWhichPor == PortionType::Multi; }
    bool IsNumberPortion() const { return mnWhichPor == PortionType::Number; } // #i23726#
    bool IsControlCharPortion() const { return mnWhichPor == PortionType::ControlChar || mnWhichPor == PortionType::Bookmark; }

    // Positioning
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    /// the parameter is actually SwTwips apparently?
    virtual TextFrameIndex GetModelPositionForViewPoint(sal_uInt16 nOfst) const;
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const;
    void CalcTextSize( const SwTextSizeInfo &rInfo );

    // Output
    virtual void Paint( const SwTextPaintInfo &rInf ) const = 0;
    void PrePaint( const SwTextPaintInfo &rInf, const SwLinePortion *pLast ) const;

    virtual bool Format( SwTextFormatInfo &rInf );
    // Is called for the line's last portion
    virtual void FormatEOL( SwTextFormatInfo &rInf );
            void Move(SwTextPaintInfo & rInf) const;

    // For SwTextSlot
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const;

    // For SwFieldPortion, SwSoftHyphPortion
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const;

    // for text- and multi-portions
    virtual SwTwips CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    bool GetJoinBorderWithPrev() const { return m_bJoinBorderWithPrev; }
    bool GetJoinBorderWithNext() const { return m_bJoinBorderWithNext; }
    void SetJoinBorderWithPrev( const bool bJoinPrev ) { m_bJoinBorderWithPrev = bJoinPrev; }
    void SetJoinBorderWithNext( const bool bJoinNext ) { m_bJoinBorderWithNext = bJoinNext; }

    virtual void dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText,
                           TextFrameIndex& rOffset) const;
    void dumpAsXmlAttributes(xmlTextWriterPtr writer, std::u16string_view rText,
                             TextFrameIndex nOffset) const;
};

inline SwLinePortion &SwLinePortion::operator=(const SwLinePortion &rPortion)
{
    *static_cast<SwPosSize*>(this) = rPortion;
    mnLineLength = rPortion.mnLineLength;
    mnAscent = rPortion.mnAscent;
    mnHangingBaseline = rPortion.mnHangingBaseline;
    mnWhichPor = rPortion.mnWhichPor;
    m_bJoinBorderWithPrev = rPortion.m_bJoinBorderWithPrev;
    m_bJoinBorderWithNext = rPortion.m_bJoinBorderWithNext;
    m_nExtraBlankWidth = rPortion.m_nExtraBlankWidth;
    m_nLayoutContext = rPortion.m_nLayoutContext;
    return *this;
}

inline SwLinePortion::SwLinePortion(const SwLinePortion &rPortion) :
    SwPosSize( rPortion ),
    mpNextPortion( nullptr ),
    mnLineLength( rPortion.mnLineLength ),
    mnAscent( rPortion.mnAscent ),
    mnHangingBaseline( rPortion.mnHangingBaseline ),
    mnWhichPor( rPortion.mnWhichPor ),
    m_bJoinBorderWithPrev( rPortion.m_bJoinBorderWithPrev ),
    m_bJoinBorderWithNext( rPortion.m_bJoinBorderWithNext ),
    m_nExtraBlankWidth(rPortion.m_nExtraBlankWidth),
    m_nLayoutContext(rPortion.m_nLayoutContext)
{
}

inline void SwLinePortion::Truncate()
{
    if ( mpNextPortion )
        Truncate_();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
