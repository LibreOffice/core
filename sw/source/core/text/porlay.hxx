/*************************************************************************
 *
 *  $RCSfile: porlay.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
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
#ifndef _PORLAY_HXX
#define _PORLAY_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_SHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "swrect.hxx"   // SwRepaint
#include "portxt.hxx"

class SwMarginPortion;
class SwDropPortion;
class SvStream;
class SvUShorts;
class SwTxtFormatter;

/*************************************************************************
 *                      class SwCharRange
 *************************************************************************/

class SwCharRange
{
    xub_StrLen nStart, nLen;
public:
    inline SwCharRange( const xub_StrLen nInitStart = 0,
        const xub_StrLen nInitLen = 0): nStart( nInitStart ), nLen(nInitLen) {}
    inline xub_StrLen &Start() { return nStart; }
    inline const xub_StrLen &Start() const { return nStart; }
    inline void LeftMove( xub_StrLen nNew )
            { if ( nNew < nStart ) { nLen += nStart-nNew; nStart = nNew; } }
    inline xub_StrLen End() const
                { return nStart + nLen; }
    inline xub_StrLen &Len() { return nLen; }
    inline const xub_StrLen &Len() const { return nLen; }
    inline sal_Bool operator<(const SwCharRange &rRange) const
                { return nStart < rRange.nStart; }
    inline sal_Bool operator>(const SwCharRange &rRange) const
                { return nStart + nLen > rRange.nStart + rRange.nLen; }
    inline sal_Bool operator!=(const SwCharRange &rRange) const
                { return *this < rRange || *this > rRange; }
    SwCharRange &operator+=(const SwCharRange &rRange);
};

/*************************************************************************
 *                      class SwRepaint
 *************************************************************************/

// SwRepaint ist ein dokumentglobales SwRect mit einem nOfst der angibt,
// ab wo in der ersten Zeile gepaintet werden soll
// und einem nRightOfst, der den rechten Rand bestimmt
class SwRepaint : public SwRect
{
    SwTwips nOfst;
    SwTwips nRightOfst;
public:
    SwRepaint() : SwRect(), nOfst( 0 ), nRightOfst( 0 ) {}
    SwRepaint( const SwRepaint& rRep ) : SwRect( rRep ), nOfst( rRep.nOfst ),
        nRightOfst( rRep.nRightOfst ) {}

    SwTwips GetOfst() const { return nOfst; }
    void   SetOfst( const SwTwips nNew ) { nOfst = nNew; }
    SwTwips GetRightOfst() const { return nRightOfst; }
    void   SetRightOfst( const SwTwips nNew ) { nRightOfst = nNew; }
};

/*************************************************************************
 *                      class SwLineLayout
 *************************************************************************/

class SwLineLayout : public SwTxtPortion
{
private:
    SwLineLayout *pNext;// Die naechste Zeile.
    SvShorts *pSpaceAdd;// Fuer den Blocksatz
    KSHORT nRealHeight; // Die aus Zeilenabstand/Register resultierende Hoehe
    sal_Bool bFormatAdj : 1;
    sal_Bool bDummy     : 1;
    sal_Bool bFntChg    : 1;
    sal_Bool bEndHyph   : 1;
    sal_Bool bMidHyph   : 1;
    sal_Bool bTab       : 1;
    sal_Bool bFly       : 1;
    sal_Bool bFlyCnt    : 1;
    sal_Bool bRest      : 1;
    sal_Bool bBlinking  : 1;
    sal_Bool bClipping  : 1; // Clipping erforderlich wg. exakter Zeilenhoehe
    sal_Bool bContent   : 1; // enthaelt Text, fuer Zeilennumerierung
    sal_Bool bRedline   : 1; // enthaelt Redlining
    sal_Bool bForcedLeftMargin : 1; // vom Fly verschobener linker Einzug

public:
    // von SwLinePortion
    virtual SwLinePortion *Insert( SwLinePortion *pPortion );
    virtual SwLinePortion *Append( SwLinePortion *pPortion );
    inline SwLinePortion *GetFirstPortion() const;

    // Flags
    inline void ResetFlags();
    inline void SetFormatAdj( const sal_Bool bNew ) { bFormatAdj = bNew; }
    inline sal_Bool IsFormatAdj() const { return bFormatAdj; }
    inline void SetFntChg( const sal_Bool bNew ) { bFntChg = bNew; }
    inline sal_Bool IsFntChg() const { return bFntChg; }
    inline void SetEndHyph( const sal_Bool bNew ) { bEndHyph = bNew; }
    inline sal_Bool IsEndHyph() const { return bEndHyph; }
    inline void SetMidHyph( const sal_Bool bNew ) { bMidHyph = bNew; }
    inline sal_Bool IsMidHyph() const { return bMidHyph; }
    inline void SetTab( const sal_Bool bNew ) { bTab = bNew; }
    inline sal_Bool IsTab() const { return bTab; }
    inline void SetFly( const sal_Bool bNew ) { bFly = bNew; }
    inline sal_Bool IsFly() const { return bFly; }
    inline void SetFlyCnt( const sal_Bool bNew ) { bFlyCnt = bNew; }
    inline sal_Bool IsFlyCnt() const { return bFlyCnt; }
    inline void SetRest( const sal_Bool bNew ) { bRest = bNew; }
    inline sal_Bool IsRest() const { return bRest; }
    inline void SetBlinking( const sal_Bool bNew = sal_True ) { bBlinking = bNew; }
    inline sal_Bool IsBlinking() const { return bBlinking; }
    inline void SetCntnt( const sal_Bool bNew = sal_True ) { bContent = bNew; }
    inline sal_Bool HasCntnt() const { return bContent; }
    inline void SetRedline( const sal_Bool bNew = sal_True ) { bRedline = bNew; }
    inline sal_Bool HasRedline() const { return bRedline; }
    inline void SetForcedLeftMargin( const sal_Bool bNew = sal_True ) { bForcedLeftMargin = bNew; }
    inline sal_Bool HasForcedLeftMargin() const { return bForcedLeftMargin; }

    // Beruecksichtigung von Dummyleerzeilen
    // 4147, 8221:
    inline void SetDummy( const sal_Bool bNew ) { bDummy = bNew; }
    inline sal_Bool IsDummy() const { return bDummy; }

    inline void SetClipping( const sal_Bool bNew ) { bClipping = bNew; }
    inline sal_Bool IsClipping() const { return bClipping; }

    inline SwLineLayout();
    virtual ~SwLineLayout();

    inline SwLineLayout *GetNext() { return pNext; }
    inline const SwLineLayout *GetNext() const { return pNext; }
    inline void SetNext( SwLineLayout *pNew ) { pNext = pNew; }

    void Init( SwLinePortion *pNextPortion = NULL);

    // Sammelt die Daten fuer die Zeile.
    void CalcLine( SwTxtFormatter &rLine );

    inline void SetRealHeight( KSHORT nNew ) { nRealHeight = nNew; }
    inline KSHORT GetRealHeight() const { return nRealHeight; }

    // Erstellt bei kurzen Zeilen die Glue-Kette.
    SwMarginPortion *CalcLeftMargin();

    // fuer die Sonderbehandlung bei leeren Zeilen
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    inline sal_Bool IsNoSpaceAdd() { return pSpaceAdd == NULL; }
    inline void InitSpaceAdd()
    { if ( !pSpaceAdd ) CreateSpaceAdd(); else (*pSpaceAdd)[0] = 0; }
    inline void FinishSpaceAdd() { delete pSpaceAdd; pSpaceAdd = NULL; }
    inline SvShorts* GetpSpaceAdd() const { return pSpaceAdd; }
    inline SvShorts &GetSpaceAdd() { return *pSpaceAdd; }
    void CreateSpaceAdd();

#ifndef PRODUCT
    void DebugPortions( SvStream &rOs, const XubString &rTxt,
                        const xub_StrLen nStart ); //$ ostream
#endif

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwLineLayout)
};

class SwParaPortion : public SwLineLayout
{
    // neu zu paintender Bereich
    SwRepaint aRepaint;
    // neu zu formatierender Bereich
    SwCharRange aReformat;
    long nDelta;

    // Wenn ein SwTxtFrm gelocked ist, werden keine Veraenderungen an den
    // Formatierungsdaten (unter pLine) vorgenommen (vgl. ORPHANS)
    sal_Bool bFlys          : 1; // Ueberlappen Flys ?
    sal_Bool bPrep          : 1; // PREP_*
    sal_Bool bPrepWidows    : 1; // PREP_WIDOWS
    sal_Bool bPrepAdjust    : 1; // PREP_ADJUST_FRM
    sal_Bool bPrepMustFit   : 1; // PREP_MUST_FIT
    sal_Bool bFollowField   : 1; // Es steht noch ein Feldrest fuer den Follow an.

    sal_Bool bFixLineHeight : 1; // Feste Zeilenhoehe
    sal_Bool bFtnNum    : 1; // contents a footnotenumberportion
    sal_Bool bFlag09    : 1; //

    sal_Bool bFlag00    : 1; //
    sal_Bool bFlag11    : 1; //
    sal_Bool bFlag12    : 1; //
    sal_Bool bFlag13    : 1; //
    sal_Bool bFlag14    : 1; //
    sal_Bool bFlag15    : 1; //
    sal_Bool bFlag16    : 1; //

public:
            SwParaPortion();

    // setzt alle Formatinformationen zurueck (ausser bFlys wg. 9916)
    inline void FormatReset();

    // Setzt die Flags zurueck
    inline void ResetPreps();

    // Get/Set-Methoden
    inline SwRepaint *GetRepaint() { return &aRepaint; }
    inline const SwRepaint *GetRepaint() const { return &aRepaint; }
    inline SwCharRange *GetReformat() { return &aReformat; }
    inline const SwCharRange *GetReformat() const { return &aReformat; }
    inline long *GetDelta() { return &nDelta; }
    inline const long *GetDelta() const { return &nDelta; }

    // fuer SwTxtFrm::Format: liefert die aktuelle Laenge des Absatzes
    xub_StrLen GetParLen() const;

    // fuer Prepare()
    sal_Bool UpdateQuoVadis( const XubString &rQuo );

    // Flags
    inline void SetFly( const sal_Bool bNew = sal_True ) { bFlys = bNew; }
    inline sal_Bool HasFly() const { return bFlys; }

    // Preps
    inline void SetPrep( const sal_Bool bNew = sal_True ) { bPrep = bNew; }
    inline sal_Bool IsPrep() const { return bPrep; }
    inline void SetPrepWidows( const sal_Bool bNew = sal_True ) { bPrepWidows = bNew; }
    inline sal_Bool IsPrepWidows() const { return bPrepWidows; }
    inline void SetPrepMustFit( const sal_Bool bNew = sal_True ) { bPrepMustFit = bNew; }
    inline sal_Bool IsPrepMustFit() const { return bPrepMustFit; }
    inline void SetPrepAdjust( const sal_Bool bNew = sal_True ) { bPrepAdjust = bNew; }
    inline sal_Bool IsPrepAdjust() const { return bPrepAdjust; }
    inline void SetFollowField( const sal_Bool bNew = sal_True ) { bFollowField = bNew; }
    inline sal_Bool IsFollowField() const { return bFollowField; }
    inline void SetFixLineHeight( const sal_Bool bNew = sal_True ) { bFixLineHeight = bNew; }
    inline sal_Bool  IsFixLineHeight() const { return bFixLineHeight; }

    inline void SetFtnNum( const sal_Bool bNew = sal_True ) { bFtnNum = bNew; }
    inline sal_Bool  IsFtnNum() const { return bFtnNum; }
    inline void SetFlag09( const sal_Bool bNew = sal_True ) { bFlag09 = bNew; }
    inline sal_Bool  IsFlag09() const { return bFlag09; }
    inline void SetFlag00( const sal_Bool bNew = sal_True ) { bFlag00 = bNew; }
    inline sal_Bool  IsFlag00() const { return bFlag00; }
    inline void SetFlag11( const sal_Bool bNew = sal_True ) { bFlag11 = bNew; }
    inline sal_Bool  IsFlag11() const { return bFlag11; }
    inline void SetFlag12( const sal_Bool bNew = sal_True ) { bFlag12 = bNew; }
    inline sal_Bool  IsFlag12() const { return bFlag12; }
    inline void SetFlag13( const sal_Bool bNew = sal_True ) { bFlag13 = bNew; }
    inline sal_Bool  IsFlag13() const { return bFlag13; }
    inline void SetFlag14( const sal_Bool bNew = sal_True ) { bFlag14 = bNew; }
    inline sal_Bool  IsFlag14() const { return bFlag14; }
    inline void SetFlag15( const sal_Bool bNew = sal_True ) { bFlag15 = bNew; }
    inline sal_Bool  IsFlag15() const { return bFlag15; }
    inline void SetFlag16( const sal_Bool bNew = sal_True ) { bFlag16 = bNew; }
    inline sal_Bool  IsFlag16() const { return bFlag16; }

    // schneller, hoeher, weiter: Read/Write-Methoden fuer den SWG-Filter
    SvStream &ReadSwg ( SvStream& rStream ); //$ istream
    SvStream &WriteSwg( SvStream& rStream ); //$ ostream

    // nErgo in der QuoVadisPortion setzen
    void SetErgoSumNum( const XubString &rErgo );

    const SwDropPortion *FindDropPortion() const;

    OUTPUT_OPERATOR
    DECL_FIXEDMEMPOOL_NEWDEL(SwParaPortion)
};

/*************************************************************************
 *                      Inline-Implementierungen
 *************************************************************************/

inline void SwLineLayout::ResetFlags()
{
    bFormatAdj = bDummy = bFntChg = bTab = bEndHyph = bMidHyph = bFly = bFlyCnt
    = bRest = bBlinking = bClipping = bContent = bRedline
    = bForcedLeftMargin = sal_False;
    pSpaceAdd = NULL;
}

inline SwLineLayout::SwLineLayout()
    : pNext(0), nRealHeight(0)
{
    ResetFlags();
    SetWhichPor( POR_LAY );
}

inline void SwParaPortion::ResetPreps()
{
    bPrep = bPrepWidows = bPrepAdjust = bPrepMustFit = sal_False;
}

inline void SwParaPortion::FormatReset()
{
    nDelta = 0;
    aReformat = SwCharRange( 0, STRING_LEN );
//  AMA 9916: bFlys muss in SwTxtFrm::_Format() erhalten bleiben, damit
//  leere Absaetze, die Rahmen ohne Umfluss ausweichen mussten, sich
//  neu formatieren, wenn der Rahmen aus dem Bereich verschwindet.
//  bFlys = sal_False;
    ResetPreps();
    bFollowField = bFixLineHeight = sal_False;
}

#ifdef UNX
// C30 ist mit dem ternaeren Ausdruck ueberfordert.
inline SwLinePortion *SwLineLayout::GetFirstPortion() const
{
    register SwLinePortion *pTmp = pPortion;
    if ( !pPortion )
        pTmp = (SwLinePortion*)this;
    return( pTmp );
}
#else
inline SwLinePortion *SwLineLayout::GetFirstPortion() const
{ return( pPortion ? pPortion : (SwLinePortion*)this ); }
#endif

CLASSIO( SwLineLayout )
CLASSIO( SwParaPortion )


#endif
