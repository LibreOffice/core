/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biff.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:30:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <sal/config.h>
#include <stdio.h>
#include <sfx2/docfile.hxx>

#include "global.hxx"
#include "scerrors.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "biff.hxx"

ScBiffReader::ScBiffReader( SfxMedium & rMedium ) :
    mnId(0),
    mnLength(0),
    mnOffset(0)
{
    mpStream = rMedium.GetInStream();
    if( mpStream )
    {
        mpStream->SetBufferSize( 65536 );
        mpStream->SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
    }
}

ScBiffReader::~ScBiffReader()
{
    if( mpStream )
        mpStream->SetBufferSize( 0 );
}

bool ScBiffReader::nextRecord()
{
    if( !recordsLeft() )
        return false;

    if( IsEndOfFile() )
        return false;

    sal_uInt32 nPos = mpStream->Tell();
    if( nPos != mnOffset + mnLength )
        mpStream->Seek( mnOffset + mnLength );

    mnLength = mnId = 0;
    *mpStream >> mnId >> mnLength;

    mnOffset = mpStream->Tell();
#ifdef DEBUG
    fprintf( stderr, "Read record 0x%x length 0x%x at offset 0x%x\n",
        mnId, mnLength, mnOffset );

#if 1  // rather verbose
    int len = mnLength;
    while (len > 0) {
        int i, chunk = len < 16 ? len : 16;
        unsigned char data[16];
        mpStream->Read( data, chunk );

        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%.2x ", data[i] );
        fprintf( stderr, "| " );
        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%c", data[i] < 127 && data[i] > 30 ? data[i] : '.' );
        fprintf( stderr, "\n" );

        len -= chunk;
    }
    mpStream->Seek( mnOffset );
#endif
#endif
    return true;
}

