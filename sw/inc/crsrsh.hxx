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

#include <IShellCursorSupplier.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <viewsh.hxx>
#include <calbck.hxx>
#include <cshtyp.hxx>
#include <crstate.hxx>
#include <toxe.hxx>
#include <tblsel.hxx>
#include <viscrs.hxx>
#include <node.hxx>
#include <IDocumentMarkAccess.hxx>

class SfxItemSet;
class SfxPoolItem;
class SwContentFrm;
class SwCrsrShell;
class SwCursor;
class SwField;
class SwFieldType;
class SwFormat;
class SwFormatField;
class SwNodeIndex;
class SwPaM;
class SwShellCrsr;
class SwShellTableCrsr;
class SwTableNode;
class SwTextFormatColl;
class SwVisCrsr;
class SwTextINetFormat;
class SwFormatINetFormat;
class SwTextAttr;
class SwTableBox;
class SwTOXMark;
class SwRangeRedline;
class SwBlockCursor;
class SwContentNode;
class SwPostItField;
class SwTextField;
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

// enum and struct to get information via the Doc-Position

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
        SW_NUMLABEL         = 0x0200,
        SW_CONTENT_CHECK    = 0x0400,
        SW_SMARTTAG         = 0x0800,
        SW_FORMCTRL         = 0x1000
#ifdef DBG_UTIL
        ,SW_CURR_ATTRS      = 0x4000        ///< only for debugging
        ,SW_TABLEBOXVALUE   = 0x8000        ///< only for debugging
#endif
    } eContentAtPos;

    union {
        const SwField* pField;
        const SfxPoolItem* pAttr;
        const SwRangeRedline* pRedl;
        SwContentNode * pNode;
        const sw::mark::IFieldmark* pFieldmark;
    } aFnd;

    int nDist;

    OUString sStr;
    const SwTextAttr* pFndTextAttr;

    SwContentAtPos( int eGetAtPos = 0xffff )
        : eContentAtPos( (IsAttrAtPos)eGetAtPos )
    {
        aFnd.pField = nullptr;
        pFndTextAttr = nullptr;
        nDist = 0; // #i23726#
    }

    bool IsInProtectSect() const;
    bool     IsInRTLText()const;
};

// return values of SetCrsr (can be combined via ||)
const int CRSR_POSOLD = 0x01,   // cursor stays at old position
          CRSR_POSCHG = 0x02;   // position changed by the layout

/// Helperfunction to resolve backward references in regular expressions
OUString *ReplaceBackReferences( const css::util::SearchOptions& rSearchOpt, SwPaM* pPam );

class SW_DLLPUBLIC SwCrsrShell
    : public SwViewShell
    , public SwModify
    , public ::sw::IShellCursorSupplier
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;

    // requires the Crsr as InternalCrsr
    friend bool GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:

    /** for calling UpdateCrsr */
    enum CrsrFlag {
        UPDOWN      = (1 << 0),     ///< keep Up/Down on columns
        SCROLLWIN   = (1 << 1),     ///< scroll window
        CHKRANGE    = (1 << 2),     ///< check overlapping PaMs
        READONLY    = (1 << 3)      ///< make visible in spite of Readonly
    };

private:

    SwRect  m_aCharRect;          ///< Char-SRectangle on which the cursor is located
    Point   m_aCrsrHeight;        ///< height & offset from visible Cursor
    Point   m_aOldRBPos;          ///< Right/Bottom of last VisArea
                                // (used in Invalidate by Cursor)

    Link<const SwFlyFrameFormat*,void> m_aFlyMacroLnk;        /**< Link will be called, if the Crsr is set
                                   into a fly. A macro can then be called */
    Link<SwCrsrShell*,void> m_aChgLnk;             /**< link will be called by every attribute/
                                   format changes at cursor position.*/
    Link<SwCrsrShell&,void> m_aGrfArrivedLnk;      ///< Link calls to UI if a graphic is arrived

    SwShellCrsr* m_pCurCrsr;      ///< current cursor
    SwShellCrsr* m_pCrsrStk;      ///< stack for the cursor
    SwVisCrsr *m_pVisCrsr;        ///< the visible cursor

    SwBlockCursor *m_pBlockCrsr;   ///< interface of cursor for block (=rectangular) selection

    SwShellTableCrsr* m_pTableCrsr; /**< table Crsr; only in tables when the
                                   selection lays over 2 columns */

    SwNodeIndex* m_pBoxIdx;       ///< for recognizing of the changed
    SwTableBox* m_pBoxPtr;        ///< table row

    long m_nUpDownX;              /**< try to move the cursor on up/down always
                                   in the same column */
    long m_nLeftFrmPos;
    sal_uLong m_nAktNode;             // save CursorPos at Start-Action
    sal_Int32 m_nAktContent;
    sal_uInt16 m_nAktNdTyp;
    bool m_bAktSelection;

    /*
     * Via the Method SttCrsrMove and EndCrsrMove this counter gets
     * incremented/decremented. As long as the counter is inequal to 0, the
     * current Cursor gets no update. This way, "complicated" cursor movements
     * (via Find()) can be realised.
     */
    sal_uInt16 m_nCrsrMove;
    sal_uInt16 m_nBasicActionCnt;     ///< Actions which are parenthesized by Basic
    CrsrMoveState m_eMvState;     ///< Status for Crsr-Travelling - GetCrsrOfst

    OUString m_sMarkedListId;
    int m_nMarkedListLevel;

    bool m_bHasFocus : 1;         ///< Shell is "active" in a window
    bool m_bSVCrsrVis : 1;        ///< SV-Cursor visible/invisible
    bool m_bChgCallFlag : 1;      ///< attribute change inside Start- and EndAction
    bool m_bVisPortChgd : 1;      ///< in VisPortChg-Call
                                // (used in Invalidate by the Cursor)

    bool m_bCallChgLnk : 1;       ///< flag for derived classes
                                // true -> call ChgLnk
                                // access only via SwChgLinkFlag
    bool m_bAllProtect : 1;       ///< Flag for areas
                                // true -> everything protected / hidden
    bool m_bInCMvVisportChgd : 1; ///< Flag for CrsrMoves
                                // true -> view was moved
    bool m_bGCAttr : 1;           // true -> non expanded attributes exist.
    bool m_bIgnoreReadonly : 1;   // true -> make the cursor visible on next
                                // EndAction in spite of Readonly
    bool m_bSelTableCells : 1;      // true -> select cells over the InputWin
    bool m_bAutoUpdateCells : 1;  // true -> autoformat cells
    bool m_bBasicHideCrsr : 1;    // true -> HideCrsr from Basic
    bool m_bSetCrsrInReadOnly : 1;// true -> Cursor is allowed in ReadOnly-Areas
    bool m_bOverwriteCrsr : 1;    // true -> show Overwrite Crsr

    bool m_bMacroExecAllowed : 1;

    SwFrm* m_oldColFrm;

    SAL_DLLPRIVATE void UpdateCrsr(
        sal_uInt16 eFlags = SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
        bool bIdleEnd = false );

    SAL_DLLPRIVATE void MoveCrsrToNum();

    SAL_DLLPRIVATE void _ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing );

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

    SAL_DLLPRIVATE short GetTextDirection( const Point* pPt = nullptr ) const;

    SAL_DLLPRIVATE bool isInHiddenTextFrm(SwShellCrsr* pShellCrsr);

typedef bool (SwCursor:: *FNCrsr)();
    SAL_DLLPRIVATE bool CallCrsrFN( FNCrsr );

    SAL_DLLPRIVATE const SwRangeRedline* _GotoRedline( sal_uInt16 nArrPos, bool bSelect );

protected:

    inline SwMoveFnCollection* MakeFindRange( sal_uInt16, sal_uInt16, SwPaM* ) const;

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

    bool _SelTableRowOrCol( bool bRow, bool bRowSimple = false );

    bool SetInFrontOfLabel( bool bNew );

    void RefreshBlockCursor();

    /** Updates the marked list level according to the cursor.
    */
    SAL_DLLPRIVATE void UpdateMarkedListLevel();

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwCrsrShell( SwDoc& rDoc, vcl::Window *pWin, const SwViewOption *pOpt = nullptr );
    // disguised copy constructor
    SwCrsrShell( SwCrsrShell& rShell, vcl::Window *pWin );
    virtual ~SwCrsrShell();

    // create new cursor and append the old one
    virtual SwPaM & CreateNewShellCursor() override;
    virtual SwPaM & GetCurrentShellCursor() override;

    SwPaM * CreateCrsr();
    ///< delete the current cursor and make the following into the current
    bool DestroyCrsr();
    ///< transform TableCursor to normal cursor, nullify Tablemode
    void TableCrsrToCursor();
    ///< enter block mode, change normal cursor into block cursor
    void CrsrToBlockCrsr();
    ///< leave block mode, change block cursor into normal cursor
    void BlockCrsrToCrsr();

    // SelAll() selects the document body content
    // if ExtendedSelect() is called afterwards, the whole nodes array is selected
    // only for usage in special cases allowed!
    void ExtendedSelectAll(bool bFootnotes = true);
    /// If ExtendedSelectAll() was called and selection didn't change since then.
    bool ExtendedSelectedAll(bool bFootnotes = true);
    /// If document body starts with a table.
    bool StartsWithTable();

    SwPaM* GetCrsr( bool bMakeTableCrsr = true ) const;
    inline SwCursor* GetSwCrsr( bool bMakeTableCrsr = true ) const;
    // return only the current cursor
          SwShellCrsr* _GetCrsr()                       { return m_pCurCrsr; }
    const SwShellCrsr* _GetCrsr() const                 { return m_pCurCrsr; }

    // show passed cursor - for UNO
    void    SetSelection(const SwPaM& rCrsr);

    // remove all cursors from ContentNodes and set to 0
    void ParkCrsr( const SwNodeIndex &rIdx );

    // return the current cursor stack
    // (required in EditShell when deleting contents)
    inline SwPaM* GetStkCrsr() const;

    // start parenthesing, hide SV-Cursor and selected areas
    void StartAction();
    // end parenthesing, show SV-Cursor and selected areas
    void EndAction( const bool bIdleEnd = false, const bool DoSetPosX = false );

    // basic cursor travelling
    long GetUpDownX() const             { return m_nUpDownX; }

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
    bool MovePara( SwWhichPara, SwPosPara );
    bool MoveSection( SwWhichSection, SwPosSection );
    bool MoveTable( SwWhichTable, SwPosTable );
    bool MoveColumn( SwWhichColumn, SwPosColumn );
    bool MoveRegion( SwWhichRegion, SwPosRegion );

    sal_uLong Find( const css::util::SearchOptions& rSearchOpt,
                bool bSearchInNotes,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng, bool bReplace = false );

    sal_uLong Find( const SwTextFormatColl& rFormatColl,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng, const SwTextFormatColl* pReplFormat = nullptr );

    sal_uLong Find( const SfxItemSet& rSet, bool bNoCollections,
                SwDocPositions eStart, SwDocPositions eEnd,
                bool& bCancel,
                FindRanges eRng,
                const css::util::SearchOptions* pSearchOpt = nullptr,
                const SfxItemSet* rReplSet = nullptr );

    //  Position the Cursor
    //  return values:
    //      CRSR_POSCHG: when cursor was corrected from SPoint by the layout
    //      CRSR_POSOLD: when the cursor was not changed
    int SetCrsr( const Point &rPt, bool bOnlyText = false, bool bBlock = true );

    /*
     * Notification that the visible area was changed. m_aVisArea is reset, then
     * scrolling is done. The passed Rectangle lays on pixel borders to avoid
     * pixel errors.
     */
    virtual void VisPortChgd( const SwRect & ) override;

    /*
     * virtual paint method to make selection visible again after Paint
     */
    void Paint(vcl::RenderContext& rRenderContext, const Rectangle & rRect) override;

    // Areas
    inline void SetMark();
    inline bool HasMark();

    void ClearMark();

    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst true: If the point is behind the mark then
       swap the PaM. false: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(bool bPointFirst = true);

    void SwapPam();
    bool ChgCurrPam( const Point & rPt,
                     bool bTstOnly = true,      // test only, don't set
                     bool bTstHit  = false );   // only exact matches
    void KillPams();

    // story a copy of the cursor in the stack
    void Push();
    /*
     * Delete a cursor (controlled by bOldCrsr)
     *      - from stack or (bOldCrsr = true)
     *      - delete the current one and replace it with the cursor from the
     *      stack
     * Return: whether there was one left one the stack
     */
    bool Pop( bool bOldCrsr = true );
    /*
     * Combine 2 Cursors.
     * Delete the topmost from the stack and move its Mark into the current.
     */
    void Combine();

    void SttCrsrMove();
    void EndCrsrMove( const bool bIdleEnd = false );

    /*
     * When the focus is lost the selected ranges are not displayed anymore.
     * On the other hand, on receiving the focus all selected ranges are displayed again
     * (ranges must be recalculated!).
     */
    bool HasShFcs() const { return m_bHasFocus; }
    void ShLooseFcs();
    void ShGetFcs( bool bUpdate = true );

    // Methods for displaying or hiding the visible text cursor.
    void ShowCrsr();
    void HideCrsr();
    // Methods for displaying or hiding the selected ranges with visible cursor.
    void ShowCrsrs( bool bCrsrVis );
    void HideCrsrs();

    bool IsOverwriteCrsr() const { return m_bOverwriteCrsr; }
    void SetOverwriteCrsr( bool bFlag ) { m_bOverwriteCrsr = bFlag; }

    // Return current frame in which the cursor is placed.
    SwContentFrm *GetCurrFrm( const bool bCalcFrm = true ) const;

    //true if cursor is hidden because of readonly.
    //false if it is working despite readonly.
    bool IsCrsrReadonly() const;

    // Cursor is placed in something that is protected or selection contains
    // something that is protected.
    bool HasReadonlySel(bool bAnnotationMode = false) const;

    // Can the cursor be set to read only ranges?
    bool IsReadOnlyAvailable() const { return m_bSetCrsrInReadOnly; }
    void SetReadOnlyAvailable( bool bFlag );
    bool IsOverReadOnlyPos( const Point& rPt ) const;

    // Methods for aFlyMacroLnk.
    void        SetFlyMacroLnk( const Link<const SwFlyFrameFormat*,void>& rLnk ) { m_aFlyMacroLnk = rLnk; }
    const Link<const SwFlyFrameFormat*,void>& GetFlyMacroLnk() const           { return m_aFlyMacroLnk; }

    // Methods returning/altering link for changes of attributes/formates.
    void        SetChgLnk( const Link<SwCrsrShell*,void> &rLnk ) { m_aChgLnk = rLnk; }
    const Link<SwCrsrShell*,void>& GetChgLnk() const           { return m_aChgLnk; }

    // Methods returning/altering ling for "graphic completely loaded".
    void        SetGrfArrivedLnk( const Link<SwCrsrShell&,void> &rLnk ) { m_aGrfArrivedLnk = rLnk; }
    const Link<SwCrsrShell&,void>& GetGrfArrivedLnk() const           { return m_aGrfArrivedLnk; }

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
    inline bool IsSelOnePara() const;

    /*
     * Returns SRectangle, at which the cursor is located.
     */
    const SwRect &GetCharRect() const { return m_aCharRect; }
    /*
     * Returns if cursor is wholly or partly within visible range.
     */
    bool IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }
    /*
     * Returns SwRect, at which the character is located.
     */
    bool GetCharRectAt(SwRect& rRect, const SwPosition* pPos);

    // Return current page number:
    // true:  in which cursor is located.
    // false: which is visible at the upper margin.
    void GetPageNum( sal_uInt16 &rnPhyNum, sal_uInt16 &rnVirtNum,
                     bool bAtCrsrPos = true, const bool bCalcFrm = true );
    // Determine how "empty pages" are handled
    // (used in PhyPage).
    sal_uInt16 GetNextPrevPageNum( bool bNext = true );

    // Move cursor at the beginning of page "nPage".
    bool GotoPage( sal_uInt16 nPage );

    sal_uInt16 GetPageCnt();

    bool GoNextCrsr();

    bool GoPrevCrsr();

    // at CurCrsr.SPoint
    ::sw::mark::IMark* SetBookmark(
        const vcl::KeyCode&,
        const OUString& rName,
        const OUString& rShortName,
        IDocumentMarkAccess::MarkType eMark = IDocumentMarkAccess::MarkType::BOOKMARK);
    bool GotoMark( const ::sw::mark::IMark* const pMark );    // sets CurCrsr.SPoint
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
    void UpdateCrsrPos();

    // get the selected text at the current cursor. it will be filled with
    // fields etc.
    OUString GetSelText() const;
    // return only the text starting from the current cursor position (to the
    // end of the node)
    OUString GetText() const;

    // Check of SPoint or Mark of current cursor are placed within a table.
    inline const SwTableNode* IsCrsrInTable( bool bIsPtInTable = true ) const;

    inline Point& GetCrsrDocPos( bool bPoint = true ) const;
    inline bool IsCrsrPtAtEnd() const;

    inline const  SwPaM* GetTableCrs() const;
    inline        SwPaM* GetTableCrs();

    bool IsTableComplexForChart();
    // get current table selection as text
    OUString GetBoxNms() const;

    // set Crsr to the next/previous cell
    bool GoNextCell( bool bAppendLine = true );
    bool GoPrevCell();
    // go to this box (if available and inside of table)
    bool GotoTable( const OUString& rName );

    // select a table row, column or box (based on the current cursor)
    bool SelTableRow() { return _SelTableRowOrCol( true  ); }
    bool SelTableCol() { return _SelTableRowOrCol( false ); }
    bool SelTableBox();

    bool SelTable();

    bool GotoNextNum();
    bool GotoPrevNum();

    bool GotoOutline( const OUString& rName );
    // to the next/previous or the given OutlineNode
    void GotoOutline( sal_uInt16 nIdx );
    // find the "outline position" in the nodes array of the current chapter
    sal_uInt16 GetOutlinePos( sal_uInt8 nLevel = UCHAR_MAX );
    // select the given range of OutlineNodes. Optionally including the children
    // the sal_uInt16s are the positions in OutlineNodes-Array (EditShell)
    bool MakeOutlineSel( sal_uInt16 nSttPos, sal_uInt16 nEndPos,
                         bool bWithChildren = false );

    bool GotoNextOutline();
    bool GotoPrevOutline();

    /** Delivers the current shell cursor

        Some operations have to run on the current cursor ring,
        some on the m_pTableCrsr (if exist) or the current cursor ring and
        some on the m_pTableCrsr or m_pBlockCrsr or the current cursor ring.
        This small function checks the existence and delivers the wished cursor.

        @param bBlock [bool]
        if the block cursor is of interest or not

        @return m_pTableCrsr if exist,
        m_pBlockCrsr if exist and of interest (param bBlock)
        otherwise m_pCurCrsr
    */
    SwShellCrsr* getShellCrsr( bool bBlock );
    const SwShellCrsr* getShellCrsr( bool bBlock ) const
        { return (const_cast<SwCrsrShell*>(this))->getShellCrsr( bBlock ); }

    bool IsBlockMode() const { return nullptr != m_pBlockCrsr; }

    // is the Crsr in a table and is the selection over 2 columns
    bool IsTableMode() const { return nullptr != m_pTableCrsr; }

    const SwShellTableCrsr* GetTableCrsr() const { return m_pTableCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return m_pTableCrsr; }
    size_t UpdateTableSelBoxes();

    bool GotoFootnoteText();      ///< jump from content to footnote
    bool GotoFootnoteAnchor();   ///< jump from footnote to anchor
    bool GotoPrevFootnoteAnchor();
    bool GotoNextFootnoteAnchor();

    bool GotoFlyAnchor();       ///< jump from the frame to the anchor
    bool GotoHeaderText();       ///< jump from the content to the header
    bool GotoFooterText();       ///< jump from the content to the footer
    // jump to the header/footer of the given or current PageDesc
    bool SetCrsrInHdFt( size_t nDescNo = SIZE_MAX,
                        bool bInHeader = true );
    // is point of cursor in header/footer. pbInHeader return true if it is
    // in a headerframe otherwise in a footerframe
    bool IsInHeaderFooter( bool* pbInHeader = nullptr ) const;

    bool GotoNextTOXBase( const OUString* = nullptr );
    bool GotoPrevTOXBase( const OUString* = nullptr );
    bool GotoTOXMarkBase();
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
    bool SelectNxtPrvHyperlink( bool bNext = true );

    bool GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType = 0,
                            sal_uInt16 nSeqNo = 0 );

    // get the nth character from the start or end of the  current selection
    sal_Unicode GetChar( bool bEnd = true, long nOffset = 0 );
    bool ExtendSelection( bool bEnd = true, sal_Int32 nCount = 1 );

    // Place only the visible cursor at the given position in the document.
    // Return false if SPoint was corrected by layout.
    // (This is needed for displaying the Drag&Drop/Copy-Cursor.)
    bool SetVisCrsr( const Point &rPt );
    inline void UnSetVisCrsr();

    // jump to the next or previous field of the corresponding type
    bool MoveFieldType(
        const SwFieldType* pFieldType,
        const bool bNext,
        const sal_uInt16 nResType = USHRT_MAX,
        const bool bAddSetExpressionFieldsToInputFields = true );

    bool GotoFormatField( const SwFormatField& rField );

    static SwTextField* GetTextFieldAtPos(
        const SwPosition* pPos,
        const bool bIncludeInputFieldAtStart );
    static SwField* GetFieldAtCrsr(
        const SwPaM* pCrsr,
        const bool bIncludeInputFieldAtStart );
    SwField* GetCurField( const bool bIncludeInputFieldAtStart = false ) const;
    bool CrsrInsideInputField() const;
    static bool PosInsideInputField( const SwPosition& rPos );
    bool DocPtInsideInputField( const Point& rDocPt ) const;
    static sal_Int32 StartOfInputFieldAtPos( const SwPosition& rPos );
    static sal_Int32 EndOfInputFieldAtPos( const SwPosition& rPos );

    // Return number of cursors in ring (The flag indicates whether
    // only cursors containing selections are requested).
    sal_uInt16 GetCrsrCnt( bool bAll = true ) const;

    // Char Travelling - methods (in crstrvl1.cxx)
    bool GoStartWord();
    bool GoEndWord();
    bool GoNextWord();
    bool GoPrevWord();
    bool GoNextSentence();
    bool GoStartSentence();
    bool GoEndSentence();
    bool SelectWord( const Point* pPt = nullptr );
    bool ExpandToSentenceBorders();

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
    bool IsAtRightMargin(bool bAPI = false) const   { return IsAtLRMargin( false, bAPI ); }

    // delete all created cursors, set the table cursor and the last cursor to
    // its TextNode (or StartNode?)
    // They all get created on the next ::GetCrsr again
    // Used for Drag&Drop/Clipboard-Paste in tables
    bool ParkTableCrsr();

    // Non expanded attributes?
    bool IsGCAttr() const { return m_bGCAttr; }
    void ClearGCAttr() { m_bGCAttr = false; }
    void    UpdateAttr() {  m_bGCAttr = true; }

    // is the whole document protected/hidden (for UI...)
    bool IsAllProtect() const { return m_bAllProtect; }

    bool BasicActionPend() const    { return m_nBasicActionCnt != mnStartAction; }

    bool GotoRegion( const OUString& rName );

    // show the current selection
    virtual void MakeSelVisible();

    // set the cursor to a NOT protected/hidden node
    bool FindValidContentNode( bool bOnlyText = false );

    bool GetContentAtPos( const Point& rPt,
                          SwContentAtPos& rContentAtPos,
                          bool bSetCrsr = false,
                          SwRect* pFieldRect = nullptr );

    const SwPostItField* GetPostItFieldAtCursor() const;

    // get smart tags at point position
    void GetSmartTagTerm( const Point& rPt,
                          SwRect& rSelectRect,
                          css::uno::Sequence< OUString >& rSmartTagTypes,
                          css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                          css::uno::Reference<css::text::XTextRange>& rRange );

    // get smart tags at current cursor position
    void GetSmartTagTerm( css::uno::Sequence< OUString >& rSmartTagTypes,
                          css::uno::Sequence< css::uno::Reference< css::container::XStringKeyMap > >& rStringKeyMaps,
                          css::uno::Reference<css::text::XTextRange>& rRange ) const;

    bool IsPageAtPos( const Point &rPt ) const;

    bool SelectTextAttr( sal_uInt16 nWhich, bool bExpand, const SwTextAttr* pAttr = nullptr );
    bool GotoINetAttr( const SwTextINetFormat& rAttr );
    const SwFormatINetFormat* FindINetAttr( const OUString& rName ) const;

    bool SelectText( const sal_Int32 nStart,
                        const sal_Int32 nEnd );

    bool CheckTableBoxContent( const SwPosition* pPos = nullptr );
    void SaveTableBoxContent( const SwPosition* pPos = nullptr );
    void ClearTableBoxContent();
    bool EndAllTableBoxEdit();

    void SetSelTableCells( bool bFlag )           { m_bSelTableCells = bFlag; }
    bool IsSelTableCells() const                  { return m_bSelTableCells; }

    bool IsAutoUpdateCells() const              { return m_bAutoUpdateCells; }
    void SetAutoUpdateCells( bool bFlag )       { m_bAutoUpdateCells = bFlag; }

    bool GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                            SwRect& rRect, short& rOrient );
    bool SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode );

    const SwRangeRedline* SelNextRedline();
    const SwRangeRedline* SelPrevRedline();
    const SwRangeRedline* GotoRedline( sal_uInt16 nArrPos, bool bSelect = false );

    // is cursor or the point in/over a vertical formatted text?
    bool IsInVerticalText( const Point* pPt = nullptr ) const;
    // is cursor or the point in/over a right to left formatted text?
    bool IsInRightToLeftText( const Point* pPt = nullptr ) const;

    static void FirePageChangeEvent(sal_uInt16 nOldPage, sal_uInt16 nNewPage);
    bool   bColumnChange();
    static void FireSectionChangeEvent(sal_uInt16 nOldSection, sal_uInt16 nNewSection);
    static void FireColumnChangeEvent(sal_uInt16 nOldColumn, sal_uInt16 nNewColumn);
    // If the current cursor position is inside a hidden range, the hidden range
    // is selected and true is returned:
    bool SelectHiddenRange();

    // remove all invalid cursors
    void ClearUpCrsrs();

    inline void SetMacroExecAllowed( const bool _bMacroExecAllowed )
    {
        m_bMacroExecAllowed = _bMacroExecAllowed;
    }
    inline bool IsMacroExecAllowed()
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
    OUString GetCrsrDescr() const;

    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
    /// Implementation of lok::Document::getPartPageRectangles() for Writer.
    OUString getPageRectangles();
};

// Cursor Inlines:
inline SwMoveFnCollection* SwCrsrShell::MakeFindRange(
            sal_uInt16 nStt, sal_uInt16 nEnd, SwPaM* pPam ) const
{
    return m_pCurCrsr->MakeFindRange( (SwDocPositions)nStt, (SwDocPositions)nEnd, pPam );
}

inline SwCursor* SwCrsrShell::GetSwCrsr( bool bMakeTableCrsr ) const
{
    return static_cast<SwCursor*>(GetCrsr( bMakeTableCrsr ));
}

inline SwPaM* SwCrsrShell::GetStkCrsr() const { return m_pCrsrStk; }

inline void SwCrsrShell::SetMark() { m_pCurCrsr->SetMark(); }

inline bool SwCrsrShell::HasMark() { return( m_pCurCrsr->HasMark() ); }

inline bool SwCrsrShell::IsSelection() const
{
    return IsTableMode() || m_pCurCrsr->HasMark() ||
            m_pCurCrsr->GetNext() != m_pCurCrsr;
}
inline bool SwCrsrShell::IsMultiSelection() const
{
    return m_pCurCrsr->GetNext() != m_pCurCrsr;
}

inline bool SwCrsrShell::IsSelOnePara() const
{
    return !m_pCurCrsr->IsMultiSelection() &&
           m_pCurCrsr->GetPoint()->nNode == m_pCurCrsr->GetMark()->nNode;
}

inline const SwTableNode* SwCrsrShell::IsCrsrInTable( bool bIsPtInTable ) const
{
    return m_pCurCrsr->GetNode( bIsPtInTable ).FindTableNode();
}

inline bool SwCrsrShell::IsCrsrPtAtEnd() const
{
    return m_pCurCrsr->End() == m_pCurCrsr->GetPoint();
}

inline Point& SwCrsrShell::GetCrsrDocPos( bool bPoint ) const
{
    return bPoint ? m_pCurCrsr->GetPtPos() : m_pCurCrsr->GetMkPos();
}

inline const SwPaM* SwCrsrShell::GetTableCrs() const
{
    return m_pTableCrsr;
}

inline SwPaM* SwCrsrShell::GetTableCrs()
{
    return m_pTableCrsr;
}

inline void SwCrsrShell::UnSetVisCrsr()
{
    m_pVisCrsr->Hide();
    m_pVisCrsr->SetDragCrsr( false );
}

#endif  // _CRSRSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
