/*************************************************************************
 *
 *  $RCSfile: mybasic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gh $ $Date: 2000-11-07 14:03:46 $
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

#include "comm_bas.hxx"
#include "processw.hxx"

TYPEINIT1(MyBasic,StarBASIC)

class MyFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( UINT16 nSbxId, UINT32 = SBXCR_SBX );
};

static SampleObjectFac aFac1;
static MyFactory aFac2;
static CommunicationFactory aComManFac;
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
        AddFactory( &aComManFac );
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
        RemoveFactory( &aComManFac );
        RemoveFactory( &aProcessFac );
    }
}

BOOL MyBasic::Compile( SbModule* p )
{
    aErrors.Clear();
    nError = 0;
    return StarBASIC::Compile( p );
}

BOOL MyBasic::ErrorHdl()
{
    AppBasEd *pCurrWin = aBasicApp.pFrame->FindModuleWin( GetActiveModule()->GetName() );
    if(pCurrWin)
        pCurrWin->ToTop();
    else
    {       // erstmal Fenster aufmachen
        String aModName = GetActiveModule()->GetName();
        if ( aModName.Copy(0,2).CompareToAscii("--") == COMPARE_EQUAL )
            aModName.Erase(0,2);
        GetActiveModule()->SetName(aModName);
        AppWin* p = new AppBasEd( aBasicApp.pFrame, GetActiveModule() );
        p->Show();
        p->GrabFocus();
    }
    if( IsCompilerError() )
    {
        aErrors.Insert(
          new BasicError
            ( pCurrWin,
              0, StarBASIC::GetErrorText(), GetLine(), GetCol1(), GetCol2() ),
              LIST_APPEND );
        nError++;
        return BOOL( nError < 20 ); // Abbruch nach 20 Fehlern
    }
    else
    {
        ReportRuntimeError( pCurrWin );
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

const String MyBasic::GetSpechialErrorText()
{
    return GetErrorText();
}

USHORT MyBasic::BreakHdl()
{
    SbModule* pMod = GetActiveModule();
    if( pMod )
    {
        AppEdit* pWin = aBasicApp.pFrame->FindModuleWin( pMod->GetName() );
        if( !pWin )
        {       // erstmal Fenster aufmachen
            String aModName = GetActiveModule()->GetName();
            if ( aModName.Copy(0,2).CompareToAscii("--") == COMPARE_EQUAL )
                aModName.Erase(0,2);
            GetActiveModule()->SetName(aModName);
            AppWin* p = new AppBasEd( aBasicApp.pFrame, GetActiveModule() );
            p->Show();
            p->GrabFocus();
            p->ToTop();
            pWin = aBasicApp.pFrame->FindModuleWin( aModName );
        }
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
    if( pWin ) {
        pWin->Highlight( nLine, nCol1, nCol2 );
        aBasicApp.pFrame->pStatus->Message( aText );
    } else MessBox( aBasicApp.pFrame, WB_OK, aBasicApp.pFrame->GetText(),
                    aText ).Execute();
}


