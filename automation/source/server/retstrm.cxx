/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: retstrm.cxx,v $
 * $Revision: 1.7 $
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

void RetStream::GenError ( SmartId aUId, String aString )
{
    CmdBaseStream::GenError ( &aUId, &aString );
}

void RetStream::GenReturn ( USHORT nRet, SmartId aUId, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, &aString );
}

void RetStream::GenReturn ( USHORT nRet, SmartId aUId, SbxValue &aValue )
{
    Write(USHORT(SIReturn));
    Write(nRet);
    Write(&aUId);
    Write(USHORT(PARAM_SBXVALUE_1));        // Typ der folgenden Parameter
    Write(aValue);
}

void RetStream::GenReturn ( USHORT nRet, SmartId aUId, comm_ULONG nNr, String aString, BOOL bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nNr, &aString, bBool );
}

void RetStream::GenReturn( USHORT nRet, SmartId aUId, comm_USHORT nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString );
}

void RetStream::GenReturn( USHORT nRet, SmartId aUId, comm_USHORT nMethod, String aString, BOOL bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString, bBool );
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

void RetStream::Write( SmartId* pId )
{
    DBG_ASSERT( !pId->HasString() || !pId->HasNumeric(), "SmartId contains Number and String. using String only." )
    if ( pId->HasString() )
    {
        String aTmp( pId->GetStr() );
        Write( &aTmp );
    }
    else
        Write( static_cast<comm_ULONG>(pId->GetNum()) ); ////GetNum() ULONG != comm_ULONG on 64bit
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

