/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <swtypes.hxx>          // for SWPOSDOC
#include <viewsh.hxx>           // for ViewShell
#include <calbck.hxx>           // for SwClient
#include <cshtyp.hxx>           // for CursorShell types
#include <crstate.hxx>          // for CursorMove-States
#include <toxe.hxx>             // SwTOXSearchDir
#include <tblsel.hxx>           // SwTblSearchType
#include <viscrs.hxx>
#include <node.hxx>
#include <tblsel.hxx>
#include <IDocumentMarkAccess.hxx>

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
class SwCntntNode;

class SwPostItField;

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

// enum and struct to get informations via the Doc-Position

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
#if OSL_DEBUG_LEVEL > 1
        ,SW_CURR_ATTRS      = 0x4000        // only for debugging
        ,SW_TABLEBOXVALUE   = 0x8000        // only for debugging
#endif
    } eCntntAtPos;

    union {
        const SwField* pFld;
        const SfxPoolItem* pAttr;
        const SwRedline* pRedl;
        SwCntntNode * pNode;
        const sw::mark::IFieldmark* pFldmark;
    } aFnd;

    int nDist;

    String sStr;
    const SwTxtAttr* pFndTxtAttr;

    SwContentAtPos( int eGetAtPos = 0xffff )
        : eCntntAtPos( (IsAttrAtPos)eGetAtPos )
    {
        aFnd.pFld = 0;
        pFndTxtAttr = 0;
        nDist = 0; // #i23726#
    }

    BOOL IsInProtectSect() const;
    bool     IsInRTLText()const;
};

// return values of SetCrsr (can be combined via ||)
const int CRSR_POSOLD = 0x01,   // cursor stays at old position
          CRSR_POSCHG = 0x02;   // position changed by the layout

// Helperfunction to resolve backward references in regular expressions

String *ReplaceBackReferences( const com::sun::star::util::SearchOptions& rSearchOpt, SwPaM* pPam );

class SW_DLLPUBLIC SwCrsrShell : public ViewShell, public SwModify
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;
    friend class SwChgLinkFlag;

    // requires the Crsr as InternalCrsr
    friend BOOL GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:

    /* for calling UpdateCrsr */
    enum CrsrFlag {
        UPDOWN      = (1 << 0),     // keep Up/Down on columns
        SCROLLWIN   = (1 << 1),     // scroll window
        CHKRANGE    = (1 << 2),     // check overlapping PaMs
        NOCALRECT   = (1 << 3),     // don't recalculate CharRect
        READONLY    = (1 << 4)      // make visible in spite of Readonly
    };

private:

    SwRect  aCharRect;          // Char-SRectangle on which the cursor is located
    Point   aCrsrHeight;        // height & offset from visible Cursor
    Point   aOldRBPos;          // Right/Bottom of last VisArea
                                // (used in Invalidate by Cursor)

    Link aFlyMacroLnk;          // Link will be called, if the Crsr is set
                                // into a fly. A macro can be then becalled
    Link aChgLnk;               // link will be called by every attribut/
                                // format changes at cursor position.
    Link aGrfArrivedLnk;        // Link calls to UI if a graphic is arrived

    SwShellCrsr* pCurCrsr;      // current cursor
    SwShellCrsr* pCrsrStk;      // stack for the cursor
    SwVisCrsr *pVisCrsr;        // the visible cursor

    IBlockCursor *pBlockCrsr;   // interface of cursor for block (=rectangular) selection

    SwShellTableCrsr* pTblCrsr; // table Crsr; only in tables when the
                                // selection lays over 2 columns

    SwNodeIndex* pBoxIdx;       // for recognizing of the changed
    SwTableBox* pBoxPtr;        // table row

    long nUpDownX;              // try to move the cursor on up/down always
                                // in the same column
    long nLeftFrmPos;
    ULONG nAktNode;             // save CursorPos at Start-Action
    xub_StrLen nAktCntnt;
    USHORT nAktNdTyp;
    bool bAktSelection;

    /*
     * Via the Method SttCrsrMove and EndCrsrMove this counter gets
     * incremented/decremented. As long as the counter is inequal to 0, the
     * current Cursor gets no update. This way, "complicated" cursor movements
     * (via Find()) can be realised.
     */
    USHORT nCrsrMove;
    USHORT nBasicActionCnt;     // Actions which are parenthesized by Basic
    CrsrMoveState eMvState;     // Status for Crsr-Travelling - GetCrsrOfst

    String sMarkedListId;
    int nMarkedListLevel;

    BOOL bHasFocus : 1;         // Shell is "active" in a window
    BOOL bSVCrsrVis : 1;        // SV-Cursor visible/invisible
    BOOL bChgCallFlag : 1;      // attribute change inside Start- and EndAction
    BOOL bVisPortChgd : 1;      // in VisPortChg-Call
                                // (used in Invalidate by the Cursor)

    BOOL bCallChgLnk : 1;       // flag for derived classes
                                // TRUE -> call ChgLnk
                                // access only via SwChgLinkFlag
    BOOL bAllProtect : 1;       // Flag for areas
                                // TRUE -> everything protected / hidden
    BOOL bInCMvVisportChgd : 1; // Flag for CrsrMoves
                                // TRUE -> view was moved
    BOOL bGCAttr : 1;           // TRUE -> non expanded attributes exist.
    BOOL bIgnoreReadonly : 1;   // TRUE -> make the cursor visible on next
                                // EndAction in spite of Readonly
    BOOL bSelTblCells : 1;      // TRUE -> select cells over the InputWin
    BOOL bAutoUpdateCells : 1;  // TRUE -> autoformat cells
    BOOL bBasicHideCrsr : 1;    // TRUE -> HideCrsr from Basic
    BOOL bSetCrsrInReadOnly : 1;// TRUE -> Cursor is allowed in ReadOnly-Areas
    BOOL bOverwriteCrsr : 1;    // TRUE -> show Overwrite Crsr

    bool mbMacroExecAllowed : 1;

    SW_DLLPRIVATE void UpdateCrsr( USHORT eFlags
                            =SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
                     BOOL bIdleEnd = FALSE );

    SW_DLLPRIVATE void _ParkPams( SwPaM* pDelRg, SwShellCrsr** ppDelRing );

    /** Mark a certain list level of a certain list

        levels of a certain lists are marked now

        @param sListId    list Id of the list whose level is marked
        @param nLevel     to be marked list level

        An empty sListId denotes that no level of a list is marked.
     */
    SW_DLLPRIVATE void MarkListLevel( const String& sListId,
                                      const int nLevel );

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

    // set all PaMs in OldNode to NewPos + Offset
    void PaMCorrAbs(const SwNodeIndex &rOldNode, const SwPosition &rNewPos,
                    const xub_StrLen nOffset = 0 );

    BOOL _SelTblRowOrCol( bool bRow, bool bRowSimple = false );

    bool SetInFrontOfLabel( BOOL bNew );

    void RefreshBlockCursor();

    /** Updates the marked list level according to the cursor.
    */
    SW_DLLPRIVATE void UpdateMarkedListLevel();

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin, const SwViewOption *pOpt = 0 );
    // disguised copy constructor
    SwCrsrShell( SwCrsrShell& rShell, Window *pWin );
    virtual ~SwCrsrShell();

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    // create new cursor and append the old one
    SwPaM * CreateCrsr();
    // delete the current cursor and make the following into the current
    BOOL DestroyCrsr();
    // transform TableCursor to normal cursor, nullify Tablemode
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
    // return only the current cursor
          SwShellCrsr* _GetCrsr()                       { return pCurCrsr; }
    const SwShellCrsr* _GetCrsr() const                 { return pCurCrsr; }

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
    void EndAction( const BOOL bIdleEnd = FALSE );

    // basic cursor travelling
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

    //  Position the Cursor
    //  return values:
    //      CRSR_POSCHG: when cursor was corrected from SPoint by the layout
    //      CRSR_POSOLD: when the cursor was not changed
    int SetCrsr( const Point &rPt, BOOL bOnlyText = FALSE, bool bBlock = true );

    /*
     * Notification that the visible area was changed. aVisArea is reset, then
     * scrolling is done. The passed Rectangle lays on pixel borders to avoid
     * pixel errors.
     */
    virtual void VisPortChgd( const SwRect & );

    /*
     * virtual paint method to make selection visible again after Paint
     */
    void Paint( const Rectangle & rRect );

    // Areas
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
                     BOOL bTstOnly = TRUE,      // test only, don't set
                     BOOL bTstHit  = FALSE );   // only exact matches
    void KillPams();

    // story a copy of the cursor in the stack
    void Push();
    /*
     * Delete a cursor (controlled by bOldCrsr)
     *      - from stack or (bOldCrsr = TRUE)
     *      - delete the current one and replace it with the cursor from the
     *      stack
     * Return: whether there was one left one the stack
     */
    BOOL Pop( BOOL bOldCrsr = TRUE );
    /*
     * Combine 2 Cursors.
     * Delete the topmost from the stack and move its Mark into the current.
     */
    void Combine();

#if OSL_DEBUG_LEVEL > 1
    void SttCrsrMove();
    void EndCrsrMove( const BOOL bIdleEnd = FALSE );
#else
    void SttCrsrMove() { ++nCrsrMove; StartAction(); }
    void EndCrsrMove( const BOOL bIdleEnd = FALSE )
            { EndAction( bIdleEnd ); --nCrsrMove; }
#endif

    /*
     * When the focus is lost the selected ranges are not displayed anymore.
     * On the other hand, on receiving the focus all selected ranges are displayed again
     * (ranges must be recalculated!).
     */
    BOOL HasShFcs() const { return bHasFocus; }
    void ShLooseFcs();
    void ShGetFcs( BOOL bUpdate = TRUE );

    // Methods for displaying or hiding the visible text cursor.
    void ShowCrsr();
    void HideCrsr();
    // Methods for displaying or hiding the selected ranges with visible cursor.
    void ShowCrsrs( BOOL bCrsrVis );
    void HideCrsrs();

    BOOL IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( BOOL bFlag ) { bOverwriteCrsr = bFlag; }

    // Return current frame in which the cursor is placed.
    SwCntntFrm *GetCurrFrm( const BOOL bCalcFrm = TRUE ) const;

    //TRUE if cursor is hidden because of readonly.
    //FALSE if it is working despite readonly.
    BOOL IsCrsrReadonly() const;

    // Cursor is placed in something that is protected or selection contains
    // something that is protected.
    BOOL HasReadonlySel() const;

    // Can the cursor be set to read only ranges?
    BOOL IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }
    void SetReadOnlyAvailable( BOOL bFlag );
    BOOL IsOverReadOnlyPos( const Point& rPt ) const;

    // Methods for aFlyMacroLnk.
    void        SetFlyMacroLnk( const Link& rLnk ) { aFlyMacroLnk = rLnk; }
    const Link& GetFlyMacroLnk() const             { return aFlyMacroLnk; }

    // Methods returning/altering link for changes of attributes/formates.
    void        SetChgLnk( const Link &rLnk ) { aChgLnk = rLnk; }
    const Link& GetChgLnk() const             { return aChgLnk; }

    // Methods returning/altering ling for "graphic completely loaded".
    void        SetGrfArrivedLnk( const Link &rLnk ) { aGrfArrivedLnk = rLnk; }
    const Link& GetGrfArrivedLnk() const             { return aGrfArrivedLnk; }

    //Call ChgLink. When within an action calling will be delayed.
    void CallChgLnk();

    // Check if the current cursor contains a selection, i.e.
    // if Mark is set and SPoint and Mark are different.
    BOOL HasSelection() const;

    // Check if a selection exists, i.e. if the current cursor comprises a selection.
    inline BOOL IsSelection() const;
    // returns if multiple cursors are available
    inline BOOL IsMultiSelection() const;

    // Check if a complete paragraph was selected.
    BOOL IsSelFullPara() const;

    // Check if selection is within one paragraph.
    inline BOOL IsSelOnePara() const;

    //Should WaitPtr be activated for Clipboard.
    BOOL ShouldWait() const;

    /*
     * Returns SRectangle, at which the cursor is located.
     */
    const SwRect &GetCharRect() const { return aCharRect; }
    /*
     * Returns if cursor is wholly or partly within visible range.
     */
    BOOL IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }

    // Return current page number:
    // TRUE:  in which cursor is located.
    // FALSE: which is visible at the upper margin.
    void GetPageNum( USHORT &rnPhyNum, USHORT &rnVirtNum,
                     BOOL bAtCrsrPos = TRUE, const BOOL bCalcFrm = TRUE );
    // Determine how "empty pages" are handled
    // (used in PhyPage).
    USHORT GetNextPrevPageNum( BOOL bNext = TRUE );

    // Move cursor at the bginning of page "nPage".
    BOOL GotoPage( USHORT nPage );

    USHORT GetPageCnt();

    BOOL GoNextCrsr();

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

    // update Cursr, i.e. reset it into content should only be called when the
    // cursor was set to a random position e.g. when deleting frames
    void UpdateCrsrPos();

    // get the selected text at the current cursor. it will be filled with
    // fields etc.
    String GetSelTxt() const;
    // return only the text starting from the current cursor position (to the
    // end of the node)
    String GetText() const;

    // Check of SPoint or Mark of current cursor are placed within a table.
    inline const SwTableNode* IsCrsrInTbl( BOOL bIsPtInTbl = TRUE ) const;

    inline Point& GetCrsrDocPos( BOOL bPoint = TRUE ) const;
    inline BOOL IsCrsrPtAtEnd() const;

    inline const  SwPaM* GetTblCrs() const;
    inline        SwPaM* GetTblCrs();

    BOOL IsTblComplex() const;
    BOOL IsTblComplexForChart();
    // get current table selection as text
    String GetBoxNms() const;

    // set Crsr to the next/previous cell
    BOOL GoNextCell( BOOL bAppendLine = TRUE );
    BOOL GoPrevCell();
    // go to this box (if available and inside of table)
    BOOL GotoTable( const String& rName );

    // select a table row, column or box (based on the current cursor)
    BOOL SelTblRow() { return _SelTblRowOrCol( true  ); }
    BOOL SelTblCol() { return _SelTblRowOrCol( false ); }
    BOOL SelTblBox();

    BOOL SelTbl();

    BOOL GotoNextNum();
    BOOL GotoPrevNum();

    BOOL GotoOutline( const String& rName );
    // to the next/previous or the given OutlineNode
    void GotoOutline( USHORT nIdx );
    // find the "outline position" in the nodes array of the current chapter
    USHORT GetOutlinePos( BYTE nLevel = UCHAR_MAX );
    // select the given range of OutlineNodes. Optionally including the children
    // the USHORTs are the positions in OutlineNodes-Array (EditShell)
    BOOL MakeOutlineSel( USHORT nSttPos, USHORT nEndPos,
                        BOOL bWithChilds = FALSE );

    BOOL GotoNextOutline();
    BOOL GotoPrevOutline();

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

    bool IsBlockMode() const { return 0 != pBlockCrsr; }
    const IBlockCursor* GetBlockCrsr() const { return pBlockCrsr; }
    IBlockCursor* GetBlockCrsr() { return pBlockCrsr; }

    // is the Crsr in a table and is the selection over 2 columns
    BOOL IsTableMode() const { return 0 != pTblCrsr; }

    const SwShellTableCrsr* GetTableCrsr() const { return pTblCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return pTblCrsr; }
    USHORT UpdateTblSelBoxes();

    BOOL GotoFtnTxt();      // jump from content to footnote
    BOOL GotoFtnAnchor();   // jump from footnote to anchor
    BOOL GotoPrevFtnAnchor();
    BOOL GotoNextFtnAnchor();

    BOOL GotoFlyAnchor();       // jump from the frame to the anchor
    BOOL GotoHeaderTxt();       // jump from the content to the header
    BOOL GotoFooterTxt();       // jump from the content to the footer
    // jump to the header/footer of the given or current PageDesc
    BOOL SetCrsrInHdFt( USHORT nDescNo = USHRT_MAX,
                            BOOL bInHeader = TRUE );
    // is point of cursor in header/footer. pbInHeader return TRUE if it is
    // in a headerframe otherwise in a footerframe
    BOOL IsInHeaderFooter( BOOL* pbInHeader = 0 ) const;

    BOOL GotoNextTOXBase( const String* = 0 );
    BOOL GotoPrevTOXBase( const String* = 0 );
    BOOL GotoTOXMarkBase();
    // jump to the next or previous index entry
    BOOL GotoNxtPrvTOXMark( BOOL bNext = TRUE );
    // jump to the next/previous index mark of this type
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rStart, SwTOXSearch eDir );

    // jump to the next or previous table formula
    // optionally only to broken formulas
    BOOL GotoNxtPrvTblFormula( BOOL bNext = TRUE,
                                    BOOL bOnlyErrors = FALSE );
    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    BOOL SelectNxtPrvHyperlink( BOOL bNext = TRUE );

    BOOL GotoRefMark( const String& rRefMark, USHORT nSubType = 0,
                            USHORT nSeqNo = 0 );

    // get the nth character from the start or end of the  current selection
    sal_Unicode GetChar( BOOL bEnd = TRUE, long nOffset = 0 );
    BOOL ExtendSelection( BOOL bEnd = TRUE, xub_StrLen nCount = 1 );

    // Place only the visible cursor at the given position in the document.
    // Return FALSE if SPoint was corrected by layout.
    // (This is needed for displaying the Drag&Drop/Copy-Cursor.)
    BOOL SetVisCrsr( const Point &rPt );
    inline void UnSetVisCrsr();

    // jump to the next or previous field of the corresponding type
    BOOL MoveFldType( const SwFieldType* pFldType, BOOL bNext,
                                            USHORT nSubType = USHRT_MAX,
                                            USHORT nResType = USHRT_MAX );
    BOOL GotoFld( const SwFmtFld& rFld );

    // Return number of cursors in ring (The flag indicates whether
    // only cursors containing selections are requested).
    USHORT GetCrsrCnt( BOOL bAll = TRUE ) const;

    // Char Travelling - methods (in crstrvl1.cxx)
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

    // get position from current cursor
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

    // delete all created cursors, set the table cursor and the last cursor to
    // its TextNode (or StartNode?)
    // They all get created on the next ::GetCrsr again
    // Used for Drag&Drop/Clipboard-Paste in tables
    BOOL ParkTblCrsr();

    // Non expanded attributes?
    BOOL IsGCAttr() const { return bGCAttr; }
    void ClearGCAttr() { bGCAttr = FALSE; }
    void    UpdateAttr() {  bGCAttr = TRUE; }

    // is the whole document protected/hidden (for UI...)
    BOOL IsAllProtect() const { return bAllProtect; }

#ifdef SW_CRSR_TIMER
    // Set flag at VisCrsr. Is it triggered by Timer (TRUE) or directly (FALSE).
    // Default is triggert by Timer.
    BOOL ChgCrsrTimerFlag( BOOL bTimerOn = TRUE );
#endif

    BOOL BasicActionPend() const    { return nBasicActionCnt != nStartAction; }

    BOOL GotoRegion( const String& rName );

    // show the current selection
    virtual void MakeSelVisible();

    // set the cursor to a NOT protected/hidden node
    BOOL FindValidCntntNode( BOOL bOnlyText = FALSE );

    BOOL GetContentAtPos( const Point& rPt,
                            SwContentAtPos& rCntntAtPos,
                            BOOL bSetCrsr = FALSE,
                            SwRect* pFldRect = 0 );

    const SwPostItField* GetPostItFieldAtCursor() const;

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

    BOOL SelectTxtAttr( USHORT nWhich, BOOL bExpand, const SwTxtAttr* pAttr = 0 );
    BOOL GotoINetAttr( const SwTxtINetFmt& rAttr );
    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    BOOL CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void SaveTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();
    BOOL EndAllTblBoxEdit();

    // gets called when a table selection gets created in UpdateCrsr without
    // the UI knowing of it
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

    inline void SetMacroExecAllowed( const bool _bMacroExecAllowed )
    {
        mbMacroExecAllowed = _bMacroExecAllowed;
    }
    inline bool IsMacroExecAllowed()
    {
        return mbMacroExecAllowed;
    }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
