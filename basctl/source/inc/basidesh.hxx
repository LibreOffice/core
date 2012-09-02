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
#ifndef BASCTL_BASIDESH_HXX
#define BASCTL_BASIDESH_HXX

#include "doceventnotifier.hxx"
#include "sbxitem.hxx"
#include "../basicide/objdlg.hxx"

#include <com/sun/star/container/XContainerListener.hpp>
#include <sfx2/viewsh.hxx>
#include <svx/ifaceids.hxx>
#include <vcl/scrbar.hxx>
#include <map>
#include <boost/scoped_ptr.hpp>

class SfxViewFactory;
class SdrView;
class TabBar;
class SbxObject;
class SbModule;
class StarBASIC;

namespace basctl
{
//----------------------------------------------------------------------------

const sal_uLong BASICIDE_UI_FEATURE_SHOW_BROWSER = 0x00000001;

//----------------------------------------------------------------------------

class Layout;
class ModulWindow;
class ModulWindowLayout;
class DialogWindow;
class DialogWindowLayout;
class TabBar;
class BaseWindow;
class LocalizationMgr;

bool RemoveDialog( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName );

class Shell :
    public SfxViewShell,
    public DocumentEventListener
{
public:
    typedef std::map<sal_uInt16, BaseWindow*> WindowTable;
    typedef WindowTable::const_iterator WindowTableIt;

private:
    friend class JavaDebuggingListenerImpl;
    friend class LocalizationMgr;
    friend bool implImportDialog( Window* pWin, const ::rtl::OUString& rCurPath, const ScriptDocument& rDocument, const ::rtl::OUString& aLibName ); // defined in baside3.cxx

    WindowTable         aWindowTable;
    sal_uInt16          nCurKey;
    BaseWindow*         pCurWin;
    ScriptDocument      m_aCurDocument;
    rtl::OUString       m_aCurLibName;
    boost::shared_ptr<LocalizationMgr> m_pCurLocalizationMgr;

    ScrollBar           aHScrollBar;
    ScrollBar           aVScrollBar;
    ScrollBarBox        aScrollBarBox;
    boost::scoped_ptr<TabBar> pTabBar; // basctl::TabBar
    bool                bTabBarSplitted;
    bool                bCreatingWindow;
    // layout windows
    boost::scoped_ptr<ModulWindowLayout> pModulLayout;
    boost::scoped_ptr<DialogWindowLayout> pDialogLayout;
    // the active layout window
    Layout* pLayout;
    // common object catalog window
    ObjectCatalog aObjectCatalog;

    bool                m_bAppBasicModified;
    DocumentEventNotifier m_aNotifier;
    friend class ContainerListenerImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xLibListener;

#if _SOLAR__PRIVATE
    void                Init();
    void                InitTabBar();
    void                InitScrollBars();
    void                CheckWindows();
    void                RemoveWindows( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, bool bDestroy );
    void                UpdateWindows();
    void                InvalidateBasicIDESlots();
    void                StoreAllWindowData( bool bPersistent = true );
    void                SetMDITitle();
    void                EnableScrollbars( bool bEnable );
    void                SetCurLib( const ScriptDocument& rDocument, ::rtl::OUString aLibName, bool bUpdateWindows = true , bool bCheck = true );
    void                SetCurLibForLocalization( const ScriptDocument& rDocument, ::rtl::OUString aLibName );

    void                ImplStartListening( StarBASIC* pBasic );

    DECL_LINK( TabBarHdl, TabBar* );
    DECL_LINK( TabBarSplitHdl, TabBar * );
#endif

    static unsigned nShellCount;

private:
    virtual void        AdjustPosSizePixel( const Point &rPos, const Size &rSize );
    virtual void        OuterResizePixel( const Point &rPos, const Size &rSize );
    virtual Size        GetOptimalSizePixel() const;
    sal_uInt16              InsertWindowInTable (BaseWindow* pNewWin);
    virtual sal_uInt16      PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing );

    void                SetCurWindow (BaseWindow* pNewWin, bool bUpdateTabBar = false, bool bRememberAsCurrent = true);
    void                ManageToolbars();
    void                ArrangeTabBar();

    ModulWindow*        CreateBasWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName );
    DialogWindow*       CreateDlgWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rDlgName );

    ModulWindow*        ShowActiveModuleWindow( StarBASIC* pBasic );

    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );

    virtual void        Activate(sal_Bool bMDI);
    virtual void        Deactivate(sal_Bool bMDI);

    virtual void        Move();
    virtual void        ShowCursor( bool bOn = true );

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument );
    virtual void onDocumentOpened( const ScriptDocument& _rDocument );
    virtual void onDocumentSave( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument );
    virtual void onDocumentClosed( const ScriptDocument& _rDocument );
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument );
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument );

public:
    TYPEINFO();
    SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_VIEWSH )
    SFX_DECL_VIEWFACTORY(Shell);

    Shell( SfxViewFrame *pFrame, SfxViewShell *pOldSh );
    ~Shell();

    BaseWindow*      GetCurWindow() const    { return pCurWin; }
    ScriptDocument const& GetCurDocument() const { return m_aCurDocument; }
    rtl::OUString const&  GetCurLibName() const { return m_aCurLibName; }
    boost::shared_ptr<LocalizationMgr> GetCurLocalizationMgr() const { return m_pCurLocalizationMgr; }

    ScrollBar&          GetHScrollBar()         { return aHScrollBar; }
    ScrollBar&          GetVScrollBar()         { return aVScrollBar; }
    ScrollBarBox&       GetScrollBarBox()       { return aScrollBarBox; }
    TabBar&             GetTabBar()             { return *pTabBar; }
    WindowTable&        GetWindowTable()        { return aWindowTable; }
    sal_uInt16          GetWindowId (BaseWindow const* pWin) const;

    SdrView*            GetCurDlgView() const;

    svl::IUndoManager*  GetUndoManager();

    virtual com::sun::star::uno::Reference< com::sun::star::view::XRenderable > GetRenderable();

    // virtual sal_uInt16           Print( SfxProgress &rProgress, sal_Bool bIsAPI, PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*     GetPrinter( sal_Bool bCreate );
    virtual sal_uInt16      SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual String          GetSelectionText( sal_Bool bCompleteWords );
    virtual sal_Bool        HasSelection( sal_Bool bText ) const;

    void                GetState( SfxItemSet& );
    void                ExecuteGlobal( SfxRequest& rReq );
    void                ExecuteCurrent( SfxRequest& rReq );
    void                ExecuteBasic( SfxRequest& rReq );
    void                ExecuteDialog( SfxRequest& rReq );

    virtual sal_Bool    HasUIFeature( sal_uInt32 nFeature );

    long                CallBasicErrorHdl( StarBASIC* pBasic );
    long                CallBasicBreakHdl( StarBASIC* pBasic );

    BaseWindow*         FindWindow( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName = ::rtl::OUString(), const ::rtl::OUString& rName = ::rtl::OUString(), ItemType nType = TYPE_UNKNOWN, bool bFindSuspended = false );
    DialogWindow*       FindDlgWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    ModulWindow*        FindBasWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    BaseWindow*         FindApplicationWindow();
    bool                NextPage( bool bPrev = false );

    bool                IsAppBasicModified () const { return m_bAppBasicModified; }
    void                SetAppBasicModified (bool bModified = true) { m_bAppBasicModified = bModified; }

    // For Dialog Drag&Drop in Dialog Organizer:
    // (defined in moduldlg.cxx)
    static void CopyDialogResources(
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >& io_xISP,
        const ScriptDocument& rSourceDoc, const ::rtl::OUString& rSourceLibName, const ScriptDocument& rDestDoc,
        const ::rtl::OUString& rDestLibName, const ::rtl::OUString& rDlgName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        GetCurrentDocument() const;

    void UpdateObjectCatalog () { aObjectCatalog.UpdateEntries(); }

    void RemoveWindow (BaseWindow* pWindow, bool bDestroy, bool bAllowChangeCurWindow = true);
};

} // namespace basctl

// This typedef helps baside.sdi,
// because I don't know how to use nested names in it.
typedef basctl::Shell basctl_Shell;

#endif // BASCTL_BASIDESH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
