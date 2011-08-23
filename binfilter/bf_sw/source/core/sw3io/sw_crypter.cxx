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
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <string.h>
#include <tools/string.hxx>

#include <crypter.hxx>

namespace binfilter {




Crypter::Crypter( const ByteString& r )
{
    // Dies sind Randomwerte, die konstant zur Verschluesselung
    // des Passworts verwendet werden. Durch die Verwendung eines
    // verschluesselten Passworts wird vermieden, dass das Passwort
    // im RAM gehalten wird.
    static const BYTE cEncode[] =
    { 0xAB, 0x9E, 0x43, 0x05, 0x38, 0x12, 0x4d, 0x44,
      0xD5, 0x7e, 0xe3, 0x84, 0x98, 0x23, 0x3f, 0xba };

    xub_StrLen nLen = r.Len();
    if( nLen > PASSWDLEN ) nLen = PASSWDLEN;
    ByteString aPasswd( r );
    if( nLen > PASSWDLEN )
        aPasswd.Erase( nLen );
    else
        aPasswd.Expand( PASSWDLEN, ' ' );
    memcpy( cPasswd, cEncode, PASSWDLEN );
    Encrypt( aPasswd );
    memcpy( cPasswd, aPasswd.GetBuffer(), PASSWDLEN );
}



void Crypter::Encrypt( ByteString& r ) const
{
    xub_StrLen nLen = r.Len();
    if( !nLen )
        return ;

    xub_StrLen nCryptPtr = 0;
    BYTE cBuf[ PASSWDLEN ];
    memcpy( cBuf, cPasswd, PASSWDLEN );
    BYTE* pSrc = (BYTE*)r.GetBufferAccess();
    BYTE* p = cBuf;

    while( nLen-- )
    {
        *pSrc = *pSrc ^ ( *p ^ (BYTE) ( cBuf[ 0 ] * nCryptPtr ) );
        *p += ( nCryptPtr < (PASSWDLEN-1) ) ? *(p+1) : cBuf[ 0 ];
        if( !*p ) *p += 1;
        p++;
        if( ++nCryptPtr >= PASSWDLEN ) nCryptPtr = 0, p = cBuf;
        pSrc++;
    }
}



void Crypter::Decrypt( ByteString& r ) const
{
    Encrypt( r );
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
