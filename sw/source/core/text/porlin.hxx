/*************************************************************************
 *
 *  $RCSfile: porlin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-30 10:00:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _PORLIN_HXX
#define _PORLIN_HXX

#include "possiz.hxx"       // SwPosSize

class XubString;
class SwTxtSizeInfo;
class SwTxtPaintInfo;
class SwTxtFormatInfo;

// Die Ausgabeoperatoren der Portions sind virtuelle Methoden der Portion.
// Das CLASSIO-Makro implementiert die 'freischwebende' Funktion.
// Auf diese Weise erhaelt man beide Vorteile: virtuelle Ausgabeoperatoren
// und allgemeine Verwendbarkeit.
#ifndef PRODUCT
#define OUTPUT_OPERATOR  virtual SvStream &operator<<( SvStream & aOs ) const;
#else
#define OUTPUT_OPERATOR
#endif

// Portiongruppen
#define PORGRP_TXT      0x8000
#define PORGRP_EXP      0x4000
#define PORGRP_FLD      0x2000
#define PORGRP_HYPH     0x1000
#define PORGRP_NUMBER   0x0800
#define PORGRP_GLUE     0x0400
#define PORGRP_FIX      0x0200
#define PORGRP_TAB      0x0100
#define PORGRP_NOTRECY  0x0080
// kleine Spezialgruppen
#define PORGRP_FIXMARG  0x0040
#define PORGRP_FLDGLU   0x0020
#define PORGRP_TABNOTLFT 0x0010
#define PORGRP_TOXREF   0x0008

// Portiontypen
#define POR_LIN         0x0000
#define POR_FLYCNT      0x0001

#define POR_HOLE        0x0080
#define POR_TMPEND      0x0081
#define POR_BRK         0x0082
#define POR_KERN        0x0083
#define POR_ARROW       0x0084
#define POR_MULTI       0x0085

#define POR_TXT         0x8000
#define POR_LAY         0x8001
#define POR_PARA        0x8002
#define POR_URL         0x8003

#define POR_DROP        0x8080
#define POR_TOX         0x8089
#define POR_ISOTOX      0x808a
#define POR_REF         0x808b
#define POR_ISOREF      0x808c

#define POR_EXP         0xc080
#define POR_BLANK       0xc081
#define POR_FTN         0xc082
#define POR_POSTITS     0xc083

#define POR_HYPH        0xd080
#define POR_HYPHSTR     0xd081
#define POR_SOFTHYPH    0xd082
#define POR_SOFTHYPHSTR 0xd083

#define POR_FLD         0xe0a0
#define POR_HIDDEN      0xe0a1
#define POR_QUOVADIS    0xe0a2
#define POR_ERGOSUM     0xe0a3

#define POR_FTNNUM      0xe8a0
#define POR_NUMBER      0xe8a1
#define POR_BULLET      0xe8a2
#define POR_GRFNUM      0xe8a3

#define POR_GLUE        0x0480

#define POR_MARGIN      0x04c0

#define POR_FIX         0x06c0
#define POR_FLY         0x06c1

#define POR_TAB         0x0750

#define POR_TABRIGHT    0x07d0
#define POR_TABCENTER   0x07d1
#define POR_TABDECIMAL  0x07d2

#define POR_TABLEFT     0x0740

/*************************************************************************
 *                      class SwLinePortion
 *************************************************************************/

class SwLinePortion: public SwPosSize
{
protected:
    // Hier gibt es Bereiche mit unterschiedlichen Attributen.
    SwLinePortion *pPortion;
    // Anzahl der Zeichen und Spaces auf der Zeile
    xub_StrLen nLineLength;
    KSHORT nAscent;      // Maximaler Ascender

    SwLinePortion();
private:
    MSHORT nWhichPor;       // Who's who?

    void _Truncate();

public:
    inline          SwLinePortion(const SwLinePortion &rPortion);
           virtual ~SwLinePortion();

    // Zugriffsmethoden
    inline SwLinePortion *GetPortion() const { return( pPortion ); }
    inline SwLinePortion &operator=(const SwLinePortion &rPortion);
    inline sal_Bool operator==( const SwLinePortion &rPortion ) const;
    inline const xub_StrLen GetLen() const { return nLineLength; }
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
    inline  void Truncate();

    // liefert 0 zurueck, wenn keine Nutzdaten enthalten sind.
    virtual SwLinePortion *Compress();

    inline void SetWhichPor( const MSHORT nNew )    { nWhichPor = nNew; }
    inline const MSHORT GetWhichPor( )          { return nWhichPor; }

// Gruppenabfragen:
    inline const sal_Bool MayRecycle()  const
        { return nWhichPor & PORGRP_NOTRECY ? sal_False : sal_True; }
    inline const sal_Bool InTxtGrp( )   const { return nWhichPor & PORGRP_TXT ? sal_True : sal_False; }
    inline const sal_Bool InGlueGrp( )  const { return nWhichPor & PORGRP_GLUE ? sal_True : sal_False;}
    inline const sal_Bool InTabGrp( )   const { return nWhichPor & PORGRP_TAB ? sal_True : sal_False; }
    inline const sal_Bool InHyphGrp( )  const { return nWhichPor & PORGRP_HYPH ? sal_True : sal_False;}
    inline const sal_Bool InNumberGrp( )const { return nWhichPor & PORGRP_NUMBER ? sal_True : sal_False;}
    inline const sal_Bool InFixGrp( )   const { return nWhichPor & PORGRP_FIX ? sal_True : sal_False;  }
    inline const sal_Bool InFldGrp( )   const { return nWhichPor & PORGRP_FLD ? sal_True : sal_False;  }
    inline const sal_Bool InToxRefGrp( ) const { return nWhichPor & PORGRP_TOXREF ? sal_True : sal_False;  }
    inline const sal_Bool InToxRefOrFldGrp( )   const { return nWhichPor &
                                ( PORGRP_FLD | PORGRP_TOXREF ) ? sal_True : sal_False;  }
    inline const sal_Bool InExpGrp( )   const { return nWhichPor & PORGRP_EXP ? sal_True : sal_False;  }
    inline const sal_Bool InTabnLftGrp( ) const
        { return nWhichPor & PORGRP_TABNOTLFT ? sal_True : sal_False;  }
    inline const sal_Bool InFldGluGrp( )const
        { return nWhichPor & PORGRP_FLDGLU ? sal_True : sal_False;  }
    inline const sal_Bool InFixMargGrp( )const
        { return nWhichPor & PORGRP_FIXMARG ? sal_True : sal_False;  }
    inline const sal_Bool InSpaceGrp( )const
        { return InTxtGrp() || IsMultiPortion();  }
// Individuelle Abfragen:
    inline const sal_Bool IsGrfNumPortion( )const{ return nWhichPor == POR_GRFNUM; }
    inline const sal_Bool IsFlyCntPortion( )const{ return nWhichPor == POR_FLYCNT; }
    inline const sal_Bool IsBlankPortion( ) const{ return nWhichPor == POR_BLANK; }
    inline const sal_Bool IsBreakPortion( ) const{ return nWhichPor == POR_BRK; }
    inline const sal_Bool IsErgoSumPortion()const{ return nWhichPor == POR_ERGOSUM;}
    inline const sal_Bool IsQuoVadisPortion()const{ return nWhichPor==POR_QUOVADIS;}
    inline const sal_Bool IsTabCntPortion( )const{ return nWhichPor==POR_TABCENTER;}
    inline const sal_Bool IsTabLeftPortion()const{ return nWhichPor == POR_TABLEFT;}
    inline const sal_Bool IsFtnNumPortion( )const{ return nWhichPor == POR_FTNNUM; }
    inline const sal_Bool IsFtnPortion( )   const{ return nWhichPor == POR_FTN; }
    inline const sal_Bool IsTmpEndPortion( )const{ return nWhichPor == POR_TMPEND; }
    inline const sal_Bool IsDropPortion( )  const{ return nWhichPor == POR_DROP; }
    inline const sal_Bool IsLayPortion( )   const{ return nWhichPor == POR_LAY; }
    inline const sal_Bool IsParaPortion( )  const{ return nWhichPor == POR_PARA; }
    inline const sal_Bool IsMarginPortion( )const{ return nWhichPor == POR_MARGIN; }
    inline const sal_Bool IsFlyPortion( )   const{ return nWhichPor == POR_FLY; }
    inline const sal_Bool IsHolePortion( )  const{ return nWhichPor == POR_HOLE; }
    inline const sal_Bool IsSoftHyphPortion()const{ return nWhichPor==POR_SOFTHYPH;}
    inline const sal_Bool IsPostItsPortion()const{ return nWhichPor == POR_POSTITS;}
    inline const sal_Bool IsTextPortion( ) const{ return nWhichPor == POR_TXT; }
    inline const sal_Bool IsURLPortion( ) const{ return nWhichPor == POR_URL; }
    inline const sal_Bool IsKernPortion( ) const{ return nWhichPor == POR_KERN; }
    inline const sal_Bool IsArrowPortion( ) const{ return nWhichPor == POR_ARROW; }
    inline const sal_Bool IsMultiPortion( ) const{ return nWhichPor == POR_MULTI; }

    // Positionierung
    SwLinePortion *FindPrevPortion( const SwLinePortion *pRoot );
    SwLinePortion *FindLastPortion();

    virtual xub_StrLen GetCrsrOfst( const KSHORT nOfst ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    void CalcTxtSize( const SwTxtSizeInfo &rInfo );

    // Ausgabe
    virtual void Paint( const SwTxtPaintInfo &rInf ) const = 0;
    void PrePaint( const SwTxtPaintInfo &rInf, SwLinePortion *pLast ) const;

#ifndef PRODUCT
    virtual sal_Bool Check( SvStream &rOs, SwTxtSizeInfo &rInfo ); //$ ostream
#endif

    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    // wird fuer die letzte Portion der Zeile extra gerufen
    virtual void FormatEOL( SwTxtFormatInfo &rInf );
            void Move( SwTxtPaintInfo &rInf );

    // Fuer SwTxtSlot
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    // fuer SwFldPortion, SwSoftHyphPortion
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    // for text- and multi-portions
    virtual long CalcSpacing( short nSpaceAdd, const SwTxtSizeInfo &rInf ) const;

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

#endif
