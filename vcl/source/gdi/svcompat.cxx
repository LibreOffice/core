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
#include "precompiled_vcl.hxx"
#include <tools/stream.hxx>
#include <svcompat.hxx>

// --------------
// - ImplCompat -
// --------------

ImplCompat::ImplCompat( SvStream& rStm, sal_uInt16 nStreamMode, sal_uInt16 nVersion ) :
            mpRWStm     ( &rStm ),
            mnStmMode   ( nStreamMode ),
            mnVersion   ( nVersion )
{
    if( !mpRWStm->GetError() )
    {
        if( STREAM_WRITE == mnStmMode )
        {
            *mpRWStm << mnVersion;
            mnTotalSize = ( mnCompatPos = mpRWStm->Tell() ) + 4UL;
            mpRWStm->SeekRel( 4L );
        }
        else
        {
            *mpRWStm >> mnVersion;
            *mpRWStm >> mnTotalSize;
            mnCompatPos = mpRWStm->Tell();
        }
    }
}

// ------------------------------------------------------------------------

ImplCompat::~ImplCompat()
{
    if( STREAM_WRITE == mnStmMode )
    {
        const sal_uInt32 nEndPos = mpRWStm->Tell();

        mpRWStm->Seek( mnCompatPos );
        *mpRWStm << ( nEndPos - mnTotalSize );
        mpRWStm->Seek( nEndPos );
    }
    else
    {
        const sal_uInt32 nReadSize = mpRWStm->Tell() - mnCompatPos;

        if( mnTotalSize > nReadSize )
            mpRWStm->SeekRel( mnTotalSize - nReadSize );
    }
}
