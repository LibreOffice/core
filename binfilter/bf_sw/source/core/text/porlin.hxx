/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _PORLIN_HXX
#define _PORLIN_HXX

#include "possiz.hxx"		// SwPosSize
class XubString; 
namespace binfilter {


class SwTxtSizeInfo;
class SwTxtPaintInfo;
class SwTxtFormatInfo;
class SwPortionHandler;

// Die Ausgabeoperatoren der Portions sind virtuelle Methoden der Portion.
// Das CLASSIO-Makro implementiert die 'freischwebende' Funktion.
// Auf diese Weise erhaelt man beide Vorteile: virtuelle Ausgabeoperatoren
// und allgemeine Verwendbarkeit.
#ifdef DBG_UTIL
#define OUTPUT_OPERATOR  virtual SvStream &operator<<( SvStream & aOs ) const;
#else
#define OUTPUT_OPERATOR
#endif

// Portiongruppen
#define PORGRP_TXT		0x8000
#define PORGRP_EXP		0x4000
#define PORGRP_FLD		0x2000
#define PORGRP_HYPH		0x1000
#define PORGRP_NUMBER	0x0800
#define PORGRP_GLUE		0x0400
#define PORGRP_FIX		0x0200
#define PORGRP_TAB		0x0100
#define PORGRP_NOTRECY	0x0080
// kleine Spezialgruppen
#define PORGRP_FIXMARG	0x0040
//#define PORGRP_?	0x0020
#define PORGRP_TABNOTLFT 0x0010
#define PORGRP_TOXREF	0x0008

/*************************************************************************
 *						class SwLinePortion
 *************************************************************************/

class SwLinePortion: public SwPosSize
{
protected:
    // Hier gibt es Bereiche mit unterschiedlichen Attributen.
    SwLinePortion *pPortion;
    // Anzahl der Zeichen und Spaces auf der Zeile
    xub_StrLen nLineLength;
    KSHORT nAscent; 	 // Maximaler Ascender

    SwLinePortion();
private:
    MSHORT nWhichPor;		// Who's who?

    void _Truncate();

public:
    inline 			SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Zugriffsmethoden
    inline SwLinePortion *GetPortion() const { return( pPortion ); }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    inline sal_Bool operator==( const SwLinePortion &rPortion ) const;
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

    // Einfuegeoperationen:
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
            SwLinePortion *Cut( SwLinePortion *pVictim );
    inline	void Truncate();

    // liefert 0 zurueck, wenn keine Nutzdaten enthalten sind.
    virtual SwLinePortion *Compress();

    inline void SetWhichPor( const MSHORT nNew )	{ nWhichPor = nNew; }
    inline MSHORT GetWhichPor( ) const        { return nWhichPor; }

// Gruppenabfragen:
    inline sal_Bool InTxtGrp( )	const { return nWhichPor & PORGRP_TXT ? sal_True : sal_False; }
    inline sal_Bool InGlueGrp( )	const { return nWhichPor & PORGRP_GLUE ? sal_True : sal_False;}
    inline sal_Bool InTabGrp( )	const { return nWhichPor & PORGRP_TAB ? sal_True : sal_False; }
    inline sal_Bool InHyphGrp( )	const { return nWhichPor & PORGRP_HYPH ? sal_True : sal_False;}
    inline sal_Bool InNumberGrp( )const { return nWhichPor & PORGRP_NUMBER ? sal_True : sal_False;}
    inline sal_Bool InFixGrp( )	const { return nWhichPor & PORGRP_FIX ? sal_True : sal_False;  }
    inline sal_Bool InFldGrp( )	const { return nWhichPor & PORGRP_FLD ? sal_True : sal_False;  }
    inline sal_Bool InToxRefGrp( ) const { return nWhichPor &	PORGRP_TOXREF ? sal_True : sal_False;  }
    inline sal_Bool InToxRefOrFldGrp( )	const { return nWhichPor &
                                ( PORGRP_FLD | PORGRP_TOXREF ) ? sal_True : sal_False;  }
    inline sal_Bool InExpGrp( )	const { return nWhichPor & PORGRP_EXP ? sal_True : sal_False;  }
    inline sal_Bool InTabnLftGrp( ) const
        { return nWhichPor & PORGRP_TABNOTLFT ? sal_True : sal_False;  }
    inline sal_Bool InFixMargGrp( )const
        { return nWhichPor & PORGRP_FIXMARG ? sal_True : sal_False;  }
    inline sal_Bool InSpaceGrp( )const
        { return InTxtGrp() || IsMultiPortion();  }
// Individuelle Abfragen:
    inline sal_Bool IsGrfNumPortion( )const{ return nWhichPor == POR_GRFNUM; }
    inline sal_Bool IsFlyCntPortion( )const{ return nWhichPor == POR_FLYCNT; }
    inline sal_Bool IsBlankPortion( )	const{ return nWhichPor == POR_BLANK; }
    inline sal_Bool IsBreakPortion( )	const{ return nWhichPor == POR_BRK; }
    inline sal_Bool IsErgoSumPortion()const{ return nWhichPor == POR_ERGOSUM;}
    inline sal_Bool IsQuoVadisPortion()const{ return nWhichPor==POR_QUOVADIS;}
    inline sal_Bool IsTabCntPortion( )const{ return nWhichPor==POR_TABCENTER;}
    inline sal_Bool IsTabLeftPortion()const{ return nWhichPor == POR_TABLEFT;}
    inline sal_Bool IsFtnNumPortion( )const{ return nWhichPor == POR_FTNNUM; }
    inline sal_Bool IsFtnPortion( )	const{ return nWhichPor == POR_FTN; }
    inline sal_Bool IsTmpEndPortion( )const{ return nWhichPor == POR_TMPEND; }
    inline sal_Bool IsDropPortion( )	const{ return nWhichPor == POR_DROP; }
    inline sal_Bool IsLayPortion( )	const{ return nWhichPor == POR_LAY; }
    inline sal_Bool IsParaPortion( )	const{ return nWhichPor == POR_PARA; }
    inline sal_Bool IsMarginPortion( )const{ return nWhichPor == POR_MARGIN; }
    inline sal_Bool IsFlyPortion( )	const{ return nWhichPor == POR_FLY; }
    inline sal_Bool IsHolePortion( )	const{ return nWhichPor == POR_HOLE; }
    inline sal_Bool IsSoftHyphPortion()const{ return nWhichPor==POR_SOFTHYPH;}
    inline sal_Bool IsPostItsPortion()const{ return nWhichPor == POR_POSTITS;}
    inline sal_Bool IsCombinedPortion()const{ return nWhichPor==POR_COMBINED;}
    inline sal_Bool IsTextPortion( ) const{ return nWhichPor == POR_TXT; }
    inline sal_Bool IsURLPortion( ) const{ return nWhichPor == POR_URL; }
    inline sal_Bool IsHangingPortion( ) const{ return nWhichPor == POR_HNG; }
    inline sal_Bool IsKernPortion( ) const{ return nWhichPor == POR_KERN; }
    inline sal_Bool IsArrowPortion( ) const{ return nWhichPor == POR_ARROW; }
    inline sal_Bool IsMultiPortion( ) const{ return nWhichPor == POR_MULTI; }

    // Positionierung
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    void CalcTxtSize( const SwTxtSizeInfo &rInfo );

    // Ausgabe
    virtual void Paint( const SwTxtPaintInfo &rInf ) const = 0;

#ifdef DBG_UTIL
#endif

    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    // wird fuer die letzte Portion der Zeile extra gerufen
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
            void Move( SwTxtPaintInfo &rInf );

    // Fuer SwTxtSlot
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    // fuer SwFldPortion, SwSoftHyphPortion

    // for text- and multi-portions
    virtual long CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

    // Accessibility: pass information about this portion to the PortionHandler

    OUTPUT_OPERATOR
};


/*************************************************************************
 *					inline - Implementations
 *************************************************************************/

inline SwLinePortion &SwLinePortion::operator=(const SwLinePortion &rPortion)
{
    *(SwPosSize*)this = rPortion;
    nLineLength = rPortion.nLineLength;
    nAscent = rPortion.nAscent;
    nWhichPor = rPortion.nWhichPor;
    return *this;
}

inline sal_Bool SwLinePortion::operator==(const SwLinePortion &rPortion ) const
{
    return( *(SwPosSize*)this == rPortion &&
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
#ifdef DBGTXT
#define CLASSIO( class ) \
    inline SvStream &operator<<( SvStream &rOs, const class &rClass ) {\
            return rClass.operator<<( rOs );\
    }
#else
#define CLASSIO( class )
#endif

CLASSIO( SwLinePortion )

} //namespace binfilter
#endif
