/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/tenccvt.hxx>
#include <osl/thread.h>

#include <vcl/virdev.hxx>
#include <tools/stream.hxx>
#include "starmath.hrc"
#include "utility.hxx"

namespace binfilter {


////////////////////////////////////////

/*N*/ SvStream& operator << (SvStream& rStream, const SmFace& rFont)
/*N*/ {
/*N*/ 	rStream.WriteByteString(ExportString(rFont.GetName()));
/*N*/ 	rStream << (sal_uInt32)rFont.GetFamily();
/*N*/     rStream << (sal_uInt32)GetSOStoreTextEncoding( rFont.GetCharSet() );
/*N*/ 	rStream << (sal_uInt32)rFont.GetWeight();
/*N*/ 	rStream << (sal_uInt32)rFont.GetItalic();
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

/*N*/ SvStream& operator >> (SvStream& rStream, SmFace& rFont)
/*N*/ {
/*N*/ 	sal_uInt32 nData;
/*N*/     ByteString  aByteStr;
/*N*/ 
/*N*/     rStream.ReadByteString( aByteStr );
/*N*/     rFont.SetName( ImportString( aByteStr ) );
/*N*/ 	rStream >> nData;
/*N*/ 	rFont.SetFamily((FontFamily)nData);
/*N*/ 	rStream >> nData;
/*N*/     rFont.SetCharSet( GetSOLoadTextEncoding( (rtl_TextEncoding) nData ) );
/*N*/ 	rStream >> nData;
/*N*/ 	rFont.SetWeight((FontWeight)nData);
/*N*/ 	rStream >> nData;
/*N*/ 	rFont.SetItalic((FontItalic)nData);
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

/*?*/ void ReadSM20Font(SvStream& rStream, Font& rFont)
/*?*/ {
/*?*/ 	BOOL	bData;
/*?*/ 	sal_uInt32 nData;
/*?*/     ByteString  aByteStr;
/*?*/ 
/*?*/     rStream.ReadByteString( aByteStr );
/*?*/     rFont.SetName( ImportString( aByteStr ) );
/*?*/ 	rStream >> nData;
/*?*/ 	rFont.SetFamily((FontFamily)nData);
/*?*/ 	rStream >> nData;
/*?*/     rFont.SetCharSet( GetSOLoadTextEncoding( (rtl_TextEncoding) nData ) );
/*?*/ 	rStream >> nData;
/*?*/ 	rFont.SetWeight((FontWeight)nData);
/*?*/ 	rStream >> bData;
/*?*/ 	if (bData)
/*?*/ 		rFont.SetItalic(ITALIC_NORMAL);
/*?*/ 	else
/*?*/ 		rFont.SetItalic(ITALIC_NONE);
/*?*/ }


////////////////////////////////////////////////////////////
//
// SmRectCache
//


/*N*/ SmRectCache::Key::Key(const XubString &rText, const Font &rFont)
/*N*/ {
/*N*/ }

/*N*/ SmRectCache::SmRectCache()
/*N*/ {
/*N*/ 	pVirDev = 0;
/*N*/ }


/*N*/ SmRectCache::~SmRectCache()
/*N*/ {
/*N*/ 	delete pVirDev;
/*N*/ }


/*N*/ const SmRect * SmRectCache::Search(const Key &rKey) const
/*N*/ {
/*N*/ 	return 0;
/*N*/ }


/*N*/ const SmRect * SmRectCache::Add(const Key &rKey, const SmRect &rRect)
/*N*/ {
/*N*/ 	return (const SmRect *)-1;
/*N*/ }


/*N*/ VirtualDevice * SmRectCache::GetVirDev()
/*N*/ {
/*N*/ 	if (!pVirDev)
/*N*/ 	{
             pVirDev = new VirtualDevice;
/*N*/ 		pVirDev->SetMapMode( MapMode(MAP_100TH_MM) );
/*N*/ 	}
/*N*/ 	DBG_ASSERT(pVirDev->GetMapMode().GetMapUnit() == MAP_100TH_MM,
/*N*/ 			   "Sm : falscher MapMode");
/*N*/ 
/*N*/ 	return pVirDev;
/*N*/ }


/*N*/ void SmRectCache::Reset()
/*N*/ {
/*N*/ }


////////////////////////////////////////////////////////////

#define TE_UCS2     "UCS2"
#define PRE_TE      "<?"
#define POST_TE     ")>"


ByteString ConvertUnknownCharacter(sal_Unicode ch)
{
    ByteString aString( RTL_CONSTASCII_STRINGPARAM( PRE_TE TE_UCS2 ) );
    aString.Append( "(" );
    aString += ByteString::CreateFromInt32(ch);
    aString += POST_TE;
    return aString;
}


/*N*/ const ByteString ExportString( const String& rString )
/*N*/ {
/*N*/ 	ByteString	aString;
/*N*/ 
/*N*/     rtl_TextEncoding nEnc = RTL_TEXTENCODING_MS_1252;
/*N*/ 	for (xub_StrLen i = 0; i < rString.Len(); i++)
/*N*/ 	{
/*N*/         sal_Unicode ch = rString.GetChar(i);
/*N*/ 		if ((ch != '\r') && (ch != '\n') && (ch != '\t'))
/*N*/ 		{
/*N*/             sal_Char cChar = ByteString::ConvertFromUnicode( ch, nEnc, FALSE );
/*N*/ 			if (cChar == 0)
/*?*/               aString += ConvertUnknownCharacter(ch);
/*N*/ 			else
/*N*/ 				aString += cChar;
/*N*/ 		}
/*N*/ 		else
/*N*/             aString += (sal_Char) ch;
/*N*/ 	}
/*N*/ 
/*N*/ 	aString.ConvertLineEnd(LINEEND_CRLF);
/*N*/     return aString;
/*N*/ }

#define TEXTENCODINGTAB_LEN     12

static const struct
{
    const char         *pText;
    rtl_TextEncoding    nEnc;
} aTextEncodingTab[ TEXTENCODINGTAB_LEN ] = 
{
    { TE_UCS2,        RTL_TEXTENCODING_UCS2 },  // is RTL_TEXTENCODING_UNICODE in 6.0
    { "DONTKNOW",     RTL_TEXTENCODING_DONTKNOW },
    { "ANSI",         RTL_TEXTENCODING_MS_1252 },
    { "MAC",          RTL_TEXTENCODING_APPLE_ROMAN },
    { "PC437",        RTL_TEXTENCODING_IBM_437 }, 
    { "PC850",        RTL_TEXTENCODING_ASCII_US },
    { "PC860",        RTL_TEXTENCODING_IBM_860 },
    { "PC861",        RTL_TEXTENCODING_IBM_861 },
    { "PC863",        RTL_TEXTENCODING_IBM_863 },
    { "PC865",        RTL_TEXTENCODING_IBM_865 },
    { "SYSTEM",       RTL_TEXTENCODING_DONTKNOW },
    { "SYMBOL",       RTL_TEXTENCODING_SYMBOL }
};

int GetTextEncodingTabIndex( const String &rTxt, xub_StrLen nPos )
{
    int nRes = -1;
    for (int i = 0;  i < TEXTENCODINGTAB_LEN  &&  nRes == -1;  ++i)
    {
        if (nPos == rTxt.SearchAscii( aTextEncodingTab[i].pText , nPos ))
            nRes = i;
    }
    return nRes;
}

/*N*/ const String ImportString( const ByteString& rByteString )
/*N*/ {
/*N*/     String  aString( rByteString, RTL_TEXTENCODING_MS_1252 );
/*N*/     
/*N*/     const xub_StrLen nPreLen  = sizeof( PRE_TE ) - 1;
/*N*/     const xub_StrLen nPostLen = sizeof( POST_TE ) - 1;
/*N*/ 
/*N*/     xub_StrLen nPreStart = 0;
/*N*/     while( STRING_NOTFOUND != ( nPreStart = 
/*N*/                                     aString.SearchAscii( PRE_TE, nPreStart )) )
/*N*/     {
              if (aString.EqualsAscii( "<?>", nPreStart, 3 ))
              {
                  nPreStart += 3;	// restart look-up after current found position
                  continue;
              }

/*?*/         //
/*N*/         // convert 'unknown character' to unicode character
/*N*/         //
/*?*/         xub_StrLen nTeStart = nPreStart + nPreLen;
/*?*/         xub_StrLen nTeLen   = 0;
/*?*/         int nIdx = GetTextEncodingTabIndex( aString, nTeStart );
/*?*/         DBG_ASSERT( nIdx >= 0, "text-encoding is missing" );
/*?*/         rtl_TextEncoding nEnc = RTL_TEXTENCODING_DONTKNOW;
/*?*/         if (nIdx >= 0)
/*?*/         {
/*?*/             nEnc = aTextEncodingTab[ nIdx ].nEnc;
/*?*/             nTeLen = strlen( aTextEncodingTab[ nIdx ].pText );
/*?*/         }
/*?*/         if (RTL_TEXTENCODING_DONTKNOW == nEnc)
/*?*/             nEnc = osl_getThreadTextEncoding();
/*?*/         //
/*?*/         xub_StrLen nNumStart = nTeStart + nTeLen + 1, // +1 because of "("
/*?*/                    nReplLen;
/*?*/         xub_StrLen nPostStart = aString.SearchAscii( POST_TE, nNumStart );
/*?*/         String sRepl;
/*?*/         if( STRING_NOTFOUND != nPostStart )
/*?*/         {
/*?*/             INT32 nCharVal = aString.Copy( nNumStart, nPostStart - nNumStart ).ToInt32();
/*?*/             DBG_ASSERT( nCharVal != 0, "String -> Int32 failed ?" );
/*?*/             if (RTL_TEXTENCODING_UNICODE == nEnc)
/*?*/             {
/*?*/                 if (nCharVal)
/*?*/                     sRepl = (sal_Unicode) nCharVal;
/*?*/             }
/*?*/             else
/*?*/             {
/*?*/                 DBG_ASSERT( 0 <= nCharVal  &&  nCharVal <= 256, 
/*?*/                         "character value out of range" );
/*?*/                 sRepl = ByteString::ConvertToUnicode( nCharVal, nEnc );
/*?*/             }
/*?*/             DBG_ASSERT( sRepl.Len() || !nCharVal, "conversion failed" );
/*?*/             nReplLen = nPostStart + nPostLen - nPreStart;
/*?*/         }
/*?*/         else
/*?*/         {
/*?*/             DBG_ERROR( "import error: 'unknown character' delimiter missing" );
/*?*/             sRepl.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "<?>" ) );
/*?*/             nReplLen = nPreLen;
/*?*/         }
/*?*/ 
/*?*/         aString.Replace( nPreStart, nReplLen, sRepl );
/*?*/         nPreStart += sRepl.Len();
/*N*/     }
/*N*/ 
/*N*/     // in old 2.0 or 3.0 formulas the strings to be imported do have an 
/*N*/     // additional '\0' character at the end that gets removed here.
/*N*/     if (aString.Len())
/*N*/         aString.EraseTrailingChars( '\0' );
/*N*/     
/*N*/ 	aString.ConvertLineEnd();
/*N*/     return aString;
/*N*/ }

////////////////////////////////////////////////////////////


/**************************************************************************/

/*N*/ SmPickList::SmPickList(USHORT nInitSize, USHORT nMaxSize) :
/*N*/ 	SfxPtrArr((BYTE) nInitSize, 1)
/*N*/ {
/*N*/ 	nSize = nMaxSize;
/*N*/ }


/*N*/ SmPickList::~SmPickList()
/*N*/ {
/*N*/ 	Clear();
/*N*/ }


/*N*/ void SmPickList::Clear()
/*N*/ {
/*N*/ 	USHORT	nPos;
/*N*/ 
/*N*/ 	for (nPos = 0; nPos < Count(); nPos++)
/*?*/ 		DestroyItem(GetPtr(nPos));
/*N*/ 
/*N*/ 	RemovePtr(0, Count());
/*N*/ }

/**************************************************************************/
/**************************************************************************/

/*?*/ void * SmFontPickList::CreateItem(const String& rString)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	return new Font();
/*?*/ }

/*?*/ void * SmFontPickList::CreateItem(const void *pItem)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 	return new Font(*((Font *) pItem));
/*?*/ }

/*?*/ void SmFontPickList::DestroyItem(void *pItem)
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	delete (Font *)pItem;
/*?*/ }

/*?*/ BOOL SmFontPickList::CompareItem(const void *pFirstItem, const void *pSecondItem) const
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 	Font	*pFirstFont, *pSecondFont;
/*?*/ }

/*?*/ String SmFontPickList::GetStringItem(void *pItem)
/*?*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); String	aString; return aString;//STRIP001 	Font   *pFont;
/*?*/ }

/*?*/ void SmFontPickList::LoadItem(SvStream& rStream, void *pItem)
/*?*/ {
/*?*/	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	rStream >> *((Font *)pItem);
/*?*/ }

/*?*/ void SmFontPickList::SaveItem(SvStream& rStream, const void *pItem) const
/*?*/ {
/*?*/	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	rStream << *(const Font *) pItem;
/*?*/ }

////////////////////////////////////////

/*N*/ void SmFace::Impl_Init()
/*N*/ {
/*N*/     SetSize( GetSize() );
/*N*/     SetTransparent( TRUE );
/*N*/     SetAlign( ALIGN_BASELINE );
/*N*/     SetColor( COL_AUTO );
/*N*/ }

/*N*/ void SmFace::SetSize(const Size& rSize)
/*N*/ {
/*N*/ 	Size  aSize (rSize);
/*N*/ 
/*N*/ 	// check the requested size against minimum value
/*N*/ 	static int __READONLY_DATA	nMinVal = SmPtsTo100th_mm(2);
/*N*/ 
/*N*/ 	if (aSize.Height() < nMinVal)
/*N*/ 		aSize.Height() = nMinVal;
/*N*/ 
/*N*/ 	//! we don't force a maximum value here because this may prevent eg the
/*N*/ 	//! parentheses in "left ( ... right )" from matching up with large
/*N*/ 	//! bodies (eg stack{...} with many entries).
/*N*/ 	//! Of course this is holds only if characters are used and not polygons.
/*N*/ 
/*N*/ 	Font::SetSize(aSize);
/*N*/ }


/*N*/ long SmFace::GetBorderWidth() const
/*N*/ {
/*N*/ 	if (nBorderWidth < 0)
/*N*/ 		return GetDefaultBorderWidth();
/*N*/ 	else
/*N*/ 		return nBorderWidth;
/*N*/ }

/*N*/ SmFace & SmFace::operator = (const SmFace &rFace)
/*N*/ {
/*N*/ 	Font::operator = (rFace);
/*N*/ 	nBorderWidth = -1;
/*N*/ 	return *this;
/*N*/ }


/*N*/ SmFace & operator *= (SmFace &rFace, const Fraction &rFrac)
/*N*/ 	// scales the width and height of 'rFace' by 'rFrac' and returns a
/*N*/ 	// reference to 'rFace'.
/*N*/ 	// It's main use is to make scaling fonts look easier.
/*N*/ {	const Size &rFaceSize = rFace.GetSize();
/*N*/ 
/*N*/ 	rFace.SetSize(Size(Fraction(rFaceSize.Width())	*= rFrac,
/*N*/ 					   Fraction(rFaceSize.Height()) *= rFrac));
/*N*/ 	return rFace;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
