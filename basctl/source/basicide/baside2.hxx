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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BASIDE2_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BASIDE2_HXX

#include "layout.hxx"
#include "bastype3.hxx"
#include "basidesh.hxx"
#include "breakpoint.hxx"
#include "linenumberwindow.hxx"

class ExtTextEngine;
class ExtTextView;
class SvxSearchItem;
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>

#include <vcl/button.hxx>
#include <basic/sbmod.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/idle.hxx>

#include <sfx2/progress.hxx>
#include <o3tl/enumarray.hxx>

#include <set>

#include <vcl/textdata.hxx>
#include <basic/codecompletecache.hxx>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/syntaxhighlight.hxx>
#include <com/sun/star/reflection/XIdlReflection.hpp>

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

namespace basctl
{

class ObjectCatalog;
class CodeCompleteListBox;
class CodeCompleteWindow;

// #108672 Helper functions to get/set text in TextEngine
// using the stream interface (get/setText() only supports
// tools Strings limited to 64K).
// defined in baside2b.cxx
OUString getTextEngineText (ExtTextEngine&);
void setTextEngineText (ExtTextEngine&, OUString const&);

class EditorWindow : public vcl::Window, public SfxListener
{
friend class CodeCompleteListBox;
private:
    class ChangesListener;

    std::unique_ptr<ExtTextView>   pEditView;
    std::unique_ptr<ExtTextEngine> pEditEngine;
    ModulWindow&                     rModulWindow;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex                        mutex_;
    css::uno::Reference< css::beans::XMultiPropertySet >
                                      notifier_;

    long            nCurTextWidth;

    SyntaxHighlighter   aHighlighter;
    Idle                aSyntaxIdle;
    std::set<sal_uInt16>       aSyntaxLineTable;
    DECL_LINK_TYPED(SyntaxTimerHdl, Idle *, void);

    // progress bar
    class ProgressInfo;
    std::unique_ptr<ProgressInfo> pProgress;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) override;

    using           Window::Notify;
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
    TextSelection GetLastHighlightPortionTextSelection();

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;
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
                    virtual ~EditorWindow();
    virtual void    dispose() override;

    ExtTextEngine*  GetEditEngine() const   { return pEditEngine.get(); }
    ExtTextView*    GetEditView() const     { return pEditView.get(); }

    void            CreateProgress( const OUString& rText, sal_uLong nRange );
    void            DestroyProgress();

    void            ParagraphInsertedDeleted( sal_uLong nNewPara, bool bInserted );
    void            DoDelayedSyntaxHighlight( sal_uLong nPara );

    void            CreateEditEngine();
    void            SetScrollBarRanges();
    void            InitScrollBars();

    void            ForceSyntaxTimeout();
    void            SetSourceInBasic();

    bool            CanModify() { return ImpCanModify(); }

    void            UpdateSyntaxHighlighting ();

    bool            GetProcedureName(OUString& rLine, OUString& rProcType, OUString& rProcName) const;
};


class BreakPointWindow : public vcl::Window
{
private:
    ModulWindow&    rModulWindow;
    long            nCurYOffset;
    sal_uInt16      nMarkerPos;
    BreakPointList  aBreakPointList;
    bool            bErrorMarker;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) override;

    void setBackgroundColor(Color aColor);

protected:
    virtual void    Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override;
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void ShowMarker(vcl::RenderContext& rRenderContext);
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;

    bool            SyncYOffset();

public:
                    BreakPointWindow (vcl::Window* pParent, ModulWindow*);

    void            SetMarkerPos( sal_uInt16 nLine, bool bErrorMarker = false );
    void            SetNoMarker ();

    void            DoScroll( long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};


class WatchTreeListBox : public SvHeaderTabListBox
{
    OUString aEditingRes;

protected:
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  ) override;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    bool            ImplBasicEntryEdited( SvTreeListEntry* pEntry, const OUString& rResult );
    SbxBase*        ImplGetSBXForEntry( SvTreeListEntry* pEntry, bool& rbArrayElement );

public:
    WatchTreeListBox( vcl::Window* pParent, WinBits nWinBits );
    virtual ~WatchTreeListBox();
    virtual void    dispose() override;

    void            RequestingChildren( SvTreeListEntry * pParent ) override;
    void            UpdateWatches( bool bBasicStopped = false );

    using           SvTabListBox::SetTabs;
    virtual void    SetTabs() override;
};


class WatchWindow : public DockingWindow
{
private:
    OUString            aWatchStr;
    VclPtr<ExtendedEdit>        aXEdit;
    VclPtr<ImageButton>         aRemoveWatchButton;
    VclPtr<WatchTreeListBox>    aTreeListBox;
    VclPtr<HeaderBar>           aHeaderBar;

protected:
    virtual void    Resize() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

    DECL_LINK_TYPED( ButtonHdl, Button *, void );
    DECL_LINK_TYPED(TreeListHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED( implEndDragHdl, HeaderBar *, void );
    DECL_LINK_TYPED( EditAccHdl, Accelerator&, void );


public:
    explicit WatchWindow (Layout* pParent);
    virtual ~WatchWindow();
    virtual void    dispose() override;

    void            AddWatch( const OUString& rVName );
    void            RemoveSelectedWatch();
    void            UpdateWatches( bool bBasicStopped = false );
};


class StackWindow : public DockingWindow
{
private:
    VclPtr<SvTreeListBox>  aTreeListBox;
    OUString               aStackStr;

protected:
    virtual void    Resize() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

public:
    explicit StackWindow (Layout* pParent);
    virtual ~StackWindow();
    virtual void    dispose() override;

    void            UpdateCalls();
};


class ComplexEditorWindow : public vcl::Window
{
private:
    VclPtr<BreakPointWindow> aBrkWindow;
    VclPtr<LineNumberWindow> aLineNumberWindow;
    VclPtr<EditorWindow>     aEdtWindow;
    VclPtr<ScrollBar>        aEWVScrollBar;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) override;

protected:
    virtual void        Resize() override;
    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );

public:
    explicit ComplexEditorWindow( ModulWindow* pParent );
    virtual             ~ComplexEditorWindow();
    virtual void        dispose() override;
    BreakPointWindow&   GetBrkWindow()      { return *aBrkWindow.get(); }
    LineNumberWindow&   GetLineNumberWindow() { return *aLineNumberWindow.get(); }
    EditorWindow&       GetEdtWindow()      { return *aEdtWindow.get(); }
    ScrollBar&          GetEWVScrollBar()   { return *aEWVScrollBar.get(); }

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

    static void         GoOnTop();

    sal_Int32           FormatAndPrint( Printer* pPrinter, sal_Int32 nPage = -1 );
    SbModuleRef const & XModule();
protected:
    virtual void    Resize() override;
    virtual void    GetFocus() override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;
    virtual void    DoInit() override;
    virtual void    DoScroll( ScrollBar* pCurScrollBar ) override;

public:
    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, OUString& aModule );

                    virtual ~ModulWindow();
    virtual void    dispose() override;

    virtual void    ExecuteCommand (SfxRequest& rReq) override;
    virtual void    ExecuteGlobal (SfxRequest& rReq) override;
    virtual void    GetState( SfxItemSet& ) override;
    virtual void    StoreData() override;
    virtual void    UpdateData() override;
    virtual bool    CanClose() override;
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) override;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) override;
    virtual OUString  GetTitle() override;
    virtual EntryDescriptor CreateEntryDescriptor() override;
    virtual bool    AllowUndo() override;
    virtual void    SetReadOnly (bool bReadOnly) override;
    virtual bool    IsReadOnly() override;

    StarBASIC*      GetBasic() { XModule(); return m_xBasic; }

    SbModule*       GetSbModule() { return m_xModule; }
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

    bool            BasicErrorHdl( StarBASIC* pBasic );
    long            BasicBreakHdl( StarBASIC* pBasic );
    void            AssertValidEditEngine();

    void            LoadBasic();
    void            SaveBasicSource();
    void            ImportDialog();

    void            EditMacro( const OUString& rMacroName );

    void            ToggleBreakPoint( sal_uLong nLine );

    BasicStatus&    GetBasicStatus() { return m_aStatus; }

    virtual bool    IsModified () override;
    virtual bool    IsPasteAllowed () override;

    void            ShowCursor( bool bOn );

    virtual SearchOptionFlags GetSearchOptions() override;
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false) override;

    EditorWindow&       GetEditorWindow()       { return m_aXEditorWindow->GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return m_aXEditorWindow->GetBrkWindow(); }
    LineNumberWindow&   GetLineNumberWindow()   { return m_aXEditorWindow->GetLineNumberWindow(); }
    ScrollBar&          GetEditVScrollBar()     { return m_aXEditorWindow->GetEWVScrollBar(); }
    ExtTextEngine*      GetEditEngine()         { return GetEditorWindow().GetEditEngine(); }
    ExtTextView*        GetEditView()           { return GetEditorWindow().GetEditView(); }
    BreakPointList&     GetBreakPoints()        { return GetBreakPointWindow().GetBreakPoints(); }
    ModulWindowLayout&  GetLayout ()            { return m_rLayout; }

    virtual void        BasicStarted() override;
    virtual void        BasicStopped() override;

    virtual ::svl::IUndoManager*
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
    virtual ~ModulWindowLayout();
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
    Color GetSyntaxColor (TokenType eType) const { return aSyntaxColors.GetColor(eType); }

protected:
    // Window:
    virtual void Paint (vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    // Layout:
    virtual void OnFirstSize (long nWidth, long nHeight) override;

private:
    // main child window
    VclPtr<ModulWindow> pChild;
    // dockable windows
    VclPtr<WatchWindow> aWatchWindow;
    VclPtr<StackWindow> aStackWindow;
    ObjectCatalog& rObjectCatalog;
private:
    virtual void DataChanged (DataChangedEvent const& rDCEvt) override;
private:
    // SyntaxColors -- stores Basic syntax highlighting colors
    class SyntaxColors : public utl::ConfigurationListener
    {
    public:
        SyntaxColors ();
        virtual ~SyntaxColors ();
    public:
        void SetActiveEditor (EditorWindow* pEditor_) { pEditor = pEditor_; }
        void SettingsChanged ();
    public:
        Color GetColor (TokenType eType) const { return aColors[eType]; }

    private:
        virtual void ConfigurationChanged (utl::ConfigurationBroadcaster*, sal_uInt32) override;
        void NewConfig (bool bFirst);

    private:
        // the color values (the indexes are TokenType, see comphelper/syntaxhighlight.hxx)
        o3tl::enumarray<TokenType, Color> aColors;
        // the configuration
        svtools::ColorConfig aConfig;
        // the active editor
        VclPtr<EditorWindow> pEditor;

    } aSyntaxColors;
};

class CodeCompleteListBox: public ListBox
{
friend class CodeCompleteWindow;
friend class EditorWindow;
private:
    OUStringBuffer aFuncBuffer;
    /* a buffer to build up function name when typing
     * a function name, used for showing/hiding listbox values
     * */
    VclPtr<CodeCompleteWindow> pCodeCompleteWindow; // parent window

    void SetMatchingEntries(); // sets the visible entries based on aFuncBuffer variable
    void HideAndRestoreFocus();
    ExtTextView* GetParentEditView();

public:
    explicit CodeCompleteListBox( CodeCompleteWindow* pPar );
    virtual ~CodeCompleteListBox();
    virtual void dispose() override;
    void InsertSelectedEntry(); //insert the selected entry

    DECL_LINK_TYPED(ImplDoubleClickHdl, ListBox&, void);
    DECL_LINK_TYPED(ImplSelectHdl, ListBox&, void);

protected:
    virtual void KeyInput( const KeyEvent& rKeyEvt ) override;
};

class CodeCompleteWindow: public vcl::Window
{
friend class CodeCompleteListBox;
private:
    VclPtr<EditorWindow> pParent; // parent window
    TextSelection aTextSelection;
    VclPtr<CodeCompleteListBox> pListBox;

    void InitListBox(); // initialize the ListBox

public:
    explicit CodeCompleteWindow( EditorWindow* pPar );
    virtual ~CodeCompleteWindow();
    virtual void dispose() override;

    void InsertEntry( const OUString& aStr );
    void ClearListBox();
    void SetTextSelection( const TextSelection& aSel );
    const TextSelection& GetTextSelection() const { return aTextSelection;}
    void ResizeAndPositionListBox();
    void SelectFirstEntry(); //selects first entry in ListBox
    void ClearAndHide();
    /*
     * clears if typed anything, then hides
     * the window, clear internal variables
     * */
    CodeCompleteListBox* GetListBox(){return pListBox;}
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
    ~UnoTypeCodeCompletetor(){}

    std::vector< OUString > GetXIdlClassMethods() const;
    std::vector< OUString > GetXIdlClassFields() const;

    bool CanCodeComplete() const { return bCanComplete;}
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BASIDE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
