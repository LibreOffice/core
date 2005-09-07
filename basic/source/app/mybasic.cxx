/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mybasic.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:17:38 $
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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <sbx.hxx>
#endif

// AB-Uno-Test
//#define unotest
#ifdef unotest
#ifndef _USR_UNO_HXX
#include <usr/uno.hxx>
#endif
#include <sbuno.hxx>
#include <sbunoobj.hxx>
#endif

#include "sbintern.hxx"

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif
#include "mybasic.hxx"
#include "basic.hrc"
#include "appbased.hxx"

#include "status.hxx"
#include "basic.hrc"

#include "object.hxx"

#include "processw.hxx"

TYPEINIT1(MyBasic,StarBASIC)

class MyFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
};

static SampleObjectFac aFac1;
static MyFactory aFac2;
static ProcessFactory aProcessFac;
static short nInst = 0;

SbxBase* MyFactory::Create( UINT16 nSbxId, UINT32 nCr )
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
    // Uno-Service-Manager holenReflection Service bolen
    createAndSetDefaultServiceManager();        // spaeter schon erledigt

    // Uno-Test-Objekt holen
    UsrAny aObjAny = getIntrospectionTestObject();

    // Objekt verpacken in ein SbUnoObject packen
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

void MyBasic::StartListening( SfxBroadcaster &rBroadcaster )
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

SbTextType MyBasic::GetSymbolType( const String &rSymbol, BOOL bWasTTControl )
{
    return SB_SYMBOL;   // Alles was hier landet ist vom Typ SB_SYMBOL und bleibt es auch
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

BOOL MyBasic::Compile( SbModule* p )
{
    Reset();
    return StarBASIC::Compile( p );
}

BOOL MyBasic::ErrorHdl()
{
    AppBasEd* pWin = aBasicApp.pFrame->FindModuleWin( GetActiveModule()->GetName() );
    if( !pWin )
    {       // erstmal Fenster aufmachen
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
        return BOOL( nError < 20 ); // Abbruch nach 20 Fehlern
    }
    else
    {
        ReportRuntimeError( pWin );
        return FALSE;
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

void MyBasic::DebugFindNoErrors( BOOL bDebugFindNoErrors )
{
}

const String MyBasic::GetSpechialErrorText()
{
    return GetErrorText();
}

USHORT MyBasic::BreakHdl()
{
    SbModule* pMod = GetActiveModule();
    if( pMod )
    {
        AppBasEd* pWin = aBasicApp.pFrame->FindModuleWin( pMod->GetName() );
        if( !pWin )
        {       // erstmal Fenster aufmachen
            pWin = aBasicApp.pFrame->CreateModuleWin( pMod );
        }
        else
            pWin->ToTop();
        pWin->Highlight( GetLine(), GetCol1(), GetCol2() );
    }

    if( IsBreak() ) // Wenn Breakpoint (oder "Run to Cursor")
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
|*                          class BasicError
|*
***************************************************************************/

BasicError::BasicError
    ( AppBasEd* w, USHORT nE, const String& r, USHORT nL, USHORT nC1, USHORT nC2 )
    : aText( ResId( IDS_ERROR1 ) )
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

// Dies ist ein Beispiel, wie die Fehler-Information geschickt
// aufgebaut werden kann, um ein Statement zu highlighten.

void BasicError::Show()
{
    if( pWin && aBasicApp.pFrame->IsWinValid( pWin ) ) {
        pWin->Highlight( nLine, nCol1, nCol2 );
        aBasicApp.pFrame->pStatus->Message( aText );
    } else MessBox( aBasicApp.pFrame, WB_OK, aBasicApp.pFrame->GetText(),
                    aText ).Execute();
}


