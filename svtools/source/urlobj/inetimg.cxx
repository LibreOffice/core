/*************************************************************************
 *
 *  $RCSfile: inetimg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-27 13:58:34 $
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
#ifndef _SOT_DTRANS_HXX
#include <sot/dtrans.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#pragma hdrstop

#include "inetimg.hxx"

// Die folgende Struktur wurde den Netscape-Sourcen entnommen:
// netscape/source/ns/cmd/winfe/feimages.cpp
struct ImageData_Impl {
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
};

#define TOKEN_SEPARATOR '\001'
//=======================================================================

String INetImage::CopyExchange() const
{
    String aString;
    aString += aImageURL;
    aString += TOKEN_SEPARATOR;
    aString += aTargetURL;
    aString += TOKEN_SEPARATOR;
    aString += aTargetFrame;
    aString += TOKEN_SEPARATOR;
    aString += aAlternateText;
    aString += TOKEN_SEPARATOR;
    aString += String::CreateFromInt32( aSizePixel.Width() );
    aString += TOKEN_SEPARATOR;
    aString += String::CreateFromInt32( aSizePixel.Height() );
    return aString;
}

//-----------------------------------------------------------------------

void INetImage::PasteExchange( const String& rString )
{
    xub_StrLen nStart = 0;
    aImageURL = rString.GetToken( 0, TOKEN_SEPARATOR, nStart );
    aTargetURL = rString.GetToken( 0, TOKEN_SEPARATOR, nStart );
    aTargetFrame = rString.GetToken( 0, TOKEN_SEPARATOR, nStart );
    aAlternateText = rString.GetToken( 0, TOKEN_SEPARATOR, nStart );
    aSizePixel.Width() = rString.GetToken( 0, TOKEN_SEPARATOR, nStart ).ToInt32();
    aSizePixel.Height() = rString.GetToken( 0, TOKEN_SEPARATOR, nStart ).ToInt32();
}

//-----------------------------------------------------------------------

ULONG INetImage::HasFormat( SotDataObject& rObj )
{
    ULONG nFmt;
    const SvDataTypeList& rTypeLst = rObj.GetTypeList();
    if( !rTypeLst.Get( nFmt = SOT_FORMATSTR_ID_INET_IMAGE ) &&
        !rTypeLst.Get( nFmt = SOT_FORMATSTR_ID_NETSCAPE_IMAGE ))
        nFmt = 0;
    return nFmt;
}

//-----------------------------------------------------------------------

BOOL INetImage::SetData( SvData& rData ) const

/*  [Beschreibung]

    Liefert TRUE, falls in dem SvDataObject ein entsp. Image-Format
    steht, das wir unterstuetzen. Wenn ja, werden unsere Daten ins
    Data-Object kopiert.
*/
{
    BOOL bOK = FALSE;
    const ULONG lFormat = rData.GetFormat();

    if( lFormat == SOT_FORMATSTR_ID_INET_IMAGE )
    {
        rData.SetData( CopyExchange() );
        bOK = TRUE;
    }
    else if( lFormat == SOT_FORMATSTR_ID_NETSCAPE_IMAGE )
    {
        // Netscape koennen wir nicht exportieren
    }
    return bOK;
}

//-----------------------------------------------------------------------

BOOL INetImage::Copy( SotDataMemberObject& rObj ) const

/*  [Beschreibung]
    Anmerkung: Das Netscape-Image-Format koennen wir nicht exportieren
*/
{
    SvData* pData = new SvData( SOT_FORMATSTR_ID_INET_IMAGE, MEDIUM_MEMORY );
    ((INetImage*)this)->SetData( *pData );
    rObj.Append( pData );
    return TRUE;
}

//-----------------------------------------------------------------------

BOOL INetImage::Paste( SotDataObject& rObj, ULONG nFormat )

/*  [Beschreibung]

    Diese Methode "uberschreibt den Wert dieser Instanz mit dem
    Inhalt des uebergebenen Datenobjects.
    nFormat: Typischerweise das Ergebnis von HasFormat
*/

{
    BOOL bRet = FALSE;

    SvData aData( nFormat, MEDIUM_ALL );
    if( rObj.GetData( &aData ) )
    {
        void *pData;
        aData.GetData( &pData, TRANSFER_REFERENCE );
        UINT32 nLen = aData.GetMinMemorySize();
        rtl_TextEncoding eSysCSet = gsl_getSystemTextEncoding();

        if( nFormat == SOT_FORMATSTR_ID_INET_IMAGE )
        {
            PasteExchange( String( (sal_Char*)pData, eSysCSet ) );
            bRet = TRUE;
        }
        else if( nFormat == SOT_FORMATSTR_ID_NETSCAPE_IMAGE )
        {
            ImageData_Impl* pImgData = (ImageData_Impl*)pData;

            const sal_Char* pStart = (const sal_Char*)pImgData;
            aImageURL = String( (const sal_Char*)&(pImgData->pImageURL[0]),
                                eSysCSet );
            if( pImgData->iAltOffset )
                aAlternateText = String( (pStart + pImgData->iAltOffset), eSysCSet );
            if( pImgData->iAnchorOffset )
                aTargetURL = String( (pStart + pImgData->iAnchorOffset), eSysCSet );

            aSizePixel.Width() = pImgData->iWidth;
            aSizePixel.Height() = pImgData->iHeight;
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL INetImage::CopyFormats( SotDataObject& rObj ) const
{
    SvDataTypeList& rTypeList = (SvDataTypeList&)rObj.GetTypeList();
    rTypeList.Append( SvDataType( SOT_FORMATSTR_ID_INET_IMAGE, MEDIUM_MEMORY ) );
    // Netscape koennen wir nicht exportieren
    // rTypeList.Append( SvDataType( SOT_FORMATSTR_ID_NETSCAPE_IMAGE, MEDIUM_MEMORY ) );
    return TRUE;
}

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
    --> structure size
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
            int nLen;
            ByteString sData;

/*
            rIStm >> nLen;
            rIStm.SeekRel( -sizeof( int ));
            sal_Char* pBuf = new sal_Char[ nLen ];
            rIStm.Read( pBuf, nLen );
            ImageData_Impl* pImgData = (ImageData_Impl*)pBuf;

            const sal_Char* pStart = (const sal_Char*)pImgData;
            aImageURL = String( (const sal_Char*)&(pImgData->pImageURL[0]),
                                eSysCSet );
            if( pImgData->iAltOffset )
                aAlternateText = String( (pStart + pImgData->iAltOffset), eSysCSet );
            if( pImgData->iAnchorOffset )
                aTargetURL = String( (pStart + pImgData->iAnchorOffset), eSysCSet );

            aSizePixel.Width() = pImgData->iWidth;
            aSizePixel.Height() = pImgData->iHeight;
            delete pBuf;
*/

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
/**/
            bRet = 0 == rIStm.GetError();
        }
        break;
    }
    return bRet;
}

