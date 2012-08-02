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

// CLOOKS:
#define _SPIN_HXX

#define _SOLAR__PRIVATE 1
#include <tools/diagnose_ex.h>
#include <basic/basmgr.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basicbox.hxx>
#include <editeng/sizeitem.hxx>
#include <objdlg.hxx>
#include <tbxctl.hxx>
#include <iderdll2.hxx>
#include <basidectrlr.hxx>
#include <localizationmgr.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objface.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/srchitem.hxx>

#define BasicIDEShell
#define SFX_TYPEMAP
#include <idetemp.hxx>
#include <basslots.hxx>
#include <iderdll.hxx>
#include <svx/pszctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/srchdlg.hxx>
#include <svx/tbcontrl.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <svx/xmlsecctrl.hxx>
#include <sfx2/viewfac.hxx>
#include <vcl/msgbox.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;

typedef ::cppu::WeakImplHelper1< container::XContainerListener > ContainerListenerBASE;

class ContainerListenerImpl : public ContainerListenerBASE
{
    BasicIDEShell* mpShell;
public:

    ContainerListenerImpl( BasicIDEShell* pShell ) : mpShell( pShell ) {}

    ~ContainerListenerImpl()
    {
    }

    void addContainerListener( const ScriptDocument& rScriptDocument, const ::rtl::OUString& aLibName )
    {
        try
        {
            uno::Reference< container::XContainer > xContainer( rScriptDocument.getLibrary( E_SCRIPTS, aLibName, sal_False ), uno::UNO_QUERY );
            if ( xContainer.is() )
            {
                uno::Reference< container::XContainerListener > xContainerListener( this );
                xContainer->addContainerListener( xContainerListener );
            }
        }
        catch(const uno::Exception& ) {}
    }
    void removeContainerListener( const ScriptDocument& rScriptDocument, const ::rtl::OUString& aLibName )
    {
        try
        {
            uno::Reference< container::XContainer > xContainer( rScriptDocument.getLibrary( E_SCRIPTS, aLibName, sal_False ), uno::UNO_QUERY );
            if ( xContainer.is() )
            {
                uno::Reference< container::XContainerListener > xContainerListener( this );
                xContainer->removeContainerListener( xContainerListener );
            }
        }
        catch(const uno::Exception& ) {}
    }

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& ) throw( uno::RuntimeException ) {}

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& Event ) throw( uno::RuntimeException )
    {
        rtl::OUString sModuleName;
        if( mpShell && ( Event.Accessor >>= sModuleName ) )
            mpShell->FindBasWin( mpShell->m_aCurDocument, mpShell->m_aCurLibName, sModuleName, true, false );
    }
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& ) throw( com::sun::star::uno::RuntimeException ) { }
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& Event ) throw( com::sun::star::uno::RuntimeException )
    {
        rtl::OUString sModuleName;
        if( mpShell  && ( Event.Accessor >>= sModuleName ) )
        {
            IDEBaseWindow* pWin = mpShell->FindWindow( mpShell->m_aCurDocument, mpShell->m_aCurLibName, sModuleName, BASICIDE_TYPE_MODULE, true );
            if( pWin )
                mpShell->RemoveWindow( pWin, true, true );
        }
    }

};

TYPEINIT1( BasicIDEShell, SfxViewShell );

SFX_IMPL_NAMED_VIEWFACTORY( BasicIDEShell, "Default" )
{
    SFX_VIEW_REGISTRATION( BasicDocShell );
}


SFX_IMPL_INTERFACE( BasicIDEShell, SfxViewShell, IDEResId( RID_STR_IDENAME ) )
{
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(SID_SHOW_PROPERTYBROWSER, BASICIDE_UI_FEATURE_SHOW_BROWSER);
    SFX_POPUPMENU_REGISTRATION( IDEResId( RID_POPUP_DLGED ) );
}



#define IDE_VIEWSHELL_FLAGS     SFX_VIEW_CAN_PRINT|SFX_VIEW_NO_NEWWINDOW


static sal_Int32 GnBasicIDEShellCount;
sal_Int32 getBasicIDEShellCount( void )
    { return GnBasicIDEShellCount; }

BasicIDEShell::BasicIDEShell( SfxViewFrame* pFrame_, SfxViewShell* /* pOldShell */ ) :
        SfxViewShell( pFrame_, IDE_VIEWSHELL_FLAGS ),
        m_aCurDocument( ScriptDocument::getApplicationScriptDocument() ),
        aHScrollBar( &GetViewFrame()->GetWindow(), WinBits( WB_HSCROLL | WB_DRAG ) ),
        aVScrollBar( &GetViewFrame()->GetWindow(), WinBits( WB_VSCROLL | WB_DRAG ) ),
        aScrollBarBox( &GetViewFrame()->GetWindow(), WinBits( WB_SIZEABLE ) ),
        m_bAppBasicModified( sal_False ),
        m_aNotifier( *this )
{
    m_xLibListener = new ContainerListenerImpl( this );
    Init();
    GnBasicIDEShellCount++;
}



void BasicIDEShell::Init()
{
    TbxControls::RegisterControl( SID_CHOOSE_CONTROLS );
    SvxPosSizeStatusBarControl::RegisterControl();
    SvxInsertStatusBarControl::RegisterControl();
    XmlSecStatusBarControl::RegisterControl( SID_SIGNATURE );
    SvxSimpleUndoRedoController::RegisterControl( SID_UNDO );
    SvxSimpleUndoRedoController::RegisterControl( SID_REDO );

    SvxSearchDialogWrapper::RegisterChildWindow( sal_False );

    BasicIDEGlobals::GetExtraData()->ShellInCriticalSection() = true;

    SetName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BasicIDE" ) ) );
    SetHelpId( SVX_INTERFACE_BASIDE_VIEWSH );

    LibBoxControl::RegisterControl( SID_BASICIDE_LIBSELECTOR );
    LanguageBoxControl::RegisterControl( SID_BASICIDE_CURRENT_LANG );

    CreateModulWindowLayout();

    GetViewFrame()->GetWindow().SetBackground();

    pCurWin = 0;
    m_aCurDocument = ScriptDocument::getApplicationScriptDocument();
    pObjectCatalog = 0;
    bCreatingWindow = false;

    pTabBar = new BasicIDETabBar( &GetViewFrame()->GetWindow() );
    pTabBar->SetSplitHdl( LINK( this, BasicIDEShell, TabBarSplitHdl ) );
    bTabBarSplitted = false;

    nCurKey = 100;
    InitScrollBars();
    InitTabBar();

    SetCurLib( ScriptDocument::getApplicationScriptDocument(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Standard")), false, false );

    BasicIDEGlobals::ShellCreated(this);

    BasicIDEGlobals::GetExtraData()->ShellInCriticalSection() = false;

    // It's enough to create the controller ...
    // It will be public by using magic :-)
    new BasicIDEController( this );

    // Force updating the title ! Because it must be set to the controller
    // it has to be called directly after creating those controller.
    SetMDITitle ();

    UpdateWindows();
}

BasicIDEShell::~BasicIDEShell()
{
    m_aNotifier.dispose();

    BasicIDEGlobals::ShellDestroyed(this);

    // so that on a basic saving error, the shell doesn't pop right up again
    BasicIDEGlobals::GetExtraData()->ShellInCriticalSection() = true;

    SetWindow( 0 );
    SetCurWindow( 0 );

    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        // no store; does already happen when the BasicManagers are destroyed
        delete it->second;
    }

    aIDEWindowTable.clear();
    delete pTabBar;
    delete pObjectCatalog;
    DestroyModulWindowLayout();

        ContainerListenerImpl* pListener = static_cast< ContainerListenerImpl* >( m_xLibListener.get() );
        // Destroy all ContainerListeners for Basic Container.
        if ( pListener )
            pListener->removeContainerListener( m_aCurDocument, m_aCurLibName );

    BasicIDEGlobals::GetExtraData()->ShellInCriticalSection() = false;

    GnBasicIDEShellCount--;
}

void BasicIDEShell::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
{
    if(pCurWin && pCurWin->IsA( TYPE(ModulWindow)))
    {
        dynamic_cast<ModulWindow*>(pCurWin)->SetLineNumberDisplay(SourceLinesDisplayed());
    }
    UpdateWindows();
}

void BasicIDEShell::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
{
    if(pCurWin && pCurWin->IsA( TYPE(ModulWindow)))
    {
        dynamic_cast<ModulWindow*>(pCurWin)->SetLineNumberDisplay(SourceLinesDisplayed());
    }
    UpdateWindows();
}

void BasicIDEShell::onDocumentSave( const ScriptDocument& /*_rDocument*/ )
{
    StoreAllWindowData();
}

void BasicIDEShell::onDocumentSaveDone( const ScriptDocument& /*_rDocument*/ )
{
    // #i115671: Update SID_SAVEDOC after saving is completed
    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
        pBindings->Invalidate( SID_SAVEDOC );
}

void BasicIDEShell::onDocumentSaveAs( const ScriptDocument& /*_rDocument*/ )
{
    StoreAllWindowData();
}

void BasicIDEShell::onDocumentSaveAsDone( const ScriptDocument& /*_rDocument*/ )
{
    // not interested in
}

void BasicIDEShell::onDocumentClosed( const ScriptDocument& _rDocument )
{
    if ( !_rDocument.isValid() )
        return;

    bool bSetCurWindow = false;
    bool bSetCurLib = ( _rDocument == m_aCurDocument );
    std::vector<IDEBaseWindow*> aDeleteVec;

    // remove all windows which belong to this document
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( pWin->IsDocument( _rDocument ) )
        {
            if ( pWin->GetStatus() & (BASWIN_RUNNINGBASIC|BASWIN_INRESCHEDULE) )
            {
                pWin->AddStatus( BASWIN_TOBEKILLED );
                pWin->Hide();
                StarBASIC::Stop();
                // there's no notify
                pWin->BasicStopped();
            }
            else
                aDeleteVec.push_back( pWin );
        }
    }
    // delete windows outside main loop so we don't invalidate the original iterator
    for( std::vector<IDEBaseWindow*>::const_iterator it = aDeleteVec.begin(); it != aDeleteVec.end(); ++it )
    {
        IDEBaseWindow* pWin = *it;
        pWin->StoreData();
        if ( pWin == pCurWin )
            bSetCurWindow = true;
        RemoveWindow( pWin, true, false );
    }

    // remove lib info
    BasicIDEData* pData = BasicIDEGlobals::GetExtraData();
    if ( pData )
        pData->GetLibInfos().RemoveInfoFor( _rDocument );

    if ( bSetCurLib )
        SetCurLib( ScriptDocument::getApplicationScriptDocument(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Standard")), true, false );
    else if ( bSetCurWindow )
        SetCurWindow( FindApplicationWindow(), true );
}

void BasicIDEShell::onDocumentTitleChanged( const ScriptDocument& /*_rDocument*/ )
{
    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
    if ( pBindings )
        pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR, sal_True, sal_False );
    SetMDITitle();
}

void BasicIDEShell::onDocumentModeChanged( const ScriptDocument& _rDocument )
{
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( pWin->IsDocument( _rDocument ) && _rDocument.isDocument() )
            pWin->SetReadOnly( _rDocument.isReadOnly() );
    }
}

void BasicIDEShell::StoreAllWindowData( bool bPersistent )
{
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        DBG_ASSERT( pWin, "PrepareClose: NULL-Pointer in Table?" );
        if ( !pWin->IsSuspended() )
            pWin->StoreData();
    }

    if ( bPersistent  )
    {
        SFX_APP()->SaveBasicAndDialogContainer();
        SetAppBasicModified( sal_False );

        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
        {
            pBindings->Invalidate( SID_SAVEDOC );
            pBindings->Update( SID_SAVEDOC );
        }
    }
}


sal_uInt16 BasicIDEShell::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    (void)bForBrowsing;

    // reset here because it's modified after printing etc. (DocInfo)
    GetViewFrame()->GetObjectShell()->SetModified(sal_False);

    if ( StarBASIC::IsRunning() )
    {
        if( bUI )
        {
            Window *pParent = &GetViewFrame()->GetWindow();
            InfoBox( pParent, IDE_RESSTR(RID_STR_CANNOTCLOSE)).Execute();
        }
        return sal_False;
    }
    else
    {
        sal_Bool bCanClose = sal_True;
        for (IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); bCanClose && (it != aIDEWindowTable.end()); ++it)
        {
            IDEBaseWindow* pWin = it->second;
            if ( !pWin->CanClose() )
            {
                if ( !m_aCurLibName.isEmpty() && ( pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName ) )
                    SetCurLib( ScriptDocument::getApplicationScriptDocument(), ::rtl::OUString(), false );
                SetCurWindow( pWin, true );
                bCanClose = sal_False;
            }
        }

        if ( bCanClose )
            StoreAllWindowData( false );    // don't write on the disk, that will be done later automatically

        return bCanClose;
    }
}

void BasicIDEShell::InitScrollBars()
{
    aVScrollBar.SetLineSize( 300 );
    aVScrollBar.SetPageSize( 2000 );
    aHScrollBar.SetLineSize( 300 );
    aHScrollBar.SetPageSize( 2000 );
    aHScrollBar.Enable();
    aVScrollBar.Enable();
    aVScrollBar.Show();
    aHScrollBar.Show();
    aScrollBarBox.Show();
}



void BasicIDEShell::InitTabBar()
{
    pTabBar->Enable();
    pTabBar->Show();
    pTabBar->SetSelectHdl( LINK( this, BasicIDEShell, TabBarHdl ) );
}


Size BasicIDEShell::GetOptimalSizePixel() const
{
    return Size( 400, 300 );
}



void BasicIDEShell::OuterResizePixel( const Point &rPos, const Size &rSize )
{
    AdjustPosSizePixel( rPos, rSize );
}


IMPL_LINK_INLINE_START( BasicIDEShell, TabBarSplitHdl, TabBar *, pTBar )
{
    (void)pTBar;
    bTabBarSplitted = true;
    ArrangeTabBar();

    return 0;
}
IMPL_LINK_INLINE_END( BasicIDEShell, TabBarSplitHdl, TabBar *, pTBar )



IMPL_LINK( BasicIDEShell, TabBarHdl, TabBar *, pCurTabBar )
{
    sal_uInt16 nCurId = pCurTabBar->GetCurPageId();
    IDEBaseWindow* pWin = aIDEWindowTable[ nCurId ];
    DBG_ASSERT( pWin, "Eintrag in TabBar passt zu keinem Fenster!" );
    SetCurWindow( pWin );

    return 0;
}



bool BasicIDEShell::NextPage( bool bPrev )
{
    bool bRet = false;
    sal_uInt16 nPos = pTabBar->GetPagePos( pTabBar->GetCurPageId() );

    if ( bPrev )
        --nPos;
    else
        ++nPos;

    if ( nPos < pTabBar->GetPageCount() )
    {
        IDEBaseWindow* pWin = aIDEWindowTable[ pTabBar->GetPageId( nPos ) ];
        SetCurWindow( pWin, true );
        bRet = true;
    }

    return bRet;
}



void BasicIDEShell::ArrangeTabBar()
{
    long nBoxPos = aScrollBarBox.GetPosPixel().X() - 1;
    long nPos = pTabBar->GetSplitSize();
    if ( nPos <= nBoxPos )
    {
        Point aPnt( pTabBar->GetPosPixel() );
        long nH = aHScrollBar.GetSizePixel().Height();
        pTabBar->SetPosSizePixel( aPnt, Size( nPos, nH ) );
        long nScrlStart = aPnt.X() + nPos;
        aHScrollBar.SetPosSizePixel( Point( nScrlStart, aPnt.Y() ), Size( nBoxPos - nScrlStart + 2, nH ) );
        aHScrollBar.Update();
    }
}



::svl::IUndoManager* BasicIDEShell::GetUndoManager()
{
    ::svl::IUndoManager* pMgr = NULL;
    if( pCurWin )
        pMgr = pCurWin->GetUndoManager();

    return pMgr;
}



void BasicIDEShell::ShowObjectDialog( bool bShow, bool bCreateOrDestroy )
{
    if ( bShow )
    {
        if ( !pObjectCatalog && bCreateOrDestroy )
        {
            pObjectCatalog = new ObjectCatalog( &GetViewFrame()->GetWindow() );
            // position is memorized in BasicIDEData and adjusted by the Dlg
            if ( pObjectCatalog )
            {
                pObjectCatalog->SetCancelHdl( LINK( this, BasicIDEShell, ObjectDialogCancelHdl ) );
                pObjectCatalog->SetCurrentEntry(pCurWin);
            }
        }

        // the very last changes...
        if ( pCurWin )
            pCurWin->StoreData();

        if ( pObjectCatalog )
        {
            pObjectCatalog->UpdateEntries();
            pObjectCatalog->Show();
        }
    }
    else if ( pObjectCatalog )
    {
        pObjectCatalog->Hide();
        if ( bCreateOrDestroy )
        {
            // pObjectCatalog to NULL before the delete because of OS/2-focus-problem
            ObjectCatalog* pTemp = pObjectCatalog;
            pObjectCatalog = 0;
            delete pTemp;
        }
    }
}



void BasicIDEShell::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId&,
                                        const SfxHint& rHint, const TypeId& )
{
    if ( BasicIDEGlobals::GetShell() )
    {
        if ( rHint.IsA( TYPE( SfxSimpleHint ) ) )
        {
            switch ( ((SfxSimpleHint&)rHint).GetId() )
            {
                case SFX_HINT_DYING:
                {
                    EndListening( rBC, sal_True /* log off all */ );
                    if ( pObjectCatalog )
                        pObjectCatalog->UpdateEntries();
                }
                break;
            }

            if ( rHint.IsA( TYPE( SbxHint ) ) )
            {
                SbxHint& rSbxHint = (SbxHint&)rHint;
                sal_uLong nHintId = rSbxHint.GetId();
                if (    ( nHintId == SBX_HINT_BASICSTART ) ||
                        ( nHintId == SBX_HINT_BASICSTOP ) )
                {
                    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                    if ( pBindings )
                    {
                        pBindings->Invalidate( SID_BASICRUN );
                        pBindings->Update( SID_BASICRUN );
                        pBindings->Invalidate( SID_BASICCOMPILE );
                        pBindings->Update( SID_BASICCOMPILE );
                        pBindings->Invalidate( SID_BASICSTEPOVER );
                        pBindings->Update( SID_BASICSTEPOVER );
                        pBindings->Invalidate( SID_BASICSTEPINTO );
                        pBindings->Update( SID_BASICSTEPINTO );
                        pBindings->Invalidate( SID_BASICSTEPOUT );
                        pBindings->Update( SID_BASICSTEPOUT );
                        pBindings->Invalidate( SID_BASICSTOP );
                        pBindings->Update( SID_BASICSTOP );
                        pBindings->Invalidate( SID_BASICIDE_TOGGLEBRKPNT );
                        pBindings->Update( SID_BASICIDE_TOGGLEBRKPNT );
                        pBindings->Invalidate( SID_BASICIDE_MANAGEBRKPNTS );
                        pBindings->Update( SID_BASICIDE_MANAGEBRKPNTS );
                        pBindings->Invalidate( SID_BASICIDE_MODULEDLG );
                        pBindings->Update( SID_BASICIDE_MODULEDLG );
                        pBindings->Invalidate( SID_BASICLOAD );
                        pBindings->Update( SID_BASICLOAD );
                    }

                    if ( nHintId == SBX_HINT_BASICSTOP )
                    {
                        // not only at error/break or explicit stoppage,
                        // if the update is turned off due to a programming bug
                        BasicIDE::BasicStopped();
                        UpdateModulWindowLayout( true );    // clear...
                        if( m_pCurLocalizationMgr )
                            m_pCurLocalizationMgr->handleBasicStopped();
                    }
                    else if( m_pCurLocalizationMgr )
                    {
                        m_pCurLocalizationMgr->handleBasicStarted();
                    }

                    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin();
                         it != aIDEWindowTable.end(); ++it )
                    {
                        IDEBaseWindow* pWin = it->second;
                        if ( nHintId == SBX_HINT_BASICSTART )
                            pWin->BasicStarted();
                        else
                            pWin->BasicStopped();
                    }
                }
            }
        }
    }
}



void BasicIDEShell::CheckWindows()
{
    bool bSetCurWindow = false;
    std::vector<IDEBaseWindow*> aDeleteVec;
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( pWin->GetStatus() & BASWIN_TOBEKILLED )
            aDeleteVec.push_back( pWin );
    }
    for ( std::vector<IDEBaseWindow*>::const_iterator it = aDeleteVec.begin(); it != aDeleteVec.end(); ++it )
    {
        IDEBaseWindow* pWin = *it;
        pWin->StoreData();
        if ( pWin == pCurWin )
            bSetCurWindow = true;
        RemoveWindow( pWin, true, false );
    }
    if ( bSetCurWindow )
        SetCurWindow( FindApplicationWindow(), true );
}



void BasicIDEShell::RemoveWindows( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, bool bDestroy )
{
    bool bChangeCurWindow = pCurWin ? false : true;
    std::vector<IDEBaseWindow*> aDeleteVec;
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName )
            aDeleteVec.push_back( pWin );
    }
    for ( std::vector<IDEBaseWindow*>::const_iterator it = aDeleteVec.begin(); it != aDeleteVec.end(); ++it )
    {
        IDEBaseWindow* pWin = *it;
        if ( pWin == pCurWin )
            bChangeCurWindow = true;
        pWin->StoreData();
        RemoveWindow( pWin, bDestroy, false );
    }
    if ( bChangeCurWindow )
        SetCurWindow( FindApplicationWindow(), true );
}



void BasicIDEShell::UpdateWindows()
{
    // remove all windows that may not be displayed
    bool bChangeCurWindow = pCurWin ? false : true;
    if ( !m_aCurLibName.isEmpty() )
    {
        std::vector<IDEBaseWindow*> aDeleteVec;
        for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin(); it != aIDEWindowTable.end(); ++it )
        {
            IDEBaseWindow* pWin = it->second;
            if ( !pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName )
            {
                if ( pWin == pCurWin )
                    bChangeCurWindow = true;
                pWin->StoreData();
                // The request of RUNNING prevents the crash when in reschedule.
                // Window is frozen at first, later the windows should be changed
                // anyway to be marked as hidden instead of being deleted.
                if ( !(pWin->GetStatus() & ( BASWIN_TOBEKILLED | BASWIN_RUNNINGBASIC | BASWIN_SUSPENDED ) ) )
                    aDeleteVec.push_back( pWin );
            }
        }
        for ( std::vector<IDEBaseWindow*>::const_iterator it = aDeleteVec.begin(); it != aDeleteVec.end(); ++it )
        {
            RemoveWindow( *it, false, false );
        }
    }

    if ( bCreatingWindow )
        return;

    IDEBaseWindow* pNextActiveWindow = 0;

    // show all windows that are to be shown
    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::AllWithApplication ) );
    for (   ScriptDocuments::const_iterator doc = aDocuments.begin();
            doc != aDocuments.end();
            ++doc
        )
    {
        StartListening( *doc->getBasicManager(), sal_True /* log on only once */ );

        // libraries
        Sequence< ::rtl::OUString > aLibNames( doc->getLibraryNames() );
        sal_Int32 nLibCount = aLibNames.getLength();
        const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

        for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        {
            ::rtl::OUString aLibName = pLibNames[ i ];

            if ( m_aCurLibName.isEmpty() || ( *doc == m_aCurDocument && aLibName == m_aCurLibName ) )
            {
                // check, if library is password protected and not verified
                bool bProtected = false;
                Reference< script::XLibraryContainer > xModLibContainer( doc->getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        bProtected = true;
                    }
                }

                if ( !bProtected )
                {
                    LibInfoItem* pLibInfoItem = 0;
                    BasicIDEData* pData = BasicIDEGlobals::GetExtraData();
                    if ( pData )
                        pLibInfoItem = pData->GetLibInfos().GetInfo( LibInfoKey( *doc, aLibName ) );

                    // modules
                    if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                    {
                        StarBASIC* pLib = doc->getBasicManager()->GetLib( aLibName );
                        if ( pLib )
                            ImplStartListening( pLib );

                        try
                        {
                            Sequence< ::rtl::OUString > aModNames( doc->getObjectNames( E_SCRIPTS, aLibName ) );
                            sal_Int32 nModCount = aModNames.getLength();
                            const ::rtl::OUString* pModNames = aModNames.getConstArray();

                            for ( sal_Int32 j = 0 ; j < nModCount ; j++ )
                            {
                                ::rtl::OUString aModName = pModNames[ j ];
                                ModulWindow* pWin = FindBasWin( *doc, aLibName, aModName, false );
                                if ( !pWin )
                                    pWin = CreateBasWin( *doc, aLibName, aModName );
                                if ( !pNextActiveWindow && pLibInfoItem && pLibInfoItem->GetCurrentName() == aModName &&
                                     pLibInfoItem->GetCurrentType() == BASICIDE_TYPE_MODULE )
                                {
                                    pNextActiveWindow = (IDEBaseWindow*)pWin;
                                }
                            }
                        }
                        catch (const container::NoSuchElementException& )
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }

                    // dialogs
                    Reference< script::XLibraryContainer > xDlgLibContainer( doc->getLibraryContainer( E_DIALOGS ) );
                    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) )
                    {
                        try
                        {
                            Sequence< ::rtl::OUString > aDlgNames = doc->getObjectNames( E_DIALOGS, aLibName );
                            sal_Int32 nDlgCount = aDlgNames.getLength();
                            const ::rtl::OUString* pDlgNames = aDlgNames.getConstArray();

                            for ( sal_Int32 j = 0 ; j < nDlgCount ; j++ )
                            {
                                ::rtl::OUString aDlgName = pDlgNames[ j ];
                                // this find only looks for non-suspended windows;
                                // suspended windows are handled in CreateDlgWin
                                DialogWindow* pWin = FindDlgWin( *doc, aLibName, aDlgName, false );
                                if ( !pWin )
                                    pWin = CreateDlgWin( *doc, aLibName, aDlgName );
                                if ( !pNextActiveWindow && pLibInfoItem && pLibInfoItem->GetCurrentName() == aDlgName &&
                                     pLibInfoItem->GetCurrentType() == BASICIDE_TYPE_DIALOG )
                                {
                                    pNextActiveWindow = (IDEBaseWindow*)pWin;
                                }
                            }
                        }
                        catch (const container::NoSuchElementException& )
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }
                }
            }
        }
    }

    if ( bChangeCurWindow )
    {
        if ( !pNextActiveWindow )
            pNextActiveWindow = FindApplicationWindow();
        SetCurWindow( pNextActiveWindow, true );
    }
}

void BasicIDEShell::RemoveWindow( IDEBaseWindow* pWindow_, bool bDestroy, bool bAllowChangeCurWindow )
{
    DBG_ASSERT( pWindow_, "Kann keinen NULL-Pointer loeschen!" );
    sal_uLong nKey = GetIDEWindowId( pWindow_ );
    pTabBar->RemovePage( (sal_uInt16)nKey );
    aIDEWindowTable.erase( nKey );
    if ( pWindow_ == pCurWin )
    {
        if ( bAllowChangeCurWindow )
            SetCurWindow( FindApplicationWindow(), true );
        else
            SetCurWindow( NULL, false );
    }
    if ( bDestroy )
    {
        if ( !( pWindow_->GetStatus() & BASWIN_INRESCHEDULE ) )
        {
            delete pWindow_;
        }
        else
        {
            pWindow_->AddStatus( BASWIN_TOBEKILLED );
            pWindow_->Hide();
            // In normal mode stop basic in windows to be deleted
            // In VBA stop basic only if the running script is trying to delete
            // its parent module
            bool bStop = true;
            if ( pWindow_->GetDocument().isInVBAMode() )
            {
                SbModule* pMod = StarBASIC::GetActiveModule();
                if ( !pMod || ( pMod && ( !pMod->GetName().Equals(pWindow_->GetName()) ) ) )
                    bStop = false;
            }
            if ( bStop )
            {
                StarBASIC::Stop();
                // there will be no notify...
                pWindow_->BasicStopped();
            }
            aIDEWindowTable[ nKey ] = pWindow_;   // jump in again
        }
    }
    else
    {
        pWindow_->Hide();
        pWindow_->AddStatus( BASWIN_SUSPENDED );
        pWindow_->Deactivating();
        aIDEWindowTable[ nKey ] = pWindow_;   // jump in again
    }

}



sal_uInt16 BasicIDEShell::InsertWindowInTable( IDEBaseWindow* pNewWin )
{
    nCurKey++;
    aIDEWindowTable[ nCurKey ] = pNewWin;
    return nCurKey;
}



void BasicIDEShell::InvalidateBasicIDESlots()
{
    // only those that have an optic effect...

    if ( BasicIDEGlobals::GetShell() )
    {
        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
        {
            pBindings->Invalidate( SID_UNDO );
            pBindings->Invalidate( SID_REDO );
            pBindings->Invalidate( SID_SAVEDOC );
            pBindings->Invalidate( SID_SIGNATURE );
            pBindings->Invalidate( SID_BASICIDE_CHOOSEMACRO );
            pBindings->Invalidate( SID_BASICIDE_MODULEDLG );
            pBindings->Invalidate( SID_BASICIDE_OBJCAT );
            pBindings->Invalidate( SID_BASICSTOP );
            pBindings->Invalidate( SID_BASICRUN );
            pBindings->Invalidate( SID_BASICCOMPILE );
            pBindings->Invalidate( SID_BASICLOAD );
            pBindings->Invalidate( SID_BASICSAVEAS );
            pBindings->Invalidate( SID_BASICIDE_MATCHGROUP );
            pBindings->Invalidate( SID_BASICSTEPINTO );
            pBindings->Invalidate( SID_BASICSTEPOVER );
            pBindings->Invalidate( SID_BASICSTEPOUT );
            pBindings->Invalidate( SID_BASICIDE_TOGGLEBRKPNT );
            pBindings->Invalidate( SID_BASICIDE_MANAGEBRKPNTS );
            pBindings->Invalidate( SID_BASICIDE_ADDWATCH );
            pBindings->Invalidate( SID_BASICIDE_REMOVEWATCH );
            pBindings->Invalidate( SID_CHOOSE_CONTROLS );
            pBindings->Invalidate( SID_PRINTDOC );
            pBindings->Invalidate( SID_PRINTDOCDIRECT );
            pBindings->Invalidate( SID_SETUPPRINTER );
            pBindings->Invalidate( SID_DIALOG_TESTMODE );

            pBindings->Invalidate( SID_DOC_MODIFIED );
            pBindings->Invalidate( SID_BASICIDE_STAT_TITLE );
            pBindings->Invalidate( SID_BASICIDE_STAT_POS );
            pBindings->Invalidate( SID_ATTR_INSERT );
            pBindings->Invalidate( SID_ATTR_SIZE );
        }
    }
}

void BasicIDEShell::EnableScrollbars( bool bEnable )
{
    if ( bEnable )
    {
        aHScrollBar.Enable();
        aVScrollBar.Enable();
    }
    else
    {
        aHScrollBar.Disable();
        aVScrollBar.Disable();
    }
}

void BasicIDEShell::SetCurLib( const ScriptDocument& rDocument, ::rtl::OUString aLibName, bool bUpdateWindows, bool bCheck )
{
    if ( !bCheck || ( rDocument != m_aCurDocument || aLibName != m_aCurLibName ) )
    {
        ContainerListenerImpl* pListener = static_cast< ContainerListenerImpl* >( m_xLibListener.get() );

        m_aCurDocument = rDocument;
        m_aCurLibName = aLibName;

        if ( pListener )
        {
            pListener->removeContainerListener( m_aCurDocument, m_aCurLibName );
            pListener->addContainerListener( m_aCurDocument, aLibName );
        }

        if ( bUpdateWindows )
            UpdateWindows();

        SetMDITitle();

        SetCurLibForLocalization( rDocument, aLibName );

        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
        {
            pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR );
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
            pBindings->Invalidate( SID_BASICIDE_MANAGE_LANG );
        }
    }
}

void BasicIDEShell::SetCurLibForLocalization( const ScriptDocument& rDocument, ::rtl::OUString aLibName )
{
    // Create LocalizationMgr
    Reference< resource::XStringResourceManager > xStringResourceManager;
    try
    {
        if( !aLibName.isEmpty() )
        {
            Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
            xStringResourceManager = LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
        }
    }
    catch (const container::NoSuchElementException& )
    {}

    m_pCurLocalizationMgr = boost::shared_ptr<LocalizationMgr>(new LocalizationMgr(this, rDocument, aLibName, xStringResourceManager));
    m_pCurLocalizationMgr->handleTranslationbar();
}

void BasicIDEShell::ImplStartListening( StarBASIC* pBasic )
{
    StartListening( pBasic->GetBroadcaster(), sal_True /* log on only once */ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
