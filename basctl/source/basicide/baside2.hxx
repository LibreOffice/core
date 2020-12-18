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

#include <memory>
#include <layout.hxx>
#include "breakpoint.hxx"
#include "linenumberwindow.hxx"

#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>

#include <svtools/colorcfg.hxx>
#include <o3tl/enumarray.hxx>
#include <rtl/ustrbuf.hxx>

#include <set>
#include <string_view>

#include <vcl/textdata.hxx>
#include <basic/codecompletecache.hxx>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <comphelper/syntaxhighlight.hxx>

class ExtTextEngine;
class TextView;
class SvxSearchItem;
namespace com::sun::star::beans { class XMultiPropertySet; }

namespace basctl
{

class ObjectCatalog;
class CodeCompleteWindow;
class ModulWindowLayout;

// #108672 Helper functions to get/set text in TextEngine
// using the stream interface (get/setText() only supports
// tools Strings limited to 64K).
// defined in baside2b.cxx
OUString getTextEngineText (ExtTextEngine&);
void setTextEngineText (ExtTextEngine&, std::u16string_view);

class EditorWindow final : public vcl::Window, public SfxListener
{
friend class CodeCompleteWindow;
friend class EditorWindowUIObject;
private:
    class ChangesListener;

    std::unique_ptr<TextView>        pEditView;
    std::unique_ptr<ExtTextEngine>   pEditEngine;
    ModulWindow&                     rModulWindow;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex                        mutex_;
    css::uno::Reference< css::beans::XMultiPropertySet >
                                      notifier_;

    tools::Long            nCurTextWidth;

    ImplSVEvent* m_nSetSourceInBasicId;

    SyntaxHighlighter   aHighlighter;
    Idle                aSyntaxIdle;
    std::set<sal_uInt16>       aSyntaxLineTable;
    DECL_LINK(SyntaxTimerHdl, Timer *, void);
    DECL_LINK(SetSourceInBasicHdl, void*, void);

    // progress bar
    class ProgressInfo;
    std::unique_ptr<ProgressInfo> pProgress;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    void            ImpDoHighlight( sal_uLong nLineOff );
    void            ImplSetFont();

    bool            bHighlighting;
    bool            bDoSyntaxHighlight;
    bool            bDelayHighlight;

    virtual css::uno::Reference< css::awt::XWindowPeer > GetComponentInterface(bool bCreate = true) override;
    CodeCompleteDataCache aCodeCompleteCache;
    VclPtr<CodeCompleteWindow> pCodeCompleteWnd;
    OUString GetActualSubName( sal_uLong nLine ); // gets the actual subroutine name according to line number
    void SetupAndShowCodeCompleteWnd(const std::vector< OUString >& aEntryVect, TextSelection aSel );
    void HandleAutoCorrect();
    void HandleAutoCloseParen();
    void HandleAutoCloseDoubleQuotes();
    void HandleCodeCompletion();
    void HandleProcedureCompletion();
    TextSelection GetLastHighlightPortionTextSelection() const;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;
    virtual void    Resize() override;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    LoseFocus() override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

    void            DoSyntaxHighlight( sal_uLong nPara );
    OUString        GetWordAtCursor();
    bool            ImpCanModify();

public:
                    EditorWindow (vcl::Window* pParent, ModulWindow*);
                    virtual ~EditorWindow() override;
    virtual void    dispose() override;

    ExtTextEngine*  GetEditEngine() const   { return pEditEngine.get(); }
    TextView*       GetEditView() const     { return pEditView.get(); }

    void            CreateProgress( const OUString& rText, sal_uInt32 nRange );
    void            DestroyProgress();

    void            ParagraphInsertedDeleted( sal_uLong nNewPara, bool bInserted );
    void            DoDelayedSyntaxHighlight( sal_uLong nPara );

    void            CreateEditEngine();
    void            SetScrollBarRanges();
    void            InitScrollBars();

    void            ForceSyntaxTimeout();
    void            SetSourceInBasic();

    bool            CanModify() { return ImpCanModify(); }

    void            ChangeFontColor( Color aColor );
    void            UpdateSyntaxHighlighting ();

    bool            GetProcedureName(OUString const & rLine, OUString& rProcType, OUString& rProcName) const;

    FactoryFunction GetUITestFactory() const override;
};

class BreakPointWindow final : public vcl::Window
{
    ModulWindow&    rModulWindow;
    tools::Long            nCurYOffset;
    sal_uInt16      nMarkerPos;
    BreakPointList  aBreakPointList;
    bool            bErrorMarker;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) override;

    void setBackgroundColor(Color aColor);

    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void ShowMarker(vcl::RenderContext& rRenderContext);
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;

    bool            SyncYOffset();

public:
                    BreakPointWindow (vcl::Window* pParent, ModulWindow*);

    void            SetMarkerPos( sal_uInt16 nLine, bool bErrorMarker = false );
    void            SetNoMarker ();

    void            DoScroll( tools::Long nVertScroll );
    tools::Long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};

class WatchWindow final : public DockingWindow
{
private:
    std::unique_ptr<weld::Container> m_xTitleArea;
    std::unique_ptr<weld::Label> m_xTitle;
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xRemoveWatchButton;
    std::unique_ptr<weld::TreeView> m_xTreeListBox;

    ImplSVEvent* m_nUpdateWatchesId;
    OUString aEditingRes;

    virtual void    Resize() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

    SbxBase* ImplGetSBXForEntry(const weld::TreeIter& rEntry, bool& rbArrayElement);

    void implEnableChildren(const weld::TreeIter& rEntry, bool bEnable);

    DECL_STATIC_LINK(WatchWindow, ButtonHdl, weld::Button&, void);
    DECL_LINK(TreeListHdl, weld::TreeView&, void);
    DECL_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(EditingEntryHdl, const weld::TreeIter&, bool);
    typedef std::pair<const weld::TreeIter&, OUString> IterString;
    DECL_LINK(EditedEntryHdl, const IterString&, bool);
    DECL_LINK(ExecuteUpdateWatches, void*, void);

public:
    explicit WatchWindow (Layout* pParent);
    virtual ~WatchWindow() override;
    virtual void    dispose() override;

    void            AddWatch( const OUString& rVName );
    void            RemoveSelectedWatch();
    void            UpdateWatches(bool bBasicStopped = false);
};

class StackWindow : public DockingWindow
{
private:
    std::unique_ptr<weld::Label> m_xTitle;
    std::unique_ptr<weld::TreeView> m_xTreeListBox;

protected:
    virtual void    Resize() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;

public:
    explicit StackWindow (Layout* pParent);
    virtual ~StackWindow() override;
    virtual void    dispose() override;

    void            UpdateCalls();
};


class ComplexEditorWindow final : public vcl::Window
{
private:
    VclPtr<BreakPointWindow> aBrkWindow;
    VclPtr<LineNumberWindow> aLineNumberWindow;
    VclPtr<EditorWindow>     aEdtWindow;
    VclPtr<ScrollBar>        aEWVScrollBar;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) override;

    virtual void        Resize() override;
    DECL_LINK( ScrollHdl, ScrollBar*, void );

public:
    explicit ComplexEditorWindow( ModulWindow* pParent );
    virtual             ~ComplexEditorWindow() override;
    virtual void        dispose() override;
    BreakPointWindow&   GetBrkWindow()      { return *aBrkWindow; }
    LineNumberWindow&   GetLineNumberWindow() { return *aLineNumberWindow; }
    EditorWindow&       GetEdtWindow()      { return *aEdtWindow; }
    ScrollBar&          GetEWVScrollBar()   { return *aEWVScrollBar; }

    void SetLineNumberDisplay(bool b);
};


class ModulWindow: public BaseWindow
{
private:
    ModulWindowLayout&  m_rLayout;
    StarBASICRef        m_xBasic;
    short               m_nValid;
    VclPtr<ComplexEditorWindow> m_aXEditorWindow;
    BasicStatus         m_aStatus;
    SbModuleRef         m_xModule;
    OUString            m_sCurPath;
    OUString            m_aModule;

    void                CheckCompileBasic();
    void                BasicExecute();

    sal_Int32           FormatAndPrint( Printer* pPrinter, sal_Int32 nPage );
    SbModuleRef const & XModule();
protected:
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;
    virtual void    DoInit() override;
    virtual void    DoScroll( ScrollBar* pCurScrollBar ) override;

public:
    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, OUString const & aModule );

                    virtual ~ModulWindow() override;
    virtual void    dispose() override;

    virtual void    ExecuteCommand (SfxRequest& rReq) override;
    virtual void    ExecuteGlobal (SfxRequest& rReq) override;
    virtual void    GetState( SfxItemSet& ) override;
    virtual void    StoreData() override;
    virtual void    UpdateData() override;
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) override;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) override;
    virtual OUString  GetTitle() override;
    virtual EntryDescriptor CreateEntryDescriptor() override;
    virtual bool    AllowUndo() override;
    virtual void    SetReadOnly (bool bReadOnly) override;
    virtual bool    IsReadOnly() override;

    StarBASIC*      GetBasic() { XModule(); return m_xBasic.get(); }

    SbModule*       GetSbModule() { return m_xModule.get(); }
    void            SetSbModule( SbModule* pModule ) { m_xModule = pModule; }
    OUString        GetSbModuleName();

    void            CompileBasic();
    void            BasicRun();
    void            BasicStepOver();
    void            BasicStepInto();
    void            BasicStepOut();
    void            BasicStop();
    void            BasicToggleBreakPoint();
    void            BasicToggleBreakPointEnabled();
    void            ManageBreakPoints();
    void            UpdateBreakPoint( const BreakPoint& rBrk );
    void            BasicAddWatch();

    void            BasicErrorHdl( StarBASIC const * pBasic );
    BasicDebugFlags BasicBreakHdl();
    void            AssertValidEditEngine();

    void            LoadBasic();
    void            SaveBasicSource();
    void            ImportDialog();

    void            EditMacro( const OUString& rMacroName );

    void            ToggleBreakPoint( sal_uInt16 nLine );

    BasicStatus&    GetBasicStatus() { return m_aStatus; }

    virtual bool    IsModified () override;
    bool            IsPasteAllowed ();

    void            ShowCursor( bool bOn );

    virtual SearchOptionFlags GetSearchOptions() override;
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false) override;

    EditorWindow&       GetEditorWindow()       { return m_aXEditorWindow->GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return m_aXEditorWindow->GetBrkWindow(); }
    LineNumberWindow&   GetLineNumberWindow()   { return m_aXEditorWindow->GetLineNumberWindow(); }
    ScrollBar&          GetEditVScrollBar()     { return m_aXEditorWindow->GetEWVScrollBar(); }
    ExtTextEngine*      GetEditEngine()         { return GetEditorWindow().GetEditEngine(); }
    TextView*           GetEditView()           { return GetEditorWindow().GetEditView(); }
    BreakPointList&     GetBreakPoints()        { return GetBreakPointWindow().GetBreakPoints(); }
    ModulWindowLayout&  GetLayout ()            { return m_rLayout; }

    virtual void        BasicStarted() override;
    virtual void        BasicStopped() override;

    virtual SfxUndoManager*
                        GetUndoManager() override;

    const OUString&         GetModule() const { return m_aModule; }
    void                    SetModule( const OUString& aModule ) { m_aModule = aModule; }

    virtual void Activating () override;
    virtual void Deactivating () override;

    virtual void OnNewDocument () override;
    virtual char const* GetHid () const override;
    virtual ItemType GetType () const override;
    virtual bool HasActiveEditor () const override;

    void UpdateModule ();
};

class ModulWindowLayout: public Layout
{
public:
    ModulWindowLayout (vcl::Window* pParent, ObjectCatalog&);
    virtual ~ModulWindowLayout() override;
    virtual void dispose() override;
public:
    // Layout:
    virtual void Activating (BaseWindow&) override;
    virtual void Deactivating () override;
    virtual void GetState (SfxItemSet&, unsigned nWhich) override;
    virtual void UpdateDebug (bool bBasicStopped) override;
public:
    void BasicAddWatch (OUString const&);
    void BasicRemoveWatch ();
    Color const & GetSyntaxBackgroundColor () const { return aSyntaxColors.GetBackgroundColor(); }
    Color const & GetFontColor () const { return aSyntaxColors.GetFontColor(); }
    Color const & GetSyntaxColor (TokenType eType) const { return aSyntaxColors.GetColor(eType); }

protected:
    // Window:
    virtual void Paint (vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    // Layout:
    virtual void OnFirstSize (tools::Long nWidth, tools::Long nHeight) override;

private:
    // main child window
    VclPtr<ModulWindow> pChild;
    // dockable windows
    VclPtr<WatchWindow> aWatchWindow;
    VclPtr<StackWindow> aStackWindow;
    ObjectCatalog& rObjectCatalog;

    // SyntaxColors -- stores Basic syntax highlighting colors
    class SyntaxColors : public utl::ConfigurationListener
    {
    public:
        SyntaxColors ();
        virtual ~SyntaxColors () override;
    public:
        void SetActiveEditor (EditorWindow* pEditor_) { pEditor = pEditor_; }
    public:
        Color const & GetBackgroundColor () const { return m_aBackgroundColor; };
        Color const & GetFontColor () const { return m_aFontColor; }
        Color const & GetColor(TokenType eType) const { return aColors[eType]; }

    private:
        virtual void ConfigurationChanged (utl::ConfigurationBroadcaster*, ConfigurationHints) override;
        void NewConfig (bool bFirst);

    private:
        Color m_aBackgroundColor;
        Color m_aFontColor;
        // the color values (the indexes are TokenType, see comphelper/syntaxhighlight.hxx)
        o3tl::enumarray<TokenType, Color> aColors;
        // the configuration
        svtools::ColorConfig aConfig;
        // the active editor
        VclPtr<EditorWindow> pEditor;

    } aSyntaxColors;
};

class CodeCompleteWindow final : public InterimItemWindow
{
private:
    VclPtr<EditorWindow> pParent; // parent window
    TextSelection m_aTextSelection;
    std::unique_ptr<weld::TreeView> m_xListBox;

    /* a buffer to build up function name when typing
     * a function name, used for showing/hiding listbox values
     * */
    OUStringBuffer aFuncBuffer;

    void InsertSelectedEntry(); // insert the selected entry
    void SetMatchingEntries(); // sets the visible entries based on aFuncBuffer variable
    TextView* GetParentEditView();

    DECL_LINK(ImplDoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(ImplSelectHdl, weld::TreeView&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    explicit CodeCompleteWindow( EditorWindow* pPar );
    virtual ~CodeCompleteWindow() override;
    virtual void dispose() override;

    void InsertEntry( const OUString& aStr );
    void ClearListBox();
    void SetTextSelection( const TextSelection& aSel );
    const TextSelection& GetTextSelection() const { return m_aTextSelection;}
    void ResizeAndPositionListBox();
    void SelectFirstEntry(); //selects first entry in ListBox

    /*
     * clears if typed anything, then hides
     * the window, clear internal variables
     * */
    void ClearAndHide();
    void HideAndRestoreFocus();

    bool HandleKeyInput(const KeyEvent& rKeyEvt);
};

class UnoTypeCodeCompletetor
{
private:
    css::uno::Reference< css::reflection::XIdlClass > xClass;
    bool bCanComplete;

    bool CheckField( const OUString& sFieldName );
    bool CheckMethod( const OUString& sMethName );

public:
    UnoTypeCodeCompletetor( const std::vector< OUString >& aVect, const OUString& sVarType );

    std::vector< OUString > GetXIdlClassMethods() const;
    std::vector< OUString > GetXIdlClassFields() const;

    bool CanCodeComplete() const { return bCanComplete;}
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
