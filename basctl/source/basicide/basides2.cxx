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

#include "docsignature.hxx"

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <ide_pch.hxx>
#include <basic/sbx.hxx>
#include "basicrenderable.hxx"

#include <com/sun/star/frame/XTitle.hpp>

#include <vcl/sound.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/signaturestate.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;

IMPL_LINK_INLINE_START( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )
{
    ShowObjectDialog( sal_False, sal_True );
    return 0;
}
IMPL_LINK_INLINE_END( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )

Reference< view::XRenderable > BasicIDEShell::GetRenderable()
{
    return Reference< view::XRenderable >( new basicide::BasicRenderable( pCurWin ) );
}

sal_Bool BasicIDEShell::HasSelection( sal_Bool /* bText */ ) const
{
    sal_Bool bSel = sal_False;
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
    {
        TextView* pEditView = ((ModulWindow*)pCurWin)->GetEditView();
        if ( pEditView && pEditView->HasSelection() )
            bSel = sal_True;
    }
    return bSel;
}

String BasicIDEShell::GetSelectionText( sal_Bool bWholeWord )
{
    String aText;
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
    {
        TextView* pEditView = ((ModulWindow*)pCurWin)->GetEditView();
        if ( pEditView )
        {
            if ( bWholeWord && !pEditView->HasSelection() )
            {
                aText = pEditView->GetTextEngine()->GetWord( pEditView->GetSelection().GetEnd() );
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

SfxPrinter* BasicIDEShell::GetPrinter( sal_Bool bCreate )
{
    if ( pCurWin ) // && pCurWin->ISA( ModulWindow ) )
    {
        BasicDocShell* pDocShell = (BasicDocShell*)GetViewFrame()->GetObjectShell();
        DBG_ASSERT( pDocShell, "DocShell ?!" );
        return pDocShell->GetPrinter( bCreate );
    }
    return 0;
}

sal_uInt16 BasicIDEShell::SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    (void)nDiffFlags;
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
        LibraryLocation eLocation = m_aCurDocument.getLibraryLocation( m_aCurLibName );
        aTitle = m_aCurDocument.getTitle( eLocation );
        aTitle += '.';
        aTitle += m_aCurLibName;
    }
    else
    {
        aTitle = String( IDEResId( RID_STR_ALL ) );
    }

    ::basctl::DocumentSignature aCurSignature( m_aCurDocument );
    if ( aCurSignature.getScriptingSignatureState() == SIGNATURESTATE_SIGNATURES_OK )
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
            pShell->SetModified( sal_False );
        }

        css::uno::Reference< css::frame::XController > xController = GetController ();
        css::uno::Reference< css::frame::XTitle >      xTitle      (xController, css::uno::UNO_QUERY);
        if (xTitle.is ())
            xTitle->setTitle (aTitle);
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

ModulWindow* BasicIDEShell::CreateBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
{
    bCreatingWindow = sal_True;

    sal_uLong nKey = 0;
    ModulWindow* pWin = 0;

    String aLibName( rLibName );
    String aModName( rModName );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    if ( !aModName.Len() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindBasWin( rDocument, aLibName, aModName, sal_False, sal_True );

    if ( !pWin )
    {
        ::rtl::OUString aModule;
        bool bSuccess = false;
        if ( rDocument.hasModule( aLibName, aModName ) )
            bSuccess = rDocument.getModule( aLibName, aModName, aModule );
        else
            bSuccess = rDocument.createModule( aLibName, aModName, sal_True, aModule );

        if ( bSuccess )
        {
            pWin = FindBasWin( rDocument, aLibName, aModName, sal_False, sal_True );
            if( !pWin )
            {
                // new module window
                pWin = new ModulWindow( pModulLayout, rDocument, aLibName, aModName, aModule );
                nKey = InsertWindowInTable( pWin );
            }
            else // we've gotten called recursively ( via listener from createModule above ), get outta here
                return pWin;
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
    if( nKey && xLib.is() && rDocument.isInVBAMode() )
    {
        // display a nice friendly name in the ObjectModule tab,
        // combining the objectname and module name, e.g. Sheet1 ( Financials )
        String sObjName;
        ModuleInfoHelper::getObjectName( xLib, rModName, sObjName );
        if( sObjName.Len() )
        {
            aModName.AppendAscii(" (").Append(sObjName).AppendAscii(")");
        }
    }
    pTabBar->InsertPage( (sal_uInt16)nKey, aModName );
    pTabBar->Sort();
    pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
    if ( !pCurWin )
        SetCurWindow( pWin, sal_False, sal_False );

    bCreatingWindow = sal_False;
    return pWin;
}

ModulWindow* BasicIDEShell::FindBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, sal_Bool bCreateIfNotExist, sal_Bool bFindSuspended )
{
    ModulWindow* pModWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pModWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( ModulWindow ) ) )
        {
            if ( !rLibName.Len() )  // nur irgendeins finden...
                pModWin = (ModulWindow*)pWin;
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rModName )
                pModWin = (ModulWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pModWin && bCreateIfNotExist )
        pModWin = CreateBasWin( rDocument, rLibName, rModName );

    return pModWin;
}

void BasicIDEShell::Move()
{
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
        ((ModulWindow*)pCurWin)->FrameWindowMoved();
}

void BasicIDEShell::ShowCursor( bool bOn )
{
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
        ((ModulWindow*)pCurWin)->ShowCursor( bOn );
}

// Hack for #101048
sal_Int32 getBasicIDEShellCount( void );

// Nur wenn Basicfenster oben:
void BasicIDEShell::ExecuteBasic( SfxRequest& rReq )
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) )
        return;

    pCurWin->ExecuteCommand( rReq );
    sal_Int32 nCount = getBasicIDEShellCount();
    if( nCount )
        CheckWindows();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
