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
#include <tools/debug.hxx>
#include <sot/formats.hxx>
#include <tools/stream.hxx>

#include <svtools/inetimg.hxx>

#define TOKEN_SEPARATOR '\001'

sal_Bool INetImage::Write( SvStream& rOStm, sal_uLong nFormat ) const
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

sal_Bool INetImage::Read( SvStream& rIStm, sal_uLong nFormat )
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
    sal_Bool    bIsMap;             // For server side maps
    sal_Int32   iWidth;             // Fixed size data correspond to fields in LO_ImageDataStruct
    sal_Int32   iHeight;            //   and EDT_ImageData
    sal_Int32   iHSpace;
    sal_Int32   iVSpace;
    sal_Int32   iBorder;
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
            // skip over iSize (int), bIsMao ( sal_Bool ) alignment of 4 !!!!
            rIStm.SeekRel( 8 );
            rIStm >> nVal;  aSizePixel.Width() = nVal;
            rIStm >> nVal;  aSizePixel.Height() = nVal;
            // skip over iHSpace, iVSpace, iBorder, iLowResOffset
            rIStm.SeekRel( 3 * sizeof( sal_Int32 ) + sizeof( int ) );
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

