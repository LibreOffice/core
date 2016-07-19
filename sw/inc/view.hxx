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
#ifndef INCLUDED_SW_INC_VIEW_HXX
#define INCLUDED_SW_INC_VIEW_HXX

#include <vcl/timer.hxx>
#include <vcl/field.hxx>
#include <vcl/floatwin.hxx>
#include <svtools/htmlcfg.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/ruler.hxx>
#include <svx/svxids.hrc>
#include <svx/fmshell.hxx>
#include <editeng/editstat.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <shellid.hxx>
#include <IMark.hxx>

class Button;
class ImageButton;
class SwTextFormatColl;
class SwPageDesc;
class SwFrameFormat;
class SwCharFormat;
class SwNumRule;
class SwGlossaryHdl;
class SwDrawBase;
class SvxRuler;
class SvxLRSpaceItem;
class SwDocShell;
class SwScrollbar;
class SvBorder;
class Ruler;
class SvxSearchItem;
class SearchAttrItemList;
class SvxSearchDialog;
class SdrView;
class Dialog;
class SdrObject;
class SdrPageView;
class SwHlpImageButton;
class SwView;
class SwEditWin;
class SwWrtShell;
class SwView_Impl;
struct SwSearchOptions;
class CommandEvent;
class InsCaptionOpt;
class SvGlobalName;
class SvtAccessibilityOptions;
class SwTransferable;
class SwMailMergeConfigItem;
class SwTextNode; // #i23726#
class SwFormatClipboard;
struct SwConversionArgs;
class Graphic;
class GraphicFilter;
class SwPostItMgr;
enum class SotExchangeDest;
class SwCursorShell;
enum class SvxSearchCmd;

namespace com{ namespace sun { namespace star {
    namespace view{ class XSelectionSupplier; }
}}}
namespace sfx2 { class FileDialogHelper; }

const long nLeftOfst = -370;
const long nScrollX  =   30;
const long nScrollY  =   30;

#define MINZOOM 20
#define MAXZOOM 600

#define MAX_MARKS 5

#define CHILDWIN_LABEL      1
#define CHILDWIN_MAILMERGE  2

enum ShellModes
{
    SHELL_MODE_TEXT,
    SHELL_MODE_FRAME,
    SHELL_MODE_GRAPHIC,
    SHELL_MODE_OBJECT,
    SHELL_MODE_DRAW,
    SHELL_MODE_DRAW_CTRL,
    SHELL_MODE_DRAW_FORM,
    SHELL_MODE_DRAWTEXT,
    SHELL_MODE_BEZIER,
    SHELL_MODE_LIST_TEXT,
    SHELL_MODE_TABLE_TEXT,
    SHELL_MODE_TABLE_LIST_TEXT,
    SHELL_MODE_MEDIA,
    SHELL_MODE_EXTRUDED_CUSTOMSHAPE,
    SHELL_MODE_FONTWORK,
    SHELL_MODE_POSTIT,
    SHELL_MODE_NAVIGATION
};

// apply a template
struct SwApplyTemplate
{
    union
    {
        SwTextFormatColl* pTextColl;
        SwPageDesc*   pPageDesc;
        SwFrameFormat*     pFrameFormat;
        SwCharFormat*    pCharFormat;
        SwNumRule*    pNumRule;
    } aColl;

    int eType;
    sal_uInt16 nColor;
    SwFormatClipboard* m_pFormatClipboard;
    size_t nUndo;     //< The initial undo stack depth.

    SwApplyTemplate() :
        eType(0),
        nColor(0),
        m_pFormatClipboard(nullptr),
        nUndo(0)
    {
        aColl.pTextColl = nullptr;
    }
};

// view of a document
class SW_DLLPUBLIC SwView: public SfxViewShell
{
    friend class SwHHCWrapper;
    friend class SwHyphWrapper;
    friend class SwView_Impl;
    friend class SwClipboardChangeListener;

    // search & replace
    static SvxSearchItem           *m_pSrchItem;

    static sal_uInt16       m_nMoveType; // for buttons below the scrollbar (viewmdi)
    static sal_Int32        m_nActMark; // current jump mark for unknown mark

    static bool             m_bExtra;
    static bool             m_bFound;
    static bool             m_bJustOpened;

    static SearchAttrItemList* m_pSrchList;
    static SearchAttrItemList* m_pReplList;

    SvxHtmlOptions      m_aHTMLOpt;
    Timer               m_aTimer;         // for delayed ChgLnks during an action
    OUString            m_sSwViewData,
    //and the new cursor position if the user double click in the PagePreview
                        m_sNewCursorPos;
    // to support keyboard the number of the page to go to can be set too
    sal_uInt16              m_nNewPage;

    sal_uInt16          m_nOldPageNum;
    OUString            m_sOldSectionName;

    Point               m_aTabColFromDocPos;  // moving table columns out of the document
    SwTextNode           * m_pNumRuleNodeFromDoc; // Moving indent of numrule #i23726#

    Size                m_aDocSz;         // current document size
    Rectangle           m_aVisArea;       // visible region

    VclPtr<SwEditWin>    m_pEditWin;
    SwWrtShell          *m_pWrtShell;

    SfxShell            *m_pShell;        // current SubShell at the dispatcher
    FmFormShell         *m_pFormShell;    // DB-FormShell

    SwView_Impl         *m_pViewImpl;     // Impl-data for UNO + Basic

    VclPtr<SwScrollbar>  m_pHScrollbar,   // MDI control elements
                         m_pVScrollbar;

    bool                m_bHScrollbarEnabled;
    bool                m_bVScrollbarEnabled;

    VclPtr<vcl::Window> m_pScrollFill;   // dummy window for filling the lower right edge
                                        // when both scrollbars are active

    VclPtr<SvxRuler>    m_pHRuler,
                        m_pVRuler;
    VclPtr<ImageButton> m_pTogglePageBtn;

    VclPtr<SwHlpImageButton> m_pPageUpBtn,
                             m_pPageDownBtn;

    SwGlossaryHdl       *m_pGlosHdl;          // handle text block
    SwDrawBase          *m_pDrawActual;

    const SwFrameFormat      *m_pLastTableFormat;

    SwFormatClipboard   *m_pFormatClipboard; //holds data for format paintbrush

    SwPostItMgr         *m_pPostItMgr;

    int                 m_nSelectionType;
    VclPtr<FloatingWindow> m_pFieldPopup;
    sal_uInt16          m_nPageCnt;

    // current draw mode
    sal_uInt16          m_nDrawSfxId;
    OUString            m_sDrawCustom; //some drawing types are marked with strings!
    sal_uInt16          m_nFormSfxId;
    SotExchangeDest     m_nLastPasteDestination;

    // save the border distance status from SwView::StateTabWin to re-use it in SwView::ExecTabWin()
    sal_uInt16          m_nLeftBorderDistance;
    sal_uInt16          m_nRightBorderDistance;

    SvxSearchCmd        m_eLastSearchCommand;

    bool m_bWheelScrollInProgress;

    bool            m_bCenterCursor : 1,
                    m_bTopCursor : 1,
                    m_bAlwaysShowSel : 1,
                    m_bTabColFromDoc : 1,
                    m_bTabRowFromDoc : 1,
                    m_bSetTabColFromDoc : 1 ,
                    m_bSetTabRowFromDoc : 1,
                    m_bAttrChgNotified : 1,
                    m_bAttrChgNotifiedWithRegistrations : 1,
                    m_bVerbsActive : 1,
                    m_bDrawRotate : 1,
                    m_bDrawSelMode : 1,
                    m_bShowAtResize : 1,
                    m_bInOuterResizePixel : 1,
                    m_bInInnerResizePixel : 1,
                    m_bPasteState : 1,
                    m_bPasteSpecialState : 1,
                    m_bInMailMerge : 1,
                    m_bInDtor : 1, //detect destructor to prevent creating of sub shells while closing
                    m_bOldShellWasPagePreview : 1,
                    m_bIsPreviewDoubleClick : 1, // #i114045#
                    m_bMakeSelectionVisible : 1, // transport the bookmark selection
                    m_bAnnotationMode; ///< The real cursor position is inside an annotation.

    /// LibreOfficeKit has to force the page size for PgUp/PgDown
    /// functionality based on the user's view, instead of using the m_aVisArea.
    SwTwips         m_nLOKPageUpDownOffset;

    // methods for searching
    // set search context
    SAL_DLLPRIVATE bool              SearchAndWrap(bool bApi = false);
    SAL_DLLPRIVATE bool          SearchAll(sal_uInt16* pFound = nullptr);
    SAL_DLLPRIVATE sal_uLong         FUNC_Search( const SwSearchOptions& rOptions );
    SAL_DLLPRIVATE void          Replace();

    bool                        IsDocumentBorder();

    SAL_DLLPRIVATE bool          IsTextTool() const;

    DECL_DLLPRIVATE_LINK_TYPED( TimeoutHdl, Timer*, void );

    DECL_DLLPRIVATE_LINK_TYPED( FieldPopupModeEndHdl, FloatingWindow*, void );

    inline long     GetXScroll() const;
    inline long     GetYScroll() const;
    SAL_DLLPRIVATE  Point    AlignToPixel(const Point& rPt) const;
    SAL_DLLPRIVATE   void        CalcPt( Point* pPt,const Rectangle& rRect,
                            sal_uInt16 nRangeX = USHRT_MAX,
                            sal_uInt16 nRangeY = USHRT_MAX);

    SAL_DLLPRIVATE bool          GetPageScrollUpOffset(SwTwips& rOff) const;
    SAL_DLLPRIVATE bool          GetPageScrollDownOffset(SwTwips& rOff) const;

    // scrollbar movements
    SAL_DLLPRIVATE long          PageUp();
    SAL_DLLPRIVATE long          PageDown();
    SAL_DLLPRIVATE bool          PageUpCursor(bool bSelect);
    SAL_DLLPRIVATE bool          PageDownCursor(bool bSelect);
    SAL_DLLPRIVATE long          PhyPageUp();
    SAL_DLLPRIVATE long          PhyPageDown();

    SAL_DLLPRIVATE int           _CreateScrollbar( bool bHori );
    DECL_DLLPRIVATE_LINK_TYPED(  ScrollHdl, ScrollBar*, void );
    DECL_DLLPRIVATE_LINK_TYPED(  EndScrollHdl, ScrollBar*, void );
    SAL_DLLPRIVATE bool          UpdateScrollbars();
    DECL_DLLPRIVATE_LINK_TYPED( WindowChildEventListener, VclWindowEvent&, void );
    SAL_DLLPRIVATE void          CalcVisArea( const Size &rPixelSz );

    // linguistics functions
    SAL_DLLPRIVATE void          HyphenateDocument();
    SAL_DLLPRIVATE bool          IsDrawTextHyphenate();
    SAL_DLLPRIVATE void          HyphenateDrawText();
    SAL_DLLPRIVATE void          StartThesaurus();

    // text conversion
    SAL_DLLPRIVATE void          StartTextConversion( LanguageType nSourceLang, LanguageType nTargetLang, const vcl::Font *pTargetFont, sal_Int32 nOptions, bool bIsInteractive );

    // used for spell checking and text conversion
    SAL_DLLPRIVATE void          SpellStart( SvxSpellArea eSpell, bool bStartDone,
                                        bool bEndDone, SwConversionArgs *pConvArgs = nullptr );
    SAL_DLLPRIVATE void          SpellEnd( SwConversionArgs *pConvArgs = nullptr );

    SAL_DLLPRIVATE void          HyphStart( SvxSpellArea eSpell );
    SAL_DLLPRIVATE void          SpellKontext(bool bOn = true)
                                 { m_bCenterCursor = bOn; m_bAlwaysShowSel = bOn; }

    // for readonly switching
    SAL_DLLPRIVATE virtual void  Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    SAL_DLLPRIVATE void          _CheckReadonlyState();
    SAL_DLLPRIVATE void          _CheckReadonlySelection();

    // method for rotating PageDesc
    SAL_DLLPRIVATE void          SwapPageMargin(const SwPageDesc&, SvxLRSpaceItem& rLR);

    SAL_DLLPRIVATE void          _SetZoom( const Size &rEditSz,
                              SvxZoomType eZoomType,
                              short nFactor = 100,
                              bool bViewOnly = false);
    SAL_DLLPRIVATE void          CalcAndSetBorderPixel( SvBorder &rToFill, bool bInner );

    SAL_DLLPRIVATE void          ShowAtResize();

    SAL_DLLPRIVATE virtual void  Move() override;

public: // #i123922# Needs to be called from a 2nd place now as a helper method
    SAL_DLLPRIVATE bool          InsertGraphicDlg( SfxRequest& );

protected:

    SwView_Impl*    GetViewImpl() {return m_pViewImpl;}

    void ImpSetVerb( int nSelType );

    int             GetSelectionType() const { return m_nSelectionType; }
    void            SetSelectionType(int nSet) { m_nSelectionType = nSet;}

    // for SwWebView
    void            SetShell( SfxShell* pS )            { m_pShell = pS; }
    void            SetFormShell( FmFormShell* pSh )    { m_pFormShell = pSh; }

    virtual void    SelectShell();

    virtual void    Activate(bool) override;
    virtual void    Deactivate(bool) override;
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize ) override;
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize ) override;

    const SwFrameFormat* GetLastTableFrameFormat() const {return m_pLastTableFormat;}
    void            SetLastTableFrameFormat(const SwFrameFormat* pSet) {m_pLastTableFormat = pSet;}

    // form letter execution
    void    GenerateFormLetter(bool bUseCurrentDocument);

    using SfxShell::GetDispatcher;

public:
    SFX_DECL_VIEWFACTORY(SwView);
    SFX_DECL_INTERFACE(SW_VIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SfxDispatcher   &GetDispatcher();

    void                    GotFocus() const;
    virtual SdrView*        GetDrawView() const override;
    virtual bool            HasUIFeature( sal_uInt32 nFeature ) override;
    virtual void            ShowCursor( bool bOn = true ) override;
    virtual ErrCode         DoVerb( long nVerb ) override;

    virtual sal_uInt16          SetPrinter( SfxPrinter* pNew,
                                        SfxPrinterChangeFlags nDiff = SFX_PRINTER_ALL, bool bIsAPI=false) override;
    ShellModes              GetShellMode();

    css::view::XSelectionSupplier*       GetUNOObject();

    OUString                GetSelectionTextParam( bool bCompleteWords,
                                                   bool bEraseTrail );
    virtual bool            HasSelection( bool bText ) const override;
    virtual OUString        GetSelectionText( bool bCompleteWords = false ) override;
    virtual bool            PrepareClose( bool bUI = true ) override;
    virtual void            MarginChanged() override;

    // replace word/selection with text from the thesaurus
    // (this code has special handling for "in word" character)
    void                    InsertThesaurusSynonym( const OUString &rSynonmText, const OUString &rLookUpText, bool bValidSelection );
    bool                    IsValidSelectionForThesaurus() const;
    OUString                GetThesaurusLookUpText( bool bSelection ) const;

    // immediately switch shell -> for GetSelectionObject
    void                    StopShellTimer();

    inline SwWrtShell&      GetWrtShell   () const { return *m_pWrtShell; }
    inline SwWrtShell*      GetWrtShellPtr() const { return  m_pWrtShell; }

    inline       SwEditWin &GetEditWin()        { return *m_pEditWin; }
    inline const SwEditWin &GetEditWin () const { return *m_pEditWin; }

#if defined WNT || defined UNX
    void ScannerEventHdl( const css::lang::EventObject& rEventObject );
#endif

    // hand the handler for text blocks to the shell; create if applicable
    SwGlossaryHdl*          GetGlosHdl();

    inline const Rectangle& GetVisArea() const { return m_aVisArea; }

    bool            IsScroll(const Rectangle& rRect) const;
    void            Scroll( const Rectangle& rRect,
                            sal_uInt16 nRangeX = USHRT_MAX,
                            sal_uInt16 nRangeY = USHRT_MAX);

    long        SetVScrollMax(long lMax);
    long        SetHScrollMax(long lMax);

    void SpellError(LanguageType eLang);
    bool            ExecSpellPopup( const Point& rPt );
    void                ExecFieldPopup( const Point& rPt, sw::mark::IFieldmark *fieldBM );
    bool            ExecSmartTagPopup( const Point& rPt );

    DECL_LINK_TYPED( OnlineSpellCallback, SpellCallbackInfo&, void );
    bool            ExecDrwTextSpellPopup(const Point& rPt);

    void            SetTabColFromDocPos( const Point &rPt ) { m_aTabColFromDocPos = rPt; }
    void            SetTabColFromDoc( bool b ) { m_bTabColFromDoc = b; }
    bool            IsTabColFromDoc() const    { return m_bTabColFromDoc; }
    void            SetTabRowFromDoc( bool b ) { m_bTabRowFromDoc = b; }
    bool            IsTabRowFromDoc() const    { return m_bTabRowFromDoc; }

    void            SetNumRuleNodeFromDoc( SwTextNode * pNumRuleNode )
                    { m_pNumRuleNodeFromDoc = pNumRuleNode; }

    void    DocSzChgd( const Size& rNewSize );
    const   Size&   GetDocSz() const { return m_aDocSz; }
    void    SetVisArea( const Rectangle&, bool bUpdateScrollbar = true);
            void    SetVisArea( const Point&, bool bUpdateScrollbar = true);
            void    CheckVisArea();

    void RecheckBrowseMode();
    static SvxSearchDialog* GetSearchDialog();

    static sal_uInt16   GetMoveType();
    static void     SetMoveType(sal_uInt16 nSet);
    DECL_LINK_TYPED( MoveNavigationHdl, void*, void );
    static void     SetActMark(sal_Int32 nSet);

    bool            HandleWheelCommands( const CommandEvent& );

    // insert frames
    void            InsFrameMode(sal_uInt16 nCols);

    void            SetZoom( SvxZoomType eZoomType, short nFactor = 100, bool bViewOnly = false);
    virtual void    SetZoomFactor( const Fraction &rX, const Fraction & ) override;

    void            SetViewLayout( sal_uInt16 nColumns, bool bBookMode, bool bViewOnly = false );

    void            ShowHScrollbar(bool bShow);
    bool            IsHScrollbarVisible()const;

    void            ShowVScrollbar(bool bShow);
    bool            IsVScrollbarVisible()const;

    void            EnableHScrollbar(bool bEnable);
    void            EnableVScrollbar(bool bEnable);

    int             CreateVRuler();
    int             KillVRuler();
    int             CreateTab();
    int             KillTab();

    bool            StatVRuler() const { return m_pVRuler->IsVisible(); }
    void            ChangeVRulerMetric(FieldUnit eUnit);
    void            GetVRulerMetric(FieldUnit& rToFill) const;

    SvxRuler&       GetHRuler()    { return *m_pHRuler; }
    SvxRuler&       GetVRuler()    { return *m_pVRuler; }
    void            InvalidateRulerPos();
    void            ChangeTabMetric(FieldUnit eUnit);
    void            GetHRulerMetric(FieldUnit& rToFill) const;

    // Handler
    void            Execute(SfxRequest&);
    void            ExecLingu(SfxRequest&);
    void            ExecDlg(SfxRequest&);
    void            ExecDlgExt(SfxRequest&);
    void            ExecColl(SfxRequest&);
    void            ExecutePrint(SfxRequest&);
    void            ExecDraw(SfxRequest&);
    void            ExecTabWin(SfxRequest&);
    void            ExecuteStatusLine(SfxRequest&);
    DECL_LINK_TYPED( ExecRulerClick, Ruler *, void );
    void            ExecSearch(SfxRequest&, bool bNoMessage = false);
    void            ExecViewOptions(SfxRequest &);

    void            StateViewOptions(SfxItemSet &);
    void            StateSearch(SfxItemSet &);
    void            GetState(SfxItemSet&);
    void            StateStatusLine(SfxItemSet&);
    void            UpdateWordCount(SfxShell*, sal_uInt16);
    void            ExecFormatFootnote();
    void            ExecNumberingOutline(SfxItemPool &);

    // functions for drawing
    void            SetDrawFuncPtr(SwDrawBase* pFuncPtr);
    inline SwDrawBase* GetDrawFuncPtr() const  { return m_pDrawActual; }
    void            GetDrawState(SfxItemSet &rSet);
    void            ExitDraw();
    inline bool     IsDrawRotate()      { return m_bDrawRotate; }
    inline void     FlipDrawRotate()    { m_bDrawRotate = !m_bDrawRotate; }
    inline bool     IsDrawSelMode()     { return m_bDrawSelMode; }
    void            SetSelDrawSlot();
    inline void     FlipDrawSelMode()   { m_bDrawSelMode = !m_bDrawSelMode; }
    void            NoRotate();     // turn off rotate mode
    bool            EnterDrawTextMode(const Point& aDocPos);
    /// Same as EnterDrawTextMode(), but takes an SdrObject instead of guessing it by document position.
    bool EnterShapeDrawTextMode(SdrObject* pObject);
    void            LeaveDrawCreate()   { m_nDrawSfxId = m_nFormSfxId = USHRT_MAX; m_sDrawCustom.clear();}
    bool            IsDrawMode()        { return (m_nDrawSfxId != USHRT_MAX || m_nFormSfxId != USHRT_MAX); }
    bool            IsFormMode() const;
    bool            IsBezierEditMode();
    bool            AreOnlyFormsSelected() const;
    bool            HasDrwObj(SdrObject *pSdrObj) const;
    bool            HasOnlyObj(SdrObject *pSdrObj, sal_uInt32 eObjInventor) const;
    bool            BeginTextEdit(  SdrObject* pObj, SdrPageView* pPV=nullptr,
                                    vcl::Window* pWin=nullptr, bool bIsNewObj=false, bool bSetSelectionToStart=false );

    void            StateTabWin(SfxItemSet&);

    // attributes have changed
    DECL_LINK_TYPED( AttrChangedNotify, SwCursorShell*, void );

    // form control has been activated
    DECL_LINK_TYPED( FormControlActivated, LinkParamNone*, void );

    // edit links
    void            EditLinkDlg();
    void            AutoCaption(const sal_uInt16 nType, const SvGlobalName *pOleId = nullptr);
    void            InsertCaption(const InsCaptionOpt *pOpt);

    // Async call by Core
    void        UpdatePageNums(sal_uInt16 nPhyNum, sal_uInt16 nVirtNum, const OUString& rPgStr);

    OUString    GetPageStr(sal_uInt16 nPhyNum, sal_uInt16 nVirtNum, const OUString& rPgStr);

    /// Force page size for PgUp/PgDown to overwrite the computation based on m_aVisArea.
    void ForcePageUpDownOffset(SwTwips nTwips)
    {
        m_nLOKPageUpDownOffset = nTwips;
    }

    // hand over Shell
                 SfxShell       *GetCurShell()  { return m_pShell; }
                 SwDocShell     *GetDocShell();
    inline const SwDocShell     *GetDocShell() const;
    inline virtual       FmFormShell    *GetFormShell()       override { return m_pFormShell; }
    inline virtual const FmFormShell    *GetFormShell() const override { return m_pFormShell; }

    // so that in the SubShells' DTors m_pShell can be reset if applicable
    void ResetSubShell()    { m_pShell = nullptr; }

    virtual void    WriteUserData(OUString &, bool bBrowse = false) override;
    virtual void    ReadUserData(const OUString &, bool bBrowse = false) override;
    virtual void    ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse ) override;
    virtual void    WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >&, bool bBrowse ) override;

    void SetCursorAtTop( bool bFlag, bool bCenter = false )
        { m_bTopCursor = bFlag; m_bCenterCursor = bCenter; }
    bool IsCursorAtTop() const                    { return m_bTopCursor; }
    bool IsCursorAtCenter() const                 { return m_bCenterCursor; }

    bool JumpToSwMark( const OUString& rMark );

    long InsertDoc( sal_uInt16 nSlotId, const OUString& rFileName,
                    const OUString& rFilterName, sal_Int16 nVersion = 0 );

    void ExecuteInsertDoc( SfxRequest& rRequest, const SfxPoolItem* pItem );
    long InsertMedium( sal_uInt16 nSlotId, SfxMedium* pMedium, sal_Int16 nVersion );
    DECL_LINK_TYPED( DialogClosedHdl, sfx2::FileDialogHelper *, void );

    // status methods for clipboard.
    // Status changes now notified from the clipboard.
    bool IsPasteAllowed();
    bool IsPasteSpecialAllowed();

    // Enable mail merge - mail merge field dialog enabled
    void EnableMailMerge(bool bEnable = true);
    //apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions);

    SwView(SfxViewFrame* pFrame, SfxViewShell*);
    virtual ~SwView();

    void NotifyDBChanged();

    SfxObjectShellLock CreateTmpSelectionDoc();

    void        AddTransferable(SwTransferable& rTransferable);

    // store MailMerge data while "Back to Mail Merge Wizard" FloatingWindow is active
    // or to support printing
    void   SetMailMergeConfigItem(SwMailMergeConfigItem*  pConfigItem, sal_uInt16 nRestart, bool bIsSource);
    SwMailMergeConfigItem*  GetMailMergeConfigItem();
    sal_uInt16              GetMailMergeRestartPage() const;
    bool                IsMailMergeSourceView() const;

    void ExecFormatPaintbrush(SfxRequest &);
    void StateFormatPaintbrush(SfxItemSet &);

    //public fuer D&D
    int     InsertGraphic( const OUString &rPath, const OUString &rFilter,
                            bool bLink = true, GraphicFilter *pFlt = nullptr,
                            Graphic* pPreviewGrf = nullptr,
                            bool bRule = false );

    void ExecuteScan( SfxRequest& rReq );

    SwPostItMgr* GetPostItMgr() { return m_pPostItMgr;}
    const SwPostItMgr* GetPostItMgr() const { return m_pPostItMgr;}

    // exhibition hack (MA,MBA)
    void SelectShellForDrop();

    void UpdateDocStats();
    /// Where is the real cursor: in the annotation or in the main document?
    void SetAnnotationMode(bool bMode);

    // methods for printing
    SAL_DLLPRIVATE virtual   SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    SAL_DLLPRIVATE virtual bool  HasPrintOptionsPage() const override;
    SAL_DLLPRIVATE virtual VclPtr<SfxTabPage> CreatePrintOptionsPage( vcl::Window* pParent,
                                                    const SfxItemSet& rSet) override;
    static SvxSearchItem* GetSearchItem() { return m_pSrchItem; }
    /// See SfxViewShell::getPart().
    int getPart() const override;
};

inline long SwView::GetXScroll() const
{
    return m_aVisArea.GetWidth() * nScrollX / 100L;
}

inline long SwView::GetYScroll() const
{
    return m_aVisArea.GetHeight() * nScrollY / 100L;
}

inline const SwDocShell *SwView::GetDocShell() const
{
    return const_cast<SwView*>(this)->GetDocShell();
}

VclPtr<SfxTabPage> CreatePrintOptionsPage( vcl::Window *pParent,
                                           const SfxItemSet &rOptions,
                                           bool bPreview);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
