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

#pragma once

#include <swdllapi.h>
#include <fesh.hxx>
#include <swurl.hxx>
#include <IMark.hxx>
#include "navmgr.hxx"
#include <optional>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <o3tl/typed_flags_set.hxx>
#include <svx/swframetypes.hxx>
#include <vcl/weld.hxx>

#include <doc.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>

namespace vcl { class Window; }
class SbxArray;
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
class SwFieldMgr;
class SfxRequest;
enum class SwLineBreakClear;
class SwContentControl;
enum class SwContentControlType;

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
    TableRow             = 0x040000, // table rows are selected
    TableCol             = 0x080000, // table columns are selected
    All                  = 0x0ffff3,
};
namespace o3tl {
    template<> struct typed_flags<SelectionType> : is_typed_flags<SelectionType, 0x0ffff3> {};
}

/** Used by the UI to modify the document model.

Eg. the Insert() method will take the current cursor position, insert the
string, and take care of undo etc.
*/
class SAL_DLLPUBLIC_RTTI SwWrtShell final : public SwFEShell
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

    typedef tools::Long (SwWrtShell::*SELECTFUNC)(const Point *, bool bProp );
    typedef void (SwWrtShell::*SELECTFUNC2)(const Point *, bool bProp );

    SELECTFUNC2 m_fnDrag      = &SwWrtShell::BeginDrag;
    SELECTFUNC  m_fnSetCursor = &SwWrtShell::SetCursor;
    SELECTFUNC2 m_fnEndDrag   = &SwWrtShell::DefaultEndDrag;
    SELECTFUNC  m_fnKillSel   = &SwWrtShell::Ignore;

public:
    using SwCursorShell::GotoFootnoteAnchor;
    using SwEditShell::Insert;

    tools::Long CallSetCursor(const Point* pPt, bool bProp) { return (this->*m_fnSetCursor)(pPt, bProp); }
    void Drag         (const Point* pPt, bool bProp) { (this->*m_fnDrag)(pPt, bProp); }
    void EndDrag      (const Point* pPt, bool bProp) { (this->*m_fnEndDrag)(pPt, bProp); }
    tools::Long KillSelection(const Point* pPt, bool bProp) { return (this->*m_fnKillSel)(pPt, bProp); }

    bool IsSplitVerticalByDefault() const;
    void SetSplitVerticalByDefault(bool value);

    // reset all selections
    SW_DLLPUBLIC tools::Long ResetSelect( const Point *, bool );

    // resets the cursorstack after movement with PageUp/-Down if a stack is built up
    inline void ResetCursorStack();
    SW_DLLPUBLIC SelectionType GetSelectionType() const;

    bool    IsModePushed() const { return nullptr != m_pModeStack; }
    void    PushMode();
    void    PopMode();

    void    SttSelect();
    SW_DLLPUBLIC void EndSelect();
    bool    IsInSelect() const { return m_bInSelect; }
    void    SetInSelect() { m_bInSelect = true; }
        // is there a text- or frameselection?
    bool    HasSelection() const { return SwCursorShell::HasSelection() ||
                                        IsMultiSelection() || IsSelFrameMode() || IsObjSelected(); }
    bool    Pop(SwCursorShell::PopMode, ::std::optional<SwCallLink>& roLink);
    SW_DLLPUBLIC bool Pop(SwCursorShell::PopMode = SwCursorShell::PopMode::DeleteStack);

    SW_DLLPUBLIC void EnterStdMode();
    bool    IsStdMode() const { return !m_bExtMode && !m_bAddMode && !m_bBlockMode; }

    void AssureStdMode();

    void    EnterExtMode();
    void    LeaveExtMode();
    void    ToggleExtMode();
    bool    IsExtMode() const { return m_bExtMode; }

    void    EnterAddMode();
    void    LeaveAddMode();
    void    ToggleAddMode();
    bool    IsAddMode() const { return m_bAddMode; }

    void    EnterBlockMode();
    void    LeaveBlockMode();
    void    ToggleBlockMode();
    bool    IsBlockMode() const { return m_bBlockMode; }

    void    SetInsMode( bool bOn = true );
    void    ToggleInsMode() { SetInsMode( !m_bIns ); }
    bool    IsInsMode() const { return m_bIns; }
    SW_DLLPUBLIC void SetRedlineFlagsAndCheckInsMode( RedlineFlags eMode );

    SW_DLLPUBLIC void EnterSelFrameMode(const Point *pStartDrag = nullptr);
    SW_DLLPUBLIC void LeaveSelFrameMode();
    bool    IsSelFrameMode() const { return m_bLayoutMode; }
        // reset selection of frames
    SW_DLLPUBLIC void UnSelectFrame();

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
    SW_DLLPUBLIC bool SelWrd(const Point * = nullptr );
    // #i32329# Enhanced selection
    void    SelSentence (const Point *);
    SW_DLLPUBLIC void SelPara     (const Point *);
    SW_DLLPUBLIC void SelAll();

    // basecursortravelling
typedef bool (SwWrtShell::*FNSimpleMove)();
    SW_DLLPUBLIC bool SimpleMove( FNSimpleMove, bool bSelect );

    SW_DLLPUBLIC bool Left( SwCursorSkipMode nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    SW_DLLPUBLIC bool Right( SwCursorSkipMode nMode, bool bSelect,
                            sal_uInt16 nCount, bool bBasicCall, bool bVisual = false );
    SW_DLLPUBLIC bool Up  ( bool bSelect, sal_uInt16 nCount = 1, bool bBasicCall = false );
    SW_DLLPUBLIC bool Down( bool bSelect, sal_uInt16 nCount = 1, bool bBasicCall = false );
    void NxtWrd     ( bool bSelect = false ) { SimpleMove( &SwWrtShell::NxtWrd_, bSelect ); }
    bool PrvWrd     ( bool bSelect = false ) { return SimpleMove( &SwWrtShell::PrvWrd_, bSelect ); }

    bool LeftMargin ( bool bSelect, bool bBasicCall );
    SW_DLLPUBLIC bool RightMargin( bool bSelect, bool bBasicCall );

    SW_DLLPUBLIC bool StartOfSection( bool bSelect = false );
    SW_DLLPUBLIC bool EndOfSection  ( bool bSelect = false );

    SW_DLLPUBLIC bool SttNxtPg   ( bool bSelect = false );
    void SttPrvPg   ( bool bSelect = false );
    void EndNxtPg   ( bool bSelect = false );
    SW_DLLPUBLIC bool EndPrvPg   ( bool bSelect = false );
    SW_DLLPUBLIC bool SttPg      ( bool bSelect = false );
    SW_DLLPUBLIC bool EndPg      ( bool bSelect = false );
    SW_DLLPUBLIC bool SttPara    ( bool bSelect = false );
    SW_DLLPUBLIC void EndPara    ( bool bSelect = false );
    bool FwdPara    ()
                { return SimpleMove( &SwWrtShell::FwdPara_, false/*bSelect*/ ); }
    void BwdPara    ()
                { SimpleMove( &SwWrtShell::BwdPara_, false/*bSelect*/ ); }
    void FwdSentence( bool bSelect = false )
                { SimpleMove( &SwWrtShell::FwdSentence_, bSelect ); }
    void EndSentence( bool bSelect = false )
                { SimpleMove( &SwWrtShell::EndSentence_, bSelect ); }
    void BwdSentence( bool bSelect = false )
                { SimpleMove( &SwWrtShell::BwdSentence_, bSelect ); }

    // #i20126# Enhanced table selection
    SW_DLLPUBLIC bool SelectTableRowCol( const Point& rPt, const Point* pEnd = nullptr, bool bRowDrag = false );
    void SelectTableRow();
    void SelectTableCol();
    void SelectTableCell();

    SW_DLLPUBLIC bool SelectTextAttr( sal_uInt16 nWhich, const SwTextAttr* pAttr = nullptr );

    // per column jumps
    void StartOfColumn      ();
    void EndOfColumn        ();
    void StartOfNextColumn  ();
    void EndOfNextColumn    ();
    void StartOfPrevColumn  ();
    void EndOfPrevColumn    ();

    // set the cursor to page "nPage" at the beginning
    // additionally to an identically named implementation in crsrsh.hxx
    // here all existing selections are being reset before setting the
    // cursor
    SW_DLLPUBLIC bool GotoPage( sal_uInt16 nPage, bool bRecord );

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
    SW_DLLPUBLIC void ChgDBData(const SwDBData& SwDBData);

    // delete
    void    DelToEndOfLine();
    SW_DLLPUBLIC void DelToStartOfLine();
    SW_DLLPUBLIC void DelLine();
    SW_DLLPUBLIC bool DelLeft();

    // also deletes the frame or sets the cursor in the frame when bDelFrame == false
    SW_DLLPUBLIC bool DelRight(bool isReplaceHeuristic = false);
    void    DelToEndOfPara();
    void    DelToStartOfPara();
    SW_DLLPUBLIC bool DelToEndOfSentence();
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
    SW_DLLPUBLIC bool InsertField2(SwField const &, SwPaM* pAnnotationRange = nullptr);
    SW_DLLPUBLIC void Insert(const OUString &);
    // graphic
    void    InsertGraphic( const OUString &rPath, const OUString &rFilter,
                    const Graphic &, SwFlyFrameAttrMgr * = nullptr,
                    RndStdIds nAnchorType = RndStdIds::FLY_AT_PARA);

    void    InsertByWord( const OUString & );
    SW_DLLPUBLIC void InsertPageBreak(const OUString *pPageDesc = nullptr, const ::std::optional<sal_uInt16>& rPgNum = std::nullopt);
    void InsertEnclosingChars(std::u16string_view sStartStr, std::u16string_view sEndStr);
    SW_DLLPUBLIC void InsertLineBreak(std::optional<SwLineBreakClear> oClear = std::nullopt);
    void    InsertColumnBreak();
    SW_DLLPUBLIC void InsertContentControl(SwContentControlType eType);
    SW_DLLPUBLIC void InsertFootnote(const OUString &, bool bEndNote = false, bool bEdit = true );
    SW_DLLPUBLIC void SplitNode( bool bAutoFormat = false );
    bool    CanInsert();

    // indexes
    void    InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = nullptr);
    SW_DLLPUBLIC void UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet = nullptr);

    // numbering and bullets
    /**
       Turns on numbering or bullets.

       @param bNum    true: turn on numbering
                      false: turn on bullets
    */
    void    NumOrBulletOn(bool bNum); // #i29560#
    void    NumOrBulletOff(); // #i29560#
    void    NumOn();
    SW_DLLPUBLIC void BulletOn();

    //OLE
    SW_DLLPUBLIC void InsertObject(     /*SvInPlaceObjectRef *pObj, */       // != 0 for clipboard
                          const svt::EmbeddedObjectRef&,
                          SvGlobalName const *pName,      // != 0 create object accordingly
                          sal_uInt16 nSlotId = 0);       // SlotId for dialog

    bool    InsertOleObject( const svt::EmbeddedObjectRef& xObj, SwFlyFrameFormat **pFlyFrameFormat = nullptr );
    SW_DLLPUBLIC void LaunchOLEObj(sal_Int32 nVerb = css::embed::EmbedVerbs::MS_OLEVERB_PRIMARY); // start server
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

    SW_DLLPUBLIC SwTextFormatColl* GetParaStyle(const OUString &rCollName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SW_DLLPUBLIC SwCharFormat* GetCharStyle(const OUString &rFormatName,
                                    GetStyle eCreate = GETSTYLE_NOCREATE);
    SW_DLLPUBLIC SwFrameFormat* GetTableStyle(std::u16string_view rFormatName);

    void            SetPageStyle(const OUString &rCollName);

    SW_DLLPUBLIC OUString const & GetCurPageStyle() const;

    // change current style using the attributes in effect
    void    QuickUpdateStyle();

    enum DoType { UNDO, REDO, REPEAT };

    enum class FieldDialogPressedButton { NONE, Previous, Next };

    void Do(DoType eDoType, sal_uInt16 nCnt = 1, sal_uInt16 nOffset = 0);
    OUString  GetDoString( DoType eDoType ) const;
    OUString  GetRepeatString() const;
    void    GetDoStrings( DoType eDoType, SfxStringListItem& rStrLstItem ) const;

    // search and replace
    SW_DLLPUBLIC sal_Int32 SearchPattern(const i18nutil::SearchOptions2& rSearchOpt,
                         bool bSearchInNotes,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         bool bReplace = false );

    sal_Int32 SearchTempl  (const OUString &rTempl,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         const OUString* pReplTempl = nullptr );

    sal_Int32 SearchAttr   (const SfxItemSet& rFindSet,
                         bool bNoColls,
                         SwDocPositions eStart, SwDocPositions eEnd,
                         FindRanges eFlags = FindRanges::InBody,
                         const i18nutil::SearchOptions2* pSearchOpt = nullptr,
                         const SfxItemSet* pReplaceSet = nullptr);

    SW_DLLPUBLIC void AutoCorrect( SvxAutoCorrect& rACorr, sal_Unicode cChar );

    // action ahead of cursor movement
    // resets selection if applicable, triggers timer and GCAttr()
    void    MoveCursor( bool bWithSelect = false );

    // update input fields
    bool    StartInputFieldDlg(SwField*, bool bPrevButton, bool bNextButton, weld::Widget* pParentWin, FieldDialogPressedButton* pPressedButton = nullptr);
    // update DropDown fields
    bool    StartDropDownFieldDlg(SwField*, bool bPrevButton, bool bNextButton, weld::Widget* pParentWin, FieldDialogPressedButton* pPressedButton = nullptr);

    //"Handler" for changes at DrawView - for controls.
    virtual void DrawSelChanged( ) override;

    // jump to bookmark and set the "selections-flags" correctly again
    SW_DLLPUBLIC void GotoMark( const ::sw::mark::IMark* const pMark );
    bool GotoMark( const ::sw::mark::IMark* const pMark, bool bSelect );
    SW_DLLPUBLIC void GotoMark( const OUString& rName );
    bool GoNextBookmark(); // true when there still was one
    bool GoPrevBookmark();

    bool GotoFieldmark(::sw::mark::IFieldmark const * const pMark, bool completeSelection = false);

    SW_DLLPUBLIC bool GotoField( const SwFormatField& rField );

    /** @param bOnlyRefresh:
     *         false: run default actions (e.g. toggle checkbox, remove placeholder content)
     *         true: do not alter the content control, just refresh the doc model
     */
    SW_DLLPUBLIC bool GotoContentControl(const SwFormatContentControl& rContentControl,
                            bool bOnlyRefresh = false);

    // jump to the next / previous hyperlink - inside text and also
    // on graphics
    void SelectNextPrevHyperlink( bool bNext );

    // determine corresponding SwView
    const SwView&       GetView() const { return m_rView; }
    SwView&             GetView() { return m_rView; }

    // Because nobody else is doing it, here is an ExecMacro()
    void ExecMacro( const SvxMacro& rMacro, OUString* pRet = nullptr, SbxArray* pArgs = nullptr );
    // call into the dark Basic/JavaScript
    sal_uInt16 CallEvent( SvMacroItemId nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bCheckPtr = false );

    // a click at the given field. the cursor is on it.
    // execute the predefined actions.
    void ClickToField( const SwField& rField, bool bExecHyperlinks );
    void ClickToINetAttr( const SwFormatINetFormat& rItem, LoadUrlFlags nFilter = LoadUrlFlags::NONE );
    bool ClickToINetGrf( const Point& rDocPt, LoadUrlFlags nFilter );
    inline bool IsInClickToEdit() const ;

    // if a URL-Button is selected, return its URL; otherwise an empty string
    bool GetURLFromButton( OUString& rURL, OUString& rDescr ) const;

    void NavigatorPaste(const NaviContentBookmark& rBkmk);

    SW_DLLPUBLIC virtual void ApplyViewOptions( const SwViewOption &rOpt ) override;
    SW_DLLPUBLIC virtual void SetReadonlyOption( bool bSet ) override;

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
    SW_DLLPUBLIC bool GotoFly( const OUString& rName, FlyCntType eType = FLYCNTTYPE_ALL,
         bool bSelFrame = true );
    bool GotoINetAttr( const SwTextINetFormat& rAttr );
    void GotoOutline( SwOutlineNodes::size_type nIdx );
    bool GotoOutline( const OUString& rName );
    bool GotoRegion( std::u16string_view rName );
    bool GotoRefMark( const OUString& rRefMark, sal_uInt16 nSubType = 0,
        sal_uInt16 nSeqNo = 0, sal_uInt16 nFlags = 0 );
    SW_DLLPUBLIC bool GotoNextTOXBase( const OUString* pName = nullptr);
    SW_DLLPUBLIC bool GotoTable( const OUString& rName );
    void GotoFormatField( const SwFormatField& rField );
    const SwRangeRedline* GotoRedline( SwRedlineTable::size_type nArrPos, bool bSelect);
    SW_DLLPUBLIC bool GotoDrawingObject(std::u16string_view rName);
    void GotoFootnoteAnchor(const SwTextFootnote& rTextFootnote);
    SW_DLLPUBLIC void ChangeHeaderOrFooter(std::u16string_view rStyleName, bool bHeader, bool bOn, bool bShowWarning);
    virtual void SetShowHeaderFooterSeparator( FrameControlType eControl, bool bShow ) override;

    /// Inserts a new annotation/comment at the current cursor position / selection.
    void InsertPostIt(SwFieldMgr& rFieldMgr, const SfxRequest& rReq);

    bool IsOutlineContentVisible(const size_t nPos);
    void MakeOutlineContentVisible(const size_t nPos, bool bMakeVisible = true, bool bSetAttrOutlineVisibility = true);
    void MakeAllFoldedOutlineContentVisible(bool bMakeVisible = true);
    void InvalidateOutlineContentVisibility();
    bool GetAttrOutlineContentVisible(const size_t nPos) const;

    void MakeOutlineLevelsVisible(const int nLevel);

    bool HasFoldedOutlineContentSelected() const;
    virtual void InfoReadOnlyDialog(bool bAsync) const override;
    virtual bool WarnHiddenSectionDialog() const override;
    virtual bool WarnSwitchToDesignModeDialog() const override;

    std::optional<OString> getLOKPayload(int nType, int nViewId) const;

private:

    void  OpenMark();
    void  CloseMark( bool bOkFlag );

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
        bool bValidCurPos : 1;
        bool bIsFrameSel : 1;
        SwTwips lOffset;

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
    bool  PushCursor(SwTwips lOffset, bool bSelect);
    bool  PopCursor(bool bUpdate, bool bSelect = false);

    // take END cursor along when PageUp / -Down
    void SttWrd();
    void EndWrd();
    bool NxtWrd_();
    bool PrvWrd_();
    // #i92468#
    bool NxtWrdForDelete();
    bool PrvWrdForDelete();
    bool FwdSentence_();
    bool EndSentence_();
    bool BwdSentence_();
    SW_DLLPUBLIC bool FwdPara_();
    bool BwdPara_();

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
    void  ResetCursorStack_();

    using SwCursorShell::SetCursor;
    tools::Long  SetCursor(const Point *, bool bProp=false );

    tools::Long  SetCursorKillSel(const Point *, bool bProp );

    void  BeginDrag(const Point *, bool bProp );
    void  DefaultDrag(const Point *, bool bProp );
    void  DefaultEndDrag(const Point *, bool bProp );

    void  ExtSelWrd(const Point *, bool bProp );
    void  ExtSelLn(const Point *, bool bProp );

     void  BeginFrameDrag(const Point *, bool bProp );

    // after SSize/Move of a frame update; Point is destination.
    void  UpdateLayoutFrame(const Point *, bool bProp );

    void  SttLeaveSelect();
    void  AddLeaveSelect();
    tools::Long  Ignore(const Point *, bool bProp );

    void  LeaveExtSel() { m_bSelWrd = m_bSelLn = false;}

    bool  GoStart(bool KeepArea, bool *,
            bool bSelect, bool bDontMoveRegion = false);
    bool  GoEnd(bool KeepArea = false, const bool * = nullptr);

    enum BookMarkMove
    {
        BOOKMARK_INDEX,
        BOOKMARK_NEXT,
        BOOKMARK_PREV
    };

    bool MoveBookMark(BookMarkMove eFuncId, const ::sw::mark::IMark* const pMark=nullptr);
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

class MakeAllOutlineContentTemporarilyVisible
{
private:
    SwWrtShell* m_pWrtSh = nullptr;
    bool m_bDone = false;
    bool m_bScrollToCursor = false;
public:
    static sal_uInt32 nLock;
    MakeAllOutlineContentTemporarilyVisible(SwDoc* pDoc, bool bScrollToCursor = false)
    {
        ++nLock;
        if (nLock > 1)
            return;
        if (SwDocShell* pDocSh = pDoc->GetDocShell())
            if ((m_pWrtSh = pDocSh->GetWrtShell()) && m_pWrtSh->GetViewOptions() &&
                    m_pWrtSh->GetViewOptions()->IsShowOutlineContentVisibilityButton())
            {
                m_pWrtSh->LockView(true);
                m_pWrtSh->LockPaint(LockPaintReason::OutlineFolding);
                m_pWrtSh->MakeAllFoldedOutlineContentVisible();
                m_bScrollToCursor = bScrollToCursor;
                m_bDone = true;
            }
    }

    ~MakeAllOutlineContentTemporarilyVisible() COVERITY_NOEXCEPT_FALSE
    {
        --nLock;
        if (nLock > 0)
            return;
        if (m_bDone && m_pWrtSh)
        {
            m_pWrtSh->MakeAllFoldedOutlineContentVisible(false);
            m_pWrtSh->UnlockPaint();
            m_pWrtSh->LockView(false);
            if (m_bScrollToCursor)
                m_pWrtSh->UpdateCursor(SwCursorShell::SCROLLWIN);
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
