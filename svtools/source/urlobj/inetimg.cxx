/*************************************************************************
 *
 *  $RCSfile: inetimg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:31:17 $
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
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef GCC
#pragma hdrstop
#endif

#include "inetimg.hxx"

#define TOKEN_SEPARATOR '\001'

sal_Bool INetImage::Write( SvStream& rOStm, ULONG nFormat ) const
{
    sal_Bool bRet = sal_False;
    switch( nFormat )
    {
    case SOT_FORMATSTR_ID_INET_IMAGE:
        {
            String sString;
            (sString += aImageURL ) += TOKEN_SEPARATOR;
            (sString += aTargetURL ) += TOKEN_SEPARATOR;
            (sString += aTargetFrame ) += TOKEN_SEPARATOR;
            (sString += aAlternateText ) += TOKEN_SEPARATOR;
            sString += String::CreateFromInt32( aSizePixel.Width() );
            sString += TOKEN_SEPARATOR;
            sString += String::CreateFromInt32( aSizePixel.Height() );
            ByteString sOut( sString, RTL_TEXTENCODING_UTF8 );

            rOStm.Write( sOut.GetBuffer(), sOut.Len() );
            static const sal_Char aEndChar[2] = { 0 };
            rOStm.Write( aEndChar, sizeof( aEndChar ));
            bRet = 0 == rOStm.GetError();
        }
        break;

    case SOT_FORMATSTR_ID_NETSCAPE_IMAGE:
        break;
    }
    return bRet;
}

sal_Bool INetImage::Read( SvStream& rIStm, ULONG nFormat )
{
    sal_Bool bRet = sal_False;
    switch( nFormat )
    {
    case SOT_FORMATSTR_ID_INET_IMAGE:
        {
            String sINetImg;
            rIStm.ReadCString( sINetImg, RTL_TEXTENCODING_UTF8 );
            xub_StrLen nStart = 0;
            aImageURL = sINetImg.GetToken( 0, TOKEN_SEPARATOR, nStart );
            aTargetURL = sINetImg.GetToken( 0, TOKEN_SEPARATOR, nStart );
            aTargetFrame = sINetImg.GetToken( 0, TOKEN_SEPARATOR, nStart );
            aAlternateText = sINetImg.GetToken( 0, TOKEN_SEPARATOR, nStart );
            aSizePixel.Width() = sINetImg.GetToken( 0, TOKEN_SEPARATOR,
                                                    nStart ).ToInt32();
            aSizePixel.Height() = sINetImg.GetToken( 0, TOKEN_SEPARATOR,
                                                    nStart ).ToInt32();
            bRet = 0 != sINetImg.Len();
        }
        break;

    case SOT_FORMATSTR_ID_NETSCAPE_IMAGE:
        {
/*
    --> structure size  MUST - alignment of 4!
    int     iSize;              // size of all data, including variable length strings
    BOOL    bIsMap;             // For server side maps
    INT32   iWidth;             // Fixed size data correspond to fields in LO_ImageDataStruct
    INT32   iHeight;            //   and EDT_ImageData
    INT32   iHSpace;
    INT32   iVSpace;
    INT32   iBorder;
    int     iLowResOffset;      // Offsets into string_data. If 0, string is NULL (not used)
    int     iAltOffset;         // (alternate text?)
    int     iAnchorOffset;      // HREF in image
    int     iExtraHTML_Offset;  // Extra HTML (stored in CImageElement)
    sal_Char pImageURL[1];      // Append all variable-length strings starting here
*/
            rtl_TextEncoding eSysCSet = gsl_getSystemTextEncoding();
            sal_Int32 nVal, nAnchorOffset, nAltOffset, nFilePos;
            ByteString sData;

            nFilePos = rIStm.Tell();
            // skip over iSize (int), bIsMao ( BOOL ) alignment of 4 !!!!
            rIStm.SeekRel( 8 );
            rIStm >> nVal;  aSizePixel.Width() = nVal;
            rIStm >> nVal;  aSizePixel.Height() = nVal;
            // skip over iHSpace, iVSpace, iBorder, iLowResOffset
            rIStm.SeekRel( 3 * sizeof( INT32 ) + sizeof( int ) );
            rIStm >> nAltOffset;
            rIStm >> nAnchorOffset;
            // skip over iExtraHTML_Offset
            rIStm.SeekRel( sizeof( int ) );

            rIStm.ReadCString( aImageURL, eSysCSet );
            if( nAltOffset )
            {
                rIStm.Seek( nFilePos + nAltOffset );
                rIStm.ReadCString( aAlternateText, eSysCSet );
            }
            else if( aAlternateText.Len() )
                aAlternateText.Erase();

            if( nAnchorOffset )
            {
                rIStm.Seek( nFilePos + nAnchorOffset );
                rIStm.ReadCString( aTargetURL, eSysCSet );
            }
            else if( aTargetURL.Len() )
                aTargetURL.Erase();

            bRet = 0 == rIStm.GetError();
        }
        break;
    }
    return bRet;
}

