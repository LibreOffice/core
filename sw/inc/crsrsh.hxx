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
#ifndef SW_CRSRSH_HXX
#define SW_CRSRSH_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/rtti.hxx>

#include <IShellCursorSupplier.hxx>
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
#ifdef DBG_UTIL
        ,SW_CURR_ATTRS      = 0x4000        ///< only for debugging
        ,SW_TABLEBOXVALUE   = 0x8000        ///< only for debugging
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

    bool IsInProtectSect() const;
    bool     IsInRTLText()const;
};

// return values of SetCrsr (can be combined via ||)
const int CRSR_POSOLD = 0x01,   // cursor stays at old position
          CRSR_POSCHG = 0x02;   // position changed by the layout

/// Helperfunction to resolve backward references in regular expressions
String *ReplaceBackReferences( const com::sun::star::util::SearchOptions& rSearchOpt, SwPaM* pPam );

class SW_DLLPUBLIC SwCrsrShell
    : public ViewShell
    , public SwModify
    , public ::sw::IShellCursorSupplier
{
    friend class SwCallLink;
    friend class SwVisCrsr;
    friend class SwSelPaintRects;

    // requires the Crsr as InternalCrsr
    friend sal_Bool GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

public:

    /** for calling UpdateCrsr */
    enum CrsrFlag {
        UPDOWN      = (1 << 0),     ///< keep Up/Down on columns
        SCROLLWIN   = (1 << 1),     ///< scroll window
        CHKRANGE    = (1 << 2),     ///< check overlapping PaMs
        NOCALRECT   = (1 << 3),     ///< don't recalculate CharRect
        READONLY    = (1 << 4)      ///< make visible in spite of Readonly
    };

private:

    SwRect  aCharRect;          ///< Char-SRectangle on which the cursor is located
    Point   aCrsrHeight;        ///< height & offset from visible Cursor
    Point   aOldRBPos;          ///< Right/Bottom of last VisArea
                                // (used in Invalidate by Cursor)

    Link aFlyMacroLnk;          /**< Link will be called, if the Crsr is set
                                   into a fly. A macro can be then becalled */
    Link aChgLnk;               /**< link will be called by every attribut/
                                   format changes at cursor position.*/
    Link aGrfArrivedLnk;        ///< Link calls to UI if a graphic is arrived

    SwShellCrsr* pCurCrsr;      ///< current cursor
    SwShellCrsr* pCrsrStk;      ///< stack for the cursor
    SwVisCrsr *pVisCrsr;        ///< the visible cursor

    IBlockCursor *pBlockCrsr;   ///< interface of cursor for block (=rectangular) selection

    SwShellTableCrsr* pTblCrsr; /**< table Crsr; only in tables when the
                                   selection lays over 2 columns */

    SwNodeIndex* pBoxIdx;       ///< for recognizing of the changed
    SwTableBox* pBoxPtr;        ///< table row

    long nUpDownX;              /**< try to move the cursor on up/down always
                                   in the same column */
    long nLeftFrmPos;
    sal_uLong nAktNode;             // save CursorPos at Start-Action
    xub_StrLen nAktCntnt;
    sal_uInt16 nAktNdTyp;
    bool bAktSelection;

    /*
     * Via the Method SttCrsrMove and EndCrsrMove this counter gets
     * incremented/decremented. As long as the counter is inequal to 0, the
     * current Cursor gets no update. This way, "complicated" cursor movements
     * (via Find()) can be realised.
     */
    sal_uInt16 nCrsrMove;
    sal_uInt16 nBasicActionCnt;     ///< Actions which are parenthesized by Basic
    CrsrMoveState eMvState;     ///< Status for Crsr-Travelling - GetCrsrOfst

    String sMarkedListId;
    int nMarkedListLevel;

    sal_Bool bHasFocus : 1;         ///< Shell is "active" in a window
    sal_Bool bSVCrsrVis : 1;        ///< SV-Cursor visible/invisible
    sal_Bool bChgCallFlag : 1;      ///< attribute change inside Start- and EndAction
    sal_Bool bVisPortChgd : 1;      ///< in VisPortChg-Call
                                // (used in Invalidate by the Cursor)

    sal_Bool bCallChgLnk : 1;       ///< flag for derived classes
                                // TRUE -> call ChgLnk
                                // access only via SwChgLinkFlag
    sal_Bool bAllProtect : 1;       ///< Flag for areas
                                // TRUE -> everything protected / hidden
    sal_Bool bInCMvVisportChgd : 1; ///< Flag for CrsrMoves
                                // TRUE -> view was moved
    sal_Bool bGCAttr : 1;           // TRUE -> non expanded attributes exist.
    sal_Bool bIgnoreReadonly : 1;   // TRUE -> make the cursor visible on next
                                // EndAction in spite of Readonly
    sal_Bool bSelTblCells : 1;      // TRUE -> select cells over the InputWin
    sal_Bool bAutoUpdateCells : 1;  // TRUE -> autoformat cells
    sal_Bool bBasicHideCrsr : 1;    // TRUE -> HideCrsr from Basic
    sal_Bool bSetCrsrInReadOnly : 1;// TRUE -> Cursor is allowed in ReadOnly-Areas
    sal_Bool bOverwriteCrsr : 1;    // sal_True -> show Overwrite Crsr

    bool mbMacroExecAllowed : 1;

    SW_DLLPRIVATE void UpdateCrsr( sal_uInt16 eFlags
                            =SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE,
                     sal_Bool bIdleEnd = sal_False );

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
                  sal_Bool LeftRight( sal_Bool, sal_uInt16, sal_uInt16, sal_Bool );
    SW_DLLPRIVATE sal_Bool UpDown( sal_Bool, sal_uInt16 );
    SW_DLLPRIVATE sal_Bool LRMargin( sal_Bool, sal_Bool bAPI = sal_False );
    SW_DLLPRIVATE sal_Bool IsAtLRMargin( sal_Bool, sal_Bool bAPI = sal_False ) const;

    SW_DLLPRIVATE short GetTextDirection( const Point* pPt = 0 ) const;

typedef sal_Bool (SwCursor:: *FNCrsr)();
    SW_DLLPRIVATE sal_Bool CallCrsrFN( FNCrsr );

    SW_DLLPRIVATE const SwRedline* _GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect );

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

    // set all PaMs in OldNode to NewPos + Offset
    void PaMCorrAbs(const SwNodeIndex &rOldNode, const SwPosition &rNewPos,
                    const xub_StrLen nOffset = 0 );

    bool _SelTblRowOrCol( bool bRow, bool bRowSimple = false );

    bool SetInFrontOfLabel( bool bNew );

    void RefreshBlockCursor();

    /** Updates the marked list level according to the cursor.
    */
    SW_DLLPRIVATE void UpdateMarkedListLevel();

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    TYPEINFO();
    SwCrsrShell( SwDoc& rDoc, Window *pWin, const SwViewOption *pOpt = 0 );
    // disguised copy constructor
    SwCrsrShell( SwCrsrShell& rShell, Window *pWin );
    virtual ~SwCrsrShell();

    // create new cursor and append the old one
    virtual SwPaM & CreateNewShellCursor();
    virtual SwPaM & GetCurrentShellCursor();

    SwPaM * CreateCrsr();
    ///< delete the current cursor and make the following into the current
    bool DestroyCrsr();
    ///< transform TableCursor to normal cursor, nullify Tablemode
    void TblCrsrToCursor();
    ///< enter block mode, change normal cursor into block cursor
    void CrsrToBlockCrsr();
    ///< leave block mode, change block cursor into normal cursor
    void BlockCrsrToCrsr();

    // SelAll() selects the document body content
    // if ExtendedSelect() is called afterwards, the whole nodes array is selected
    // only for usage in special cases allowed!
    void ExtendedSelectAll();

    SwPaM* GetCrsr( sal_Bool bMakeTblCrsr = sal_True ) const;
    inline SwCursor* GetSwCrsr( sal_Bool bMakeTblCrsr = sal_True ) const;
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
    void EndAction( const sal_Bool bIdleEnd = sal_False );

    // basic cursor travelling
    long GetUpDownX() const             { return nUpDownX; }

    sal_Bool Left( sal_uInt16 nCnt, sal_uInt16 nMode, sal_Bool bAllowVisual = sal_False )
        { return LeftRight( sal_True, nCnt, nMode, bAllowVisual ); }
    sal_Bool Right( sal_uInt16 nCnt, sal_uInt16 nMode, sal_Bool bAllowVisual = sal_False )
        { return LeftRight( sal_False, nCnt, nMode, bAllowVisual ); }
    sal_Bool Up( sal_uInt16 nCnt = 1 )      { return UpDown( sal_True, nCnt ); }
    sal_Bool Down( sal_uInt16 nCnt = 1 )    { return UpDown( sal_False, nCnt ); }
    sal_Bool LeftMargin()               { return LRMargin( sal_True ); }
    sal_Bool RightMargin(sal_Bool bAPI = sal_False) { return LRMargin( sal_False, bAPI ); }
    sal_Bool SttEndDoc( sal_Bool bStt );

    sal_Bool MovePage( SwWhichPage, SwPosPage );
    sal_Bool MovePara( SwWhichPara, SwPosPara );
    sal_Bool MoveSection( SwWhichSection, SwPosSection );
    sal_Bool MoveTable( SwWhichTable, SwPosTable );
    sal_Bool MoveColumn( SwWhichColumn, SwPosColumn );
    sal_Bool MoveRegion( SwWhichRegion, SwPosRegion );

    sal_uLong Find( const com::sun::star::util::SearchOptions& rSearchOpt,
                sal_Bool bSearchInNotes,
                SwDocPositions eStart, SwDocPositions eEnde,
                sal_Bool& bCancel,
                FindRanges eRng, int bReplace = sal_False );

    sal_uLong Find( const SwTxtFmtColl& rFmtColl,
                SwDocPositions eStart, SwDocPositions eEnde,
                sal_Bool& bCancel,
                FindRanges eRng, const SwTxtFmtColl* pReplFmt = 0 );

    sal_uLong Find( const SfxItemSet& rSet, sal_Bool bNoCollections,
                SwDocPositions eStart, SwDocPositions eEnde,
                sal_Bool& bCancel,
                FindRanges eRng,
                const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                const SfxItemSet* rReplSet = 0 );

    //  Position the Cursor
    //  return values:
    //      CRSR_POSCHG: when cursor was corrected from SPoint by the layout
    //      CRSR_POSOLD: when the cursor was not changed
    int SetCrsr( const Point &rPt, sal_Bool bOnlyText = sal_False, bool bBlock = true );

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
    inline sal_Bool HasMark();

    void ClearMark();

    /**
       Ensure point and mark of the current PaM are in a specific order.

       @param bPointFirst TRUE: If the point is behind the mark then
       swap the PaM. FALSE: If the mark is behind the point then swap
       the PaM.
    */
    void NormalizePam(sal_Bool bPointFirst = sal_True);

    void SwapPam();
    sal_Bool ChgCurrPam( const Point & rPt,
                     sal_Bool bTstOnly = sal_True,      // test only, don't set
                     sal_Bool bTstHit  = sal_False );   // only exact matches
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
    sal_Bool Pop( sal_Bool bOldCrsr = sal_True );
    /*
     * Combine 2 Cursors.
     * Delete the topmost from the stack and move its Mark into the current.
     */
    void Combine();

#ifdef DBG_UTIL
    void SttCrsrMove();
    void EndCrsrMove( const sal_Bool bIdleEnd = sal_False );
#else
    void SttCrsrMove() { ++nCrsrMove; StartAction(); }
    void EndCrsrMove( const sal_Bool bIdleEnd = sal_False )
            { EndAction( bIdleEnd ); --nCrsrMove; }
#endif

    /*
     * When the focus is lost the selected ranges are not displayed anymore.
     * On the other hand, on receiving the focus all selected ranges are displayed again
     * (ranges must be recalculated!).
     */
    sal_Bool HasShFcs() const { return bHasFocus; }
    void ShLooseFcs();
    void ShGetFcs( sal_Bool bUpdate = sal_True );

    // Methods for displaying or hiding the visible text cursor.
    void ShowCrsr();
    void HideCrsr();
    // Methods for displaying or hiding the selected ranges with visible cursor.
    void ShowCrsrs( sal_Bool bCrsrVis );
    void HideCrsrs();

    sal_Bool IsOverwriteCrsr() const { return bOverwriteCrsr; }
    void SetOverwriteCrsr( sal_Bool bFlag ) { bOverwriteCrsr = bFlag; }

    // Return current frame in which the cursor is placed.
    SwCntntFrm *GetCurrFrm( const sal_Bool bCalcFrm = sal_True ) const;

    //TRUE if cursor is hidden because of readonly.
    //FALSE if it is working despite readonly.
    sal_Bool IsCrsrReadonly() const;

    // Cursor is placed in something that is protected or selection contains
    // something that is protected.
    sal_Bool HasReadonlySel() const;

    // Can the cursor be set to read only ranges?
    sal_Bool IsReadOnlyAvailable() const { return bSetCrsrInReadOnly; }
    void SetReadOnlyAvailable( sal_Bool bFlag );
    sal_Bool IsOverReadOnlyPos( const Point& rPt ) const;

    /// Is the rPt point in the header or footer area?
    sal_Bool IsOverHeaderFooterPos( const Point& rPt ) const;

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
    sal_Bool HasSelection() const;

    // Check if a selection exists, i.e. if the current cursor comprises a selection.
    inline sal_Bool IsSelection() const;
    // returns if multiple cursors are available
    inline sal_Bool IsMultiSelection() const;

    // Check if a complete paragraph was selected.
    sal_Bool IsSelFullPara() const;

    // Check if selection is within one paragraph.

    //Should WaitPtr be activated for Clipboard.
    bool ShouldWait() const;

    // Check if selection is within one paragraph.
    inline sal_Bool IsSelOnePara() const;

    /*
     * Returns SRectangle, at which the cursor is located.
     */
    const SwRect &GetCharRect() const { return aCharRect; }
    /*
     * Returns if cursor is wholly or partly within visible range.
     */
    sal_Bool IsCrsrVisible() const { return VisArea().IsOver( GetCharRect() ); }

    // Return current page number:
    // TRUE:  in which cursor is located.
    // FALSE: which is visible at the upper margin.
    void GetPageNum( sal_uInt16 &rnPhyNum, sal_uInt16 &rnVirtNum,
                     sal_Bool bAtCrsrPos = sal_True, const sal_Bool bCalcFrm = sal_True );
    // Determine how "empty pages" are handled
    // (used in PhyPage).
    sal_uInt16 GetNextPrevPageNum( sal_Bool bNext = sal_True );

    // Move cursor at the bginning of page "nPage".
    sal_Bool GotoPage( sal_uInt16 nPage );

    sal_uInt16 GetPageCnt();

    sal_Bool GoNextCrsr();

    sal_Bool GoPrevCrsr();

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
    inline const SwTableNode* IsCrsrInTbl( sal_Bool bIsPtInTbl = sal_True ) const;

    inline Point& GetCrsrDocPos( sal_Bool bPoint = sal_True ) const;
    inline sal_Bool IsCrsrPtAtEnd() const;

    inline const  SwPaM* GetTblCrs() const;
    inline        SwPaM* GetTblCrs();

    bool IsTblComplex() const;
    bool IsTblComplexForChart();
    // get current table selection as text
    String GetBoxNms() const;

    // set Crsr to the next/previous cell
    sal_Bool GoNextCell( sal_Bool bAppendLine = sal_True );
    sal_Bool GoPrevCell();
    // go to this box (if available and inside of table)
    bool GotoTable( const String& rName );

    // select a table row, column or box (based on the current cursor)
    sal_Bool SelTblRow() { return _SelTblRowOrCol( true  ); }
    sal_Bool SelTblCol() { return _SelTblRowOrCol( false ); }
    sal_Bool SelTblBox();

    sal_Bool SelTbl();

    sal_Bool GotoNextNum();
    sal_Bool GotoPrevNum();

    bool GotoOutline( const String& rName );
    // to the next/previous or the given OutlineNode
    void GotoOutline( sal_uInt16 nIdx );
    // find the "outline position" in the nodes array of the current chapter
    sal_uInt16 GetOutlinePos( sal_uInt8 nLevel = UCHAR_MAX );
    // select the given range of OutlineNodes. Optionally including the children
    // the sal_uInt16s are the positions in OutlineNodes-Array (EditShell)
    sal_Bool MakeOutlineSel( sal_uInt16 nSttPos, sal_uInt16 nEndPos,
                        sal_Bool bWithChildren = sal_False );

    sal_Bool GotoNextOutline();
    sal_Bool GotoPrevOutline();

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
    sal_Bool IsTableMode() const { return 0 != pTblCrsr; }

    const SwShellTableCrsr* GetTableCrsr() const { return pTblCrsr; }
    SwShellTableCrsr* GetTableCrsr() { return pTblCrsr; }
    size_t UpdateTblSelBoxes();

    sal_Bool GotoFtnTxt();      ///< jump from content to footnote
    sal_Bool GotoFtnAnchor();   ///< jump from footnote to anchor
    sal_Bool GotoPrevFtnAnchor();
    sal_Bool GotoNextFtnAnchor();

    sal_Bool GotoFlyAnchor();       ///< jump from the frame to the anchor
    sal_Bool GotoHeaderTxt();       ///< jump from the content to the header
    sal_Bool GotoFooterTxt();       ///< jump from the content to the footer
    // jump to the header/footer of the given or current PageDesc
    sal_Bool SetCrsrInHdFt( sal_uInt16 nDescNo = USHRT_MAX,
                            sal_Bool bInHeader = sal_True );
    // is point of cursor in header/footer. pbInHeader return sal_True if it is
    // in a headerframe otherwise in a footerframe
    bool IsInHeaderFooter( sal_Bool* pbInHeader = 0 ) const;

    sal_Bool GotoNextTOXBase( const String* = 0 );
    sal_Bool GotoPrevTOXBase( const String* = 0 );
    sal_Bool GotoTOXMarkBase();
    // jump to the next or previous index entry
    sal_Bool GotoNxtPrvTOXMark( sal_Bool bNext = sal_True );
    // jump to the next/previous index mark of this type
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rStart, SwTOXSearch eDir );

    // jump to the next or previous table formula
    // optionally only to broken formulas
    sal_Bool GotoNxtPrvTblFormula( sal_Bool bNext = sal_True,
                                    sal_Bool bOnlyErrors = sal_False );
    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    sal_Bool SelectNxtPrvHyperlink( sal_Bool bNext = sal_True );

    sal_Bool GotoRefMark( const String& rRefMark, sal_uInt16 nSubType = 0,
                            sal_uInt16 nSeqNo = 0 );

    // get the nth character from the start or end of the  current selection
    sal_Unicode GetChar( sal_Bool bEnd = sal_True, long nOffset = 0 );
    sal_Bool ExtendSelection( sal_Bool bEnd = sal_True, xub_StrLen nCount = 1 );

    // Place only the visible cursor at the given position in the document.
    // Return FALSE if SPoint was corrected by layout.
    // (This is needed for displaying the Drag&Drop/Copy-Cursor.)
    sal_Bool SetVisCrsr( const Point &rPt );
    inline void UnSetVisCrsr();

    // jump to the next or previous field of the corresponding type
    sal_Bool MoveFldType( const SwFieldType* pFldType, sal_Bool bNext,
                                            sal_uInt16 nSubType = USHRT_MAX,
                                            sal_uInt16 nResType = USHRT_MAX );
    sal_Bool GotoFld( const SwFmtFld& rFld );

    // Return number of cursors in ring (The flag indicates whether
    // only cursors containing selections are requested).
    sal_uInt16 GetCrsrCnt( sal_Bool bAll = sal_True ) const;

    // Char Travelling - methods (in crstrvl1.cxx)
    sal_Bool GoStartWord();
    sal_Bool GoEndWord();
    sal_Bool GoNextWord();
    sal_Bool GoPrevWord();
    sal_Bool GoNextSentence();
    sal_Bool GoStartSentence();
    sal_Bool GoEndSentence();
    sal_Bool SelectWord( const Point* pPt = 0 );
    sal_Bool ExpandToSentenceBorders();

    // get position from current cursor
    sal_Bool IsStartWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES )const;
    sal_Bool IsEndWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    sal_Bool IsInWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    sal_Bool IsStartSentence() const;
    sal_Bool IsEndSentence() const;
    bool IsSttPara() const;
    bool IsEndPara() const;
    sal_Bool IsStartOfDoc() const;
    sal_Bool IsEndOfDoc() const;
    bool IsInFrontOfLabel() const;
    sal_Bool IsAtLeftMargin()   const       { return IsAtLRMargin( sal_True ); }
    sal_Bool IsAtRightMargin(sal_Bool bAPI = sal_False) const   { return IsAtLRMargin( sal_False, bAPI ); }

    // delete all created cursors, set the table cursor and the last cursor to
    // its TextNode (or StartNode?)
    // They all get created on the next ::GetCrsr again
    // Used for Drag&Drop/Clipboard-Paste in tables
    sal_Bool ParkTblCrsr();

    // Non expanded attributes?
    sal_Bool IsGCAttr() const { return bGCAttr; }
    void ClearGCAttr() { bGCAttr = sal_False; }
    void    UpdateAttr() {  bGCAttr = sal_True; }

    // is the whole document protected/hidden (for UI...)
    sal_Bool IsAllProtect() const { return bAllProtect; }

    sal_Bool BasicActionPend() const    { return nBasicActionCnt != nStartAction; }

    bool GotoRegion( const String& rName );

    // show the current selection
    virtual void MakeSelVisible();

    // set the cursor to a NOT protected/hidden node
    sal_Bool FindValidCntntNode( sal_Bool bOnlyText = sal_False );

    sal_Bool GetContentAtPos( const Point& rPt,
                            SwContentAtPos& rCntntAtPos,
                            sal_Bool bSetCrsr = sal_False,
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

    sal_Bool IsPageAtPos( const Point &rPt ) const;

    sal_Bool SelectTxtAttr( sal_uInt16 nWhich, sal_Bool bExpand, const SwTxtAttr* pAttr = 0 );
    bool GotoINetAttr( const SwTxtINetFmt& rAttr );
    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    sal_Bool CheckTblBoxCntnt( const SwPosition* pPos = 0 );
    void SaveTblBoxCntnt( const SwPosition* pPos = 0 );
    void ClearTblBoxCntnt();
    sal_Bool EndAllTblBoxEdit();

    void SetSelTblCells( sal_Bool bFlag )           { bSelTblCells = bFlag; }
    sal_Bool IsSelTblCells() const                  { return bSelTblCells; }

    sal_Bool IsAutoUpdateCells() const              { return bAutoUpdateCells; }
    void SetAutoUpdateCells( sal_Bool bFlag )       { bAutoUpdateCells = bFlag; }

    sal_Bool GetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode,
                            SwRect& rRect, short& rOrient );
    sal_Bool SetShadowCrsrPos( const Point& rPt, SwFillMode eFillMode );

    const SwRedline* SelNextRedline();
    const SwRedline* SelPrevRedline();
    const SwRedline* GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect = sal_False );

    // is cursor or the point in/over a vertical formatted text?
    sal_Bool IsInVerticalText( const Point* pPt = 0 ) const;
    // is cursor or the point in/over a right to left formatted text?
    sal_Bool IsInRightToLeftText( const Point* pPt = 0 ) const;

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
            sal_uInt16 nStt, sal_uInt16 nEnd, SwPaM* pPam ) const
{
    return pCurCrsr->MakeFindRange( (SwDocPositions)nStt, (SwDocPositions)nEnd, pPam );
}

inline SwCursor* SwCrsrShell::GetSwCrsr( sal_Bool bMakeTblCrsr ) const
{
    return (SwCursor*)GetCrsr( bMakeTblCrsr );
}

inline SwPaM* SwCrsrShell::GetStkCrsr() const { return pCrsrStk; }

inline void SwCrsrShell::SetMark() { pCurCrsr->SetMark(); }

inline sal_Bool SwCrsrShell::HasMark() { return( pCurCrsr->HasMark() ); }

inline sal_Bool SwCrsrShell::IsSelection() const
{
    return IsTableMode() || pCurCrsr->HasMark() ||
            pCurCrsr->GetNext() != pCurCrsr;
}
inline sal_Bool SwCrsrShell::IsMultiSelection() const
{
    return pCurCrsr->GetNext() != pCurCrsr;
}

inline sal_Bool SwCrsrShell::IsSelOnePara() const
{
    return pCurCrsr == pCurCrsr->GetNext() &&
           pCurCrsr->GetPoint()->nNode == pCurCrsr->GetMark()->nNode;
}

inline const SwTableNode* SwCrsrShell::IsCrsrInTbl( sal_Bool bIsPtInTbl ) const
{
    return pCurCrsr->GetNode( bIsPtInTbl )->FindTableNode();
}

inline sal_Bool SwCrsrShell::IsCrsrPtAtEnd() const
{
    return pCurCrsr->End() == pCurCrsr->GetPoint();
}

inline Point& SwCrsrShell::GetCrsrDocPos( sal_Bool bPoint ) const
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
    pVisCrsr->SetDragCrsr( false );
}

#endif  // _CRSRSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
