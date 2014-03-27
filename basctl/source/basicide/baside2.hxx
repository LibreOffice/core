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

#ifndef BASCTL_BASIDE2_HXX
#define BASCTL_BASIDE2_HXX

#include "layout.hxx"
#include "bastypes.hxx"
#include "bastype3.hxx"
#include "basidesh.hxx"
#include "breakpoint.hxx"
#include "linenumberwindow.hxx"

#include <svheader.hxx>

class ExtTextEngine;
class ExtTextView;
class SvxSearchItem;
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>

#include <vcl/button.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <vcl/split.hxx>
#include <svl/lstner.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>

#include <sfx2/progress.hxx>
#include <unotools/options.hxx>
#include <rtl/ref.hxx>

#include <set>
#include <boost/scoped_ptr.hpp>

#include <vcl/textdata.hxx>
#include <basic/codecompletecache.hxx>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/syntaxhighlight.hxx>
#include <comphelper/configurationhelper.hxx>
#include <com/sun/star/reflection/XIdlReflection.hpp>

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

namespace basctl
{

class ObjectCatalog;
class CodeCompleteListBox;
class CodeCompleteWindow;

DBG_NAMEEX( ModulWindow )

// #108672 Helper functions to get/set text in TextEngine
// using the stream interface (get/setText() only supports
// tools Strings limited to 64K).
// defined in baside2b.cxx
OUString getTextEngineText (ExtTextEngine&);
void setTextEngineText (ExtTextEngine&, OUString const&);

class EditorWindow : public Window, public SfxListener
{
friend class CodeCompleteListBox;
private:
    class ChangesListener;

    boost::scoped_ptr<ExtTextView> pEditView;
    boost::scoped_ptr<ExtTextEngine> pEditEngine;
    ModulWindow&    rModulWindow;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex mutex_;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >
        notifier_;

    long            nCurTextWidth;

    SyntaxHighlighter   aHighlighter;
    Timer           aSyntaxIdleTimer;
    typedef std::set<sal_uInt16> SyntaxLineSet;
    SyntaxLineSet   aSyntaxLineTable;
    DECL_LINK(SyntaxTimerHdl, void *);

    // progress bar
    class ProgressInfo;
    boost::scoped_ptr<ProgressInfo> pProgress;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) SAL_OVERRIDE;

    using           Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    void            ImpDoHighlight( sal_uLong nLineOff );
    void            ImplSetFont();

    bool            bHighlightning;
    bool            bDoSyntaxHighlight;
    bool            bDelayHighlight;

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
    GetComponentInterface(sal_Bool bCreate = true) SAL_OVERRIDE;
    CodeCompleteDataCache aCodeCompleteCache;
    boost::scoped_ptr< CodeCompleteWindow > pCodeCompleteWnd;
    OUString GetActualSubName( sal_uLong nLine ); // gets the actual subroutine name according to line number
    void SetupAndShowCodeCompleteWnd(const std::vector< OUString >& aEntryVect, TextSelection aSel );
    void HandleAutoCorrect();
    void HandleAutoCloseParen();
    void HandleAutoCloseDoubleQuotes();
    void HandleCodeCompletion();
    void HandleProcedureCompletion();
    TextSelection GetLastHighlightPortionTextSelection();

protected:
    virtual void    Paint( const Rectangle& ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

    void            DoSyntaxHighlight( sal_uLong nPara );
    OUString        GetWordAtCursor();
    bool            ImpCanModify();

public:
                    EditorWindow (Window* pParent, ModulWindow*);
                    ~EditorWindow();

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
    bool            SetSourceInBasic();

    bool            CanModify() { return ImpCanModify(); }

    void            UpdateSyntaxHighlighting ();
};


class BreakPointWindow : public Window
{
private:
    ModulWindow&    rModulWindow;
    long            nCurYOffset;
    sal_uInt16      nMarkerPos;
    BreakPointList  aBreakPointList;
    bool            bErrorMarker;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) SAL_OVERRIDE;

    void setBackgroundColor(Color aColor);

protected:
    virtual void    Paint( const Rectangle& ) SAL_OVERRIDE;
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void            ShowMarker( bool bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;

    bool            SyncYOffset();

public:
                    BreakPointWindow (Window* pParent, ModulWindow*);
                    ~BreakPointWindow();

    void            SetMarkerPos( sal_uInt16 nLine, bool bErrorMarker = false );
    void            SetNoMarker ();

    void            DoScroll( long nHorzScroll, long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};


class WatchTreeListBox : public SvHeaderTabListBox
{
    OUString aEditingRes;

protected:
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    bool            ImplBasicEntryEdited( SvTreeListEntry* pEntry, const OUString& rResult );
    SbxBase*        ImplGetSBXForEntry( SvTreeListEntry* pEntry, bool& rbArrayElement );

public:
    WatchTreeListBox( Window* pParent, WinBits nWinBits );
    ~WatchTreeListBox();

    void            RequestingChildren( SvTreeListEntry * pParent ) SAL_OVERRIDE;
    void            UpdateWatches( bool bBasicStopped = false );

    using           SvTabListBox::SetTabs;
    virtual void    SetTabs() SAL_OVERRIDE;
};



class WatchWindow : public DockingWindow
{
private:
    OUString            aWatchStr;
    ExtendedEdit        aXEdit;
    ImageButton         aRemoveWatchButton;
    WatchTreeListBox    aTreeListBox;
    HeaderBar           aHeaderBar;

protected:
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    DECL_LINK( ButtonHdl, ImageButton * );
    DECL_LINK(TreeListHdl, void *);
    DECL_LINK( implEndDragHdl, HeaderBar * );
    DECL_LINK( EditAccHdl, Accelerator * );


public:
                    WatchWindow (Layout* pParent);
                    ~WatchWindow();

    void            AddWatch( const OUString& rVName );
    bool            RemoveSelectedWatch();
    void            UpdateWatches( bool bBasicStopped = false );

    WatchTreeListBox&   GetWatchTreeListBox() { return aTreeListBox; }
};


class StackWindow : public DockingWindow
{
private:
    SvTreeListBox   aTreeListBox;
    OUString        aStackStr;

protected:
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;

public:
                    StackWindow (Layout* pParent);
                    ~StackWindow();

    void            UpdateCalls();
};


class ComplexEditorWindow : public Window
{
private:
    BreakPointWindow    aBrkWindow;
    LineNumberWindow    aLineNumberWindow;
    EditorWindow        aEdtWindow;
    ScrollBar           aEWVScrollBar;

    virtual void DataChanged(DataChangedEvent const & rDCEvt) SAL_OVERRIDE;

protected:
    virtual void        Resize() SAL_OVERRIDE;
    DECL_LINK( ScrollHdl, ScrollBar * );

public:
                        ComplexEditorWindow( ModulWindow* pParent );

    BreakPointWindow&   GetBrkWindow()      { return aBrkWindow; }
    LineNumberWindow&   GetLineNumberWindow() { return aLineNumberWindow; }
    EditorWindow&       GetEdtWindow()      { return aEdtWindow; }
    ScrollBar&          GetEWVScrollBar()   { return aEWVScrollBar; }

    void SetLineNumberDisplay(bool b);
};


class ModulWindow: public BaseWindow
{
private:
    ModulWindowLayout&  rLayout;
    StarBASICRef        xBasic;
    short               nValid;
    ComplexEditorWindow aXEditorWindow;
    BasicStatus         aStatus;
    SbModuleRef         xModule;
    OUString            aCurPath;
    OUString            m_aModule;

    void                CheckCompileBasic();
    bool                BasicExecute();

    void                GoOnTop();

    sal_Int32           FormatAndPrint( Printer* pPrinter, sal_Int32 nPage = -1 );
    SbModuleRef         XModule();
protected:
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& ) SAL_OVERRIDE;
    virtual void    DoInit() SAL_OVERRIDE;
    virtual void    DoScroll( ScrollBar* pCurScrollBar ) SAL_OVERRIDE;

public:
                    TYPEINFO_OVERRIDE();

    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, const OUString& aLibName, const OUString& aName, OUString& aModule );

                    ~ModulWindow();

    virtual void    ExecuteCommand (SfxRequest& rReq) SAL_OVERRIDE;
    virtual void    ExecuteGlobal (SfxRequest& rReq) SAL_OVERRIDE;
    virtual void    GetState( SfxItemSet& ) SAL_OVERRIDE;
    virtual void    StoreData() SAL_OVERRIDE;
    virtual void    UpdateData() SAL_OVERRIDE;
    virtual bool    CanClose() SAL_OVERRIDE;
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) SAL_OVERRIDE;
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter ) SAL_OVERRIDE;
    virtual OUString  GetTitle() SAL_OVERRIDE;
    virtual EntryDescriptor CreateEntryDescriptor() SAL_OVERRIDE;
    virtual bool    AllowUndo() SAL_OVERRIDE;
    virtual void    SetReadOnly (bool bReadOnly) SAL_OVERRIDE;
    virtual bool    IsReadOnly() SAL_OVERRIDE;

    StarBASIC*      GetBasic() { XModule(); return xBasic; }

    SbModule*       GetSbModule() { return xModule; }
    void            SetSbModule( SbModule* pModule ) { xModule = pModule; }
    OUString        GetSbModuleName();

    bool            CompileBasic();
    bool            BasicRun();
    bool            BasicStepOver();
    bool            BasicStepInto();
    bool            BasicStepOut();
    void            BasicStop();
    bool            BasicToggleBreakPoint();
    void            BasicToggleBreakPointEnabled();
    void            ManageBreakPoints();
    void            UpdateBreakPoint( const BreakPoint& rBrk );
    void            BasicAddWatch();
    void            BasicRemoveWatch();

    bool            BasicErrorHdl( StarBASIC* pBasic );
    long            BasicBreakHdl( StarBASIC* pBasic );
    void            AssertValidEditEngine();

    bool            LoadBasic();
    bool            SaveBasicSource();
    bool            ImportDialog();

    void            EditMacro( const OUString& rMacroName );

    bool            ToggleBreakPoint( sal_uLong nLine );

    BasicStatus&    GetBasicStatus() { return aStatus; }

    virtual bool    IsModified () SAL_OVERRIDE;
    virtual bool    IsPasteAllowed () SAL_OVERRIDE;

    void            FrameWindowMoved();
    void            ShowCursor( bool bOn );

    virtual sal_uInt16  GetSearchOptions() SAL_OVERRIDE;
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false) SAL_OVERRIDE;

    EditorWindow&       GetEditorWindow()       { return aXEditorWindow.GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return aXEditorWindow.GetBrkWindow(); }
    LineNumberWindow&   GetLineNumberWindow()   { return aXEditorWindow.GetLineNumberWindow(); }
    ScrollBar&          GetEditVScrollBar()     { return aXEditorWindow.GetEWVScrollBar(); }
    ExtTextEngine*      GetEditEngine()         { return GetEditorWindow().GetEditEngine(); }
    ExtTextView*        GetEditView()           { return GetEditorWindow().GetEditView(); }
    BreakPointList&     GetBreakPoints()        { return GetBreakPointWindow().GetBreakPoints(); }
    ModulWindowLayout&  GetLayout ()            { return rLayout; }

    virtual void        BasicStarted() SAL_OVERRIDE;
    virtual void        BasicStopped() SAL_OVERRIDE;

    virtual ::svl::IUndoManager*
                        GetUndoManager() SAL_OVERRIDE;

    const OUString&         GetModule() const { return m_aModule; }
    void                    SetModule( const OUString& aModule ) { m_aModule = aModule; }

    virtual void Activating () SAL_OVERRIDE;
    virtual void Deactivating () SAL_OVERRIDE;

    virtual void OnNewDocument () SAL_OVERRIDE;
    virtual char const* GetHid () const SAL_OVERRIDE;
    virtual ItemType GetType () const SAL_OVERRIDE;
    virtual bool HasActiveEditor () const SAL_OVERRIDE;

    void UpdateModule ();
};

class ModulWindowLayout: public Layout
{
public:
    ModulWindowLayout (Window* pParent, ObjectCatalog&);
public:
    // Layout:
    virtual void Activating (BaseWindow&) SAL_OVERRIDE;
    virtual void Deactivating () SAL_OVERRIDE;
    virtual void GetState (SfxItemSet&, unsigned nWhich) SAL_OVERRIDE;
    virtual void UpdateDebug (bool bBasicStopped) SAL_OVERRIDE;
public:
    void BasicAddWatch (OUString const&);
    void BasicRemoveWatch ();
    Color GetSyntaxColor (TokenTypes eType) const { return aSyntaxColors.GetColor(eType); }

protected:
    // Window:
    virtual void Paint (const Rectangle& rRect) SAL_OVERRIDE;
    // Layout:
    virtual void OnFirstSize (long nWidth, long nHeight) SAL_OVERRIDE;

private:
    // main child window
    ModulWindow* pChild;
    // dockable windows
    WatchWindow aWatchWindow;
    StackWindow aStackWindow;
    ObjectCatalog& rObjectCatalog;
private:
    virtual void DataChanged (DataChangedEvent const& rDCEvt) SAL_OVERRIDE;
private:
    // SyntaxColors -- stores Basic syntax highlighting colors
    class SyntaxColors : public utl::ConfigurationListener
    {
    public:
        SyntaxColors ();
        ~SyntaxColors ();
    public:
        void SetActiveEditor (EditorWindow* pEditor_) { pEditor = pEditor_; }
        void SettingsChanged ();
    public:
        Color GetColor (TokenTypes eType) const { return aColors[eType]; }

    private:
        virtual void ConfigurationChanged (utl::ConfigurationBroadcaster*, sal_uInt32) SAL_OVERRIDE;
        void NewConfig (bool bFirst);

    private:
        // the color values (the indexes are TokenTypes, see comphelper/syntaxhighlight.hxx)
        Color aColors[TT_KEYWORDS + 1];
        // the configuration
        svtools::ColorConfig aConfig;
        // the active editor
        EditorWindow* pEditor;

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
    CodeCompleteWindow* pCodeCompleteWindow; // parent window

    void SetMatchingEntries(); // sets the visible entries based on aFuncBuffer variable
    void HideAndRestoreFocus();
    ExtTextView* GetParentEditView();

public:
    CodeCompleteListBox( CodeCompleteWindow* pPar );
    void InsertSelectedEntry(); //insert the selected entry

    DECL_LINK(ImplDoubleClickHdl, void*);
    DECL_LINK(ImplSelectHdl, void*);

protected:
    virtual void KeyInput( const KeyEvent& rKeyEvt ) SAL_OVERRIDE;
};

class CodeCompleteWindow: public Window
{
friend class CodeCompleteListBox;
private:
    EditorWindow* pParent; // parent window
    TextSelection aTextSelection;
    CodeCompleteListBox* pListBox;

    void InitListBox(); // initialize the ListBox

public:
    CodeCompleteWindow( EditorWindow* pPar );
    ~CodeCompleteWindow(){ delete pListBox; }

    void InsertEntry( const OUString& aStr );
    void ClearListBox();
    void SetTextSelection( const TextSelection& aSel );
    const TextSelection& GetTextSelection() const;
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
    ::css::uno::Reference< ::css::lang::XMultiServiceFactory > xFactory;
    ::css::uno::Reference< ::css::reflection::XIdlReflection > xRefl;
    ::css::uno::Reference< ::css::reflection::XIdlClass > xClass;
    bool bCanComplete;

    bool CheckField( const OUString& sFieldName );
    bool CheckMethod( const OUString& sMethName );

public:
    UnoTypeCodeCompletetor( const std::vector< OUString >& aVect, const OUString& sVarType );
    ~UnoTypeCodeCompletetor(){}

    std::vector< OUString > GetXIdlClassMethods() const;
    std::vector< OUString > GetXIdlClassFields() const;

    bool CanCodeComplete() const;
};

} // namespace basctl

#endif // BASCTL_BASIDE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
