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
    SwTxtFrm  *pFrm;
    SwTxtInfo *pInf;
    SwLineLayout *pCurr;
    SwLineLayout *pPrev;
    SwTwips nFrameStart;
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

    SwLineLayout *_GetPrev();

    // Zuruecksetzen in die erste Zeile.
    void Init();
    void CtorInitTxtIter( SwTxtFrm *pFrm, SwTxtInfo *pInf );
    inline SwTxtIter(SwTxtNode* pTxtNode) : SwAttrIter(pTxtNode) { }

public:
    inline SwTxtIter( SwTxtFrm *pTxtFrm, SwTxtInfo *pTxtInf ) : SwAttrIter(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtIter( pTxtFrm, pTxtInf ); }
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
    void CalcAscentAndHeight( KSHORT &rAscent, KSHORT &rHeight ) const;

    // 5298, viel Aerger durch die Abfrage auf pCurr == pPara
    inline sal_Bool IsFirstTxtLine() const
    { return nStart == GetInfo().GetTxtStart() &&
        !( pCurr->IsDummy() && GetNextLine() ); }

    // Als Ersatz fuer das alte IsFirstLine()
    inline sal_Bool IsParaLine() const
        { return pCurr == pInf->GetParaPortion(); }

    const SwLineInfo &GetLineInfo() const { return aLineInf; }
    inline SwTwips GetFirstPos() const { return nFrameStart; }
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
          // --> OD 2008-06-30 #i91133#
          SwTwips mnTabLeft;
          // <--

protected:
    // fuer FormatQuoVadis
    inline void Right( const SwTwips nNew ) { nRight = nNew; }
    // fuer CalcFlyAdjust
    inline void SetDropLeft( const KSHORT nNew ) { nDropLeft = nNew; }

    void CtorInitTxtMargin( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf );
    inline SwTxtMargin(SwTxtNode* pTxtNode) : SwTxtIter(pTxtNode) { }
public:
    inline SwTxtMargin( SwTxtFrm *pTxtFrm, SwTxtSizeInfo *pTxtSizeInf ) : SwTxtIter(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtMargin( pTxtFrm, pTxtSizeInf ); }
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

    // --> OD 2008-06-30 #i91133#
    inline SwTwips GetTabLeft() const
    {
        return mnTabLeft;
    }
    // <--
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

    // ruft SplitGlues und CalcBlockAdjust
    void FormatBlock( );

    // Erstellt bei kurzen Zeilen die Glue-Kette.
    SwMarginPortion* CalcRightMargin( SwLineLayout *pCurr, SwTwips nReal = 0 );

    // Berechnung des Adjustments (FlyPortions)
    SwFlyPortion *CalcFlyPortion( const long nRealWidth,
                                  const SwRect &rCurrRect );

protected:
    inline SwTxtAdjuster(SwTxtNode* pTxtNode) : SwTxtMargin(pTxtNode) { }
    // spannt beim Blocksatz die Glues auf.
    void CalcNewBlock( SwLineLayout *pCurr, const SwLinePortion *pStopAt,
        SwTwips nReal = 0, bool bSkipKashida = false );
    SwTwips CalcKanaAdj( SwLineLayout *pCurr );
public:
    inline SwTxtAdjuster( SwTxtFrm *pTxtFrm, SwTxtSizeInfo *pTxtSizeInf ) : SwTxtMargin(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtMargin( pTxtFrm, pTxtSizeInf ); }

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
    void _GetCharRect(SwRect *, const xub_StrLen, SwCrsrMoveState* );
protected:
    void CtorInitTxtCursor( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf );
    inline SwTxtCursor(SwTxtNode* pTxtNode) : SwTxtAdjuster(pTxtNode) { }
public:
    inline SwTxtCursor( SwTxtFrm *pTxtFrm, SwTxtSizeInfo *pTxtSizeInf ) : SwTxtAdjuster(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtCursor( pTxtFrm, pTxtSizeInf ); }
    sal_Bool GetCharRect(SwRect *, const xub_StrLen, SwCrsrMoveState* = 0,
        const long nMax = 0 );
    sal_Bool GetEndCharRect(SwRect *, const xub_StrLen, SwCrsrMoveState* = 0,
        const long nMax = 0 );
    xub_StrLen GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
                const MSHORT nChgNode, SwCrsrMoveState* = 0 ) const;
    // 1170: beruecksichtigt Mehrdeutigkeiten; Implementierung s.u.
    const SwLineLayout *CharCrsrToLine( const xub_StrLen nPos );

    // calculates baseline for portion rPor
    // bAutoToCentered indicates, if AUTOMATIC mode means CENTERED or BASELINE
    sal_uInt16 AdjustBaseLine( const SwLineLayout& rLine, const SwLinePortion* pPor,
                           sal_uInt16 nPorHeight = 0, sal_uInt16 nAscent = 0,
                           const sal_Bool bAutoToCentered = sal_False ) const;

    static inline void SetRightMargin( const sal_Bool bNew ){ bRightMargin = bNew; }
    static inline sal_Bool IsRightMargin() { return bRightMargin; }
};

/*************************************************************************
 *                          SwHookOut
 *
 * Change current output device to printer, this has to be done before
 * formatting.
 *************************************************************************/

class SwHookOut
{
    SwTxtSizeInfo* pInf;
    OutputDevice* pOut;
    sal_Bool bOnWin;
public:
    SwHookOut( SwTxtSizeInfo& rInfo );
    ~SwHookOut();
};

/*************************************************************************
 *                      Inline-Implementierungen
 *************************************************************************/

inline sal_Bool SwTxtIter::SeekAndChg( SwTxtSizeInfo &rInf )
{
    return SeekAndChgAttrIter( rInf.GetIdx(), rInf.GetOut() );
}

inline sal_Bool SwTxtIter::SeekAndChgBefore( SwTxtSizeInfo &rInf )
{
    if ( rInf.GetIdx() )
        return SeekAndChgAttrIter( rInf.GetIdx()-1, rInf.GetOut() );
    else
        return SeekAndChgAttrIter( rInf.GetIdx(), rInf.GetOut() );
}

inline sal_Bool SwTxtIter::SeekStartAndChg( SwTxtSizeInfo &rInf, const sal_Bool bPara )
{
    return SeekStartAndChgAttrIter( rInf.GetOut(), bPara );
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
