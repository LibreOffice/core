/*************************************************************************
 *
 *  $RCSfile: imap3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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



