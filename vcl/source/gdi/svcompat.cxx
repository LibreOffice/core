/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svcompat.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:11:41 $
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
#include "precompiled_vcl.hxx"

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SV_SVCOMPAT_HXX
#include <svcompat.hxx>
#endif

// --------------
// - ImplCompat -
// --------------

ImplCompat::ImplCompat( SvStream& rStm, USHORT nStreamMode, USHORT nVersion ) :
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
        const UINT32 nEndPos = mpRWStm->Tell();

        mpRWStm->Seek( mnCompatPos );
        *mpRWStm << ( nEndPos - mnTotalSize );
        mpRWStm->Seek( nEndPos );
    }
    else
    {
        const UINT32 nReadSize = mpRWStm->Tell() - mnCompatPos;

        if( mnTotalSize > nReadSize )
            mpRWStm->SeekRel( mnTotalSize - nReadSize );
    }
}
