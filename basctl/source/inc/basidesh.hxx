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

#include "doceventnotifier.hxx"
#include <basctl/sbxitem.hxx>
#include "ObjectCatalog.hxx"

#include <com/sun/star/container/XContainerListener.hpp>
#include <sfx2/viewsh.hxx>
#include <svx/ifaceids.hxx>
#include <svl/srchitem.hxx>
#include <svtools/scrolladaptor.hxx>
#include <map>
#include <memory>
#include <string_view>

class SfxViewFactory;
class SdrView;
class TabBar;
class StarBASIC;

namespace basctl
{

// Used to control zoom level
constexpr sal_uInt16 MIN_ZOOM_LEVEL = 50;
constexpr sal_uInt16 DEFAULT_ZOOM_LEVEL = 100;
constexpr sal_uInt16 MAX_ZOOM_LEVEL = 400;

class Layout;
class ModulWindow;
class ModulWindowLayout;
class DialogWindow;
class DialogWindowLayout;
class LocalizationMgr;
class BasicColorConfig;
class ContainerListenerImpl;

class Shell :
    public SfxViewShell,
    public DocumentEventListener
{
public:
    typedef std::map<sal_uInt16, VclPtr<BaseWindow> > WindowTable;

private:
    friend class JavaDebuggingListenerImpl;
    friend class LocalizationMgr;
    friend bool implImportDialog(weld::Window* pWin, const ScriptDocument& rDocument, const OUString& rLibName); // defined in baside3.cxx

    WindowTable         aWindowTable;
    sal_uInt16          nCurKey;
    VclPtr<BaseWindow>  pCurWin;
    ScriptDocument      m_aCurDocument;
    OUString            m_aCurLibName;
    std::shared_ptr<LocalizationMgr> m_pCurLocalizationMgr;

    // Current value of the zoom slider
    sal_uInt16            m_nCurrentZoomSliderValue;
    VclPtr<ScrollAdaptor> aHScrollBar;
    VclPtr<ScrollAdaptor> aVScrollBar;
    VclPtr<TabBar>       pTabBar;           // basctl::TabBar
    bool                 bCreatingWindow;

    // Basic editor color configuration
    std::shared_ptr<BasicColorConfig> m_aColorConfig;

    // layout windows
    VclPtr<ModulWindowLayout>   pModulLayout;
    VclPtr<DialogWindowLayout>  pDialogLayout;
    VclPtr<Layout>              pLayout;    // the active layout window
    // common object catalog window
    VclPtr<ObjectCatalog>       aObjectCatalog;

    bool    m_bAppBasicModified;
    bool    mbJustOpened = false;

    DocumentEventNotifier m_aNotifier;

    friend class ContainerListenerImpl;
    rtl::Reference<ContainerListenerImpl> m_xLibListener;
    std::unique_ptr<SvxSearchItem> mpSearchItem;

    void                Init();
    void                InitTabBar();
    void                InitScrollBars();
    void                InitZoomLevel();
    void                CheckWindows();
    void                RemoveWindows( const ScriptDocument& rDocument, std::u16string_view rLibName );
    void                UpdateWindows();
    static void         InvalidateBasicIDESlots();
    void                StoreAllWindowData( bool bPersistent = true );
    void                SetMDITitle();
    void                SetCurLib( const ScriptDocument& rDocument, const OUString& aLibName, bool bUpdateWindows = true , bool bCheck = true );
    void                SetCurLibForLocalization( const ScriptDocument& rDocument, const OUString& aLibName );

    DECL_LINK( TabBarHdl, ::TabBar*, void );

    static unsigned nShellCount;

private:
    void                AdjustPosSizePixel( const Point &rPos, const Size &rSize );
    virtual void        OuterResizePixel( const Point &rPos, const Size &rSize ) override;
    sal_uInt16          InsertWindowInTable (BaseWindow* pNewWin);
    virtual bool        PrepareClose( bool bUI = true ) override;

    void                SetCurWindow (BaseWindow* pNewWin, bool bUpdateTabBar = false, bool bRememberAsCurrent = true);
    void                ManageToolbars();

    VclPtr<ModulWindow>  CreateBasWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName );
    VclPtr<DialogWindow> CreateDlgWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rDlgName );

    VclPtr<ModulWindow>  ShowActiveModuleWindow( StarBASIC const * pBasic );

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void        Activate(bool bMDI) override;
    virtual void        Deactivate(bool bMDI) override;

    virtual void        Move() override;
    virtual void        ShowCursor( bool bOn = true ) override;

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentOpened( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSave( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentClosed( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument ) override;
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument ) override;

public:
    SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_VIEWSH )
    SFX_DECL_VIEWFACTORY(Shell);

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    Shell(SfxViewFrame& rFrame, SfxViewShell *pOldSh);
    virtual ~Shell() override;

    BaseWindow*      GetCurWindow() const    { return pCurWin; }
    OUString const&  GetCurLibName() const { return m_aCurLibName; }
    const std::shared_ptr<LocalizationMgr>& GetCurLocalizationMgr() const { return m_pCurLocalizationMgr; }

    TabBar&             GetTabBar()             { return *pTabBar; }
    WindowTable&        GetWindowTable()        { return aWindowTable; }
    sal_uInt16          GetWindowId (BaseWindow const* pWin) const;

    SdrView*            GetCurDlgView() const;

    SfxUndoManager*     GetUndoManager() override;

    void                SetGlobalEditorZoomLevel(sal_uInt16 nNewZoomLevel);
    sal_uInt16          GetCurrentZoomSliderValue() { return m_nCurrentZoomSliderValue; }
    static sal_uInt16   GetMinZoom() { return MIN_ZOOM_LEVEL; }
    static sal_uInt16   GetMaxZoom() { return MAX_ZOOM_LEVEL; }

    virtual css::uno::Reference< css::view::XRenderable > GetRenderable() override;

    // virtual sal_uInt16           Print( SfxProgress &rProgress, sal_Bool bIsAPI, PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*     GetPrinter( bool bCreate = false ) override;
    virtual sal_uInt16      SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL ) override;
    virtual OUString        GetSelectionText( bool bCompleteWords = false, bool bOnlyASample = false ) override;
    virtual bool            HasSelection( bool bText = true ) const override;

    void                GetState( SfxItemSet& );
    void                ExecuteGlobal( SfxRequest& rReq );
    void                ExecuteSearch( SfxRequest& rReq );
    void                ExecuteCurrent( SfxRequest& rReq );
    void                ExecuteBasic( SfxRequest& rReq );
    void                ExecuteDialog( SfxRequest& rReq );

    virtual bool        HasUIFeature(SfxShellFeature nFeature) const override;

    bool                CallBasicErrorHdl( StarBASIC const * pBasic );
    BasicDebugFlags     CallBasicBreakHdl( StarBASIC const * pBasic );

    VclPtr<BaseWindow>   FindWindow( const ScriptDocument& rDocument, std::u16string_view rLibName, std::u16string_view rName, SbxItemType nSbxType, bool bFindSuspended = false );
    VclPtr<DialogWindow> FindDlgWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    VclPtr<ModulWindow>  FindBasWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName, bool bCreateIfNotExist = false, bool bFindSuspended = false );
    VclPtr<BaseWindow>   FindApplicationWindow();
    bool                 NextPage( bool bPrev );

    bool                IsAppBasicModified () const { return m_bAppBasicModified; }
    void                SetAppBasicModified (bool bModified) { m_bAppBasicModified = bModified; }

    // For Dialog Drag&Drop in Dialog Organizer:
    // (defined in moduldlg.cxx)
    static void CopyDialogResources(
        css::uno::Reference< css::io::XInputStreamProvider >& io_xISP,
        const ScriptDocument& rSourceDoc, const OUString& rSourceLibName, const ScriptDocument& rDestDoc,
        const OUString& rDestLibName, std::u16string_view rDlgName );

    static void InvalidateControlSlots();

    virtual css::uno::Reference< css::frame::XModel >
                        GetCurrentDocument() const override;

    void UpdateObjectCatalog () { aObjectCatalog->UpdateEntries(); }

    void RemoveWindow (BaseWindow* pWindow, bool bDestroy, bool bAllowChangeCurWindow = true);

    const std::shared_ptr<BasicColorConfig>& GetColorConfig() const { return m_aColorConfig; }
};

} // namespace basctl

// This typedef helps baside.sdi,
// because I don't know how to use nested names in it.
typedef basctl::Shell basctl_Shell;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
