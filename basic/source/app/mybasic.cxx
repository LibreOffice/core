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

// AB-Uno-Test
//#define unotest
#ifdef unotest
#ifndef _USR_UNO_HXX
#include <usr/uno.hxx>
#endif
#include <basic/sbuno.hxx>
#include <sbunoobj.hxx>
#endif

#include "sbintern.hxx"

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include <basic/mybasic.hxx>
#include "basic.hrc"
#include "appbased.hxx"

#include "status.hxx"
#include "basic.hrc"

#include "object.hxx"

#include "processw.hxx"
#include "basrid.hxx"

TYPEINIT1(MyBasic,StarBASIC)

class MyFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
};

static SampleObjectFac aFac1;
static MyFactory aFac2;
static ProcessFactory aProcessFac;
static short nInst = 0;

SbxBase* MyFactory::Create( sal_uInt16 nSbxId, sal_uInt32 nCr )
{
    if( nCr == SBXCR_TEST && nSbxId == SBXID_MYBASIC )
        return new MyBasic;
    else
        return NULL;
}

MyBasic::MyBasic() : StarBASIC()
{
    nError = 0;
    if( !nInst++ )
    {
        AddFactory( &aFac1 );
        AddFactory( &aFac2 );
        AddFactory( &aProcessFac );
    }
    SbxVariable* p = new SbxCollection( CUniString("MyColl") );
    p->SetName( CUniString("Objects") );
    Insert( p );

    // AB-Uno-Test
#ifdef unotest
    // Get Uno-Service-Manager and Reflection Service
    createAndSetDefaultServiceManager();        // done later

    // Get Uno-Test-Object
    UsrAny aObjAny = getIntrospectionTestObject();

    // Box object into SbUnoObject
    String aName( "UnoObject" );
    SbxObjectRef xSbUnoObj = GetSbUnoObject( aName, aObjAny );
    //SbxObjectRef xSbUnoObj = new SbUnoObject( aName, aObjAny );
    Insert( (SbxObject*)xSbUnoObj );
#endif

    pTestObject = NULL;
}

Link MyBasic::GenLogHdl()
{
    return LINK( GetpApp()->GetAppWindow(), BasicFrame, Log );
}

Link MyBasic::GenWinInfoHdl()
{
    return LINK( GetpApp()->GetAppWindow(), BasicFrame, WinInfo );
}

Link MyBasic::GenModuleWinExistsHdl()
{
    return LINK( GetpApp()->GetAppWindow(), BasicFrame, ModuleWinExists );
}

Link MyBasic::GenWriteStringHdl()
{
    return LINK( GetpApp()->GetAppWindow(), BasicFrame, WriteString );
}

void MyBasic::StartListeningTT( SfxBroadcaster &rBroadcaster )
{
    ((BasicFrame*)GetpApp()->GetAppWindow())->StartListening( rBroadcaster );
}

void MyBasic::SetCompileModule( SbModule *pMod )
{
    GetSbData()->pCompMod = pMod;
}

SbModule *MyBasic::GetCompileModule()
{
    return GetSbData()->pCompMod;
}

String MyBasic::GenRealString( const String &aResString )
{
    return ((BasicFrame*)GetpApp()->GetAppWindow())->GenRealString( aResString );
}

void MyBasic::LoadIniFile()
{
}

SbTextType MyBasic::GetSymbolType( const String &rSymbol, sal_Bool bWasTTControl )
{
    (void) rSymbol;       /* avoid warning about unused parameter */
    (void) bWasTTControl; /* avoid warning about unused parameter */
    return SB_SYMBOL;     // Everything here is of type SB_SYMBOL and continues to be so
}


MyBasic::~MyBasic()
{
    aErrors.Clear();
    if( !--nInst )
    {
        RemoveFactory( &aFac1 );
        RemoveFactory( &aFac2 );
        RemoveFactory( &aProcessFac );
    }
}

void MyBasic::Reset()
{
    aErrors.Clear();
    nError = 0;
}

sal_Bool MyBasic::Compile( SbModule* p )
{
    Reset();
    return StarBASIC::Compile( p );
}

sal_Bool MyBasic::ErrorHdl()
{
    AppBasEd* pWin = aBasicApp.pFrame->FindModuleWin( GetActiveModule()->GetName() );
    if( !pWin )
    {       // open a window
        pWin = aBasicApp.pFrame->CreateModuleWin( GetActiveModule() );
    }
    else
        pWin->ToTop();
    if( IsCompilerError() )
    {
        aErrors.Insert(
          new BasicError
            ( pWin,
              0, StarBASIC::GetErrorText(), GetLine(), GetCol1(), GetCol2() ),
              LIST_APPEND );
        nError++;
        return sal_Bool( nError < 20 ); // Cancel after 20 errors
    }
    else
    {
        ReportRuntimeError( pWin );
        return sal_False;
    }
}

void MyBasic::ReportRuntimeError( AppBasEd *pEditWin )
{
    String nErrorText;
    nErrorText = GetSpechialErrorText();

    if ( pEditWin )     // just in case the focus is not right
        pEditWin->ToTop();

    BasicError( pEditWin,
        GetVBErrorCode( GetErrorCode() ), nErrorText, GetLine(),
        GetCol1(), GetCol2() ).Show();
}

void MyBasic::DebugFindNoErrors( sal_Bool bDebugFindNoErrors )
{
    (void) bDebugFindNoErrors; /* avoid warning about unused parameter */
}

const String MyBasic::GetSpechialErrorText()
{
    return GetErrorText();
}

sal_uInt16 MyBasic::BreakHdl()
{
    SbModule* pMod = GetActiveModule();
    if( pMod )
    {
        AppBasEd* pWin = aBasicApp.pFrame->FindModuleWin( pMod->GetName() );
        if( !pWin )
        {       // open a window
            pWin = aBasicApp.pFrame->CreateModuleWin( pMod );
        }
        else
            pWin->ToTop();
        pWin->Highlight( GetLine(), GetCol1(), GetCol2() );
    }

    if( IsBreak() ) // If Breakpoint (or "Run to Cursor")
    {
//      if ( GetActiveModule()->IsBP(GetLine()) )
//          GetActiveModule()->ClearBP(GetLine());
        return aBasicApp.pFrame->BreakHandler();
    }
    else
    {
        return aBasicApp.pFrame->BreakHandler();
    }
}

/***************************************************************************
|*
|*    class BasicError
|*
***************************************************************************/

BasicError::BasicError
    ( AppBasEd* w, sal_uInt16 nE, const String& r, sal_uInt16 nL, sal_uInt16 nC1, sal_uInt16 nC2 )
    : aText( SttResId( IDS_ERROR1 ) )
{
    pWin  = w;
    nLine = nL;
    nCol1 = nC1;
    nCol2 = nC2;
    if( nE )
    {
        aText += String::CreateFromInt32( nE );
        aText.AppendAscii(": ");
        aText += r;
    }
    else
        aText = r;
}

// This is a sample how to build the error information
// to highlight a statement
void BasicError::Show()
{
    if( pWin && aBasicApp.pFrame->IsWinValid( pWin ) )
    {
        pWin->Highlight( nLine, nCol1, nCol2 );
        aBasicApp.pFrame->pStatus->Message( aText );
    }
    else
        MessBox( aBasicApp.pFrame, WB_OK, aBasicApp.pFrame->GetText(),
                 aText ).Execute();
}


