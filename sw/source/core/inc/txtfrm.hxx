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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TXTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TXTFRM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <tools/mempool.hxx>
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

#define NON_PRINTING_CHARACTER_COLOR RGB_COLORDATA( 0x6A, 0xBE, 0xD3 )

/// Represents the visualization of a paragraph.
class SwTxtFrm: public SwCntntFrm
{
    friend class SwTxtIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class SwTxtFrmLocker;        // duerfen Lock()/Unlock()
    friend bool sw_ChangeOffset( SwTxtFrm* pFrm, sal_Int32 nNew );

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


    sal_Int32 nOfst;           //nOfst gibt den Offset im Cntnt (Anzahl Zeichen) an.

    sal_uInt16 nCacheIdx;           //Index in den Cache, USHRT_MAX wenn definitiv
                                //kein passendes Objekt im Cache steht.

    //Teilt den Master ab und erzeugt einen Follow oder passt die
    //Daten im Follow an.
           void _AdjustFollow( SwTxtFormatter &rLine, const sal_Int32 nOffset,
                               const sal_Int32 nStrEnd, const sal_uInt8 nMode );
    inline void AdjustFollow( SwTxtFormatter &rLine, const sal_Int32 nOffset,
                              const sal_Int32 nStrEnd, const sal_uInt8 nMode );

    //Iteriert ueber alle Zeilen und stellt das Linespacing
    //entsprechend dem Attribut ein.
    void CalcLineSpace();

    // Wird nur in Format gerufen:
    void AdjustFrm( const SwTwips nChgHeight, bool bHasToFit = false );

    // wertet in Format() die Preps aus.
    bool CalcPreps();
    void PrepWidows( const sal_uInt16 nNeed, bool bNotify = true );
    void _InvalidateRange( const SwCharRange &, const long = 0);
    inline void InvalidateRange( const SwCharRange &, const long = 0);

    // WidowsAndOrphans, AdjustFrm, AdjustFollow
    void FormatAdjust( SwTxtFormatter &rLine, WidowsAndOrphans &rFrmBreak,
                       const sal_Int32 nStrLen, const bool bDummy );

    bool bLocked        : 1;        // im Format?
    bool bFormatted     : 1;        // nach Format auf true
    bool bWidow         : 1;        // sind wir ein Widow
    bool bJustWidow     : 1;        // haben wir soeben Widow angefordert
    bool bEmpty         : 1;        // sind wir ein leerer Absatz
    bool bInFtnConnect  : 1;        // Steht gerade im Connect
    bool bFtn           : 1;        // Hat mindestens eine Fussnote
    bool bRepaint       : 1;        // TxtFrm: Repaint steht zur Abholung bereit
    bool bBlinkPor      : 1;        // enthaelt Blink-Portions
    bool bFieldFollow   : 1;        // beginne mit Feldrest des Masters
    bool bHasAnimation  : 1;        // enthaelt animierte SwGrfNumPortion
    bool bIsSwapped     : 1;        // during text formatting we swap the
                                        // width and height for vertical formatting
    // OD 14.03.2003 #i11760# - flag to control, if follow is formatted in
    // method <CalcFollow(..)>.
    // E.g., avoid formatting of follow, if method <SwLayoutFrm::FormatWidthCols(..)>
    // is running.
    bool mbFollowFormatAllowed : 1;

    void ResetPreps();
    inline void Lock() { bLocked = true; }
    inline void Unlock() { bLocked = false; }
    inline void SetFormatted( const bool bNew ) { bFormatted = bNew; }
    inline void SetWidow( const bool bNew ) { bWidow = bNew; }
    inline void SetJustWidow( const bool bNew ) { bJustWidow = bNew; }
    inline void SetEmpty( const bool bNew ) { bEmpty = bNew; }
    inline void SetFieldFollow( const bool bNew ) { bFieldFollow = bNew; }

    bool IsIdxInside( const sal_Int32 nPos, const sal_Int32 nLen ) const;

    // Wechselt den Frame oder auch nicht (vgl. FlyCnt)
    bool _GetCrsrOfst(SwPosition *pPos, const Point &rPoint,
                      const bool bChgFrm, SwCrsrMoveState* = 0 ) const;
    void FillCrsrPos( SwFillData &rFill ) const;

    // formatiert genau eine Zeile ...
    bool FormatLine( SwTxtFormatter &rLine, const bool bPrev );

    // Um Stack einzusparen aufgeteilt ...
    // _Format ruft _Format mit Parametern
    void _Format( SwParaPortion *pPara );
    void _Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                  const bool bAdjust = false );
    void FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf );

    // formatiert den Follow und sorgt fuer die Entsorgung bei Orphans
    bool CalcFollow(  const sal_Int32 nTxtOfst );

    // korrigiert die Stelle ab der formatiert werden muss.
    sal_Int32 FindBrk(const OUString &rTxt, const sal_Int32 nStart,
                                       const sal_Int32 nEnd) const;

    // inline-Weiche
    SwTwips _GetFtnFrmHeight() const;

    // Aus CalcPreps ausgelagert.
    bool CalcPrepFtnAdjust();

    // Fuer Ftn und WidOrp: Zwangsvalidierung
    void ValidateFrm();
    void ValidateBodyFrm();

    bool _GetDropRect( SwRect &rRect ) const;

    void SetPara( SwParaPortion *pNew, bool bDelete = true );

    bool _IsFtnNumFrm() const;

    // 6995: Formatinformationen auffrischen
    bool FormatQuick( bool bForceQuickFormat );

    // Opt: Leere Absaetze formatieren
    bool FormatEmpty();
    SwTwips EmptyHeight() const;
    // Opt: Leere Absaetze painten
    bool PaintEmpty( const SwRect &, bool bCheck ) const;

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
    */
    void _CalcHeightOfLastLine( const bool _bUseFont = false );

    // ST2
    SwWrongList* _SmartTagScan ( OUString aTxtToScan, SwWrongList *pSmartTagList,
                                 sal_Int32 nBegin,sal_Int32 nEnd,
                                 sal_Int32 nInsertPos, sal_Int32 nActPos,
                                 sal_Int32 &nChgStart, sal_Int32 &nChgEnd,
                                 sal_Int32 &nInvStart, sal_Int32 &nInvEnd);
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:

    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >  GetTabStopInfo( SwTwips CurrentPos );
    //public, weil der eine oder andere die Methode rufen darf um das
    //Prepare zu sparen - mit Vorsicht zu geniessen!
    void Init();

    // Wird von FormatSpelling( ) gerufen
    SwRect _AutoSpell( const SwCntntNode*, const SwViewOption&, sal_Int32 );
    // is called from the FormatSpelling( ) method
    SwRect SmartTagScan( SwCntntNode* , sal_Int32 );
    // Wird vom CollectAutoCmplWords gerufen
    void CollectAutoCmplWrds( SwCntntNode* , sal_Int32 );

    // Returns the screen position of rPos. The values are relative to the upper
    // left position of the page frame.
    // Additional information can be obtained by passing an SwCrsrMoveState object.
    // Returns false if rPos > number of character is string
    virtual bool GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCrsrMoveState* pCMS = 0 ) const;
    // Eine etwas abgespeckte GetCharRect-Version fuer autopositionierte Rahmen
    bool GetAutoPos( SwRect &, const SwPosition& ) const;

    /** determine top of line for given position in the text frame

        OD 11.11.2003 #i22341#
        Assumption: given position exists in the text frame or in a follow of it
        OD 2004-02-02 - adjustment
        Top of first paragraph line is the top of the paragraph.
        OD 2004-03-18 #i11860# - Consider upper space amount considered for
        previous frame and the page grid.

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
    //liefert die Funktion false, true sonst.
    virtual bool GetCrsrOfst( SwPosition *, Point&,
                                  SwCrsrMoveState* = 0, bool bTestBackground = false ) const;

    // GetKeyCrsrOfst sorgt dafuer, dass der Frame nicht gewechselt wird
    // (z.B. Wechsel in den zeichengebundenen Frame).
    inline bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, false ); }

    void   PaintExtraData( const SwRect & rRect ) const; //Seitennummer usw.
    SwRect Paint();
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual bool GetInfo( SfxPoolItem & ) const;

    //Layoutorientiertes Cursortravelling: Linker, rechter Rand,
    //vorhergehende/naechste Zeile, gleiche horizontale Position.
    virtual bool LeftMargin(SwPaM *) const;
    virtual bool RightMargin(SwPaM *, bool bAPI = false) const;

    virtual bool UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;
    virtual bool UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;
    bool _UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;
    bool _UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;

    // Prepares the cursor position for a visual cursor move (BiDi).
    // The behaviour is different for insert and overwrite cursors
    void PrepareVisualMove( sal_Int32& nPos, sal_uInt8& nCrsrLevel,
                            bool& bRight, bool bInsertCrsr );

    // Methoden zur Verwaltung von FolgeFrames
           SwCntntFrm *SplitFrm( const sal_Int32 nTxtPos );
           SwCntntFrm *JoinFrm();
    inline sal_Int32  GetOfst() const { return nOfst; }
           void        _SetOfst( const sal_Int32 nNewOfst );
    inline void        SetOfst ( const sal_Int32 nNewOfst );
    inline void        ManipOfst ( const sal_Int32 nNewOfst ){ nOfst = nNewOfst; }
           SwTxtFrm   *GetFrmAtPos ( const SwPosition &rPos);
    inline const SwTxtFrm *GetFrmAtPos ( const SwPosition &rPos) const;
           // OD 07.10.2003 #110978# - return <reference> instead of <pointer>
    SwTxtFrm&   GetFrmAtOfst( const sal_Int32 nOfst );
    // Wenn es einen Follow gibt und wir selbst keinen Text enthalten:
    inline bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    // Liefert den zu bearbeitenden Textausschnitt zurueck (inline, s.u.)
    const OUString& GetTxt() const;
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
                          const void *pVoid = 0, bool bNotify = true );

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
    bool Hyphenate( SwInterHyphInfo &rInf );

    // Probegrow
    inline SwTwips GrowTst( const SwTwips nGrow );

    SwParaPortion *GetPara();
    inline const SwParaPortion *GetPara() const;
    inline bool HasPara() const;
    bool _HasPara() const;
    // If there are any hanging punctuation portions in the margin
    // the offset will be returned.
    SwTwips HangingMargin() const;

    // RTTI
    TYPEINFO();
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtFrm)

    // Locking
    inline bool IsLocked()      const { return bLocked;     }
    inline bool IsFormatted()   const { return bFormatted;  }

    inline bool IsWidow()       const { return bWidow;      }
    inline bool IsJustWidow()   const { return bJustWidow;  }
    inline bool IsEmpty()       const { return bEmpty;      }
    inline bool HasFtn()        const { return bFtn;        }
    inline bool IsInFtnConnect()const { return bInFtnConnect;}
    inline bool IsFieldFollow() const { return bFieldFollow;}

    inline void SetRepaint() const;
    inline void ResetRepaint() const;
    inline bool HasRepaint() const { return bRepaint; }
    inline void SetBlinkPor() const;
    inline void ResetBlinkPor() const;
    inline bool HasBlinkPor() const { return bBlinkPor; }
    inline void SetAnimation() const
        { ( (SwTxtFrm*)this )->bHasAnimation = true; }
    inline bool HasAnimation() const { return bHasAnimation; }

    inline bool IsSwapped() const { return bIsSwapped; }

    // Hat der Frm eine lokale Fussnote (in diesem Frm bzw. Follow)?
#ifdef DBG_UTIL
    void CalcFtnFlag( sal_Int32 nStop = COMPLETE_STRING );//For testing SplitFrm
#else
    void CalcFtnFlag();
#endif

    // Hidden
    bool IsHiddenNow() const;       // bHidden && pOut == pPrt
    void HideHidden();              // Anhaengsel entfernen wenn Hidden
    void HideFootnotes( sal_Int32 nStart, sal_Int32 nEnd );

    /** method to hide/show objects

        OD 2004-01-15 #110582#
        method hides respectively shows objects, which are anchored at paragraph,
        at/as a character of the paragraph, corresponding to the paragraph and
        paragraph portion visibility.
    */
    void HideAndShowObjects();

    // Ftn
    void RemoveFtn( const sal_Int32 nStart = 0,
                    const sal_Int32 nLen = COMPLETE_STRING );
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
    virtual void CheckDirection( bool bVert );

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
    inline void SetFtn( const bool bNew ) { bFtn = bNew; }

    // Beruecksichtigung der Follows
    inline bool IsInside( const sal_Int32 nPos ) const;

    const SwBodyFrm   *FindBodyFrm()   const;

    // DropCaps und Selektionen
    inline bool GetDropRect( SwRect &rRect ) const
    { return HasPara() && _GetDropRect( rRect ); }

    static SwCache *GetTxtCache() { return pTxtCache; }
    static void     SetTxtCache( SwCache *pNew ) { pTxtCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }
    static void SetMinPrtLine( long nNew ) { nMinPrtLine = nNew; }

    inline sal_uInt16 GetCacheIdx() const { return nCacheIdx; }
    inline void   SetCacheIdx( const sal_uInt16 nNew ) { nCacheIdx = nNew; }

    //Entfert die Line-Informationen aus dem Cache.
    void ClearPara();

    // Bin ich ein FtnFrm, der eine Nummer am Absatzanfang hat?
    inline bool IsFtnNumFrm() const
    { return IsInFtn() && !GetIndPrev() && _IsFtnNumFrm(); }

    // simuliert eine Formatierung, als wenn es keinen rechten Rand und
    // keine Flys oder andere Hindernisse gaebe und liefert die Breite.
    SwTwips CalcFitToContent();

    /** simulate format for a list item paragraph, whose list level attributes
        are in LABEL_ALIGNMENT mode, in order to determine additional first
        line offset for the real text formatting due to the value of label
        adjustment attribute of the list level.
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
    void MoveFlyInCnt( SwTxtFrm *pNew, sal_Int32 nStart, sal_Int32 nEnd );

    // Berechnet die Position von FlyInCntFrms
    sal_Int32 CalcFlyPos( SwFrmFmt* pSearch );

    // Ermittelt die Startposition und Schrittweite des Registers
    bool FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff );


    sal_uInt16 GetLineCount( sal_Int32 nPos );     //Ermittelt die Zeilenanzahl

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

    SwTwips GetBaseOfstForFly( bool bIgnoreFlysAnchoredAtThisFrame ) const
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

inline bool SwTxtFrm::HasPara() const
{
    return nCacheIdx!=USHRT_MAX && _HasPara();
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
    return Grow( nGrow, true );
}

inline bool SwTxtFrm::IsInside( const sal_Int32 nPos ) const
{
    bool bRet = true;
    if( nPos < GetOfst() )
        bRet = false;
    else
    {
        const SwTxtFrm *pFoll = GetFollow();
        if( pFoll && nPos >= pFoll->GetOfst() )
            bRet = false;
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
    const sal_Int32 nOffset, const sal_Int32 nStrEnd, const sal_uInt8 nMode )
{
    if ( HasFollow() )
        _AdjustFollow( rLine, nOffset, nStrEnd, nMode );
}

inline void SwTxtFrm::SetOfst( const sal_Int32 nNewOfst )
{
    if ( nOfst != nNewOfst )
        _SetOfst( nNewOfst );
}

inline void SwTxtFrm::SetRepaint() const
{
    ((SwTxtFrm*)this)->bRepaint = true;
}
inline void SwTxtFrm::ResetRepaint() const
{
    ((SwTxtFrm*)this)->bRepaint = false;
}

inline void SwTxtFrm::SetBlinkPor() const
{
    ((SwTxtFrm*)this)->bBlinkPor = true;
}
inline void SwTxtFrm::ResetBlinkPor() const
{
    ((SwTxtFrm*)this)->bBlinkPor = false;
}

#define SWAP_IF_SWAPPED( pFrm )\
    bool bUndoSwap = false;   \
    if ( pFrm->IsVertical() && pFrm->IsSwapped() )\
    {                                 \
        bUndoSwap = true;         \
        ((SwTxtFrm*)pFrm)->SwapWidthAndHeight();         \
    }

#define SWAP_IF_NOT_SWAPPED( pFrm )\
    bool bUndoSwap = false;     \
    if ( pFrm->IsVertical() && ! pFrm->IsSwapped() )\
    {                                   \
        bUndoSwap = true;           \
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
    bool bUndo;
public:
    SwFrmSwapper( const SwTxtFrm* pFrm, bool bSwapIfNotSwapped );
    ~SwFrmSwapper();
};

class SwLayoutModeModifier
{
    const OutputDevice& rOut;
    long nOldLayoutMode;
public:
    SwLayoutModeModifier( const OutputDevice& rOutp );
    ~SwLayoutModeModifier();
    void Modify( bool bChgToRTL );
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
