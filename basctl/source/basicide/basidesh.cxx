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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

// CLOOKS:
//#define _MENUBTN_HXX
#define _SPIN_HXX
#define _PRVWIN_HXX
//#define _FIELD_HXX ***
//#define _TAB_HXX ***
#define _DIALOGS_HXX
#define _SVRTF_HXX
#define _ISETBRW_HXX
#define _VCTRLS_HXX
#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#define ITEMID_SIZE 0

// Falls ohne PCH's:
#include <ide_pch.hxx>


#define _SOLAR__PRIVATE 1
#include <basic/sbx.hxx>
#include <svl/hint.hxx>
#include <tools/diagnose_ex.h>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <bastypes.hxx>
#include <basicbox.hxx>
#include <objdlg.hxx>
#include <sbxitem.hxx>
#include <tbxctl.hxx>
#include <iderdll2.hxx>
#include <basidectrlr.hxx>
#include <localizationmgr.hxx>

#define BasicIDEShell
#define SFX_TYPEMAP
#include <idetemp.hxx>
#include <basslots.hxx>
#include <iderdll.hxx>
#include <svx/pszctrl.hxx>
#include <svx/insctrl.hxx>
#include <svx/srchdlg.hxx>
#include <svx/lboxctrl.hxx>
#include <svx/tbcontrl.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>

#include <svx/xmlsecctrl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;

static const rtl::OUString sStandardLibName( RTL_CONSTASCII_USTRINGPARAM("Standard"));

typedef ::cppu::WeakImplHelper1< container::XContainerListener > ContainerListenerBASE;

class ContainerListenerImpl : public ContainerListenerBASE
{
    BasicIDEShell* mpShell;
public:

    ContainerListenerImpl( BasicIDEShell* pShell ) : mpShell( pShell ) {}

    ~ContainerListenerImpl()
    {
    }

    void addContainerListener( const ScriptDocument& rScriptDocument, const String& aLibName )
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
        catch( uno::Exception& ) {}
    }
    void removeContainerListener( const ScriptDocument& rScriptDocument, const String& aLibName )
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
        catch( uno::Exception& ) {}
    }

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& ) throw( uno::RuntimeException ) {}

    // XContainerListener
    virtual void SAL_CALL elementInserted( const container::ContainerEvent& Event ) throw( uno::RuntimeException )
    {
        rtl::OUString sModuleName;
        if( mpShell && ( Event.Accessor >>= sModuleName ) )
            mpShell->FindBasWin( mpShell->m_aCurDocument, mpShell->m_aCurLibName, sModuleName, sal_True, sal_False );
    }
    virtual void SAL_CALL elementReplaced( const container::ContainerEvent& ) throw( com::sun::star::uno::RuntimeException ) { }
    virtual void SAL_CALL elementRemoved( const container::ContainerEvent& Event ) throw( com::sun::star::uno::RuntimeException )
    {
        rtl::OUString sModuleName;
        if( mpShell  && ( Event.Accessor >>= sModuleName ) )
        {
            IDEBaseWindow* pWin = mpShell->FindWindow( mpShell->m_aCurDocument, mpShell->m_aCurLibName, sModuleName, BASICIDE_TYPE_MODULE, sal_True );
            if( pWin )
                mpShell->RemoveWindow( pWin, sal_True, sal_True );
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

    IDE_DLL()->GetExtraData()->ShellInCriticalSection() = sal_True;

    SetName( String( RTL_CONSTASCII_USTRINGPARAM( "BasicIDE" ) ) );
    SetHelpId( SVX_INTERFACE_BASIDE_VIEWSH );

    LibBoxControl::RegisterControl( SID_BASICIDE_LIBSELECTOR );
    LanguageBoxControl::RegisterControl( SID_BASICIDE_CURRENT_LANG );

    CreateModulWindowLayout();

    GetViewFrame()->GetWindow().SetBackground();

    pCurWin = 0;
    m_aCurDocument = ScriptDocument::getApplicationScriptDocument();
    pObjectCatalog = 0;
    bCreatingWindow = sal_False;

    m_pCurLocalizationMgr = NULL;

    pTabBar = new BasicIDETabBar( &GetViewFrame()->GetWindow() );
    pTabBar->SetSplitHdl( LINK( this, BasicIDEShell, TabBarSplitHdl ) );
    bTabBarSplitted = sal_False;

    nCurKey = 100;
    InitScrollBars();
    InitTabBar();

    SetCurLib( ScriptDocument::getApplicationScriptDocument(), String::CreateFromAscii( "Standard" ), false, false );

    if ( IDE_DLL() && IDE_DLL()->pShell == NULL )
        IDE_DLL()->pShell = this;

    IDE_DLL()->GetExtraData()->ShellInCriticalSection() = sal_False;

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

    if ( IDE_DLL() && IDE_DLL()->pShell == this )
        IDE_DLL()->pShell = NULL;

    // Damit bei einem Basic-Fehler beim Speichern die Shell nicht sofort
    // wieder hoch kommt:
    IDE_DLL()->GetExtraData()->ShellInCriticalSection() = sal_True;

    SetWindow( 0 );
    SetCurWindow( 0 );

    // Alle Fenster zerstoeren:
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin )
    {
        // Kein Store, passiert bereits, wenn die BasicManager zerstoert werden.
        delete pWin;
        pWin = aIDEWindowTable.Next();
    }

    aIDEWindowTable.Clear();
    delete pTabBar;
    delete pObjectCatalog;
    DestroyModulWindowLayout();

        ContainerListenerImpl* pListener = static_cast< ContainerListenerImpl* >( m_xLibListener.get() );
        // Destroy all ContainerListeners for Basic Container.
        if ( pListener )
            pListener->removeContainerListener( m_aCurDocument, m_aCurLibName );

    IDE_DLL()->GetExtraData()->ShellInCriticalSection() = sal_False;

    GnBasicIDEShellCount--;
}

void BasicIDEShell::onDocumentCreated( const ScriptDocument& /*_rDocument*/ )
{
    UpdateWindows();
}

void BasicIDEShell::onDocumentOpened( const ScriptDocument& /*_rDocument*/ )
{
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

    // remove all windows which belong to this document
    for ( sal_uLong nWin = aIDEWindowTable.Count(); nWin; )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.GetObject( --nWin );
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
            {
                pWin->StoreData();
                if ( pWin == pCurWin )
                    bSetCurWindow = true;
                RemoveWindow( pWin, sal_True, sal_False );
            }
        }
    }

    // remove lib info
    BasicIDEData* pData = IDE_DLL()->GetExtraData();
    if ( pData )
        pData->GetLibInfos().RemoveInfoFor( _rDocument );

    if ( bSetCurLib )
        SetCurLib( ScriptDocument::getApplicationScriptDocument(), String::CreateFromAscii( "Standard" ), true, false );
    else if ( bSetCurWindow )
        SetCurWindow( FindApplicationWindow(), sal_True );
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
    for ( sal_uLong nWin = aIDEWindowTable.Count(); nWin; )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.GetObject( --nWin );
        if ( pWin->IsDocument( _rDocument ) && _rDocument.isDocument() )
            pWin->SetReadOnly( _rDocument.isReadOnly() );
    }
}

void BasicIDEShell::StoreAllWindowData( sal_Bool bPersistent )
{
    for ( sal_uLong nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
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

    // da es nach Drucken etc. (DocInfo) modifiziert ist, hier resetten
    GetViewFrame()->GetObjectShell()->SetModified(sal_False);

    if ( StarBASIC::IsRunning() )
    {
        if( bUI )
        {
            String aErrorStr( IDEResId( RID_STR_CANNOTCLOSE ) );
            Window *pParent = &GetViewFrame()->GetWindow();
            InfoBox( pParent, aErrorStr ).Execute();
        }
        return sal_False;
    }
    else
    {
        sal_Bool bCanClose = sal_True;
        for ( sal_uLong nWin = 0; bCanClose && ( nWin < aIDEWindowTable.Count() ); nWin++ )
        {
            IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
            if ( !pWin->CanClose() )
            {
                if ( m_aCurLibName.Len() && ( pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName ) )
                    SetCurLib( ScriptDocument::getApplicationScriptDocument(), String(), false );
                SetCurWindow( pWin, sal_True );
                bCanClose = sal_False;
            }
        }

        if ( bCanClose )
            StoreAllWindowData( sal_False );    // Nicht auf Platte schreiben, das passiert am Ende automatisch

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
    // Adjust fliegt irgendwann raus...
    AdjustPosSizePixel( rPos, rSize );
}


IMPL_LINK_INLINE_START( BasicIDEShell, TabBarSplitHdl, TabBar *, pTBar )
{
    (void)pTBar;
    bTabBarSplitted = sal_True;
    ArrangeTabBar();

    return 0;
}
IMPL_LINK_INLINE_END( BasicIDEShell, TabBarSplitHdl, TabBar *, pTBar )



IMPL_LINK( BasicIDEShell, TabBarHdl, TabBar *, pCurTabBar )
{
    sal_uInt16 nCurId = pCurTabBar->GetCurPageId();
    IDEBaseWindow* pWin = aIDEWindowTable.Get( nCurId );
    DBG_ASSERT( pWin, "Eintrag in TabBar passt zu keinem Fenster!" );
    SetCurWindow( pWin );

    return 0;
}



sal_Bool BasicIDEShell::NextPage( sal_Bool bPrev )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nPos = pTabBar->GetPagePos( pTabBar->GetCurPageId() );

    if ( bPrev )
        --nPos;
    else
        ++nPos;

    if ( nPos < pTabBar->GetPageCount() )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.Get( pTabBar->GetPageId( nPos ) );
        SetCurWindow( pWin, sal_True );
        bRet = sal_True;
    }

    return bRet;
}



void BasicIDEShell::ArrangeTabBar()
{
    Size aSz( GetViewFrame()->GetWindow().GetOutputSizePixel() );
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



void BasicIDEShell::ShowObjectDialog( sal_Bool bShow, sal_Bool bCreateOrDestroy )
{
    if ( bShow )
    {
        if ( !pObjectCatalog && bCreateOrDestroy )
        {
            pObjectCatalog = new ObjectCatalog( &GetViewFrame()->GetWindow() );
            // Position wird in BasicIDEData gemerkt und vom Dlg eingestellt
            if ( pObjectCatalog )
            {
                pObjectCatalog->SetCancelHdl( LINK( this, BasicIDEShell, ObjectDialogCancelHdl ) );
                BasicEntryDescriptor aDesc;
                IDEBaseWindow* pCurWin_ = GetCurWindow();
                if ( pCurWin_ )
                    aDesc = pCurWin_->CreateEntryDescriptor();
                pObjectCatalog->SetCurrentEntry( aDesc );
            }
        }

        // Die allerletzten Aenderungen...
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
            // Wegen OS/2-Focus-Problem pObjectCatalog vorm delete auf NULL
            ObjectCatalog* pTemp = pObjectCatalog;
            pObjectCatalog = 0;
            delete pTemp;
        }
    }
}



void BasicIDEShell::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId&,
                                        const SfxHint& rHint, const TypeId& )
{
    if ( IDE_DLL()->GetShell() )
    {
        if ( rHint.IsA( TYPE( SfxSimpleHint ) ) )
        {
            switch ( ((SfxSimpleHint&)rHint).GetId() )
            {
                case SFX_HINT_DYING:
                {
                    EndListening( rBC, sal_True /* Alle abmelden */ );
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
                        // Nicht nur bei Error/Break oder explizitem anhalten,
                        // falls durch einen Programmierfehler das Update abgeschaltet ist.
                        BasicIDE::BasicStopped();
                        UpdateModulWindowLayout( true );    // Leer machen...
                        if( m_pCurLocalizationMgr )
                            m_pCurLocalizationMgr->handleBasicStopped();
                    }
                    else if( m_pCurLocalizationMgr )
                    {
                        m_pCurLocalizationMgr->handleBasicStarted();
                    }

                    IDEBaseWindow* pWin = aIDEWindowTable.First();
                    while ( pWin )
                    {
                        if ( nHintId == SBX_HINT_BASICSTART )
                            pWin->BasicStarted();
                        else
                            pWin->BasicStopped();
                        pWin = aIDEWindowTable.Next();
                    }
                }
            }
        }
    }
}



void BasicIDEShell::CheckWindows()
{
    sal_Bool bSetCurWindow = sal_False;
    for ( sal_uLong nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
        if ( pWin->GetStatus() & BASWIN_TOBEKILLED )
        {
            pWin->StoreData();
            if ( pWin == pCurWin )
                bSetCurWindow = sal_True;
            RemoveWindow( pWin, sal_True, sal_False );
            nWin--;
        }
    }
    if ( bSetCurWindow )
        SetCurWindow( FindApplicationWindow(), sal_True );
}



void BasicIDEShell::RemoveWindows( const ScriptDocument& rDocument, const String& rLibName, sal_Bool bDestroy )
{
    sal_Bool bChangeCurWindow = pCurWin ? sal_False : sal_True;
    for ( sal_uLong nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
    {
        IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
        if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName )
        {
            if ( pWin == pCurWin )
                bChangeCurWindow = sal_True;
            pWin->StoreData();
            RemoveWindow( pWin, bDestroy, sal_False );
            nWin--;
        }
    }
    if ( bChangeCurWindow )
        SetCurWindow( FindApplicationWindow(), sal_True );
}



void BasicIDEShell::UpdateWindows()
{
    // Alle Fenster, die nicht angezeigt werden duerfen, entfernen
    sal_Bool bChangeCurWindow = pCurWin ? sal_False : sal_True;
    if ( m_aCurLibName.Len() )
    {
        for ( sal_uLong nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
        {
            IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
            if ( !pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName )
            {
                if ( pWin == pCurWin )
                    bChangeCurWindow = sal_True;
                pWin->StoreData();
                // Die Abfrage auf RUNNING verhindert den Absturz, wenn in Reschedule.
                // Fenster bleibt erstmal stehen, spaeter sowieso mal umstellen,
                // dass Fenster nur als Hidden markiert werden und nicht
                // geloescht.
                if ( !(pWin->GetStatus() & ( BASWIN_TOBEKILLED | BASWIN_RUNNINGBASIC | BASWIN_SUSPENDED ) ) )
                {
                    RemoveWindow( pWin, sal_False, sal_False );
                    nWin--;
                }
            }
        }
    }

    if ( bCreatingWindow )
        return;

    IDEBaseWindow* pNextActiveWindow = 0;

    // Alle anzuzeigenden Fenster anzeigen
    ScriptDocuments aDocuments( ScriptDocument::getAllScriptDocuments( ScriptDocument::AllWithApplication ) );
    for (   ScriptDocuments::const_iterator doc = aDocuments.begin();
            doc != aDocuments.end();
            ++doc
        )
    {
        StartListening( *doc->getBasicManager(), sal_True /* Nur einmal anmelden */ );

        // libraries
        Sequence< ::rtl::OUString > aLibNames( doc->getLibraryNames() );
        sal_Int32 nLibCount = aLibNames.getLength();
        const ::rtl::OUString* pLibNames = aLibNames.getConstArray();

        for ( sal_Int32 i = 0 ; i < nLibCount ; i++ )
        {
            String aLibName = pLibNames[ i ];

            if ( !m_aCurLibName.Len() || ( *doc == m_aCurDocument && aLibName == m_aCurLibName ) )
            {
                // check, if library is password protected and not verified
                sal_Bool bProtected = sal_False;
                Reference< script::XLibraryContainer > xModLibContainer( doc->getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        bProtected = sal_True;
                    }
                }

                if ( !bProtected )
                {
                    LibInfoItem* pLibInfoItem = 0;
                    BasicIDEData* pData = IDE_DLL()->GetExtraData();
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
                                String aModName = pModNames[ j ];
                                ModulWindow* pWin = FindBasWin( *doc, aLibName, aModName, sal_False );
                                if ( !pWin )
                                    pWin = CreateBasWin( *doc, aLibName, aModName );
                                if ( !pNextActiveWindow && pLibInfoItem && pLibInfoItem->GetCurrentName() == aModName &&
                                        pLibInfoItem->GetCurrentType() == BASICIDE_TYPE_MODULE )
                                {
                                    pNextActiveWindow = (IDEBaseWindow*)pWin;
                                }
                            }
                        }
                        catch ( container::NoSuchElementException& )
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
                                String aDlgName = pDlgNames[ j ];
                                // this find only looks for non-suspended windows;
                                // suspended windows are handled in CreateDlgWin
                                DialogWindow* pWin = FindDlgWin( *doc, aLibName, aDlgName, sal_False );
                                if ( !pWin )
                                    pWin = CreateDlgWin( *doc, aLibName, aDlgName );
                                if ( !pNextActiveWindow && pLibInfoItem && pLibInfoItem->GetCurrentName() == aDlgName &&
                                        pLibInfoItem->GetCurrentType() == BASICIDE_TYPE_DIALOG )
                                {
                                    pNextActiveWindow = (IDEBaseWindow*)pWin;
                                }
                            }
                        }
                        catch ( container::NoSuchElementException& )
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
        SetCurWindow( pNextActiveWindow, sal_True );
    }
}

void BasicIDEShell::RemoveWindow( IDEBaseWindow* pWindow_, sal_Bool bDestroy, sal_Bool bAllowChangeCurWindow )
{
    DBG_ASSERT( pWindow_, "Kann keinen NULL-Pointer loeschen!" );
    sal_uLong nKey = aIDEWindowTable.GetKey( pWindow_ );
    pTabBar->RemovePage( (sal_uInt16)nKey );
    aIDEWindowTable.Remove( nKey );
    if ( pWindow_ == pCurWin )
    {
        if ( bAllowChangeCurWindow )
            SetCurWindow( FindApplicationWindow(), sal_True );
        else
            SetCurWindow( NULL, sal_False );
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
                if ( !pMod || ( pMod && ( pMod->GetName() != pWindow_->GetName() ) ) )
                    bStop = false;
            }
            if ( bStop )
            {
                StarBASIC::Stop();
                // Es kommt kein Notify...
                pWindow_->BasicStopped();
            }
            aIDEWindowTable.Insert( nKey, pWindow_ );   // wieder einhaegen
        }
    }
    else
    {
        pWindow_->Hide();
        pWindow_->AddStatus( BASWIN_SUSPENDED );
        pWindow_->Deactivating();
        aIDEWindowTable.Insert( nKey, pWindow_ );   // wieder einhaegen
    }

}



sal_uInt16 BasicIDEShell::InsertWindowInTable( IDEBaseWindow* pNewWin )
{
    // Eigentlich prueffen,
    nCurKey++;
    aIDEWindowTable.Insert( nCurKey, pNewWin );
    return nCurKey;
}



void BasicIDEShell::InvalidateBasicIDESlots()
{
    // Nur die, die eine optische Auswirkung haben...

    if ( IDE_DLL()->GetShell() )
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

void BasicIDEShell::EnableScrollbars( sal_Bool bEnable )
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

void BasicIDEShell::SetCurLib( const ScriptDocument& rDocument, String aLibName, bool bUpdateWindows, bool bCheck )
{
    if ( !bCheck || ( rDocument != m_aCurDocument || aLibName != m_aCurLibName ) )
    {
        ContainerListenerImpl* pListener = static_cast< ContainerListenerImpl* >( m_xLibListener.get() );

        if ( pListener )
            pListener->removeContainerListener( m_aCurDocument, m_aCurLibName );

        m_aCurDocument = rDocument;

        pListener->addContainerListener( m_aCurDocument, aLibName );

        m_aCurLibName = aLibName;

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

void BasicIDEShell::SetCurLibForLocalization( const ScriptDocument& rDocument, String aLibName )
{
    // Create LocalizationMgr
    delete m_pCurLocalizationMgr;
    Reference< resource::XStringResourceManager > xStringResourceManager;
    try
    {
        if( aLibName.Len() )
        {
            Reference< container::XNameContainer > xDialogLib( rDocument.getLibrary( E_DIALOGS, aLibName, sal_True ) );
            xStringResourceManager = LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
        }
    }
    catch ( container::NoSuchElementException& )
    {}
    m_pCurLocalizationMgr = new LocalizationMgr
        ( this, rDocument, aLibName, xStringResourceManager );

    m_pCurLocalizationMgr->handleTranslationbar();
}

void BasicIDEShell::ImplStartListening( StarBASIC* pBasic )
{
    StartListening( pBasic->GetBroadcaster(), sal_True /* Nur einmal anmelden */ );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
