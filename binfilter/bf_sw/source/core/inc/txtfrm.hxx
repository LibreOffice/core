/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _TXTFRM_HXX
#define _TXTFRM_HXX

#include <tools/mempool.hxx>
#include <tools/string.hxx>
#include "cntfrm.hxx"
/*N*/ #include <tools/debug.hxx> //for stripping
class OutputDevice; 
namespace binfilter {
#define STRSIZE(x) (sizeof(x)-1)

class SwCharRange;
class SwTxtNode;
class SwTxtFormatter;
class SwTxtFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class SwBodyFrm;
class SwFooterFrm;
class SwTxtFtn;
class SwInterHyphInfo;		// Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrmFmt;
class SwFldPortion;

class SwTestFormat;
class SwStripes;
struct SwCrsrMoveState;
struct SwFillData;
class SwPortionHandler;
class SwScriptInfo;

#define GRID_ON         0
#define GRID_HEIGHT     1
#define RUBY_HEIGHT     2
#define RUBY_TOP        3
#define GRID_CELLS      4

class SwTxtFrm: public SwCntntFrm
{
    friend class SwTxtIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class SwTxtFrmLocker;		// duerfen Lock()/Unlock()

    static SwCache *pTxtCache;	//Pointer auf den Line-Cache
    static long nMinPrtLine; 	//Diese Linie darf beim Drucken nicht
        //unterschritten werden, Hack fuer Tabellenzellen ueber mehrere Seiten

    ULONG  nAllLines		:24;//Anzahl der Zeilen fuer das Paint (inkl. nThisLines)
    ULONG  nThisLines		:8;	//Anzahl der Zeilen dieses Frames

    // The x position for flys anchored at this paragraph.
    // These values are calculated in SwTxtFrm::CalcBaseOfstForFly()
    SwTwips mnFlyAnchorOfst;
    // The x position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorOfstNoWrap;

    xub_StrLen nOfst;			//nOfst gibt den Offset im Cntnt (Anzahl Zeichen) an.

    USHORT nCacheIdx;			//Index in den Cache, USHRT_MAX wenn definitiv
                                //kein passendes Objekt im Cache steht.

    //Teilt den Master ab und erzeugt einen Follow oder passt die
    //Daten im Follow an.
           void _AdjustFollow( SwTxtFormatter &rLine, const xub_StrLen nOffset,
                               const xub_StrLen nStrEnd, const sal_uInt8 nMode );
    inline void AdjustFollow( SwTxtFormatter &rLine, const xub_StrLen nOffset,
                              const xub_StrLen nStrEnd, const sal_uInt8 nMode );

    //Iteriert ueber alle Zeilen und stellt das Linespacing
    //entsprechend dem Attribut ein.
    void CalcLineSpace();

    void InitCtor();		// Wird in beiden Ctoren gerufen

    // Wird nur in Format gerufen:
    void AdjustFrm( const SwTwips nChgHeight, sal_Bool bHasToFit = sal_False );

    // wertet in Format() die Preps aus.
    sal_Bool CalcPreps();
    void PrepWidows( const USHORT nNeed, sal_Bool bNotify = sal_True );
    void _InvalidateRange( const SwCharRange &, const long = 0);
    inline void InvalidateRange( const SwCharRange &, const long = 0);

    // WidowsAndOrphans, AdjustFrm, AdjustFollow
    void FormatAdjust( SwTxtFormatter &rLine, WidowsAndOrphans &rFrmBreak,
                       const xub_StrLen nStrLen, const sal_Bool bDummy );

    sal_Bool bLocked		: 1;		// im Format?
    sal_Bool bFormatted 	: 1;		// nach Format auf sal_True
    sal_Bool bWidow			: 1;		// sind wir ein Widow
    sal_Bool bJustWidow		: 1;		// haben wir soeben Widow angefordert
    sal_Bool bEmpty			: 1;		// sind wir ein leerer Absatz
    sal_Bool bInFtnConnect	: 1;		// Steht gerade im Connect
    sal_Bool bFtn			: 1;		// Hat mindestens eine Fussnote
    sal_Bool bRepaint		: 1;		// TxtFrm: Repaint steht zur Abholung bereit
    sal_Bool bBlinkPor		: 1;		// enthaelt Blink-Portions
    sal_Bool bFieldFollow	: 1;		// beginne mit Feldrest des Masters
    sal_Bool bHasAnimation	: 1;		// enthaelt animierte SwGrfNumPortion
    sal_Bool bIsSwapped     : 1;        // during text formatting we swap the
                                        // width and height for vertical formatting
    // OD 14.03.2003 #i11760# - flag to control, if follow is formatted in
    // method <CalcFollow(..)>.
    // E.g., avoid formatting of follow, if method <SwLayoutFrm::FormatWidthCols(..)>
    // is running.
    sal_Bool mbFollowFormatAllowed : 1;

    void ResetPreps();
    inline void Lock() { bLocked = sal_True; }
    inline void Unlock() { bLocked = sal_False; }
    inline void SetFormatted( const sal_Bool bNew ) { bFormatted = bNew; }
    inline void SetWidow( const sal_Bool bNew ) { bWidow = bNew; }
    inline void SetJustWidow( const sal_Bool bNew ) { bJustWidow = bNew; }
    inline void SetEmpty( const sal_Bool bNew ) { bEmpty = bNew; }
    inline void SetFieldFollow( const sal_Bool bNew ) { bFieldFollow = bNew; }

    sal_Bool IsIdxInside( const xub_StrLen nPos, const xub_StrLen nLen ) const;

    // Wechselt den Frame oder auch nicht (vgl. FlyCnt)
    sal_Bool _GetCrsrOfst(SwPosition *pPos, const Point &rPoint,
                      const sal_Bool bChgFrm, const SwCrsrMoveState* = 0 ) const;

    // formatiert genau eine Zeile ...
    sal_Bool FormatLine( SwTxtFormatter &rLine, const sal_Bool bPrev );

    // Um Stack einzusparen aufgeteilt ...
    // _Format ruft _Format mit Parametern
    void _Format( SwParaPortion *pPara );
    void _Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                  const sal_Bool bAdjust = sal_False );
    void FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf );

    // formatiert den Follow und sorgt fuer die Entsorgung bei Orphans
    sal_Bool CalcFollow(  const xub_StrLen nTxtOfst );

    // korrigiert die Stelle ab der formatiert werden muss.
    xub_StrLen FindBrk(const String &rTxt, const xub_StrLen nStart,
                                       const xub_StrLen nEnd) const;

    // inline-Weiche
    SwTwips _GetFtnFrmHeight() const;

    // Aus CalcPreps ausgelagert.
    sal_Bool CalcPrepFtnAdjust();

    // Fuer Ftn und WidOrp: Zwangsvalidierung
    void ValidateFrm();
    void ValidateBodyFrm();


    void SetPara( SwParaPortion *pNew, sal_Bool bDelete = sal_True );

    sal_Bool _IsFtnNumFrm() const;

    // 6995: Formatinformationen auffrischen
    sal_Bool FormatQuick();

    // Opt: Leere Absaetze formatieren
    sal_Bool FormatEmpty();
    SwTwips EmptyHeight() const;
    // Opt: Leere Absaetze painten

    void ChgThisLines();//Muss immer gerufen werden, wenn sich die Zeilenazahl
                        //veraendert haben kann.

        // required for 'new' relative anchor position
    void CalcBaseOfstForFly();

public:

    //public, weil der eine oder andere die Methode rufen darf um das
    //Prepare zu sparen - mit Vorsicht zu geniessen!
    void Init();

    // Wird von FormatSpelling( ) gerufen
    SwRect _AutoSpell( SwCntntNode* , USHORT );
    // Wird vom CollectAutoCmplWords gerufen
    void CollectAutoCmplWrds( SwCntntNode* , USHORT , sal_Bool bIsVisArea );

    // Returns the screen position of rPos. The values are relative to the upper
    // left position of the page frame.
    // Additional information can be obtained by passing an SwCrsrMoveState object.
    // Returns sal_False if rPos > number of character is string
    virtual sal_Bool   GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCrsrMoveState* pCMS = 0 ) const;
    // Eine etwas abgespeckte GetCharRect-Version fuer autopositionierte Rahmen
    sal_Bool GetAutoPos( SwRect &, const SwPosition& ) const;

    //Liefert in nOffset den Offset des Characters innerhalb des
    //gesetzten Textbuffers zurueck, welcher der durch aPoint
    //gegebenen Position innerhalb der SSize des Layout am
    //naechsten ist. Wenn der SPoint ausserhalb der SSize liegt,
    //liefert die Funktion sal_False, sal_True sonst.
    virtual sal_Bool GetCrsrOfst( SwPosition *, Point&,
                              const SwCrsrMoveState* = 0) const;

    // GetKeyCrsrOfst sorgt dafuer, dass der Frame nicht gewechselt wird
    // (z.B. Wechsel in den zeichengebundenen Frame).
    inline	sal_Bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, sal_False ); }

    SwRect Paint();
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual sal_Bool GetInfo( SfxPoolItem & ) const;

    //Layoutorientiertes Cursortravelling: Linker, rechter Rand,
    //vorhergehende/naechste Zeile, gleiche horizontale Position.


    // Prepares the cursor position for a visual cursor move (BiDi).
    // The behaviour is different for insert and overwrite cursors
    void PrepareVisualMove( xub_StrLen& nPos, BYTE& nCrsrLevel,
                            sal_Bool& bRight, sal_Bool bInsertCrsr );

    // Methoden zur Verwaltung von FolgeFrames
           SwCntntFrm *SplitFrm( const xub_StrLen nTxtPos );
           SwCntntFrm *JoinFrm();
    inline USHORT	   GetOfst() const { return nOfst; }
           void		   _SetOfst( const USHORT nNewOfst );
    inline void 	   SetOfst ( const USHORT nNewOfst );
    inline void 	   ManipOfst ( const USHORT nNewOfst ){ nOfst = nNewOfst; }
           SwTxtFrm   *GetFrmAtPos ( const SwPosition &rPos);
    inline const SwTxtFrm *GetFrmAtPos ( const SwPosition &rPos) const;
            SwTxtFrm   *GetFrmAtOfst( const xub_StrLen nOfst );
    // Wenn es einen Follow gibt und wir selbst keinen Text enthalten:
    inline sal_Bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    // Liefert den zu bearbeitenden Textausschnitt zurueck (inline, s.u.)
    const String& GetTxt() const;
    inline SwTxtNode *GetTxtNode()
        { return (SwTxtNode*)SwCntntFrm::GetNode(); }
    inline const SwTxtNode *GetTxtNode() const
        { return (SwTxtNode*)SwCntntFrm::GetNode(); }

    SwTxtFrm(SwTxtNode * const);
    inline ~SwTxtFrm() { if( HasAnimation() ) ClearPara(); }

    // SwCntntFrm: der "kurze Dienstweg" fuer die Frames.
    // Wer den void* falsch casted ist selbst Schuld!
    // Auf jedenfall muss der void* auf 0 geprueft werden.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

    //nMaxHeight liefert die benoetigte Hoehe,
    //bSplit sagt, obj der Absatz gesplittet werden muss.
    virtual sal_Bool WouldFit( SwTwips &nMaxHeight, sal_Bool &bSplit );

    // Hier das WouldFit-Aequivalent fuer mal kurz probeweise
    // umgehaengte TextFrames, auch hier liefert
    // nMaxHeight die benoetigte Hoehe,
    // und bSplit sagt, obj der Absatz gesplittet werden muss.
    // Uebergeben wird der potentielle Vorgaenger fuer die Abstandsberechnung
    sal_Bool TestFormat( const SwFrm* pPrv, SwTwips &nMaxHeight, sal_Bool &bSplit );

    // Wir formatieren eine Zeile fuer die interaktive Trennung
    // Return: found

    // Probegrow
    inline SwTwips GrowTst( const SwTwips nGrow );

    SwParaPortion *GetPara();
    inline const SwParaPortion *GetPara() const;
    inline sal_Bool HasPara() const;
    sal_Bool _HasPara() const;
    // If there are any hanging punctuation portions in the margin
    // the offset will be returned.
    SwTwips HangingMargin() const;

    // RTTI
    TYPEINFO();
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtFrm)

    // Locking
    inline sal_Bool IsLocked() 		const { return bLocked;		}
    inline sal_Bool IsFormatted() 	const { return bFormatted;	}

    inline sal_Bool IsWidow() 		const { return bWidow;		}
    inline sal_Bool IsJustWidow()	const { return bJustWidow;	}
    inline sal_Bool IsEmpty() 		const { return bEmpty;		}
    inline sal_Bool HasFtn() 		const { return bFtn; 		}
    inline sal_Bool IsInFtnConnect()const { return bInFtnConnect;}
    inline sal_Bool IsFieldFollow() const { return bFieldFollow;}

    inline void SetRepaint() const;
    inline void ResetRepaint() const;
    inline sal_Bool HasRepaint() const { return bRepaint; }
    inline void SetBlinkPor() const;
    inline void ResetBlinkPor() const;
    inline sal_Bool HasBlinkPor() const { return bBlinkPor; }
    inline void SetAnimation() const
        { ( (SwTxtFrm*)this )->bHasAnimation = sal_True; }
    inline sal_Bool HasAnimation() const { return bHasAnimation; }

    inline sal_Bool IsSwapped() const { return bIsSwapped; }

    // Hat der Frm eine lokale Fussnote (in diesem Frm bzw. Follow)?
#ifndef DBG_UTIL
    void CalcFtnFlag();
#else
    void CalcFtnFlag( xub_StrLen nStop = STRING_LEN );//Fuer den Test von SplitFrm
#endif

    // Hidden
    sal_Bool IsHiddenNow() const;		// bHidden && pOut == pPrt
    void HideHidden();				// Anhaengsel entfernen wenn Hidden

    // Ftn
    void RemoveFtn( const xub_StrLen nStart = 0,
                    const xub_StrLen nLen = STRING_LEN );
    inline SwTwips GetFtnFrmHeight() const;
    SwTxtFrm *FindFtnRef( const SwTxtFtn *pFtn );
    inline const SwTxtFrm *FindFtnRef( const SwTxtFtn *pFtn ) const
    { return FindFtnRef( pFtn ); }
    void ConnectFtn( SwTxtFtn *pFtn, const SwTwips nDeadLine );

    // Wenn wir eine Ftn sind, die auf ihre Referenz zu waechst...
    // public weil von SwCntntFrm::MakeAll benoetigt.
    SwTwips GetFtnLine( const SwTxtFtn *pFtn, sal_Bool bLocked ) const;

    // Liefern den linken und den rechten Rand in
    // Dokumentkoordinaten (unter Beachtung der Absatzattribute).
    inline SwTwips GetLeftMargin() const;
    inline SwTwips GetRightMargin() const;

    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void  CheckDirection( BOOL bVert );

    // Liefert die Summe der Zeilenhoehen in pLine zurueck.
    USHORT GetParHeight() const;

    // Liefert die Resthoehe zurueck
    inline SwTwips GetRstHeight() const;

    inline 		 SwTxtFrm *GetFollow();
    inline const SwTxtFrm *GetFollow() const;
    inline 		 SwTxtFrm *FindMaster();
    inline const SwTxtFrm *FindMaster() const;

    // Suche die Seitennummer von ErgoSum und QuoVadis
    SwTxtFrm *FindQuoVadisFrm();

    // holt die Formatierug nach, wenn der Idle-Handler zugeschlagen hat.
    SwTxtFrm *GetFormatted();

    // wird demnaechst uebertragen
    inline void SetFtn( const sal_Bool bNew ) { bFtn = bNew; }

    // Beruecksichtigung der Follows
    inline sal_Bool IsInside( const xub_StrLen nPos ) const;

    const SwBodyFrm   *FindBodyFrm()   const;

    // DropCaps und Selektionen

    static SwCache *GetTxtCache() { return pTxtCache; }
    static void		SetTxtCache( SwCache *pNew ) { pTxtCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }
    static void	SetMinPrtLine( long nNew ) { nMinPrtLine = nNew; }

    inline USHORT GetCacheIdx() const { return nCacheIdx; }
    inline void	  SetCacheIdx( const USHORT nNew ) { nCacheIdx = nNew; }

    //Entfert die Line-Informationen aus dem Cache.
    void ClearPara();

    // Bin ich ein FtnFrm, der eine Nummer am Absatzanfang hat?
    inline sal_Bool IsFtnNumFrm() const
    { return IsInFtn() && !GetIndPrev() && _IsFtnNumFrm(); }

    // simuliert eine Formatierung, als wenn es keinen rechten Rand und
    // keine Flys oder andere Hindernisse gaebe und liefert die Breite.

    // liefert den zusaetzlichen Zeilenabstand fuer den naechsten Absatz
    USHORT GetLineSpace() const;

    // liefert die erste Zeilenhoehe zurueck
    USHORT FirstLineHeight() const;

    const SwFrmFmt* IsFirstBullet();

    // Haengt FlyInCntFrm um, wenn nEnd > Index >= nStart ist.
    void MoveFlyInCnt( SwTxtFrm *pNew, USHORT nStart, USHORT nEnd );

    // Berechnet die Position von FlyInCntFrms
    USHORT CalcFlyPos( SwFrmFmt* pSearch );

    // Ermittelt die Startposition und Schrittweite des Registers
    sal_Bool FillRegister( SwTwips& rRegStart, USHORT& rRegDiff );



    //Fuer die Anzeige der Zeilennummern.
    ULONG GetAllLines()  const { return nAllLines; }
    ULONG GetThisLines() const { return nThisLines;}
    void RecalcAllLines();

    // Stoppt Animationen innerhalb von Numerierungen


    // visit all portions for Accessibility

    // returns the script info stored at the paraportion
    const SwScriptInfo* GetScriptInfo() const;

    // Swaps width and height of the text frame
    void SwapWidthAndHeight();
    // Calculates the coordinates of a rectangle when switching from
    // horizontal to vertical layout.
        void SwitchHorizontalToVertical( SwRect& rRect ) const{DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001     void SwitchHorizontalToVertical( SwRect& rRect ) const;
    // Calculates the coordinates of a point when switching from
    // horizontal to vertical layout.
        void SwitchHorizontalToVertical( Point& rPoint ) const{DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001     void SwitchHorizontalToVertical( Point& rPoint ) const;
    // Calculates the a limit value when switching from
    // horizontal to vertical layout.
    long SwitchHorizontalToVertical( long nLimit ) const;
    // Calculates the coordinates of a rectangle when switching from
    // vertical to horizontal layout.
    void SwitchVerticalToHorizontal( SwRect& rRect ) const;
    // Calculates the coordinates of a point when switching from
    // vertical to horizontal layout.
    void SwitchVerticalToHorizontal( Point& rPoint ) const;

#ifdef BIDI
    // Calculates the coordinates of a point when switching from
    // LTR to RTL layout.
    void SwitchLTRtoRTL( Point& rPoint ) const;
    // Calculates the coordinates of a point when switching from
    // RTL to LTR layout.
    inline void SwitchRTLtoLTR( Point& rPoint ) const { SwitchLTRtoRTL( rPoint ); };

#endif

    // OD 14.03.2003 #i11760# - access to new member <mbNoFollowFormat>
    inline bool FollowFormatAllowed() const
    {
        return mbFollowFormatAllowed;
    }
    inline void AllowFollowFormat()
    {
        mbFollowFormatAllowed = true;
    }
    inline void ForbidFollowFormat()
    {
        mbFollowFormatAllowed = false;
    }
    
    SwTwips GetBaseOfstForFly( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const
    {
        return ( bIgnoreFlysAnchoredAtThisFrame ?
                 mnFlyAnchorOfst :
                 mnFlyAnchorOfstNoWrap );
    }
};

/*************************************************************************
 *							class SwTxtFrmLocker
 *************************************************************************/

class SwTxtFrmLocker
{
private:
    SwTxtFrm * const pFrm;
public:
    inline SwTxtFrmLocker( SwTxtFrm *pTxtFrm )
        : pFrm( pTxtFrm->IsLocked() ? 0 : pTxtFrm )
    { if( pFrm ) pFrm->Lock(); }
    inline ~SwTxtFrmLocker() { if( pFrm ) pFrm->Unlock(); }
};

/*************************************************************************
 *						Inline-Implementierung
 *************************************************************************/

inline const SwParaPortion *SwTxtFrm::GetPara() const
{
    return ((SwTxtFrm*)this)->GetPara();
}

inline sal_Bool SwTxtFrm::HasPara() const
{
    return nCacheIdx != USHRT_MAX ? _HasPara() : sal_False;
}

// 9104: Frm().Height() - Prt().Height(), siehe widorp.cxx und 7455, 6114, 7908
inline SwTwips SwTxtFrm::GetRstHeight() const
{
    return !GetUpper() ? 0 : ((const SwFrm*)GetUpper())->Frm().Top()
                           + ((const SwFrm*)GetUpper())->Prt().Top()
                           + ((const SwFrm*)GetUpper())->Prt().Height()
                           - Frm().Top() - (Frm().Height() - Prt().Height());
}

inline SwTwips SwTxtFrm::GetLeftMargin() const
{
    return Frm().Left() + Prt().Left();
}
inline SwTwips SwTxtFrm::GetRightMargin() const
{
    return Frm().Left() + Prt().Left() + Prt().Width();
}
inline SwTwips SwTxtFrm::GrowTst( const SwTwips nGrow )
{
    return Grow( nGrow, sal_True );
}

#if OSL_DEBUG_LEVEL > 1
// fragt auf WYSIWYG DBG ab
extern sal_Bool IsDbg( const SwTxtFrm *pFrm );
#define DBTXTFRM aDbstream << "SwTxtFrm[" << GetFrmId() << "]"
#endif

inline sal_Bool SwTxtFrm::IsInside( const xub_StrLen nPos ) const
{
    sal_Bool bRet = sal_True;
    if( nPos < GetOfst() )
        bRet = sal_False;
    else
    {
        const SwTxtFrm *pFoll = GetFollow();
        if( pFoll && nPos >= pFoll->GetOfst() )
            bRet = sal_False;
    }
    return bRet;
}

inline SwTwips SwTxtFrm::GetFtnFrmHeight() const
{
    if(  !IsFollow() && IsInFtn() && HasPara() )
        return _GetFtnFrmHeight();
    else
        return 0;
}

inline SwTxtFrm *SwTxtFrm::FindMaster()
{
    return (SwTxtFrm*)SwFlowFrm::FindMaster();
}
inline const SwTxtFrm *SwTxtFrm::FindMaster() const
{
    return (const SwTxtFrm*)SwFlowFrm::FindMaster();
}
inline const SwTxtFrm *SwTxtFrm::GetFollow() const
{
    return (const SwTxtFrm*)SwCntntFrm::GetFollow();
}
inline SwTxtFrm *SwTxtFrm::GetFollow()
{
    return (SwTxtFrm*)SwCntntFrm::GetFollow();
}

inline const SwTxtFrm *SwTxtFrm::GetFrmAtPos( const SwPosition &rPos) const
{
    return ((SwTxtFrm*)this)->GetFrmAtPos( rPos );
}

inline void SwTxtFrm::AdjustFollow( SwTxtFormatter &rLine,
    const xub_StrLen nOffset, const xub_StrLen nStrEnd, const sal_uInt8 nMode )
{
    if ( HasFollow() )
        _AdjustFollow( rLine, nOffset, nStrEnd, nMode );
}

inline void SwTxtFrm::SetOfst( const xub_StrLen nNewOfst )
{
    if ( nOfst != nNewOfst )
        _SetOfst( nNewOfst );
}

inline void SwTxtFrm::SetRepaint() const
{
    ((SwTxtFrm*)this)->bRepaint = sal_True;
}
inline void SwTxtFrm::ResetRepaint() const
{
    ((SwTxtFrm*)this)->bRepaint = sal_False;
}

inline void SwTxtFrm::SetBlinkPor() const
{
    ((SwTxtFrm*)this)->bBlinkPor = sal_True;
}
inline void SwTxtFrm::ResetBlinkPor() const
{
    ((SwTxtFrm*)this)->bBlinkPor = sal_False;
}

#ifdef LINGU_STATISTIK

class SwLinguStatistik
{
public:
    long nWords;	// gepruefte Worte
    long nFlushCnt;	// zaehlt die Messungen

    long nWrong;  // als falsch erkannt
    long nAlter;  // Alternativvorschlaege
    long nSpellTime; // Zeitmessung
    long nSynonym; // Thesaurus
    long nNoSynonym; // Thesaurus ratlos
    long nMeaning; // Thesaurus-Bedeutung
    long nNoMeaning; // Thesaurus meinungslos
    long nTheTime; // Zeitmessung
    long nHyphens; // Trennstellen
    long nNoHyph; // Worte ohne Trennstellen
    long nHyphErr; // Fehler beim Trennen
    long nHyphTime; // Zeitmessung
    SpellCheck *pSpell;
    LanguageType eLang;

    void Flush();

    inline SwLinguStatistik()
        { nWords = nWrong = nAlter = nSynonym = nNoSynonym =
          nHyphens = nNoHyph = nHyphErr = nSpellTime = nTheTime =
          nHyphTime = nFlushCnt = 0;
          pSpell = NULL;
          eLang = LANGUAGE_DONTKNOW; }
    inline ~SwLinguStatistik(){ Flush(); }
};

// globale Variable, implementiert in txtfrm.cxx
extern SwLinguStatistik aSwLinguStat;

#define SW_LING(nWhich,nInc) (aSwLinguStat.nWhich) += nInc;

#endif

#define SWAP_IF_SWAPPED( pFrm )\
    sal_Bool bUndoSwap = sal_False;   \
    if ( pFrm->IsVertical() && pFrm->IsSwapped() )\
    {                                 \
        bUndoSwap = sal_True;         \
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();         \
    }

#define SWAP_IF_NOT_SWAPPED( pFrm )\
    sal_Bool bUndoSwap = sal_False;     \
    if ( pFrm->IsVertical() && ! pFrm->IsSwapped() )\
    {                                   \
        bUndoSwap = sal_True;           \
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();         \
    }

#define UNDO_SWAP( pFrm )\
    if ( bUndoSwap )\
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();

// Helper class which can be used instead of the macros if a function
// has too many returns
class SwFrmSwapper
{
    const SwTxtFrm* pFrm;
    sal_Bool bUndo;
public:
    SwFrmSwapper( const SwTxtFrm* pFrm, sal_Bool bSwapIfNotSwapped );
    ~SwFrmSwapper();
};

#ifdef BIDI

class SwLayoutModeModifier
{
    const OutputDevice& rOut;
    long nOldLayoutMode;
public:
    SwLayoutModeModifier( const OutputDevice& rOutp );
    ~SwLayoutModeModifier();
    void SetAuto();
};

#endif

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
