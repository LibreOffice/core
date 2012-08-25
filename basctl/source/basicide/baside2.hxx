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
#include <bastypes.hxx>
#include <bastype3.hxx>
#include <basidesh.hxx>

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
#include "svl/lstner.hxx"
#include <svtools/colorcfg.hxx>

#include <sfx2/progress.hxx>
#include <unotools/options.hxx>

#include "breakpoint.hxx"
#include "linenumberwindow.hxx"
#include <set>

#define MARKER_NOMARKER 0xFFFF

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

namespace basctl
{

class ObjectCatalog;

DBG_NAMEEX( ModulWindow )

// #108672 Helper functions to get/set text in TextEngine
// using the stream interface (get/setText() only supports
// tools Strings limited to 64K).
::rtl::OUString getTextEngineText( ExtTextEngine* pEngine );
void setTextEngineText( ExtTextEngine* pEngine, const ::rtl::OUString aStr );

class ProgressInfo : public SfxProgress
{
private:
    sal_uLong                   nCurState;

public:

    inline          ProgressInfo( SfxObjectShell* pObjSh, const String& rText, sal_uLong nRange );
    inline void     StepProgress();
};

inline ProgressInfo::ProgressInfo( SfxObjectShell* pObjSh, const String& rText, sal_uLong nRange )
    : SfxProgress( pObjSh, rText, nRange )
{
    nCurState = 0;
}

inline void ProgressInfo::StepProgress()
{
    SetState( ++nCurState );
}

typedef std::set<sal_uInt16> SyntaxLineSet;

class EditorWindow : public Window, public SfxListener
{
private:
    class ChangesListener;

    ExtTextView*    pEditView;
    ExtTextEngine*  pEditEngine;
    ModulWindow&    rModulWindow;

    rtl::Reference< ChangesListener > listener_;
    osl::Mutex mutex_;
    com::sun::star::uno::Reference< com::sun::star::beans::XMultiPropertySet >
        notifier_;

    long            nCurTextWidth;

    SyntaxHighlighter   aHighlighter;
    Timer           aSyntaxIdleTimer;
    SyntaxLineSet   aSyntaxLineTable;
    DECL_LINK(SyntaxTimerHdl, void *);
    ProgressInfo*   pProgress;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    using           Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            ImpDoHighlight( sal_uLong nLineOff );
    void            ImplSetFont();

    bool            bHighlightning;
    bool            bDoSyntaxHighlight;
    bool            bDelayHighlight;

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
    GetComponentInterface(sal_Bool bCreate = true);

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKeyEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    LoseFocus();
    virtual void    RequestHelp( const HelpEvent& rHEvt );

    void            DoSyntaxHighlight( sal_uLong nPara );
    String          GetWordAtCursor();
    bool            ImpCanModify();

public:
                    EditorWindow (Window* pParent, ModulWindow*);
                    ~EditorWindow();

    ExtTextEngine*  GetEditEngine() const   { return pEditEngine; }
    ExtTextView*    GetEditView() const     { return pEditView; }
    ProgressInfo*   GetProgress() const     { return pProgress; }

    void            CreateProgress( const String& rText, sal_uLong nRange );
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

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    void setBackgroundColor(Color aColor);

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void            ShowMarker( bool bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    bool            SyncYOffset();

public:
                    BreakPointWindow (Window* pParent, ModulWindow*);
                    ~BreakPointWindow();

    void            SetMarkerPos( sal_uInt16 nLine, bool bErrorMarker = false );

    void            DoScroll( long nHorzScroll, long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};


class WatchTreeListBox : public SvHeaderTabListBox
{
    String aEditingRes;

protected:
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText );

    bool            ImplBasicEntryEdited( SvLBoxEntry* pEntry, const String& rResult );
    SbxBase*        ImplGetSBXForEntry( SvLBoxEntry* pEntry, bool& rbArrayElement );

public:
    WatchTreeListBox( Window* pParent, WinBits nWinBits );
    ~WatchTreeListBox();

    void            RequestingChildren( SvLBoxEntry * pParent );
    void            UpdateWatches( bool bBasicStopped = false );

    using           SvTabListBox::SetTabs;
    virtual void    SetTabs();
};



class WatchWindow : public DockingWindow
{
private:
    String              aWatchStr;
    ExtendedEdit        aXEdit;
    ImageButton         aRemoveWatchButton;
    WatchTreeListBox    aTreeListBox;
    HeaderBar           aHeaderBar;

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );

    DECL_LINK( ButtonHdl, ImageButton * );
    DECL_LINK(TreeListHdl, void *);
    DECL_LINK( implEndDragHdl, HeaderBar * );
    DECL_LINK( EditAccHdl, Accelerator * );


public:
                    WatchWindow (Layout* pParent);
                    ~WatchWindow();

    void            AddWatch( const String& rVName );
    bool            RemoveSelectedWatch();
    void            UpdateWatches( bool bBasicStopped = false );

    WatchTreeListBox&   GetWatchTreeListBox() { return aTreeListBox; }
};


class StackWindow : public DockingWindow
{
private:
    SvTreeListBox   aTreeListBox;
    String          aStackStr;

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );

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

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

protected:
    virtual void        Resize();
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
    ::rtl::OUString     aCurPath;
    ::rtl::OUString     m_aModule;

    void                CheckCompileBasic();
    bool                BasicExecute();

    void                GoOnTop();

    sal_Int32           FormatAndPrint( Printer* pPrinter, sal_Int32 nPage = -1 );
    SbModuleRef         XModule();
protected:
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    Paint( const Rectangle& );
    virtual void    DoInit();
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();

    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName, ::rtl::OUString& aModule );

                    ~ModulWindow();

    virtual void    ExecuteCommand (SfxRequest& rReq);
    virtual void    ExecuteGlobal (SfxRequest& rReq);
    virtual void    GetState( SfxItemSet& );
    virtual void    StoreData();
    virtual void    UpdateData();
    virtual bool    CanClose();
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter );
    virtual ::rtl::OUString  GetTitle();
    virtual EntryDescriptor CreateEntryDescriptor();
    virtual bool    AllowUndo();
    virtual void    SetReadOnly (bool bReadOnly);
    virtual bool    IsReadOnly();

    StarBASIC*      GetBasic() { XModule(); return xBasic; }

    SbModule*       GetSbModule() { return xModule; }
    void            SetSbModule( SbModule* pModule ) { xModule = pModule; }
    ::rtl::OUString GetSbModuleName();

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

    long            BasicErrorHdl( StarBASIC* pBasic );
    long            BasicBreakHdl( StarBASIC* pBasic );
    void            AssertValidEditEngine();

    bool            LoadBasic();
    bool            SaveBasicSource();
    bool            ImportDialog();

    void            EditMacro( const String& rMacroName );

    bool            ToggleBreakPoint( sal_uLong nLine );

    BasicStatus&    GetBasicStatus() { return aStatus; }

    virtual bool    IsModified ();
    virtual bool    IsPasteAllowed ();

    void            FrameWindowMoved();
    void            ShowCursor( bool bOn );

    virtual sal_uInt16  GetSearchOptions();
    virtual sal_uInt16  StartSearchAndReplace (SvxSearchItem const&, bool bFromStart = false);

    EditorWindow&       GetEditorWindow()       { return aXEditorWindow.GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return aXEditorWindow.GetBrkWindow(); }
    LineNumberWindow&   GetLineNumberWindow()   { return aXEditorWindow.GetLineNumberWindow(); }
    ScrollBar&          GetEditVScrollBar()     { return aXEditorWindow.GetEWVScrollBar(); }
    ExtTextEngine*      GetEditEngine()         { return GetEditorWindow().GetEditEngine(); }
    ExtTextView*        GetEditView()           { return GetEditorWindow().GetEditView(); }
    BreakPointList&     GetBreakPoints()        { return GetBreakPointWindow().GetBreakPoints(); }
    ModulWindowLayout&  GetLayout ()            { return rLayout; }

    virtual void        BasicStarted();
    virtual void        BasicStopped();

    virtual ::svl::IUndoManager*
                        GetUndoManager();

    const ::rtl::OUString&  GetModule() const { return m_aModule; }
    void                    SetModule( const ::rtl::OUString& aModule ) { m_aModule = aModule; }

    virtual void Activating ();
    virtual void Deactivating ();

    virtual void OnNewDocument ();
    virtual char const* GetHid () const;
    virtual ItemType GetType () const;
    virtual bool HasActiveEditor () const;

    void UpdateModule ();
};

class ModulWindowLayout: public Layout
{
public:
    ModulWindowLayout (Window* pParent, ObjectCatalog&);
public:
    // Layout:
    virtual void Activating (BaseWindow&);
    virtual void Deactivating ();
    virtual void GetState (SfxItemSet&, unsigned nWhich);
    virtual void UpdateDebug (bool bBasicStopped);
public:
    void BasicAddWatch (String const&);
    void BasicRemoveWatch ();
    Color GetSyntaxColor (TokenTypes eType) const { return aSyntaxColors.GetColor(eType); }

protected:
    // Window:
    virtual void Paint (const Rectangle& rRect);
    // Layout:
    virtual void OnFirstSize (int nWidth, int nHeight);

private:
    // main child window
    ModulWindow* pChild;
    // dockable windows
    WatchWindow aWatchWindow;
    StackWindow aStackWindow;
    ObjectCatalog& rObjectCatalog;
private:
    virtual void DataChanged (DataChangedEvent const& rDCEvt);
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
        virtual void ConfigurationChanged (utl::ConfigurationBroadcaster*, sal_uInt32);
        void NewConfig (bool bFirst);

    private:
        // the color values (the indexes are TokenTypes, see svtools/syntaxhighlight.hxx)
        Color aColors[TT_KEYWORDS + 1];
        // the configuration
        svtools::ColorConfig aConfig;
        // the active editor
        EditorWindow* pEditor;

    } aSyntaxColors;
};

} // namespace basctl

#endif // BASCTL_BASIDE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
