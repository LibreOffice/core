/*************************************************************************
 *
 *  $RCSfile: retstrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:05:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    if ( pId->IsNumeric() )
        Write( pId->GetNum() );
    else
    {
        String aTmp( pId->GetStr() );
        Write( &aTmp );
    }
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

