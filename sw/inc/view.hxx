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
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/zoomitem.hxx>
#include <svx/ruler.hxx>
#include <svx/fmshell.hxx>
#include "swdllapi.h"
#include "swtypes.hxx"
#include "shellid.hxx"

class SwTextFormatColl;
class SwPageDesc;
class SwFrameFormat;
class SwCharFormat;
class SwNumRule;
class SwGlossaryHdl;
class SwDrawBase;
class SvxLRSpaceItem;
class SwDocShell;
class SwScrollbar;
class SvBorder;
class Ruler;
class SvxSearchItem;
class SearchAttrItemList;
class SvxSearchDialog;
class SdrView;
class SdrPageView;
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
class GraphicFilter;
class SwPostItMgr;
enum class SotExchangeDest;
class SwCursorShell;
enum class SvxSearchCmd;
enum class SelectionType : sal_Int32;

namespace com{ namespace sun { namespace star {
    namespace view{ class XSelectionSupplier; }
}}}
namespace sfx2 { class FileDialogHelper; }
namespace sw { namespace mark { class IFieldmark; } }

const long nLeftOfst = -370;
const long nScrollX  =   30;
const long nScrollY  =   30;

#define MINZOOM 20
#define MAXZOOM 600

#define MAX_MARKS 5

enum class ShellMode
{
    Text,
    Frame,
    Graphic,
    Object,
    Draw,
    DrawForm,
    DrawText,
    Bezier,
    ListText,
    TableText,
    TableListText,
    Media,
    ExtrudedCustomShape,
    FontWork,
    PostIt
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

    SfxStyleFamily eType;
    sal_uInt16 nColor;
    SwFormatClipboard* m_pFormatClipboard;
    size_t nUndo;     //< The initial undo stack depth.

    SwApplyTemplate() :
        eType(SfxStyleFamily::None),
        nColor(0),
        m_pFormatClipboard(nullptr),
        nUndo(0)
    {
        aColl.pTextColl = nullptr;
    }
};

class SwView;

// manage connection and disconnection of SwView and SwDocShell
class SwViewGlueDocShell
{
private:
    SwView& m_rView;
public:
    SwViewGlueDocShell(SwView& rView, SwDocShell& rDocSh);
    ~SwViewGlueDocShell();
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
    tools::Rectangle           m_aVisArea;       // visible region

    VclPtr<SwEditWin>    m_pEditWin;
    std::unique_ptr<SwWrtShell> m_pWrtShell;
    std::unique_ptr<SwViewGlueDocShell> m_xGlueDocShell;

    SfxShell            *m_pShell;        // current SubShell at the dispatcher
    FmFormShell         *m_pFormShell;    // DB-FormShell

    std::unique_ptr<SwView_Impl> m_pViewImpl;     // Impl-data for UNO + Basic

    VclPtr<SwScrollbar>  m_pHScrollbar,   // MDI control elements
                         m_pVScrollbar;

    bool                m_bHScrollbarEnabled;
    bool                m_bVScrollbarEnabled;

    VclPtr<vcl::Window> m_pScrollFill;   // dummy window for filling the lower right edge
                                        // when both scrollbars are active

    VclPtr<SvxRuler>    m_pHRuler,
                        m_pVRuler;

    std::unique_ptr<SwGlossaryHdl> m_pGlosHdl;          // handle text block
    std::unique_ptr<SwDrawBase>    m_pDrawActual;

    const SwFrameFormat      *m_pLastTableFormat;

    std::unique_ptr<SwFormatClipboard> m_pFormatClipboard; //holds data for format paintbrush

    std::unique_ptr<SwPostItMgr> m_pPostItMgr;

    SelectionType       m_nSelectionType;
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
                    m_bMakeSelectionVisible : 1; // transport the bookmark selection
    bool m_bInitOnceCompleted = false;

    /// LibreOfficeKit has to force the page size for PgUp/PgDown
    /// functionality based on the user's view, instead of using the m_aVisArea.
    SwTwips         m_nLOKPageUpDownOffset;

    // methods for searching
    // set search context
    SAL_DLLPRIVATE bool          SearchAndWrap(bool bApi);
    SAL_DLLPRIVATE bool          SearchAll();
    SAL_DLLPRIVATE sal_uLong     FUNC_Search( const SwSearchOptions& rOptions );
    SAL_DLLPRIVATE void          Replace();

    bool                        IsDocumentBorder();

    SAL_DLLPRIVATE bool          IsTextTool() const;

    DECL_DLLPRIVATE_LINK( TimeoutHdl, Timer*, void );

    inline long                  GetXScroll() const;
    inline long                  GetYScroll() const;
    SAL_DLLPRIVATE Point         AlignToPixel(const Point& rPt) const;
    SAL_DLLPRIVATE void          CalcPt( Point* pPt,const tools::Rectangle& rRect,
                                    sal_uInt16 nRangeX,
                                    sal_uInt16 nRangeY);

    SAL_DLLPRIVATE bool          GetPageScrollUpOffset(SwTwips& rOff) const;
    SAL_DLLPRIVATE bool          GetPageScrollDownOffset(SwTwips& rOff) const;

    // scrollbar movements
    SAL_DLLPRIVATE bool          PageUp();
    SAL_DLLPRIVATE bool          PageDown();
    SAL_DLLPRIVATE bool          PageUpCursor(bool bSelect);
    SAL_DLLPRIVATE bool          PageDownCursor(bool bSelect);
    SAL_DLLPRIVATE void          PhyPageUp();
    SAL_DLLPRIVATE void          PhyPageDown();

    SAL_DLLPRIVATE void           CreateScrollbar( bool bHori );
    DECL_DLLPRIVATE_LINK(  ScrollHdl, ScrollBar*, void );
    DECL_DLLPRIVATE_LINK(  EndScrollHdl, ScrollBar*, void );
    SAL_DLLPRIVATE bool          UpdateScrollbars();
    DECL_DLLPRIVATE_LINK( WindowChildEventListener, VclWindowEvent&, void );
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
                                        bool bEndDone, SwConversionArgs *pConvArgs );
    SAL_DLLPRIVATE void          SpellEnd( SwConversionArgs const *pConvArgs );

    SAL_DLLPRIVATE void          HyphStart( SvxSpellArea eSpell );
    SAL_DLLPRIVATE void          SpellContext(bool bOn = true)
                                 { m_bCenterCursor = bOn; }

    // for readonly switching
    SAL_DLLPRIVATE virtual void  Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    SAL_DLLPRIVATE void          CheckReadonlyState();
    SAL_DLLPRIVATE void          CheckReadonlySelection();

    // method for rotating PageDesc
    SAL_DLLPRIVATE void          SwapPageMargin(const SwPageDesc&, SvxLRSpaceItem& rLR);

    SAL_DLLPRIVATE void          SetZoom_( const Size &rEditSz,
                                      SvxZoomType eZoomType,
                                      short nFactor,
                                      bool bViewOnly);
    SAL_DLLPRIVATE void          CalcAndSetBorderPixel( SvBorder &rToFill );

    SAL_DLLPRIVATE void          ShowAtResize();

    SAL_DLLPRIVATE virtual void  Move() override;

public: // #i123922# Needs to be called from a 2nd place now as a helper method
    SAL_DLLPRIVATE bool          InsertGraphicDlg( SfxRequest& );

protected:

    SwView_Impl*    GetViewImpl() {return m_pViewImpl.get();}

    void ImpSetVerb( SelectionType nSelType );

    SelectionType   GetSelectionType() const { return m_nSelectionType; }
    void            SetSelectionType(SelectionType nSet) { m_nSelectionType = nSet;}

    // for SwWebView
    void            SetShell( SfxShell* pS )            { m_pShell = pS; }
    void            SetFormShell( FmFormShell* pSh )    { m_pFormShell = pSh; }

    virtual void    SelectShell();

    virtual void    Activate(bool) override;
    virtual void    Deactivate(bool) override;
    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize, bool inplaceEditModeChange ) override;
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
    virtual bool            HasUIFeature(SfxShellFeature nFeature) const override;
    virtual void            ShowCursor( bool bOn = true ) override;
    virtual ErrCode         DoVerb( long nVerb ) override;

    virtual sal_uInt16      SetPrinter( SfxPrinter* pNew,
                                        SfxPrinterChangeFlags nDiff = SFX_PRINTER_ALL) override;
    ShellMode               GetShellMode();

    css::view::XSelectionSupplier*       GetUNOObject();

    OUString                GetSelectionTextParam( bool bCompleteWords,
                                                   bool bEraseTrail );
    virtual bool            HasSelection( bool bText = true ) const override;
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

    SwWrtShell&      GetWrtShell   () const { return *m_pWrtShell; }
    SwWrtShell*      GetWrtShellPtr() const { return  m_pWrtShell.get(); }

    SwEditWin &GetEditWin()        { return *m_pEditWin; }
    const SwEditWin &GetEditWin () const { return *m_pEditWin; }

#if defined(_WIN32) || defined UNX
    void ScannerEventHdl();
#endif

    // hand the handler for text blocks to the shell; create if applicable
    SwGlossaryHdl*          GetGlosHdl();

    const tools::Rectangle& GetVisArea() const { return m_aVisArea; }

    bool            IsScroll(const tools::Rectangle& rRect) const;
    void            Scroll( const tools::Rectangle& rRect,
                            sal_uInt16 nRangeX = USHRT_MAX,
                            sal_uInt16 nRangeY = USHRT_MAX);

    long            SetVScrollMax(long lMax);
    long            SetHScrollMax(long lMax);

    void            SpellError(LanguageType eLang);
    bool            ExecSpellPopup( const Point& rPt );
    void            ExecSmartTagPopup( const Point& rPt );

    DECL_LINK( OnlineSpellCallback, SpellCallbackInfo&, void );
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
    void    SetVisArea( const tools::Rectangle&, bool bUpdateScrollbar = true);
            void    SetVisArea( const Point&, bool bUpdateScrollbar = true);
            void    CheckVisArea();

    void RecheckBrowseMode();
    static SvxSearchDialog* GetSearchDialog();

    static sal_uInt16   GetMoveType();
    static void     SetMoveType(sal_uInt16 nSet);
    DECL_LINK( MoveNavigationHdl, void*, void );
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

    void            CreateVRuler();
    void            KillVRuler();
    void            CreateTab();
    void            KillTab();

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
    void            ExecDlg(SfxRequest const &);
    void            ExecDlgExt(SfxRequest const &);
    void            ExecColl(SfxRequest const &);
    void            ExecutePrint(SfxRequest&);
    void            ExecDraw(SfxRequest&);
    void            ExecTabWin(SfxRequest const &);
    void            ExecuteStatusLine(SfxRequest&);
    DECL_LINK( ExecRulerClick, Ruler *, void );
    void            ExecSearch(SfxRequest&);
    void            ExecViewOptions(SfxRequest &);

    virtual bool    IsConditionalFastCall( const SfxRequest &rReq ) override;

    void            StateViewOptions(SfxItemSet &);
    void            StateSearch(SfxItemSet &);
    void            GetState(SfxItemSet&);
    void            StateStatusLine(SfxItemSet&);
    void            UpdateWordCount(SfxShell*, sal_uInt16);
    void            ExecFormatFootnote();
    void            ExecNumberingOutline(SfxItemPool &);

    // functions for drawing
    void            SetDrawFuncPtr(std::unique_ptr<SwDrawBase> pFuncPtr);
    SwDrawBase*     GetDrawFuncPtr() const  { return m_pDrawActual.get(); }
    void            GetDrawState(SfxItemSet &rSet);
    void            ExitDraw();
    bool     IsDrawRotate()      { return m_bDrawRotate; }
    void     FlipDrawRotate()    { m_bDrawRotate = !m_bDrawRotate; }
    bool     IsDrawSelMode()     { return m_bDrawSelMode; }
    void            SetSelDrawSlot();
    void     FlipDrawSelMode()   { m_bDrawSelMode = !m_bDrawSelMode; }
    void            NoRotate();     // turn off rotate mode
    bool            EnterDrawTextMode(const Point& aDocPos);
    /// Same as EnterDrawTextMode(), but takes an SdrObject instead of guessing it by document position.
    bool EnterShapeDrawTextMode(SdrObject* pObject);
    void            LeaveDrawCreate()   { m_nDrawSfxId = m_nFormSfxId = USHRT_MAX; m_sDrawCustom.clear();}
    bool            IsDrawMode()        { return (m_nDrawSfxId != USHRT_MAX || m_nFormSfxId != USHRT_MAX); }
    bool            IsFormMode() const;
    bool            IsBezierEditMode();
    bool            AreOnlyFormsSelected() const;
    bool            HasOnlyObj(SdrObject const *pSdrObj, SdrInventor eObjInventor) const;
    bool            BeginTextEdit(  SdrObject* pObj, SdrPageView* pPV=nullptr,
                                    vcl::Window* pWin=nullptr, bool bIsNewObj=false, bool bSetSelectionToStart=false );
    bool isSignatureLineSelected();
    bool isSignatureLineSigned();

    void            StateTabWin(SfxItemSet&);

    // attributes have changed
    DECL_LINK( AttrChangedNotify, SwCursorShell*, void );

    // form control has been activated
    DECL_LINK( FormControlActivated, LinkParamNone*, void );

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
    virtual       FmFormShell    *GetFormShell()       override { return m_pFormShell; }
    virtual const FmFormShell    *GetFormShell() const override { return m_pFormShell; }

    // so that in the SubShells' DTors m_pShell can be reset if applicable
    void ResetSubShell()    { m_pShell = nullptr; }

    virtual void    WriteUserData(OUString &, bool bBrowse = false) override;
    virtual void    ReadUserData(const OUString &, bool bBrowse = false) override;
    virtual void    ReadUserDataSequence ( const css::uno::Sequence < css::beans::PropertyValue >& ) override;
    virtual void    WriteUserDataSequence ( css::uno::Sequence < css::beans::PropertyValue >& ) override;

    void SetCursorAtTop( bool bFlag, bool bCenter = false )
        { m_bTopCursor = bFlag; m_bCenterCursor = bCenter; }

    bool JumpToSwMark( const OUString& rMark );

    long InsertDoc( sal_uInt16 nSlotId, const OUString& rFileName,
                    const OUString& rFilterName, sal_Int16 nVersion = 0 );

    void ExecuteInsertDoc( SfxRequest& rRequest, const SfxPoolItem* pItem );
    long InsertMedium( sal_uInt16 nSlotId, std::unique_ptr<SfxMedium> pMedium, sal_Int16 nVersion );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper *, void );

    // status methods for clipboard.
    // Status changes now notified from the clipboard.
    bool IsPasteAllowed();
    bool IsPasteSpecialAllowed();

    // Enable mail merge - mail merge field dialog enabled
    void EnableMailMerge();
    //apply Accessiblity options
    void ApplyAccessiblityOptions(SvtAccessibilityOptions const & rAccessibilityOptions);

    SwView(SfxViewFrame* pFrame, SfxViewShell*);
    virtual ~SwView() override;

    void NotifyDBChanged();

    SfxObjectShellLock CreateTmpSelectionDoc();

    void        AddTransferable(SwTransferable& rTransferable);

    // store MailMerge data while "Back to Mail Merge Wizard" FloatingWindow is active
    // or to support printing
    void SetMailMergeConfigItem(std::shared_ptr<SwMailMergeConfigItem> const & rConfigItem);
    std::shared_ptr<SwMailMergeConfigItem> const & GetMailMergeConfigItem() const;
    std::shared_ptr<SwMailMergeConfigItem> EnsureMailMergeConfigItem(const SfxItemSet* pArgs = nullptr);

    void ExecFormatPaintbrush(SfxRequest const &);
    void StateFormatPaintbrush(SfxItemSet &);

    // public for D&D
    ErrCode     InsertGraphic( const OUString &rPath, const OUString &rFilter,
                            bool bLink, GraphicFilter *pFlt );

    void ExecuteScan( SfxRequest& rReq );

    SwPostItMgr* GetPostItMgr() { return m_pPostItMgr.get();}
    const SwPostItMgr* GetPostItMgr() const { return m_pPostItMgr.get();}

    // exhibition hack (MA,MBA)
    void SelectShellForDrop();

    void UpdateDocStats();

    // methods for printing
    SAL_DLLPRIVATE virtual   SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    SAL_DLLPRIVATE virtual bool  HasPrintOptionsPage() const override;
    SAL_DLLPRIVATE virtual VclPtr<SfxTabPage> CreatePrintOptionsPage(TabPageParent pParent,
                                                    const SfxItemSet& rSet) override;
    static SvxSearchItem* GetSearchItem() { return m_pSrchItem; }
    /// See SfxViewShell::getPart().
    int getPart() const override;
    /// See SfxViewShell::dumpAsXml().
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    void SetRedlineAuthor(const OUString& rAuthor);
    const OUString& GetRedlineAuthor();
    /// See SfxViewShell::NotifyCursor().
    void NotifyCursor(SfxViewShell* pViewShell) const override;
    void ShowUIElement(const OUString& sElementURL) const;


    enum CachedStringID
    {
        OldGrfCat,
        OldTabCat,
        OldFrameCat,
        OldDrwCat,
        CachedStrings
    };

    OUString m_StringCache[CachedStrings];

    const OUString& GetCachedString(CachedStringID id)
    {
        return m_StringCache[id];
    }

    void SetCachedString(CachedStringID id, const OUString& sStr)
    {
        m_StringCache[id] = sStr;
    }

    const OUString& GetOldGrfCat();
    void SetOldGrfCat(const OUString& sStr);
    const OUString& GetOldTabCat();
    void SetOldTabCat(const OUString& sStr);
    const OUString& GetOldFrameCat();
    void SetOldFrameCat(const OUString& sStr);
    const OUString& GetOldDrwCat();
    void SetOldDrwCat(const OUString& sStr);
};

inline long SwView::GetXScroll() const
{
    return (m_aVisArea.GetWidth() * nScrollX) / 100;
}

inline long SwView::GetYScroll() const
{
    return (m_aVisArea.GetHeight() * nScrollY) / 100;
}

inline const SwDocShell *SwView::GetDocShell() const
{
    return const_cast<SwView*>(this)->GetDocShell();
}

VclPtr<SfxTabPage> CreatePrintOptionsPage(TabPageParent pParent,
                                          const SfxItemSet &rOptions,
                                          bool bPreview);

extern bool bDocSzUpdated;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
