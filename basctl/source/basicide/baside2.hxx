/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASIDE2_HXX
#define _BASIDE2_HXX

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
#include <basic/sbmod.hxx>
#include <vcl/split.hxx>
#include "svl/lstner.hxx"
#include <svtools/colorcfg.hxx>

#include <sfx2/progress.hxx>
#include <svtools/syntaxhighlight.hxx>

DBG_NAMEEX( ModulWindow )

#define MARKER_NOMARKER 0xFFFF

namespace utl
{
    class SourceViewConfig;
}

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


namespace svt {
class SourceViewConfig;
}

class EditorWindow : public Window, public SfxListener, public utl::ConfigurationListener
{
private:
    ExtTextView*    pEditView;
    ExtTextEngine*  pEditEngine;

    utl::SourceViewConfig* pSourceViewConfig;

    long            nCurTextWidth;

    SyntaxHighlighter   aHighlighter;
    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;
    DECL_LINK( SyntaxTimerHdl, Timer * );
    ProgressInfo*   pProgress;
    ModulWindow*    pModulWindow;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    using           Window::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            ImpDoHighlight( sal_uLong nLineOff );
    void            ImplSetFont();

    sal_Bool            bHighlightning;
    sal_Bool            bDoSyntaxHighlight;
    sal_Bool            bDelayHighlight;

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
    virtual void    ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

    void            DoSyntaxHighlight( sal_uLong nPara );
    String          GetWordAtCursor();
    sal_Bool            ImpCanModify();

public:
                    EditorWindow( Window* pParent );
                    ~EditorWindow();

    ExtTextEngine*  GetEditEngine() const   { return (ExtTextEngine*)pEditEngine; }
    ExtTextView*    GetEditView() const     { return pEditView; }
    ProgressInfo*   GetProgress() const     { return pProgress; }

    void            CreateProgress( const String& rText, sal_uLong nRange );
    void            DestroyProgress();

    void            ParagraphInsertedDeleted( sal_uLong nNewPara, sal_Bool bInserted );
    void            DoDelayedSyntaxHighlight( sal_uLong nPara );

    void            CreateEditEngine();
    void            SetScrollBarRanges();
    void            InitScrollBars();
    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

    void            ForceSyntaxTimeout();
    sal_Bool            SetSourceInBasic( sal_Bool bQuiet = sal_True );

    sal_Bool            CanModify() { return ImpCanModify(); }
};


class BreakPointWindow : public Window
{
private:
    long            nCurYOffset;
    sal_uInt16          nMarkerPos;
    BreakPointList  aBreakPointList;
    ModulWindow*    pModulWindow;
    sal_Bool            bErrorMarker;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    void setBackgroundColor(Color aColor);

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void            ShowMarker( sal_Bool bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    sal_Bool            SyncYOffset();

public:
                    BreakPointWindow( Window* pParent );
                    ~BreakPointWindow();

    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

    void            SetMarkerPos( sal_uInt16 nLine, sal_Bool bErrorMarker = sal_False );

    void            DoScroll( long nHorzScroll, long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};


class WatchTreeListBox : public SvHeaderTabListBox
{
    String aEditingRes;

protected:
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

    sal_Bool            ImplBasicEntryEdited( SvLBoxEntry* pEntry, const String& rResult );
    SbxBase*        ImplGetSBXForEntry( SvLBoxEntry* pEntry, bool& rbArrayElement );

public:
    WatchTreeListBox( Window* pParent, WinBits nWinBits );
    ~WatchTreeListBox();

    void            RequestingChilds( SvLBoxEntry * pParent );
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
    DECL_LINK( TreeListHdl, SvTreeListBox * );
    DECL_LINK( implEndDragHdl, HeaderBar * );
    DECL_LINK( EditAccHdl, Accelerator * );


public:
                    WatchWindow( Window* pParent );
                    ~WatchWindow();

    void            AddWatch( const String& rVName );
    sal_Bool            RemoveSelectedWatch();
    void            UpdateWatches( bool bBasicStopped = false );

    WatchTreeListBox&   GetWatchTreeListBox() { return aTreeListBox; }
};


class StackWindow : public BasicDockingWindow
{
private:
    SvTreeListBox   aTreeListBox;
    ImageButton     aGotoCallButton;
    String          aStackStr;

protected:
    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );
    DECL_LINK( ButtonHdl, ImageButton * );

public:
                    StackWindow( Window* pParent );
                    ~StackWindow();

    void            UpdateCalls();
};


class ComplexEditorWindow : public Window
{
private:
    BreakPointWindow    aBrkWindow;
    EditorWindow        aEdtWindow;
    ScrollBar           aEWVScrollBar;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

protected:
    virtual void        Resize();
    DECL_LINK( ScrollHdl, ScrollBar * );

public:
                        ComplexEditorWindow( ModulWindow* pParent );

    BreakPointWindow&   GetBrkWindow()      { return aBrkWindow; }
    EditorWindow&       GetEdtWindow()      { return aEdtWindow; }
    ScrollBar&          GetEWVScrollBar()   { return aEWVScrollBar; }
};


#define SYNTAX_COLOR_MAX    5

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
    String              aCurPath;
    ::rtl::OUString     m_aModule;

    DECL_LINK( BasicErrorHdl, StarBASIC * );
    long                BasicBreakHdl( StarBASIC* pBasic );

    void                CheckCompileBasic();
    sal_Bool                BasicExecute();

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

                    ModulWindow( ModulWindowLayout* pParent, const ScriptDocument& rDocument, String aLibName,
                        String aName, ::rtl::OUString& aModule );

                    ~ModulWindow();

    virtual void    ExecuteCommand( SfxRequest& rReq );
    virtual void    GetState( SfxItemSet& );
    virtual void    StoreData();
    virtual void    UpdateData();
    virtual sal_Bool    CanClose();
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter );
    virtual String  GetTitle();
    virtual BasicEntryDescriptor CreateEntryDescriptor();
    virtual sal_Bool    AllowUndo();
    virtual void    SetReadOnly( sal_Bool bReadOnly );
    virtual sal_Bool    IsReadOnly();

    StarBASIC*      GetBasic() { XModule(); return xBasic; }

    SbModule*       GetSbModule() { return xModule; }
    void            SetSbModule( SbModule* pModule ) { xModule = pModule; }
    String          GetSbModuleName();

    sal_Bool            CompileBasic();
    sal_Bool            BasicRun();
    sal_Bool            BasicStepOver();
    sal_Bool            BasicStepInto();
    sal_Bool            BasicStepOut();
    void            BasicStop();
    sal_Bool            BasicToggleBreakPoint();
    void            BasicToggleBreakPointEnabled();
    void            ManageBreakPoints();
    void            UpdateBreakPoint( const BreakPoint& rBrk );
    void            BasicAddWatch();
    void            BasicRemoveWatch();

    sal_Bool            LoadBasic();
    sal_Bool            SaveBasicSource();
    sal_Bool            ImportDialog();

    void            EditMacro( const String& rMacroName );

    sal_Bool            ToggleBreakPoint( sal_uLong nLine );

    BasicStatus&    GetBasicStatus() { return aStatus; }

    virtual sal_Bool    IsModified();
    virtual sal_Bool    IsPasteAllowed();

    void            FrameWindowMoved();
    void            ShowCursor( sal_Bool bOn );

    virtual sal_uInt16  GetSearchOptions();
    sal_uInt16          StartSearchAndReplace( const SvxSearchItem& rSearchItem, sal_Bool bFromStart = sal_False );

    virtual Window* GetLayoutWindow();

    EditorWindow&       GetEditorWindow()       { return aXEditorWindow.GetEdtWindow(); }
    BreakPointWindow&   GetBreakPointWindow()   { return aXEditorWindow.GetBrkWindow(); }
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

    Splitter        aVSplitter;
    Splitter        aHSplitter;

    WatchWindow     aWatchWindow;
    StackWindow     aStackWindow;

    sal_Bool            bVSplitted;
    sal_Bool            bHSplitted;

    ModulWindow * m_pModulWindow;

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
    sal_Bool            IsToBeDocked( DockingWindow* pDockingWin, const Point& rPos, Rectangle& rRect );

    void            SetModulWindow( ModulWindow* pModWin );
    ModulWindow*    GetModulWindow() const { return m_pModulWindow; }

    WatchWindow&    GetWatchWindow()    { return aWatchWindow; }
    StackWindow&    GetStackWindow()    { return aStackWindow; }

    Image getImage(sal_uInt16 nId) const;

    inline Color const & getSyntaxColor(TokenTypes eType) const
    { return m_aSyntaxColors[eType]; }
};

#endif  // _BASIDE2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
