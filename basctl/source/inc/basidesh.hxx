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
#ifndef _BASIDESH_HXX
#define _BASIDESH_HXX

#include "sbxitem.hxx"

#include "scriptdocument.hxx"
#include "doceventnotifier.hxx"
#include <sfx2/viewfac.hxx>
#include <sfx2/shell.hxx>
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <sfx2/viewsh.hxx>
#include <svx/ifaceids.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif

#include <com/sun/star/container/XContainerListener.hpp>

//----------------------------------------------------------------------------

const ULONG BASICIDE_UI_FEATURE_SHOW_BROWSER = 0x00000001;

//----------------------------------------------------------------------------

class ModulWindow;
class ModulWindowLayout;
class DialogWindow;
class SdrView;
class ObjectCatalog;
class BasicIDETabBar;
class TabBar;
class IDEBaseWindow;
class SbxObject;
class SbModule;
class StarBASIC;
class LocalizationMgr;
struct BasicIDEShell_Impl;

#if _SOLAR__PRIVATE
DECLARE_TABLE( IDEWindowTable, IDEBaseWindow* )
#else
typedef Table IDEWindowTable;
#endif

namespace BasicIDE
{
    bool RemoveDialog( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );
}

class BasicIDEShell :public SfxViewShell
                    ,public ::basctl::DocumentEventListener
{
friend class JavaDebuggingListenerImpl;
friend class LocalizationMgr;
friend BOOL implImportDialog( Window* pWin, const String& rCurPath, const ScriptDocument& rDocument, const String& aLibName );
friend bool BasicIDE::RemoveDialog( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

    ObjectCatalog*      pObjectCatalog;

    IDEWindowTable      aIDEWindowTable;
    USHORT              nCurKey;
    IDEBaseWindow*      pCurWin;
    ScriptDocument      m_aCurDocument;
    String              m_aCurLibName;
    LocalizationMgr*    m_pCurLocalizationMgr;

    ScrollBar           aHScrollBar;
    ScrollBar           aVScrollBar;
    ScrollBarBox        aScrollBarBox;
    BasicIDETabBar*     pTabBar;
    BOOL                bTabBarSplitted;
    BOOL                bCreatingWindow;
    ModulWindowLayout*  pModulLayout;
    BOOL                m_bAppBasicModified;
    ::basctl::DocumentEventNotifier
                        m_aNotifier;
friend class ContainerListenerImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xLibListener;

#if _SOLAR__PRIVATE
    void                Init();
    void                InitTabBar();
    void                InitScrollBars();
    void                CheckWindows();
    void                RemoveWindows( const ScriptDocument& rDocument, const String& rLibName, BOOL bDestroy );
    void                UpdateWindows();
    void                ShowObjectDialog( BOOL bShow, BOOL bCreateOrDestroy );
    void                InvalidateBasicIDESlots();
    void                StoreAllWindowData( BOOL bPersistent = TRUE );
    void                SetMDITitle();
    void                EnableScrollbars( BOOL bEnable );
    void                SetCurLib( const ScriptDocument& rDocument, String aLibName, bool bUpdateWindows = true , bool bCheck = true );
    void                SetCurLibForLocalization( const ScriptDocument& rDocument, String aLibName );

    void                ImplStartListening( StarBASIC* pBasic );

    DECL_LINK( TabBarHdl, TabBar* );
    DECL_LINK( AccelSelectHdl, Accelerator* );
    DECL_LINK( ObjectDialogCancelHdl, ObjectCatalog * );
    DECL_LINK( TabBarSplitHdl, TabBar * );
#endif

protected:
    virtual void        AdjustPosSizePixel( const Point &rPos, const Size &rSize );
    virtual void        OuterResizePixel( const Point &rPos, const Size &rSize );
    virtual Size        GetOptimalSizePixel() const;
    USHORT              InsertWindowInTable( IDEBaseWindow* pNewWin );
    virtual USHORT      PrepareClose( BOOL bUI, BOOL bForBrowsing );

    void                SetCurWindow( IDEBaseWindow* pNewWin, BOOL bUpdateTabBar = FALSE, BOOL bRememberAsCurrent = TRUE );
    void                ManageToolbars();
    void                RemoveWindow( IDEBaseWindow* pWindow, BOOL bDestroy, BOOL bAllowChangeCurWindow = TRUE );
    void                ArrangeTabBar();

    ModulWindow*        CreateBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName );
    DialogWindow*       CreateDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

    ModulWindow*        FindBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, BOOL bCreateIfNotExist, BOOL bFindSuspended = FALSE );
    ModulWindow*        ShowActiveModuleWindow( StarBASIC* pBasic );

    virtual void        SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                const SfxHint& rHint, const TypeId& rHintType );

    virtual void        Activate(BOOL bMDI);
    virtual void        Deactivate(BOOL bMDI);

    virtual void        Move();
    virtual void        ShowCursor( FASTBOOL bOn = TRUE );

    void                CreateModulWindowLayout();
    void                DestroyModulWindowLayout();
    void                UpdateModulWindowLayout( bool bBasicStopped );

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
                        SFX_DECL_VIEWFACTORY(BasicIDEShell);

                        BasicIDEShell( SfxViewFrame *pFrame, SfxViewShell *pOldSh );
                        ~BasicIDEShell();

    IDEBaseWindow*      GetCurWindow() const    { return pCurWin; }
    const ScriptDocument&
                        GetCurDocument() const { return m_aCurDocument; }
    const String&       GetCurLibName() const { return m_aCurLibName; }
    ObjectCatalog*      GetObjectCatalog() const    { return pObjectCatalog; }
    LocalizationMgr*    GetCurLocalizationMgr() const { return m_pCurLocalizationMgr; }

    ScrollBar&          GetHScrollBar()         { return aHScrollBar; }
    ScrollBar&          GetVScrollBar()         { return aVScrollBar; }
    ScrollBarBox&       GetScrollBarBox()       { return aScrollBarBox; }
    TabBar*             GetTabBar()             { return (TabBar*)pTabBar; }
    IDEWindowTable&     GetIDEWindowTable()     { return aIDEWindowTable; }

    SdrView*            GetCurDlgView() const;

    ::svl::IUndoManager*
                        GetUndoManager();

    virtual com::sun::star::uno::Reference< com::sun::star::view::XRenderable > GetRenderable();

    // virtual USHORT           Print( SfxProgress &rProgress, BOOL bIsAPI, PrintDialog *pPrintDialog = 0 );
    virtual SfxPrinter*     GetPrinter( BOOL bCreate );
    virtual USHORT          SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual String          GetSelectionText( BOOL bCompleteWords );
    virtual BOOL            HasSelection( BOOL bText ) const;

    void                GetState( SfxItemSet& );
    void                ExecuteGlobal( SfxRequest& rReq );
    void                ExecuteCurrent( SfxRequest& rReq );
    void                ExecuteBasic( SfxRequest& rReq );
    void                ExecuteDialog( SfxRequest& rReq );

    virtual sal_Bool    HasUIFeature( sal_uInt32 nFeature );

    long                CallBasicErrorHdl( StarBASIC* pBasic );
    long                CallBasicBreakHdl( StarBASIC* pBasic );

    ModulWindowLayout*  GetLayoutWindow() const { return pModulLayout; }

    IDEBaseWindow*      FindWindow( const ScriptDocument& rDocument, const String& rLibName = String(), const String& rName = String(), USHORT nType = BASICIDE_TYPE_UNKNOWN, BOOL bFindSuspended = FALSE );
    DialogWindow*       FindDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName, BOOL bCreateIfNotExist, BOOL bFindSuspended = FALSE );
    IDEBaseWindow*      FindApplicationWindow();
    BOOL                NextPage( BOOL bPrev = FALSE );

    BOOL                IsAppBasicModified() const { return m_bAppBasicModified; }
    void                SetAppBasicModified( BOOL bModified = TRUE ) { m_bAppBasicModified = bModified; }

    // For Dialog Drag&Drop in Dialog Organizer
    static void CopyDialogResources(
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >& io_xISP,
        const ScriptDocument& rSourceDoc, const String& rSourceLibName, const ScriptDocument& rDestDoc,
        const String& rDestLibName, const String& rDlgName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        GetCurrentDocument() const;

};

#endif // _BASIDESH_HXX
