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
#include "precompiled_basic.hxx"

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <basic/sbx.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <basic/sbmeth.hxx>
#include <svtools/stringtransfer.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

#include "basic.hrc"
#include "status.hxx"
#include "appbased.hxx"
#include "brkpnts.hxx"
#include <basic/testtool.hxx>      // defines for Syntaxhighlighting
#include "basrid.hxx"


TYPEINIT1(AppBasEd,AppEdit);
AppBasEd::AppBasEd( BasicFrame* pParent, SbModule* p )
: AppEdit( pParent )
, pBreakpoints( NULL )
{
    pBreakpoints = new BreakpointWindow( this );
    pBreakpoints->SetFont( ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->GetFont() );

    pBreakpoints->Show();


    ((TextEdit*)pDataEdit)->GetTextEditImp().pTextView->SetAutoIndentMode( sal_True );
    ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->SetMaxTextLen( STRING_MAXLEN );
//  ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->SetWordDelimiters( CUniString(" ,.;:(){}[]\"'+-*/<>^\\") );
    ((TextEdit*)pDataEdit)->GetTextEditImp().SyntaxHighlight( sal_True );
    ((TextEdit*)pDataEdit)->SaveAsUTF8( sal_True );

    String aEmpty;

    pMod = p;
    if( !pMod )
    {
        String aModName = *pNoName;
        aModName += String::CreateFromInt32( nCount );
        pMod = pFrame->Basic().MakeModule( aModName, aEmpty );
    }
    bCompiled = pMod->IsCompiled();

    pBreakpoints->SetModule( pMod );

    // Define icon:
//  pIcon = new Icon( ResId( RID_WORKICON ) );
//  if( pIcon ) SetIcon( *pIcon );

    SetText( pMod->GetName() );
    pDataEdit->SetText( pMod->GetSource() );

    // If a module was given, load the source from harddisk
    if ( p )
        LoadSource();

    // Dispatch event AFTER loading the sourcecode
    ((TextEdit*)pDataEdit)->SetBreakpointWindow( pBreakpoints );

    // Touch compile flag
    pDataEdit->SetModifyHdl( LINK( this, AppBasEd, EditChange ) );

}

AppBasEd::~AppBasEd()
{
    pBreakpoints->SaveBreakpoints( GetText() );
    delete pBreakpoints;
    pMod->SetName( CUniString("--").Append( pMod->GetName() ) );
}

void AppBasEd::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxSimpleHint* p = PTR_CAST(SfxSimpleHint,&rHint);
    if( p )
    {
        sal_uIntPtr nHintId = p->GetId();
        if( nHintId == SBX_HINT_LANGUAGE_EXTENSION_LOADED )
        {
            ((TextEdit*)pDataEdit)->GetTextEditImp().InvalidateSyntaxHighlight();
        }
    }
}

FileType AppBasEd::GetFileType()
{
    return FT_BASIC_SOURCE;
}

IMPL_LINK_INLINE_START( AppBasEd, EditChange, void *, p )
{
    (void) p; /* avoid warning about unused parameter */
    bCompiled = sal_False;
    return sal_True;
}
IMPL_LINK_INLINE_END( AppBasEd, EditChange, void *, p )

// Set up the menu
long AppBasEd::InitMenu( Menu* pMenu )
{
    AppEdit::InitMenu (pMenu );
    sal_Bool bRunning = pFrame->Basic().IsRunning();
    pMenu->EnableItem( RID_RUNCOMPILE,  !bCompiled && !bRunning );
    return sal_True;
}

long AppBasEd::DeInitMenu( Menu* pMenu )
{
    AppEdit::DeInitMenu (pMenu );
    pMenu->EnableItem( RID_RUNCOMPILE );
    return sal_True;
}

// Menu Handler
void AppBasEd::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() ) {
        case RID_TOGLEBRKPNT:
            ((TextEdit*)pDataEdit)->GetBreakpointWindow()->ToggleBreakpoint( pDataEdit->GetLineNr() );
            break;
        default:
            AppEdit::Command( rCEvt );
    }
}

void AppBasEd::Resize()
{
  if( pDataEdit )
  {
    AppEdit::Resize();

    // Insert breakpoint window
    Size aEditSize = pDataEdit->GetSizePixel();
    Point aEditPos = pDataEdit->GetPosPixel();

    pBreakpoints->SetPosPixel( aEditPos );

    aEditPos.X() += BREAKPOINTSWIDTH;
    pDataEdit->SetPosPixel( aEditPos );
    aEditSize.Width() -= BREAKPOINTSWIDTH;
    pDataEdit->SetSizePixel( aEditSize );

    aEditSize.Width() = BREAKPOINTSWIDTH;
    pBreakpoints->SetSizePixel( aEditSize );
  }
}

void AppBasEd::PostLoad()
{
    pMod->SetName( GetText() );
    pMod->Clear();
    pMod->SetSource( pDataEdit->GetText() );
    bCompiled = sal_False;  // because the code might have changed in the meantime
    AppEdit::PostLoad();

    pBreakpoints->LoadBreakpoints( GetText() );
}

sal_uInt16 AppBasEd::ImplSave()
{
    pBreakpoints->SaveBreakpoints( GetText() );
    return AppEdit::ImplSave();
}

void AppBasEd::Reload()
{
    TextSelection aSelMemo = pDataEdit->GetSelection();
    LoadSource();
    pDataEdit->SetSelection( aSelMemo );
}

// Reload source code file after change
void AppBasEd::LoadSource()
{
    sal_Bool bErr;

//  if( pDataEdit->GetText().Len() != 0 ) return;
    String aName = pMod->GetName();
    bErr = !pDataEdit->Load( aName );
    pBreakpoints->LoadBreakpoints( GetText() );
    if( bErr )
        ErrorBox( this, SttResId( IDS_READERROR ) ).Execute();
    else
        UpdateFileInfo( HAS_BEEN_LOADED );
    bCompiled = sal_False;  // because the code might have changed in the meantime
}

// Save as (new name)
void AppBasEd::PostSaveAs()
{
    pMod->SetName( GetText() );
    AppEdit::PostSaveAs();
}

// Compile
sal_Bool AppBasEd::Compile()
{
    if( !pDataEdit->HasText() || bCompiled )
        return sal_True;
    pMod->SetSource( pDataEdit->GetText() );
    sal_Bool bRes = sal_False;
    if( pFrame->Basic().Compile( pMod ) )
    {
        bRes = sal_True;
        if( pFrame->bDisas )
            Disassemble();
        TextSelection aSel( pDataEdit->GetSelection() );
        String aString;
        pFrame->pStatus->Message( aString );
        if( aSel.HasRange() )
          aSel.GetStart() = aSel.GetEnd(), pDataEdit->SetSelection( aSel );

        pBreakpoints->SetBPsInModule();
    }
    else
    {
        BasicError* pErr = pFrame->Basic().aErrors.First();
        if( pErr ) pErr->Show();
    }
    return bCompiled = bRes;
}

void AppBasEd::Disassemble()
{
    String aText;
    if( pFrame->Basic().Disassemble( pMod, aText ) )
        ::svt::OStringTransfer::CopyString( aText, this );
}

void AppBasEd::Run()
{
    pFrame->Basic().Reset();
    SbxArray* pAllModules = pFrame->Basic().GetModules();
    for (sal_uInt16 i = 0; i < pAllModules->Count(); i++)
    {
        if ( (pAllModules->Get(i)->GetName()).Copy(0,2).CompareToAscii( "--" ) == COMPARE_EQUAL )
        {
            // Little hack to get around basic
            SbxVariableRef pRMod = pAllModules->Get(i);
            pFrame->Basic().Remove(pRMod);
            i--;
        }
    }

    SbMethod* pMain = (SbMethod*) pMod->Find( CUniString("Main"), SbxCLASS_METHOD );
    if( pMain )
    {
        pMain->SetDebugFlags( pFrame->nFlags );
        // Triggers a call!
        pFrame->SetAppMode( String( SttResId( IDS_APPMODE_RUN ) ) );
        pMain->Run();
        if (aBasicApp.pFrame)
        {
            BasicError* pErr = aBasicApp.pFrame->Basic().aErrors.First();
            if( pErr )
                pErr->Show();
            aBasicApp.pFrame->SetAppMode( String() );
        }
        pMain->SetDebugFlags( 0 );
    }
}


