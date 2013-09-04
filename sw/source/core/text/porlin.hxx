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
#ifndef _PORLIN_HXX
#define _PORLIN_HXX

#include "possiz.hxx"       // SwPosSize

#ifdef DBG_UTIL
#include <libxml/xmlwriter.h>
#endif

class SwTxtSizeInfo;
class SwTxtPaintInfo;
class SwTxtFormatInfo;
class SwPortionHandler;

// The portions output operators are virtual methods of the portion.
// The CLASSIO macro implements the 'levitating' function.
// In doing so we end up with both benefits: virtual output operators and general utility
#ifdef DBG_UTIL
#define OUTPUT_OPERATOR  virtual SvStream &operator<<( SvStream & aOs ) const;
#else
#define OUTPUT_OPERATOR
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

/*************************************************************************
 *                      class SwLinePortion
 *************************************************************************/

/// Base class for anything that can be part of a line in the Writer layout.
class SwLinePortion: public SwPosSize
{
protected:
    // Here we have areas with different attributes
    SwLinePortion *pPortion;
    // Count of chars and spaces on the line
    xub_StrLen nLineLength;
    KSHORT nAscent;      // Maximum ascender

    SwLinePortion();
private:
    MSHORT nWhichPor;       // Who's who?

    void _Truncate();

public:
    inline          SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Access methods
    inline SwLinePortion *GetPortion() const { return( pPortion ); }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    inline bool operator==( const SwLinePortion &rPortion ) const;
    inline xub_StrLen GetLen() const { return nLineLength; }
    inline void SetLen( const xub_StrLen nLen ) { nLineLength = nLen; }
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
    inline sal_Bool InTxtGrp( ) const { return nWhichPor & PORGRP_TXT ? sal_True : sal_False; }
    inline sal_Bool InGlueGrp( )    const { return nWhichPor & PORGRP_GLUE ? sal_True : sal_False;}
    inline sal_Bool InTabGrp( ) const { return nWhichPor & PORGRP_TAB ? sal_True : sal_False; }
    inline sal_Bool InHyphGrp( )    const { return nWhichPor & PORGRP_HYPH ? sal_True : sal_False;}
    inline sal_Bool InNumberGrp( )const { return nWhichPor & PORGRP_NUMBER ? sal_True : sal_False;}
    inline sal_Bool InFixGrp( ) const { return nWhichPor & PORGRP_FIX ? sal_True : sal_False;  }
    inline sal_Bool InFldGrp( ) const { return nWhichPor & PORGRP_FLD ? sal_True : sal_False;  }
    inline sal_Bool InToxRefGrp( ) const { return nWhichPor &   PORGRP_TOXREF ? sal_True : sal_False;  }
    inline sal_Bool InToxRefOrFldGrp( ) const { return nWhichPor &
                                ( PORGRP_FLD | PORGRP_TOXREF ) ? sal_True : sal_False;  }
    inline sal_Bool InExpGrp( ) const { return nWhichPor & PORGRP_EXP ? sal_True : sal_False;  }
    inline sal_Bool InTabnLftGrp( ) const
        { return nWhichPor & PORGRP_TABNOTLFT ? sal_True : sal_False;  }
    inline sal_Bool InFixMargGrp( )const
        { return nWhichPor & PORGRP_FIXMARG ? sal_True : sal_False;  }
    inline sal_Bool InSpaceGrp( )const
        { return InTxtGrp() || IsMultiPortion();  }
// Individual queries
    inline sal_Bool IsGrfNumPortion( )const{ return nWhichPor == POR_GRFNUM; }
    inline sal_Bool IsFlyCntPortion( )const{ return nWhichPor == POR_FLYCNT; }
    inline sal_Bool IsBlankPortion( )   const{ return nWhichPor == POR_BLANK; }
    inline sal_Bool IsBreakPortion( )   const{ return nWhichPor == POR_BRK; }
    inline sal_Bool IsErgoSumPortion()const{ return nWhichPor == POR_ERGOSUM;}
    inline sal_Bool IsQuoVadisPortion()const{ return nWhichPor==POR_QUOVADIS;}
    inline sal_Bool IsTabCntPortion( )const{ return nWhichPor==POR_TABCENTER;}
    inline sal_Bool IsTabDecimalPortion() const { return nWhichPor == POR_TABDECIMAL;}
    inline sal_Bool IsTabLeftPortion()const{ return nWhichPor == POR_TABLEFT;}
    inline sal_Bool IsFtnNumPortion( )const{ return nWhichPor == POR_FTNNUM; }
    inline sal_Bool IsFtnPortion( ) const{ return nWhichPor == POR_FTN; }
    inline sal_Bool IsTmpEndPortion( )const{ return nWhichPor == POR_TMPEND; }
    inline sal_Bool IsDropPortion( )    const{ return nWhichPor == POR_DROP; }
    inline sal_Bool IsLayPortion( ) const{ return nWhichPor == POR_LAY; }
    inline sal_Bool IsParaPortion( )    const{ return nWhichPor == POR_PARA; }
    inline sal_Bool IsMarginPortion( )const{ return nWhichPor == POR_MARGIN; }
    inline sal_Bool IsFlyPortion( ) const{ return nWhichPor == POR_FLY; }
    inline sal_Bool IsHolePortion( )    const{ return nWhichPor == POR_HOLE; }
    inline sal_Bool IsSoftHyphPortion()const{ return nWhichPor==POR_SOFTHYPH;}
    inline sal_Bool IsPostItsPortion()const{ return nWhichPor == POR_POSTITS;}
    inline sal_Bool IsCombinedPortion()const{ return nWhichPor==POR_COMBINED;}
    inline sal_Bool IsTextPortion( ) const{ return nWhichPor == POR_TXT; }
    inline sal_Bool IsURLPortion( ) const{ return nWhichPor == POR_URL; }
    inline sal_Bool IsHangingPortion( ) const{ return nWhichPor == POR_HNG; }
    inline sal_Bool IsKernPortion( ) const{ return nWhichPor == POR_KERN; }
    inline sal_Bool IsArrowPortion( ) const{ return nWhichPor == POR_ARROW; }
    inline sal_Bool IsMultiPortion( ) const{ return nWhichPor == POR_MULTI; }
    inline sal_Bool IsNumberPortion( ) const{ return nWhichPor == POR_NUMBER; } // #i23726#
    inline sal_Bool IsControlCharPortion() const { return nWhichPor == POR_CONTROLCHAR; }

    // Positioning
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    void CalcTxtSize( const SwTxtSizeInfo &rInfo );

    // Output
    virtual void Paint( const SwTxtPaintInfo &rInf ) const = 0;
    void PrePaint( const SwTxtPaintInfo &rInf, const SwLinePortion *pLast ) const;

    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    // Is called for the line's last portion
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
            void Move( SwTxtPaintInfo &rInf );

    // For SwTxtSlot
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;

    // For SwFldPortion, SwSoftHyphPortion
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // for text- and multi-portions
    virtual long CalcSpacing( long nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};


/*************************************************************************
 *                  inline - Implementations
 *************************************************************************/

inline SwLinePortion &SwLinePortion::operator=(const SwLinePortion &rPortion)
{
    *(SwPosSize*)this = rPortion;
    nLineLength = rPortion.nLineLength;
    nAscent = rPortion.nAscent;
    nWhichPor = rPortion.nWhichPor;
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
    nWhichPor( rPortion.nWhichPor )
{
}

inline void SwLinePortion::Truncate()
{
    if ( pPortion )
        _Truncate();
}


//$ ostream
#define CLASSIO( class )

CLASSIO( SwLinePortion )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
