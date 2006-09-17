/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imap3.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:11:35 $
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
#include "precompiled_svtools.hxx"


#include "imap.hxx"

#include <tools/debug.hxx>


/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCompat::IMapCompat( SvStream& rStm, const USHORT nStreamMode ) :
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
            UINT32 nTotalSizeTmp;
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
            const ULONG nEndPos = pRWStm->Tell();

            pRWStm->Seek( nCompatPos );
            *pRWStm << (UINT32) ( nEndPos - nTotalSize );
            pRWStm->Seek( nEndPos );
        }
        else
        {
            const ULONG nReadSize = pRWStm->Tell() - nCompatPos;

            if ( nTotalSize > nReadSize )
                pRWStm->SeekRel( nTotalSize - nReadSize );
        }
    }
}



