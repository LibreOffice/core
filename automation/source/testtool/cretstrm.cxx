/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cretstrm.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:31:07 $
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
#ifndef _CRetStream_HXX
#include "cretstrm.hxx"
#endif
#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#include "rcontrol.hxx"
#include "svcommstream.hxx"


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
    USHORT nLenInChars;
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
    if ( IS_TYPE( SbxValue, xBase ) )
        aValue = *PTR_CAST( SbxValue, &xBase );
}

