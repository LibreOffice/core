/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            sString += OUString::number( aSizePixel.Width() );
            sString += TOKEN_SEPARATOR;
            sString += OUString::number( aSizePixel.Height() );

            rtl::OString sOut(rtl::OUStringToOString(sString,
                RTL_TEXTENCODING_UTF8));

            rOStm.Write(sOut.getStr(), sOut.getLength());
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
            String sINetImg = read_zeroTerminated_uInt8s_ToOUString(rIStm, RTL_TEXTENCODING_UTF8);
            sal_Int32 nStart = 0;
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
            rtl_TextEncoding eSysCSet = osl_getThreadTextEncoding();
            sal_Int32 nVal, nAnchorOffset, nAltOffset, nFilePos;

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

            aImageURL = read_zeroTerminated_uInt8s_ToOUString(rIStm, eSysCSet);
            if( nAltOffset )
            {
                rIStm.Seek( nFilePos + nAltOffset );
                aAlternateText = read_zeroTerminated_uInt8s_ToOUString(rIStm, eSysCSet);
            }
            else if( aAlternateText.Len() )
                aAlternateText.Erase();

            if( nAnchorOffset )
            {
                rIStm.Seek( nFilePos + nAnchorOffset );
                aTargetURL = read_zeroTerminated_uInt8s_ToOUString(rIStm, eSysCSet);
            }
            else if( aTargetURL.Len() )
                aTargetURL.Erase();

            bRet = 0 == rIStm.GetError();
        }
        break;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
