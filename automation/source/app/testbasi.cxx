/*************************************************************************
 *
 *  $RCSfile: testbasi.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 16:03:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include <basic/testtool.hxx>
#include "testbasi.hxx"
//#include "app.hxx"
//#include "basic.hrc"
//#include "appbased.hxx"
#define P_FEHLERLISTE ((TestToolObj*)pTestObject)->GetFehlerListe()

//#include "status.hxx"
//#include "basic.hrc"

//#include "object.hxx"

#include <testapp.hxx>
#include <testtool.hxx>
#ifndef _SB_INTERN_HXX
//#include "sbintern.hxx"
#endif

//#include "comm_bas.hxx"
//#include "processw.hxx"

TYPEINIT1(TTBasic,MyBasic)

/*class MyFactory : public SbxFactory
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
} */

TTBasic::TTBasic() : MyBasic()
{
// Eigenes Objekt reinbraten
    TestToolObj* pTTO = new TestToolObj( CUniString("App"), this );
    pTTO->SetLogHdl( GenLogHdl() );
    pTTO->SetWinInfoHdl( GenWinInfoHdl() );
    pTTO->SetModuleWinExistsHdl( GenModuleWinExistsHdl() );
    pTTO->SetWriteStringHdl( GenWriteStringHdl() );
    pTTO->SetCErrorHdl( LINK( this, TTBasic, CErrorImpl ) );

    StartListening( pTTO->GetTTBroadcaster() );

    pTestObject = pTTO;
    pTestObject->SetFlag( SBX_EXTSEARCH );
    Insert( pTestObject );
}

MyBasic* TTBasic::CreateMyBasic()
{
    return new TTBasic();
}

void TTBasic::LoadIniFile()
{
    ((TestToolObj*)pTestObject)->LoadIniFile();
}

SbTextType TTBasic::GetSymbolType( const String &rSymbol, BOOL bWasTTControl )
{
    return ((TestToolObj*)pTestObject)->GetSymbolType( rSymbol, bWasTTControl );
}


TTBasic::~TTBasic()
{
}

IMPL_LINK( TTBasic, CErrorImpl, ErrorEntry*, pData )
{
    return CError( pData->nError, pData->aText, pData->nLine, pData->nCol1, pData->nCol2 );
}

BOOL TTBasic::Compile( SbModule* p )
{
    SbModule* pOldModule = GetCompileModule();
    SetCompileModule( p );
    p->SetSource( ((TestToolObj*)pTestObject)->PreCompile(p->GetSource()) );
    SetCompileModule( pOldModule );
    if ( ((TestToolObj*)pTestObject)->WasPrecompilerError() )
        return FALSE;
    return MyBasic::Compile( p );
}

const String TTBasic::GetSpechialErrorText()
{
    String nErrorText;
    if ( pTestObject && IS_ERROR() && GetErrorCode() == GET_ERROR()->nError )
    {
        nErrorText = GenRealString( GET_ERROR()->aText );
        nErrorText.AppendAscii( ": " );
        nErrorText += String::CreateFromInt64( GET_ERROR()->nError );
    }
    else
    {
        nErrorText = GetErrorText();
    }
    return nErrorText;
}

void TTBasic::ReportRuntimeError( AppBasEd *pEditWin )
{
    SbxVariableRef aDummy = new SbxVariable;
    aDummy->SetUserData( 24 );  // ID_MaybeAddErr
    ((TestToolObj*)pTestObject)->SFX_NOTIFY( pTestObject->GetBroadcaster(), xx, SbxHint( SBX_HINT_DATAWANTED, aDummy ), xx );
    aDummy->SetUserData( 18 );  // ID_ExceptLog
    ((TestToolObj*)pTestObject)->SFX_NOTIFY( pTestObject->GetBroadcaster(), xx, SbxHint( SBX_HINT_DATAWANTED, aDummy ), xx );
    MyBasic::ReportRuntimeError( pEditWin );
}

void TTBasic::DebugFindNoErrors( BOOL bDebugFindNoErrors )
{
    ((TestToolObj*)pTestObject)->DebugFindNoErrors( bDebugFindNoErrors );
}
