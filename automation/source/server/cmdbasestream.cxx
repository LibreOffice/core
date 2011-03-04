/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/
#include <osl/endian.h>
#include <osl/diagnose.h>

#include "cmdbasestream.hxx"
#include "rcontrol.hxx"

CmdBaseStream::CmdBaseStream()
: pCommStream( NULL )
{
}

CmdBaseStream::~CmdBaseStream()
{
}

void CmdBaseStream::GenError (SmartId *pUId, comm_String *pString )
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

void CmdBaseStream::GenReturn (comm_USHORT nRet, SmartId *pUId, comm_ULONG nNr )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_ULONG_1));          // Typ der folgenden Parameter
    Write(nNr);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, SmartId *pUId, comm_String *pString )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_STR_1));                // Typ der folgenden Parameter
    Write(pString);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, SmartId *pUId, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_BOOL_1));           // Typ der folgenden Parameter
    Write(bBool);
}

void CmdBaseStream::GenReturn (comm_USHORT nRet, SmartId *pUId, comm_ULONG nNr, comm_String *pString, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_ULONG_1|PARAM_STR_1|PARAM_BOOL_1));     // Typ der folgenden Parameter
    Write(nNr);
    Write(pString);
    Write(bBool);
}

// MacroRecorder
void CmdBaseStream::GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1));     // Typ der folgenden Parameter
    Write(nMethod);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_String *pString )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_STR_1));     // Typ der folgenden Parameter
    Write(nMethod);
    Write(pString);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_String *pString, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_STR_1|PARAM_BOOL_1));        // Typ der folgenden Parameter
    Write(nMethod);
    Write(pString);
    Write(bBool);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_BOOL bBool )
{
    Write(comm_USHORT(SIReturn));
    Write(nRet);
    Write(pUId);
    Write(comm_USHORT(PARAM_USHORT_1|PARAM_BOOL_1));        // Typ der folgenden Parameter
    Write(nMethod);
    Write(bBool);
}

void CmdBaseStream::GenReturn( comm_USHORT nRet, SmartId *pUId, comm_USHORT nMethod, comm_ULONG nNr )
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
    if (nId != BinUSHORT) OSL_TRACE( "Falscher Typ im Stream: Erwartet USHORT, gefunden :%hu", nId );
#endif
    *pCommStream >> nNr;
}

void CmdBaseStream::Read (comm_ULONG &nNr)
{
    comm_USHORT nId;
    *pCommStream >> nId;
    if (pCommStream->IsEof()) return;
#ifdef DBG_UTIL
    if (nId != BinULONG) OSL_TRACE( "Falscher Typ im Stream: Erwartet ULONG, gefunden :%hu", nId );
#endif
    *pCommStream >> nNr;
}

void CmdBaseStream::Read (comm_UniChar* &aString, comm_USHORT &nLenInChars )
{
    comm_USHORT nId;
    *pCommStream >> nId;
#ifdef DBG_UTIL
    if (nId != BinString) OSL_TRACE( "Falscher Typ im Stream: Erwartet String, gefunden :%hu", nId );
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
    if (nId != BinBool) OSL_TRACE( "Falscher Typ im Stream: Erwartet BOOL, gefunden :%hu", nId );
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
    OSL_FAIL("Read ( comm_String* &pString ) Not Implemented");
}
void CmdBaseStream::Read ( SmartId* &pId )
{
    (void) pId; /* avoid warning about unused parameter */
    OSL_FAIL("Read ( SmartId* &pId ) Not Implemented");
}

void CmdBaseStream::Write( comm_String *pString )
{
    (void) pString; /* avoid warning about unused parameter */
    OSL_FAIL("Write( comm_String *pString ) Not Implemented");
}
void CmdBaseStream::Write( SmartId* pId )
{
    (void) pId; /* avoid warning about unused parameter */
    OSL_FAIL("Write( SmartId* pId ) Not Implemented");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
