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

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>			// fuer ViewShell
#endif
#ifndef _CSHTYP_HXX
#include <cshtyp.hxx>        	// fuer die CursorShell Typen
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>			// fuer die CursorMove-Staties
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>				//SwTblSearchType
#endif

#if !defined(DBG_UTIL) && !defined(WIN)
// fuer die Inline-Methoden
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#define CRSR_INLINE inline
#else
#define CRSR_INLINE
#endif
namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }
class KeyCode;
class Region;
namespace binfilter {
class SfxItemSet;
class SfxPoolItem;

// einige Forward Deklarationen


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

//STRIP008 namespace com { namespace sun { namespace star { namespace util {
//STRIP008 	struct SearchOptions;
//STRIP008 } } } }


// enum und struktur, um ueber die Doc-Position Informationen zu erfragen

struct SwContentAtPos
{
    enum IsAttrAtPos
    {
        SW_NOTHING 		= 0x0000,
        SW_FIELD		= 0x0001,
        SW_CLICKFIELD	= 0x0002,
        SW_FTN			= 0x0004,
        SW_INETATTR		= 0x0008,
        SW_TABLEBOXFML	= 0x0010,
        SW_REDLINE		= 0x0020,
        SW_OUTLINE		= 0x0040,
        SW_TOXMARK		= 0x0080,
        SW_REFMARK		= 0x0100
#ifdef DBG_UTIL
        ,SW_CURR_ATTRS 		= 0x4000		// nur zum Debuggen
        ,SW_TABLEBOXVALUE	= 0x8000		// nur zum Debuggen
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
};


// defines fuers GetCharCount
#define GETCHARCOUNT_NONE		0
#define GETCHARCOUNT_PARA 		1
#define GETCHARCOUNT_SECTION 	2


// ReturnWerte von SetCrsr (werden verodert)
const int 	CRSR_NOERROR =	0x00,
            CRSR_POSOLD =	0x01,	// Cursor bleibt an alter Doc-Position
            CRSR_POSCHG =	0x02;	// Position vom Layout veraendert




// die Cursor - Shell
class SwCrsrShell : public ViewShell, public SwModify
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;
    friend class SwChgLinkFlag;

    friend BOOL GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:     // public, damit defaultet werden kann !!

    /* ein enum fuer den Aufruf von UpdateCrsr */
    enum CrsrFlag {
        UPDOWN		= (1 << 0),     // Up/Down auf Spalte halten
        SCROLLWIN	= (1 << 1),     // Window Scrollen
        CHKRANGE	= (1 << 2),     // ueberpruefen von ueberlappenden PaM's
        NOCALRECT 	= (1 << 3),		// CharRect nicht neu berechnen
        READONLY	= (1 << 4)		// Sichtbar machen trotz Readonly
    };

private:

    SwRect	aCharRect;			// Char-SRectangle auf dem der Cursor steht
    Point	aCrsrHeight;     	// Hohe&Offset von sichtbaren Cursor
    Point 	aOldRBPos;			// Right/Bottom von letzter VisArea
                                // (wird im Invalidate vom Cursor benutzt)


    // um event. Macro was anhaengt auszufuehren.
    Link aFlyMacroLnk;			// Link will be called, if the Crsr is set
                                // into a fly. A macro can be then becalled
    Link aChgLnk;				// link will be called by every attribut/
                                // format changes at cursor position.
    Link aGrfArrivedLnk;		// Link calls to UI if a grafik is arrived


    SwShellCrsr* pCurCrsr; 		// der aktuelle Cursor
    SwShellCrsr* pCrsrStk;		// Stack fuer den Cursor
    SwVisCrsr *pVisCrsr;        // der Sichtbare-Cursor

    SwShellTableCrsr* pTblCrsr;	// Tabellen-Crsr; nur in Tabellen, wenn
                                // die Selection ueber 2 Spalten liegt

    SwNodeIndex* pBoxIdx;		// fuers erkennen der veraenderten
    SwTableBox* pBoxPtr;		// Tabellen-Zelle

    long nUpDownX;				// versuche den Cursor bei Up/Down immer in
                                // der gleichen Spalte zu bewegen
    long nLeftFrmPos;
    ULONG nAktNode;				// save CursorPos at Start-Action
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
    USHORT nBasicActionCnt;		// Actions, die vom Basic geklammert wurden
    CrsrMoveState eMvState;		// Status fuers Crsr-Travelling - GetCrsrOfst

    BOOL bHasFocus : 1;         // Shell ist in einem Window "aktiv"
    BOOL bSVCrsrVis : 1;		// SV-Cursor Un-/Sichtbar
    BOOL bChgCallFlag : 1;		// Attributaenderung innerhalb von
                                // Start- und EndAction
    BOOL bVisPortChgd : 1;		// befindet sich im VisPortChg-Aufruf
                                // (wird im Invalidate vom Cursor benutzt)

    BOOL bCallChgLnk : 1;		// Flag fuer abgeleitete Klassen:
                                // TRUE -> ChgLnk callen
                                // Zugriff nur ueber SwChgLinkFlag
    BOOL bAllProtect : 1;		// Flag fuer Bereiche
                                // TRUE -> alles geschuetzt / versteckt
    BOOL bInCMvVisportChgd : 1;	// Flag fuer CrsrMoves
                                // TRUE -> die Sicht wurde verschoben
    BOOL bGCAttr : 1;			// TRUE -> es existieren nichtaufgespannte Attr.
    BOOL bIgnoreReadonly : 1;	// TRUE -> Beim naechsten EndAction trotz
                                // Readonly den Crsr sichtbar machen.
    BOOL bSelTblCells : 1;		// TRUE -> Zellen uebers InputWin selektieren
    BOOL bAutoUpdateCells : 1;	// TRUE -> Zellen werden autoformatiert
    BOOL bBasicHideCrsr : 1;	// TRUE -> HideCrsr vom Basic
    BOOL bSetCrsrInReadOnly : 1;// TRUE -> Cursor darf in ReadOnly-Bereiche
    BOOL bOverwriteCrsr : 1;	// TRUE -> show Overwrite Crsr
    // OD 11.02.2003 #100556# - flag to allow/avoid execution of marcos (default: true)
    bool mbMacroExecAllowed : 1;

    void UpdateCrsr( USHORT eFlags
                            =SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
                     BOOL bIdleEnd = FALSE );


typedef FASTBOOL (SwCursor:: *FNCrsr)();


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

    USHORT IncBasicAction()				{ return ++nBasicActionCnt; }
    USHORT DecBasicAction()				{ return --nBasicActionCnt; }

    // Setzt alle PaMs in OldNode auf NewPos + Offset
    // Setzt alle PaMs im Bereich von [StartNode, EndNode] nach NewPos

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin,
                SwRootFrm * = 0, const SwViewOption *pOpt = 0 );
    virtual ~SwCrsrShell();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    // loesche den aktuellen Cursor und der folgende wird zum Aktuellen
    // TableCursor in normale Cursor verwandeln, Tablemode aufheben

    SwPaM* GetCrsr( FASTBOOL bMakeTblCrsr = TRUE ) const;
    CRSR_INLINE SwCursor* GetSwCrsr( FASTBOOL bMakeTblCrsr = TRUE ) const;
    // nur den akt. Cursor returnen
          SwShellCrsr* _GetCrsr()  						{ return pCurCrsr; }
    const SwShellCrsr* _GetCrsr() const 				{ return pCurCrsr; }

    // alle Cursor aus den ContentNodes entfernen und auf 0 setzen.
    // Wurde aus der FEShell hierher verschoben.

    // gebe den akt. Cursor-Stack zurueck.
    // ( Wird in der EditShell beim Loeschen von Inhalten benoetigt! )
    CRSR_INLINE SwPaM* GetStkCrsr() const;

    // Start der Klammerung, SV-Cursor und selektierte Bereiche hiden
    void StartAction();
    // Ende der Klammerung, SV-Cursor und selektierte Bereiche anzeigen
    void EndAction( const BOOL bIdleEnd = FALSE );

    USHORT GetBasicActionCnt() const 	{ return nBasicActionCnt; }

    // Basiscursortravelling
    long GetUpDownX() const 			{ return nUpDownX; }


    // die Suchfunktionen



    /*
     * Benachrichtung, dass der sichtbare Bereich sich geaendert
     * hat. aVisArea wird neu gesetzt, anschliessend wird
     * gescrollt. Das uebergebene Rectangle liegt auf
     * Pixelgrenzen, um Pixelfehler beim Scrollen zu vermeiden.
     */

    /*
     * Virtuelle PaintMethode, damit die Selection nach dem Paint wieder
     * sichtbar wird.
     */

    // Bereiche


    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst TRUE: If the point is behind the mark then
       swap the PaM. FALSE: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(BOOL bPointFirst = TRUE);


    // erzeuge eine Kopie vom Cursor und speicher diese im Stack
    /*
     *  Loescht einen Cursor (gesteuert durch bOldCrsr)
     * 		- vom Stack oder	( bOldCrsr = TRUE )
     * 		- den aktuellen und der auf dem Stack stehende wird zum aktuellen
     *
     * 	Return:  es war auf dem Stack noch einer vorhanden
     */
    /*
     * Verbinde zwei Cursor miteinander.
     * Loesche vom Stack den obersten und setzen dessen Mark im Aktuellen.
     */

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

    // Methoden zum Anzeigen bzw. Verstecken der selektierten Bereiche mit
    // dem sichtbaren Cursor
    void ShowCrsrs( BOOL bCrsrVis );
    // Methoden zum Anzeigen bzw. Verstecken der selektierten Bereiche mit
    // dem sichtbaren Cursor

    FASTBOOL IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( FASTBOOL bFlag ) { bOverwriteCrsr = bFlag; }

    // gebe den aktuellen Frame, in dem der Cursor steht, zurueck
    SwCntntFrm *GetCurrFrm( const BOOL bCalcFrm = TRUE ) const;

    //TRUE wenn der Crsr wenn der Crsr wegen Readonly gehidet ist,
    //FALSE wenn der arbeitet (trotz Readonly).
    FASTBOOL IsCrsrReadonly() const;
    // darf der Cursor in ReadOnlyBereiche?
    FASTBOOL IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }

    // Methoden fuer aFlyMacroLnk
    void 		SetFlyMacroLnk( const Link& rLnk ) { aFlyMacroLnk = rLnk; }
    const Link& GetFlyMacroLnk() const 			   { return aFlyMacroLnk; }

    // Methoden geben/aendern den Link fuer die Attribut/Format-Aenderungen
    void 		SetChgLnk( const Link &rLnk ) { aChgLnk = rLnk; }
    const Link& GetChgLnk() const 			  { return aChgLnk; }

    // Methoden geben/aendern den Link fuers "Grafik vollstaendig geladen"
    void 		SetGrfArrivedLnk( const Link &rLnk ) { aGrfArrivedLnk = rLnk; }
    const Link& GetGrfArrivedLnk() const 			 { return aGrfArrivedLnk; }

    //ChgLink callen, innerhalb einer Action wird der Ruf verzoegert.
    void CallChgLnk();

    // Abfrage, ob der aktuelle Cursor eine Selektion aufspannt,
    // also, ob Mark gesetzt und SPoint und Mark unterschiedlich sind.
    FASTBOOL HasSelection() const;

    // Abfrage, ob ueberhaupt eine Selektion existiert, sprich der akt. Cursor
    // aufgespannt oder nicht der einzigste ist.
    CRSR_INLINE FASTBOOL IsSelection() const;
        // returns if multiple cursors are available
    CRSR_INLINE FASTBOOL IsMultiSelection() const;

    // Abfrage, ob ein kompletter Absatz selektiert wurde
    // Abfrage, ob die Selektion in einem Absatz ist
    CRSR_INLINE FASTBOOL IsSelOnePara() const;

    //Sollte fuer das Clipboard der WaitPtr geschaltet werden.

    /*
     * liefert das SRectangle, auf dem der Cursor steht.
     */
    const SwRect &GetCharRect() const { return aCharRect; }
    /*
     * liefert zurueck, ob der Cursor sich ganz oder teilweise im
     * sichtbaren Bereich befindet.
     */
    FASTBOOL IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }

    // aktualisiere den Crsrs, d.H. setze ihn wieder in den Content.
    // Das sollte nur aufgerufen werden, wenn der Cursor z.B. beim
    // Loeschen von Rahmen irgendwohin gesetzt wurde. Die Position
    // ergibt sich aus seiner aktuellen Position im Layout !!
    void UpdateCrsrPos();

    // returne den am akt. Cursor selektierten Text. Dieser wird mit
    // Felder etc. aufgefuellt!!
    // gebe nur den Text ab der akt. Cursor Position zurueck (bis zum NodeEnde)
    // retrurne die Anzahl der selektierten Zeichen.
    // Falls keine Selektion vorliegt entscheided nType was selektiert wird
    // bIntrnlChar besagt ob interne Zeichen erhalten bleiben (TRUE) oder
    // ob sie expandiert werden (z.B Felder/...)

    // pruefe ob vom aktuellen Crsr der SPoint/Mark in einer Tabelle stehen
    CRSR_INLINE const SwTableNode* IsCrsrInTbl( BOOL bIsPtInTbl = TRUE ) const;
    // erfrage die Document - Layout - Position vom akt. Crsr
    CRSR_INLINE Point& GetCrsrDocPos( BOOL bPoint = TRUE ) const;
    CRSR_INLINE FASTBOOL IsCrsrPtAtEnd() const;

    CRSR_INLINE const 	SwPaM* GetTblCrs() const;
     CRSR_INLINE 		SwPaM* GetTblCrs();

    // select a table row, column or box (based on the current cursor)

    // zum naechsten/vorhergehenden Punkt auf gleicher Ebene

        // zu diesem Gliederungspunkt
        // zum naechsten/vorhergehenden oder angegebenen OultineNode
        // suche die "Outline-Position" im Nodes-Array vom akt. Kaiptel
        // selektiere den angeben Bereich von OutlineNodes. Optional
        // inclusive der Childs. Die USHORT sind die Positionen im
        // OutlineNds-Array!! (EditShell)


        // ist der Crsr in einer Tabelle und ist die Selection ueber
        // zwei Spalten
    FASTBOOL IsTableMode() const { return 0 != pTblCrsr; }

        // erfrage den Tabellen Crsr; ausserhalb von Tabellen immer 0
    const SwShellTableCrsr* GetTableCrsr() const { return pTblCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return pTblCrsr; }


    // springe in den Header/Footer des angegebenen oder akt. PageDesc
    // is point of cursor in header/footer. pbInHeader return TRUE if it is
    // in a headerframe otherwise in a footerframe

    // springe zum naechsten Verzeichnis [mit dem Namen]
    // springe zum vorherigen Verzeichnis [mit dem Namen]
    // springe zum naechsten (vorherigen) Verzeichniseintrag
    // Zur naechsten/ vorherigen Verzeichnismarke dieses Typs traveln

    // springe zum naechsten (vorherigen) Tabellenformel
    // optional auch nur zu kaputten Formeln springen
    // jump to the next / previous hyperlink - inside text and also
    // on graphics

    // springe zu dieser Refmark

    // hole vom Start/Ende der akt. Selection das nte Zeichen
    // erweiter die akt. Selection am Anfang/Ende um n Zeichen
    // setze nur den sichtbaren Cursor an die angegebene Dokument-Pos.
    // returnt FALSE: wenn der ob der SPoint vom Layout korrigiert wurde.
    // (wird zum Anzeigen von Drag&Drop/Copy-Cursor benoetigt)
    CRSR_INLINE void UnSetVisCrsr();

    // Char Travelling - Methoden (in crstrvl1.cxx)

    // Abfrage vom CrsrTravelling Status
    CrsrMoveState GetMoveState() const { return eMvState; }

    // loesche alle erzeugten Crsr, setze den Tabellen-Crsr und den letzten
    // Cursor auf seinen TextNode (oder StartNode?).
    // Beim naechsten ::GetCrsr werden sie wieder alle erzeugt.
    // Wird fuers Drag&Drop/ClipBorad-Paste in Tabellen benoetigt.

    // erfrage die selektierte "Region" aller Cursor (fuer D&D auf Mac)
    Region GetCrsrRegion() const;

    // gibt es nicht aufgespannte Attribute?
    FASTBOOL IsGCAttr() const { return bGCAttr; }
    void	ClearGCAttr() {	bGCAttr = FALSE; }
    void	UpdateAttr() {	bGCAttr = TRUE; }

    // ist das gesamte Dokument geschuetzt/versteckt?? (fuer UI,..)
    FASTBOOL IsAllProtect() const { return bAllProtect; }

#ifdef SW_CRSR_TIMER
    // setze das Flag am VisCrsr, ob dieser ueber Timer getriggert (TRUE)
    // oder direkt (FALSE) angezeigt wird. (default ist Timer getriggert)
#endif

    // steht der Curor auf einem "Symbol"-Zeichen

    BOOL BasicActionPend() const 	{ return nBasicActionCnt != nStartAction; }

        // springe zum benannten Bereich

    // zeige die aktuelle Selektion an
    virtual void MakeSelVisible();

    // setzte den Cursor auf einen NICHT geschuetzten/versteckten Node



        // Attribut selelktieren

    FASTBOOL CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();
    FASTBOOL EndAllTblBoxEdit();

    // wird gerufen, wenn eine Tabellenselektion im UpdateCrsr erzeugt wird,
    // ohne das die UI davon etaws weiss

    void SetSelTblCells( BOOL bFlag )			{ bSelTblCells = bFlag; }
    BOOL IsSelTblCells() const 					{ return bSelTblCells; }

    BOOL IsAutoUpdateCells() const 				{ return bAutoUpdateCells; }
    void SetAutoUpdateCells( BOOL bFlag ) 		{ bAutoUpdateCells = bFlag; }



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

#if !defined(DBG_UTIL) && !defined(WIN)

inline SwMoveFnCollection* SwCrsrShell::MakeFindRange(
            USHORT nStt, USHORT nEnd, SwPaM* pPam ) const
{
    DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 return pCurCrsr->MakeFindRange( (SwDocPositions)nStt, (SwDocPositions)nEnd, pPam );
}

inline SwCursor* SwCrsrShell::GetSwCrsr( FASTBOOL bMakeTblCrsr ) const
{
    return (SwCursor*)GetCrsr( bMakeTblCrsr );
}

inline SwPaM* SwCrsrShell::GetStkCrsr() const { return pCrsrStk; }



/*N*/ inline FASTBOOL SwCrsrShell::IsSelection() const
/*N*/ {
/*N*/ 	return IsTableMode() || pCurCrsr->HasMark() ||
/*N*/ 			pCurCrsr->GetNext() != pCurCrsr;
/*N*/ }
inline FASTBOOL SwCrsrShell::IsMultiSelection() const
{
    return pCurCrsr->GetNext() != pCurCrsr;
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


} //namespace binfilter
#endif 	// _CRSRSH_HXX
