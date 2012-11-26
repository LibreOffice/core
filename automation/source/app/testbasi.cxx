/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"
#include <basic/sbx.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include <basic/testtool.hxx>
#include "testbasi.hxx"
//#include "app.hxx"
//#include "basic.hrc"
//#include "appbased.hxx"
#define P_FEHLERLISTE TestToolObj::pFehlerListe

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

/*class MyFactory : public SbxFactory
{
public:
    virtual SbxBase* Create( sal_uInt16 nSbxId, sal_uInt32 = SBXCR_SBX );
};

static SampleObjectFac aFac1;
static MyFactory aFac2;
static CommunicationFactory aComManFac;
static ProcessFactory aProcessFac;
static short nInst = 0;

SbxBase* MyFactory::Create( sal_uInt16 nSbxId, sal_uInt32 nCr )
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

    StartListeningTT( pTTO->GetTTBroadcaster() );

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

SbTextType TTBasic::GetSymbolType( const String &rSymbol, sal_Bool bWasTTControl )
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

sal_Bool TTBasic::Compile( SbModule* p )
{
    p->SetComment( ((TestToolObj*)pTestObject)->GetRevision(p->GetSource()) );
    SbModule* pOldModule = GetCompileModule();
    SetCompileModule( p );
    p->SetSource( ((TestToolObj*)pTestObject)->PreCompile(p->GetSource()) );
    SetCompileModule( pOldModule );
    if ( ((TestToolObj*)pTestObject)->WasPrecompilerError() )
        return sal_False;
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
    ((TestToolObj*)pTestObject)->Notify( pTestObject->GetBroadcaster(), SbxHint( SBX_HINT_DATAWANTED, aDummy ) );
    aDummy->SetUserData( 18 );  // ID_ExceptLog
    ((TestToolObj*)pTestObject)->Notify( pTestObject->GetBroadcaster(), SbxHint( SBX_HINT_DATAWANTED, aDummy ) );
    MyBasic::ReportRuntimeError( pEditWin );
}

void TTBasic::DebugFindNoErrors( sal_Bool bDebugFindNoErrors )
{
    ((TestToolObj*)pTestObject)->DebugFindNoErrors( bDebugFindNoErrors );
}
