/*************************************************************************
 *
 *  $RCSfile: mieclip.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:28 $
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

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
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

BOOL MSE40HTMLClipFormatObj::GetData( SotDataObject& rObj )
{
    SvData aData( SOT_FORMATSTR_ID_HTML_SIMPLE );
    if( rObj.GetData( &aData ) )
        return GetData( aData );
    return FALSE;
}

BOOL MSE40HTMLClipFormatObj::GetData( SvData& rData )
{
    BOOL bRet = FALSE;
    SvStorageStreamRef xStrm;
    rData.GetData( (SvStorageStreamRef&)xStrm );
    if( xStrm.Is() )
    {
        ByteString sLine, sVersion;
        ULONG nStt = 0, nEnd = 0;
        USHORT nIndex = 0;

        xStrm->Seek(STREAM_SEEK_TO_BEGIN);
        xStrm->ResetError();

        if( xStrm->ReadLine( sLine ) &&
            sLine.GetToken( 0, ':', nIndex ) == "Version" )
        {
            sVersion = sLine.Copy( nIndex );
            while( xStrm->ReadLine( sLine ) )
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
                    ( sBaseURL.Len() || xStrm->Tell() >= nStt ))
                {
                    bRet = TRUE;
                    break;
                }
            }
        }

        if( bRet )
        {
            xStrm->Seek( nStt );

            pStrm = new SvCacheStream( ( nEnd - nStt < 0x10000l
                                            ? nEnd - nStt + 32
                                            : 0 ));
            *pStrm << *xStrm;
            pStrm->SetStreamSize( nEnd - nStt );
            pStrm->Seek( STREAM_SEEK_TO_BEGIN );

            bRet = TRUE;
        }
    }
    return bRet;
}


////////////////////////////////////////////////////////////////////////////////
//
/* $Log: not supported by cvs2svn $
/* Revision 1.9  2000/09/17 16:47:18  willem.vandorp
/* OpenOffice header added.
/*
/* Revision 1.8  2000/09/06 14:49:46  willem.vandorp
/* Header and footer replaced.
/*
/* Revision 1.7  2000/05/02 09:16:06  as
/* unicode changes
/*
/* Revision 1.6  2000/03/02 19:08:47  jp
/* Bug #73756#: GetData - Base URL may not be exist
/*
/* Revision 1.5  1999/12/16 19:33:27  er
/* #60614# add: GetData with SvData
/*
/* Revision 1.4  1999/01/18 13:36:12  JP
/* Task #59398#: unnoetiges RegisterClipb... gegen IDS ausgetauscht
/*

      Rev 1.3   18 Jan 1999 14:36:12   JP
   Task #59398#: unnoetiges RegisterClipb... gegen IDS ausgetauscht

      Rev 1.2   23 Nov 1998 12:14:26   MBA
   Fix #56371#: Umstellung TF_ONE51

      Rev 1.1   22 Jun 1998 21:20:44   JP
   SvDataObject gegen SorDataObject ausgetauscht

      Rev 1.0   18 Feb 1998 17:28:08   OK
   NEW: MSE40HTMLClipFormatObj

*/

