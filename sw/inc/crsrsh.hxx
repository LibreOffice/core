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
#ifndef INCLUDED_SW_INC_CRSRSH_HXX
#define INCLUDED_SW_INC_CRSRSH_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <vcl/keycod.hxx>
#include <o3tl/typed_flags_set.hxx>

#include "IShellCursorSupplier.hxx"
#include "swdllapi.h"
#include "docary.hxx"
#include "viewsh.hxx"
#include "calbck.hxx"
#include "cshtyp.hxx"
#include "crstate.hxx"
#include "toxe.hxx"
#include "tblsel.hxx"
#include "viscrs.hxx"
#include "node.hxx"
#include "fldbas.hxx"
#include "IDocumentMarkAccess.hxx"

class SfxItemSet;
class SfxPoolItem;
class SwContentFrame;
class SwUnoCursor;
class SwFormatField;
class SwTextFormatColl;
class SwTextINetFormat;
class SwFormatINetFormat;
class SwTextAttr;
class SwTableBox;
class SwTOXMark;
class SwRangeRedline;
class SwBlockCursor;
class SwPostItField;
class SwTextField;
class SwTextFootnote;

namespace i18nutil {
    struct SearchOptions2;
}
namespace com::sun::star::text { class XTextRange; }
namespace com::sun::star::container { class XStringKeyMap; }

// enum and struct to get information via the Doc-Position

enum class IsAttrAtPos
{
    NONE             = 0x0000,
    Field            = 0x0001,
    ClickField       = 0x0002,
    Ftn              = 0x0004,
    InetAttr         = 0x0008,
    TableBoxFml      = 0x0010,
    Redline          = 0x0020,
    Outline          = 0x0040,
    ToxMark          = 0x0080,
    RefMark          = 0x0100,
    NumLabel         = 0x0200,
    ContentCheck     = 0x0400,
    SmartTag         = 0x0800,
    FormControl      = 0x1000,
    TableRedline     = 0x2000
#ifdef DBG_UTIL
    ,CurrAttrs       = 0x4000        ///< only for debugging
    ,TableBoxValue   = 0x8000        ///< only for debugging
#endif
};
namespace o3tl {
    template<> struct typed_flags<IsAttrAtPos> : is_typed_flags<IsAttrAtPos, 0xffff> {};
}

struct SwContentAtPos
{
    union {
        const SwField* pField;
        const SfxPoolItem* pAttr;
        const SwRangeRedline* pRedl;
        SwContentNode * pNode;
        const sw::mark::IFieldmark* pFieldmark;
    } aFnd;
    IsAttrAtPos eContentAtPos;
    int nDist;
    OUString sStr;
    const SwTextAttr* pFndTextAttr;

    SwContentAtPos( IsAttrAtPos eGetAtPos )
        : eContentAtPos( eGetAtPos )
    {
        aFnd.pField = nullptr;
        pFndTextAttr = nullptr;
        nDist = 0; // #i23726#
    }

    bool IsInProtectSect() const;
    bool IsInRTLText() const;
};

// return values of SetCursor (can be combined via ||)
const int CRSR_POSOLD = 0x01,   // cursor stays at old position
          CRSR_POSCHG = 0x02;   // position changed by the layout

namespace sw {

bool ReplaceImpl(SwPaM & rCursor, OUString const& rReplacement,
        bool const bRegExp, SwDoc & rDoc, SwRootFrame const*const pLayout);

/// Helperfunction to resolve backward references in regular expressions
std::optional<OUString> ReplaceBackReferences(const i18nutil::SearchOptions2& rSearchOpt,
        SwPaM* pPam, SwRootFrame const* pLayout );

bool GetRanges(std::vector<std::shared_ptr<SwUnoCursor>> & rRanges,
        SwDoc & rDoc, SwPaM const& rDelPam);

} // namespace sw

class SW_DLLPUBLIC SwCursorShell
    : public SwViewShell
    , public sw::BroadcastingModify
    , public ::sw::IShellCursorSupplier
{
    friend class SwCallLink;
    friend class SwVisibleCursor;
    friend class SwSelPaintRects;

    // requires the Cursor as InternalCursor
    friend bool GetAutoSumSel( const SwCursorShell&, SwCellFrames& );

public:

    /** for calling UpdateCursor */
    enum CursorFlag {
        UPDOWN      = (1 << 0),     ///< keep Up/Down on columns
        SCROLLWIN   = (1 << 1),     ///< scroll window
        CHKRANGE    = (1 << 2),     ///< check overlapping PaMs
        READONLY    = (1 << 3)      ///< make visible in spite of Readonly
    };

    SAL_DLLPRIVATE void UpdateCursor(
        sal_uInt16 eFlags = SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE,
        bool bIdleEnd = false );

private:

    SwRect  m_aCharRect;          ///< Char-SRectangle on which the cursor is located
    Point   m_aCursorHeight;        ///< height & offset from visible Cursor
    Point   m_aOldRBPos;          ///< Right/Bottom of last VisArea
                                // (used in Invalidate by Cursor)

    Link<const SwFlyFrameFormat*,void> m_aFlyMacroLnk;        /**< Link will be called, if the Cursor is set
                                   into a fly. A macro can then be called */
    Link<LinkParamNone*,void> m_aChgLnk;             /**< link will be called by every attribute/
                                   format changes at cursor position.*/
    Link<SwCursorShell&,void> m_aGrfArrivedLnk;      ///< Link calls to UI if a graphic is arrived

    SwShellCursor* m_pCurrentCursor;      ///< current cursor
    SwShellCursor* m_pStackCursor;      ///< stack for the cursor
    SwVisibleCursor *m_pVisibleCursor;        ///< the visible cursor

    SwBlockCursor *m_pBlockCursor;   ///< interface of cursor for block (=rectangular) selection

    SwShellTableCursor* m_pTableCursor; /**< table Cursor; only in tables when the
                                   selection lays over 2 columns */

    SwNodeIndex* m_pBoxIdx;       ///< for recognizing of the changed
    SwTableBox* m_pBoxPtr;        ///< table row

    tools::Long m_nUpDownX;              /**< try to move the cursor on up/down always
                                   in the same column */
    tools::Long m_nLeftFramePos;
    SwNodeOffset m_nCurrentNode;     // save CursorPos at Start-Action
    sal_Int32 m_nCurrentContent;
    SwNodeType m_nCurrentNdTyp;

    /*
     * Via the Method SttCursorMove and EndCursorMove this counter gets
     * incremented/decremented. As long as the counter is inequal to 0, the
     * current Cursor gets no update. This way, "complicated" cursor movements
     * (via Find()) can be realised.
     */
    sal_uInt16 m_nCursorMove;
    CursorMoveState m_eMvState;     ///< Status for Cursor-Travelling - GetModelPositionForViewPoint
    SwTable::SearchType m_eEnhancedTableSel; /// table rows or columns selected by not cell by cell

    OUString m_sMarkedListId;
    int m_nMarkedListLevel;

    bool m_bHasFocus : 1;         ///< Shell is "active" in a window
    bool m_bSVCursorVis : 1;        ///< SV-Cursor visible/invisible
    bool m_bChgCallFlag : 1;      ///< attribute change inside Start- and EndAction
    bool m_bVisPortChgd : 1;      ///< in VisPortChg-Call
                                // (used in Invalidate by the Cursor)

    bool m_bCallChgLnk : 1;       ///< flag for derived classes
                                // true -> call ChgLnk
                                // access only via SwChgLinkFlag
    bool m_bAllProtect : 1;       ///< Flag for areas
                                // true -> everything protected / hidden
    bool m_bInCMvVisportChgd : 1; ///< Flag for CursorMoves
                                // true -> view was moved
    bool m_bGCAttr : 1;           // true -> non expanded attributes exist.
    bool m_bIgnoreReadonly : 1;   // true -> make the cursor visible on next
                                // EndAction in spite of Readonly
    bool m_bSelTableCells : 1;      // true -> select cells over the InputWin
    bool m_bAutoUpdateCells : 1;  // true -> autoformat cells
    bool m_bBasicHideCursor : 1;    // true -> HideCursor from Basic
    bool m_bSetCursorInReadOnly : 1;// true -> Cursor is allowed in ReadOnly-Areas
    bool m_bOverwriteCursor : 1;    // true -> show Overwrite Cursor

    bool m_bMacroExecAllowed : 1;

    SwFrame* m_oldColFrame;

    SAL_DLLPRIVATE void MoveCursorToNum();

    SAL_DLLPRIVATE void ParkPams( SwPaM* pDelRg, SwShellCursor** ppDelRing );

    /** Mark a certain list level of a certain list

        levels of a certain lists are marked now

        @param sListId    list Id of the list whose level is marked
        @param nLevel     to be marked list level

        An empty sListId denotes that no level of a list is marked.
     */
    SAL_DLLPRIVATE void MarkListLevel( const OUString& sListId,
                                      const int nLevel );

    // private method(s) accessed from public inline method(s) must be exported.
                   bool LeftRight( bool, sal_uInt16, sal_uInt16, bool );
    SAL_DLLPRIVATE bool UpDown( bool, sal_uInt16 );
    SAL_DLLPRIVATE bool LRMargin( bool, bool bAPI = false );
    SAL_DLLPRIVATE bool IsAtLRMargin( bool, bool bAPI = false ) const;

    SAL_DLLPRIVATE bool isInHiddenTextFrame(SwShellCursor* pShellCursor);

    SAL_DLLPRIVATE bool GoStartWordImpl();
    SAL_DLLPRIVATE bool GoEndWordImpl();
    SAL_DLLPRIVATE bool GoNextWordImpl();
    SAL_DLLPRIVATE bool GoPrevWordImpl();
    SAL_DLLPRIVATE bool GoNextSentenceImpl();
    SAL_DLLPRIVATE bool GoEndSentenceImpl();
    SAL_DLLPRIVATE bool GoStartSentenceImpl();

    typedef bool (SwCursor::*FNCursor)();
    typedef bool (SwCursorShell::*FNCursorShell)();
    SAL_DLLPRIVATE bool CallCursorFN( FNCursor );
    SAL_DLLPRIVATE bool CallCursorShellFN( FNCursorShell );

    SAL_DLLPRIVATE const SwRangeRedline* GotoRedline_( SwRedlineTable::size_type nArrPos, bool bSelect );

    SAL_DLLPRIVATE void sendLOKCursorUpdates();
protected:

    inline SwMoveFnCollection const & MakeFindRange( SwDocPositions, SwDocPositions, SwPaM* ) const;

    /*
     * Compare-Method for the StackCursor and the current Cursor.
     * The Methods return -1, 0, 1 for lower, equal, greater.
     */
    int CompareCursorStackMkCurrPt() const;

    bool SelTableRowOrCol( bool bRow, bool bRowSimple = false );

    bool SetInFrontOfLabel( bool bNew );

    void RefreshBlockCursor();

    /** Updates the marked list level according to the cursor.
    */
    SAL_DLLPRIVATE void UpdateMarkedListLevel();

protected:
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    SwCursorShell( SwDoc& rDoc, vcl::Window *pWin, const SwViewOption *pOpt );
    // disguised copy constructor
    SwCursorShell( SwCursorShell& rShell, vcl::Window *pWin );
    virtual ~SwCursorShell() override;

    // create new cursor and append the old one
    virtual SwCursor & CreateNewShellCursor() override;
    virtual SwCursor & GetCurrentShellCursor() override;

    SwPaM * CreateCursor();
    ///< delete the current cursor and make the following into the current
    void DestroyCursor();
    ///< transform TableCursor to normal cursor, nullify Tablemode
    void TableCursorToCursor();
    ///< enter block mode, change normal cursor into block cursor
    void CursorToBlockCursor();
    ///< leave block mode, change block cursor into normal cursor
    void BlockCursorToCursor();

    // SelAll() selects the document body content
    // if ExtendedSelect() is called afterwards, the whole nodes array is selected
    // only for usage in special cases allowed!
    void ExtendedSelectAll(bool bFootnotes = true);
    /// If ExtendedSelectAll() was called and selection didn't change since then.
    bool ExtendedSelectedAll();
    /// If document body starts with a table.
    bool StartsWithTable();

    SwCursor* GetCursor( bool bMakeTableCursor = true ) const;
    // return only the current cursor
          SwShellCursor* GetCursor_()                       { return m_pCurrentCursor; }
    const SwShellCursor* GetCursor_() const                 { return m_pCurrentCursor; }

    // show passed cursor - for UNO
    void    SetSelection(const SwPaM& rCursor);

    // remove all cursors from ContentNodes and set to 0
    void ParkCursor( const SwNodeIndex &rIdx );

    // return the current cursor stack
    // (required in EditShell when deleting contents)
    inline SwPaM* GetStackCursor() const;

    // start parenthesing, hide SV-Cursor and selected areas
    void StartAction();
    // end parenthesing, show SV-Cursor and selected areas
    void EndAction( const bool bIdleEnd = false );

    // basic cursor travelling
    tools::Long GetUpDownX() const             { return m_nUpDownX; }

    bool Left( sal_uInt16 nCnt, sal_uInt16 nMode, bool bAllowVisual = false )
        { return LeftRight( true, nCnt, nMode, bAllowVisual ); }
    bool Right( sal_uInt16 nCnt, sal_uInt16 nMode, bool bAllowVisual = false )
        { return LeftRight( false, nCnt, nMode, bAllowVisual ); }
    bool Up( sal_uInt16 nCnt = 1 )      { return UpDown( true, nCnt ); }
    bool Down( sal_uInt16 nCnt = 1 )    { return UpDown( false, nCnt ); }
    bool LeftMargin()               { return LRMargin( true ); }
    bool RightMargin(bool bAPI = false) { return LRMargin( false, bAPI ); }
    bool SttEndDoc( bool bStt );

    bool MovePage( SwWhichPage, SwPosPage );
    bool MovePara( SwWhichPara, SwMoveFnCollection const & );
    bool MoveSection( SwWhichSection, SwMoveFnCollection const & );
    bool MoveTable( SwWhichTable, SwMoveFnCollection const & );
    void MoveColumn( SwWhichColumn, SwPosColumn );
    bool MoveRegion( SwWhichRegion, SwMoveFnCollection const & );

    // note: DO NOT call it FindText because windows.h
    sal_uLong Find_Text( const i18nutil::SearchOptions2& rSearchOpt,
                bool bSearchInNotes,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng, bool bReplace = false );

    sal_uLong FindFormat( const SwTextFormatColl& rFormatColl,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng, const SwTextFormatColl* pReplFormat );

    sal_uLong FindAttrs( const SfxItemSet& rSet, bool bNoCollections,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng,
                const i18nutil::SearchOptions2* pSearchOpt,
                const SfxItemSet* rReplSet );

    //  Position the Cursor
    //  return values:
    //      CRSR_POSCHG: when cursor was corrected from SPoint by the layout
    //      CRSR_POSOLD: when the cursor was not changed
    int SetCursor( const Point &rPt, bool bOnlyText = false, bool bBlock = true );

    /*
     * Notification that the visible area was changed. m_aVisArea is reset, then
     * scrolling is done. The passed Rectangle lays on pixel borders to avoid
     * pixel errors.
     */
    virtual void VisPortChgd( const SwRect & ) override;

    /*
     * virtual paint method to make selection visible again after Paint
     */
    void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle & rRect) override;

    // Areas
    inline void SetMark();
    inline bool HasMark() const;

    void ClearMark();

    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst true: If the point is behind the mark then
       swap the PaM. false: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(bool bPointFirst = true);

    void SwapPam();
    bool TestCurrPam( const Point & rPt,
                      bool bTstHit = false);   // only exact matches
    void KillPams();

    /// store a copy of the current cursor on the cursor stack
    void Push();
    enum class PopMode { DeleteCurrent, DeleteStack };
    /*
     * Delete a cursor
     *    - either from the top of the stack
     *    - or delete the current one and replace it with the cursor from the
     *      stack
     *  @return <true> if there was one on the stack, <false> otherwise
     */
    bool Pop(PopMode);
    /*
     * Combine 2 Cursors.
     * Delete the topmost from the stack and move its Mark into the current.
     */
    void Combine();

    void SttCursorMove();
    void EndCursorMove( const bool bIdleEnd = false );

    /*
     * When the focus is lost the selected ranges are not displayed anymore.
     * On the other hand, on receiving the focus all selected ranges are displayed again
     * (ranges must be recalculated!).
     */
    bool HasShellFocus() const { return m_bHasFocus; }
    void ShellLoseFocus();
    void ShellGetFocus();

    // Methods for displaying or hiding the visible text cursor.
    void ShowCursor();
    void HideCursor();
    // Methods for displaying or hiding the selected ranges with visible cursor.
    void ShowCursors( bool bCursorVis );
    void HideCursors();

    bool IsOverwriteCursor() const { return m_bOverwriteCursor; }
    void SetOverwriteCursor( bool bFlag ) { m_bOverwriteCursor = bFlag; }

    // Return current frame in which the cursor is placed.
    SwContentFrame *GetCurrFrame( const bool bCalcFrame = true ) const;

    //true if cursor is hidden because of readonly.
    //false if it is working despite readonly.
    bool IsCursorReadonly() const;

    // Cursor is placed in something that is protected or selection contains
    // something that is protected.
    bool HasReadonlySel() const;

    // Can the cursor be set to read only ranges?
    bool IsReadOnlyAvailable() const { return m_bSetCursorInReadOnly; }
    void SetReadOnlyAvailable( bool bFlag );
    bool IsOverReadOnlyPos( const Point& rPt ) const;

    // Methods for aFlyMacroLnk.
    void        SetFlyMacroLnk( const Link<const SwFlyFrameFormat*,void>& rLnk ) { m_aFlyMacroLnk = rLnk; }
    const Link<const SwFlyFrameFormat*,void>& GetFlyMacroLnk() const           { return m_aFlyMacroLnk; }

    // Methods returning/altering link for changes of attributes/formats.
    void        SetChgLnk( const Link<LinkParamNone*,void> &rLnk ) { m_aChgLnk = rLnk; }
    const Link<LinkParamNone*,void>& GetChgLnk() const           { return m_aChgLnk; }

    // Methods returning/altering link for "graphic completely loaded".
    void        SetGrfArrivedLnk( const Link<SwCursorShell&,void> &rLnk ) { m_aGrfArrivedLnk = rLnk; }
    const Link<SwCursorShell&,void>& GetGrfArrivedLnk() const           { return m_aGrfArrivedLnk; }

    //Call ChgLink. When within an action calling will be delayed.
    void CallChgLnk();

    // Check if the current cursor contains a selection, i.e.
    // if Mark is set and SPoint and Mark are different.
    bool HasSelection() const;

    // Check if a selection exists, i.e. if the current cursor comprises a selection.
    inline bool IsSelection() const;
    // returns if multiple cursors are available
    inline bool IsMultiSelection() const;

    // Check if a complete paragraph was selected.
    bool IsSelFullPara() const;

    // Check if selection is within one paragraph.

    //Should WaitPtr be activated for Clipboard.
    bool ShouldWait() const;

    // Check if selection is within one paragraph.
    bool IsSelOnePara() const;

    /*
     * Returns SRectangle, at which the cursor is located.
     */
    const SwRect &GetCharRect() const { return m_aCharRect; }
    /*
     * Returns if cursor is wholly or partly within visible range.
     */
    bool IsCursorVisible() const { return VisArea().Overlaps( GetCharRect() ); }
    /*
     * Returns SwRect, at which the character is located.
     */
    void GetCharRectAt(SwRect& rRect, const SwPosition* pPos);

    // Return current page number:
    // true:  in which cursor is located.
    // false: which is visible at the upper margin.
    void GetPageNum( sal_uInt16 &rnPhyNum, sal_uInt16 &rnVirtNum,
                     bool bAtCursorPos = true, const bool bCalcFrame = true );
    // Returns current page's sequential number (1-based),in which cursor is located, ignoring autoinserted empty pages.
    // Returns 0 on error
    sal_uInt16 GetPageNumSeqNonEmpty();
    // Determine how "empty pages" are handled
    // (used in PhyPage).
    sal_uInt16 GetNextPrevPageNum( bool bNext = true );

    // Move cursor at the beginning of page "nPage".
    bool GotoPage( sal_uInt16 nPage );

    sal_uInt16 GetPageCnt();

    bool GoNextCursor();

    bool GoPrevCursor();

    void GoNextPrevCursorSetSearchLabel(const bool bNext);

    // at CurrentCursor.SPoint
    ::sw::mark::IMark* SetBookmark(
        const vcl::KeyCode&,
        const OUString& rName,
        IDocumentMarkAccess::MarkType eMark = IDocumentMarkAccess::MarkType::BOOKMARK);
    ::sw::mark::IMark* SetBookmark2(
        const vcl::KeyCode&,
        const OUString& rName,
        bool bHide,
        const OUString& rCondition);
    bool GotoMark( const ::sw::mark::IMark* const pMark );    // sets CurrentCursor.SPoint
    bool GotoMark( const ::sw::mark::IMark* const pMark, bool bAtStart );
    bool GoNextBookmark(); // true, if there was one
    bool GoPrevBookmark();

    bool IsFormProtected();
    ::sw::mark::IFieldmark* GetCurrentFieldmark();
    ::sw::mark::IFieldmark* GetFieldmarkAfter();
    ::sw::mark::IFieldmark* GetFieldmarkBefore();
    bool GotoFieldmark( const ::sw::mark::IFieldmark* const pMark );

    // update Cursr, i.e. reset it into content should only be called when the
    // cursor was set to a random position e.g. when deleting frames
    void UpdateCursorPos();

    // get the selected text at the current cursor. It will be filled with
    // fields etc.
    OUString GetSelText() const;

    // Check of SPoint or Mark of current cursor are placed within a table.
    inline const SwTableNode* IsCursorInTable() const;

    bool IsCursorInFootnote() const;

    inline Point& GetCursorDocPos() const;
    inline bool IsCursorPtAtEnd() const;

    inline const  SwPaM* GetTableCrs() const;
    inline        SwPaM* GetTableCrs();

    bool IsTableComplexForChart();
    // get current table selection as text
    OUString GetBoxNms() const;

    // set Cursor to the next/previous cell
    bool GoNextCell( bool bAppendLine = true );
    bool GoPrevCell();
    // go to this box (if available and inside of table)
    bool GotoTable( const OUString& rName );

    // select a table row, column or box (based on the current cursor)
    bool SelTableRow() { return SelTableRowOrCol( true  ); }
    bool SelTableCol() { return SelTableRowOrCol( false ); }
    bool SelTableBox();

    bool SelTable();

    void GotoNextNum();
    void GotoPrevNum();

    bool GotoOutline( const OUString& rName );
    // to the next/previous or the given OutlineNode
    void GotoOutline( SwOutlineNodes::size_type nIdx );
    // find the "outline position" in the nodes array of the current chapter
    SwOutlineNodes::size_type GetOutlinePos(sal_uInt8 nLevel = UCHAR_MAX, SwPaM* pPaM = nullptr);
    // select the given range of OutlineNodes. Optionally including the children
    // the sal_uInt16s are the positions in OutlineNodes-Array (EditShell)
    void MakeOutlineSel(SwOutlineNodes::size_type nSttPos, SwOutlineNodes::size_type nEndPos,
                         bool bWithChildren, bool bKillPams = true );

    bool GotoNextOutline();
    bool GotoPrevOutline();

    /** Delivers the current shell cursor

        Some operations have to run on the current cursor ring,
        some on the m_pTableCursor (if exist) or the current cursor ring and
        some on the m_pTableCursor or m_pBlockCursor or the current cursor ring.
        This small function checks the existence and delivers the wished cursor.

        @param bBlock [bool]
        if the block cursor is of interest or not

        @return m_pTableCursor if exist,
        m_pBlockCursor if exist and of interest (param bBlock)
        otherwise m_pCurrentCursor
    */
    SwShellCursor* getShellCursor( bool bBlock );
    const SwShellCursor* getShellCursor( bool bBlock ) const
        { return const_cast<SwCursorShell*>(this)->getShellCursor( bBlock ); }

    bool IsBlockMode() const { return nullptr != m_pBlockCursor; }

    // is the Cursor in a table and is the selection over 2 columns
    bool IsTableMode() const { return nullptr != m_pTableCursor; }

    const SwShellTableCursor* GetTableCursor() const { return m_pTableCursor; }
    SwShellTableCursor* GetTableCursor() { return m_pTableCursor; }
    size_t UpdateTableSelBoxes();

    bool GotoFootnoteText();      ///< jump from content to footnote
    bool GotoFootnoteAnchor();   ///< jump from footnote to anchor
    bool GotoPrevFootnoteAnchor();
    bool GotoNextFootnoteAnchor();

    void GotoFlyAnchor();       ///< jump from the frame to the anchor
    bool GotoHeaderText();       ///< jump from the content to the header
    bool GotoFooterText();       ///< jump from the content to the footer
    // jump to the header/footer of the given or current PageDesc
    bool SetCursorInHdFt( size_t nDescNo, bool bInHeader );
    // is point of cursor in header/footer. pbInHeader return true if it is
    // in a headerframe otherwise in a footerframe
    bool IsInHeaderFooter( bool* pbInHeader = nullptr ) const;

    bool GotoNextTOXBase( const OUString* = nullptr );
    bool GotoPrevTOXBase( const OUString* = nullptr );
    void GotoTOXMarkBase();
    // jump to the next or previous index entry
    bool GotoNxtPrvTOXMark( bool bNext = true );
    // jump to the next/previous index mark of this type
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rStart, SwTOXSearch eDir );

    // jump to the next or previous table formula
    // optionally only to broken formulas
    bool GotoNxtPrvTableFormula( bool bNext = true,
                               bool bOnlyErrors = false );
    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    bool SelectNxtPrvHyperlink( bool bNext );

    bool GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType,
                            sal_uInt16 nSeqNo );

    // get the nth character from the start or end of the  current selection
    sal_Unicode GetChar( bool bEnd = true, tools::Long nOffset = 0 );
    bool ExtendSelection( bool bEnd = true, sal_Int32 nCount = 1 );

    // Place only the visible cursor at the given position in the document.
    // Return false if SPoint was corrected by layout.
    // (This is needed for displaying the Drag&Drop/Copy-Cursor.)
    bool SetVisibleCursor( const Point &rPt );
    inline void UnSetVisibleCursor();
    SwVisibleCursor* GetVisibleCursor() const;

    // jump to the next or previous field of the corresponding type
    bool MoveFieldType(
        const SwFieldType* pFieldType,
        const bool bNext,
        const SwFieldIds nResType = SwFieldIds::Unknown,
        const bool bAddSetExpressionFieldsToInputFields = true );

    bool GotoFormatField( const SwFormatField& rField );

    static SwTextField* GetTextFieldAtPos(
        const SwPosition* pPos,
        const bool bIncludeInputFieldAtStart );
    static SwTextField* GetTextFieldAtCursor(
        const SwPaM* pCursor,
        const bool bIncludeInputFieldAtStart );
    static SwField* GetFieldAtCursor(
        const SwPaM* pCursor,
        const bool bIncludeInputFieldAtStart );
    SwField* GetCurField( const bool bIncludeInputFieldAtStart = false ) const;
    bool CursorInsideInputField() const;
    static bool PosInsideInputField( const SwPosition& rPos );
    bool DocPtInsideInputField( const Point& rDocPt ) const;
    static sal_Int32 StartOfInputFieldAtPos( const SwPosition& rPos );
    static sal_Int32 EndOfInputFieldAtPos( const SwPosition& rPos );

    // Return number of cursors in ring (The flag indicates whether
    // only cursors containing selections are requested).
    sal_uInt16 GetCursorCnt( bool bAll = true ) const;

    // Char Travelling - methods (in crstrvl1.cxx)
    bool GoStartWord();
    bool GoEndWord();
    bool GoNextWord();
    bool GoPrevWord();
    bool GoNextSentence();
    bool GoStartSentence();
    bool GoEndSentence();
    bool SelectWord( const Point* pPt );
    void ExpandToSentenceBorders();

    // get position from current cursor
    bool IsStartWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES )const;
    bool IsEndWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsInWord( sal_Int16 nWordType = css::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    bool IsStartSentence() const;
    bool IsEndSentence() const;
    bool IsSttPara() const;
    bool IsEndPara() const;
    bool IsEndOfTable() const; ///< at the very last SwPosition inside a table
    bool IsStartOfDoc() const;
    bool IsEndOfDoc() const;
    bool IsInFrontOfLabel() const;
    bool IsAtLeftMargin()   const       { return IsAtLRMargin( true ); }
    bool IsAtRightMargin() const   { return IsAtLRMargin( false, true/*bAPI*/ ); }

    // delete all created cursors, set the table cursor and the last cursor to
    // its TextNode (or StartNode?)
    // They all get created on the next ::GetCursor again
    // Used for Drag&Drop/Clipboard-Paste in tables
    bool ParkTableCursor();

    // Non expanded attributes?
    bool IsGCAttr() const { return m_bGCAttr; }
    void ClearGCAttr() { m_bGCAttr = false; }
    void    UpdateAttr() {  m_bGCAttr = true; }

    // is the whole document protected/hidden (for UI...)
    bool IsAllProtect() const { return m_bAllProtect; }

    bool GotoRegion( std::u16string_view rName );

    // show the current selection
    virtual void MakeSelVisible();

    // set the cursor to a NOT protected/hidden node
    bool FindValidContentNode( bool bOnlyText );

    bool GetContentAtPos( const Point& rPt,
                          SwContentAtPos& rContentAtPos,
                          bool bSetCursor = false,
                          SwRect* pFieldRect = nullptr );

    const SwPostItField* GetPostItFieldAtCursor() const;

    // get smart tags rectangle for the given point
    void GetSmartTagRect( const Point& rPt, SwRect& rSelectRect );

    // get smart tags at current cursor position
    void GetSmartTagTerm( std::vector< OUString >& rSmartTagTypes,
                          css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                          css::uno::Reference<css::text::XTextRange>& rRange ) const;

    bool IsPageAtPos( const Point &rPt ) const;

    bool SelectTextAttr( sal_uInt16 nWhich, bool bExpand, const SwTextAttr* pAttr = nullptr );
    bool GotoINetAttr( const SwTextINetFormat& rAttr );
    const SwFormatINetFormat* FindINetAttr( std::u16string_view rName ) const;

    bool SelectText( const sal_Int32 nStart,
                        const sal_Int32 nEnd );

    bool CheckTableBoxContent( const SwPosition* pPos = nullptr );
    void SaveTableBoxContent( const SwPosition* pPos = nullptr );
    void ClearTableBoxContent();
    bool EndAllTableBoxEdit();

    void SetSelTableCells( bool bFlag )           { m_bSelTableCells = bFlag; }
    bool IsSelTableCells() const                  { return m_bSelTableCells; }

    void UnsetEnhancedTableSelection()            { m_eEnhancedTableSel = SwTable::SEARCH_NONE; }
    SwTable::SearchType GetEnhancedTableSelection() const  { return m_eEnhancedTableSel; }

    bool IsAutoUpdateCells() const              { return m_bAutoUpdateCells; }
    void SetAutoUpdateCells( bool bFlag )       { m_bAutoUpdateCells = bFlag; }

    bool GetShadowCursorPos( const Point& rPt, SwFillMode eFillMode,
                            SwRect& rRect, sal_Int16& rOrient );
    bool SetShadowCursorPos( const Point& rPt, SwFillMode eFillMode );

    const SwRangeRedline* SelNextRedline();
    const SwRangeRedline* SelPrevRedline();
    const SwRangeRedline* GotoRedline( SwRedlineTable::size_type nArrPos, bool bSelect );

    bool GotoFootnoteAnchor(const SwTextFootnote& rTextFootnote);

    SAL_DLLPRIVATE SvxFrameDirection GetTextDirection( const Point* pPt = nullptr ) const;
    // is cursor or the point in/over a vertical formatted text?
    bool IsInVerticalText( const Point* pPt = nullptr ) const;
    // is cursor or the point in/over a right to left formatted text?
    bool IsInRightToLeftText() const;

    static void FirePageChangeEvent(sal_uInt16 nOldPage, sal_uInt16 nNewPage);
    bool   bColumnChange();
    static void FireSectionChangeEvent(sal_uInt16 nOldSection, sal_uInt16 nNewSection);
    static void FireColumnChangeEvent(sal_uInt16 nOldColumn, sal_uInt16 nNewColumn);
    // If the current cursor position is inside a hidden range, the hidden range
    // is selected and true is returned:
    bool SelectHiddenRange();

    // remove all invalid cursors
    void ClearUpCursors();

    void SetMacroExecAllowed( const bool _bMacroExecAllowed )
    {
        m_bMacroExecAllowed = _bMacroExecAllowed;
    }
    bool IsMacroExecAllowed() const
    {
        return m_bMacroExecAllowed;
    }

    /**
       Returns textual description of the current selection.

       - If the current selection is a multi-selection the result is
         STR_MULTISEL.
       - Else the result is the text of the selection.

       @return the textual description of the current selection
     */
    OUString GetCursorDescr() const;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    /// Implementation of lok::Document::getPartPageRectangles() for Writer.
    OUString getPageRectangles();

    /// See SwView::NotifyCursor().
    void NotifyCursor(SfxViewShell* pViewShell) const;
};

// Cursor Inlines:
inline SwMoveFnCollection const & SwCursorShell::MakeFindRange(
            SwDocPositions nStt, SwDocPositions nEnd, SwPaM* pPam ) const
{
    return m_pCurrentCursor->MakeFindRange( nStt, nEnd, pPam );
}

inline SwPaM* SwCursorShell::GetStackCursor() const { return m_pStackCursor; }

inline void SwCursorShell::SetMark() { m_pCurrentCursor->SetMark(); }

inline bool SwCursorShell::HasMark() const { return m_pCurrentCursor->HasMark(); }

inline bool SwCursorShell::IsSelection() const
{
    return IsTableMode() || m_pCurrentCursor->HasMark() ||
            m_pCurrentCursor->GetNext() != m_pCurrentCursor;
}
inline bool SwCursorShell::IsMultiSelection() const
{
    return m_pCurrentCursor->GetNext() != m_pCurrentCursor;
}

inline const SwTableNode* SwCursorShell::IsCursorInTable() const
{
    return m_pCurrentCursor->GetNode().FindTableNode();
}

inline bool SwCursorShell::IsCursorPtAtEnd() const
{
    return m_pCurrentCursor->End() == m_pCurrentCursor->GetPoint();
}

inline Point& SwCursorShell::GetCursorDocPos() const
{
    return m_pCurrentCursor->GetPtPos();
}

inline const SwPaM* SwCursorShell::GetTableCrs() const
{
    return m_pTableCursor;
}

inline SwPaM* SwCursorShell::GetTableCrs()
{
    return m_pTableCursor;
}

inline void SwCursorShell::UnSetVisibleCursor()
{
    m_pVisibleCursor->Hide();
    m_pVisibleCursor->SetDragCursor( false );
}

#endif  // _CRSRSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
