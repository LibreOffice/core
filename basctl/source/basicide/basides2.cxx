/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basides2.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:58:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <ide_pch.hxx>

#ifndef _SBXCLASS_HXX //autogen
#include <basic/sbx.hxx>
#endif

#pragma hdrstop
#define _SOLAR__PRIVATE 1

#include <vcl/sound.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>
#include <sfx2/sfxdefs.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


IMPL_LINK_INLINE_START( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )
{
    ShowObjectDialog( FALSE, TRUE );
    return 0;
}
IMPL_LINK_INLINE_END( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )

/*
IMPL_LINK( BasicIDEShell, ObjectDialogInsertHdl, ObjectCatalog *, pObjCat )
{
    if ( !pCurWin )
        return 0;

    if ( pCurWin->IsA( TYPE( ModulWindow ) ) )
    {
        ModulWindow* pEditWin = (ModulWindow*)pCurWin;
        pEditWin->InsertFromObjectCatalog( pObjCat );
    }
    else
        Sound::Beep();

    return 0;
}
*/

USHORT __EXPORT BasicIDEShell::Print( SfxProgress &rProgress, PrintDialog *pPrintDialog )
{
    if ( pCurWin )
    {
        SfxPrinter* pPrinter = GetPrinter( TRUE );
        if ( pPrinter )
        {
            SfxViewShell::Print( rProgress, pPrintDialog );
            pCurWin->PrintData( pPrinter );
        }
    }
    return 0;
}

BOOL BasicIDEShell::HasSelection( BOOL /* bText */ ) const
{
    BOOL bSel = FALSE;
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
    {
        TextView* pEditView = ((ModulWindow*)pCurWin)->GetEditView();
        if ( pEditView && pEditView->HasSelection() )
            bSel = TRUE;
    }
    return bSel;
}

String BasicIDEShell::GetSelectionText( BOOL bWholeWord )
{
    String aText;
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
    {
        TextView* pEditView = ((ModulWindow*)pCurWin)->GetEditView();
        if ( pEditView )
        {
            if ( bWholeWord && !pEditView->HasSelection() )
            {
                // String aStrCurrentDelimiters = pEngine->GetWordDelimiters();
                // pEngine->SetWordDelimiters( " .,;\"'" );
                aText = pEditView->GetTextEngine()->GetWord( pEditView->GetSelection().GetEnd() );
                // pEngine->SetWordDelimiters( aStrCurrentDelimiters );
            }
            else
            {
                TextSelection aSel = pEditView->GetSelection();
                if ( !bWholeWord || ( aSel.GetStart().GetPara() == aSel.GetEnd().GetPara() ) )
                    aText = pEditView->GetSelected();
            }
        }
    }
    return aText;
}

SfxPrinter* __EXPORT BasicIDEShell::GetPrinter( BOOL bCreate )
{
    if ( pCurWin ) // && pCurWin->ISA( ModulWindow ) )
    {
        BasicDocShell* pDocShell = (BasicDocShell*)GetViewFrame()->GetObjectShell();
        DBG_ASSERT( pDocShell, "DocShell ?!" );
        return pDocShell->GetPrinter( bCreate );
    }
    return 0;
}

USHORT __EXPORT BasicIDEShell::SetPrinter( SfxPrinter *pNewPrinter, USHORT nDiffFlags )
{
    BasicDocShell* pDocShell = (BasicDocShell*)GetViewFrame()->GetObjectShell();
    DBG_ASSERT( pDocShell, "DocShell ?!" );
    pDocShell->SetPrinter( pNewPrinter );
    return 0;
}

void BasicIDEShell::SetMDITitle()
{
    String aTitle;

    if ( m_aCurLibName.Len() )
    {
        LibraryLocation eLocation = BasicIDE::GetLibraryLocation( m_pCurShell, m_aCurLibName );
        aTitle = BasicIDE::GetTitle( m_pCurShell, eLocation, SFX_TITLE_CAPTION );
        aTitle += '.';
        aTitle += m_aCurLibName;
    }
    else
    {
        aTitle = String( IDEResId( RID_STR_ALL ) );
    }

    if ( m_pCurShell &&
         m_pCurShell->GetScriptingSignatureState() == SIGNATURESTATE_SIGNATURES_OK )
    {
        aTitle += String::CreateFromAscii( " " );
        aTitle += String( IDEResId( RID_STR_SIGNED ) );
        aTitle += String::CreateFromAscii( " " );
    }

    SfxViewFrame* pViewFrame = GetViewFrame();
    if ( pViewFrame )
    {
        SfxObjectShell* pShell = pViewFrame->GetObjectShell();
        if ( pShell && aTitle != pShell->GetTitle( SFX_TITLE_CAPTION ) )
        {
            pShell->SetTitle( aTitle );
            pShell->SetModified( FALSE );
        }
    }
}

void BasicIDEShell::DestroyModulWindowLayout()
{
    delete pModulLayout;
    pModulLayout = 0;
}


void BasicIDEShell::UpdateModulWindowLayout( bool bBasicStopped )
{
    if ( pModulLayout )
    {
        pModulLayout->GetStackWindow().UpdateCalls();
        pModulLayout->GetWatchWindow().UpdateWatches( bBasicStopped );
    }
}

void BasicIDEShell::CreateModulWindowLayout()
{
    pModulLayout = new ModulWindowLayout( &GetViewFrame()->GetWindow() );
}

ModulWindow* BasicIDEShell::CreateBasWin( SfxObjectShell* pShell, const String& rLibName, const String& rModName )
{
    bCreatingWindow = TRUE;

    ULONG nKey = 0;
    ModulWindow* pWin = 0;

    String aLibName( rLibName );
    String aModName( rModName );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    if ( !BasicIDE::HasModuleLibrary( pShell, aLibName ) )
        BasicIDE::CreateModuleLibrary( pShell, aLibName );

    if ( !aModName.Len() )
        aModName = BasicIDE::CreateModuleName( pShell, aLibName );

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindBasWin( pShell, aLibName, aModName, FALSE, TRUE );

    if ( !pWin )
    {
        try
        {
            ::rtl::OUString aModule;
            if ( BasicIDE::HasModule( pShell, aLibName, aModName ) )
            {
                // get module
                aModule = BasicIDE::GetModule( pShell, aLibName, aModName );
            }
            else
            {
                // create module
                aModule = BasicIDE::CreateModule( pShell, aLibName, aModName, TRUE );
            }

            // new module window
            pWin = new ModulWindow( pModulLayout, pShell, aLibName, aModName, aModule );
            nKey = InsertWindowInTable( pWin );
        }
        catch ( container::ElementExistException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        IDEBaseWindow* pTmp = aIDEWindowTable.First();
        while ( pTmp && !nKey )
        {
            if ( pTmp == pWin )
                nKey = aIDEWindowTable.GetCurKey();
            pTmp = aIDEWindowTable.Next();
        }
        DBG_ASSERT( nKey, "CreateBasWin: Kein Key- Fenster nicht gefunden!" );
    }
    pTabBar->InsertPage( (USHORT)nKey, aModName );
    pTabBar->Sort();
    pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
    if ( !pCurWin )
        SetCurWindow( pWin, FALSE, FALSE );

    bCreatingWindow = FALSE;
    return pWin;
}

ModulWindow* BasicIDEShell::FindBasWin( SfxObjectShell* pShell, const String& rLibName, const String& rModName, BOOL bCreateIfNotExist, BOOL bFindSuspended )
{
    ModulWindow* pModWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pModWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( ModulWindow ) ) )
        {
            if ( !rLibName.Len() )  // nur irgendeins finden...
                pModWin = (ModulWindow*)pWin;
            else if ( pWin->GetShell() == pShell && pWin->GetLibName() == rLibName && pWin->GetName() == rModName )
                pModWin = (ModulWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pModWin && bCreateIfNotExist )
        pModWin = CreateBasWin( pShell, rLibName, rModName );

    return pModWin;
}

void __EXPORT BasicIDEShell::Move()
{
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
        ((ModulWindow*)pCurWin)->FrameWindowMoved();
}

void __EXPORT BasicIDEShell::ShowCursor( FASTBOOL bOn )
{
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
        ((ModulWindow*)pCurWin)->ShowCursor( (BOOL)bOn );
}

// Hack for #101048
sal_Int32 getBasicIDEShellCount( void );

// Nur wenn Basicfenster oben:
void __EXPORT BasicIDEShell::ExecuteBasic( SfxRequest& rReq )
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) )
        return;

    pCurWin->ExecuteCommand( rReq );
    sal_Int32 nCount = getBasicIDEShellCount();
    if( nCount )
        CheckWindows();
}

