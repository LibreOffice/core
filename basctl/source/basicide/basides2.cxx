/*************************************************************************
 *
 *  $RCSfile: basides2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:11:40 $
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

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <ide_pch.hxx>

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#pragma hdrstop
#define _SOLAR__PRIVATE 1

#include <vcl/sound.hxx>
#include <basidesh.hxx>
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
    String aTitle = BasicIDE::GetStdTitle();
    if ( pCurWin )
    {
        StarBASIC* pLib = pCurWin->GetBasic();
        DBG_ASSERT( pLib, "Lib?!" );
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pLib );
        if ( pBasMgr )
        {
            aTitle += String( RTL_CONSTASCII_USTRINGPARAM( " - " ) );
            aTitle += BasicIDE::FindTitle( pBasMgr, SFX_TITLE_FILENAME );
            aTitle += '.';
            aTitle += pLib->GetName();
        }
    }
    // Wenn DocShell::SetTitle, erfolgt beim Schliessen Abfrage, ob Speichern!
    GetViewFrame()->GetObjectShell()->SetTitle( aTitle );
    GetViewFrame()->GetObjectShell()->SetModified( FALSE );
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

ModulWindow* BasicIDEShell::CreateBasWin( StarBASIC* pBasic, String aModName )
{
    bCreatingWindow = TRUE;

    ULONG nKey = 0;
    ModulWindow* pWin = 0;

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindBasWin( pBasic, aModName, FALSE, TRUE );

    if ( !pWin )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            String aLibName = pBasic->GetName();

            if ( aModName.Len() == 0 )
                aModName = BasicIDE::CreateModuleName( pShell, aLibName );

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
                pWin = new ModulWindow( pModulLayout, pBasic, pShell, aLibName, aModName, aModule );
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

ModulWindow* BasicIDEShell::FindBasWin( StarBASIC* pBasic, const String& rModName, BOOL bCreateIfNotExist, BOOL bFindSuspended )
{
    ModulWindow* pModWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pModWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( ModulWindow ) ) )
        {
            String aMod( pWin->GetName() );
            if ( !pBasic )  // nur irgendeins finden...
                pModWin = (ModulWindow*)pWin;
            else if ( ( pWin->GetBasic() == pBasic ) && ( aMod == rModName ) )
            {
                pModWin = (ModulWindow*)pWin;
            }
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pModWin && bCreateIfNotExist )
        pModWin = CreateBasWin( pBasic, rModName );

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

