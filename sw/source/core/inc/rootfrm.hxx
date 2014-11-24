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
#include <IDocumentTimerAccess.hxx>

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
    // Needs to disable the Superfluous temporarily
    friend void AdjustSizeChgNotify( SwRootFrm *pRoot );

    // Maintains the pLastPage (Cut() and Paste() of SwPageFrm
    friend inline void SetLastPage( SwPageFrm* );

    // For creating and destroying of the virtual output device manager
    friend void _FrmInit(); // Creates pVout
    friend void _FrmFinit(); // Destroys pVout

    std::vector<SwRect> maPageRects;// returns the current rectangle for each page frame
                                    // the rectangle is extended to the top/bottom/left/right
                                    // for pages located at the outer margins
    SwRect  maPagesArea;            // the area covered by the pages
    long    mnViewWidth;            // the current page layout bases on this view width
    sal_uInt16  mnColumns;          // the current page layout bases on this number of columns
    bool    mbBookMode;             // the current page layout is in book view
    bool    mbSidebarChanged;       // the notes sidebar state has changed

    bool    mbNeedGrammarCheck;     // true when sth needs to be checked (not necessarily started yet!)

    static SwLayVout     *pVout;
    static bool           bInPaint; // Protection against double Paints
    static bool           bNoVirDev;// No virt. Device for SystemPaints

    bool    bCheckSuperfluous   :1; // Search for empty Pages?
    bool    bIdleFormat         :1; // Trigger Idle Formatter?
    bool    bBrowseWidthValid   :1; // Is nBrowseWidth valid?
    bool    bTurboAllowed       :1;
    bool    bAssertFlyPages     :1; // Insert more Pages for Flys if needed?
    bool    bIsVirtPageNum      :1; // Do we have a virtual pagenumber?
    bool    bIsNewLayout        :1; // Layout loaded or newly created
    bool    bCallbackActionEnabled:1; // No Action in Notification desired
                                      // @see dcontact.cxx, ::Changed()
    bool    bLayoutFreezed;

    /**
     * For BrowseMode
     * nBrowseWidth is the outer margin of the object most to the right.
     * The page's right edge should not be smaller than this value.
     */
    long    nBrowseWidth;

    /// If we only have to format one CntntFrm, its in pTurbo
    const SwCntntFrm *pTurbo;

    /// We should not need to always struggle to find the last page, so store it here
    SwPageFrm *pLastPage;

    /** [ Comment from the original StarOffice checkin ]:
     * The root takes care of the shell access. Via the document
     * it should be possible to get at the root frame, and thus always
     * have access to the shell.
     * the pointer pCurrShell is the pointer to any of the shells for
     * the document.
     * Because sometimes it matters which shell is used, it is necessary to
     * know the active shell.
     * this is approximated by setting the pointer pCurrShell when a
     * shell gets the focus (FEShell). Acditionally the pointer will be
     * set temporarily by SwCurrShell typically via  SET_CURR_SHELL
     * The macro and class can be found in the SwViewShell. These object can
     * be created nested (also for different kinds of Shells). They are
     * collected into the Array pCurrShells.
     * Furthermore it can happen that a shell is activated while a curshell
     * object is still 'active'. This one will be entered into pWaitingCurrShell
     * and will be activated by the last d'tor of CurrShell.
     * One other problem is the destruction of a shell while it is active.
     * The pointer pCurrShell is then reset to an arbitrary other shell.
     * If at the time of the destruction of a shell, which is still referneced
     * by a curshell object, that will be cleaned up as well.
     */
    friend class CurrShell;
    friend void SetShell( SwViewShell *pSh );
    friend void InitCurrShells( SwRootFrm *pRoot );
    SwViewShell *pCurrShell;
    SwViewShell *pWaitingCurrShell;
    SwCurrShells *pCurrShells;

    /// One Page per DrawModel per Document; is always the size of the Root
    SdrPage *pDrawPage;

    SwDestroyList* pDestroy;

    sal_uInt16  nPhyPageNums; /// Page count
    sal_uInt16 nAccessibleShells; // Number of accessible shells

    void ImplCalcBrowseWidth();
    void ImplInvalidateBrowseWidth();

    void _DeleteEmptySct(); // Destroys the registered SectionFrms
    void _RemoveFromList( SwSectionFrm* pSct ); // Removes SectionFrms from the Delete List

protected:

    virtual void MakeAll() SAL_OVERRIDE;

public:

    /// Remove MasterObjects from the Page (called by the ctors)
    static void RemoveMasterObjs( SdrPage *pPg );

    void AllCheckPageDescs() const;
    void AllInvalidateAutoCompleteWords() const;
    void AllAddPaintRect() const;
    void AllRemoveFtns() ;
    void AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const;

    /// Output virtual Device (e.g. for animations)
    static bool FlushVout();

    /// Save Clipping if exactly the ClipRect is outputted
    static bool HasSameRect( const SwRect& rRect );

    SwRootFrm( SwFrmFmt*, SwViewShell* );
    virtual ~SwRootFrm();
    void Init(SwFrmFmt*);

    SwViewShell *GetCurrShell() const { return pCurrShell; }
    void DeRegisterShell( SwViewShell *pSh );

    /**
     * Set up Start-/EndAction for all Shells on a as high as possible
     * (Shell section) level.
     * For the StarONE binding, which does not know the Shells directly.
     * The ChangeLinkd of the CrsrShell (UI notifications) is called
     * automatically in the EndAllAction.
     */
    void StartAllAction();
    void EndAllAction( bool bVirDev = false );

    /**
     * Certain UNO Actions (e.g. table cursor) require that all Actions are reset temporarily
     * In order for that to work, every SwViewShell needs to remember its old Action counter
     */
    void UnoRemoveAllActions();
    void UnoRestoreAllActions();

    const SdrPage* GetDrawPage() const { return pDrawPage; }
          SdrPage* GetDrawPage()       { return pDrawPage; }
          void     SetDrawPage( SdrPage* pNew ){ pDrawPage = pNew; }

    virtual bool  GetCrsrOfst( SwPosition *, Point&,
                               SwCrsrMoveState* = 0, bool bTestBackground = false ) const SAL_OVERRIDE;

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) SAL_OVERRIDE;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) SAL_OVERRIDE;
#ifdef DBG_UTIL
    virtual void Cut() SAL_OVERRIDE;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;
#endif

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const SAL_OVERRIDE;

    Point  GetNextPrevCntntPos( const Point &rPoint, bool bNext ) const;

    virtual Size ChgSize( const Size& aNewSize ) SAL_OVERRIDE;

    void SetIdleFlags()
    {
        bIdleFormat = true;

        SwViewShell* lcl_pCurrShell = GetCurrShell();
        // May be NULL if called from SfxBaseModel::dispose
        // (this happens in the build test 'rtfexport').
        if (lcl_pCurrShell != NULL)
            lcl_pCurrShell->GetDoc()->getIDocumentTimerAccess().StartBackgroundJobs();
    }
    bool IsIdleFormat()  const { return bIdleFormat; }
    void ResetIdleFormat()     { bIdleFormat = false; }

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
                lcl_pCurrShell->GetDoc()->getIDocumentTimerAccess().StartBackgroundJobs();
        }
    }

    /// Makes sure that all requested page-bound Flys find a Page
    void SetAssertFlyPages() { bAssertFlyPages = true; }
    void AssertFlyPages();
    bool IsAssertFlyPages()  { return bAssertFlyPages; }

    /**
     * Makes sure that, starting from the passed Page, all page-bound Frames
     * are on the right Page (pagenumber).
     */
    void AssertPageFlys( SwPageFrm * );

    /// Invalidate all Cntnt, Size or PrtArea
    void InvalidateAllCntnt( sal_uInt8 nInvalidate = INV_SIZE );

    /**
     * Invalidate/re-calculate the position of all floating
     * screen objects (Writer fly frames and drawing objects), which are
     * anchored to paragraph or to character.
    */
    void InvalidateAllObjPos();

    /// Remove superfluous Pages
    void SetSuperfluous()      { bCheckSuperfluous = true; }
    bool IsSuperfluous() const { return bCheckSuperfluous; }
    void RemoveSuperfluous();

    /**
     * Query/set the current Page and the collective Page count
     * We'll format as much as necessary
     */
    sal_uInt16  GetCurrPage( const SwPaM* ) const;
    sal_uInt16  SetCurrPage( SwCursor*, sal_uInt16 nPageNum );
    Point   GetPagePos( sal_uInt16 nPageNum ) const;
    sal_uInt16  GetPageNum() const      { return nPhyPageNums; }
    void    DecrPhyPageNums()       { --nPhyPageNums; }
    void    IncrPhyPageNums()       { ++nPhyPageNums; }
    bool    IsVirtPageNum() const   { return bIsVirtPageNum; }
    inline  void SetVirtPageNum( const bool bOf ) const;
    bool    IsDummyPage( sal_uInt16 nPageNum ) const;

    /**
     * Point rPt: The point that should be used to find the page
     * Size pSize: If given, we return the (first) page that overlaps with the
     * rectangle defined by rPt and pSize
     * bool bExtend: Extend each page to the left/right/top/botton up to the
     * next page margin
     */
    const SwPageFrm* GetPageAtPos( const Point& rPt, const Size* pSize = 0, bool bExtend = false ) const;

    void CalcFrmRects( SwShellCrsr& );

    /**
     * Calculates the cells included from the current selection
     *
     * @returns false: There was no result because of an invalid layout
     * @returns true: Everything worked fine.
     */
    bool MakeTblCrsrs( SwTableCursor& );

    void DisallowTurbo()  const { const_cast<SwRootFrm*>(this)->bTurboAllowed = false; }
    void ResetTurboFlag() const { const_cast<SwRootFrm*>(this)->bTurboAllowed = true; }
    bool IsTurboAllowed() const { return bTurboAllowed; }
    void SetTurbo( const SwCntntFrm *pCntnt ) { pTurbo = pCntnt; }
    void ResetTurbo() { pTurbo = 0; }
    const SwCntntFrm *GetTurbo() { return pTurbo; }

    /// Update the footernumbers of all Pages
    void UpdateFtnNums(); // Only for page by page numnbering!

    /// Remove all footnotes (but no references)
    void RemoveFtns( SwPageFrm *pPage = 0, bool bPageOnly = false,
                     bool bEndNotes = false );
    void CheckFtnPageDescs( bool bEndNote );

    const SwPageFrm *GetLastPage() const { return pLastPage; }
          SwPageFrm *GetLastPage()       { return pLastPage; }

    static bool IsInPaint() { return bInPaint; }

    static void SetNoVirDev( const bool bNew ) { bNoVirDev = bNew; }

    inline long GetBrowseWidth() const;
    void SetBrowseWidth( long n ) { bBrowseWidthValid = true; nBrowseWidth = n;}
    inline void InvalidateBrowseWidth();

    bool IsNewLayout() const { return bIsNewLayout; }
    void ResetNewLayout()    { bIsNewLayout = false;}

    /**
     * Empty SwSectionFrms are registered here for deletion and
     * destroyed later on or deregistered.
     */
    void InsertEmptySct( SwSectionFrm* pDel );
    void DeleteEmptySct() { if( pDestroy ) _DeleteEmptySct(); }
    void RemoveFromList( SwSectionFrm* pSct ) { if( pDestroy ) _RemoveFromList( pSct ); }
#ifdef DBG_UTIL
    bool IsInDelList( SwSectionFrm* pSct ) const;
#endif

    void SetCallbackActionEnabled( bool b ) { bCallbackActionEnabled = b; }
    bool IsCallbackActionEnabled() const    { return bCallbackActionEnabled; }

    bool IsAnyShellAccessible() const { return nAccessibleShells > 0; }
    void AddAccessibleShell() { ++nAccessibleShells; }
    void RemoveAccessibleShell() { --nAccessibleShells; }

    /**
     * Get page frame by phyiscal page number
     * looping through the lowers, which are page frame, in order to find the
     * page frame with the given physical page number.
     * if no page frame is found, 0 is returned.
     * Note: Empty page frames are also returned.
     *
     * @param _nPageNum: physical page number of page frame to be searched and
     *                   returned.
     *
     * @return pointer to the page frame with the given physical page number
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
        const_cast<SwRootFrm*>(this)->ImplCalcBrowseWidth();
    return nBrowseWidth;
}

inline void SwRootFrm::InvalidateBrowseWidth()
{
    if ( bBrowseWidthValid )
        ImplInvalidateBrowseWidth();
}

inline  void SwRootFrm::SetVirtPageNum( const bool bOf) const
{
    const_cast<SwRootFrm*>(this)->bIsVirtPageNum = bOf;
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROOTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
