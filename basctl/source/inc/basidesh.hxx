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
#ifndef INCLUDED_BASCTL_SOURCE_INC_BASIDESH_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASIDESH_HXX

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

const sal_uLong BASICIDE_UI_FEATURE_SHOW_BROWSER = 0x00000001;

class Layout;
class ModulWindow;
class ModulWindowLayout;
class DialogWindow;
class DialogWindowLayout;
class TabBar;
class BaseWindow;
class LocalizationMgr;

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
    friend bool implImportDialog( vcl::Window* pWin, const OUString& rCurPath, const ScriptDocument& rDocument, const OUString& aLibName ); // defined in baside3.cxx

    WindowTable         aWindowTable;
    sal_uInt16          nCurKey;
    BaseWindow*         pCurWin;
    ScriptDocument      m_aCurDocument;
    OUString            m_aCurLibName;
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

    void                Init();
    void                InitTabBar();
    void                InitScrollBars();
    void                CheckWindows();
    void                RemoveWindows( const ScriptDocument& rDocument, const OUString& rLibName, bool bDestroy );
    void                UpdateWindows();
    void                InvalidateBasicIDESlots();
    void                StoreAllWindowData( bool bPersistent = true );
    void                SetMDITitle();
    void                EnableScrollbars( bool bEnable );
    void                SetCurLib( const ScriptDocument& rDocument, const OUString& aLibName, bool bUpdateWindows = true , bool bCheck = true );
    void                SetCurLibForLocalization( const ScriptDocument& rDocument, const OUString& aLibName );

    void                ImplStartListening( StarBASIC* pBasic );

    DECL_LINK( TabBarHdl, TabBar* );
    DECL_LINK( TabBarSplitHdl, TabBar * );

    static unsigned nShellCount;

private:
    virtual void        AdjustPosSizePixel( const Point &rPos, const Size &rSize ) SAL_OVERRIDE;
    virtual void        OuterResizePixel( const Point &rPos, const Size &rSize ) SAL_OVERRIDE;
    sal_uInt16          InsertWindowInTable (BaseWindow* pNewWin);
    virtual bool        PrepareClose( bool bUI ) SAL_OVERRIDE;

    void                SetCurWindow (BaseWindow* pNewWin, bool bUpdateTabBar = false, bool bRememberAsCurrent = true);
    void                ManageToolbars();
    void                ArrangeTabBar();

    ModulWindow*        CreateBasWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName );
    DialogWindow*       CreateDlgWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName );

    ModulWindow*        ShowActiveModuleWindow( StarBASIC* pBasic );

    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType ) SAL_OVERRIDE;

    virtual void        Activate(bool bMDI) SAL_OVERRIDE;
    virtual void        Deactivate(bool bMDI) SAL_OVERRIDE;

    virtual void        Move() SAL_OVERRIDE;
    virtual void        ShowCursor( bool bOn = true ) SAL_OVERRIDE;

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentOpened( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSave( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentClosed( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument ) SAL_OVERRIDE;
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument ) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();
    SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_VIEWSH )
    SFX_DECL_VIEWFACTORY(Shell);

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    Shell( SfxViewFrame *pFrame, SfxViewShell *pOldSh );
    virtual ~Shell();

    BaseWindow*      GetCurWindow() const    { return pCurWin; }
    ScriptDocument const& GetCurDocument() const { return m_aCurDocument; }
    OUString const&  GetCurLibName() const { return m_aCurLibName; }
    boost::shared_ptr<LocalizationMgr> GetCurLocalizationMgr() const { return m_pCurLocalizationMgr; }

    ScrollBar&          GetHScrollBar()         { return aHScrollBar; }
    ScrollBar&          GetVScrollBar()         { return aVScrollBar; }
    ScrollBarBox&       GetScrollBarBox()       { return aScrollBarBox; }
    TabBar&             GetTabBar()             { return *pTabBar; }
    WindowTable&        GetWindowTable()        { return aWindowTable; }
    sal_uInt16          GetWindowId (BaseWindow const* pWin) const;

    SdrView*            GetCurDlgView() const;

    svl::IUndoManager*  GetUndoManager() SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::view::XRenderable > GetRenderable() SAL_OVERRIDE;

    // virtual sal_uInt16           Print( SfxProgress &rProgress, sal_Bool bIsAPI, PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*     GetPrinter( bool bCreate ) SAL_OVERRIDE;
    virtual sal_uInt16      SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false ) SAL_OVERRIDE;
    virtual OUString        GetSelectionText( bool bCompleteWords ) SAL_OVERRIDE;
    virtual bool            HasSelection( bool bText ) const SAL_OVERRIDE;

    void                GetState( SfxItemSet& );
    void                ExecuteGlobal( SfxRequest& rReq );
    void                ExecuteCurrent( SfxRequest& rReq );
    void                ExecuteBasic( SfxRequest& rReq );
    void                ExecuteDialog( SfxRequest& rReq );

    virtual bool        HasUIFeature( sal_uInt32 nFeature ) SAL_OVERRIDE;

    bool                CallBasicErrorHdl( StarBASIC* pBasic );
    long                CallBasicBreakHdl( StarBASIC* pBasic );

    BaseWindow*         FindWindow( const ScriptDocument& rDocument, const OUString& rLibName = OUString(), const OUString& rName = OUString(), ItemType nType = TYPE_UNKNOWN, bool bFindSuspended = false );
    DialogWindow*       FindDlgWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    ModulWindow*        FindBasWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    BaseWindow*         FindApplicationWindow();
    bool                NextPage( bool bPrev = false );

    bool                IsAppBasicModified () const { return m_bAppBasicModified; }
    void                SetAppBasicModified (bool bModified = true) { m_bAppBasicModified = bModified; }

    // For Dialog Drag&Drop in Dialog Organizer:
    // (defined in moduldlg.cxx)
    static void CopyDialogResources(
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >& io_xISP,
        const ScriptDocument& rSourceDoc, const OUString& rSourceLibName, const ScriptDocument& rDestDoc,
        const OUString& rDestLibName, const OUString& rDlgName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        GetCurrentDocument() const SAL_OVERRIDE;

    void UpdateObjectCatalog () { aObjectCatalog.UpdateEntries(); }

    void RemoveWindow (BaseWindow* pWindow, bool bDestroy, bool bAllowChangeCurWindow = true);
};

} // namespace basctl

// This typedef helps baside.sdi,
// because I don't know how to use nested names in it.
typedef basctl::Shell basctl_Shell;

#endif // INCLUDED_BASCTL_SOURCE_INC_BASIDESH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
