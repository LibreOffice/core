/*************************************************************************
 *
 *  $RCSfile: basides2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:36 $
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
#define __PRIVATE 1

#include <vcl/rcid.h>
#include <vcl/sound.hxx>
#include <basidesh.hxx>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>
#include <sfx2/sfxdefs.hxx>

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


void BasicIDEShell::UpdateModulWindowLayout()
{
    if ( pModulLayout )
    {
        pModulLayout->GetStackWindow().UpdateCalls();
        pModulLayout->GetWatchWindow().UpdateWatches();
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

    SbModule* pModule = pBasic->FindModule( aModName );
    if ( !pModule )
        pModule = BasicIDE::CreateModule( pBasic, aModName, TRUE );
    else    // Vielleicht gibt es ein suspendiertes?
        pWin = FindBasWin( pBasic, aModName, FALSE, TRUE );

    if ( !pWin )
    {
        pWin = new ModulWindow( pModulLayout, pBasic, pModule );
        nKey = InsertWindowInTable( pWin );
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
    pTabBar->InsertPage( (USHORT)nKey, pModule->GetName() );
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
            String aMod( ((ModulWindow*)pWin)->GetModuleName() );
            if ( !pBasic )  // nur irgendeins finden...
                pModWin = (ModulWindow*)pWin;
            else if (  ( pWin->GetBasic() == pBasic ) &&
                    ( ( rModName.Len() == 0 ) || ( aMod == rModName ) ) )
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

// Nur wenn Basicfenster oben:
void __EXPORT BasicIDEShell::ExecuteBasic( SfxRequest& rReq )
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) )
        return;

    pCurWin->ExecuteCommand( rReq );
    CheckWindows();
}

