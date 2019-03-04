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
#include <o3tl/typed_flags_set.hxx>
#include <set>
#include <vector>

class SwContentFrame;
class SwViewShell;
class SdrPage;
class SwFrameFormat;
class SwPaM;
class SwCursor;
class SwShellCursor;
class SwTableCursor;
class SwLayVout;
class SwViewOption;
class SwSelectionList;
struct SwPosition;
struct SwCursorMoveState;

namespace sw {
    enum class RedlineMode
    {
        Shown, Hidden
    };
};

enum class SwInvalidateFlags
{
    Size      = 0x01,
    PrtArea   = 0x02,
    Pos       = 0x04,
    Table     = 0x08,
    Section   = 0x10,
    LineNum   = 0x20,
    Direction = 0x40,
};

namespace o3tl
{
    template<> struct typed_flags<SwInvalidateFlags> : is_typed_flags<SwInvalidateFlags, 0x7f> {};
};

enum class SwRemoveResult
{
    Next,
    Prev
};

using SwCurrShells = std::set<CurrShell*>;

class SwSectionFrame;
using SwDestroyList = std::set<SwSectionFrame*>;

/// The root element of a Writer document layout. Lower frames are expected to
/// be SwPageFrame instances.
class SwRootFrame: public SwLayoutFrame
{
    // Needs to disable the Superfluous temporarily
    friend void AdjustSizeChgNotify( SwRootFrame *pRoot );

    // Maintains the mpLastPage (Cut() and Paste() of SwPageFrame
    friend inline void SetLastPage( SwPageFrame* );

    // For creating and destroying of the virtual output device manager
    friend void FrameInit(); // Creates s_pVout
    friend void FrameFinit(); // Destroys s_pVout

    std::vector<SwRect> maPageRects;// returns the current rectangle for each page frame
                                    // the rectangle is extended to the top/bottom/left/right
                                    // for pages located at the outer margins
    SwRect  maPagesArea;            // the area covered by the pages
    long    mnViewWidth;            // the current page layout bases on this view width
    sal_uInt16  mnColumns;          // the current page layout bases on this number of columns
    bool    mbBookMode;             // the current page layout is in book view
    bool    mbSidebarChanged;       // the notes sidebar state has changed

    bool    mbNeedGrammarCheck;     // true when sth needs to be checked (not necessarily started yet!)

    static SwLayVout     *s_pVout;
    static bool           s_isInPaint; // Protection against double Paints
    static bool           s_isNoVirDev;// No virt. Device for SystemPaints

    /// Width of the HTML / Web document if not defined otherwise: 20cm.
    static constexpr sal_Int64 MIN_BROWSE_WIDTH = convertMm100ToTwip(20000);

    bool    mbCheckSuperfluous   :1; // Search for empty Pages?
    bool    mbIdleFormat         :1; // Trigger Idle Formatter?
    bool    mbBrowseWidthValid   :1; // Is mnBrowseWidth valid?
    bool    mbTurboAllowed       :1;
    bool    mbAssertFlyPages     :1; // Insert more Pages for Flys if needed?
    bool    mbIsVirtPageNum      :1; // Do we have a virtual pagenumber?
    bool    mbIsNewLayout        :1; // Layout loaded or newly created
    bool    mbCallbackActionEnabled:1; // No Action in Notification desired
                                      // @see dcontact.cxx, ::Changed()
    bool    mbLayoutFreezed;
    bool    mbHideRedlines;

    /**
     * For BrowseMode
     * mnBrowseWidth is the outer margin of the object most to the right.
     * The page's right edge should not be smaller than this value.
     */
    long    mnBrowseWidth;

    /// If we only have to format one ContentFrame, its in mpTurbo
    const SwContentFrame *mpTurbo;

    /// We should not need to always struggle to find the last page, so store it here
    SwPageFrame *mpLastPage;

    /** [ Comment from the original StarOffice checkin ]:
     * The root takes care of the shell access. Via the document
     * it should be possible to get at the root frame, and thus always
     * have access to the shell.
     * the pointer mpCurrShell is the pointer to any of the shells for
     * the document.
     * Because sometimes it matters which shell is used, it is necessary to
     * know the active shell.
     * this is approximated by setting the pointer mpCurrShell when a
     * shell gets the focus (FEShell). Additionally the pointer will be
     * set temporarily by SwCurrShell typically via  SET_CURR_SHELL
     * The macro and class can be found in the SwViewShell. These object can
     * be created nested (also for different kinds of Shells). They are
     * collected into the Array mpCurrShells.
     * Furthermore it can happen that a shell is activated while a curshell
     * object is still 'active'. This one will be entered into mpWaitingCurrShell
     * and will be activated by the last d'tor of CurrShell.
     * One other problem is the destruction of a shell while it is active.
     * The pointer mpCurrShell is then reset to an arbitrary other shell.
     * If at the time of the destruction of a shell, which is still referenced
     * by a curshell object, that will be cleaned up as well.
     */
    friend class CurrShell;
    friend void SetShell( SwViewShell *pSh );
    friend void InitCurrShells( SwRootFrame *pRoot );
    SwViewShell *mpCurrShell;
    SwViewShell *mpWaitingCurrShell;
    std::unique_ptr<SwCurrShells> mpCurrShells;

    /// One Page per DrawModel per Document; is always the size of the Root
    SdrPage *mpDrawPage;

    std::unique_ptr<SwDestroyList> mpDestroy;

    sal_uInt16  mnPhyPageNums; /// Page count
    sal_uInt16 mnAccessibleShells; // Number of accessible shells

    void ImplCalcBrowseWidth();
    void ImplInvalidateBrowseWidth();

    void DeleteEmptySct_(); // Destroys the registered SectionFrames
    void RemoveFromList_( SwSectionFrame* pSct ); // Removes SectionFrames from the Delete List

    virtual void DestroyImpl() override;
    virtual ~SwRootFrame() override;

protected:

    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;

public:

    /// Remove MasterObjects from the Page (called by the ctors)
    static void RemoveMasterObjs( SdrPage *pPg );

    void AllCheckPageDescs() const;
    void AllInvalidateAutoCompleteWords() const;
    void AllAddPaintRect() const;
    void AllRemoveFootnotes() ;
    void AllInvalidateSmartTagsOrSpelling(bool bSmartTags) const;

    /// Output virtual Device (e.g. for animations)
    static bool FlushVout();

    /// Save Clipping if exactly the ClipRect is outputted
    static bool HasSameRect( const SwRect& rRect );

    SwRootFrame( SwFrameFormat*, SwViewShell* );
    void Init(SwFrameFormat*);

    SwViewShell *GetCurrShell() const { return mpCurrShell; }
    void DeRegisterShell( SwViewShell *pSh );

    /**
     * Set up Start-/EndAction for all Shells on a as high as possible
     * (Shell section) level.
     * For the StarONE binding, which does not know the Shells directly.
     * The ChangeLinkd of the CursorShell (UI notifications) is called
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

    const SdrPage* GetDrawPage() const { return mpDrawPage; }
          SdrPage* GetDrawPage()       { return mpDrawPage; }
          void     SetDrawPage( SdrPage* pNew ){ mpDrawPage = pNew; }

    virtual bool  GetCursorOfst( SwPosition *, Point&,
                               SwCursorMoveState* = nullptr, bool bTestBackground = false ) const override;

    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) override;
#ifdef DBG_UTIL
    virtual void Cut() override;
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) override;
#endif

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const override;

    Point  GetNextPrevContentPos( const Point &rPoint, bool bNext ) const;

    virtual Size ChgSize( const Size& aNewSize ) override;

    void SetIdleFlags()
    {
        mbIdleFormat = true;

        SwViewShell* pCurrShell = GetCurrShell();
        // May be NULL if called from SfxBaseModel::dispose
        // (this happens in the build test 'rtfexport').
        if (pCurrShell != nullptr)
            pCurrShell->GetDoc()->getIDocumentTimerAccess().StartIdling();
    }
    bool IsIdleFormat()  const { return mbIdleFormat; }
    void ResetIdleFormat()     { mbIdleFormat = false; }

    bool IsNeedGrammarCheck() const         { return mbNeedGrammarCheck; }
    void SetNeedGrammarCheck( bool bVal )
    {
        mbNeedGrammarCheck = bVal;

        if ( bVal )
        {
            SwViewShell* pCurrShell = GetCurrShell();
            // May be NULL if called from SfxBaseModel::dispose
            // (this happens in the build test 'rtfexport').
            if (pCurrShell != nullptr)
                pCurrShell->GetDoc()->getIDocumentTimerAccess().StartIdling();
        }
    }

    /// Makes sure that all requested page-bound Flys find a Page
    void SetAssertFlyPages() { mbAssertFlyPages = true; }
    void AssertFlyPages();
    bool IsAssertFlyPages()  { return mbAssertFlyPages; }

    /**
     * Makes sure that, starting from the passed Page, all page-bound Frames
     * are on the right Page (pagenumber).
     */
    static void AssertPageFlys( SwPageFrame * );

    /// Invalidate all Content, Size or PrtArea
    void InvalidateAllContent( SwInvalidateFlags nInvalidate );

    /**
     * Invalidate/re-calculate the position of all floating
     * screen objects (Writer fly frames and drawing objects), which are
     * anchored to paragraph or to character.
    */
    void InvalidateAllObjPos();

    /// Remove superfluous Pages
    void SetSuperfluous()      { mbCheckSuperfluous = true; }
    bool IsSuperfluous() const { return mbCheckSuperfluous; }
    void RemoveSuperfluous();

    /**
     * Query/set the current Page and the collective Page count
     * We'll format as much as necessary
     */
    sal_uInt16  GetCurrPage( const SwPaM* ) const;
    sal_uInt16  SetCurrPage( SwCursor*, sal_uInt16 nPageNum );
    Point   GetPagePos( sal_uInt16 nPageNum ) const;
    sal_uInt16  GetPageNum() const      { return mnPhyPageNums; }
    void    DecrPhyPageNums()       { --mnPhyPageNums; }
    void    IncrPhyPageNums()       { ++mnPhyPageNums; }
    bool    IsVirtPageNum() const   { return mbIsVirtPageNum; }
    inline  void SetVirtPageNum( const bool bOf ) const;
    bool    IsDummyPage( sal_uInt16 nPageNum ) const;

    /**
     * Point rPt: The point that should be used to find the page
     * Size pSize: If given, we return the (first) page that overlaps with the
     * rectangle defined by rPt and pSize
     * bool bExtend: Extend each page to the left/right/top/bottom up to the
     * next page margin
     */
    const SwPageFrame* GetPageAtPos( const Point& rPt, const Size* pSize = nullptr, bool bExtend = false ) const;

    /**
    * Point rPt: The point to test
    * @returns true: if rPt is between top/bottom margins of two pages
    *                in hide-whitespace, rPt can be near the gap, but
    *                not strictly between pages (in a page) as gap is small.
    * @returns false: if rPt is in a page or not strictly between two pages
    */
    bool IsBetweenPages(const Point& rPt) const;

    void CalcFrameRects( SwShellCursor& );

    /**
     * Calculates the cells included from the current selection
     *
     * @returns false: There was no result because of an invalid layout
     * @returns true: Everything worked fine.
     */
    bool MakeTableCursors( SwTableCursor& );

    void DisallowTurbo()  const { const_cast<SwRootFrame*>(this)->mbTurboAllowed = false; }
    void ResetTurboFlag() const { const_cast<SwRootFrame*>(this)->mbTurboAllowed = true; }
    bool IsTurboAllowed() const { return mbTurboAllowed; }
    void SetTurbo( const SwContentFrame *pContent ) { mpTurbo = pContent; }
    void ResetTurbo() { mpTurbo = nullptr; }
    const SwContentFrame *GetTurbo() { return mpTurbo; }

    /// Update the footnote numbers of all Pages
    void UpdateFootnoteNums(); // Only for page by page numbering!

    /// Remove all footnotes (but no references)
    void RemoveFootnotes( SwPageFrame *pPage = nullptr, bool bPageOnly = false,
                     bool bEndNotes = false );
    void CheckFootnotePageDescs( bool bEndNote );

    const SwPageFrame *GetLastPage() const { return mpLastPage; }
          SwPageFrame *GetLastPage()       { return mpLastPage; }

    static bool IsInPaint() { return s_isInPaint; }

    static void SetNoVirDev(const bool bNew) { s_isNoVirDev = bNew; }

    inline long GetBrowseWidth() const;
    inline void InvalidateBrowseWidth();

    bool IsNewLayout() const { return mbIsNewLayout; }
    void ResetNewLayout()    { mbIsNewLayout = false;}

    /**
     * Empty SwSectionFrames are registered here for deletion and
     * destroyed later on or deregistered.
     */
    void InsertEmptySct( SwSectionFrame* pDel );
    void DeleteEmptySct() { if( mpDestroy ) DeleteEmptySct_(); }
    void RemoveFromList( SwSectionFrame* pSct ) { if( mpDestroy ) RemoveFromList_( pSct ); }
#ifdef DBG_UTIL
    bool IsInDelList( SwSectionFrame* pSct ) const;
#endif

    void SetCallbackActionEnabled( bool b ) { mbCallbackActionEnabled = b; }
    bool IsCallbackActionEnabled() const    { return mbCallbackActionEnabled; }

    bool IsAnyShellAccessible() const { return mnAccessibleShells > 0; }
    void AddAccessibleShell() { ++mnAccessibleShells; }
    void RemoveAccessibleShell() { --mnAccessibleShells; }

    /**
     * Get page frame by physical page number
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
    SwPageFrame* GetPageByPageNum( sal_uInt16 _nPageNum ) const;

    void CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea );
    bool IsLeftToRightViewLayout() const;
    const SwRect& GetPagesArea() const { return maPagesArea; }
    void SetSidebarChanged() { mbSidebarChanged = true; }

    bool IsLayoutFreezed() const { return mbLayoutFreezed; }
    void FreezeLayout( bool freeze ) { mbLayoutFreezed = freeze; }

    void RemovePage( SwPageFrame **pDel, SwRemoveResult eResult );

    /**
     * Replacement for sw::DocumentRedlineManager::GetRedlineFlags()
     * (this is layout-level redline hiding).
     */
    bool IsHideRedlines() const { return mbHideRedlines; }
    void SetHideRedlines(bool);
};

inline long SwRootFrame::GetBrowseWidth() const
{
    if ( !mbBrowseWidthValid )
        const_cast<SwRootFrame*>(this)->ImplCalcBrowseWidth();
    return mnBrowseWidth;
}

inline void SwRootFrame::InvalidateBrowseWidth()
{
    if ( mbBrowseWidthValid )
        ImplInvalidateBrowseWidth();
}

inline  void SwRootFrame::SetVirtPageNum( const bool bOf) const
{
    const_cast<SwRootFrame*>(this)->mbIsVirtPageNum = bOf;
}

/// helper class to disable creation of an action by a callback event
/// in particular, change event from a drawing object (SwDrawContact::Changed())
class DisableCallbackAction
{
    private:
        SwRootFrame & m_rRootFrame;
        bool const m_bOldCallbackActionState;

    public:
        explicit DisableCallbackAction(SwRootFrame & rRootFrame)
            : m_rRootFrame(rRootFrame)
            , m_bOldCallbackActionState(rRootFrame.IsCallbackActionEnabled())
        {
            m_rRootFrame.SetCallbackActionEnabled(false);
        }

        ~DisableCallbackAction()
        {
            m_rRootFrame.SetCallbackActionEnabled(m_bOldCallbackActionState);
        }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROOTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
