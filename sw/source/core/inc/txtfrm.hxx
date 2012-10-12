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
#ifndef SW_TXTFRM_HXX
#define SW_TXTFRM_HXX

#include <tools/mempool.hxx>
#include <tools/string.hxx>
#include "cntfrm.hxx"
#include "ndtxt.hxx"

class SwCharRange;
class SwTxtNode;
class SwTxtFormatter;
class SwTxtFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class SwBodyFrm;
class SwTxtFtn;
class SwInterHyphInfo;      // Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrmFmt;
class OutputDevice;
class SwTestFormat;
struct SwCrsrMoveState;
struct SwFillData;
class SwPortionHandler;
class SwScriptInfo;
class SwViewOption;
class SwWrongList;

/// Represents the visualization of a paragraph.
class SwTxtFrm: public SwCntntFrm
{
    friend class SwTxtIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class SwTxtFrmLocker;        // duerfen Lock()/Unlock()
    friend sal_Bool sw_ChangeOffset( SwTxtFrm* pFrm, sal_uInt16 nNew );

    static SwCache *pTxtCache;  //Pointer auf den Line-Cache
    static long nMinPrtLine;    //Diese Linie darf beim Drucken nicht
        //unterschritten werden, Hack fuer Tabellenzellen ueber mehrere Seiten

    sal_uLong  nAllLines        :24;//Anzahl der Zeilen fuer das Paint (inkl. nThisLines)
    sal_uLong  nThisLines       :8; //Anzahl der Zeilen dieses Frames

    // The x position for flys anchored at this paragraph.
    // These values are calculated in SwTxtFrm::CalcBaseOfstForFly()
    SwTwips mnFlyAnchorOfst;
    // The x position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorOfstNoWrap;
    SwTwips mnFtnLine;
    // OD 2004-03-17 #i11860# - re-factoring of #i11859#
    // member for height of last line (value needed for proportional line spacing)
    SwTwips mnHeightOfLastLine;
    // member for the additional first line offset, which is caused by the list
    // label alignment for list level position and space mode LABEL_ALIGNMENT.
    // This additional first line offset is used for the text formatting.
    // It is NOT used for the determination of printing area.
    SwTwips mnAdditionalFirstLineOffset;


    xub_StrLen nOfst;           //nOfst gibt den Offset im Cntnt (Anzahl Zeichen) an.

    sal_uInt16 nCacheIdx;           //Index in den Cache, USHRT_MAX wenn definitiv
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

    // Wird nur in Format gerufen:
    void AdjustFrm( const SwTwips nChgHeight, sal_Bool bHasToFit = sal_False );

    // wertet in Format() die Preps aus.
    sal_Bool CalcPreps();
    void PrepWidows( const sal_uInt16 nNeed, sal_Bool bNotify = sal_True );
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
                      const sal_Bool bChgFrm, SwCrsrMoveState* = 0 ) const;
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
    sal_Bool FormatQuick( bool bForceQuickFormat );

    // Opt: Leere Absaetze formatieren
    sal_Bool FormatEmpty();
    SwTwips EmptyHeight() const;
    // Opt: Leere Absaetze painten
    sal_Bool PaintEmpty( const SwRect &, sal_Bool bCheck ) const;

    void ChgThisLines();//Muss immer gerufen werden, wenn sich die Zeilenazahl
                        //veraendert haben kann.

    // required for 'new' relative anchor position
    void CalcBaseOfstForFly();

    /** method to determine height of last line, needed for proportional line spacing

        OD 2004-03-17 #i11860#
        OD 2005-05-20 #i47162# - introduce new optional parameter <_bUseFont>
        in order to force the usage of the former algorithm to determine the
        height of the last line, which uses the font.

        @param _bUseFont
        optional input parameter - boolean indicating, if the font has to be
        used to determine the height of the last line. default value: false

        @author OD
    */
    void _CalcHeightOfLastLine( const bool _bUseFont = false );

    // ST2
    SwWrongList* _SmartTagScan ( ::rtl::OUString aTxtToScan, SwWrongList *pSmartTagList,
                                 xub_StrLen nBegin,xub_StrLen nEnd,
                                 xub_StrLen nInsertPos, xub_StrLen nActPos,
                                 xub_StrLen &nChgStart, xub_StrLen &nChgEnd,
                                 xub_StrLen &nInvStart, xub_StrLen &nInvEnd);
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:

    //public, weil der eine oder andere die Methode rufen darf um das
    //Prepare zu sparen - mit Vorsicht zu geniessen!
    void Init();

    // Wird von FormatSpelling( ) gerufen
    SwRect _AutoSpell( const SwCntntNode*, const SwViewOption&, sal_uInt16 );
    // is called from the FormatSpelling( ) method
    SwRect SmartTagScan( SwCntntNode* , sal_uInt16 );
    // Wird vom CollectAutoCmplWords gerufen
    void CollectAutoCmplWrds( SwCntntNode* , sal_uInt16 );

    // Returns the screen position of rPos. The values are relative to the upper
    // left position of the page frame.
    // Additional information can be obtained by passing an SwCrsrMoveState object.
    // Returns sal_False if rPos > number of character is string
    virtual sal_Bool   GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCrsrMoveState* pCMS = 0 ) const;
    // Eine etwas abgespeckte GetCharRect-Version fuer autopositionierte Rahmen
    sal_Bool GetAutoPos( SwRect &, const SwPosition& ) const;

    /** determine top of line for given position in the text frame

        OD 11.11.2003 #i22341#
        Assumption: given position exists in the text frame or in a follow of it
        OD 2004-02-02 - adjustment
        Top of first paragraph line is the top of the paragraph.
        OD 2004-03-18 #i11860# - Consider upper space amount considered for
        previous frame and the page grid.

        @author OD

        @param _onTopOfLine
        output parameter - top of line, if the given position is found in the
        text frame.

        @param _rPos
        input parameter - reference to the position in the text frame

        @return boolean indicating, if the top of line for the given position
        has been determined or not.
    */
    bool GetTopOfLine( SwTwips& _onTopOfLine,
                       const SwPosition& _rPos ) const;

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;


    //Liefert in nOffset den Offset des Characters innerhalb des
    //gesetzten Textbuffers zurueck, welcher der durch aPoint
    //gegebenen Position innerhalb der SSize des Layout am
    //naechsten ist. Wenn der SPoint ausserhalb der SSize liegt,
    //liefert die Funktion sal_False, sal_True sonst.
    virtual sal_Bool GetCrsrOfst( SwPosition *, Point&,
                                  SwCrsrMoveState* = 0) const;

    // GetKeyCrsrOfst sorgt dafuer, dass der Frame nicht gewechselt wird
    // (z.B. Wechsel in den zeichengebundenen Frame).
    inline  sal_Bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, sal_False ); }

    void   PaintExtraData( const SwRect & rRect ) const; //Seitennummer usw.
    SwRect Paint();
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
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

    // Prepares the cursor position for a visual cursor move (BiDi).
    // The behaviour is different for insert and overwrite cursors
    void PrepareVisualMove( xub_StrLen& nPos, sal_uInt8& nCrsrLevel,
                            sal_Bool& bRight, sal_Bool bInsertCrsr );

    // Methoden zur Verwaltung von FolgeFrames
           SwCntntFrm *SplitFrm( const xub_StrLen nTxtPos );
           SwCntntFrm *JoinFrm();
    inline sal_uInt16      GetOfst() const { return nOfst; }
           void        _SetOfst( const sal_uInt16 nNewOfst );
    inline void        SetOfst ( const sal_uInt16 nNewOfst );
    inline void        ManipOfst ( const sal_uInt16 nNewOfst ){ nOfst = nNewOfst; }
           SwTxtFrm   *GetFrmAtPos ( const SwPosition &rPos);
    inline const SwTxtFrm *GetFrmAtPos ( const SwPosition &rPos) const;
           // OD 07.10.2003 #110978# - return <reference> instead of <pointer>
    SwTxtFrm&   GetFrmAtOfst( const xub_StrLen nOfst );
    // Wenn es einen Follow gibt und wir selbst keinen Text enthalten:
    inline sal_Bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    // Liefert den zu bearbeitenden Textausschnitt zurueck (inline, s.u.)
    const String& GetTxt() const;
    inline SwTxtNode *GetTxtNode()
        { return static_cast< SwTxtNode* >( SwCntntFrm::GetNode()); }
    inline const SwTxtNode *GetTxtNode() const
        { return static_cast< const SwTxtNode* >( SwCntntFrm::GetNode()); }

    SwTxtFrm(SwTxtNode * const, SwFrm* );
    virtual ~SwTxtFrm();

    // SwCntntFrm: der "kurze Dienstweg" fuer die Frames.
    // Wer den void* falsch casted ist selbst Schuld!
    // Auf jedenfall muss der void* auf 0 geprueft werden.
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, sal_Bool bNotify = sal_True );

    // nMaxHeight is the required height
    // bSplit indicates, that the paragraph has to be split
    // bTst indicates, that we are currently doing a test formatting
    virtual sal_Bool WouldFit( SwTwips &nMaxHeight, sal_Bool &bSplit, sal_Bool bTst );

    // Hier das WouldFit-Aequivalent fuer mal kurz probeweise
    // umgehaengte TextFrames, auch hier liefert
    // nMaxHeight die benoetigte Hoehe,
    // und bSplit sagt, obj der Absatz gesplittet werden muss.
    // Uebergeben wird der potentielle Vorgaenger fuer die Abstandsberechnung
    sal_Bool TestFormat( const SwFrm* pPrv, SwTwips &nMaxHeight, sal_Bool &bSplit );

    // Wir formatieren eine Zeile fuer die interaktive Trennung
    // Return: found
    sal_Bool Hyphenate( SwInterHyphInfo &rInf );

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

    inline sal_Bool IsSwapped() const { return bIsSwapped; }

    // Hat der Frm eine lokale Fussnote (in diesem Frm bzw. Follow)?
#ifdef DBG_UTIL
    void CalcFtnFlag( xub_StrLen nStop = STRING_LEN );//For testing SplitFrm
#else
    void CalcFtnFlag();
#endif

    // Hidden
    sal_Bool IsHiddenNow() const;       // bHidden && pOut == pPrt
    void HideHidden();              // Anhaengsel entfernen wenn Hidden
    void HideFootnotes( xub_StrLen nStart, xub_StrLen nEnd );

    /** method to hide/show objects

        OD 2004-01-15 #110582#
        method hides respectively shows objects, which are anchored at paragraph,
        at/as a character of the paragraph, corresponding to the paragraph and
        paragraph portion visibility.

        @author OD
    */
    void HideAndShowObjects();

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
    SwTwips GetFtnLine( const SwTxtFtn *pFtn ) const;

    // Liefern den linken und den rechten Rand in
    // Dokumentkoordinaten (unter Beachtung der Absatzattribute).
    inline SwTwips GetLeftMargin() const;
    inline SwTwips GetRightMargin() const;

    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual void  CheckDirection( sal_Bool bVert );

    // Liefert die Summe der Zeilenhoehen in pLine zurueck.
    sal_uInt16 GetParHeight() const;

    // Liefert die Resthoehe zurueck
    inline SwTwips GetRstHeight() const;

    inline       SwTxtFrm *GetFollow();
    inline const SwTxtFrm *GetFollow() const;

    // Suche die Seitennummer von ErgoSum und QuoVadis
    SwTxtFrm *FindQuoVadisFrm();

    // holt die Formatierug nach, wenn der Idle-Handler zugeschlagen hat.
    // #i29062# GetFormatted() can trigger a full formatting
    // of the paragraph, causing other layout frames to become invalid. This
    // has to be avoided during painting. Therefore we need to pass the
    // information that we are currently in the paint process.
    SwTxtFrm* GetFormatted( bool bForceQuickFormat = false );

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

    inline sal_uInt16 GetCacheIdx() const { return nCacheIdx; }
    inline void   SetCacheIdx( const sal_uInt16 nNew ) { nCacheIdx = nNew; }

    //Entfert die Line-Informationen aus dem Cache.
    void ClearPara();

    // Bin ich ein FtnFrm, der eine Nummer am Absatzanfang hat?
    inline sal_Bool IsFtnNumFrm() const
    { return IsInFtn() && !GetIndPrev() && _IsFtnNumFrm(); }

    // simuliert eine Formatierung, als wenn es keinen rechten Rand und
    // keine Flys oder andere Hindernisse gaebe und liefert die Breite.
    SwTwips CalcFitToContent();

    /** simulate format for a list item paragraph, whose list level attributes
        are in LABEL_ALIGNMENT mode, in order to determine additional first
        line offset for the real text formatting due to the value of label
        adjustment attribute of the list level.

        @author OD
    */
    void CalcAdditionalFirstLineOffset();

    inline SwTwips GetAdditionalFirstLineOffset() const
    {
        return mnAdditionalFirstLineOffset;
    }

    // liefert den zusaetzlichen Zeilenabstand fuer den naechsten Absatz
    // OD 07.01.2004 #i11859# - change return data type;
    //      add default parameter <_bNoPropLineSpacing> to control, if the
    //      value of a proportional line spacing is returned or not
    long GetLineSpace( const bool _bNoPropLineSpacing = false ) const;

    // liefert die erste Zeilenhoehe zurueck
    sal_uInt16 FirstLineHeight() const;

    // Haengt FlyInCntFrm um, wenn nEnd > Index >= nStart ist.
    void MoveFlyInCnt( SwTxtFrm *pNew, sal_uInt16 nStart, sal_uInt16 nEnd );

    // Berechnet die Position von FlyInCntFrms
    sal_uInt16 CalcFlyPos( SwFrmFmt* pSearch );

    // Ermittelt die Startposition und Schrittweite des Registers
    sal_Bool FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff );


    sal_uInt16 GetLineCount( sal_uInt16 nPos );     //Ermittelt die Zeilenanzahl

    //Fuer die Anzeige der Zeilennummern.
    sal_uLong GetAllLines()  const { return nAllLines; }
    sal_uLong GetThisLines() const { return nThisLines;}
    void RecalcAllLines();

    // Stoppt Animationen innerhalb von Numerierungen
    void StopAnimation( OutputDevice *pOut );

    // visit all portions for Accessibility
    void VisitPortions( SwPortionHandler& rPH ) const;

    // returns the script info stored at the paraportion
    const SwScriptInfo* GetScriptInfo() const;

    // Swaps width and height of the text frame
    void SwapWidthAndHeight();
    // Calculates the coordinates of a rectangle when switching from
    // horizontal to vertical layout.
    void SwitchHorizontalToVertical( SwRect& rRect ) const;
    // Calculates the coordinates of a point when switching from
    // horizontal to vertical layout.
    void SwitchHorizontalToVertical( Point& rPoint ) const;
    // Calculates the a limit value when switching from
    // horizontal to vertical layout.
    long SwitchHorizontalToVertical( long nLimit ) const;
    // Calculates the coordinates of a rectangle when switching from
    // vertical to horizontal layout.
    void SwitchVerticalToHorizontal( SwRect& rRect ) const;
    // Calculates the coordinates of a point when switching from
    // vertical to horizontal layout.
    void SwitchVerticalToHorizontal( Point& rPoint ) const;
    // Calculates the a limit value when switching from
    // vertical to horizontal layout.
    long SwitchVerticalToHorizontal( long nLimit ) const;

    // Calculates the coordinates of a rectangle when switching from
    // LTR to RTL layout
    void SwitchLTRtoRTL( SwRect& rRect ) const;
    // Calculates the coordinates of a point when switching from
    // LTR to RTL layout.
    void SwitchLTRtoRTL( Point& rPoint ) const;
    // Calculates the coordinates of a rectangle when switching from
    // RTL to LTR layout
    inline void SwitchRTLtoLTR( SwRect& rRect ) const { SwitchLTRtoRTL( rRect ); }
    // Calculates the coordinates of a point when switching from
    // RTL to LTR layout.
    inline void SwitchRTLtoLTR( Point& rPoint ) const { SwitchLTRtoRTL( rPoint ); };

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

    // OD 2004-03-17 #i11860#
    inline SwTwips GetHeightOfLastLine() const
    {
        return mnHeightOfLastLine;
    }

    static void repaintTextFrames( const SwTxtNode& rNode );

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer);
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
    return Grow( nGrow, sal_True );
}

#ifdef DBG_UTIL
// query WYSIWYG DBG
extern sal_Bool IsDbg( const SwTxtFrm *pFrm );
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

class SwLayoutModeModifier
{
    const OutputDevice& rOut;
    long nOldLayoutMode;
public:
    SwLayoutModeModifier( const OutputDevice& rOutp );
    ~SwLayoutModeModifier();
    void Modify( sal_Bool bChgToRTL );
    void SetAuto();
};

class SwDigitModeModifier
{
    const OutputDevice& rOut;
    LanguageType nOldLanguageType;
public:
    SwDigitModeModifier( const OutputDevice& rOutp, LanguageType eCurLang );
    ~SwDigitModeModifier();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
