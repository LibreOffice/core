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
#ifndef SW_ROOTFRM_HXX
#define SW_ROOTFRM_HXX

#include "layfrm.hxx"

class SwCntntFrm;
class ViewShell;
class SdrPage;
class SwFrmFmt;
class SwPaM;
class SwCursor;
class SwShellCrsr;
class SwTableCursor;
class SwLayVout;
class SwDestroyList;
class SwCurrShells;
class SwViewOption;
class SwSelectionList;
struct SwPosition;
struct SwCrsrMoveState;

#define HACK_TABLEMODE_INIT         0
#define HACK_TABLEMODE_LOCKLINES    1
#define HACK_TABLEMODE_PAINTLINES   2
#define HACK_TABLEMODE_UNLOCKLINES  3
#define HACK_TABLEMODE_EXIT         4

#define INV_SIZE    1
#define INV_PRTAREA 2
#define INV_POS     4
#define INV_TABLE   8
#define INV_SECTION 16
#define INV_LINENUM 32
#define INV_DIRECTION 64

#include <vector>

class SwRootFrm: public SwLayoutFrm
{
    //Muss das Superfluous temporaer abschalten.
    friend void AdjustSizeChgNotify( SwRootFrm *pRoot );

    //Pflegt pLastPage (Cut() und Paste() vom SwPageFrm
    friend inline void SetLastPage( SwPageFrm* );

    // Fuer das Anlegen und Zerstoeren des virtuellen Outputdevice-Managers
    friend void _FrmInit();     //erzeugt pVout
    friend void _FrmFinit();    //loescht pVout

    // PAGES01
    std::vector<SwRect> maPageRects;// returns the current rectangle for each page frame
                                    // the rectangle is extended to the top/bottom/left/right
                                    // for pages located at the outer borders
    SwRect  maPagesArea;            // the area covered by the pages
    long    mnViewWidth;            // the current page layout bases on this view width
    USHORT  mnColumns;              // the current page layout bases on this number of columns
    bool    mbBookMode;             // the current page layout is in book view
    bool    mbSidebarChanged;       // the notes sidebar state has changed
    // <--

    bool    mbNeedGrammarCheck;     // true when sth needs to be checked (not necessarily started yet!)

    static SwLayVout     *pVout;
    static BOOL           bInPaint;     //Schutz gegen doppelte Paints.
    static BOOL           bNoVirDev;    //Bei SystemPaints kein virt. Device

    BOOL    bCheckSuperfluous   :1; //Leere Seiten suchen?
    BOOL    bIdleFormat         :1; //Idle-Formatierer anwerfen?
    BOOL    bBrowseWidthValid   :1; //Ist nBrowseWidth gueltig?
    BOOL    bDummy2             :1; //Unbenutzt
    BOOL    bTurboAllowed       :1;
    BOOL    bAssertFlyPages     :1; //Ggf. weitere Seiten fuer Flys einfuegen?
    BOOL    bDummy              :1; //Unbenutzt
    BOOL    bIsVirtPageNum      :1; //gibt es eine virtuelle Seitennummer ?
    BOOL    bIsNewLayout        :1; //Layout geladen oder neu erzeugt.
    BOOL    bCallbackActionEnabled:1; //Keine Action in Benachrichtung erwuenscht
                                    //siehe dcontact.cxx, ::Changed()

    //Fuer den BrowseMode. nBrowseWidth ist die Aeussere Kante des am weitesten
    //rechts stehenden Objectes. Die rechte Kante der Seiten soll im BrowseMode
    //nicht kleiner werden als dieser Wert.
    long    nBrowseWidth;

    //Wenn nur _ein: CntntFrm zu formatieren ist, so steht dieser in pTurbo.
    const SwCntntFrm *pTurbo;

    //Die letzte Seite wollen wir uns nicht immer muehsam zusammensuchen.
    SwPageFrm *pLastPage;

    //Die Root kuemmert sich nun auch um den Shell-Zugriff. Ueber das Dokument
    //sollte man auch immer an die Root herankommen und somit auch immer
    //einen Zugriff auf die Shell haben.
    //Der Pointer pCurrShell ist der Pointer auf irgendeine der Shells fuer
    //das Dokument
    //Da es durchaus nicht immer egal ist, auf welcher Shell gearbeitet wird,
    //ist es notwendig die aktive Shell zu kennen. Das wird dadurch angenaehert,
    //dass der Pointer pCurrShell immer dann umgesetzt wird, wenn eine
    //Shell den Fokus erhaelt (FEShell). Zusaetzlich wird der Pointer
    //Temporaer von SwCurrShell umgesetzt, dieses wird typischerweise
    //ueber das Macro SET_CURR_SHELL erledigt. Makro + Klasse sind in der
    //ViewShell zu finden. Diese Objekte koennen auch verschachtelt (auch fuer
    //unterschiedliche Shells) erzeugt werden. Sie werden im Array pCurrShells
    //gesammelt.
    //Weiterhin kann es noch vorkommen, dass eine Shell aktiviert wird,
    //waehrend noch ein CurrShell-Objekt "aktiv" ist. Dieses wird dann in
    //pWaitingCurrShell eingetragen und vom letzten DTor der CurrShell
    //"aktiviert".
    //Ein weiteres Problem ist dass Zerstoeren einer Shell waehrend sie aktiv
    //ist. Der Pointer pCurrShell wird dann auf eine beliebige andere Shell
    //umgesetzt.
    //Wenn zum Zeitpunkt der zerstoerung einer Shell diese noch in irgendwelchen
    //CurrShell-Objekten referenziert wird, so wird auch dies aufgeklart.
    friend class CurrShell;
    friend void SetShell( ViewShell *pSh );
    friend void InitCurrShells( SwRootFrm *pRoot );
    ViewShell *pCurrShell;
    ViewShell *pWaitingCurrShell;
    SwCurrShells *pCurrShells;

    //Eine Page im DrawModel pro Dokument, hat immer die Groesse der Root.
    SdrPage *pDrawPage;

    SwDestroyList* pDestroy;

    USHORT  nPhyPageNums;           //Anzahl der Seiten.
    sal_uInt16 nAccessibleShells;   // Number of accessible shells

    void ImplCalcBrowseWidth();
    void ImplInvalidateBrowseWidth();

    void _DeleteEmptySct(); // zerstoert ggf. die angemeldeten SectionFrms
    void _RemoveFromList( SwSectionFrm* pSct ); // entfernt SectionFrms aus der Delete-Liste

protected:

    virtual void MakeAll();

public:

    //MasterObjekte aus der Page entfernen (von den Ctoren gerufen).
    static void RemoveMasterObjs( SdrPage *pPg );

    //Virtuelles Device ausgeben (z.B. wenn Animationen ins Spiel kommen)
    static BOOL FlushVout();
    //Clipping sparen, wenn im Vout eh genau das Cliprechteck ausgegeben wird
    static BOOL HasSameRect( const SwRect& rRect );

    SwRootFrm( SwFrmFmt*, ViewShell* );
    ~SwRootFrm();

    ViewShell *GetCurrShell() const { return pCurrShell; }
    void DeRegisterShell( ViewShell *pSh );

    //Start-/EndAction fuer alle Shells auf moeglichst hoeher
    //(Shell-Ableitungs-)Ebene aufsetzen. Fuer die StarONE Anbindung, die
    //die Shells nicht dirkt kennt.
    //Der ChangeLinkd der CrsrShell (UI-Benachrichtigung) wird im EndAllAction
    //automatisch gecallt.
    void StartAllAction();
    void EndAllAction( BOOL bVirDev = FALSE );

    // fuer bestimmte UNO-Aktionen (Tabellencursor) ist es notwendig, dass alle Actions
    // kurzfristig zurueckgesetzt werden. Dazu muss sich jede ViewShell ihren alten Action-zaehler
    // merken
    void UnoRemoveAllActions();
    void UnoRestoreAllActions();

    const SdrPage* GetDrawPage() const { return pDrawPage; }
          SdrPage* GetDrawPage()       { return pDrawPage; }
          void     SetDrawPage( SdrPage* pNew ){ pDrawPage = pNew; }

    virtual BOOL  GetCrsrOfst( SwPosition *, Point&,
                               SwCrsrMoveState* = 0 ) const;

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual SwTwips ShrinkFrm( SwTwips, BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, BOOL bTst = FALSE, BOOL bInfo = FALSE );
#ifdef DBG_UTIL
    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    Point  GetNextPrevCntntPos( const Point &rPoint, BOOL bNext ) const;

    virtual Size ChgSize( const Size& aNewSize );

    void SetIdleFlags() { bIdleFormat = TRUE; }
    BOOL IsIdleFormat()  const { return bIdleFormat; }
    void ResetIdleFormat()     { bIdleFormat = FALSE; }

    bool IsNeedGrammarCheck() const         { return mbNeedGrammarCheck; }
    void SetNeedGrammarCheck( bool bVal )   { mbNeedGrammarCheck = bVal; }

    //Sorgt dafuer, dass alle gewuenschten Seitengebunden Flys eine Seite finden
    void SetAssertFlyPages() { bAssertFlyPages = TRUE; }
    void AssertFlyPages();
    BOOL IsAssertFlyPages()  { return bAssertFlyPages; }

    //Stellt sicher, dass ab der uebergebenen Seite auf allen Seiten die
    //Seitengebundenen Rahmen auf der richtigen Seite (Seitennummer) stehen.
    void AssertPageFlys( SwPageFrm * );

    //Saemtlichen Inhalt invalidieren, Size oder PrtArea
    void InvalidateAllCntnt( BYTE nInvalidate = INV_SIZE );

    /** method to invalidate/re-calculate the position of all floating
        screen objects (Writer fly frames and drawing objects), which are
        anchored to paragraph or to character.

        OD 2004-03-16 #i11860#

        @author OD
    */
    void InvalidateAllObjPos();

    //Ueberfluessige Seiten entfernen.
    void SetSuperfluous()      { bCheckSuperfluous = TRUE; }
    BOOL IsSuperfluous() const { return bCheckSuperfluous; }
    void RemoveSuperfluous();

    //abfragen/setzen der aktuellen Seite und der Gesamtzahl der Seiten.
    //Es wird soweit wie notwendig Formatiert.
    USHORT  GetCurrPage( const SwPaM* ) const;
    USHORT  SetCurrPage( SwCursor*, USHORT nPageNum );
    Point   GetPagePos( USHORT nPageNum ) const;
    USHORT  GetPageNum() const      { return nPhyPageNums; }
    void    DecrPhyPageNums()       { --nPhyPageNums; }
    void    IncrPhyPageNums()       { ++nPhyPageNums; }
    BOOL    IsVirtPageNum() const   { return bIsVirtPageNum; }
    inline  void SetVirtPageNum( const BOOL bOf ) const;
    BOOL    IsDummyPage( USHORT nPageNum ) const;

    // Point rPt: The point that should be used to find the page
    // Size pSize: If given, we return the (first) page that overlaps with the
    // rectangle defined by rPt and pSize
    // bool bExtend: Extend each page to the left/right/top/botton up to the
    // next page border
    const SwPageFrm* GetPageAtPos( const Point& rPt, const Size* pSize = 0, bool bExtend = false ) const;

    //Der Crsr moechte die zu selektierenden Bereiche wissen.
    void CalcFrmRects( SwShellCrsr&, BOOL bIsTblSel );

    // Calculates the cells included from the current selection
    // false: There was no result because of an invalid layout
    // true: Everything worked fine.
    bool MakeTblCrsrs( SwTableCursor& );

    void DisallowTurbo()  const { ((SwRootFrm*)this)->bTurboAllowed = FALSE; }
    void ResetTurboFlag() const { ((SwRootFrm*)this)->bTurboAllowed = TRUE; }
    BOOL IsTurboAllowed() const { return bTurboAllowed; }
    void SetTurbo( const SwCntntFrm *pCntnt ) { pTurbo = pCntnt; }
    void ResetTurbo() { pTurbo = 0; }
    const SwCntntFrm *GetTurbo() { return pTurbo; }

    //Fussnotennummern aller Seiten auf den neuesten Stand bringen.
    void UpdateFtnNums();           //nur bei Seitenweiser Nummerierung!

    //Alle Fussnoten (nicht etwa die Referenzen) entfernen.
    void RemoveFtns( SwPageFrm *pPage = 0, BOOL bPageOnly = FALSE,
                     BOOL bEndNotes = FALSE );
    void CheckFtnPageDescs( BOOL bEndNote );

    const SwPageFrm *GetLastPage() const { return pLastPage; }
          SwPageFrm *GetLastPage()       { return pLastPage; }

    static BOOL IsInPaint() { return bInPaint; }

    static void SetNoVirDev( const BOOL bNew ) { bNoVirDev = bNew; }

    inline long GetBrowseWidth() const;
    void SetBrowseWidth( long n ) { bBrowseWidthValid = TRUE; nBrowseWidth = n;}
    inline void InvalidateBrowseWidth();

#ifdef LONG_TABLE_HACK
    void HackPrepareLongTblPaint( int nMode );
#endif

    BOOL IsNewLayout() const { return bIsNewLayout; }
    void ResetNewLayout()    { bIsNewLayout = FALSE;}

    // Hier werden leere SwSectionFrms zur Zerstoerung angemeldet
    // und spaeter zerstoert oder wieder abgemeldet
    void InsertEmptySct( SwSectionFrm* pDel );
    void DeleteEmptySct() { if( pDestroy ) _DeleteEmptySct(); }
    void RemoveFromList( SwSectionFrm* pSct ) { if( pDestroy ) _RemoveFromList( pSct ); }
#ifdef DBG_UTIL
    // Wird zur Zeit nur fuer ASSERTs benutzt:
    BOOL IsInDelList( SwSectionFrm* pSct ) const; // Ist der SectionFrm in der Liste enthalten?
#endif


    void SetCallbackActionEnabled( BOOL b ) { bCallbackActionEnabled = b; }
    BOOL IsCallbackActionEnabled() const    { return bCallbackActionEnabled; }

    sal_Bool IsAnyShellAccessible() const { return nAccessibleShells > 0; }
    void AddAccessibleShell() { ++nAccessibleShells; }
    void RemoveAccessibleShell() { --nAccessibleShells; }

    /** get page frame by phyiscal page number

        OD 14.01.2003 #103492#
        looping through the lowers, which are page frame, in order to find the
        page frame with the given physical page number.
        if no page frame is found, 0 is returned.
        Note: Empty page frames are also returned.

        @param _nPageNum
        input parameter - physical page number of page frame to be searched and
        returned.

        @return pointer to the page frame with the given physical page number
    */
    SwPageFrm* GetPageByPageNum( sal_uInt16 _nPageNum ) const;

    // --> PAGES01
    void CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea );
    bool IsLeftToRightViewLayout() const;
    const SwRect& GetPagesArea() const { return maPagesArea; }
    void SetSidebarChanged() { mbSidebarChanged = true; }
    // <--
};

inline long SwRootFrm::GetBrowseWidth() const
{
    if ( !bBrowseWidthValid )
        ((SwRootFrm*)this)->ImplCalcBrowseWidth();
    return nBrowseWidth;
}

inline void SwRootFrm::InvalidateBrowseWidth()
{
    if ( bBrowseWidthValid )
        ImplInvalidateBrowseWidth();
}

inline  void SwRootFrm::SetVirtPageNum( const BOOL bOf) const
{
    ((SwRootFrm*)this)->bIsVirtPageNum = bOf;
}

#endif  // SW_ROOTFRM_HXX

