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

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
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
    ULONG                   nCurState;

public:

    inline          ProgressInfo( SfxObjectShell* pObjSh, const String& rText, ULONG nRange );

//  inline void     StartProgress( const String& rName, ULONG nStates );
//  inline void     EndProgress();
    inline void     StepProgress();
};

inline ProgressInfo::ProgressInfo( SfxObjectShell* pObjSh, const String& rText, ULONG nRange )
    : SfxProgress( pObjSh, rText, nRange )
{
    nCurState = 0;
}

// inline void ProgressInfo::StartProgress( const String& rName, ULONG nStates )
// {
//  nCurState = 0;
//  if ( pStbMgr )
//      pStbMgr->StartProgressMode( rName, nStates );
// }

// inline void ProgressInfo::EndProgress()
// {
//  if ( pStbMgr )
//      pStbMgr->EndProgressMode();
//}

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

    void            ImpDoHighlight( ULONG nLineOff );
    void            ImplSetFont();

    BOOL            bHighlightning;
    BOOL            bDoSyntaxHighlight;
    BOOL            bDelayHighlight;

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
    GetComponentInterface(BOOL bCreate = TRUE);

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

    void            DoSyntaxHighlight( ULONG nPara );
    String          GetWordAtCursor();
    BOOL            ImpCanModify();

public:
                    EditorWindow( Window* pParent );
                    ~EditorWindow();

    ExtTextEngine*  GetEditEngine() const   { return (ExtTextEngine*)pEditEngine; }
    ExtTextView*    GetEditView() const     { return pEditView; }
    ProgressInfo*   GetProgress() const     { return pProgress; }

    void            CreateProgress( const String& rText, ULONG nRange );
    void            DestroyProgress();

    void            ParagraphInsertedDeleted( ULONG nNewPara, BOOL bInserted );
    void            DoDelayedSyntaxHighlight( ULONG nPara );

    void            CreateEditEngine();
    void            SetScrollBarRanges();
    void            InitScrollBars();
    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

    void            ForceSyntaxTimeout();
    BOOL            SetSourceInBasic( BOOL bQuiet = TRUE );

    BOOL            CanModify() { return ImpCanModify(); }
};


class BreakPointWindow : public Window
{
private:
    long            nCurYOffset;
    USHORT          nMarkerPos;
    BreakPointList  aBreakPointList;
    ModulWindow*    pModulWindow;
    BOOL            bErrorMarker;
    bool m_bHighContrastMode;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    void setBackgroundColor(Color aColor);

protected:
    virtual void    Paint( const Rectangle& );
    virtual void    Resize();
    BreakPoint*     FindBreakPoint( const Point& rMousePos );
    void            ShowMarker( BOOL bShow );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    Command( const CommandEvent& rCEvt );

    BOOL            SyncYOffset();

public:
                    BreakPointWindow( Window* pParent );
                    ~BreakPointWindow();

    void            SetModulWindow( ModulWindow* pWin )
                        { pModulWindow = pWin; }

    void            SetMarkerPos( USHORT nLine, BOOL bErrorMarker = FALSE );

    void            DoScroll( long nHorzScroll, long nVertScroll );
    long&           GetCurYOffset()         { return nCurYOffset; }
    BreakPointList& GetBreakPoints()        { return aBreakPointList; }
};


class WatchTreeListBox : public SvHeaderTabListBox
{
    String aEditingRes;

protected:
    virtual BOOL    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

    BOOL            ImplBasicEntryEdited( SvLBoxEntry* pEntry, const String& rResult );
    SbxBase*        ImplGetSBXForEntry( SvLBoxEntry* pEntry, bool& rbArrayElement );

//  virtual DragDropMode    NotifyBeginDrag( SvLBoxEntry* );
//  virtual BOOL    NotifyQueryDrop( SvLBoxEntry* );

//  virtual BOOL    NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
//                      SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );
//  virtual BOOL    NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
//                      SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );
//  BOOL            NotifyCopyingMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
//                      SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos, BOOL bMove );

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
    BOOL            RemoveSelectedWatch();
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
    BOOL                BasicExecute();

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
    virtual BOOL    CanClose();
    // virtual void PrintData( Printer* pPrinter );
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter );
    virtual String  GetTitle();
    virtual BasicEntryDescriptor CreateEntryDescriptor();
    virtual BOOL    AllowUndo();
    virtual void    SetReadOnly( BOOL bReadOnly );
    virtual BOOL    IsReadOnly();

    StarBASIC*      GetBasic() { XModule(); return xBasic; }

    SbModule*       GetSbModule() { return xModule; }
    void            SetSbModule( SbModule* pModule ) { xModule = pModule; }
    String          GetSbModuleName();

    BOOL            CompileBasic();
    BOOL            BasicRun();
    BOOL            BasicStepOver();
    BOOL            BasicStepInto();
    BOOL            BasicStepOut();
    void            BasicStop();
    BOOL            BasicToggleBreakPoint();
    void            BasicToggleBreakPointEnabled();
    void            ManageBreakPoints();
    void            UpdateBreakPoint( const BreakPoint& rBrk );
    void            BasicAddWatch();
    void            BasicRemoveWatch();

    BOOL            LoadBasic();
    BOOL            SaveBasicSource();
    BOOL            ImportDialog();

    void            EditMacro( const String& rMacroName );

//  void            InsertFromObjectCatalog( ObjectCatalog* pObjDlg );

    BOOL            ToggleBreakPoint( ULONG nLine );

    BasicStatus&    GetBasicStatus() { return aStatus; }

    virtual BOOL    IsModified();
    virtual BOOL    IsPasteAllowed();

    void            FrameWindowMoved();
    void            ShowCursor( BOOL bOn );

    virtual USHORT  GetSearchOptions();
    USHORT          StartSearchAndReplace( const SvxSearchItem& rSearchItem, BOOL bFromStart = FALSE );

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

    BOOL            bVSplitted;
    BOOL            bHSplitted;

    ModulWindow * m_pModulWindow;

    Color m_aSyntaxColors[TT_KEYWORDS + 1];
    svtools::ColorConfig m_aColorConfig;

    ImageList m_aImagesNormal;
    ImageList m_aImagesHighContrast;

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
    BOOL            IsToBeDocked( DockingWindow* pDockingWin, const Point& rPos, Rectangle& rRect );

    void            SetModulWindow( ModulWindow* pModWin );
    ModulWindow*    GetModulWindow() const { return m_pModulWindow; }

    WatchWindow&    GetWatchWindow()    { return aWatchWindow; }
    StackWindow&    GetStackWindow()    { return aStackWindow; }

    Image getImage(USHORT nId, bool bHighContrastMode) const;

    inline Color const & getSyntaxColor(TokenTypes eType) const
    { return m_aSyntaxColors[eType]; }
};

#endif  // _BASIDE2_HXX
