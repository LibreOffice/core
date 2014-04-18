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

#ifdef DBG_UTIL
#include <libxml/xmlwriter.h>
#endif

class SwTxtSizeInfo;
class SwTxtPaintInfo;
class SwTxtFormatInfo;
class SwPortionHandler;

// The portions output operators are virtual methods of the portion.
#ifdef DBG_UTIL
#define OUTPUT_OPERATOR  virtual SvStream & operator<<( SvStream & aOs ) const;
#define OUTPUT_OPERATOR_OVERRIDE virtual SvStream & operator<<( SvStream & aOs ) const SAL_OVERRIDE;
#else
#define OUTPUT_OPERATOR
#define OUTPUT_OPERATOR_OVERRIDE
#endif

// Portion groups
#define PORGRP_TXT      0x8000
#define PORGRP_EXP      0x4000
#define PORGRP_FLD      0x2000
#define PORGRP_HYPH     0x1000
#define PORGRP_NUMBER   0x0800
#define PORGRP_GLUE     0x0400
#define PORGRP_FIX      0x0200
#define PORGRP_TAB      0x0100
#define PORGRP_NOTRECY  0x0080
// Small special groups
#define PORGRP_FIXMARG  0x0040
//#define PORGRP_?  0x0020
#define PORGRP_TABNOTLFT 0x0010
#define PORGRP_TOXREF   0x0008

/// Base class for anything that can be part of a line in the Writer layout.
class SwLinePortion: public SwPosSize
{
protected:
    // Here we have areas with different attributes
    SwLinePortion *pPortion;
    // Count of chars and spaces on the line
    sal_Int32 nLineLength;
    KSHORT nAscent;      // Maximum ascender

    SwLinePortion();
private:
    MSHORT nWhichPor;       // Who's who?
    bool m_bJoinBorderWithPrev;
    bool m_bJoinBorderWithNext;

    void _Truncate();

public:
    inline          SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Access methods
    inline SwLinePortion *GetPortion() const { return( pPortion ); }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    inline bool operator==( const SwLinePortion &rPortion ) const;
    inline sal_Int32 GetLen() const { return nLineLength; }
    inline void SetLen( const sal_Int32 nLen ) { nLineLength = nLen; }
    inline void SetPortion( SwLinePortion *pNew ){ pPortion = pNew; }
    inline KSHORT &GetAscent() { return nAscent; }
    inline KSHORT GetAscent() const { return nAscent; }
    inline void SetAscent( const KSHORT nNewAsc ) { nAscent = nNewAsc; }
    inline void  PrtWidth( KSHORT nNewWidth ) { Width( nNewWidth ); }
    inline KSHORT PrtWidth() const { return Width(); }
    inline void AddPrtWidth( const KSHORT nNew ) { Width( Width() + nNew ); }
    inline void SubPrtWidth( const KSHORT nNew ) { Width( Width() - nNew ); }

    inline const SwPosSize &PrtSize() const { return *this; }

    // Insert methods
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
            SwLinePortion *Cut( SwLinePortion *pVictim );
    inline  void Truncate();

    // Returns 0, if there's no payload
    virtual SwLinePortion *Compress();

    inline void SetWhichPor( const MSHORT nNew )    { nWhichPor = nNew; }
    inline MSHORT GetWhichPor( ) const        { return nWhichPor; }

// Group queries
    inline bool InTxtGrp() const { return nWhichPor & PORGRP_TXT; }
    inline bool InGlueGrp() const { return nWhichPor & PORGRP_GLUE; }
    inline bool InTabGrp() const { return nWhichPor & PORGRP_TAB; }
    inline bool InHyphGrp() const { return nWhichPor & PORGRP_HYPH; }
    inline bool InNumberGrp() const { return nWhichPor & PORGRP_NUMBER; }
    inline bool InFixGrp() const { return nWhichPor & PORGRP_FIX; }
    inline bool InFldGrp() const { return nWhichPor & PORGRP_FLD; }
    inline bool InToxRefGrp() const { return nWhichPor & PORGRP_TOXREF; }
    inline bool InToxRefOrFldGrp() const { return nWhichPor & ( PORGRP_FLD | PORGRP_TOXREF ); }
    inline bool InExpGrp() const { return nWhichPor & PORGRP_EXP; }
    inline bool InTabnLftGrp() const { return nWhichPor & PORGRP_TABNOTLFT; }
    inline bool InFixMargGrp() const { return nWhichPor & PORGRP_FIXMARG; }
    inline bool InSpaceGrp() const { return InTxtGrp() || IsMultiPortion(); }
// Individual queries
    inline bool IsGrfNumPortion() const { return nWhichPor == POR_GRFNUM; }
    inline bool IsFlyCntPortion() const { return nWhichPor == POR_FLYCNT; }
    inline bool IsBlankPortion() const { return nWhichPor == POR_BLANK; }
    inline bool IsBreakPortion() const { return nWhichPor == POR_BRK; }
    inline bool IsErgoSumPortion() const { return nWhichPor == POR_ERGOSUM; }
    inline bool IsQuoVadisPortion() const { return nWhichPor == POR_QUOVADIS; }
    inline bool IsTabCntPortion() const { return nWhichPor == POR_TABCENTER; }
    inline bool IsTabDecimalPortion() const { return nWhichPor == POR_TABDECIMAL; }
    inline bool IsTabLeftPortion() const { return nWhichPor == POR_TABLEFT; }
    inline bool IsFtnNumPortion() const { return nWhichPor == POR_FTNNUM; }
    inline bool IsFtnPortion() const { return nWhichPor == POR_FTN; }
    inline bool IsTmpEndPortion() const { return nWhichPor == POR_TMPEND; }
    inline bool IsDropPortion() const { return nWhichPor == POR_DROP; }
    inline bool IsLayPortion() const { return nWhichPor == POR_LAY; }
    inline bool IsParaPortion() const { return nWhichPor == POR_PARA; }
    inline bool IsMarginPortion() const { return nWhichPor == POR_MARGIN; }
    inline bool IsFlyPortion() const { return nWhichPor == POR_FLY; }
    inline bool IsHolePortion() const { return nWhichPor == POR_HOLE; }
    inline bool IsSoftHyphPortion() const { return nWhichPor == POR_SOFTHYPH; }
    inline bool IsPostItsPortion() const { return nWhichPor == POR_POSTITS; }
    inline bool IsCombinedPortion() const { return nWhichPor == POR_COMBINED; }
    inline bool IsTextPortion() const { return nWhichPor == POR_TXT; }
    inline bool IsURLPortion() const { return nWhichPor == POR_URL; }
    inline bool IsHangingPortion() const { return nWhichPor == POR_HNG; }
    inline bool IsKernPortion() const { return nWhichPor == POR_KERN; }
    inline bool IsArrowPortion() const { return nWhichPor == POR_ARROW; }
    inline bool IsMultiPortion() const { return nWhichPor == POR_MULTI; }
    inline bool IsNumberPortion() const { return nWhichPor == POR_NUMBER; } // #i23726#
    inline bool IsControlCharPortion() const { return nWhichPor == POR_CONTROLCHAR; }

    // Positioning
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    virtual sal_Int32 GetCrsrOfst( const KSHORT nOfst ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    void CalcTxtSize( const SwTxtSizeInfo &rInfo );

    // Output
    virtual void Paint( const SwTxtPaintInfo &rInf ) const = 0;
    void PrePaint( const SwTxtPaintInfo &rInf, const SwLinePortion *pLast ) const;

    virtual bool Format( SwTxtFormatInfo &rInf );
    // Is called for the line's last portion
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
            void Move( SwTxtPaintInfo &rInf );

    // For SwTxtSlot
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;

    // For SwFldPortion, SwSoftHyphPortion
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // for text- and multi-portions
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    bool GetJoinBorderWithPrev() const { return m_bJoinBorderWithPrev; }
    bool GetJoinBorderWithNext() const { return m_bJoinBorderWithNext; }
    void SetJoinBorderWithPrev( const bool bJoinPrev ) { m_bJoinBorderWithPrev = bJoinPrev; }
    void SetJoinBorderWithNext( const bool bJoinNext ) { m_bJoinBorderWithNext = bJoinNext; }

    OUTPUT_OPERATOR
};

inline SwLinePortion &SwLinePortion::operator=(const SwLinePortion &rPortion)
{
    *(SwPosSize*)this = rPortion;
    nLineLength = rPortion.nLineLength;
    nAscent = rPortion.nAscent;
    nWhichPor = rPortion.nWhichPor;
    m_bJoinBorderWithPrev = rPortion.m_bJoinBorderWithPrev;
    m_bJoinBorderWithNext = rPortion.m_bJoinBorderWithNext;
    return *this;
}

inline bool SwLinePortion::operator==(const SwLinePortion &rPortion ) const
{
    return( Height() == rPortion.Height() &&
            Width() == rPortion.Width() &&
            nLineLength == rPortion.GetLen() &&
            nAscent == rPortion.GetAscent() );
}

inline SwLinePortion::SwLinePortion(const SwLinePortion &rPortion) :
    SwPosSize( rPortion ),
    pPortion( 0 ),
    nLineLength( rPortion.nLineLength ),
    nAscent( rPortion.nAscent ),
    nWhichPor( rPortion.nWhichPor ),
    m_bJoinBorderWithPrev( rPortion.m_bJoinBorderWithPrev ),
    m_bJoinBorderWithNext( rPortion.m_bJoinBorderWithNext )
{
}

inline void SwLinePortion::Truncate()
{
    if ( pPortion )
        _Truncate();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
