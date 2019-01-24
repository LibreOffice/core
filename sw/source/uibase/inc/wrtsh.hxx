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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WRTSH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WRTSH_HXX

#include <swdllapi.h>
#include <fesh.hxx>
#include <sortopt.hxx>
#include <swurl.hxx>
#include <IMark.hxx>
#include "navmgr.hxx"
#include <boost/optional.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <svx/swframetypes.hxx>

namespace vcl { class Window; }
class SbxArray;
class SwDoc;
class SwViewOption;
class SwFlyFrameAttrMgr;
class SwField;
class SwTOXBase;
class SwView;
class SvGlobalName;
class SwInputFieldList;
class SwSectionData;
class Timer;
class SvxMacro;
class SwFormatINetFormat;
class SvxAutoCorrect;
class NaviContentBookmark;
struct SwCallMouseEvent;
class SfxStringListItem;
enum class SvMacroItemId : sal_uInt16;

namespace i18nutil {
    struct SearchOptions2;
}

enum class SelectionType : sal_Int32
{
    NONE                 = 0x000000,
    Text                 =  CNT_TXT, // text, never frames too   0x0001
    Graphic              =  CNT_GRF, // graphic          0x0002
    Ole                  =  CNT_OLE, // OLE              0x0010
    Frame                = 0x000020, // frame, no content type
    NumberList           = 0x000040, // NumList
    Table                = 0x000080, // cursor is in table
    TableCell            = 0x000100, // table cells are selected
    DrawObject           = 0x000200, // drawing objects (rectangle, circle...)
    DrawObjectEditMode   = 0x000400, // draw-textobjects in edit mode
    Ornament             = 0x000800, // edit ornament objects
    DbForm               = 0x001000, // drawing objects: DB-Forms
    FormControl          = 0x002000, // a form control is focused. Neither set nor evaluated by the SwWrtShell itself, only by its clients.
    Media                = 0x004000, // Media object
    ExtrudedCustomShape  = 0x008000, // extruded custom shape
    FontWork             = 0x010000, // fontwork
    PostIt               = 0x020000, // annotation
    All                  = 0x03fff3,
};
namespace o3tl {
    template<> struct typed_flags<SelectionType> : is_typed_flags<SelectionType, 0x03fff3> {};
}

/** Used by the UI to modify the document model.

Eg. the Insert() method will take the current cursor position, insert the
string, and take care of undo etc.
*/
class SW_DLLPUBLIC SwWrtShell: public SwFEShell
{
private:
    using SwCursorShell::Left;
    using SwCursorShell::Right;
    using SwCursorShell::Up;
    using SwCursorShell::Down;
    using SwCursorShell::LeftMargin;
    using SwCursorShell::RightMargin;
    using SwCursorShell::SelectTextAttr;
    using SwCursorShell::GotoPage;
    using SwFEShell::InsertObject;
    using SwEditShell::AutoCorrect;
    using SwCursorShell::GotoMark;

    typedef long (SwWrtShell::*SELECTFUNC)(const Point *, bool bProp );
    typedef void (SwWrtShell::*SELECTFUNC2)(const Point *, bool bProp );

    SELECTFUNC2 m_fnDrag      = &SwWrtShell::BeginDrag;
    SELECTFUNC  m_fnSetCursor = &SwWrtShell::SetCursor;
    SELECTFUNC2 m_fnEndDrag   = &SwWrtShell::DefaultEndDrag;
    SELECTFUNC  m_fnKillSel   = &SwWrtShell::Ignore;

public:

    using SwEditShell::Insert;

    long CallSetCursor(const Point* pPt, bool bProp) { return (this->*m_fnSetCursor)(pPt, bProp); }
    void Drag         (const Point* pPt, bool bProp) { (this->*m_fnDrag)(pPt, bProp); }
    void EndDrag      (const Point* pPt, bool bProp) { (this->*m_fnEndDrag)(pPt, bProp); }
    long KillSelection(const Point* pPt, bool bProp) { return (this->*m_fnKillSel)(pPt, bProp); }

    // reset all selections
    long ResetSelect( const Point *, bool );

    // resets the cursorstack after movement with PageUp/-Down if a stack is built up
    inline void ResetCursorStack();
    SelectionType   GetSelectionType() const;

    bool    IsModePushed() const { return nullptr != m_pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    bool    IsInSelect() const { return m_bInSelect; }
    void    SetInSelect() { m_bInSelect = true; }
        // is there a text- or frameselection?
    bool    HasSelection() const { return SwCursorShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrameMode() || IsObjSelected(); }
    bool    Pop(SwCursorShell::PopMode = SwCursorShell::PopMode::DeleteStack);

    void    EnterStdMode();
    bool    IsStdMode() const { return !m_bExtMode && !m_bAddMode && !m_bBlockMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    bool    ToggleExtMode();
    bool    IsExtMode() const { return m_bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    void    ToggleAddMode();
    bool    IsAddMode() const { return m_bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    bool    ToggleBlockMode();
    bool    IsBlockMode() const { return m_bBlockMode; }

    void    SetInsMode( bool bOn = true );
    void    ToggleInsMode() { SetInsMode( !m_bIns ); }
    bool    IsInsMode() const { return m_bIns; }
    void    SetRedlineFlagsAndCheckInsMode( RedlineFlags eMode );

    void    EnterSelFrameMode(const Point *pStartDrag = nullptr);
    void    LeaveSelFrameMode();
    bool    IsSelFrameMode() const { return m_bLayoutMode; }
        // reset selection of frames
    void    UnSelectFrame();

    void    Invalidate();

    // select table cells for editing of formulas in the ribbonbar
    inline void SelTableCells( const Link<SwWrtShell&,void> &rLink );
    inline void EndSelTableCells();

    // leave per word or per line selection mode. Is usually called in MB-Up.
    bool    IsExtSel() const { return m_bSelWrd || m_bSelLn; }

    // query whether the active m_fnDrag pointer is set to BeginDrag
    // is needed for MouseMove to work around bugs 55592/55931
    inline bool Is_FnDragEQBeginDrag() const;

    // base requests
    bool    IsEndWrd();
    bool    IsSttOfPara() const { return IsSttPara(); }
    bool    IsEndOfPara() const { return IsEndPara(); }

    // select word / sentence
    bool    SelNearestWrd();
    bool    SelWrd      (const Point * = nullptr );
    // #i32329# Enhanced selection
    void    SelSentence (const Point *);
    void    SelPara     (const Point *);
    void    SelAll();

    // basecursortravelling
typedef bool (SwWrtShell:: *FNSimpleMove)();
    bool SimpleMove( FNSimpleMove, bool bSelect );

    bool Left       ( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    bool Right      ( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    bool Up         ( bool bSelect, sal_uInt16 nCount = 1, bool bBasicCall = false );
    bool Down       ( bool bSelect, sal_uInt16 nCount = 1, bool bBasicCall = false );
    void NxtWrd     ( bool bSelect = false ) { SimpleMove( &SwWrtShell::NxtWrd_, bSelect ); }
    bool PrvWrd     ( bool bSelect = false ) { return SimpleMove( &SwWrtShell::PrvWrd_, bSelect ); }

    bool LeftMargin ( bool bSelect, bool bBasicCall );
    bool RightMargin( bool bSelect, bool bBasicCall );

    bool StartOfSection( bool bSelect = false );
    bool EndOfSection  ( bool bSelect = false );

    bool SttNxtPg   ( bool bSelect = false );
    void SttPrvPg   ( bool bSelect = false );
    void EndNxtPg   ( bool bSelect = false );
    bool EndPrvPg   ( bool bSelect = false );
    bool SttPg      ( bool bSelect = false );
    bool EndPg      ( bool bSelect = false );
    bool SttPara    ( bool bSelect = false );
    void EndPara    ( bool bSelect = false );
    bool FwdPara    ()
                { return SimpleMove( &SwWrtShell::FwdPara_, false/*bSelect*/ ); }
    void BwdPara    ()
                { SimpleMove( &SwWrtShell::BwdPara_, false/*bSelect*/ ); }
    void FwdSentence( bool bSelect = false )
                { SimpleMove( &SwWrtShell::FwdSentence_, bSelect ); }
    void BwdSentence( bool bSelect = false )
                { SimpleMove( &SwWrtShell::BwdSentence_, bSelect ); }

    // #i20126# Enhanced table selection
    bool SelectTableRowCol( const Point& rPt, const Point* pEnd = nullptr, bool bRowDrag = false );
    void SelectTableRow();
    void SelectTableCol();
    void SelectTableCell();

    bool SelectTextAttr( sal_uInt16 nWhich, const SwTextAttr* pAttr = nullptr );

    // per column jumps
    void StartOfColumn      ();
    void EndOfColumn        ();
    void StartOfNextColumn  ();
    void EndOfNextColumn    ();
    void StartOfPrevColumn  ();
    void EndOfPrevColumn    ();

    // set the cursor to page "nPage" at the beginning
    // additionally to a identically named implementation in crsrsh.hxx
    // here all existing selections are being reset before setting the
    // cursor
    bool    GotoPage( sal_uInt16 nPage, bool bRecord );

    // setting the cursor; remember the old position for turning back
    DECL_LINK( ExecFlyMac, const SwFlyFrameFormat*, void );

    bool    PageCursor(SwTwips lOffset, bool bSelect);

    // update fields
    void    UpdateInputFields( SwInputFieldList* pLst = nullptr );

    void    NoEdit(bool bHideCursor = true);
    void    Edit();

    bool IsRetainSelection() const { return m_bRetainSelection; }
    void SetRetainSelection( bool bRet ) { m_bRetainSelection = bRet; }

    // change current data base and notify
    void ChgDBData(const SwDBData& SwDBData);

    // delete
    void    DelToEndOfLine();
    void    DelToStartOfLine();
    void    DelLine();
    bool    DelLeft();

    // also deletes the frame or sets the cursor in the frame when bDelFrame == false
    bool    DelRight();
    void    DelToEndOfPara();
    void    DelToStartOfPara();
    bool    DelToEndOfSentence();
    void    DelToStartOfSentence();
    void    DelNxtWord();
    void    DelPrvWord();

    // checks whether a word selection exists.
    // According to the rules for intelligent Cut / Paste
    // surrounding spaces are cut out.
    // returns type of word selection (see enum)
    enum word {
            NO_WORD = 0,
            WORD_SPACE_BEFORE = 1,
            WORD_SPACE_AFTER = 2,
            WORD_NO_SPACE = 3
        };
    int     IntelligentCut(SelectionType nSelectionType, bool bCut = true);

    // edit
    void    Insert(SwField const &);
    void    Insert(const OUString &);
    // graphic
    void    Insert( const OUString &rPath, const OUString &rFilter,
                    const Graphic &, SwFlyFrameAttrMgr * = nullptr,
                    RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA);

    void    InsertByWord( const OUString & );
    void    InsertPageBreak(const OUString *pPageDesc = nullptr, const ::boost::optional<sal_uInt16>& rPgNum = boost::none);
    void    InsertLineBreak();
    void    InsertColumnBreak();
    void    InsertFootnote(const OUString &, bool bEndNote = false, bool bEdit = true );
    void    SplitNode( bool bAutoFormat = false );
    bool    CanInsert();

    // indexes
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = nullptr);
    void    UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = nullptr);

    // numbering and bullets
    /**
       Turns on numbering or bullets.

       @param bNum    true: turn on numbering
                      false: turn on bullets
    */
    void    NumOrBulletOn(bool bNum); // #i29560#
    void    NumOrBulletOff(); // #i29560#
    void    NumOn();
    void    BulletOn();

    //OLE
    void    InsertObject(     /*SvInPlaceObjectRef *pObj, */       // != 0 for clipboard
                          const svt::EmbeddedObjectRef&,
                          SvGlobalName const *pName,      // != 0 create object accordingly
                          sal_uInt16 nSlotId = 0);       // SlotId for dialog

    bool    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrameFormat **pFlyFrameFormat = nullptr );
    void    LaunchOLEObj( long nVerb = 0 );             // start server
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset ) override;
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = nullptr,
                                  const SwRect *pFlyFrameRect = nullptr,
                                  const bool bNoTextFramePrtAreaChanged = false ) override;
    virtual void ConnectObj( svt::EmbeddedObjectRef&  xIPObj, const SwRect &rPrt,
                             const SwRect &rFrame ) override;

    // styles and formats

    // enum tells when should happen when the style was not found
    enum GetStyle { GETSTYLE_NOCREATE,          // create none
                    GETSTYLE_CREATESOME,        // if on PoolId create mapt
                    GETSTYLE_CREATEANY };       // return standard if applicable

    SwTextFormatColl*   GetParaStyle(const OUString &rCollName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwCharFormat*      GetCharStyle(const OUString &rFormatName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwFrameFormat*       GetTableStyle(const OUString &rFormatName);

    void            SetPageStyle(const OUString &rCollName);

    OUString const & GetCurPageStyle() const;

    // change current style using the attributes in effect
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    enum class FieldDialogPressedButton { NONE, Previous, Next };

    void    Do( DoType eDoType, sal_uInt16 nCnt = 1 );
    OUString  GetDoString( DoType eDoType ) const;
    OUString  GetRepeatString() const;
    void    GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    // search and replace
    sal_uLong SearchPattern(const i18nutil::SearchOptions2& rSearchOpt,
                         bool bSearchInNotes,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         bool bReplace = false );

    sal_uLong SearchTempl  (const OUString &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         const OUString* pReplTempl = nullptr );

    sal_uLong SearchAttr   (const SfxItemSet& rFindSet,
                         bool bNoColls,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         const i18nutil::SearchOptions2* pSearchOpt = nullptr,
                         const SfxItemSet* pReplaceSet = nullptr);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar );

    // action ahead of cursor movement
    // resets selection if applicable, triggers timer and GCAttr()
    void    MoveCursor( bool bWithSelect = false );

    // update input fields
    bool    StartInputFieldDlg(SwField*, bool bPrevButton, bool bNextButton, weld::Window* pParentWin, FieldDialogPressedButton* pPressedButton = nullptr);
    // update DropDown fields
    bool    StartDropDownFieldDlg(SwField*, bool bPrevButton, bool bNextButton, weld::Window* pParentWin, FieldDialogPressedButton* pPressedButton = nullptr);

    //"Handler" for changes at DrawView - for controls.
    virtual void DrawSelChanged( ) override;

    // jump to bookmark and set the "selections-flags" correctly again
    void GotoMark( const ::sw::mark::IMark* const pMark );
    bool GotoMark( const ::sw::mark::IMark* const pMark, bool bSelect );
    void GotoMark( const OUString& rName );
    bool GoNextBookmark(); // true when there still was one
    bool GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark);

    bool GotoField( const SwFormatField& rField );

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    void SelectNextPrevHyperlink( bool bNext );

    // determine corresponding SwView
    const SwView&       GetView() const { return m_rView; }
    SwView&             GetView() { return m_rView; }

    // Because nobody else is doing it, here is a ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, OUString* pRet = nullptr, SbxArray* pArgs = nullptr );
    // call into the dark Basic/JavaScript
    sal_uInt16 CallEvent( SvMacroItemId nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bCheckPtr = false );

    // a click at the given field. the cursor is on it.
    // execute the predefined actions.
    void ClickToField( const SwField& rField );
    void ClickToINetAttr( const SwFormatINetFormat& rItem, LoadUrlFlags nFilter = LoadUrlFlags::NONE );
    bool ClickToINetGrf( const Point& rDocPt, LoadUrlFlags nFilter );
    inline bool IsInClickToEdit() const ;

    // if a URL-Button is selected, return its URL; otherwise an empty string
    bool GetURLFromButton( OUString& rURL, OUString& rDescr ) const;

    void NavigatorPaste( const NaviContentBookmark& rBkmk,
                         const sal_uInt16 nAction );

    virtual void ApplyViewOptions( const SwViewOption &rOpt ) override;
    virtual void SetReadonlyOption( bool bSet ) override;

    // automatic update of styles
    void AutoUpdateFrame(SwFrameFormat* pFormat, const SfxItemSet& rStyleSet);
    void AutoUpdatePara(SwTextFormatColl* pColl, const SfxItemSet& rStyleSet, SwPaM* pPaM = nullptr );

    // starts dialog for inserting ranges via Drag&Drop/Clipboard
    void StartInsertRegionDialog(const SwSectionData&);

    // ctor, the first one is a kind of a controlled copy ctor for more views of a document
    SwWrtShell( SwWrtShell&, vcl::Window *pWin, SwView &rShell);
    SwWrtShell( SwDoc& rDoc, vcl::Window *pWin, SwView &rShell,
                const SwViewOption *pViewOpt);
    virtual ~SwWrtShell() override;

    bool TryRemoveIndent(); // #i23725#

    OUString GetSelDescr() const;

    SwNavigationMgr& GetNavigationMgr() { return m_aNavigationMgr; }

    void addCurrentPosition();
    bool GotoFly( const OUString& rName, FlyCntType eType = FLYCNTTYPE_ALL,
         bool bSelFrame = true );
    bool GotoINetAttr( const SwTextINetFormat& rAttr );
    void GotoOutline( SwOutlineNodes::size_type nIdx );
    bool GotoOutline( const OUString& rName );
    bool GotoRegion( const OUString& rName );
    void GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType = 0,
        sal_uInt16 nSeqNo = 0 );
    bool GotoNextTOXBase( const OUString* pName = nullptr);
    bool GotoTable( const OUString& rName );
    void GotoFormatField( const SwFormatField& rField );
    const SwRangeRedline* GotoRedline( SwRedlineTable::size_type nArrPos, bool bSelect);

    void ChangeHeaderOrFooter(const OUString& rStyleName, bool bHeader, bool bOn, bool bShowWarning);
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow ) override;

private:

    SAL_DLLPRIVATE void  OpenMark();
    SAL_DLLPRIVATE void  CloseMark( bool bOkFlag );

    struct ModeStack
    {
        ModeStack   *pNext;
        bool const  bAdd,
                    bBlock,
                    bExt,
                    bIns;
        ModeStack(ModeStack *pNextMode, bool _bIns, bool _bExt, bool _bAdd, bool _bBlock):
            pNext(pNextMode),
            bAdd(_bAdd),
            bBlock(_bBlock),
            bExt(_bExt),
            bIns(_bIns)
             {}
    } *m_pModeStack = nullptr;

    // carry cursor along when PageUp / -Down
    enum PageMove
    {
        MV_NO,
        MV_PAGE_UP,
        MV_PAGE_DOWN
    } m_ePageMove = MV_NO;

    struct CursorStack
    {
        Point aDocPos;
        std::unique_ptr<CursorStack> pNext;
        bool const bValidCurPos : 1;
        bool bIsFrameSel : 1;
        SwTwips const lOffset;

        CursorStack( bool bValid, bool bFrameSel, const Point &rDocPos,
                    SwTwips lOff, std::unique_ptr<CursorStack> pN )
            : aDocPos(rDocPos),
            pNext(std::move(pN)),
            bValidCurPos( bValid ),
            bIsFrameSel( bFrameSel ),
            lOffset(lOff)
        {
        }

    };
    std::unique_ptr<CursorStack> m_pCursorStack;

    SwView  &m_rView;
    SwNavigationMgr m_aNavigationMgr;

    Point   m_aDest;
    bool    m_bDestOnStack = false;
    bool    HasCursorStack() const { return nullptr != m_pCursorStack; }
    SAL_DLLPRIVATE bool  PushCursor(SwTwips lOffset, bool bSelect);
    SAL_DLLPRIVATE bool  PopCursor(bool bUpdate, bool bSelect = false);

    // take END cursor along when PageUp / -Down
    SAL_DLLPRIVATE void SttWrd();
    SAL_DLLPRIVATE void EndWrd();
    SAL_DLLPRIVATE bool NxtWrd_();
    SAL_DLLPRIVATE bool PrvWrd_();
    // #i92468#
    SAL_DLLPRIVATE bool NxtWrdForDelete();
    SAL_DLLPRIVATE bool PrvWrdForDelete();
    SAL_DLLPRIVATE bool FwdSentence_();
    SAL_DLLPRIVATE bool BwdSentence_();
    bool FwdPara_();
    SAL_DLLPRIVATE bool BwdPara_();

        // selections
    bool    m_bIns            :1;
    bool    m_bInSelect       :1;
    bool    m_bExtMode        :1;
    bool    m_bAddMode        :1;
    bool    m_bBlockMode      :1;
    bool    m_bLayoutMode     :1;
    bool    m_bSelWrd         :1;
    bool    m_bSelLn          :1;
    bool    m_bIsInClickToEdit:1;
    bool    m_bClearMark      :1;     // don't delete selection for ChartAutoPilot
    bool    m_bRetainSelection :1; // Do not remove selections

    Point   m_aStart;
    Link<SwWrtShell&,void>  m_aSelTableLink;

    // resets the cursor stack after movement by PageUp/-Down
    SAL_DLLPRIVATE void  ResetCursorStack_();

    using SwCursorShell::SetCursor;
    SAL_DLLPRIVATE long  SetCursor(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  SetCursorKillSel(const Point *, bool bProp );

    SAL_DLLPRIVATE void  BeginDrag(const Point *, bool bProp );
    SAL_DLLPRIVATE void  DefaultDrag(const Point *, bool bProp );
    SAL_DLLPRIVATE void  DefaultEndDrag(const Point *, bool bProp );

    SAL_DLLPRIVATE void  ExtSelWrd(const Point *, bool bProp );
    SAL_DLLPRIVATE void  ExtSelLn(const Point *, bool bProp );

    SAL_DLLPRIVATE void  BeginFrameDrag(const Point *, bool bProp );

    // after SSize/Move of a frame update; Point is destination.
    SAL_DLLPRIVATE void  UpdateLayoutFrame(const Point *, bool bProp );

    SAL_DLLPRIVATE void  SttLeaveSelect();
    SAL_DLLPRIVATE void  AddLeaveSelect();
    SAL_DLLPRIVATE long  Ignore(const Point *, bool bProp );

    SAL_DLLPRIVATE void  LeaveExtSel() { m_bSelWrd = m_bSelLn = false;}

    SAL_DLLPRIVATE bool  GoStart(bool KeepArea, bool *,
            bool bSelect, bool bDontMoveRegion = false);
    SAL_DLLPRIVATE bool  GoEnd(bool KeepArea = false, const bool * = nullptr);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV
    };

    SAL_DLLPRIVATE bool MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=nullptr);
};

inline void SwWrtShell::ResetCursorStack()
{
    if ( HasCursorStack() )
        ResetCursorStack_();
}

inline void SwWrtShell::SelTableCells(const Link<SwWrtShell&,void> &rLink )
{
    SetSelTableCells( true );
    m_bClearMark = true;
    m_aSelTableLink = rLink;
}
inline void SwWrtShell::EndSelTableCells()
{
    SetSelTableCells( false );
    m_bClearMark = true;
}

inline bool SwWrtShell::IsInClickToEdit() const { return m_bIsInClickToEdit; }

inline bool SwWrtShell::Is_FnDragEQBeginDrag() const
{
#ifdef __GNUC__
    SELECTFUNC2 fnTmp = &SwWrtShell::BeginDrag;
    return m_fnDrag == fnTmp;
#else
    return m_fnDrag == &SwWrtShell::BeginDrag;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
