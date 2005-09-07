/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eqnolefilehdr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:07:01 $
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

#ifndef __EQNOLEFILEHDR_HXX__
#include "eqnolefilehdr.hxx"
#endif

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

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

