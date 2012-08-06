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

#include "docsignature.hxx"

#include <basic/sbx.hxx>
#include "basicrenderable.hxx"

#include <com/sun/star/frame/XTitle.hpp>

#include <basidesh.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <vcl/xtextedt.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/childwin.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/signaturestate.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;

Reference< view::XRenderable > BasicIDEShell::GetRenderable()
{
    return Reference< view::XRenderable >( new basicide::BasicRenderable( pCurWin ) );
}

sal_Bool BasicIDEShell::HasSelection( sal_Bool /* bText */ ) const
{
    bool bSel = false;
    if ( pCurWin && pCurWin->ISA( ModulWindow ) )
    {
        TextView* pEditView = ((ModulWindow*)pCurWin)->GetEditView();
        if ( pEditView && pEditView->HasSelection() )
            bSel = true;
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
    ::rtl::OUStringBuffer aTitleBuf;
    if ( !m_aCurLibName.isEmpty() )
    {
        LibraryLocation eLocation = m_aCurDocument.getLibraryLocation( m_aCurLibName );
        aTitleBuf.append(m_aCurDocument.getTitle(eLocation));
        aTitleBuf.append('.');
        aTitleBuf.append(m_aCurLibName);
    }
    else
        aTitleBuf.append(IDE_RESSTR(RID_STR_ALL));

    ::basctl::DocumentSignature aCurSignature( m_aCurDocument );
    if ( aCurSignature.getScriptingSignatureState() == SIGNATURESTATE_SIGNATURES_OK )
    {
        aTitleBuf.append(' ');
        aTitleBuf.append(IDE_RESSTR(RID_STR_SIGNED));
        aTitleBuf.append(' ');
    }
    ::rtl::OUString aTitle(aTitleBuf.makeStringAndClear());

    SfxViewFrame* pViewFrame = GetViewFrame();
    if ( pViewFrame )
    {
        SfxObjectShell* pShell = pViewFrame->GetObjectShell();
        if ( pShell && !pShell->GetTitle( SFX_TITLE_CAPTION ).Equals(aTitle) )
        {
            pShell->SetTitle( aTitle );
            pShell->SetModified(false);
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

ModulWindow* BasicIDEShell::CreateBasWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName )
{
    bCreatingWindow = true;

    sal_uLong nKey = 0;
    ModulWindow* pWin = 0;

    ::rtl::OUString aLibName( rLibName );
    ::rtl::OUString aModName( rModName );

    if ( aLibName.isEmpty() )
        aLibName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Standard"));

    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    if ( aModName.isEmpty() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    // maybe there's an suspended one?
    pWin = FindBasWin( rDocument, aLibName, aModName, false, true );

    if ( !pWin )
    {
        ::rtl::OUString aModule;
        bool bSuccess = false;
        if ( rDocument.hasModule( aLibName, aModName ) )
            bSuccess = rDocument.getModule( aLibName, aModName, aModule );
        else
            bSuccess = rDocument.createModule( aLibName, aModName, true, aModule );

        if ( bSuccess )
        {
            pWin = FindBasWin( rDocument, aLibName, aModName, false, true );
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
        nKey = GetIDEWindowId( pWin );
        DBG_ASSERT( nKey, "CreateBasWin: Kein Key- Fenster nicht gefunden!" );
    }
    if( nKey && xLib.is() && rDocument.isInVBAMode() )
    {
        // display a nice friendly name in the ObjectModule tab,
        // combining the objectname and module name, e.g. Sheet1 ( Financials )
        ::rtl::OUString sObjName;
        ModuleInfoHelper::getObjectName( xLib, rModName, sObjName );
        if( !sObjName.isEmpty() )
        {
            ::rtl::OUStringBuffer aModNameBuf(aModName);
            aModNameBuf.append(' ');
            aModNameBuf.append('(');
            aModNameBuf.append(sObjName);
            aModNameBuf.append(')');
            aModName = aModNameBuf.makeStringAndClear();
        }
    }
    pTabBar->InsertPage( (sal_uInt16)nKey, aModName );
    pTabBar->Sort();
    pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
    if ( !pCurWin )
        SetCurWindow( pWin, false, false );

    bCreatingWindow = false;
    return pWin;
}

ModulWindow* BasicIDEShell::FindBasWin( const ScriptDocument& rDocument, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModName, bool bCreateIfNotExist, bool bFindSuspended )
{
    ModulWindow* pModWin = 0;
    for( IDEWindowTable::const_iterator it = aIDEWindowTable.begin();
         it != aIDEWindowTable.end(); ++it )
    {
        IDEBaseWindow* pWin = it->second;
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( ModulWindow ) ) )
        {
            if ( rLibName.isEmpty() )
            {
                pModWin = (ModulWindow*)pWin;
                break;
            }
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rModName )
            {
                pModWin = (ModulWindow*)pWin;
                break;
            }
        }
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

// only if basic window above:
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
