/*************************************************************************
 *
 *  $RCSfile: mieclip.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dv $ $Date: 2001-07-26 12:03:55 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _CACHESTR_HXX //autogen
#include <tools/cachestr.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif

#include <mieclip.hxx>
#include "sfxuno.hxx"

MSE40HTMLClipFormatObj::~MSE40HTMLClipFormatObj()
{
    delete pStrm;
}

SvStream* MSE40HTMLClipFormatObj::IsValid( SvStream& rStream )
{
    BOOL bRet = FALSE;
    if( pStrm )
        delete pStrm, pStrm = 0;

    ByteString sLine, sVersion;
    ULONG nStt = 0, nEnd = 0;
    USHORT nIndex = 0;

    rStream.Seek(STREAM_SEEK_TO_BEGIN);
    rStream.ResetError();

    if( rStream.ReadLine( sLine ) &&
        sLine.GetToken( 0, ':', nIndex ) == "Version" )
    {
        sVersion = sLine.Copy( nIndex );
        while( rStream.ReadLine( sLine ) )
        {
            nIndex = 0;
            ByteString sTmp( sLine.GetToken( 0, ':', nIndex ) );
            if( sTmp == "StartHTML" )
                nStt = (ULONG)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "EndHTML" )
                nEnd = (ULONG)(sLine.Erase( 0, nIndex ).ToInt32());
            else if( sTmp == "SourceURL" )
                sBaseURL = String(S2U(sLine.Erase( 0, nIndex )));

            if( nEnd && nStt &&
                ( sBaseURL.Len() || rStream.Tell() >= nStt ))
            {
                bRet = TRUE;
                break;
            }
        }
    }

    if( bRet )
    {
        rStream.Seek( nStt );

        pStrm = new SvCacheStream( ( nEnd - nStt < 0x10000l
                                        ? nEnd - nStt + 32
                                        : 0 ));
        *pStrm << rStream;
        pStrm->SetStreamSize( nEnd - nStt );
        pStrm->Seek( STREAM_SEEK_TO_BEGIN );
    }

    return pStrm;
}

