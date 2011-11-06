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
#include <tools/stream.hxx>
#include <basic/ttstrhlp.hxx>

#include "retstrm.hxx"
#include "rcontrol.hxx"
#include "svcommstream.hxx"


RetStream::RetStream()
{
    pSammel = new SvMemoryStream();
    pCommStream = new SvCommStream( pSammel );
//  SetCommStream( pCommStream );
}

RetStream::~RetStream()
{
    delete pCommStream;
    delete pSammel;
}

void RetStream::GenError ( rtl::OString aUId, String aString )
{
    CmdBaseStream::GenError ( &aUId, &aString );
}

void RetStream::GenReturn ( sal_uInt16 nRet, rtl::OString aUId, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, &aString );
}

void RetStream::GenReturn ( sal_uInt16 nRet, rtl::OString aUId, comm_ULONG nNr, String aString, sal_Bool bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nNr, &aString, bBool );
}

// MacroRecorder
void RetStream::GenReturn( sal_uInt16 nRet, rtl::OString aUId, comm_USHORT nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString );
}

void RetStream::GenReturn( sal_uInt16 nRet, rtl::OString aUId, comm_USHORT nMethod, String aString, sal_Bool bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString, bBool );
}


void RetStream::GenReturn ( sal_uInt16 nRet, sal_uInt16 nMethod, SbxValue &aValue )
{
    Write(sal_uInt16(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no sal_uLong needed)
    Write(sal_uInt16(PARAM_SBXVALUE_1));        // Typ der folgenden Parameter
    Write(aValue);
}

void RetStream::GenReturn( sal_uInt16 nRet, sal_uInt16 nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, nMethod, &aString );
}




void RetStream::Write( String *pString )
{
    CmdBaseStream::Write( pString->GetBuffer(), pString->Len() );
}

void RetStream::Write( SbxValue &aValue )
{
    *pSammel << sal_uInt16( BinSbxValue );
    aValue.Store( *pSammel );
}

void RetStream::Write( rtl::OString* pId )
{
    //HELPID BACKWARD (should use ByteString or OString)
    String aTmp( Id2Str( *pId ) );
    Write( &aTmp );
}


SvStream* RetStream::GetStream()
{
    return pSammel;
}

void RetStream::Reset ()
{
    delete pCommStream;
    delete pSammel;
    pSammel = new SvMemoryStream();
    pCommStream = new SvCommStream( pSammel );
//  SetCommStream( pCommStream );
}

