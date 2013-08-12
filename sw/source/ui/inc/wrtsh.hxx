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
#ifndef _WRTSH_HXX
#define _WRTSH_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include "swdllapi.h"
#include <fesh.hxx>
#include <sortopt.hxx>
#include <swurl.hxx>
#include <IMark.hxx>
#include "navmgr.hxx"

class Window;
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
class SwFmtINetFmt;
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


class SW_DLLPUBLIC SwWrtShell: public SwFEShell
{
private:
    using SwCrsrShell::Left;
    using SwCrsrShell::Right;
    using SwCrsrShell::Up;
    using SwCrsrShell::Down;
    using SwCrsrShell::LeftMargin;
    using SwCrsrShell::RightMargin;
    using SwCrsrShell::SelectTxtAttr;
    using SwCrsrShell::GotoPage;
    using SwFEShell::InsertObject;
    using SwEditShell::AutoCorrect;
    using SwCrsrShell::GotoMark;

    typedef long (SwWrtShell::*SELECTFUNC)(const Point *, sal_Bool bProp );

    SELECTFUNC  fnDrag;
    SELECTFUNC  fnSetCrsr;
    SELECTFUNC  fnEndDrag;
    SELECTFUNC  fnKillSel;

public:

    using SwEditShell::Insert;

    long SetCursor    (const Point* pPt, bool bProp) { return (this->*fnSetCrsr)(pPt, bProp); }
    long Drag         (const Point* pPt, bool bProp) { return (this->*fnDrag)(pPt, bProp); }
    long EndDrag      (const Point* pPt, bool bProp) { return (this->*fnEndDrag)(pPt, bProp); }
    long KillSelection(const Point* pPt, bool bProp) { return (this->*fnKillSel)(pPt, bProp); }

    // reset all selections
    long ResetSelect( const Point *, sal_Bool );

    // resets the cursorstack after movement with PageUp/-Down if a stack is built up
    inline void ResetCursorStack();
    SelectionType   GetSelectionType() const;

    bool    IsModePushed() const { return 0 != pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    void    EndSelect();
    sal_Bool    IsInSelect() const { return bInSelect; }
    void    SetInSelect(sal_Bool bSel = sal_True) { bInSelect = bSel; }
        // is there a text- or frameselection?
    sal_Bool    HasSelection() const { return SwCrsrShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrmMode() || IsObjSelected(); }
    sal_Bool Pop( sal_Bool bOldCrsr = sal_True );

    void    EnterStdMode();
    bool    IsStdMode() const { return !bExtMode && !bAddMode && !bBlockMode; }

    void    EnterExtMode();
    void    LeaveExtMode();
    long    ToggleExtMode();
    sal_Bool    IsExtMode() const { return bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    long    ToggleAddMode();
    sal_Bool    IsAddMode() const { return bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    long    ToggleBlockMode();
    sal_Bool    IsBlockMode() const { return bBlockMode; }

    void    SetInsMode( sal_Bool bOn = sal_True );
    void    ToggleInsMode() { SetInsMode( !bIns ); }
    sal_Bool    IsInsMode() const { return bIns; }
    void    SetRedlineModeAndCheckInsMode( sal_uInt16 eMode );

    void    EnterSelFrmMode(const Point *pStartDrag = 0);
    void    LeaveSelFrmMode();
    sal_Bool    IsSelFrmMode() const { return bLayoutMode; }
        // reset selection of frames
    void    UnSelectFrm();

    void    Invalidate();

    // select table cells for editing of formulas in the ribbonbar
    inline void SelTblCells( const Link &rLink, sal_Bool bMark = sal_True );
    inline void EndSelTblCells();

    // leave per word or per line selection mode. Is usually called in MB-Up.
    bool    IsExtSel() const { return bSelWrd || bSelLn; }

    // query whether the active fnDrag pointer is set to BeginDrag
    // is needed for MouseMove to work around bugs 55592/55931
    inline bool Is_FnDragEQBeginDrag() const;

    // base requests
    sal_Bool    IsInWrd()           { return IsInWord(); }
    sal_Bool    IsSttWrd()          { return IsStartWord(); }
    sal_Bool    IsEndWrd();
    bool    IsSttOfPara() const { return IsSttPara(); }
    bool    IsEndOfPara() const { return IsEndPara(); }

    // select word / sentense
    sal_Bool    SelNearestWrd();
    sal_Bool    SelWrd      (const Point * = 0, sal_Bool bProp=sal_False );
    // #i32329# Enhanced selection
    void    SelSentence (const Point * = 0, sal_Bool bProp=sal_False );
    void    SelPara     (const Point * = 0, sal_Bool bProp=sal_False );
    long    SelAll();

    // basecursortravelling
typedef sal_Bool (SwWrtShell:: *FNSimpleMove)();
    sal_Bool SimpleMove( FNSimpleMove, sal_Bool bSelect );

    sal_Bool Left       ( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual = sal_False );
    sal_Bool Right      ( sal_uInt16 nMode, sal_Bool bSelect,
                            sal_uInt16 nCount, sal_Bool bBasicCall, sal_Bool bVisual = sal_False );
    sal_Bool Up         ( sal_Bool bSelect = sal_False, sal_uInt16 nCount = 1,
                            sal_Bool bBasicCall = sal_False );
    sal_Bool Down       ( sal_Bool bSelect = sal_False, sal_uInt16 nCount = 1,
                            sal_Bool bBasicCall = sal_False );
    sal_Bool NxtWrd     ( sal_Bool bSelect = sal_False ) { return SimpleMove( &SwWrtShell::_NxtWrd, bSelect ); }
    sal_Bool PrvWrd     ( sal_Bool bSelect = sal_False ) { return SimpleMove( &SwWrtShell::_PrvWrd, bSelect ); }

    sal_Bool LeftMargin ( sal_Bool bSelect, sal_Bool bBasicCall );
    sal_Bool RightMargin( sal_Bool bSelect, sal_Bool bBasicCall );

    sal_Bool SttDoc     ( sal_Bool bSelect = sal_False );
    sal_Bool EndDoc     ( sal_Bool bSelect = sal_False );

    sal_Bool SttNxtPg   ( sal_Bool bSelect = sal_False );
    sal_Bool SttPrvPg   ( sal_Bool bSelect = sal_False );
    sal_Bool EndNxtPg   ( sal_Bool bSelect = sal_False );
    sal_Bool EndPrvPg   ( sal_Bool bSelect = sal_False );
    sal_Bool SttPg      ( sal_Bool bSelect = sal_False );
    sal_Bool EndPg      ( sal_Bool bSelect = sal_False );
    sal_Bool SttPara    ( sal_Bool bSelect = sal_False );
    sal_Bool EndPara    ( sal_Bool bSelect = sal_False );
    sal_Bool FwdPara    ( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_FwdPara, bSelect ); }
    sal_Bool BwdPara    ( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_BwdPara, bSelect ); }
    sal_Bool FwdSentence( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_FwdSentence, bSelect ); }
    sal_Bool BwdSentence( sal_Bool bSelect = sal_False )
                { return SimpleMove( &SwWrtShell::_BwdSentence, bSelect ); }

    // #i20126# Enhanced table selection
    sal_Bool SelectTableRowCol( const Point& rPt, const Point* pEnd = 0, bool bRowDrag = false );
    sal_Bool SelectTableRow();
    sal_Bool SelectTableCol();
    sal_Bool SelectTableCell();

    sal_Bool SelectTxtAttr( sal_uInt16 nWhich, const SwTxtAttr* pAttr = 0 );

    // per column jumps
    sal_Bool StartOfColumn      ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfColumn        ( sal_Bool bSelect = sal_False );
    sal_Bool StartOfNextColumn  ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfNextColumn    ( sal_Bool bSelect = sal_False );
    sal_Bool StartOfPrevColumn  ( sal_Bool bSelect = sal_False );
    sal_Bool EndOfPrevColumn    ( sal_Bool bSelect = sal_False );

    // set the cursor to page "nPage" at the beginning
    // additionally to a identically named implementation in crsrsh.hxx
    // here all existing selections are being reset before setting the
    // cursor
    sal_Bool    GotoPage( sal_uInt16 nPage, sal_Bool bRecord );

    // setting the cursor; remember the old position for turning back
    DECL_LINK( ExecFlyMac, void * );

    sal_Bool    PageCrsr(SwTwips lOffset, sal_Bool bSelect);

    // update fields
    void    UpdateInputFlds( SwInputFieldList* pLst = 0, sal_Bool bOnlyInSel = sal_False );

    void    NoEdit(bool bHideCrsr = true);
    void    Edit();

    sal_Bool IsRetainSelection() const { return mbRetainSelection; }
    void SetRetainSelection( sal_Bool bRet ) { mbRetainSelection = bRet; }

    // change current data base and notify
    void ChgDBData(const SwDBData& SwDBData);

    // delete
    long    DelToEndOfLine();
    long    DelToStartOfLine();
    long    DelLine();
    long    DelLeft();

    // also deletes the frame or sets the cursor in the frame when bDelFrm == sal_False
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
    int     IntelligentCut(int nSelectionType, sal_Bool bCut = sal_True);

    // edit
    void    Insert(SwField &, SwPaM *pCommentRange = NULL);
    void    Insert(const String &);
    // graphic
    void    Insert( const String &rPath, const String &rFilter,
                    const Graphic &, SwFlyFrmAttrMgr * = 0,
                    sal_Bool bRule = sal_False );

    void    InsertByWord( const String & );
    void    InsertPageBreak(const String *pPageDesc = 0, sal_uInt16 nPgNum = 0 );
    void    InsertLineBreak();
    void    InsertColumnBreak();
    void    InsertFootnote(const String &, sal_Bool bEndNote = sal_False, sal_Bool bEdit = sal_True );
    void    SplitNode( sal_Bool bAutoFormat = sal_False, sal_Bool bCheckTableStart = sal_True );
    bool    CanInsert();

    // indexes
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);
    sal_Bool    UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = 0);

    // numbering and bullets
    /**
       Turns on numbering or bullets.

       @param bNum    sal_True: turn on numbering
                      sal_False: turn on bullets
    */
    void    NumOrBulletOn(sal_Bool bNum); // #i29560#
    void    NumOrBulletOff(); // #i29560#
    void    NumOn();
    void    BulletOn();

    //OLE
    void    InsertObject(     /*SvInPlaceObjectRef *pObj, */       // != 0 for clipboard
                          const svt::EmbeddedObjectRef&,
                          SvGlobalName *pName = 0,      // != 0 create object accordingly
                          sal_Bool bActivate = sal_True,
                          sal_uInt16 nSlotId = 0);       // SlotId for dialog

    sal_Bool    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrmFmt **pFlyFrmFmt = 0 );
    void    LaunchOLEObj( long nVerb = 0 );             // start server
    virtual void MoveObjectIfActive( svt::EmbeddedObjectRef& xObj, const Point& rOffset );
    virtual void CalcAndSetScale( svt::EmbeddedObjectRef& xObj,
                                  const SwRect *pFlyPrtRect = 0,
                                  const SwRect *pFlyFrmRect = 0,
                                  const bool bNoTxtFrmPrtAreaChanged = false );
    virtual void ConnectObj( svt::EmbeddedObjectRef&  xIPObj, const SwRect &rPrt,
                             const SwRect &rFrm );

    // styles and formats

    // enum tells when should happen when the style was not found
    enum GetStyle { GETSTYLE_NOCREATE,          // create none
                    GETSTYLE_CREATESOME,        // if on PoolId create mapt
                    GETSTYLE_CREATEANY };       // return standard if applicable

    SwTxtFmtColl*   GetParaStyle(const String &rCollName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwCharFmt*      GetCharStyle(const String &rFmtName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SwFrmFmt*       GetTblStyle(const OUString &rFmtName);

    void    SetPageStyle(const String &rCollName);

    String  GetCurPageStyle( const sal_Bool bCalcFrm = sal_True ) const;

    // change current style using the attributes in effect
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    void    Do( DoType eDoType, sal_uInt16 nCnt = 1 );
    String  GetDoString( DoType eDoType ) const;
    String  GetRepeatString() const;
    sal_uInt16  GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    // search and replace
    sal_uLong SearchPattern(const com::sun::star::util::SearchOptions& rSearchOpt,
                         sal_Bool bSearchInNotes,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         int bReplace = sal_False );

    sal_uLong SearchTempl  (const String &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const String* pReplTempl = 0 );

    sal_uLong SearchAttr   (const SfxItemSet& rFindSet,
                         sal_Bool bNoColls,
                         SwDocPositions eStart,SwDocPositions eEnde,
                         FindRanges eFlags = FND_IN_BODY,
                         const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                         const SfxItemSet* pReplaceSet = 0);

    void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar = ' ' );

    // action ahead of cursor movement
    // resets selection if applicable, triggers timer and GCAttr()
    void    MoveCrsr( sal_Bool bWithSelect = sal_False );

    // update input fields
    sal_Bool    StartInputFldDlg(SwField*, sal_Bool bNextButton, Window* pParentWin = 0, OString* pWindowState = 0);
    // update DropDown fields
    sal_Bool    StartDropDownFldDlg(SwField*, sal_Bool bNextButton, OString* pWindowState = 0);

    //"Handler" for changes at DrawView - for controls.
    virtual void DrawSelChanged( );

    // jump to bookmark and set the "selctions-flags" correctly again
    sal_Bool GotoMark( const ::sw::mark::IMark* const pMark );
    sal_Bool GotoMark( const ::sw::mark::IMark* const pMark, sal_Bool bSelect, sal_Bool bStart );
    sal_Bool GotoMark( const OUString& rName );
    sal_Bool GoNextBookmark(); // sal_True when there still was one
    sal_Bool GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark);

    sal_Bool GotoField( const SwFmtFld& rFld );

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    bool SelectNextPrevHyperlink( bool bNext = true );

    // determine corresponding SwView
    const SwView&       GetView() const { return rView; }
    SwView&             GetView() { return rView; }

    // Because nobody else is doing it, here is a ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // call into the dark Basic/JavaScript
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bCheckPtr = false, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    // a click at the given field. the cursor is on it.
    // execute the predefined actions.
    void ClickToField( const SwField& rFld );
    void ClickToINetAttr( const SwFmtINetFmt& rItem, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    bool ClickToINetGrf( const Point& rDocPt, sal_uInt16 nFilter = URLLOAD_NOFILTER );
    inline bool IsInClickToEdit() const ;

    // if a URL-Button is selected, return its URL; otherwise an empty string
    bool GetURLFromButton( String& rURL, String& rDescr ) const;

    void NavigatorPaste( const NaviContentBookmark& rBkmk,
                         const sal_uInt16 nAction );

    virtual void ApplyViewOptions( const SwViewOption &rOpt );
    virtual void SetReadonlyOption( sal_Bool bSet );

    // automatic update of styles
    void AutoUpdateFrame(SwFrmFmt* pFmt, const SfxItemSet& rStyleSet);
    void AutoUpdatePara(SwTxtFmtColl* pColl, const SfxItemSet& rStyleSet, SwPaM* pPaM = NULL );

    // link for inserting ranges via Drag&Drop/Clipboard
    DECL_STATIC_LINK( SwWrtShell, InsertRegionDialog, SwSectionData* );


    // ctor, the first one is a kind of a controlled copy ctor for more views of a document
    SwWrtShell( SwWrtShell&, Window *pWin, SwView &rShell);
    SwWrtShell( SwDoc& rDoc, Window *pWin, SwView &rShell,
                const SwViewOption *pViewOpt = 0);
    virtual ~SwWrtShell();

    bool TryRemoveIndent(); // #i23725#

    String GetSelDescr() const;

    SwNavigationMgr& GetNavigationMgr();
    void addCurrentPosition();
    sal_Bool GotoFly( const String& rName, FlyCntType eType = FLYCNTTYPE_ALL,
         sal_Bool bSelFrame = sal_True );
    bool GotoINetAttr( const SwTxtINetFmt& rAttr );
    void GotoOutline( sal_uInt16 nIdx );
    bool GotoOutline( const String& rName );
    bool GotoRegion( const String& rName );
    sal_Bool GotoRefMark( const String& rRefMark, sal_uInt16 nSubType = 0,
        sal_uInt16 nSeqNo = 0 );
    sal_Bool GotoNextTOXBase( const String* pName = 0);
    bool GotoTable( const String& rName );
    sal_Bool GotoFld( const SwFmtFld& rFld );
    const SwRedline* GotoRedline( sal_uInt16 nArrPos, sal_Bool bSelect = sal_False);

    void ChangeHeaderOrFooter(const String& rStyleName, sal_Bool bHeader, sal_Bool bOn, sal_Bool bShowWarning);
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow );

private:

    SW_DLLPRIVATE void  OpenMark();
    SW_DLLPRIVATE void  CloseMark( bool bOkFlag );

    SW_DLLPRIVATE String    GetWrdDelim();
    SW_DLLPRIVATE String    GetSDelim();
    SW_DLLPRIVATE String    GetBothDelim();

    struct ModeStack
    {
        ModeStack   *pNext;
        sal_Bool        bAdd,
                    bBlock,
                    bExt,
                    bIns;
        ModeStack(ModeStack *pNextMode, sal_Bool _bIns, sal_Bool _bExt, sal_Bool _bAdd, sal_Bool _bBlock):
            pNext(pNextMode),
            bAdd(_bAdd),
            bBlock(_bBlock),
            bExt(_bExt),
            bIns(_bIns)
             {}
    } *pModeStack;

    // carry cursor along when PageUp / -Down
    enum PageMove
    {
        MV_NO,
        MV_PAGE_UP,
        MV_PAGE_DOWN
    }  ePageMove;

    struct CrsrStack
    {
        Point aDocPos;
        CrsrStack *pNext;
        sal_Bool bValidCurPos : 1;
        sal_Bool bIsFrmSel : 1;
        SwTwips lOffset;

        CrsrStack( sal_Bool bValid, sal_Bool bFrmSel, const Point &rDocPos,
                    SwTwips lOff, CrsrStack *pN )
            : aDocPos(rDocPos),
            pNext(pN),
            bValidCurPos( bValid ),
            bIsFrmSel( bFrmSel ),
            lOffset(lOff)
        {


        }

    } *pCrsrStack;

    SwView  &rView;
    SwNavigationMgr aNavigationMgr;

    Point   aDest;
    bool    bDestOnStack;
    bool    HasCrsrStack() const { return 0 != pCrsrStack; }
    SW_DLLPRIVATE sal_Bool  PushCrsr(SwTwips lOffset, sal_Bool bSelect);
    SW_DLLPRIVATE sal_Bool  PopCrsr(sal_Bool bUpdate, sal_Bool bSelect = sal_False);

    // take END cursor along when PageUp / -Down
    SW_DLLPRIVATE bool _SttWrd();
    SW_DLLPRIVATE bool _EndWrd();
    SW_DLLPRIVATE sal_Bool _NxtWrd();
    SW_DLLPRIVATE sal_Bool _PrvWrd();
    // #i92468#
    SW_DLLPRIVATE sal_Bool _NxtWrdForDelete();
    SW_DLLPRIVATE sal_Bool _PrvWrdForDelete();
    SW_DLLPRIVATE sal_Bool _FwdSentence();
    SW_DLLPRIVATE sal_Bool _BwdSentence();
    sal_Bool _FwdPara();
    SW_DLLPRIVATE sal_Bool _BwdPara();

        // selections
    sal_Bool    bIns            :1;
    sal_Bool    bInSelect       :1;
    sal_Bool    bExtMode        :1;
    sal_Bool    bAddMode        :1;
    sal_Bool    bBlockMode      :1;
    sal_Bool    bLayoutMode     :1;
    sal_Bool    bCopy           :1;
    sal_Bool    bSelWrd         :1;
    sal_Bool    bSelLn          :1;
    bool    bIsInClickToEdit:1;
    sal_Bool    bClearMark      :1;     // don't delete selection for ChartAutoPilot
    sal_Bool    mbRetainSelection :1; // Do not remove selections

    Point   aStart;
    Link    aSelTblLink;

    // resets the cursor stack after movement by PageUp/-Down
    SW_DLLPRIVATE void  _ResetCursorStack();

    SW_DLLPRIVATE void  SttDragDrop(Timer *);

    using SwCrsrShell::SetCrsr;
    SW_DLLPRIVATE long  SetCrsr(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  SetCrsrKillSel(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  StdSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  BeginDrag(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  Drag(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  EndDrag(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  ExtSelWrd(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  ExtSelLn(const Point *, sal_Bool bProp=sal_False );

    // move text from Drag and Drop; Point is destination for all selections.
    SW_DLLPRIVATE long  MoveText(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  BeginFrmDrag(const Point *, sal_Bool bProp=sal_False );

    // after SSize/Move of a frame update; Point is destination.
    SW_DLLPRIVATE long  UpdateLayoutFrm(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE long  SttLeaveSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  AddLeaveSelect(const Point *, sal_Bool bProp=sal_False );
    SW_DLLPRIVATE long  Ignore(const Point *, sal_Bool bProp=sal_False );

    SW_DLLPRIVATE void  LeaveExtSel() { bSelWrd = bSelLn = sal_False;}

    SW_DLLPRIVATE sal_Bool  GoStart(sal_Bool KeepArea = sal_False, sal_Bool * = 0,
            sal_Bool bSelect = sal_False, sal_Bool bDontMoveRegion = sal_False);
    SW_DLLPRIVATE sal_Bool  GoEnd(sal_Bool KeepArea = sal_False, sal_Bool * = 0);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV,
        BOOKMARK_LAST_LAST_ENTRY
    };

    SW_DLLPRIVATE sal_Bool MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=NULL);
};

inline void SwWrtShell::ResetCursorStack()
{
    if ( HasCrsrStack() )
        _ResetCursorStack();
}

inline void SwWrtShell::SelTblCells(const Link &rLink, sal_Bool bMark )
{
    SetSelTblCells( sal_True );
    bClearMark = bMark;
    aSelTblLink = rLink;
}
inline void SwWrtShell::EndSelTblCells()
{
    SetSelTblCells( sal_False );
    bClearMark = sal_True;
}

inline bool SwWrtShell::IsInClickToEdit() const { return bIsInClickToEdit; }

inline bool SwWrtShell::Is_FnDragEQBeginDrag() const
{
#ifdef __GNUC__
    SELECTFUNC  fnTmp = &SwWrtShell::BeginDrag;
    return fnDrag == fnTmp;
#else
    return fnDrag == &SwWrtShell::BeginDrag;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
