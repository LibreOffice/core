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

#include <docsignature.hxx>

#include "basicrenderable.hxx"

#include <com/sun/star/frame/XTitle.hpp>

#include <strings.hrc>
#include "baside2.hxx"
#include "basdoc.hxx"
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/signaturestate.hxx>
#include <com/sun/star/container/XNamed.hpp>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

Reference< view::XRenderable > Shell::GetRenderable()
{
    return Reference<view::XRenderable>( new Renderable(pCurWin) );
}

bool Shell::HasSelection( bool /* bText */ ) const
{
    if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin.get()))
    {
        TextView* pEditView = pMCurWin->GetEditView();
        if ( pEditView && pEditView->HasSelection() )
            return true;
    }
    return false;
}

OUString Shell::GetSelectionText( bool bWholeWord )
{
    OUString aText;
    if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin.get()))
    {
        if (TextView* pEditView = pMCurWin->GetEditView())
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

SfxPrinter* Shell::GetPrinter( bool bCreate )
{
    if ( pCurWin )
    {
        DocShell* pDocShell = static_cast<DocShell*>(GetViewFrame()->GetObjectShell());
        assert(pDocShell && "DocShell ?!");
        return pDocShell->GetPrinter( bCreate );
    }
    return nullptr;
}

sal_uInt16 Shell::SetPrinter( SfxPrinter *pNewPrinter, SfxPrinterChangeFlags )
{
    DocShell* pDocShell = static_cast<DocShell*>(GetViewFrame()->GetObjectShell());
    assert(pDocShell && "DocShell ?!");
    pDocShell->SetPrinter( pNewPrinter );
    return 0;
}

void Shell::SetMDITitle()
{
    OUString aTitle;
    if ( !m_aCurLibName.isEmpty() )
    {
        LibraryLocation eLocation = m_aCurDocument.getLibraryLocation( m_aCurLibName );
        aTitle = m_aCurDocument.getTitle(eLocation) + "." + m_aCurLibName ;
    }
    else
        aTitle = IDEResId(RID_STR_ALL) ;

    DocumentSignature aCurSignature( m_aCurDocument );
    if ( aCurSignature.getScriptingSignatureState() == SignatureState::OK )
    {
        aTitle += " " + IDEResId(RID_STR_SIGNED) + " ";
    }

    SfxViewFrame* pViewFrame = GetViewFrame();
    if ( pViewFrame )
    {
        SfxObjectShell* pShell = pViewFrame->GetObjectShell();
        if ( pShell && pShell->GetTitle( SFX_TITLE_CAPTION ) != aTitle )
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

VclPtr<ModulWindow> Shell::CreateBasWin( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName )
{
    bCreatingWindow = true;

    sal_uLong nKey = 0;
    VclPtr<ModulWindow> pWin;

    OUString aLibName( rLibName );
    OUString aModName( rModName );

    if ( aLibName.isEmpty() )
        aLibName = "Standard" ;

    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    if ( aModName.isEmpty() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    // maybe there's an suspended one?
    pWin = FindBasWin( rDocument, aLibName, aModName, false, true );

    if ( !pWin )
    {
        OUString aModule;
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
                if (!pModulLayout)
                    pModulLayout.reset(VclPtr<ModulWindowLayout>::Create(&GetViewFrame()->GetWindow(), *aObjectCatalog));
                pWin = VclPtr<ModulWindow>::Create(pModulLayout.get(), rDocument, aLibName, aModName, aModule);
                nKey = InsertWindowInTable( pWin );
            }
            else // we've gotten called recursively ( via listener from createModule above ), get outta here
                return pWin;
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        nKey = GetWindowId( pWin );
        DBG_ASSERT( nKey, "CreateBasWin: No Key - Window not found!" );
    }
    if( nKey && xLib.is() && rDocument.isInVBAMode() )
    {
        // display a nice friendly name in the ObjectModule tab,
        // combining the objectname and module name, e.g. Sheet1 ( Financials )
        OUString sObjName;
        ModuleInfoHelper::getObjectName( xLib, rModName, sObjName );
        if( !sObjName.isEmpty() )
        {
            aModName += " (" + sObjName + ")";
        }
    }
    pTabBar->InsertPage( static_cast<sal_uInt16>(nKey), aModName );
    pTabBar->Sort();
    if(pWin)
    {
        pWin->GrabScrollBars( aHScrollBar.get(), aVScrollBar.get() );
        if ( !pCurWin )
            SetCurWindow( pWin, false, false );
    }
    bCreatingWindow = false;
    return pWin;
}

VclPtr<ModulWindow> Shell::FindBasWin (
    ScriptDocument const& rDocument,
    OUString const& rLibName, OUString const& rName,
    bool bCreateIfNotExist, bool bFindSuspended
)
{
    if (VclPtr<BaseWindow> pWin = FindWindow(rDocument, rLibName, rName, TYPE_MODULE, bFindSuspended))
        return VclPtr<ModulWindow>(static_cast<ModulWindow*>(pWin.get()));
    return bCreateIfNotExist ? CreateBasWin(rDocument, rLibName, rName) : nullptr;
}

void Shell::Move()
{
}

void Shell::ShowCursor( bool bOn )
{
    if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin.get()))
        pMCurWin->ShowCursor(bOn);
}

// only if basic window above:
void Shell::ExecuteBasic( SfxRequest& rReq )
{
    if (dynamic_cast<ModulWindow*>(pCurWin.get()))
    {
        pCurWin->ExecuteCommand( rReq );
        if (nShellCount)
            CheckWindows();
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
