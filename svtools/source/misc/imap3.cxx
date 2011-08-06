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
#include "precompiled_svtools.hxx"


#include <svtools/imap.hxx>

#include <tools/debug.hxx>


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCompat::IMapCompat( SvStream& rStm, const sal_uInt16 nStreamMode ) :
            pRWStm      ( &rStm ),
            nStmMode    ( nStreamMode )
{
    DBG_ASSERT( nStreamMode == STREAM_READ || nStreamMode == STREAM_WRITE, "Wrong Mode!" );

    if ( !pRWStm->GetError() )
    {
        if ( nStmMode == STREAM_WRITE )
        {
            nCompatPos = pRWStm->Tell();
            pRWStm->SeekRel( 4 );
            nTotalSize = nCompatPos + 4;
        }
        else
        {
            sal_uInt32 nTotalSizeTmp;
            *pRWStm >> nTotalSizeTmp;
            nTotalSize = nTotalSizeTmp;
            nCompatPos = pRWStm->Tell();
        }
    }
}


/******************************************************************************
|*
|* Dtor
|*
\******************************************************************************/

IMapCompat::~IMapCompat()
{
    if ( !pRWStm->GetError() )
    {
        if ( nStmMode == STREAM_WRITE )
        {
            const sal_uLong nEndPos = pRWStm->Tell();

            pRWStm->Seek( nCompatPos );
            *pRWStm << (sal_uInt32) ( nEndPos - nTotalSize );
            pRWStm->Seek( nEndPos );
        }
        else
        {
            const sal_uLong nReadSize = pRWStm->Tell() - nCompatPos;

            if ( nTotalSize > nReadSize )
                pRWStm->SeekRel( nTotalSize - nReadSize );
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
