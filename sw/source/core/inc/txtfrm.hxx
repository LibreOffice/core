/*************************************************************************
 *
 *  $RCSfile: txtfrm.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _TXTFRM_HXX
#define _TXTFRM_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#include "cntfrm.hxx"

#define STRSIZE(x) (sizeof(x)-1)

class SwCharRange;
class SwTxtNode;
class SwTxtFormatter;
class SwTxtFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class Sw3FrameIo;
class SwBodyFrm;
class SwFooterFrm;
class SwTxtFtn;
class SwInterHyphInfo;      // Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrmFmt;
class SwFldPortion;
class OutputDevice;
class SwTestFormat;
class SwStripes;
struct SwCrsrMoveState;
struct SwFillData;

class SwTxtFrm: public SwCntntFrm
{
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class SwTxtFrmLocker;        // duerfen Lock()/Unlock()
    friend sal_Bool lcl_ChangeOffset( SwTxtFrm* pFrm, USHORT nNew );


    static SwCache *pTxtCache;  //Pointer auf den Line-Cache
    static long nMinPrtLine;    //Diese Linie darf beim Drucken nicht
        //unterschritten werden, Hack fuer Tabellenzellen ueber mehrere Seiten

    ULONG  nAllLines        :24;//Anzahl der Zeilen fuer das Paint (inkl. nThisLines)
    ULONG  nThisLines       :8; //Anzahl der Zeilen dieses Frames

    xub_StrLen nOfst;           //nOfst gibt den Offset im Cntnt (Anzahl Zeichen) an.

    USHORT nCacheIdx;           //Index in den Cache, USHRT_MAX wenn definitiv
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

    void InitCtor();        // Wird in beiden Ctoren gerufen

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

    sal_Bool bLocked        : 1;        // im Format?
    sal_Bool bFormatted     : 1;        // nach Format auf sal_True
    sal_Bool bWidow         : 1;        // sind wir ein Widow
    sal_Bool bJustWidow     : 1;        // haben wir soeben Widow angefordert
    sal_Bool bEmpty         : 1;        // sind wir ein leerer Absatz
    sal_Bool bInFtnConnect  : 1;        // Steht gerade im Connect
    sal_Bool bFtn           : 1;        // Hat mindestens eine Fussnote
    sal_Bool bRepaint       : 1;        // TxtFrm: Repaint steht zur Abholung bereit
    sal_Bool bBlinkPor      : 1;        // enthaelt Blink-Portions
    sal_Bool bFieldFollow   : 1;        // beginne mit Feldrest des Masters
    sal_Bool bHasAnimation  : 1;        // enthaelt animierte SwGrfNumPortion

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
    void FillCrsrPos( SwFillData &rFill ) const;

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

    sal_Bool _GetDropRect( SwRect &rRect ) const;

    void SetPara( SwParaPortion *pNew, sal_Bool bDelete = sal_True );

    sal_Bool _IsFtnNumFrm() const;

    // 6995: Formatinformationen auffrischen
    sal_Bool FormatQuick();

    // Opt: Leere Absaetze formatieren
    sal_Bool FormatEmpty();
    SwTwips EmptyHeight() const;
    // Opt: Leere Absaetze painten
    sal_Bool PaintEmpty( const SwRect &, sal_Bool bCheck ) const;

    void ChgThisLines();//Muss immer gerufen werden, wenn sich die Zeilenazahl
                        //veraendert haben kann.
public:

    //public, weil der eine oder andere die Methode rufen darf um das
    //Prepare zu sparen - mit Vorsicht zu geniessen!
    void Init();

    // Wird von FormatSpelling( ) gerufen
    SwRect _AutoSpell( SwCntntNode* , USHORT );
    // Wird vom CollectAutoCmplWords gerufen
    void CollectAutoCmplWrds( SwCntntNode* , USHORT , sal_Bool bIsVisArea );

    //Liefert in pPoint den X- und Y-Offset der linken, oberen
    //Ecke eines Characters innerhalb der SSize des
    //Paragraph-Layout (nicht absolut oder Window-relativ)
    //zurueck. Wenn nOffset groesser ist als die Anzahl der
    //Character im Textbuffer, liefert die Funktion sal_False, sal_True
    //sonst.
    virtual sal_Bool   GetCharRect( SwRect &, const SwPosition&,
                                SwCrsrMoveState* = 0) const;
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
    inline  sal_Bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, sal_False ); }

    void   PaintExtraData( const SwRect & rRect ) const; //Seitennummer usw.
    SwRect Paint();
    virtual void Paint( const SwRect & ) const;
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
    virtual sal_Bool GetInfo( SfxPoolItem & ) const;

    //Layoutorientiertes Cursortravelling: Linker, rechter Rand,
    //vorhergehende/naechste Zeile, gleiche horizontale Position.
    virtual sal_Bool LeftMargin(SwPaM *) const;
    virtual sal_Bool RightMargin(SwPaM *, sal_Bool bAPI = sal_False) const;

    virtual sal_Bool UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False  ) const;
    virtual sal_Bool UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False ) const;
    sal_Bool _UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False ) const;
    sal_Bool _UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            sal_Bool bSetInReadOnly = sal_False ) const;

    // Methoden zur Verwaltung von FolgeFrames
           SwCntntFrm *SplitFrm( const xub_StrLen nTxtPos );
           SwCntntFrm *JoinFrm();
    inline USHORT      GetOfst() const { return nOfst; }
           void        _SetOfst( const USHORT nNewOfst );
    inline void        SetOfst ( const USHORT nNewOfst );
    inline void        ManipOfst ( const USHORT nNewOfst ){ nOfst = nNewOfst; }
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
    SwTxtFrm( Sw3FrameIo&, SwLayoutFrm* );
    inline ~SwTxtFrm() { if( HasAnimation() ) ClearPara(); }

    virtual void Store( Sw3FrameIo& ) const;

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
    sal_Bool Hyphenate( SwInterHyphInfo &rInf );

    // Liefert den Rest eines Feldes fuer den Follow zurueck
    const SwFldPortion* GetRestPortion();

    // Probegrow
    inline SwTwips GrowTst( const SwTwips nGrow );

    SwParaPortion *GetPara();
    inline const SwParaPortion *GetPara() const;
    inline sal_Bool HasPara() const;
    sal_Bool _HasPara() const;

    // RTTI
    TYPEINFO();
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtFrm)

    // Locking
    inline sal_Bool IsLocked()      const { return bLocked;     }
    inline sal_Bool IsFormatted()   const { return bFormatted;  }

    inline sal_Bool IsWidow()       const { return bWidow;      }
    inline sal_Bool IsJustWidow()   const { return bJustWidow;  }
    inline sal_Bool IsEmpty()       const { return bEmpty;      }
    inline sal_Bool HasFtn()        const { return bFtn;        }
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

    // Hat der Frm eine lokale Fussnote (in diesem Frm bzw. Follow)?
#ifdef PRODUCT
    void CalcFtnFlag();
#else
    void CalcFtnFlag( xub_StrLen nStop = STRING_LEN );//Fuer den Test von SplitFrm
#endif

    // Hidden
    sal_Bool IsHiddenNow() const;       // bHidden && pOut == pPrt
    void HideHidden();              // Anhaengsel entfernen wenn Hidden

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

    // Liefert die Summe der Zeilenhoehen in pLine zurueck.
    USHORT GetParHeight() const;

    // Liefert die Resthoehe zurueck
    inline SwTwips GetRstHeight() const;

    inline       SwTxtFrm *GetFollow();
    inline const SwTxtFrm *GetFollow() const;
    inline       SwTxtFrm *FindMaster();
    inline const SwTxtFrm *FindMaster() const;

    // Suche die Seitennummer von ErgoSum und QuoVadis
    SwTxtFrm *FindQuoVadisFrm();
    SwTxtFrm *FindErgoSumFrm();
    void SetErgoSumNum( const USHORT nErgo );

    // holt die Formatierug nach, wenn der Idle-Handler zugeschlagen hat.
    SwTxtFrm *GetFormatted();

    // wird demnaechst uebertragen
    inline void SetFtn( const sal_Bool bNew ) { bFtn = bNew; }

    // Beruecksichtigung der Follows
    inline sal_Bool IsInside( const xub_StrLen nPos ) const;

    const SwBodyFrm   *FindBodyFrm()   const;

    // DropCaps und Selektionen
    inline sal_Bool GetDropRect( SwRect &rRect ) const
    { return HasPara() ? _GetDropRect( rRect ) : sal_False; }

    static SwCache *GetTxtCache() { return pTxtCache; }
    static void     SetTxtCache( SwCache *pNew ) { pTxtCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }
    static void SetMinPrtLine( long nNew ) { nMinPrtLine = nNew; }

    inline USHORT GetCacheIdx() const { return nCacheIdx; }
    inline void   SetCacheIdx( const USHORT nNew ) { nCacheIdx = nNew; }

    //Entfert die Line-Informationen aus dem Cache.
    void ClearPara();

    // Bin ich ein FtnFrm, der eine Nummer am Absatzanfang hat?
    inline sal_Bool IsFtnNumFrm() const
    { return IsInFtn() && !GetIndPrev() && _IsFtnNumFrm(); }

    // simuliert eine Formatierung, als wenn es keinen rechten Rand und
    // keine Flys oder andere Hindernisse gaebe und liefert die Breite.
    USHORT CalcFitToContent();

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


    USHORT GetLineCount( USHORT nPos );     //Ermittelt die Zeilenanzahl

    //Fuer die Anzeige der Zeilennummern.
    ULONG GetAllLines()  const { return nAllLines; }
    ULONG GetThisLines() const { return nThisLines;}
    void RecalcAllLines();

    // Stoppt Animationen innerhalb von Numerierungen
    void StopAnimation( OutputDevice *pOut );

    void CriticalLines(const OutputDevice& rOut,SwStripes &rStripes,long nOffs);
};

/*************************************************************************
 *                          class SwTxtFrmLocker
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
 *                      Inline-Implementierung
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
    return Grow( nGrow, pHeight, sal_True );
}


#ifdef DEBUG
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
    long nWords;    // gepruefte Worte
    long nFlushCnt; // zaehlt die Messungen

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

#endif
