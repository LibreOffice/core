/*************************************************************************
 *
 *  $RCSfile: crsrsh.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-12 08:13:15 $
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
#ifndef _CRSRSH_HXX
#define _CRSRSH_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>          // fuer SWPOSDOC
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>           // fuer ViewShell
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>           // fuer SwClient
#endif
#ifndef _CSHTYP_HXX
#include <cshtyp.hxx>           // fuer die CursorShell Typen
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>          // fuer die CursorMove-Staties
#endif
#ifndef _BKMRKE_HXX //autogen
#include <bkmrke.hxx>
#endif
#ifndef _TOXE_HXX
#include <toxe.hxx>             // SwTOXSearchDir
#endif

#if defined(PRODUCT) && !defined(WIN)
// fuer die Inline-Methoden
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#define CRSR_INLINE inline
#else
#define CRSR_INLINE
#endif

// einige Forward Deklarationen

class KeyCode;
class Region;
class SfxItemSet;
class SfxPoolItem;
class SpellCheck;
class SwBookmark;
class SwCntntFrm;
class SwCrsrShell;
class SwCursor;
class SwField;
class SwFieldType;
class SwFmt;
class SwFmtFld;
class SwIndex;
class SwNodeIndex;
class SwNode;
class SwNodes;
class SwPaM;
class SwSelBoxes;
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
struct SwPosition;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

// enum und struktur, um ueber die Doc-Position Informationen zu erfragen

struct SwContentAtPos
{
    enum IsAttrAtPos
    {
        SW_NOTHING      = 0x0000,
        SW_FIELD        = 0x0001,
        SW_CLICKFIELD   = 0x0002,
        SW_FTN          = 0x0004,
        SW_INETATTR     = 0x0008,
        SW_TABLEBOXFML  = 0x0010,
        SW_REDLINE      = 0x0020,
        SW_OUTLINE      = 0x0040,
        SW_TOXMARK      = 0x0080,
        SW_REFMARK      = 0x0100
#ifndef PRODUCT
        ,SW_CURR_ATTRS      = 0x4000        // nur zum Debuggen
        ,SW_TABLEBOXVALUE   = 0x8000        // nur zum Debuggen
#endif
    } eCntntAtPos;

    union {
        const SwField* pFld;
        const SfxPoolItem* pAttr;
        const SwRedline* pRedl;
    } aFnd;

    String sStr;
    const SwTxtAttr* pFndTxtAttr;

    SwContentAtPos( int eGetAtPos = 0xffff )
        : eCntntAtPos( (IsAttrAtPos)eGetAtPos )
    {
        aFnd.pFld = 0;
        pFndTxtAttr = 0;
    }

    // befindet sich der Node in einem geschuetzten Bereich?
    FASTBOOL IsInProtectSect() const;
};


// defines fuers GetCharCount
#define GETCHARCOUNT_NONE       0
#define GETCHARCOUNT_PARA       1
#define GETCHARCOUNT_SECTION    2


// ReturnWerte von SetCrsr (werden verodert)
const int   CRSR_NOERROR =  0x00,
            CRSR_POSOLD =   0x01,   // Cursor bleibt an alter Doc-Position
            CRSR_POSCHG =   0x02;   // Position vom Layout veraendert




// die Cursor - Shell
class SwCrsrShell : public ViewShell, public SwModify
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;
    friend class SwChgLinkFlag;

    //Braucht den Crsr als IntrnlCrsr.
    friend void GetTblSel( const SwCrsrShell&, SwSelBoxes& rBoxes,
                           const SwTblSearchType );
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

    /*
     * Mit den Methoden SttCrsrMove und EndCrsrMove wird dieser Zaehler
     * Inc-/Decrementiert. Solange der Zaehler ungleich 0 ist, erfolgt
     * auf den akt. Cursor kein Update. Dadurch koennen "komplizierte"
     * Cursorbewegungen (ueber Find()) realisiert werden.
     */
    USHORT nCrsrMove;
    USHORT nBasicActionCnt;     // Actions, die vom Basic geklammert wurden
    CrsrMoveState eMvState;     // Status fuers Crsr-Travelling - GetCrsrOfst

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

    void UpdateCrsr( USHORT eFlags
                            =SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
                     BOOL bIdleEnd = FALSE );

    void _ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing );

    FASTBOOL LeftRight( BOOL, USHORT );
    FASTBOOL UpDown( BOOL, USHORT );
    FASTBOOL LRMargin( BOOL, BOOL bAPI = FALSE );
    FASTBOOL IsAtLRMargin( BOOL, BOOL bAPI = FALSE ) const;
    FASTBOOL SttEndDoc( BOOL bStt );

typedef FASTBOOL (SwCursor:: *FNCrsr)();
    FASTBOOL CallCrsrFN( FNCrsr );

    const SwRedline* _GotoRedline( USHORT nArrPos, BOOL bSelect );

protected:

    CRSR_INLINE SwMoveFnCollection* MakeFindRange( USHORT, USHORT, SwPaM* ) const;

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

    USHORT IncBasicAction()             { return ++nBasicActionCnt; }
    USHORT DecBasicAction()             { return --nBasicActionCnt; }

    // Setzt alle PaMs in OldNode auf NewPos + Offset
    void PaMCorrAbs(const SwNodeIndex &rOldNode, const SwPosition &rNewPos,
                    const xub_StrLen nOffset = 0 );
    // Setzt alle PaMs im Bereich von [StartNode, EndNode] nach NewPos
    void PaMCorrAbs(const SwNodeIndex &rStartNode, const SwNodeIndex &rEndNode,
                    const SwPosition &rNewPos );

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin,
                SwRootFrm * = 0, const SwViewOption *pOpt = 0 );
    // verkleideter Copy-Constructor
    SwCrsrShell( SwCrsrShell& rShell, Window *pWin );
    virtual ~SwCrsrShell();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    // neuen Cusror erzeugen und den alten anhaengen
    SwPaM * CreateCrsr();
    // loesche den aktuellen Cursor und der folgende wird zum Aktuellen
    FASTBOOL DestroyCrsr();
    // TableCursor in normale Cursor verwandeln, Tablemode aufheben
    void TblCrsrToCursor();

    SwPaM* GetCrsr( FASTBOOL bMakeTblCrsr = TRUE ) const;
    CRSR_INLINE SwCursor* GetSwCrsr( FASTBOOL bMakeTblCrsr = TRUE ) const;
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
    CRSR_INLINE SwPaM* GetStkCrsr() const;

    // Start der Klammerung, SV-Cursor und selektierte Bereiche hiden
    void StartAction();
    // Ende der Klammerung, SV-Cursor und selektierte Bereiche anzeigen
    void EndAction( const BOOL bIdleEnd = FALSE );

    USHORT GetBasicActionCnt() const    { return nBasicActionCnt; }

    // Basiscursortravelling
    long GetUpDownX() const             { return nUpDownX; }

    FASTBOOL Left( USHORT nCnt = 1 )    { return LeftRight( TRUE, nCnt ); }
    FASTBOOL Right( USHORT nCnt = 1 )   { return LeftRight( FALSE, nCnt ); }
    FASTBOOL Up( USHORT nCnt = 1 )      { return UpDown( TRUE, nCnt ); }
    FASTBOOL Down( USHORT nCnt = 1 )    { return UpDown( FALSE, nCnt ); }
    FASTBOOL LeftMargin()               { return LRMargin( TRUE ); }
    FASTBOOL RightMargin(BOOL bAPI = FALSE) { return LRMargin( FALSE, bAPI ); }
    FASTBOOL SttDoc()                   { return SttEndDoc( TRUE ); }
    FASTBOOL EndDoc()                   { return SttEndDoc( FALSE ); }

    FASTBOOL MovePage( SwWhichPage, SwPosPage );
    FASTBOOL MovePara( SwWhichPara, SwPosPara );
    FASTBOOL MoveSection( SwWhichSection, SwPosSection );
    FASTBOOL MoveTable( SwWhichTable, SwPosTable );
    FASTBOOL MoveColumn( SwWhichColumn, SwPosColumn );
    FASTBOOL MoveRegion( SwWhichRegion, SwPosRegion );

    // die Suchfunktionen
    ULONG Find( const com::sun::star::util::SearchOptions& rSearchOpt,
                SwDocPositions eStart, SwDocPositions eEnde,
                FindRanges eRng, int bReplace = FALSE );

    ULONG Find( const SwTxtFmtColl& rFmtColl,
                SwDocPositions eStart, SwDocPositions eEnde,
                FindRanges eRng, const SwTxtFmtColl* pReplFmt = 0 );

    ULONG Find( const SfxItemSet& rSet, FASTBOOL bNoCollections,
                SwDocPositions eStart, SwDocPositions eEnde,
                FindRanges eRng,
                const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                const SfxItemSet* rReplSet = 0 );

    // Positionieren des Cursors
    // returnt
    //  CRSR_POSCHG: wenn der ob der SPoint vom Layout korrigiert wurde.
    //  CRSR_POSOLD: wenn der Crsr nicht veraendert wurde
    int SetCrsr( const Point &rPt, BOOL bOnlyText = FALSE );

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
    CRSR_INLINE void SetMark();
    CRSR_INLINE FASTBOOL HasMark();

    void ClearMark();
    void SwapPam();
    FASTBOOL ChgCurrPam( const Point & rPt,
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
    FASTBOOL Pop( BOOL bOldCrsr = TRUE );
    /*
     * Verbinde zwei Cursor miteinander.
     * Loesche vom Stack den obersten und setzen dessen Mark im Aktuellen.
     */
    void Combine();

#if defined( PRODUCT )
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
    // Methoden zum Anzeigen bzw. Verstecken der selektierten Bereiche mit
    // dem sichtbaren Cursor
    void BasicShowCrsrs()
        { bBasicHideCrsr = FALSE; bSVCrsrVis = TRUE; ShowCrsrs(TRUE); }
    void BasicHideCrsrs()
        { HideCrsrs(); bBasicHideCrsr = TRUE; bSVCrsrVis = FALSE; }
    FASTBOOL IsBasicHideCrsr() const { return bBasicHideCrsr; }

    FASTBOOL IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( FASTBOOL bFlag ) { bOverwriteCrsr = bFlag; }

    // gebe den aktuellen Frame, in dem der Cursor steht, zurueck
    SwCntntFrm *GetCurrFrm( const BOOL bCalcFrm = TRUE ) const;

    //TRUE wenn der Crsr wenn der Crsr wegen Readonly gehidet ist,
    //FALSE wenn der arbeitet (trotz Readonly).
    FASTBOOL IsCrsrReadonly() const;
    // Cursor steht in etwas geschuetztem oder in die Selektion umspannt
    // etwas geschuetztes.
    FASTBOOL HasReadonlySel() const;
    // darf der Cursor in ReadOnlyBereiche?
    FASTBOOL IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }
    void SetReadOnlyAvailable( BOOL bFlag );
    FASTBOOL IsOverReadOnlyPos( const Point& rPt ) const;

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
    FASTBOOL HasSelection();

    // Abfrage, ob ueberhaupt eine Selektion existiert, sprich der akt. Cursor
    // aufgespannt oder nicht der einzigste ist.
    CRSR_INLINE FASTBOOL IsSelection() const;

    // Abfrage, ob ein kompletter Absatz selektiert wurde
    FASTBOOL IsSelFullPara() const;
    // Abfrage, ob die Selektion in einem Absatz ist
    CRSR_INLINE FASTBOOL IsSelOnePara() const;

    //Sollte fuer das Clipboard der WaitPtr geschaltet werden.
    FASTBOOL ShouldWait() const;

    /*
     * liefert das SRectangle, auf dem der Cursor steht.
     */
    const SwRect &GetCharRect() const { return aCharRect; }
    /*
     * liefert zurueck, ob der Cursor sich ganz oder teilweise im
     * sichtbaren Bereich befindet.
     */
    FASTBOOL IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }

    // gebe die aktuelle Seitennummer zurueck:
    // TRUE:  in der der Cursor steht
    // FALSE: die am oberen Rand sichtbar ist
    void GetPageNum( USHORT &rnPhyNum, USHORT &rnVirtNum,
                     BOOL bAtCrsrPos = TRUE, const BOOL bCalcFrm = TRUE );
    // bestimme in welche Richtung "leere Seiten" behandelt werden!
    // (wird benutzt im PhyPage.. )
    USHORT GetNextPrevPageNum( BOOL bNext = TRUE );

    // setze den Cursor auf die Seite "nPage" an den Anfang
    FASTBOOL GotoPage( USHORT nPage );

    // gebe alle Dokumentseiten zurueck
    USHORT GetPageCnt();

    // Gehe zur naechsten Selection
    FASTBOOL GoNextCrsr();
    // gehe zur vorherigen Selection
    FASTBOOL GoPrevCrsr();

    // am CurCrsr.SPoint
    FASTBOOL SetBookmark( const KeyCode&, const String& rName,
                const String& rShortName, BOOKMARK_TYPE eMark  = BOOKMARK );
    FASTBOOL GotoBookmark( USHORT );    // setzt CurCrsr.SPoint
    FASTBOOL GotoBookmark( USHORT nPos, BOOL bAtStart ); //
    FASTBOOL GoNextBookmark(); // TRUE, wenn's noch eine gab
    FASTBOOL GoPrevBookmark();
    USHORT GetBookmarkCnt(BOOL bBkmrk = FALSE) const;
    SwBookmark& GetBookmark( USHORT, BOOL bBkmrk = FALSE );
    void DelBookmark( USHORT );
    void DelBookmark( const String& rName );
    USHORT FindBookmark( const String& rName );
        // erzeugt einen eindeutigen Namen. Der Name selbst muss vorgegeben
        // werden, es wird dann bei gleichen Namen nur durchnumeriert.
    void MakeUniqueBookmarkName( String& rNm );

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
    // retrurne die Anzahl der selektierten Zeichen.
    // Falls keine Selektion vorliegt entscheided nType was selektiert wird
    // bIntrnlChar besagt ob interne Zeichen erhalten bleiben (TRUE) oder
    // ob sie expandiert werden (z.B Felder/...)
    ULONG GetCharCount( USHORT nType, BOOL bIntrnlChrs = TRUE ) const;

    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    CRSR_INLINE const SwTableNode* IsCrsrInTbl( BOOL bIsPtInTbl = TRUE ) const;
    // erfrage die Document - Layout - Position vom akt. Crsr
    CRSR_INLINE Point& GetCrsrDocPos( BOOL bPoint = TRUE ) const;
    CRSR_INLINE FASTBOOL IsCrsrPtAtEnd() const;

    CRSR_INLINE const   SwPaM* GetTblCrs() const;
    CRSR_INLINE         SwPaM* GetTblCrs();

    FASTBOOL IsTblComplex() const;
    FASTBOOL IsTblComplexForChart() const;
    // erfrage die akt. TabellenSelektion als Text
    String GetBoxNms() const;

    // setze Crsr in die naechsten/vorherigen Celle
    FASTBOOL GoNextCell( BOOL bAppendLine = TRUE );
    FASTBOOL GoPrevCell();
    // gehe zu dieser Box (wenn vorhanden und in Tabelle!)
    FASTBOOL GotoTblBox( const String& rName );
    FASTBOOL GotoTable( const String& rName );

    // selectiere diese Zeile/Spalte
    FASTBOOL SelTblRow();
    FASTBOOL SelTblCol();

    // zum naechsten/vorhergehenden Punkt auf gleicher Ebene
    FASTBOOL GotoNextNum();
    FASTBOOL GotoPrevNum();

        // zu diesem Gliederungspunkt
    FASTBOOL GotoOutline( const String& rName );
        // zum naechsten/vorhergehenden oder angegebenen OultineNode
    void GotoOutline( USHORT nIdx );
        // suche die "Outline-Position" im Nodes-Array vom akt. Kaiptel
    USHORT GetOutlinePos( BYTE nLevel = UCHAR_MAX );
        // selektiere den angeben Bereich von OutlineNodes. Optional
        // inclusive der Childs. Die USHORT sind die Positionen im
        // OutlineNds-Array!! (EditShell)
    FASTBOOL MakeOutlineSel( USHORT nSttPos, USHORT nEndPos,
                        BOOL bWithChilds = FALSE );

    FASTBOOL GotoNextOutline();         // naechster Node mit Outline-Num.
    FASTBOOL GotoPrevOutline();         // vorheriger Node mit Outline-Num.

        // ist der Crsr in einer Tabelle und ist die Selection ueber
        // zwei Spalten
    FASTBOOL IsTableMode() const { return 0 != pTblCrsr; }

        // erfrage den Tabellen Crsr; ausserhalb von Tabellen immer 0
    const SwShellTableCrsr* GetTableCrsr() const { return pTblCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return pTblCrsr; }
    USHORT UpdateTblSelBoxes();

    FASTBOOL GotoFtnTxt();      // springe aus dem Content zur Fussnote
    FASTBOOL GotoFtnAnchor();   // springe aus der Fussnote zum Anker
    FASTBOOL GotoNextFtnAnchor();
    FASTBOOL GotoPrevFtnAnchor();
    FASTBOOL GotoNextFtnCntnt();
    FASTBOOL GotoPrevFtnCntnt();

    FASTBOOL GotoFlyTxt();          // springe aus dem Content zum "naechsten" Rahmen
    FASTBOOL GotoFlyAnchor();       // springe aus dem Rahmen zum Anker
    FASTBOOL GotoHeaderTxt();       // springe aus dem Content zum Header
    FASTBOOL GotoFooterTxt();       // springe aus dem Content zum Footer
    // springe in den Header/Footer des angegebenen oder akt. PageDesc
    FASTBOOL SetCrsrInHdFt( USHORT nDescNo = USHRT_MAX,
                            FASTBOOL bInHeader = TRUE );
    // is point of cursor in header/footer. pbInHeader return TRUE if it is
    // in a headerframe otherwise in a footerframe
    FASTBOOL IsInHeaderFooter( FASTBOOL* pbInHeader = 0 ) const;

    // springe zum naechsten Verzeichnis [mit dem Namen]
    FASTBOOL GotoNextTOXBase( const String* = 0 );
    // springe zum vorherigen Verzeichnis [mit dem Namen]
    FASTBOOL GotoPrevTOXBase( const String* = 0 );
    FASTBOOL GotoTOXMarkBase();     // springe zum Verzeichnis vom TOXMark
    // springe zum naechsten (vorherigen) Verzeichniseintrag
    FASTBOOL GotoNxtPrvTOXMark( BOOL bNext = TRUE );
    // Zur naechsten/ vorherigen Verzeichnismarke dieses Typs traveln
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rStart, SwTOXSearch eDir );

    // springe zum naechsten (vorherigen) Tabellenformel
    // optional auch nur zu kaputten Formeln springen
    FASTBOOL GotoNxtPrvTblFormula( BOOL bNext = TRUE,
                                    BOOL bOnlyErrors = FALSE );

    // springe zu dieser Refmark
    FASTBOOL GotoRefMark( const String& rRefMark, USHORT nSubType = 0,
                            USHORT nSeqNo = 0 );

    // hole vom Start/Ende der akt. Selection das nte Zeichen
    sal_Unicode GetChar( BOOL bEnd = TRUE, long nOffset = 0 );
    // erweiter die akt. Selection am Anfang/Ende um n Zeichen
    FASTBOOL ExtendSelection( BOOL bEnd = TRUE, xub_StrLen nCount = 1 );
    // setze nur den sichtbaren Cursor an die angegebene Dokument-Pos.
    // returnt FALSE: wenn der ob der SPoint vom Layout korrigiert wurde.
    // (wird zum Anzeigen von Drag&Drop/Copy-Cursor benoetigt)
    FASTBOOL SetVisCrsr( const Point &rPt );
    CRSR_INLINE void UnSetVisCrsr();

    // springe zum nachsten/vorherigen Feld des entsprechenden Types
    FASTBOOL MoveFldType( const SwFieldType* pFldType, BOOL bNext,
                                            USHORT nSubType = USHRT_MAX,
                                            USHORT nResType = USHRT_MAX );
    // springe genau zu diesem Feld
    FASTBOOL GotoFld( const SwFmtFld& rFld );

    // returne die Anzahl der Cursor im Ring (Flag besagt ob man nur
    // aufgepspannte haben will - sprich etwas selektiert ist (Basic))
    USHORT GetCrsrCnt( BOOL bAll = TRUE ) const;

    // Char Travelling - Methoden (in crstrvl1.cxx)
    FASTBOOL IsStartWord()const;
    FASTBOOL IsEndWord() const;
    FASTBOOL IsInWord() const;
    FASTBOOL GoStartWord();
    FASTBOOL GoEndWord();
    FASTBOOL GoNextWord();
    FASTBOOL GoPrevWord();
    FASTBOOL GoNextSentence();
    FASTBOOL GoPrevSentence();
    FASTBOOL SelectWord( const Point* pPt = 0 );

    // Abfrage vom CrsrTravelling Status
    CrsrMoveState GetMoveState() const { return eMvState; }

    // Position vom akt. Cursor erfragen
    FASTBOOL IsStartOfDoc() const;
    FASTBOOL IsEndOfDoc() const;
    FASTBOOL IsSttPara() const;
    FASTBOOL IsEndPara() const;
    FASTBOOL IsAtLeftMargin()   const       { return IsAtLRMargin( TRUE ); }
    FASTBOOL IsAtRightMargin(BOOL bAPI = FALSE) const   { return IsAtLRMargin( FALSE, bAPI ); }

    // loesche alle erzeugten Crsr, setze den Tabellen-Crsr und den letzten
    // Cursor auf seinen TextNode (oder StartNode?).
    // Beim naechsten ::GetCrsr werden sie wieder alle erzeugt.
    // Wird fuers Drag&Drop/ClipBorad-Paste in Tabellen benoetigt.
    FASTBOOL ParkTblCrsr();

    // erfrage die selektierte "Region" aller Cursor (fuer D&D auf Mac)
    Region GetCrsrRegion() const;

    // gibt es nicht aufgespannte Attribute?
    FASTBOOL IsGCAttr() const { return bGCAttr; }
    void    ClearGCAttr() { bGCAttr = FALSE; }
    void    UpdateAttr() {  bGCAttr = TRUE; }

    // ist das gesamte Dokument geschuetzt/versteckt?? (fuer UI,..)
    FASTBOOL IsAllProtect() const { return bAllProtect; }

#ifdef SW_CRSR_TIMER
    // setze das Flag am VisCrsr, ob dieser ueber Timer getriggert (TRUE)
    // oder direkt (FALSE) angezeigt wird. (default ist Timer getriggert)
    FASTBOOL ChgCrsrTimerFlag( BOOL bTimerOn = TRUE );
#endif

    // steht der Curor auf einem "Symbol"-Zeichen
    FASTBOOL IsInSymbolFont() const;

    BOOL BasicActionPend() const    { return nBasicActionCnt != nStartAction; }

        // springe zum benannten Bereich
    FASTBOOL GotoRegion( const String& rName );

    // zeige die aktuelle Selektion an
    virtual void MakeSelVisible();

    // setzte den Cursor auf einen NICHT geschuetzten/versteckten Node
    FASTBOOL FindValidCntntNode( BOOL bOnlyText = FALSE );

    FASTBOOL GetContentAtPos( const Point& rPt,
                            SwContentAtPos& rCntntAtPos,
                            FASTBOOL bSetCrsr = FALSE,
                            SwRect* pFldRect = 0 );

        // Attribut selelktieren
    FASTBOOL SelectTxtAttr( USHORT nWhich, BOOL bExpand = FALSE,
                            const SwTxtAttr* pAttr = 0 );
    FASTBOOL GotoINetAttr( const SwTxtINetFmt& rAttr );
    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    FASTBOOL CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void SaveTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();
    FASTBOOL EndAllTblBoxEdit();

    // wird gerufen, wenn eine Tabellenselektion im UpdateCrsr erzeugt wird,
    // ohne das die UI davon etaws weiss
    virtual void NewCoreSelection();

    void SetSelTblCells( BOOL bFlag )           { bSelTblCells = bFlag; }
    BOOL IsSelTblCells() const                  { return bSelTblCells; }

    BOOL IsAutoUpdateCells() const              { return bAutoUpdateCells; }
    void SetAutoUpdateCells( BOOL bFlag )       { bAutoUpdateCells = bFlag; }

    FASTBOOL GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                            SwRect& rRect, SwHoriOrient& rOrient );
    FASTBOOL SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode );

    const SwRedline* SelNextRedline();
    const SwRedline* SelPrevRedline();
    const SwRedline* GotoRedline( USHORT nArrPos, BOOL bSelect = FALSE );
};


class SwChgLinkFlag
{
    BOOL bOldFlag;
    SwCrsrShell& rCrsrShell;
    long nLeftFrmPos;
public:
    SwChgLinkFlag( SwCrsrShell& rShell );
    ~SwChgLinkFlag();
};



// Cursor Inlines:

#if defined(PRODUCT) && !defined(WIN)

inline SwMoveFnCollection* SwCrsrShell::MakeFindRange(
            USHORT nStt, USHORT nEnd, SwPaM* pPam ) const
{
    return pCurCrsr->MakeFindRange( (SwDocPositions)nStt, (SwDocPositions)nEnd, pPam );
}

inline SwCursor* SwCrsrShell::GetSwCrsr( FASTBOOL bMakeTblCrsr ) const
{
    return (SwCursor*)GetCrsr( bMakeTblCrsr );
}

inline SwPaM* SwCrsrShell::GetStkCrsr() const { return pCrsrStk; }

inline void SwCrsrShell::SetMark() { pCurCrsr->SetMark(); }

inline FASTBOOL SwCrsrShell::HasMark() { return( pCurCrsr->HasMark() ); }

inline FASTBOOL SwCrsrShell::IsSelection() const
{
    return IsTableMode() || pCurCrsr->HasMark() ||
            pCurCrsr->GetNext() != pCurCrsr;
}

inline FASTBOOL SwCrsrShell::IsSelOnePara() const
{
    return pCurCrsr == pCurCrsr->GetNext() &&
           pCurCrsr->GetPoint()->nNode == pCurCrsr->GetMark()->nNode;
}

inline const SwTableNode* SwCrsrShell::IsCrsrInTbl( BOOL bIsPtInTbl ) const
{
    return pCurCrsr->GetNode( bIsPtInTbl )->FindTableNode();
}

inline FASTBOOL SwCrsrShell::IsCrsrPtAtEnd() const
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

#endif


#endif  // _CRSRSH_HXX
