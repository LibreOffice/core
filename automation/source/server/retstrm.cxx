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

void RetStream::GenReturn ( USHORT nRet, rtl::OString aUId, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, &aString );
}

void RetStream::GenReturn ( USHORT nRet, rtl::OString aUId, comm_ULONG nNr, String aString, BOOL bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nNr, &aString, bBool );
}

// MacroRecorder
void RetStream::GenReturn( USHORT nRet, rtl::OString aUId, comm_USHORT nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString );
}

void RetStream::GenReturn( USHORT nRet, rtl::OString aUId, comm_USHORT nMethod, String aString, BOOL bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString, bBool );
}


void RetStream::GenReturn ( USHORT nRet, USHORT nMethod, SbxValue &aValue )
{
    Write(USHORT(SIReturn));
    Write(nRet);
    Write((comm_ULONG)nMethod); //HELPID BACKWARD (no ULONG needed)
    Write(USHORT(PARAM_SBXVALUE_1));        // Typ der folgenden Parameter
    Write(aValue);
}

void RetStream::GenReturn( USHORT nRet, USHORT nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, nMethod, &aString );
}




void RetStream::Write( String *pString )
{
    CmdBaseStream::Write( pString->GetBuffer(), pString->Len() );
}

void RetStream::Write( SbxValue &aValue )
{
    *pSammel << USHORT( BinSbxValue );
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

