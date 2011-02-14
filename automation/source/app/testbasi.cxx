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

TYPEINIT1(TTBasic,MyBasic)

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
