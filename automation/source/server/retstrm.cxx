/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: retstrm.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:25:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

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

void RetStream::GenReturn ( USHORT nRet, SmartId aUId, ULONG nNr, String aString, BOOL bBool )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nNr, &aString, bBool );
}

void RetStream::GenReturn( USHORT nRet, SmartId aUId, USHORT nMethod, String aString )
{
    CmdBaseStream::GenReturn ( nRet, &aUId, nMethod, &aString );
}

void RetStream::GenReturn( USHORT nRet, SmartId aUId, USHORT nMethod, String aString, BOOL bBool )
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
        Write( pId->GetNum() );
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

