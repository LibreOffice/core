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

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include "swdllapi.h"
#include <fesh.hxx>
#include <sortopt.hxx>
#include <swurl.hxx>
#include <IMark.hxx>
#include "navmgr.hxx"
#include <boost/optional.hpp>

namespace vcl { class Window; }
class SbxArray;
class SwDoc;
class SwViewOption;
class SwFlyFrmAttrMgr;
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

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }

typedef sal_Int32 SelectionType;
namespace nsSelectionType
{
    const SelectionType SEL_TXT             = CNT_TXT;  // text, never frames too   0x0001
    const SelectionType SEL_GRF             = CNT_GRF;  // graphic          0x0002
    const SelectionType SEL_OLE             = CNT_OLE;  // OLE              0x0010
    const SelectionType SEL_FRM             = 0x000020; // frame, no content type
    const SelectionType SEL_NUM             = 0x000040; // NumList
    const SelectionType SEL_TBL             = 0x000080; // cursor is in table
    const SelectionType SEL_TBL_CELLS       = 0x000100; // table cells are selected
    const SelectionType SEL_DRW             = 0x000200; // drawing objects (rectangle, circle...)
    const SelectionType SEL_DRW_TXT         = 0x000400; // draw-textobjects in edit mode
    const SelectionType SEL_BEZ             = 0x000800; // edit ornament objects
    const SelectionType SEL_DRW_FORM        = 0x001000; // drawing objects: DB-Forms
    const SelectionType SEL_FOC_FRM_CTRL    = 0x002000; // a form control is focused. Neither set nor evaluated by the SwWrtShell itself, only by it's clients.
    const SelectionType SEL_MEDIA           = 0x004000; // Media object
    const SelectionType SEL_EXTRUDED_CUSTOMSHAPE = 0x008000;    // extruded custom shape
    const SelectionType SEL_FONTWORK        = 0x010000; // fontwork
    const SelectionType SEL_POSTIT          = 0x020000; //annotation
}

/** Used by the UI to modify the document model.

Eg. the Insert() method will take the current cursor position, insert the
string, and take care of undo etc.
*/
class SW_DLLPUBLIC SwWrtShell: public SwFEShell
{
private:
    using SwCrsrShell::Left;
    using SwCrsrShell::Right;
    using SwCrsrShell::Up;
    using SwCrsrShell::Down;
    using SwCrsrShell::LeftMargin;
    using SwCrsrShell::RightMargin;
    using SwCrsrShell::SelectTextAttr;
    using SwCrsrShell::GotoPage;
    using SwFEShell::InsertObject;
    using SwEditShell::AutoCorrect;
    using SwCrsrShell::GotoMark;

    typedef long (SwWrtShell::*SELECTFUNC)(const Point *, bool bProp );

    SELECTFUNC  m_fnDrag;
    SELECTFUNC  m_fnSetCrsr;
    SELECTFUNC  m_fnEndDrag;
    SELECTFUNC  m_fnKillSel;

public:

    using SwEditShell::Insert;

    long SetCursor    (const Point* pPt, bool bProp) { return (this->*m_fnSetCrsr)(pPt, bProp); }
    long Drag         (const Point* pPt, bool bProp) { return (this->*m_fnDrag)(pPt, bProp); }
    long EndDrag      (const Point* pPt, bool bProp) { return (this->*m_fnEndDrag)(pPt, bProp); }
    long KillSelection(const Point* pPt, bool bProp) { return (this->*m_fnKillSel)(pPt, bProp); }

    // reset all selections
    long ResetSelect( const Point *, bool );

    // resets the cursorstack after movement with PageUp/-Down if a stack is built up
    inline void ResetCursorStack();
    SelectionType   GetSelectionType() const;

    bool    IsModePushed() const { return 0 != m_pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    bool    IsInSelect() const { return m_bInSelect; }
    void    SetInSelect(bool bSel = true) { m_bInSelect = bSel; }
        // is there a text- or frameselection?
    bool    HasSelection() const { return SwCrsrShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrmMode() || IsObjSelected(); }
    bool     Pop( bool bOldCrsr = true );

    void    EnterStdMode();
    bool    IsStdMode() const { return !m_bExtMode && !m_bAddMode && !m_bBlockMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    bool    ToggleExtMode();
    bool    IsExtMode() const { return m_bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    bool    ToggleAddMode();
    bool    IsAddMode() const { return m_bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    bool    ToggleBlockMode();
    bool    IsBlockMode() const { return m_bBlockMode; }

    void    SetInsMode( bool bOn = true );
    void    ToggleInsMode() { SetInsMode( !m_bIns ); }
    bool    IsInsMode() const { return m_bIns; }
    void    SetRedlineModeAndCheckInsMode( sal_uInt16 eMode );

    void    EnterSelFrmMode(const Point *pStartDrag = 0);
    void    LeaveSelFrmMode();
    bool    IsSelFrmMode() const { return m_bLayoutMode; }
        // reset selection of frames
    void    UnSelectFrm();

    void    Invalidate();

    // select table cells for editing of formulas in the ribbonbar
    inline void SelTableCells( const Link<> &rLink, bool bMark = true );
    inline void EndSelTableCells();

    // leave per word or per line selection mode. Is usually called in MB-Up.
    bool    IsExtSel() const { return m_bSelWrd || m_bSelLn; }

    // query whether the active m_fnDrag pointer is set to BeginDrag
    // is needed for MouseMove to work around bugs 55592/55931
    inline bool Is_FnDragEQBeginDrag() const;

    // base requests
    bool    IsInWrd()           { return IsInWord(); }
    bool    IsSttWrd()          { return IsStartWord(); }
    bool    IsEndWrd();
    bool    IsSttOfPara() const { return IsSttPara(); }
    bool    IsEndOfPara() const { return IsEndPara(); }

    // select word / sentense
    bool    SelNearestWrd();
    bool    SelWrd      (const Point * = 0, bool bProp=false );
    // #i32329# Enhanced selection
    void    SelSentence (const Point * = 0, bool bProp=false );
    void    SelPara     (const Point * = 0, bool bProp=false );
    long    SelAll();

    // basecursortravelling
typedef bool (SwWrtShell:: *FNSimpleMove)();
    bool SimpleMove( FNSimpleMove, bool bSelect );

    bool Left       ( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    bool Right      ( sal_uInt16 nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    bool Up         ( bool bSelect = false, sal_uInt16 nCount = 1, bool bBasicCall = false );
    bool Down       ( bool bSelect = false, sal_uInt16 nCount = 1, bool bBasicCall = false );
    bool NxtWrd     ( bool bSelect = false ) { return SimpleMove( &SwWrtShell::_NxtWrd, bSelect ); }
    bool PrvWrd     ( bool bSelect = false ) { return SimpleMove( &SwWrtShell::_PrvWrd, bSelect ); }

    bool LeftMargin ( bool bSelect, bool bBasicCall );
    bool RightMargin( bool bSelect, bool bBasicCall );

    bool SttDoc     ( bool bSelect = false );
    bool EndDoc     ( bool bSelect = false );

    bool SttNxtPg   ( bool bSelect = false );
    bool SttPrvPg   ( bool bSelect = false );
    bool EndNxtPg   ( bool bSelect = false );
    bool EndPrvPg   ( bool bSelect = false );
    bool SttPg      ( bool bSelect = false );
    bool EndPg      ( bool bSelect = false );
    bool SttPara    ( bool bSelect = false );
    bool EndPara    ( bool bSelect = false );
    bool FwdPara    ( bool bSelect = false )
                { return SimpleMove( &SwWrtShell::_FwdPara, bSelect ); }
    bool BwdPara    ( bool bSelect = false )
                { return SimpleMove( &SwWrtShell::_BwdPara, bSelect ); }
    bool FwdSentence( bool bSelect = false )
                { return SimpleMove( &SwWrtShell::_FwdSentence, bSelect ); }
    bool BwdSentence( bool bSelect = false )
                { return SimpleMove( &SwWrtShell::_BwdSentence, bSelect ); }

    // #i20126# Enhanced table selection
    bool SelectTableRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );
    bool SelectTableRow();
    bool SelectTableCol();
    bool SelectTableCell();

    bool SelectTextAttr( sal_uInt16 nWhich, const SwTextAttr* pAttr = 0 );

    // per column jumps
    bool StartOfColumn      ( bool bSelect = false );
    bool EndOfColumn        ( bool bSelect = false );
    bool StartOfNextColumn  ( bool bSelect = false );
    bool EndOfNextColumn    ( bool bSelect = false );
    bool StartOfPrevColumn  ( bool bSelect = false );
    bool EndOfPrevColumn    ( bool bSelect = false );

    // set the cursor to page "nPage" at the beginning
    // additionally to a identically named implementation in crsrsh.hxx
    // here all existing selections are being reset before setting the
    // cursor
    bool    GotoPage( sal_uInt16 nPage, bool bRecord );

    // setting the cursor; remember the old position for turning back
    DECL_LINK_TYPED( ExecFlyMac, const SwFlyFrameFormat*, void );

    bool    PageCrsr(SwTwips lOffset, bool bSelect);

    // update fields
    void    UpdateInputFields( SwInputFieldList* pLst = 0 );

    void    NoEdit(bool bHideCrsr = true);
    void    Edit();

    bool IsRetainSelection() const { return m_bRetainSelection; }
    void SetRetainSelection( bool bRet ) { m_bRetainSelection = bRet; }

    // change current data base and notify
    void ChgDBData(const SwDBData& SwDBData);

    // delete
    long    DelToEndOfLine();
    long    DelToStartOfLine();
    long    DelLine();
    long    DelLeft();

    // also deletes the frame or sets the cursor in the frame when bDelFrm == false
    long    DelRight();
    long    DelToEndOfPara();
    long    DelToStartOfPara();
    long    DelToEndOfSentence();
    long    DelToStartOfSentence();
    long    DelNxtWord();
    long    DelPrvWord();

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
    int     IntelligentCut(int nSelectionType, bool bCut = true);

    // edit
    void    Insert(SwField &);
    void    Insert(const OUString &);
    // graphic
    void    Insert( const OUString &rPath, const OUString &rFilter,
                    const Graphic &, SwFlyFrmAttrMgr * = 0,
                    bool bRule = false );

    void    InsertByWord( const OUString & );
    void    InsertPageBreak(const OUString *pPageDesc = 0, const ::boost::optional<sal_uInt16>& rPgNum = boost::none);
    void    InsertLineBreak();
    void    InsertColumnBreak();
    void    InsertFootnote(const OUString &, bool bEndNote = false, bool bEdit = true );
    void    SplitNode( bool bAutoFormat = false, bool bCheckTableStart = true );
    bool    CanInsert();

    // indexes
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);
    bool    UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);

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
                          SvGlobalName *pName = 0,      // != 0 create object accordingly
                          bool bActivate = true,
                          sal_uInt16 nSlotId = 0);       // SlotId for dialog

    bool    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrameFormat **pFlyFrameFormat = 0 );
    void    LaunchOLEObj( long nVerb = 0 );             // start server
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset ) SAL_OVERRIDE;
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0,
                                  const bool bNoTextFrmPrtAreaChanged = false ) SAL_OVERRIDE;
    virtual void ConnectObj( svt::EmbeddedObjectRef&  xIPObj, const SwRect &rPrt,
                             const SwRect &rFrm ) SAL_OVERRIDE;

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

    OUString        GetCurPageStyle( const bool bCalcFrm = true ) const;

    // change current style using the attributes in effect
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    void    Do( DoType eDoType, sal_uInt16 nCnt = 1 );
    OUString  GetDoString( DoType eDoType ) const;
    OUString  GetRepeatString() const;
    void    GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    // search and replace
    sal_uLong SearchPattern(const com::sun::star::util::SearchOptions& rSearchOpt,
                         bool bSearchInNotes,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FND_IN_BODY,
                         bool bReplace = false );

    sal_uLong SearchTempl  (const OUString &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FND_IN_BODY,
                         const OUString* pReplTempl = 0 );

    sal_uLong SearchAttr   (const SfxItemSet& rFindSet,
                         bool bNoColls,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FND_IN_BODY,
                         const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                         const SfxItemSet* pReplaceSet = 0);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar );

    // action ahead of cursor movement
    // resets selection if applicable, triggers timer and GCAttr()
    void    MoveCrsr( bool bWithSelect = false );

    // update input fields
    bool    StartInputFieldDlg(SwField*, bool bNextButton, vcl::Window* pParentWin = 0, OString* pWindowState = 0);
    // update DropDown fields
    bool    StartDropDownFieldDlg(SwField*, bool bNextButton, OString* pWindowState = 0);

    //"Handler" for changes at DrawView - for controls.
    virtual void DrawSelChanged( ) SAL_OVERRIDE;

    // jump to bookmark and set the "selections-flags" correctly again
    bool GotoMark( const ::sw::mark::IMark* const pMark );
    bool GotoMark( const ::sw::mark::IMark* const pMark, bool bSelect, bool bStart );
    bool GotoMark( const OUString& rName );
    bool GoNextBookmark(); // true when there still was one
    bool GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark);

    bool GotoField( const SwFormatField& rField );

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    bool SelectNextPrevHyperlink( bool bNext = true );

    // determine corresponding SwView
    const SwView&       GetView() const { return m_rView; }
    SwView&             GetView() { return m_rView; }

    // Because nobody else is doing it, here is a ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, OUString* pRet = 0, SbxArray* pArgs = 0 );
    // call into the dark Basic/JavaScript
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bCheckPtr = false, SbxArray* pArgs = 0,
                        const Link<>* pCallBack = 0 );

    // a click at the given field. the cursor is on it.
    // execute the predefined actions.
    void ClickToField( const SwField& rField );
    void ClickToINetAttr( const SwFormatINetFormat& rItem, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    bool ClickToINetGrf( const Point& rDocPt, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    inline bool IsInClickToEdit() const ;

    // if a URL-Button is selected, return its URL; otherwise an empty string
    bool GetURLFromButton( OUString& rURL, OUString& rDescr ) const;

    void NavigatorPaste( const NaviContentBookmark& rBkmk,
                         const sal_uInt16 nAction );

    virtual void ApplyViewOptions( const SwViewOption &rOpt ) SAL_OVERRIDE;
    virtual void SetReadonlyOption( bool bSet ) SAL_OVERRIDE;

    // automatic update of styles
    void AutoUpdateFrame(SwFrameFormat* pFormat, const SfxItemSet& rStyleSet);
    void AutoUpdatePara(SwTextFormatColl* pColl, const SfxItemSet& rStyleSet, SwPaM* pPaM = NULL );

    // link for inserting ranges via Drag&Drop/Clipboard
    DECL_LINK_TYPED( InsertRegionDialog, void*, void );

    // ctor, the first one is a kind of a controlled copy ctor for more views of a document
    SwWrtShell( SwWrtShell&, vcl::Window *pWin, SwView &rShell);
    SwWrtShell( SwDoc& rDoc, vcl::Window *pWin, SwView &rShell,
                const SwViewOption *pViewOpt = 0);
    virtual ~SwWrtShell();

    bool TryRemoveIndent(); // #i23725#

    OUString GetSelDescr() const;

    SwNavigationMgr& GetNavigationMgr() { return m_aNavigationMgr; }

    void addCurrentPosition();
    bool GotoFly( const OUString& rName, FlyCntType eType = FLYCNTTYPE_ALL,
         bool bSelFrame = true );
    bool GotoINetAttr( const SwTextINetFormat& rAttr );
    void GotoOutline( sal_uInt16 nIdx );
    bool GotoOutline( const OUString& rName );
    bool GotoRegion( const OUString& rName );
    bool GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType = 0,
        sal_uInt16 nSeqNo = 0 );
    bool GotoNextTOXBase( const OUString* pName = 0);
    bool GotoTable( const OUString& rName );
    bool GotoFormatField( const SwFormatField& rField );
    const SwRangeRedline* GotoRedline( sal_uInt16 nArrPos, bool bSelect = false);

    void ChangeHeaderOrFooter(const OUString& rStyleName, bool bHeader, bool bOn, bool bShowWarning);
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow ) SAL_OVERRIDE;

private:

    SAL_DLLPRIVATE void  OpenMark();
    SAL_DLLPRIVATE void  CloseMark( bool bOkFlag );

    struct ModeStack
    {
        ModeStack   *pNext;
        bool        bAdd,
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
    } *m_pModeStack;

    // carry cursor along when PageUp / -Down
    enum PageMove
    {
        MV_NO,
        MV_PAGE_UP,
        MV_PAGE_DOWN
    }  m_ePageMove;

    struct CrsrStack
    {
        Point aDocPos;
        CrsrStack *pNext;
        bool bValidCurPos : 1;
        bool bIsFrmSel : 1;
        SwTwips lOffset;

        CrsrStack( bool bValid, bool bFrmSel, const Point &rDocPos,
                    SwTwips lOff, CrsrStack *pN )
            : aDocPos(rDocPos),
            pNext(pN),
            bValidCurPos( bValid ),
            bIsFrmSel( bFrmSel ),
            lOffset(lOff)
        {

        }

    } *m_pCrsrStack;

    SwView  &m_rView;
    SwNavigationMgr m_aNavigationMgr;

    Point   m_aDest;
    bool    m_bDestOnStack;
    bool    HasCrsrStack() const { return 0 != m_pCrsrStack; }
    SAL_DLLPRIVATE bool  PushCrsr(SwTwips lOffset, bool bSelect);
    SAL_DLLPRIVATE bool  PopCrsr(bool bUpdate, bool bSelect = false);

    // take END cursor along when PageUp / -Down
    SAL_DLLPRIVATE bool _SttWrd();
    SAL_DLLPRIVATE bool _EndWrd();
    SAL_DLLPRIVATE bool _NxtWrd();
    SAL_DLLPRIVATE bool _PrvWrd();
    // #i92468#
    SAL_DLLPRIVATE bool _NxtWrdForDelete();
    SAL_DLLPRIVATE bool _PrvWrdForDelete();
    SAL_DLLPRIVATE bool _FwdSentence();
    SAL_DLLPRIVATE bool _BwdSentence();
    bool _FwdPara();
    SAL_DLLPRIVATE bool _BwdPara();

        // selections
    bool    m_bIns            :1;
    bool    m_bInSelect       :1;
    bool    m_bExtMode        :1;
    bool    m_bAddMode        :1;
    bool    m_bBlockMode      :1;
    bool    m_bLayoutMode     :1;
    bool    m_bCopy           :1;
    bool    m_bSelWrd         :1;
    bool    m_bSelLn          :1;
    bool    m_bIsInClickToEdit:1;
    bool    m_bClearMark      :1;     // don't delete selection for ChartAutoPilot
    bool    m_bRetainSelection :1; // Do not remove selections

    Point   m_aStart;
    Link<>  m_aSelTableLink;

    // resets the cursor stack after movement by PageUp/-Down
    SAL_DLLPRIVATE void  _ResetCursorStack();

    using SwCrsrShell::SetCrsr;
    SAL_DLLPRIVATE long  SetCrsr(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  SetCrsrKillSel(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  BeginDrag(const Point *, bool bProp=false );
    SAL_DLLPRIVATE long  DefaultDrag(const Point *, bool bProp=false );
    SAL_DLLPRIVATE long  DefaultEndDrag(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  ExtSelWrd(const Point *, bool bProp=false );
    SAL_DLLPRIVATE long  ExtSelLn(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  BeginFrmDrag(const Point *, bool bProp=false );

    // after SSize/Move of a frame update; Point is destination.
    SAL_DLLPRIVATE long  UpdateLayoutFrm(const Point *, bool bProp=false );

    SAL_DLLPRIVATE long  SttLeaveSelect(const Point *, bool bProp=false );
    SAL_DLLPRIVATE long  AddLeaveSelect(const Point *, bool bProp=false );
    SAL_DLLPRIVATE long  Ignore(const Point *, bool bProp=false );

    SAL_DLLPRIVATE void  LeaveExtSel() { m_bSelWrd = m_bSelLn = false;}

    SAL_DLLPRIVATE bool  GoStart(bool KeepArea = false, bool * = 0,
            bool bSelect = false, bool bDontMoveRegion = false);
    SAL_DLLPRIVATE bool  GoEnd(bool KeepArea = false, bool * = 0);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV,
        BOOKMARK_LAST_LAST_ENTRY
    };

    SAL_DLLPRIVATE bool MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=NULL);
};

inline void SwWrtShell::ResetCursorStack()
{
    if ( HasCrsrStack() )
        _ResetCursorStack();
}

inline void SwWrtShell::SelTableCells(const Link<> &rLink, bool bMark )
{
    SetSelTableCells( true );
    m_bClearMark = bMark;
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
    SELECTFUNC  fnTmp = &SwWrtShell::BeginDrag;
    return m_fnDrag == fnTmp;
#else
    return m_fnDrag == &SwWrtShell::BeginDrag;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
