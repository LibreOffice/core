/*************************************************************************
 *
 *  $RCSfile: baside2.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:11:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BASIDE2_HXX
#define _BASIDE2_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#include <bastypes.hxx>
#include <bastype3.hxx>
#include <basidesh.hxx>

#include <basic/hilight.hxx>

class ExtTextEngine;
class ExtTextView;
class SvxSearchItem;

#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif

#ifndef _SV_SPLIT_HXX //autogen
#include <vcl/split.hxx>
#endif

#ifndef _SFXLSTNER_HXX
#include "svtools/lstner.hxx"
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include <sfx2/progress.hxx>


DBG_NAMEEX( ModulWindow );

#define MARKER_NOMARKER 0xFFFF


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

class EditorWindow : public Window, public SfxListener
{
private:
    ExtTextView*    pEditView;
    ExtTextEngine*  pEditEngine;

    svt::SourceViewConfig* pSourceViewConfig;

    long            nCurTextWidth;

    SyntaxHighlighter   aHighlighter;
    Timer           aSyntaxIdleTimer;
    Table           aSyntaxLineTable;
    DECL_LINK( SyntaxTimerHdl, Timer * );
    ProgressInfo*   pProgress;
    ModulWindow*    pModulWindow;

    Timer           aHelpAgentTimer;
    DECL_LINK(      HelpAgentTimerHdl, Timer * );

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

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
    //virtual BOOL  Drop( const DropEvent& rEvt );
    //virtual BOOL  QueryDrop( DropEvent& rEvt );
    virtual void    LoseFocus();
    virtual void    RequestHelp( const HelpEvent& rHEvt );

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

    void            Scroll( long nHorzScroll, long nVertScroll );
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

protected:
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    Paint( const Rectangle& );
    virtual void    Deactivating();
    virtual void    DoInit();
    virtual void    DoScroll( ScrollBar* pCurScrollBar );

public:
                    TYPEINFO();

                    ModulWindow( ModulWindowLayout* pParent, StarBASIC* pBasic,
                            SfxObjectShell* pShell, String aLibName, String aName, ::rtl::OUString& aModule );

                    ~ModulWindow();

    virtual void    ExecuteCommand( SfxRequest& rReq );
    virtual void    GetState( SfxItemSet& );
    virtual void    StoreData();
    virtual void    UpdateData();
    virtual BOOL    CanClose();
    virtual void    PrintData( Printer* pPrinter );
    virtual String  GetTitle();
    virtual String  CreateSbxDescription();
    virtual BOOL    AllowUndo();
    virtual void    SetReadOnly( BOOL bReadOnly );
    virtual BOOL    IsReadOnly();

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

    void            EditMacro( const String& rMacroName );

//  void            InsertFromObjectCatalog( ObjectCatalog* pObjDlg );

    BOOL            ToggleBreakPoint( ULONG nLine );
    BOOL            RenameModule( const String& rNewName );

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

    virtual SfxUndoManager* GetUndoManager();

    const ::rtl::OUString&  GetModule() const { return m_aModule; }
    void                    SetModule( const ::rtl::OUString& aModule ) { m_aModule = aModule; }
};

class ModulWindowLayout: public Window, public SfxListener
{
private:

    Splitter        aVSplitter;
    Splitter        aHSplitter;

    WatchWindow     aWatchWindow;
    StackWindow     aStackWindow;

    BOOL            bVSplitted;
    BOOL            bHSplitted;

    ModulWindow * m_pModulWindow;

    Color m_aSyntaxColors[TT_KEYWORD + 1];
    svtools::ColorConfig m_aColorConfig;

    ImageList m_aImagesNormal;
    ImageList m_aImagesHighContrast;

    virtual void DataChanged(DataChangedEvent const & rDCEvt);

    virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

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
