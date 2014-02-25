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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_ROOTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ROOTFRM_HXX

#include "layfrm.hxx"
#include <viewsh.hxx>
#include <doc.hxx>

class SwCntntFrm;
class SwViewShell;
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

#define INV_SIZE    1
#define INV_PRTAREA 2
#define INV_POS     4
#define INV_TABLE   8
#define INV_SECTION 16
#define INV_LINENUM 32
#define INV_DIRECTION 64

#include <vector>

/// The root element of a Writer document layout.
class SwRootFrm: public SwLayoutFrm
{
    //Muss das Superfluous temporaer abschalten.
    friend void AdjustSizeChgNotify( SwRootFrm *pRoot );

    //Pflegt pLastPage (Cut() und Paste() vom SwPageFrm
    friend inline void SetLastPage( SwPageFrm* );

    // Fuer das Anlegen und Zerstoeren des virtuellen Outputdevice-Managers
    friend void _FrmInit();     //erzeugt pVout
    friend void _FrmFinit();    //loescht pVout

    std::vector<SwRect> maPageRects;// returns the current rectangle for each page frame
                                    // the rectangle is extended to the top/bottom/left/right
                                    // for pages located at the outer borders
    SwRect  maPagesArea;            // the area covered by the pages
    long    mnViewWidth;            // the current page layout bases on this view width
    sal_uInt16  mnColumns;              // the current page layout bases on this number of columns
    bool    mbBookMode;             // the current page layout is in book view
    bool    mbSidebarChanged;       // the notes sidebar state has changed

    bool    mbNeedGrammarCheck;     // true when sth needs to be checked (not necessarily started yet!)

    static SwLayVout     *pVout;
    static bool           bInPaint;     //Schutz gegen doppelte Paints.
    static sal_Bool           bNoVirDev;    //Bei SystemPaints kein virt. Device

    sal_Bool    bCheckSuperfluous   :1; //Leere Seiten suchen?
    sal_Bool    bIdleFormat         :1; //Idle-Formatierer anwerfen?
    sal_Bool    bBrowseWidthValid   :1; //Ist nBrowseWidth gueltig?
    sal_Bool    bTurboAllowed       :1;
    sal_Bool    bAssertFlyPages     :1; //Ggf. weitere Seiten fuer Flys einfuegen?
    sal_Bool    bIsVirtPageNum      :1; //gibt es eine virtuelle Seitennummer ?
    sal_Bool    bIsNewLayout        :1; //Layout geladen oder neu erzeugt.
    sal_Bool    bCallbackActionEnabled:1; //Keine Action in Benachrichtung erwuenscht
                                    //siehe dcontact.cxx, ::Changed()
    bool        bLayoutFreezed;

    //Fuer den BrowseMode. nBrowseWidth ist die Aeussere Kante des am weitesten
    //rechts stehenden Objectes. Die rechte Kante der Seiten soll im BrowseMode
    //nicht kleiner werden als dieser Wert.
    long    nBrowseWidth;

    //Wenn nur _ein: CntntFrm zu formatieren ist, so steht dieser in pTurbo.
    const SwCntntFrm *pTurbo;

    //Die letzte Seite wollen wir uns nicht immer muehsam zusammensuchen.
    SwPageFrm *pLastPage;

    // [ Comment from the original StarOffice checkin ]:
    // The root takes care of the shell access. Via the document
    // it should be possible to get at the root frame, and thus always
    // have access to the shell.
    // the pointer pCurrShell is the pointer to any of the shells for
    // the document.
    // Because sometimes it matters which shell is used, it is necessary to
    // know the active shell.
    // this is approximated by setting the pointer pCurrShell when a
    // shell gets the focus (FEShell). Acditionally the pointer will be
    // set temporarily by SwCurrShell typically via  SET_CURR_SHELL
    // The macro and class can be found in the SwViewShell. These object can
    // be created nested (also for different kinds of Shells). They are
    // collected into the Array pCurrShells.
    // Futhermore it can happen that a shell is activated while a curshell
    // object is still 'active'. This one will be entered into pWaitingCurrShell
    // and will be activated by the last d'tor of CurrShell.
    // One other problem is the destruction of a shell while it is active.
    // The pointer pCurrShell is then reset to an arbitrary other shell.
    // If at the time of the destruction of a shell, which is still referneced
    // by a curshell object, that will be cleaned up as well.
    friend class CurrShell;
    friend void SetShell( SwViewShell *pSh );
    friend void InitCurrShells( SwRootFrm *pRoot );
    SwViewShell *pCurrShell;
    SwViewShell *pWaitingCurrShell;
    SwCurrShells *pCurrShells;

    //Eine Page im DrawModel pro Dokument, hat immer die Groesse der Root.
    SdrPage *pDrawPage;

    SwDestroyList* pDestroy;

    sal_uInt16  nPhyPageNums;           //Anzahl der Seiten.
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

    void AllCheckPageDescs() const;
    void AllInvalidateAutoCompleteWords() const;
    void AllAddPaintRect() const;
    void AllRemoveFtns() ;
    void AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const;
    //Virtuelles Device ausgeben (z.B. wenn Animationen ins Spiel kommen)
    static sal_Bool FlushVout();
    //Clipping sparen, wenn im Vout eh genau das Cliprechteck ausgegeben wird
    static sal_Bool HasSameRect( const SwRect& rRect );

    SwRootFrm( SwFrmFmt*, SwViewShell* );
    virtual ~SwRootFrm();
    void Init(SwFrmFmt*);

    SwViewShell *GetCurrShell() const { return pCurrShell; }
    void DeRegisterShell( SwViewShell *pSh );

    //Start-/EndAction fuer alle Shells auf moeglichst hoeher
    //(Shell-Ableitungs-)Ebene aufsetzen. Fuer die StarONE Anbindung, die
    //die Shells nicht dirkt kennt.
    //Der ChangeLinkd der CrsrShell (UI-Benachrichtigung) wird im EndAllAction
    //automatisch gecallt.
    void StartAllAction();
    void EndAllAction( sal_Bool bVirDev = sal_False );

    // fuer bestimmte UNO-Aktionen (Tabellencursor) ist es notwendig, dass alle Actions
    // kurzfristig zurueckgesetzt werden. Dazu muss sich jede SwViewShell ihren alten Action-zaehler
    // merken
    void UnoRemoveAllActions();
    void UnoRestoreAllActions();

    const SdrPage* GetDrawPage() const { return pDrawPage; }
          SdrPage* GetDrawPage()       { return pDrawPage; }
          void     SetDrawPage( SdrPage* pNew ){ pDrawPage = pNew; }

    virtual bool  GetCrsrOfst( SwPosition *, Point&,
                               SwCrsrMoveState* = 0, bool bTestBackground = false ) const;

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const;
    virtual SwTwips ShrinkFrm( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips GrowFrm  ( SwTwips, sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
#ifdef DBG_UTIL
    virtual void Cut();
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 );
#endif

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const;

    Point  GetNextPrevCntntPos( const Point &rPoint, sal_Bool bNext ) const;

    virtual Size ChgSize( const Size& aNewSize );

    void SetIdleFlags()
    {
        bIdleFormat = sal_True;

        SwViewShell* lcl_pCurrShell = GetCurrShell();
        // May be NULL if called from SfxBaseModel::dispose
        // (this happens in the build test 'rtfexport').
        if (lcl_pCurrShell != NULL)
            lcl_pCurrShell->GetDoc()->StartBackgroundJobs();
    }
    sal_Bool IsIdleFormat()  const { return bIdleFormat; }
    void ResetIdleFormat()     { bIdleFormat = sal_False; }

    bool IsNeedGrammarCheck() const         { return mbNeedGrammarCheck; }
    void SetNeedGrammarCheck( bool bVal )
    {
        mbNeedGrammarCheck = bVal;

        if ( bVal )
        {
            SwViewShell* lcl_pCurrShell = GetCurrShell();
            // May be NULL if called from SfxBaseModel::dispose
            // (this happens in the build test 'rtfexport').
            if (lcl_pCurrShell != NULL)
                lcl_pCurrShell->GetDoc()->StartBackgroundJobs();
        }
    }

    //Sorgt dafuer, dass alle gewuenschten Seitengebunden Flys eine Seite finden
    void SetAssertFlyPages() { bAssertFlyPages = sal_True; }
    void AssertFlyPages();
    sal_Bool IsAssertFlyPages()  { return bAssertFlyPages; }

    //Stellt sicher, dass ab der uebergebenen Seite auf allen Seiten die
    //Seitengebundenen Rahmen auf der richtigen Seite (Seitennummer) stehen.
    void AssertPageFlys( SwPageFrm * );

    //Saemtlichen Inhalt invalidieren, Size oder PrtArea
    void InvalidateAllCntnt( sal_uInt8 nInvalidate = INV_SIZE );

    /** method to invalidate/re-calculate the position of all floating
        screen objects (Writer fly frames and drawing objects), which are
        anchored to paragraph or to character.

        OD 2004-03-16 #i11860#
    */
    void InvalidateAllObjPos();

    //Ueberfluessige Seiten entfernen.
    void SetSuperfluous()      { bCheckSuperfluous = sal_True; }
    sal_Bool IsSuperfluous() const { return bCheckSuperfluous; }
    void RemoveSuperfluous();

    //abfragen/setzen der aktuellen Seite und der Gesamtzahl der Seiten.
    //Es wird soweit wie notwendig Formatiert.
    sal_uInt16  GetCurrPage( const SwPaM* ) const;
    sal_uInt16  SetCurrPage( SwCursor*, sal_uInt16 nPageNum );
    Point   GetPagePos( sal_uInt16 nPageNum ) const;
    sal_uInt16  GetPageNum() const      { return nPhyPageNums; }
    void    DecrPhyPageNums()       { --nPhyPageNums; }
    void    IncrPhyPageNums()       { ++nPhyPageNums; }
    sal_Bool    IsVirtPageNum() const   { return bIsVirtPageNum; }
    inline  void SetVirtPageNum( const sal_Bool bOf ) const;
    sal_Bool    IsDummyPage( sal_uInt16 nPageNum ) const;

    // Point rPt: The point that should be used to find the page
    // Size pSize: If given, we return the (first) page that overlaps with the
    // rectangle defined by rPt and pSize
    // bool bExtend: Extend each page to the left/right/top/botton up to the
    // next page border
    const SwPageFrm* GetPageAtPos( const Point& rPt, const Size* pSize = 0, bool bExtend = false ) const;

    void CalcFrmRects(
        SwShellCrsr&,
        const sal_Bool bIsTblSel );

    // Calculates the cells included from the current selection
    // false: There was no result because of an invalid layout
    // true: Everything worked fine.
    bool MakeTblCrsrs( SwTableCursor& );

    void DisallowTurbo()  const { ((SwRootFrm*)this)->bTurboAllowed = sal_False; }
    void ResetTurboFlag() const { ((SwRootFrm*)this)->bTurboAllowed = sal_True; }
    sal_Bool IsTurboAllowed() const { return bTurboAllowed; }
    void SetTurbo( const SwCntntFrm *pCntnt ) { pTurbo = pCntnt; }
    void ResetTurbo() { pTurbo = 0; }
    const SwCntntFrm *GetTurbo() { return pTurbo; }

    //Fussnotennummern aller Seiten auf den neuesten Stand bringen.
    void UpdateFtnNums();           //nur bei Seitenweiser Nummerierung!

    //Alle Fussnoten (nicht etwa die Referenzen) entfernen.
    void RemoveFtns( SwPageFrm *pPage = 0, sal_Bool bPageOnly = sal_False,
                     sal_Bool bEndNotes = sal_False );
    void CheckFtnPageDescs( sal_Bool bEndNote );

    const SwPageFrm *GetLastPage() const { return pLastPage; }
          SwPageFrm *GetLastPage()       { return pLastPage; }

    static bool IsInPaint() { return bInPaint; }

    static void SetNoVirDev( const sal_Bool bNew ) { bNoVirDev = bNew; }

    inline long GetBrowseWidth() const;
    void SetBrowseWidth( long n ) { bBrowseWidthValid = sal_True; nBrowseWidth = n;}
    inline void InvalidateBrowseWidth();

    sal_Bool IsNewLayout() const { return bIsNewLayout; }
    void ResetNewLayout()    { bIsNewLayout = sal_False;}

    // Hier werden leere SwSectionFrms zur Zerstoerung angemeldet
    // und spaeter zerstoert oder wieder abgemeldet
    void InsertEmptySct( SwSectionFrm* pDel );
    void DeleteEmptySct() { if( pDestroy ) _DeleteEmptySct(); }
    void RemoveFromList( SwSectionFrm* pSct ) { if( pDestroy ) _RemoveFromList( pSct ); }
#ifdef DBG_UTIL
    bool IsInDelList( SwSectionFrm* pSct ) const;
#endif

    void SetCallbackActionEnabled( sal_Bool b ) { bCallbackActionEnabled = b; }
    sal_Bool IsCallbackActionEnabled() const    { return bCallbackActionEnabled; }

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

    void CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea );
    bool IsLeftToRightViewLayout() const;
    const SwRect& GetPagesArea() const { return maPagesArea; }
    void SetSidebarChanged() { mbSidebarChanged = true; }

    bool IsLayoutFreezed() const { return bLayoutFreezed; }
    void FreezeLayout( bool freeze ) { bLayoutFreezed = freeze; }
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

inline  void SwRootFrm::SetVirtPageNum( const sal_Bool bOf) const
{
    ((SwRootFrm*)this)->bIsVirtPageNum = bOf;
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROOTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
