/*************************************************************************
 *
 *  $RCSfile: itrtxt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2000-10-23 10:15:30 $
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
#ifndef _ITRTXT_HXX
#define _ITRTXT_HXX

#include "swtypes.hxx"
#include "itratr.hxx"
#include "inftxt.hxx"

class SwTxtFrm;
struct SwPosition;
struct SwCrsrMoveState;
class SwMarginPortion;
class SwFlyPortion;

/*************************************************************************
 *                      class SwTxtIter
 *************************************************************************/

class SwTxtIter : public SwAttrIter
{
protected:
    SwLineInfo aLineInf;
    Point aTopLeft;         // erste Ausgabeposition
    SwTxtFrm  *pFrm;
    SwTxtInfo *pInf;
    SwLineLayout *pCurr;
    SwLineLayout *pPrev;
    SwTwips nY;
    SwTwips nRegStart;          // Anfangsposition (Y) des Registers
    xub_StrLen nStart;          // Start im Textstring, Ende = pCurr->GetLen()
    KSHORT nRegDiff;            // Zeilenabstand des Registers
    MSHORT nLineNr;             // Zeilennummer
    sal_Bool bPrev          : 1;
    sal_Bool bRegisterOn    : 1;    // Registerhaltigkeit
    sal_Bool bOneBlock      : 1;    // Blocksatz: Einzelwoerter austreiben
    sal_Bool bLastBlock     : 1;    // Blocksatz: Auch die letzte Zeile
    sal_Bool bLastCenter    : 1;    // Blocksatz: Letzte Zeile zentrieren
    sal_Bool bUnderPara     : 1;    // Absatz unterstrichen
    sal_Bool bEscPara       : 1;    // Absatz mit Escapement

    SwLineLayout *_GetPrev();

    // Zuruecksetzen in die erste Zeile.
    void Init();
    void CtorInit( SwTxtFrm *pFrm, SwTxtInfo *pInf );
    inline SwTxtIter() { }

public:
    inline SwTxtIter( SwTxtFrm *pFrm, SwTxtInfo *pInf )
           { CtorInit( pFrm, pInf ); }
    inline const SwLineLayout *GetCurr() const { return pCurr; } // niemals 0!
    inline const SwLineLayout *GetNext() const { return pCurr->GetNext(); }
           const SwLineLayout *GetPrev();
    inline xub_StrLen GetLength() const { return pCurr->GetLen(); }
    inline MSHORT GetLineNr() const { return nLineNr; }
    inline xub_StrLen GetStart() const { return nStart; }
    inline xub_StrLen GetEnd() const { return GetStart() + GetLength(); }
    inline SwTwips Y() const { return nY; }

    inline SwTwips RegStart() const { return nRegStart; }
    inline KSHORT RegDiff() const { return nRegDiff; }
    inline sal_Bool IsRegisterOn() const { return bRegisterOn; }

    inline SwTxtInfo &GetInfo() { return *pInf; }
    inline const SwTxtInfo &GetInfo() const { return *pInf; }

    inline void Top() { Init(); }
    void Bottom();
    const SwLineLayout *Next();
    const SwLineLayout *Prev();

    void RecalcRealHeight();

    // Ueberspringt die Dummyzeilen der FlyFrms
    const SwLineLayout *NextLine();
    const SwLineLayout *PrevLine();
    const SwLineLayout *GetNextLine() const;
    const SwLineLayout *GetPrevLine();

    void CharToLine( const xub_StrLen );
    const SwLineLayout *TwipsToLine(const SwTwips);

    // schneidet ab pCurr alle ab.
    void TruncLines( sal_Bool bNoteFollow = sal_False );

    inline KSHORT GetLineHeight() const { return pCurr->GetRealHeight(); }
    void CalcRealHeight( sal_Bool bNewLine = sal_False );
    void CalcAscentAndHeight( KSHORT &rAscent, KSHORT &rHeight ) const;

    // 5298, viel Aerger durch die Abfrage auf pCurr == pPara
    inline sal_Bool IsFirstTxtLine() const
    { return nStart == GetInfo().GetTxtStart() &&
        !( pCurr->IsDummy() && GetNextLine() ); }

    // Als Ersatz fuer das alte IsFirstLine()
    inline sal_Bool IsParaLine() const
        { return pCurr == pInf->GetParaPortion(); }

    const SwLineInfo &GetLineInfo() const { return aLineInf; }
    inline const Point &GetFirstPos() const { return aTopLeft; }

    inline sal_Bool SeekAndChg( SwTxtSizeInfo &rInf );
    inline sal_Bool SeekAndChgBefore( SwTxtSizeInfo &rInf );
    inline sal_Bool SeekStartAndChg( SwTxtSizeInfo &rInf, const sal_Bool bPara=sal_False );

    inline SwTxtFrm *GetTxtFrm() { return pFrm; }
    inline const SwTxtFrm *GetTxtFrm() const { return pFrm; }

    // zaehlt aufeinanderfolgende Trennungen, um MaxHyphens einzuhalten
    void CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const;
};

/*************************************************************************
 *                      class SwTxtMargin
 *************************************************************************/

class SwTxtMargin : public SwTxtIter
{
private:
          SwTwips nLeft;
          SwTwips nRight;
          SwTwips nFirst;
          KSHORT  nDropLeft;
          KSHORT  nDropHeight;
          KSHORT  nDropDescent;
          MSHORT  nDropLines;
          MSHORT  nAdjust;

protected:
    // fuer FormatQuoVadis
    inline void Right( const KSHORT nNew ) { nRight = nNew; }
    // fuer CalcFlyAdjust
    inline void SetDropLeft( const KSHORT nNew ) { nDropLeft = nNew; }

    void CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf );
    inline SwTxtMargin() { }
public:
    inline SwTxtMargin( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf )
           { CtorInit( pFrm, pInf ); }
    inline SwTwips GetLeftMargin() const;
    inline SwTwips Left() const;
    inline SwTwips Right() const { return nRight; }
    inline SwTwips FirstLeft() const { return nFirst; }
    inline SwTwips CurrWidth() const { return pCurr->PrtWidth(); }
           SwTwips GetLineStart() const;
    inline SwTwips GetLineEnd() const { return GetLineStart() + CurrWidth(); }
    inline Point GetTopLeft() const { return Point( GetLineStart(), Y() ); }
    inline sal_Bool IsOneBlock() const { return bOneBlock; }
    inline sal_Bool IsLastBlock() const { return bLastBlock; }
    inline sal_Bool IsLastCenter() const { return bLastCenter; }
    inline MSHORT GetAdjust() const { return nAdjust; }
    inline KSHORT GetLineWidth() const
           { return KSHORT( Right() - GetLeftMargin() + 1 ); }
    inline SwTwips GetLeftMin() const { return nFirst < nLeft ? nFirst : nLeft; }
    inline sal_Bool HasNegFirst() const { return nFirst < nLeft; }

    // DropCaps
    inline MSHORT GetDropLines() const { return nDropLines; }
    inline void SetDropLines( const MSHORT nNew ) { nDropLines = nNew; }
    inline KSHORT GetDropLeft() const { return nDropLeft; }
    inline KSHORT GetDropHeight() const { return nDropHeight; }
    inline void SetDropHeight( const KSHORT nNew ) { nDropHeight = nNew; }
    inline KSHORT GetDropDescent() const { return nDropDescent; }
    inline void SetDropDescent( const KSHORT nNew ) { nDropDescent = nNew; }
    void DropInit();

    // liefert TxtPos fuer Start und Ende der aktuellen Zeile ohne whitespaces
    // In frminf.cxx implementiert.
    xub_StrLen GetTxtStart() const;
    xub_StrLen GetTxtEnd() const;

    inline SwTxtSizeInfo &GetInfo()
        { return (SwTxtSizeInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtSizeInfo &GetInfo() const
        { return (const SwTxtSizeInfo&)SwTxtIter::GetInfo(); }

};


/*************************************************************************
 *                      class SwTxtAdjuster
 *************************************************************************/

class SwTxtAdjuster : public SwTxtMargin
{
    // Gleicht die Portions aus, wenn Adjustment und FlyFrms vorliegen.
    void CalcFlyAdjust( SwLineLayout *pCurr );

    // spannt beim Blocksatz die Glues auf.
    void CalcNewBlock( SwLineLayout *pCurr, const SwLinePortion *pStopAt );
    // ruft SplitGlues und CalcBlockAdjust
    void FormatBlock( );

    // Erstellt bei kurzen Zeilen die Glue-Kette.
    SwMarginPortion *CalcRightMargin( SwLineLayout *pCurr );

    // Berechnung des Adjustments (FlyPortions)
    SwFlyPortion *CalcFlyPortion( const long nRealWidth,
                                  const SwRect &rCurrRect );

protected:
    inline SwTxtAdjuster() { }
public:
    inline SwTxtAdjuster( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf )
           { SwTxtMargin::CtorInit( pFrm, pInf ); }

    // wird von SwTxtFormatter wegen UpdatePos ueberladen
    void CalcAdjLine( SwLineLayout *pCurr );

    // sorgt fuer das nachtraegliche adjustieren
    inline void GetAdjusted() const
    {
        if( pCurr->IsFormatAdj() )
            ((SwTxtAdjuster*)this)->CalcAdjLine( pCurr );
    }

    // DropCaps-Extrawurst
    void CalcDropAdjust();
    void CalcDropRepaint();
};

/*************************************************************************
 *                      class SwTxtCursor
 *************************************************************************/

class SwTxtCursor : public SwTxtAdjuster
{
    // A small helper-class to save SwTxtCursor member, manipulate them
    // and to restore them
    friend class SwTxtCursorSave;

    // 1170: Mehrdeutigkeiten
    static sal_Bool bRightMargin;
protected:
    void CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf );
    inline SwTxtCursor() { }
public:
    inline SwTxtCursor( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf )
           { CtorInit( pFrm, pInf ); }
    sal_Bool GetCharRect(SwRect *, const xub_StrLen, SwCrsrMoveState* = 0,
        const long nMax = 0 );
    sal_Bool GetEndCharRect(SwRect *, const xub_StrLen, SwCrsrMoveState* = 0,
        const long nMax = 0 );
    xub_StrLen GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
                const MSHORT nChgNode, const SwCrsrMoveState* = 0 ) const;
    // 1170: beruecksichtigt Mehrdeutigkeiten; Implementierung s.u.
    const SwLineLayout *CharCrsrToLine( const xub_StrLen nPos );

    static inline void SetRightMargin( const sal_Bool bNew ){ bRightMargin = bNew; }
    static inline sal_Bool IsRightMargin() { return bRightMargin; }
};

/*************************************************************************
 *                          SwHookOut
 *************************************************************************/

class SwHookOut
{
    SwTxtSizeInfo *pInf;
public:
    inline SwHookOut( SwTxtSizeInfo *pInfo )
         : pInf(pInfo->OnWin() && pInfo->GetPrt() ? pInfo : 0)
    {
        if( pInf )
            pInf->SetPrtOut();
    }
    inline ~SwHookOut()
    {
        if( pInf )
            pInf->SetWinOut();
    }
};

/*************************************************************************
 *                      Inline-Implementierungen
 *************************************************************************/

inline sal_Bool SwTxtIter::SeekAndChg( SwTxtSizeInfo &rInf )
{
    return SwAttrIter::SeekAndChg( rInf.GetIdx(), rInf.GetOut() );
}

inline sal_Bool SwTxtIter::SeekAndChgBefore( SwTxtSizeInfo &rInf )
{
    if ( rInf.GetIdx() )
        return SwAttrIter::SeekAndChg( rInf.GetIdx()-1, rInf.GetOut() );
    else
        return SwAttrIter::SeekAndChg( rInf.GetIdx(), rInf.GetOut() );
}

inline sal_Bool SwTxtIter::SeekStartAndChg( SwTxtSizeInfo &rInf, const sal_Bool bPara )
{
    return SwAttrIter::SeekStartAndChg( rInf.GetOut(), bPara );
}

inline SwTwips SwTxtMargin::GetLeftMargin() const
{
    return IsFirstTxtLine() ? nFirst : Left();
}

inline SwTwips SwTxtMargin::Left() const
{
    return (nDropLines >= nLineNr && 1 != nLineNr) ? nFirst + nDropLeft : nLeft;
}



#endif
