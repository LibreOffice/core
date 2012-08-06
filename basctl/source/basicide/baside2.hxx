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

#include <svheader.hxx>

#include <bastypes.hxx>
#include <bastype3.hxx>
#include <basidesh.hxx>

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
#include <svtools/syntaxhighlight.hxx>
#include <unotools/options.hxx>

#include "breakpoint.hxx"
#include "linenumberwindow.hxx"
#include "objdlg.hxx"
#include <set>

DBG_NAMEEX( ModulWindow )

#define MARKER_NOMARKER 0xFFFF

namespace com { namespace sun { namespace star { namespace beans {
    class XMultiPropertySet;
} } } }

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
    friend class ChangesListener;

    ExtTextView*    pEditView;
    ExtTextEngine*  pEditEngine;

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
    ModulWindow*    pModulWindow;

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
    GetComponentInterface(sal_Bool bCreate = sal_True);

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
                    EditorWindow( Window* pParent );
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
    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

    void            ForceSyntaxTimeout();
    bool            SetSourceInBasic();

    bool            CanModify() { return ImpCanModify(); }
};


class BreakPointWindow : public Window
{
private:
    long            nCurYOffset;
    sal_uInt16          nMarkerPos;
    BreakPointList  aBreakPointList;
    ModulWindow*    pModulWindow;
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
                    BreakPointWindow( Window* pParent );
                    ~BreakPointWindow();

    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

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

    sal_Bool            ImplBasicEntryEdited( SvLBoxEntry* pEntry, const String& rResult );
    SbxBase*        ImplGetSBXForEntry( SvLBoxEntry* pEntry, bool& rbArrayElement );

public:
    WatchTreeListBox( Window* pParent, WinBits nWinBits );
    ~WatchTreeListBox();

    void            RequestingChildren( SvLBoxEntry * pParent );
    void            UpdateWatches( bool bBasicStopped = false );

    using           SvTabListBox::SetTabs;
    virtual void    SetTabs();
};



class WatchWindow : public BasicDockingWindow
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
                    WatchWindow( Window* pParent );
                    ~WatchWindow();

    void            AddWatch( const String& rVName );
    bool            RemoveSelectedWatch();
    void            UpdateWatches( bool bBasicStopped = false );

    WatchTreeListBox&   GetWatchTreeListBox() { return aTreeListBox; }
};


class StackWindow : public BasicDockingWindow
{
private:
    SvTreeListBox   aTreeListBox;
    String          aStackStr;

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );

public:
                    StackWindow( Window* pParent );
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
    bool                bLineNumberDisplay;
    bool                bObjectCatalogDisplay;

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
    void SetObjectCatalogDisplay(bool b);
};


class ModulWindow: public IDEBaseWindow
{
    friend class BasicIDEShell;

private:
    StarBASICRef        xBasic;
    short               nValid;
    ComplexEditorWindow aXEditorWindow;
    BasicStatus         aStatus;
    SbModuleRef         xModule;
    ModulWindowLayout*  pLayout;
    ::rtl::OUString     aCurPath;
    ::rtl::OUString     m_aModule;

    long                BasicErrorHdl( StarBASIC* pBasic );
    long                BasicBreakHdl( StarBASIC* pBasic );

    void                CheckCompileBasic();
    bool                BasicExecute();

    void                GoOnTop();
    void                AssertValidEditEngine();

    sal_Int32           FormatAndPrint( Printer* pPrinter, sal_Int32 nPage = -1 );
    SbModuleRef         XModule();
protected:
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    Paint( const Rectangle& );
    virtual void    Deactivating();
    virtual void    DoInit();
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();

    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName, ::rtl::OUString& aModule );

                    ~ModulWindow();

    virtual void    ExecuteCommand( SfxRequest& rReq );
    virtual void    GetState( SfxItemSet& );
    virtual void    StoreData();
    virtual void    UpdateData();
    virtual bool    CanClose();
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter );
    virtual ::rtl::OUString  GetTitle();
    virtual BasicEntryDescriptor CreateEntryDescriptor();
    virtual bool    AllowUndo();
    virtual void    SetReadOnly( sal_Bool bReadOnly );
    virtual sal_Bool    IsReadOnly();

    void SetLineNumberDisplay(bool);
    void SetObjectCatalogDisplay(bool);

    StarBASIC*      GetBasic() { XModule(); return xBasic; }

    SbModule*       GetSbModule() { return xModule; }
    void            SetSbModule( SbModule* pModule ) { xModule = pModule; }
    ::rtl::OUString GetSbModuleName();

    sal_Bool            CompileBasic();
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

    bool            LoadBasic();
    bool            SaveBasicSource();
    bool            ImportDialog();

    void            EditMacro( const String& rMacroName );

    bool            ToggleBreakPoint( sal_uLong nLine );

    BasicStatus&    GetBasicStatus() { return aStatus; }

    virtual sal_Bool    IsModified();
    virtual sal_Bool    IsPasteAllowed();

    void            FrameWindowMoved();
    void            ShowCursor( bool bOn );

    virtual sal_uInt16  GetSearchOptions();
    sal_uInt16          StartSearchAndReplace( const SvxSearchItem& rSearchItem, bool bFromStart = false );

    virtual Window* GetLayoutWindow();

    EditorWindow&       GetEditorWindow()       { return aXEditorWindow.GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return aXEditorWindow.GetBrkWindow(); }
    LineNumberWindow&   GetLineNumberWindow()   { return aXEditorWindow.GetLineNumberWindow(); }
    ScrollBar&          GetEditVScrollBar()     { return aXEditorWindow.GetEWVScrollBar(); }
    ExtTextEngine*      GetEditEngine()         { return GetEditorWindow().GetEditEngine(); }
    ExtTextView*        GetEditView()           { return GetEditorWindow().GetEditView(); }
    BreakPointList&     GetBreakPoints()        { return GetBreakPointWindow().GetBreakPoints(); }
    ModulWindowLayout*  GetLayout() const       { return pLayout; }

    virtual void        BasicStarted();
    virtual void        BasicStopped();

    virtual ::svl::IUndoManager*
                        GetUndoManager();

    const ::rtl::OUString&  GetModule() const { return m_aModule; }
    void                    SetModule( const ::rtl::OUString& aModule ) { m_aModule = aModule; }
};

class ModulWindowLayout: public Window, public utl::ConfigurationListener
{
private:
    // is ArrangeWindows() called first in this object?
    bool bFirstArrange;

    // splitter lines
    Splitter aLeftSplit, aBottomSplit, aVertSplit;

    // dockable windows
    ObjectCatalog aObjectCatalog;
    WatchWindow aWatchWindow;
    StackWindow aStackWindow;

    ModulWindow* m_pModulWindow;

    Color m_aSyntaxColors[TT_KEYWORDS + 1];
    svtools::ColorConfig m_aColorConfig;

    ImageList m_aImagesNormal;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    void updateSyntaxHighlighting();

    DECL_LINK( SplitHdl, Splitter * );

    void            ArrangeWindows();

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );

public:
                    ModulWindowLayout( Window* pParent );
                    ~ModulWindowLayout();

    void            DockaWindow( DockingWindow* pDockingWin );
    bool            IsToBeDocked( DockingWindow* pDockingWin, const Point& rPos, Rectangle& rRect );

    void            SetModulWindow( ModulWindow* pModWin );
    ModulWindow*    GetModulWindow() const { return m_pModulWindow; }

    WatchWindow&    GetWatchWindow()    { return aWatchWindow; }
    StackWindow&    GetStackWindow()    { return aStackWindow; }
    ObjectCatalog&  GetObjectCatalog()  { return aObjectCatalog; }

    Image getImage(sal_uInt16 nId) const;

    inline Color const & getSyntaxColor(TokenTypes eType) const
    { return m_aSyntaxColors[eType]; }

    void ToggleObjectCatalog ();
    bool HasObjectCatalog () const { return aObjectCatalog.IsVisible(); }
    void UpdateObjectCatalog ();
};

#endif // BASCTL_BASIDE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
