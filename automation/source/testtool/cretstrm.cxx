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
#ifndef _CRetStream_HXX
#include "cretstrm.hxx"
#endif
#include <tools/debug.hxx>
#include "rcontrol.hxx"
#include "svcommstream.hxx"
#include <typeinfo>


SV_IMPL_REF(SbxBase)

CRetStream::CRetStream(SvStream *pIn)
{
    pSammel = pIn;
    delete pCommStream;
    pCommStream = new SvCommStream( pSammel );
}

CRetStream::~CRetStream()
{
    delete pCommStream;
}

void CRetStream::Read ( String &aString )
{
    comm_UniChar* pStr;
    sal_uInt16 nLenInChars;
    CmdBaseStream::Read( pStr, nLenInChars );

    aString = String( pStr, nLenInChars );
    delete [] pStr;
}

void CRetStream::Read( SbxValue &aValue )
{
    *pSammel >> nId;
    if (nId != BinSbxValue)
    {
        DBG_ERROR1( "Falscher Typ im Stream: Erwartet SbxValue, gefunden :%hu", nId );
    }
    SbxBaseRef xBase = SbxBase::Load( *pSammel );

    // old RTTI did the following:
    //  if ( IS_TYPE( SbxValue, xBase ) )
    //      aValue = *dynamic_cast< SbxValue* >( &xBase );
    SbxValue* pSbxValue = dynamic_cast< SbxValue* >(&xBase);
    if(pSbxValue)
    {
        aValue = *pSbxValue;
    }
}

