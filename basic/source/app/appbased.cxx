/*************************************************************************
 *
 *  $RCSfile: appbased.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:08 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _TEXTENG_HXX //autogen
#include <svtools/texteng.hxx>
#endif
#ifndef _TEXTVIEW_HXX //autogen
#include <svtools/textview.hxx>
#endif
#ifndef _SB_SBMETH_HXX //autogen
#include <sbmeth.hxx>
#endif

#ifndef _CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

#include "basic.hrc"
#include "status.hxx"
#include "appbased.hxx"
#include "brkpnts.hxx"
#include "testtool.hxx"     // defines für das Syntaxhighlighting


TYPEINIT1(AppBasEd,AppEdit);
AppBasEd::AppBasEd( BasicFrame* pParent, SbModule* p )
: AppEdit( pParent )
, pBreakpoints( NULL )
{
    pBreakpoints = new BreakpointWindow( this );
    pBreakpoints->SetFont( ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->GetFont() );

    pBreakpoints->Show();


    ((TextEdit*)pDataEdit)->GetTextEditImp().pTextView->SetAutoIndentMode( TRUE );
    ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->SetMaxTextLen( STRING_MAXLEN );
    ((TextEdit*)pDataEdit)->GetTextEditImp().pTextEngine->SetWordDelimiters( CUniString(" ,.;:(){}[]\"'+-*/<>^\\") );
    ((TextEdit*)pDataEdit)->GetTextEditImp().SyntaxHighlight( TRUE );
    ((TextEdit*)pDataEdit)->SaveAsUTF8( TRUE );

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

    // Icon definieren:
//  pIcon = new Icon( ResId( RID_WORKICON ) );
//  if( pIcon ) SetIcon( *pIcon );

    SetText( pMod->GetName() );
    pDataEdit->SetText( pMod->GetSource() );

    // Wurde ein Modul übergeben, dann den Quelltext von Platte laden
    if ( p )
        LoadSource();

    // Erst nach Laden des Quelltextes die Events weiterleiten
    ((TextEdit*)pDataEdit)->SetBreakpointWindow( pBreakpoints );

    // Compiled-Flag pflegen:
    pDataEdit->SetModifyHdl( LINK( this, AppBasEd, EditChange ) );

}

AppBasEd::~AppBasEd()
{
    pBreakpoints->SaveBreakpoints( GetText() );
    delete pBreakpoints;
    pMod->SetName( CUniString("--").Append( pMod->GetName() ) );
}

void AppBasEd::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
                            const SfxHint& rHint, const TypeId& )
{
    const SfxSimpleHint* p = PTR_CAST(SfxSimpleHint,&rHint);
    if( p )
    {
        ULONG nHintId = p->GetId();
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
    bCompiled = FALSE;
    return TRUE;
}
IMPL_LINK_INLINE_END( AppBasEd, EditChange, void *, p )

// Set up the menu
long AppBasEd::InitMenu( Menu* pMenu )
{
    AppEdit::InitMenu (pMenu );
    BOOL bRunning = pFrame->Basic().IsRunning();
    pMenu->EnableItem( RID_RUNCOMPILE,  !bCompiled && !bRunning );
    return TRUE;
}

long AppBasEd::DeInitMenu( Menu* pMenu )
{
    AppEdit::DeInitMenu (pMenu );
    pMenu->EnableItem( RID_RUNCOMPILE );
    return TRUE;
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

// Sourcecode-Datei laden

void AppBasEd::Resize()
{
    if( pDataEdit ) {
        AppEdit::Resize();

        // Breakpoint window einfügen
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
    AppEdit::PostLoad();

    pBreakpoints->LoadBreakpoints( GetText() );
}

USHORT AppBasEd::ImplSave()
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

// Sourcecode-Datei nach Änderung auf Platte neu laden
void AppBasEd::LoadSource()
{
    BOOL bErr;

//  if( pDataEdit->GetText().Len() != 0 ) return;
    String aName = pMod->GetName();
    bErr = !pDataEdit->Load( aName );
    pBreakpoints->LoadBreakpoints( GetText() );
    if( bErr )
        ErrorBox( this, ResId( IDS_READERROR ) ).Execute();
    else
        UpdateFileInfo( HAS_BEEN_LOADED );
}

// mit neuem Namen speichern
void AppBasEd::PostSaveAs()
{
    pMod->SetName( GetText() );
    AppEdit::PostSaveAs();
}

// Compilieren

BOOL AppBasEd::Compile()
{
    if( !pDataEdit->HasText() || bCompiled )
        return TRUE;
    pMod->SetSource( pDataEdit->GetText() );
    BOOL bRes = FALSE;
    if( pFrame->Basic().Compile( pMod ) )
    {
        bRes = TRUE;
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
        Clipboard::CopyString( aText );
}

void AppBasEd::Run()
{
    SbxArray* pAllModules = pFrame->Basic().GetModules();
    for (USHORT i = 0; i < pAllModules->Count(); i++)
    {
        if ( (pAllModules->Get(i)->GetName()).Copy(0,2).CompareToAscii( "--" ) == COMPARE_EQUAL )
        {
            SbxVariableRef pMod = pAllModules->Get(i);  // Kleiner Hack um ums basic rumzukommen. Sollte demnächst wieder dirkt gehen.
            pFrame->Basic().Remove(pMod);
            i--;
        }
    }

    SbMethod* pMain = (SbMethod*) pMod->Find( CUniString("Main"), SbxCLASS_METHOD );
    if( pMain )
    {
        pMain->SetDebugFlags( pFrame->nFlags );
        // Loest Call aus!
        pFrame->SetAppMode( String( ResId ( IDS_APPMODE_RUN ) ) );
        pMain->Run();
        if (aBasicApp.pFrame)
        {
            BasicError* pErr = aBasicApp.pFrame->Basic().aErrors.First();
                if( pErr ) pErr->Show();
            aBasicApp.pFrame->SetAppMode( String() );
        }
        pMain->SetDebugFlags( 0 );
    }
}


