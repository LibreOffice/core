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
#include "precompiled_starmath.hxx"
#include "eqnolefilehdr.hxx"
#include <sot/storage.hxx>

//////////////////////////////////////////////////////////////////////

void EQNOLEFILEHDR::Read(SvStorageStream *pS)
{
    *pS >> nCBHdr;
    *pS >> nVersion;
    *pS >> nCf;
    *pS >> nCBObject;
    *pS >> nReserved1;
    *pS >> nReserved2;
    *pS >> nReserved3;
    *pS >> nReserved4;
}


void EQNOLEFILEHDR::Write(SvStorageStream *pS)
{
    *pS << nCBHdr;
    *pS << nVersion;
    *pS << nCf;
    *pS << nCBObject;
    *pS << nReserved1;
    *pS << nReserved2;
    *pS << nReserved3;
    *pS << nReserved4;
}


sal_Bool GetMathTypeVersion( SotStorage* pStor, sal_uInt8 &nVersion )
{
    sal_uInt8 nVer = 0;
    sal_Bool bSuccess = sal_False;

    //
    // code sniplet copied from MathType::Parse
    //
    SvStorageStreamRef xSrc = pStor->OpenSotStream(
        String::CreateFromAscii("Equation Native"),
        STREAM_STD_READ | STREAM_NOCREATE);
    if ( (!xSrc.Is()) || (SVSTREAM_OK != xSrc->GetError()))
        return bSuccess;
    SvStorageStream *pS = &xSrc;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    //
    EQNOLEFILEHDR aHdr;
    aHdr.Read(pS);
    *pS >> nVer;

    if (!pS->GetError())
    {
        nVersion = nVer;
        bSuccess = sal_True;
    }
    return bSuccess;
}

//////////////////////////////////////////////////////////////////////

