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

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/
#include <osl/endian.h>

#include "cmdbasestream.hxx"
#include "rcontrol.hxx"

CmdBaseStream::CmdBaseStream()
: pCommStream( NULL )
{
}

CmdBaseStream::~CmdBaseStream()
{
}

void CmdBaseStream::GenError (rtl::OString *pUId, comm_String *pString )
{
    Write(comm_USHORT(SIReturnError));
    Write(pUId);
    Write(pString);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, comm_ULONG nUId )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(nUId);
    Write(comm_USHORT(PARAM_NONE));             // Typ der folgenden Parameter
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, rtl::OString *pUId, comm_ULONG nNr )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    if ( pUId->equals( rtl::OString( "UID_ACTIVE" ) ) )
        Write(comm_ULONG(0));
    else
        Write(pUId);
    Write(comm_USHORT(PARAM_ULONG_1));          // Typ der folgenden Parameter
    Write(nNr);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, rtl::OString *pUId, comm_String *pString )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    if ( pUId->equals( rtl::OString( "UID_ACTIVE" ) ) )
        Write(comm_ULONG(0));
    else
        Write(pUId);
    Write(comm_USHORT(PARAM_STR_1));                // Typ der folgenden Parameter
    Write(pString);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, rtl::OString *pUId, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    if ( pUId->equals( rtl::OString( "UID_ACTIVE" ) ) )
        Write(comm_ULONG(0));
    else
        Write(pUId);
    Write(comm_USHORT(PARAM_BOOL_1));           // Typ der folgenden Parameter
    Write(bBool);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, rtl::OString *pUId, comm_ULONG nNr, comm_String *pString, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    if ( pUId->equals( rtl::OString( "UID_ACTIVE" ) ) )
        Write(comm_ULONG(0));
    else
        Write(pUId);
    Write(comm_USHORT(PARAM_ULONG_1|PARAM_STR_1|PARAM_BOOL_1));     // Typ der folgenden Parameter
    Write(nNr);
    Write(pString);
    Write(bBool);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, comm_USHORT nMethod, comm_ULONG nNr )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no sal_uLong needed)
    Write(comm_USHORT(PARAM_ULONG_1));          // Typ der folgenden Parameter
    Write(nNr);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, comm_USHORT nMethod, comm_String *pString )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no sal_uLong needed)
    Write(comm_USHORT(PARAM_STR_1));                // Typ der folgenden Parameter
    Write(pString);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, comm_USHORT nMethod, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no sal_uLong needed)
    Write(comm_USHORT(PARAM_BOOL_1));           // Typ der folgenden Parameter
    Write(bBool);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, comm_USHORT nMethod, comm_USHORT nNr )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no sal_uLong needed)
    Write(comm_USHORT(PARAM_USHORT_1));         // Typ der folgenden Parameter
    Write(nNr);
}


// MacroRecorder
void CmdBaseStream::GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1));     // Typ der folgenden Parameter
    Write(nMethod);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_String *pString )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_STR_1));     // Typ der folgenden Parameter
    Write(nMethod);
    Write(pString);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_String *pString, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_STR_1|PARAM_BOOL_1));        // Typ der folgenden Parameter
    Write(nMethod);
    Write(pString);
    Write(bBool);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_BOOL_1));        // Typ der folgenden Parameter
    Write(nMethod);
    Write(bBool);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, rtl::OString *pUId, comm_USHORT nMethod, comm_ULONG nNr )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_ULONG_1));       // Typ der folgenden Parameter
    Write(nMethod);
    Write(nNr);
}



void CmdBaseStream::Read (comm_USHORT &nNr)
{
    comm_USHORT nId;
    *pCommStream >> nId;
    if (pCommStream->IsEof()) return;
#ifdef DBG_UTIL
    if (nId != BinUSHORT) DBG_ERROR1( "Falscher Typ im Stream: Erwartet USHORT, gefunden :%hu", nId );
#endif
    *pCommStream >> nNr;
}

void CmdBaseStream::Read (comm_ULONG &nNr)
{
    comm_USHORT nId;
    *pCommStream >> nId;
    if (pCommStream->IsEof()) return;
#ifdef DBG_UTIL
    if (nId != BinULONG) DBG_ERROR1( "Falscher Typ im Stream: Erwartet ULONG, gefunden :%hu", nId );
#endif
    *pCommStream >> nNr;
}

void CmdBaseStream::Read (comm_UniChar* &aString, comm_USHORT &nLenInChars )
{
    comm_USHORT nId;
    *pCommStream >> nId;
#ifdef DBG_UTIL
    if (nId != BinString) DBG_ERROR1( "Falscher Typ im Stream: Erwartet String, gefunden :%hu", nId );
#endif

    *pCommStream >> nLenInChars;

    aString = new comm_UniChar [nLenInChars];
    pCommStream->Read( aString, ((comm_ULONG)nLenInChars) * sizeof( comm_UniChar ) );
#ifdef OSL_BIGENDIAN
    // we have to change the byteorder
    comm_USHORT n;
    for ( n = 0 ; n < nLenInChars ; n++ )
        aString[ n ] = aString[ n ] >> 8 | aString[ n ] << 8;
#endif
}

void CmdBaseStream::Read (comm_BOOL &bBool)
{
    comm_USHORT nId;
    *pCommStream >> nId;
#ifdef DBG_UTIL
    if (nId != BinBool) DBG_ERROR1( "Falscher Typ im Stream: Erwartet BOOL, gefunden :%hu", nId );
#endif
    *pCommStream >> bBool;
}

comm_USHORT CmdBaseStream::GetNextType()
{
    comm_USHORT nId;
    *pCommStream >> nId;
    pCommStream->SeekRel(-2);
    return nId;
}


void CmdBaseStream::Write( comm_USHORT nNr )
{
    *pCommStream << comm_USHORT( BinUSHORT );
    *pCommStream << nNr;
}

void CmdBaseStream::Write( comm_ULONG nNr )
{
    *pCommStream << comm_USHORT( BinULONG );
    *pCommStream << nNr;
}

void CmdBaseStream::Write( const comm_UniChar* aString, comm_USHORT nLenInChars )
{
    *pCommStream << comm_USHORT(BinString);

    comm_USHORT n;

    // remove BiDi and zero-width-markers    0x200B - 0x200F
    // remove BiDi and paragraph-markers     0x2028 - 0x202E

    comm_UniChar* aNoBiDiString;
    aNoBiDiString = new comm_UniChar [nLenInChars];
    comm_USHORT nNewLenInChars = 0;
    for ( n = 0 ; n < nLenInChars ; n++ )
    {
        comm_UniChar c = aString[ n ];
        if (  ((c >= 0x200B) && (c <= 0x200F))
            ||((c >= 0x2028) && (c <= 0x202E)) )
        {   //Ignore character
        }
        else
        {
            aNoBiDiString[ nNewLenInChars ] = c;
            nNewLenInChars++;
        }
    }

    *pCommStream << nNewLenInChars;

#ifdef OSL_BIGENDIAN
    // we have to change the byteorder
    comm_UniChar* aNewString;
    aNewString = new comm_UniChar [nNewLenInChars];
    for ( n = 0 ; n < nNewLenInChars ; n++ )
        aNewString[ n ] = aNoBiDiString[ n ] >> 8 | aNoBiDiString[ n ] << 8;
    pCommStream->Write( aNewString, ((comm_ULONG)nNewLenInChars) * sizeof( comm_UniChar ) );
    delete [] aNewString;
#else
    pCommStream->Write( aNoBiDiString, ((comm_ULONG)nNewLenInChars) * sizeof( comm_UniChar ) );
#endif

    delete [] aNoBiDiString;
}

void CmdBaseStream::Write( comm_BOOL bBool )
{
    *pCommStream << comm_USHORT( BinBool );
    *pCommStream << bBool;
}

void CmdBaseStream::Read ( comm_String* &pString )
{
    (void) pString; /* avoid warning about unused parameter */
    DBG_ERROR("Read ( comm_String* &pString ) Not Implemented");
}
void CmdBaseStream::Read ( rtl::OString* &pId )
{
    (void) pId; /* avoid warning about unused parameter */
    DBG_ERROR("Read ( rtl::OString* &pId ) Not Implemented");
}

void CmdBaseStream::Write( comm_String *pString )
{
    (void) pString; /* avoid warning about unused parameter */
    DBG_ERROR("Write( comm_String *pString ) Not Implemented");
}
void CmdBaseStream::Write( rtl::OString* pId )
{
    (void) pId; /* avoid warning about unused parameter */
    DBG_ERROR("Write( rtl::OString* pId ) Not Implemented");
}

