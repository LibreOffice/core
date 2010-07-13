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
#ifndef _CRSRSH_HXX
#define _CRSRSH_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/rtti.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>          // fuer SWPOSDOC
#include <viewsh.hxx>           // fuer ViewShell
#include <calbck.hxx>           // fuer SwClient
#include <cshtyp.hxx>           // fuer die CursorShell Typen
#include <crstate.hxx>          // fuer die CursorMove-Staties
#include <toxe.hxx>             // SwTOXSearchDir
#include <tblsel.hxx>               //SwTblSearchType
#include <viscrs.hxx>
#include <node.hxx>
#include <tblsel.hxx>
#include <IDocumentMarkAccess.hxx>


// einige Forward Deklarationen

class KeyCode;
class SfxItemSet;
class SfxPoolItem;
class SwCntntFrm;
class SwCrsrShell;
class SwCursor;
class SwField;
class SwFieldType;
class SwFmt;
class SwFmtFld;
class SwNodeIndex;
class SwPaM;
class SwShellCrsr;
class SwShellTableCrsr;
class SwTableNode;
class SwTxtFmtColl;
class SwVisCrsr;
class SwTxtINetFmt;
class SwFmtINetFmt;
class SwTxtAttr;
class SwTableBox;
class SwCellFrms;
class SwTOXMark;
class SwRedline;
class IBlockCursor;
class SwCntntNode; //  #i23726#
// --> OD 2008-06-19 #i90516#
class SwPostItField;
// <--
struct SwPosition;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
}}}}

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
}}}}

// enum und struktur, um ueber die Doc-Position Informationen zu erfragen

struct SwContentAtPos
{
    enum IsAttrAtPos
    {
        SW_NOTHING          = 0x0000,
        SW_FIELD            = 0x0001,
        SW_CLICKFIELD       = 0x0002,
        SW_FTN              = 0x0004,
        SW_INETATTR         = 0x0008,
        SW_TABLEBOXFML      = 0x0010,
        SW_REDLINE          = 0x0020,
        SW_OUTLINE          = 0x0040,
        SW_TOXMARK          = 0x0080,
        SW_REFMARK          = 0x0100,
        SW_NUMLABEL         = 0x0200, // #i23726#
        SW_CONTENT_CHECK    = 0x0400, // --> FME 2005-05-13 #i43742# <--
        SW_SMARTTAG         = 0x0800,
        SW_FORMCTRL         = 0x1000
#ifndef PRODUCT
        ,SW_CURR_ATTRS      = 0x4000        // nur zum Debuggen
        ,SW_TABLEBOXVALUE   = 0x8000        // nur zum Debuggen
#endif
    } eCntntAtPos;

    union {
        const SwField* pFld;
        const SfxPoolItem* pAttr;
        const SwRedline* pRedl;
        SwCntntNode * pNode; // #i23726#
        const sw::mark::IFieldmark* pFldmark;
    } aFnd;

    int nDist; // #i23726#

    String sStr;
    const SwTxtAttr* pFndTxtAttr;

    SwContentAtPos( int eGetAtPos = 0xffff )
        : eCntntAtPos( (IsAttrAtPos)eGetAtPos )
    {
        aFnd.pFld = 0;
        pFndTxtAttr = 0;
        nDist = 0; // #i23726#
    }

    // befindet sich der Node in einem geschuetzten Bereich?
    BOOL IsInProtectSect() const;
    bool     IsInRTLText()const;
};

// ReturnWerte von SetCrsr (werden verodert)
const int CRSR_POSOLD = 0x01,   // Cursor bleibt an alter Doc-Position
          CRSR_POSCHG = 0x02;   // Position vom Layout veraendert

// Helperfunction to resolve backward references in regular expressions

String *ReplaceBackReferences( const com::sun::star::util::SearchOptions& rSearchOpt, SwPaM* pPam );

// die Cursor - Shell
class SW_DLLPUBLIC SwCrsrShell : public ViewShell, public SwModify
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;
    friend class SwChgLinkFlag;

    //Braucht den Crsr als IntrnlCrsr.
    friend BOOL GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:     // public, damit defaultet werden kann !!

    /* ein enum fuer den Aufruf von UpdateCrsr */
    enum CrsrFlag {
        UPDOWN      = (1 << 0),     // Up/Down auf Spalte halten
        SCROLLWIN   = (1 << 1),     // Window Scrollen
        CHKRANGE    = (1 << 2),     // ueberpruefen von ueberlappenden PaM's
        NOCALRECT   = (1 << 3),     // CharRect nicht neu berechnen
        READONLY    = (1 << 4)      // Sichtbar machen trotz Readonly
    };

private:

    SwRect  aCharRect;          // Char-SRectangle auf dem der Cursor steht
    Point   aCrsrHeight;        // Hohe&Offset von sichtbaren Cursor
    Point   aOldRBPos;          // Right/Bottom von letzter VisArea
                                // (wird im Invalidate vom Cursor benutzt)


    // um event. Macro was anhaengt auszufuehren.
    Link aFlyMacroLnk;          // Link will be called, if the Crsr is set
                                // into a fly. A macro can be then becalled
    Link aChgLnk;               // link will be called by every attribut/
                                // format changes at cursor position.
    Link aGrfArrivedLnk;        // Link calls to UI if a grafik is arrived


    SwShellCrsr* pCurCrsr;      // der aktuelle Cursor
    SwShellCrsr* pCrsrStk;      // Stack fuer den Cursor
    SwVisCrsr *pVisCrsr;        // der Sichtbare-Cursor

    IBlockCursor *pBlockCrsr;   // interface of cursor for block (=rectangular) selection

    SwShellTableCrsr* pTblCrsr; // Tabellen-Crsr; nur in Tabellen, wenn
                                // die Selection ueber 2 Spalten liegt

    SwNodeIndex* pBoxIdx;       // fuers erkennen der veraenderten
    SwTableBox* pBoxPtr;        // Tabellen-Zelle

    long nUpDownX;              // versuche den Cursor bei Up/Down immer in
                                // der gleichen Spalte zu bewegen
    long nLeftFrmPos;
    ULONG nAktNode;             // save CursorPos at Start-Action
    xub_StrLen nAktCntnt;
    USHORT nAktNdTyp;
    bool bAktSelection;

    /*
     * Mit den Methoden SttCrsrMove und EndCrsrMove wird dieser Zaehler
     * Inc-/Decrementiert. Solange der Zaehler ungleich 0 ist, erfolgt
     * auf den akt. Cursor kein Update. Dadurch koennen "komplizierte"
     * Cursorbewegungen (ueber Find()) realisiert werden.
     */
    USHORT nCrsrMove;
    USHORT nBasicActionCnt;     // Actions, die vom Basic geklammert wurden
    CrsrMoveState eMvState;     // Status fuers Crsr-Travelling - GetCrsrOfst

    // --> OD 2008-04-02 #refactorlists#
    String sMarkedListId;
    int nMarkedListLevel;
    // <--

    BOOL bHasFocus : 1;         // Shell ist in einem Window "aktiv"
    BOOL bSVCrsrVis : 1;        // SV-Cursor Un-/Sichtbar
    BOOL bChgCallFlag : 1;      // Attributaenderung innerhalb von
                                // Start- und EndAction
    BOOL bVisPortChgd : 1;      // befindet sich im VisPortChg-Aufruf
                                // (wird im Invalidate vom Cursor benutzt)

    BOOL bCallChgLnk : 1;       // Flag fuer abgeleitete Klassen:
                                // TRUE -> ChgLnk callen
                                // Zugriff nur ueber SwChgLinkFlag
    BOOL bAllProtect : 1;       // Flag fuer Bereiche
                                // TRUE -> alles geschuetzt / versteckt
    BOOL bInCMvVisportChgd : 1; // Flag fuer CrsrMoves
                                // TRUE -> die Sicht wurde verschoben
    BOOL bGCAttr : 1;           // TRUE -> es existieren nichtaufgespannte Attr.
    BOOL bIgnoreReadonly : 1;   // TRUE -> Beim naechsten EndAction trotz
                                // Readonly den Crsr sichtbar machen.
    BOOL bSelTblCells : 1;      // TRUE -> Zellen uebers InputWin selektieren
    BOOL bAutoUpdateCells : 1;  // TRUE -> Zellen werden autoformatiert
    BOOL bBasicHideCrsr : 1;    // TRUE -> HideCrsr vom Basic
    BOOL bSetCrsrInReadOnly : 1;// TRUE -> Cursor darf in ReadOnly-Bereiche
    BOOL bOverwriteCrsr : 1;    // TRUE -> show Overwrite Crsr

    // OD 11.02.2003 #100556# - flag to allow/avoid execution of marcos (default: true)
    bool mbMacroExecAllowed : 1;

    SW_DLLPRIVATE void UpdateCrsr( USHORT eFlags
                            =SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
                     BOOL bIdleEnd = FALSE );

    SW_DLLPRIVATE void _ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing );

    // -> #i27615#

    /** Mark a certain list level of a certain list

        OD 2008-04-02 #refactorlists#
        levels of a certain lists are marked now

        @param sListId    list Id of the list whose level is marked
        @param nLevel     to be marked list level

        An empty sListId denotes that no level of a list is marked.
     */
    SW_DLLPRIVATE void MarkListLevel( const String& sListId,
                                      const int nLevel );
    // <- #i27615#

    // private method(s) accessed from public inline method(s) must be exported.
                  BOOL LeftRight( BOOL, USHORT, USHORT, BOOL );
    SW_DLLPRIVATE BOOL UpDown( BOOL, USHORT );
    SW_DLLPRIVATE BOOL LRMargin( BOOL, BOOL bAPI = FALSE );
    SW_DLLPRIVATE BOOL IsAtLRMargin( BOOL, BOOL bAPI = FALSE ) const;

    SW_DLLPRIVATE short GetTextDirection( const Point* pPt = 0 ) const;

typedef BOOL (SwCursor:: *FNCrsr)();
    SW_DLLPRIVATE BOOL CallCrsrFN( FNCrsr );

    SW_DLLPRIVATE const SwRedline* _GotoRedline( USHORT nArrPos, BOOL bSelect );

protected:

    inline SwMoveFnCollection* MakeFindRange( USHORT, USHORT, SwPaM* ) const;

    /*
     * Compare-Methode for the StackCursor and the current Cursor.
     * The Methods return -1, 0, 1 for lower, equal, greater. The enum
     * CrsrCompareType says which position is compared.
     */
    enum CrsrCompareType {
        StackPtStackMk,
        StackPtCurrPt,
        StackPtCurrMk,
        StackMkCurrPt,
        StackMkCurrMk,
        CurrPtCurrMk
    };
    int CompareCursor( CrsrCompareType eType ) const;

    // Setzt alle PaMs in OldNode auf NewPos + Offset
    void PaMCorrAbs(const SwNodeIndex &rOldNode, const SwPosition &rNewPos,
                    const xub_StrLen nOffset = 0 );

    // --> FME 2004-07-30 #i32329# Enhanced table selection
    BOOL _SelTblRowOrCol( bool bRow, bool bRowSimple = false );
    // <--

    // --> FME 2005-01-31 #i41424# Only update the marked number levels if necessary
    bool SetInFrontOfLabel( BOOL bNew );
    // <--

    void RefreshBlockCursor();

    /** Updates the marked list level according to the cursor.
    */
    SW_DLLPRIVATE void UpdateMarkedListLevel();

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin, const SwViewOption *pOpt = 0 );
    // verkleideter Copy-Constructor
    SwCrsrShell( SwCrsrShell& rShell, Window *pWin );
    virtual ~SwCrsrShell();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    // neuen Cusror erzeugen und den alten anhaengen
    SwPaM * CreateCrsr();
    // loesche den aktuellen Cursor und der folgende wird zum Aktuellen
    BOOL DestroyCrsr();
    // TableCursor in normale Cursor verwandeln, Tablemode aufheben
    void TblCrsrToCursor();
    // enter block mode, change normal cursor into block cursor
    void CrsrToBlockCrsr();
    // leave block mode, change block cursor into normal cursor
    void BlockCrsrToCrsr();

    // SelAll() selects the document body content
    // if ExtendedSelect() is called afterwards, the whole nodes array is selected
    // only for usage in special cases allowed!
    void ExtendedSelectAll();

    SwPaM* GetCrsr( BOOL bMakeTblCrsr = TRUE ) const;
    inline SwCursor* GetSwCrsr( BOOL bMakeTblCrsr = TRUE ) const;
    // nur den akt. Cursor returnen
          SwShellCrsr* _GetCrsr()                       { return pCurCrsr; }
    const SwShellCrsr* _GetCrsr() const                 { return pCurCrsr; }

    // uebergebenen Cursor anzeigen - fuer UNO
    void    SetSelection(const SwPaM& rCrsr);

    // alle Cursor aus den ContentNodes entfernen und auf 0 setzen.
    // Wurde aus der FEShell hierher verschoben.
    void ParkCrsr( const SwNodeIndex &rIdx );

    // gebe den akt. Cursor-Stack zurueck.
    // ( Wird in der EditShell beim Loeschen von Inhalten benoetigt! )
    inline SwPaM* GetStkCrsr() const;

    // Start der Klammerung, SV-Cursor und selektierte Bereiche hiden
    void StartAction();
    // Ende der Klammerung, SV-Cursor und selektierte Bereiche anzeigen
    void EndAction( const BOOL bIdleEnd = FALSE );

    // Basiscursortravelling
    long GetUpDownX() const             { return nUpDownX; }

    BOOL Left( USHORT nCnt, USHORT nMode, BOOL bAllowVisual = FALSE )
        { return LeftRight( TRUE, nCnt, nMode, bAllowVisual ); }
    BOOL Right( USHORT nCnt, USHORT nMode, BOOL bAllowVisual = FALSE )
        { return LeftRight( FALSE, nCnt, nMode, bAllowVisual ); }
    BOOL Up( USHORT nCnt = 1 )      { return UpDown( TRUE, nCnt ); }
    BOOL Down( USHORT nCnt = 1 )    { return UpDown( FALSE, nCnt ); }
    BOOL LeftMargin()               { return LRMargin( TRUE ); }
    BOOL RightMargin(BOOL bAPI = FALSE) { return LRMargin( FALSE, bAPI ); }
    BOOL SttEndDoc( BOOL bStt );

    BOOL MovePage( SwWhichPage, SwPosPage );
    BOOL MovePara( SwWhichPara, SwPosPara );
    BOOL MoveSection( SwWhichSection, SwPosSection );
    BOOL MoveTable( SwWhichTable, SwPosTable );
    BOOL MoveColumn( SwWhichColumn, SwPosColumn );
    BOOL MoveRegion( SwWhichRegion, SwPosRegion );

    // die Suchfunktionen
    ULONG Find( const com::sun::star::util::SearchOptions& rSearchOpt,
                BOOL bSearchInNotes,
                SwDocPositions eStart, SwDocPositions eEnde,
                BOOL& bCancel,
                FindRanges eRng, int bReplace = FALSE );

    ULONG Find( const SwTxtFmtColl& rFmtColl,
                SwDocPositions eStart, SwDocPositions eEnde,
                BOOL& bCancel,
                FindRanges eRng, const SwTxtFmtColl* pReplFmt = 0 );

    ULONG Find( const SfxItemSet& rSet, BOOL bNoCollections,
                SwDocPositions eStart, SwDocPositions eEnde,
                BOOL& bCancel,
                FindRanges eRng,
                const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                const SfxItemSet* rReplSet = 0 );

    // Positionieren des Cursors
    // returnt
    //  CRSR_POSCHG: wenn der ob der SPoint vom Layout korrigiert wurde.
    //  CRSR_POSOLD: wenn der Crsr nicht veraendert wurde
    int SetCrsr( const Point &rPt, BOOL bOnlyText = FALSE, bool bBlock = true );


    /*
     * Benachrichtung, dass der sichtbare Bereich sich geaendert
     * hat. aVisArea wird neu gesetzt, anschliessend wird
     * gescrollt. Das uebergebene Rectangle liegt auf
     * Pixelgrenzen, um Pixelfehler beim Scrollen zu vermeiden.
     */
    virtual void VisPortChgd( const SwRect & );

    /*
     * Virtuelle PaintMethode, damit die Selection nach dem Paint wieder
     * sichtbar wird.
     */
    void Paint( const Rectangle & rRect );

    // Bereiche
    inline void SetMark();
    inline BOOL HasMark();

    void ClearMark();

    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst TRUE: If the point is behind the mark then
       swap the PaM. FALSE: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(BOOL bPointFirst = TRUE);

    void SwapPam();
    BOOL ChgCurrPam( const Point & rPt,
                     BOOL bTstOnly = TRUE,      //Nur testen, nicht setzen
                     BOOL bTstHit  = FALSE );   //Nur genaue Treffer
    void KillPams();

    // erzeuge eine Kopie vom Cursor und speicher diese im Stack
    void Push();
    /*
     *  Loescht einen Cursor (gesteuert durch bOldCrsr)
     *      - vom Stack oder    ( bOldCrsr = TRUE )
     *      - den aktuellen und der auf dem Stack stehende wird zum aktuellen
     *
     *  Return:  es war auf dem Stack noch einer vorhanden
     */
    BOOL Pop( BOOL bOldCrsr = TRUE );
    /*
     * Verbinde zwei Cursor miteinander.
     * Loesche vom Stack den obersten und setzen dessen Mark im Aktuellen.
     */
    void Combine();

#if !defined(DBG_UTIL)
    void SttCrsrMove() { ++nCrsrMove; StartAction(); }
    void EndCrsrMove( const BOOL bIdleEnd = FALSE )
            { EndAction( bIdleEnd ); --nCrsrMove; }
#else
    void SttCrsrMove();
    void EndCrsrMove( const BOOL bIdleEnd = FALSE );
#endif

    /*
     * Beim Abgeben des Focuses werden die selektierten Bereiche nicht mehr
     * angezeigt; andererseits beim Erhalten des Focuses, werden alle selek-
     * tierten Bereiche wieder angezeigt. (Bereiche muessen neu berechnet
     * werden!)
     */
    BOOL HasShFcs() const { return bHasFocus; }
    void ShLooseFcs();
    void ShGetFcs( BOOL bUpdate = TRUE );

    // Methoden zum Anzeigen bzw. Verstecken des sichtbaren Text-Cursors
    void ShowCrsr();
    void HideCrsr();
    // Methoden zum Anzeigen bzw. Verstecken der selektierten Bereiche mit
    // dem sichtbaren Cursor
    void ShowCrsrs( BOOL bCrsrVis );
    void HideCrsrs();

    BOOL IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( BOOL bFlag ) { bOverwriteCrsr = bFlag; }

    // gebe den aktuellen Frame, in dem der Cursor steht, zurueck
    SwCntntFrm *GetCurrFrm( const BOOL bCalcFrm = TRUE ) const;

    //TRUE wenn der Crsr wenn der Crsr wegen Readonly gehidet ist,
    //FALSE wenn der arbeitet (trotz Readonly).
    BOOL IsCrsrReadonly() const;
    // Cursor steht in etwas geschuetztem oder in die Selektion umspannt
    // etwas geschuetztes.
    BOOL HasReadonlySel() const;
    // darf der Cursor in ReadOnlyBereiche?
    BOOL IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }
    void SetReadOnlyAvailable( BOOL bFlag );
    BOOL IsOverReadOnlyPos( const Point& rPt ) const;

    // Methoden fuer aFlyMacroLnk
    void        SetFlyMacroLnk( const Link& rLnk ) { aFlyMacroLnk = rLnk; }
    const Link& GetFlyMacroLnk() const             { return aFlyMacroLnk; }

    // Methoden geben/aendern den Link fuer die Attribut/Format-Aenderungen
    void        SetChgLnk( const Link &rLnk ) { aChgLnk = rLnk; }
    const Link& GetChgLnk() const             { return aChgLnk; }

    // Methoden geben/aendern den Link fuers "Grafik vollstaendig geladen"
    void        SetGrfArrivedLnk( const Link &rLnk ) { aGrfArrivedLnk = rLnk; }
    const Link& GetGrfArrivedLnk() const             { return aGrfArrivedLnk; }

    //ChgLink callen, innerhalb einer Action wird der Ruf verzoegert.
    void CallChgLnk();

    // Abfrage, ob der aktuelle Cursor eine Selektion aufspannt,
    // also, ob Mark gesetzt und SPoint und Mark unterschiedlich sind.
    BOOL HasSelection() const;

    // Abfrage, ob ueberhaupt eine Selektion existiert, sprich der akt. Cursor
    // aufgespannt oder nicht der einzigste ist.
    inline BOOL IsSelection() const;
    // returns if multiple cursors are available
    inline BOOL IsMultiSelection() const;

    // Abfrage, ob ein kompletter Absatz selektiert wurde
    BOOL IsSelFullPara() const;
    // Abfrage, ob die Selektion in einem Absatz ist
    inline BOOL IsSelOnePara() const;

    //Sollte fuer das Clipboard der WaitPtr geschaltet werden.
    BOOL ShouldWait() const;

    /*
     * liefert das SRectangle, auf dem der Cursor steht.
     */
    const SwRect &GetCharRect() const { return aCharRect; }
    /*
     * liefert zurueck, ob der Cursor sich ganz oder teilweise im
     * sichtbaren Bereich befindet.
     */
    BOOL IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }

    // gebe die aktuelle Seitennummer zurueck:
    // TRUE:  in der der Cursor steht
    // FALSE: die am oberen Rand sichtbar ist
    void GetPageNum( USHORT &rnPhyNum, USHORT &rnVirtNum,
                     BOOL bAtCrsrPos = TRUE, const BOOL bCalcFrm = TRUE );
    // bestimme in welche Richtung "leere Seiten" behandelt werden!
    // (wird benutzt im PhyPage.. )
    USHORT GetNextPrevPageNum( BOOL bNext = TRUE );

    // setze den Cursor auf die Seite "nPage" an den Anfang
    BOOL GotoPage( USHORT nPage );

    // gebe alle Dokumentseiten zurueck
    USHORT GetPageCnt();

    // Gehe zur naechsten Selection
    BOOL GoNextCrsr();
    // gehe zur vorherigen Selection
    BOOL GoPrevCrsr();

    // at CurCrsr.SPoint
    ::sw::mark::IMark* SetBookmark(
        const KeyCode&,
        const ::rtl::OUString& rName,
        const ::rtl::OUString& rShortName,
        IDocumentMarkAccess::MarkType eMark = IDocumentMarkAccess::BOOKMARK);
    bool GotoMark( const ::sw::mark::IMark* const pMark );    // sets CurCrsr.SPoint
    bool GotoMark( const ::sw::mark::IMark* const pMark, bool bAtStart );
    bool GoNextBookmark(); // true, if there was one
    bool GoPrevBookmark();

    bool IsFormProtected();
    ::sw::mark::IFieldmark* GetCurrentFieldmark();
    ::sw::mark::IFieldmark* GetFieldmarkAfter();
    ::sw::mark::IFieldmark* GetFieldmarkBefore();
    bool GotoFieldmark( const ::sw::mark::IFieldmark* const pMark );

    // aktualisiere den Crsrs, d.H. setze ihn wieder in den Content.
    // Das sollte nur aufgerufen werden, wenn der Cursor z.B. beim
    // Loeschen von Rahmen irgendwohin gesetzt wurde. Die Position
    // ergibt sich aus seiner aktuellen Position im Layout !!
    void UpdateCrsrPos();

    // returne den am akt. Cursor selektierten Text. Dieser wird mit
    // Felder etc. aufgefuellt!!
    String GetSelTxt() const;
    // gebe nur den Text ab der akt. Cursor Position zurueck (bis zum NodeEnde)
    String GetText() const;

    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    inline const SwTableNode* IsCrsrInTbl( BOOL bIsPtInTbl = TRUE ) const;
    // erfrage die Document - Layout - Position vom akt. Crsr
    inline Point& GetCrsrDocPos( BOOL bPoint = TRUE ) const;
    inline BOOL IsCrsrPtAtEnd() const;

    inline const  SwPaM* GetTblCrs() const;
    inline        SwPaM* GetTblCrs();

    BOOL IsTblComplex() const;
    BOOL IsTblComplexForChart();
    // erfrage die akt. TabellenSelektion als Text
    String GetBoxNms() const;

    // setze Crsr in die naechsten/vorherigen Celle
    BOOL GoNextCell( BOOL bAppendLine = TRUE );
    BOOL GoPrevCell();
    // gehe zu dieser Box (wenn vorhanden und in Tabelle!)
    BOOL GotoTable( const String& rName );

    // select a table row, column or box (based on the current cursor)
    BOOL SelTblRow() { return _SelTblRowOrCol( true  ); }
    BOOL SelTblCol() { return _SelTblRowOrCol( false ); }
    BOOL SelTblBox();
    // --> FME 2004-07-30 #i32329# Enhanced table selection
    BOOL SelTbl();
    // <--

    // zum naechsten/vorhergehenden Punkt auf gleicher Ebene
    BOOL GotoNextNum();
    BOOL GotoPrevNum();

        // zu diesem Gliederungspunkt
    BOOL GotoOutline( const String& rName );
        // zum naechsten/vorhergehenden oder angegebenen OultineNode
    void GotoOutline( USHORT nIdx );
        // suche die "Outline-Position" im Nodes-Array vom akt. Kaiptel
    USHORT GetOutlinePos( BYTE nLevel = UCHAR_MAX );
        // selektiere den angeben Bereich von OutlineNodes. Optional
        // inclusive der Childs. Die USHORT sind die Positionen im
        // OutlineNds-Array!! (EditShell)
    BOOL MakeOutlineSel( USHORT nSttPos, USHORT nEndPos,
                        BOOL bWithChilds = FALSE );

    BOOL GotoNextOutline();         // naechster Node mit Outline-Num.
    BOOL GotoPrevOutline();         // vorheriger Node mit Outline-Num.

    /** Delivers the current shell cursor

        Some operations have to run on the current cursor ring,
        some on the pTblCrsr (if exist) or the current cursor ring and
        some on the pTblCrsr or pBlockCrsr or the current cursor ring.
        This small function checks the existence and delivers the wished cursor.

        @param bBlock [bool]
        if the block cursor is of interest or not

        @return pTblCrsr if exist,
        pBlockCrsr if exist and of interest (param bBlock)
        otherwise pCurCrsr
    */
    SwShellCrsr* getShellCrsr( bool bBlock );
    const SwShellCrsr* getShellCrsr( bool bBlock ) const
        { return (const_cast<SwCrsrShell*>(this))->getShellCrsr( bBlock ); }

    FASTBOOL IsBlockMode() const { return 0 != pBlockCrsr; }
    const IBlockCursor* GetBlockCrsr() const { return pBlockCrsr; }
    IBlockCursor* GetBlockCrsr() { return pBlockCrsr; }

        // ist der Crsr in einer Tabelle und ist die Selection ueber
        // zwei Spalten
    BOOL IsTableMode() const { return 0 != pTblCrsr; }

        // erfrage den Tabellen Crsr; ausserhalb von Tabellen immer 0
    const SwShellTableCrsr* GetTableCrsr() const { return pTblCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return pTblCrsr; }
    USHORT UpdateTblSelBoxes();

    BOOL GotoFtnTxt();      // springe aus dem Content zur Fussnote
    BOOL GotoFtnAnchor();   // springe aus der Fussnote zum Anker
    BOOL GotoPrevFtnAnchor();
    BOOL GotoNextFtnAnchor();

    BOOL GotoFlyAnchor();       // springe aus dem Rahmen zum Anker
    BOOL GotoHeaderTxt();       // springe aus dem Content zum Header
    BOOL GotoFooterTxt();       // springe aus dem Content zum Footer
    // springe in den Header/Footer des angegebenen oder akt. PageDesc
    BOOL SetCrsrInHdFt( USHORT nDescNo = USHRT_MAX,
                            BOOL bInHeader = TRUE );
    // is point of cursor in header/footer. pbInHeader return TRUE if it is
    // in a headerframe otherwise in a footerframe
    BOOL IsInHeaderFooter( BOOL* pbInHeader = 0 ) const;

    // springe zum naechsten Verzeichnis [mit dem Namen]
    BOOL GotoNextTOXBase( const String* = 0 );
    // springe zum vorherigen Verzeichnis [mit dem Namen]
    BOOL GotoPrevTOXBase( const String* = 0 );
    BOOL GotoTOXMarkBase();     // springe zum Verzeichnis vom TOXMark
    // springe zum naechsten (vorherigen) Verzeichniseintrag
    BOOL GotoNxtPrvTOXMark( BOOL bNext = TRUE );
    // Zur naechsten/ vorherigen Verzeichnismarke dieses Typs traveln
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rStart, SwTOXSearch eDir );

    // springe zum naechsten (vorherigen) Tabellenformel
    // optional auch nur zu kaputten Formeln springen
    BOOL GotoNxtPrvTblFormula( BOOL bNext = TRUE,
                                    BOOL bOnlyErrors = FALSE );
    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    BOOL SelectNxtPrvHyperlink( BOOL bNext = TRUE );

    // springe zu dieser Refmark
    BOOL GotoRefMark( const String& rRefMark, USHORT nSubType = 0,
                            USHORT nSeqNo = 0 );

    // hole vom Start/Ende der akt. Selection das nte Zeichen
    sal_Unicode GetChar( BOOL bEnd = TRUE, long nOffset = 0 );
    // erweiter die akt. Selection am Anfang/Ende um n Zeichen
    BOOL ExtendSelection( BOOL bEnd = TRUE, xub_StrLen nCount = 1 );
    // setze nur den sichtbaren Cursor an die angegebene Dokument-Pos.
    // returnt FALSE: wenn der ob der SPoint vom Layout korrigiert wurde.
    // (wird zum Anzeigen von Drag&Drop/Copy-Cursor benoetigt)
    BOOL SetVisCrsr( const Point &rPt );
    inline void UnSetVisCrsr();

    // springe zum nachsten/vorherigen Feld des entsprechenden Types
    BOOL MoveFldType( const SwFieldType* pFldType, BOOL bNext,
                                            USHORT nSubType = USHRT_MAX,
                                            USHORT nResType = USHRT_MAX );
    // springe genau zu diesem Feld
    BOOL GotoFld( const SwFmtFld& rFld );

    // returne die Anzahl der Cursor im Ring (Flag besagt ob man nur
    // aufgepspannte haben will - sprich etwas selektiert ist (Basic))
    USHORT GetCrsrCnt( BOOL bAll = TRUE ) const;

    // Char Travelling - Methoden (in crstrvl1.cxx)
    BOOL GoStartWord();
    BOOL GoEndWord();
    BOOL GoNextWord();
    BOOL GoPrevWord();
    BOOL GoNextSentence();
    BOOL GoPrevSentence();
    BOOL GoStartSentence();
    BOOL GoEndSentence();
    BOOL SelectWord( const Point* pPt = 0 );
    BOOL ExpandToSentenceBorders();

    // Position vom akt. Cursor erfragen
    BOOL IsStartWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES )const;
    BOOL IsEndWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    BOOL IsInWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    BOOL IsStartSentence() const;
    BOOL IsEndSentence() const;
    BOOL IsSttPara() const;
    BOOL IsEndPara() const;
    BOOL IsStartOfDoc() const;
    BOOL IsEndOfDoc() const;
    BOOL IsInFrontOfLabel() const;
    BOOL IsAtLeftMargin()   const       { return IsAtLRMargin( TRUE ); }
    BOOL IsAtRightMargin(BOOL bAPI = FALSE) const   { return IsAtLRMargin( FALSE, bAPI ); }

    // loesche alle erzeugten Crsr, setze den Tabellen-Crsr und den letzten
    // Cursor auf seinen TextNode (oder StartNode?).
    // Beim naechsten ::GetCrsr werden sie wieder alle erzeugt.
    // Wird fuers Drag&Drop/ClipBorad-Paste in Tabellen benoetigt.
    BOOL ParkTblCrsr();

    // gibt es nicht aufgespannte Attribute?
    BOOL IsGCAttr() const { return bGCAttr; }
    void ClearGCAttr() { bGCAttr = FALSE; }
    void    UpdateAttr() {  bGCAttr = TRUE; }

    // ist das gesamte Dokument geschuetzt/versteckt?? (fuer UI,..)
    BOOL IsAllProtect() const { return bAllProtect; }

#ifdef SW_CRSR_TIMER
    // setze das Flag am VisCrsr, ob dieser ueber Timer getriggert (TRUE)
    // oder direkt (FALSE) angezeigt wird. (default ist Timer getriggert)
    BOOL ChgCrsrTimerFlag( BOOL bTimerOn = TRUE );
#endif

    BOOL BasicActionPend() const    { return nBasicActionCnt != nStartAction; }

        // springe zum benannten Bereich
    BOOL GotoRegion( const String& rName );

    // zeige die aktuelle Selektion an
    virtual void MakeSelVisible();

    // setzte den Cursor auf einen NICHT geschuetzten/versteckten Node
    BOOL FindValidCntntNode( BOOL bOnlyText = FALSE );

    BOOL GetContentAtPos( const Point& rPt,
                            SwContentAtPos& rCntntAtPos,
                            BOOL bSetCrsr = FALSE,
                            SwRect* pFldRect = 0 );

    // --> OD 2008-06-19 #i90516#
    const SwPostItField* GetPostItFieldAtCursor() const;
    // <--

    // get smart tags at point position
    void GetSmartTagTerm( const Point& rPt,
                          SwRect& rSelectRect,
                          ::com::sun::star::uno::Sequence< rtl::OUString >& rSmartTagTypes,
                          ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                          ::com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& rRange );

    // get smart tags at current cursor position
    void GetSmartTagTerm( ::com::sun::star::uno::Sequence< rtl::OUString >& rSmartTagTypes,
                          ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > >& rStringKeyMaps,
                          ::com::sun::star::uno::Reference<com::sun::star::text::XTextRange>& rRange ) const;

    BOOL IsPageAtPos( const Point &rPt ) const;

        // Attribut selelktieren
    BOOL SelectTxtAttr( USHORT nWhich, BOOL bExpand, const SwTxtAttr* pAttr = 0 );
    BOOL GotoINetAttr( const SwTxtINetFmt& rAttr );
    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    BOOL CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void SaveTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();
    BOOL EndAllTblBoxEdit();

    // wird gerufen, wenn eine Tabellenselektion im UpdateCrsr erzeugt wird,
    // ohne das die UI davon etaws weiss
    virtual void NewCoreSelection();

    void SetSelTblCells( BOOL bFlag )           { bSelTblCells = bFlag; }
    BOOL IsSelTblCells() const                  { return bSelTblCells; }

    BOOL IsAutoUpdateCells() const              { return bAutoUpdateCells; }
    void SetAutoUpdateCells( BOOL bFlag )       { bAutoUpdateCells = bFlag; }

    BOOL GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                            SwRect& rRect, short& rOrient );
    BOOL SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode );

    const SwRedline* SelNextRedline();
    const SwRedline* SelPrevRedline();
    const SwRedline* GotoRedline( USHORT nArrPos, BOOL bSelect = FALSE );

    // is cursor or the point in/over a vertical formatted text?
    BOOL IsInVerticalText( const Point* pPt = 0 ) const;
    // is cursor or the point in/over a right to left formatted text?
    BOOL IsInRightToLeftText( const Point* pPt = 0 ) const;

    // If the current cursor position is inside a hidden range, the hidden range
    // is selected and true is returned:
    bool SelectHiddenRange();

    // remove all invalid cursors
    void ClearUpCrsrs();

    // OD 11.02.2003 #100556# - set/get flag to allow/avoid execution of macros
    inline void SetMacroExecAllowed( const bool _bMacroExecAllowed )
    {
        mbMacroExecAllowed = _bMacroExecAllowed;
    }
    inline bool IsMacroExecAllowed()
    {
        return mbMacroExecAllowed;
    }

    // #111827#
    /**
       Returns textual description of the current selection.

       - If the current selection is a multi-selection the result is
         STR_MULTISEL.
       - Else the result is the text of the selection.

       @return the textual description of the current selection
     */
    String GetCrsrDescr() const;

    SwRect GetRectOfCurrentChar();
};


// Cursor Inlines:
inline SwMoveFnCollection* SwCrsrShell::MakeFindRange(
            USHORT nStt, USHORT nEnd, SwPaM* pPam ) const
{
    return pCurCrsr->MakeFindRange( (SwDocPositions)nStt, (SwDocPositions)nEnd, pPam );
}

inline SwCursor* SwCrsrShell::GetSwCrsr( BOOL bMakeTblCrsr ) const
{
    return (SwCursor*)GetCrsr( bMakeTblCrsr );
}

inline SwPaM* SwCrsrShell::GetStkCrsr() const { return pCrsrStk; }

inline void SwCrsrShell::SetMark() { pCurCrsr->SetMark(); }

inline BOOL SwCrsrShell::HasMark() { return( pCurCrsr->HasMark() ); }

inline BOOL SwCrsrShell::IsSelection() const
{
    return IsTableMode() || pCurCrsr->HasMark() ||
            pCurCrsr->GetNext() != pCurCrsr;
}
inline BOOL SwCrsrShell::IsMultiSelection() const
{
    return pCurCrsr->GetNext() != pCurCrsr;
}

inline BOOL SwCrsrShell::IsSelOnePara() const
{
    return pCurCrsr == pCurCrsr->GetNext() &&
           pCurCrsr->GetPoint()->nNode == pCurCrsr->GetMark()->nNode;
}

inline const SwTableNode* SwCrsrShell::IsCrsrInTbl( BOOL bIsPtInTbl ) const
{
    return pCurCrsr->GetNode( bIsPtInTbl )->FindTableNode();
}

inline BOOL SwCrsrShell::IsCrsrPtAtEnd() const
{
    return pCurCrsr->End() == pCurCrsr->GetPoint();
}

inline Point& SwCrsrShell::GetCrsrDocPos( BOOL bPoint ) const
{
    return bPoint ? pCurCrsr->GetPtPos() : pCurCrsr->GetMkPos();
}

inline const SwPaM* SwCrsrShell::GetTblCrs() const
{
    return pTblCrsr;
}

inline SwPaM* SwCrsrShell::GetTblCrs()
{
    return pTblCrsr;
}

inline void SwCrsrShell::UnSetVisCrsr()
{
    pVisCrsr->Hide();
    pVisCrsr->SetDragCrsr( FALSE );
}

#endif  // _CRSRSH_HXX
