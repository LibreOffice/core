/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hdu $ $Date: 2001-02-16 14:14:38 $
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

#define _SV_SALGDI3_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#if !( defined(FREEBSD) || defined(NETBSD) )
#include <alloca.h>
#endif

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#if !defined(USE_PSPRINT) && !defined(PRINTER_DUMMY) && !defined(_USE_PRINT_EXTENSION_)
#define Font XLIB_Font
#define Region XLIB_Region
#include <xprinter/xp.h>
#undef Font
#undef Region
#endif

#if defined(USE_PSPRINT)
#include <psprint/printergfx.hxx>
#include <psprint/fontmanager.hxx>
#include <psprint/jobdata.hxx>
#endif

#ifndef ANSI1252_HXX_
#include "ansi1252.hxx"
#endif
#ifndef XLFD_ATTRIBUTE_HXX
#include "xlfd_attr.hxx"
#endif
#ifndef XLFD_SIMPLE_HXX
#include "xlfd_smpl.hxx"
#endif
#ifndef XLFD_EXTENDED_HXX
#include "xlfd_extd.hxx"
#endif
#ifndef SAL_CONVERTER_CACHE_HXX_
#include "salcvt.hxx"
#endif
#include <osl/types.h>

#ifdef USE_BUILTIN_RASTERIZER
#include <gcach_xpeer.hxx>
#endif // USE_BUILTIN_RASTERIZER

// -----------------------------------------------------------------------

#ifdef USE_BUILTIN_RASTERIZER
static X11GlyphPeer aX11GlyphPeer;
//### HACK, the clip region is needed in lower levels now
// because XRender reimplements GC clipping but cannot
// be satisfied with a GC
static XLIB_Region tmpClipRegion = 0;
#endif // USE_BUILTIN_RASTERIZER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalGraphicsData::FaxPhoneComment( const sal_Unicode* pStr, USHORT nLen ) const
{
#ifdef USE_PSPRINT
    if( ! m_pPhoneNr )
        return;
#else
    if( ! bPrinter_ )
        return;
#endif

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"

    USHORT nPos;
    ByteString aPhone( pStr, nLen, gsl_getSystemTextEncoding() );

    static ByteString aPhoneNumber;
    static BOOL   bIsCollecting = FALSE;

    if( ! bIsCollecting )
    {
        if( ( nPos = aPhone.Search( FAX_PHONE_TOKEN ) ) != STRING_NOTFOUND )
        {
            aPhone.Erase( 0, nPos + FAX_PHONE_TOKEN_LENGTH );
            bIsCollecting = TRUE;
            aPhoneNumber.Erase();
        }
    }
    if( bIsCollecting )
    {
        if( ( nPos = aPhone.Search( FAX_END_TOKEN ) ) != STRING_NOTFOUND )
        {
            aPhone.Erase( nPos );
            bIsCollecting = FALSE;
        }
        aPhoneNumber += aPhone;
        if( ! bIsCollecting )
        {
#ifndef PRINTER_DUMMY
#ifndef USE_PSPRINT
            aPhone = "PhoneNumber(";
            aPhone += aPhoneNumber;
            aPhone += ")\n";
#if !defined(_USE_PRINT_EXTENSION_)
            XpPSComment( GetDisplay()->GetDisplay(), aPhone.GetBuffer() );
#endif
#else
            *m_pPhoneNr = String( aPhoneNumber, gsl_getSystemTextEncoding() );
#endif
#endif
            aPhoneNumber = ByteString();
        }
    }
    if( aPhoneNumber.Len() > 1024 )
    {
        bIsCollecting = FALSE;
        aPhoneNumber = ByteString();
    }
}

// ----------------------------------------------------------------------------
//
// SalDisplay
//
// ----------------------------------------------------------------------------

XlfdStorage*
SalDisplay::GetXlfdList()
{
    if ( mpFontList != NULL )
    {
        return mpFontList;
    }
    else
    {
        // on a display an xlfd of *-0-0-75-75-* means this is a scalable
        // bitmap font, thus it is ugly and thus to avoid. On a printer
        // *-0-0-300-300-* means this is a printer resident font thus nice
        // thus to prefer :-(
        eDeviceT eDevice = this->IsDisplay() ? eDeviceDisplay : eDevicePrinter;

        mpFactory  = new AttributeProvider( eDevice );
        mpFontList = new XlfdStorage();

        int i, nFontCount;
        const int nMaxCount  = 64 * 1024 - 1;
        Display *pDisplay = GetDisplay();
        char **ppFontList = XListFonts(pDisplay, "-*", nMaxCount, &nFontCount);

        //
        // create a list of simple Xlfd font information
        //

        Xlfd  *pXlfdList = (Xlfd*)malloc( nFontCount * sizeof(Xlfd) );
        int    nXlfdCount = 0;

        for ( i = 0; i < nFontCount; i++ )
        {
            if ( pXlfdList[ nXlfdCount ].FromString(ppFontList[i], mpFactory) )
                ++nXlfdCount;
        }

        XFreeFontNames( ppFontList );

        // classification information is needed for sorting, classification
        // of charset (i.e. iso8859-1 <-> ansi-1252) depends on wether the
        // display points to a printer or to a real display. On a printer all
        // iso8859-1 fonts are really capable of ansi-1252
        mpFactory->AddClassification();
        // add some pretty print description
        mpFactory->AddAnnotation();
        // misc feature checking
        mpFactory->TagFeature();

        // sort according to font style
        qsort( pXlfdList, nXlfdCount, sizeof(Xlfd), XlfdCompare );

        //
        // create a font list with merged encoding information
        //

        BitmapXlfdStorage   aBitmapList;
        ScalableXlfd       *pScalableFont = NULL;
        PrinterFontXlfd    *pPrinterFont  = NULL;

        int nFrom = 0;
        for ( i = 0; i < nXlfdCount; i++ )
        {
            // exclude openlook glyph and cursor
            Attribute *pAttr = mpFactory->RetrieveFamily(pXlfdList[i].mnFamily);
            if ( pAttr->HasFeature(   XLFD_FEATURE_OL_GLYPH
                                    | XLFD_FEATURE_OL_CURSOR) )
            {
                continue;
            }
            // exclude fonts with unknown encoding
            if ( pXlfdList[i].GetEncoding() == RTL_TEXTENCODING_DONTKNOW )
            {
                continue;
            }
            // exclude "interface system" and "interface user" in bold
            if (pAttr->HasFeature( XLFD_FEATURE_APPLICATION_FONT ) )
            {
                continue;
            }

            Bool bSameOutline = pXlfdList[i].SameFontoutline(pXlfdList + nFrom);
            XlfdFonttype eType = pXlfdList[i].Fonttype();

            // flush the old merged font list if the name doesn't match any more
            if ( !bSameOutline )
            {
                mpFontList->Add( pScalableFont );
                mpFontList->Add( pPrinterFont );

                if (pAttr->HasFeature( XLFD_FEATURE_APPLICATION_FONT ) )
                    aBitmapList.AddScalableFont( pScalableFont );

                mpFontList->Add( &aBitmapList );

                pScalableFont = NULL;
                 pPrinterFont  = NULL;
                aBitmapList.Reset();
            }

            // merge the font or generate a new one
            switch( eType )
            {
                case eTypeScalable:

                    if ( pScalableFont == NULL )
                        pScalableFont = new ScalableXlfd;
                    pScalableFont->AddEncoding(pXlfdList + i);

                    break;

                case eTypeBitmap:

                    aBitmapList.AddBitmapFont( pXlfdList + i );

                    break;

                case eTypePrinterBuiltIn:
                case eTypePrinterDownload:

                    if ( pPrinterFont == NULL )
                        pPrinterFont = new PrinterFontXlfd;
                    pPrinterFont->AddEncoding( pXlfdList + i );

                    break;

                case eTypeScalableBitmap:
                default:

                    break;
            }

            nFrom = i;
        }

        // flush the merged list into the global list
        mpFontList->Add( pScalableFont );
        mpFontList->Add( pPrinterFont );
        mpFontList->Add( &aBitmapList );

        // create a font set for user interface
        mpFontList->InterfaceFont( mpFactory );

        // cleanup the list of simple font information
        if ( pXlfdList != NULL )
            free( pXlfdList );

        return mpFontList;
    }
}

// ---------------------------------------------------------------------------

ExtendedFontStruct*
SalDisplay::GetFont( ExtendedXlfd *pRequestedFont, int nPixelSize )
{
    if( !pFontCache_ )
    {
        mpCvtCache = new SalConverterCache;
        pFontCache_ = new SalFontCache( 64, 64, 16 ); // ???
    }
    else
    {
        ExtendedFontStruct *pItem;
        for ( pItem  = pFontCache_->First();
              pItem != NULL;
              pItem  = pFontCache_->Next() )
        {
            if ( pItem->Match(pRequestedFont, nPixelSize) )
            {
                if( pFontCache_->GetCurPos() )
                {
                    pFontCache_->Remove( pItem );
                    pFontCache_->Insert( pItem, 0UL );
                }
                return pItem;
            }
        }
    }

    // before we expand the cache, we look for very old and unused items
    if( pFontCache_->Count() >= 64 )
    {
        ExtendedFontStruct *pItem;
        for ( pItem = pFontCache_->Last();
              pItem != NULL;
              pItem = pFontCache_->Prev() )
        {
            if( 1 == pItem->GetRefCount() )
            {
                pFontCache_->Remove( pItem );
                pItem->ReleaseRef();

                if( pFontCache_->Count() < 64 )
                    break;
            }
        }

    }

    ExtendedFontStruct *pItem = new ExtendedFontStruct( GetDisplay(),
                                    nPixelSize, pRequestedFont, mpCvtCache );
    pFontCache_->Insert( pItem, 0UL );
    pItem->AddRef();

    return pItem;
}

// ---------------------------------------------------------------------------

void
SalDisplay::DestroyFontCache()
{
    if( pFontCache_ )
    {
        ExtendedFontStruct *pItem = pFontCache_->First();
        while( pItem )
        {
            delete pItem;
            pItem = pFontCache_->Next();
        }
        delete pFontCache_;
    }
    if( mpFontList )
    {
        mpFontList->Dispose();
        delete mpFontList;
    }
    if ( mpFactory )
    {
        delete mpFactory;
    }
    if ( mpCvtCache )
    {
        delete mpCvtCache;
    }

    pFontCache_ = (SalFontCache*)NULL;
    mpFontList = (XlfdStorage*)NULL;
    mpFactory  = (AttributeProvider*)NULL;
    mpCvtCache = (SalConverterCache*)NULL;
}

// ----------------------------------------------------------------------------
//
// SalGraphicsData
//
// ----------------------------------------------------------------------------

GC
SalGraphicsData::SelectFont()
{
    Display *pDisplay = GetXDisplay();

    if( !pFontGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = IncludeInferiors;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = True;
        values.foreground           = nTextPixel_;
#ifdef _USE_PRINT_EXTENSION_
                values.background = xColormap_->GetWhitePixel();
#endif
#ifdef _USE_PRINT_EXTENSION_
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                                GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCBackground | GCForeground,
                              &values );
    }
#else
            pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                                                            GCSubwindowMode | GCFillRule
                                                          | GCGraphicsExposures | GCForeground,
                                                          &values );
        }
#endif
    if( !bFontGC_ )
    {
        XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
        SetClipRegion( pFontGC_ );
        bFontGC_ = TRUE;
    }

    return pFontGC_;
}

//--------------------------------------------------------------------------

// Select the max size of a font, which is token for real
// This routine is (and should be) called only once, the result should be
// stored in some static variable

static int
GetMaxFontHeight()
{
    #define DEFAULT_MAXFONTHEIGHT 250

    int  nMaxFontHeight = 0;

    char *FontHeight = getenv ("SAL_MAXFONTHEIGHT");
    if (FontHeight)
        nMaxFontHeight = atoi (FontHeight);

    if (nMaxFontHeight <= 0)
        nMaxFontHeight = DEFAULT_MAXFONTHEIGHT;

    return nMaxFontHeight;
}

void
SalGraphicsData::SetFont( const ImplFontSelectData *pEntry )
{
    bFontGC_    = FALSE;
    xFont_      = NULL; // ->ReleaseRef()
    aScale_     = Fraction( 1, 1 );
    nFontOrientation_ = pEntry->mnOrientation;
    bFontVertical_  = pEntry->mbVertical;

#ifdef USE_BUILTIN_RASTERIZER
    if( mpServerSideFont != NULL )
    {
        // old server side font is no longer referenced
        GlyphCache::GetInstance().UncacheFont( *mpServerSideFont );
        mpServerSideFont = NULL;
    }
#endif //USE_BUILTIN_RASTERIZER

    if( pEntry->mpFontData && pEntry->mpFontData->mpSysData )
    {
#ifdef USE_BUILTIN_RASTERIZER
        if( pEntry->mpFontData->mpSysData == SERVERFONT_MAGIC )
        {
            // requesting a font provided by builtin rasterizer
            mpServerSideFont = GlyphCache::GetInstance().CacheFont( *pEntry );
#ifdef USE_PSPRINT
            if( m_pPrinterGfx != NULL && mpServerSideFont )
            {
                // we need to notify printergfx of the font change
                if( const ::rtl::OString* pOString = mpServerSideFont->GetFontFileName() )
                {
                    // we have a fontfile the printfontmanager should know about
                    ::rtl::OUString aFontFileName =  OStringToOUString( *pOString, RTL_TEXTENCODING_UNICODE );
                    int nFaceNum = mpServerSideFont->GetFontFaceNumber();
                    psp::PrintFontManager& rPSPFontManager = psp::PrintFontManager::get();
                    if( int nID = rPSPFontManager.addFontFile( aFontFileName, nFaceNum ) )
                        m_pPrinterGfx->SetFont( nID, pEntry->mnHeight, pEntry->mnOrientation );
                }
            }
#endif // USE_PSPRINT
            return;
        }
#endif //USE_BUILTIN_RASTERIZER

        ExtendedXlfd *pSysFont = (ExtendedXlfd*)pEntry->mpFontData->mpSysData;
        static int nMaxFontHeight = GetMaxFontHeight();

        USHORT         nH, nW;
        if( bWindow_ )
        {
            // see BugId #44528# FontWork (-> #45038#) and as well Bug #47127#
            if( pEntry->mnHeight > nMaxFontHeight )
                nH = nMaxFontHeight;
            else if( pEntry->mnHeight > 2 )
                nH = pEntry->mnHeight;
            else
                nH = 2;
            nW = 0; // pEntry->mnWidth;
        }
        else
        {
            nH = pEntry->mnHeight;
            nW = pEntry->mnWidth;
        }

        xFont_ = GetDisplay()->GetFont( pSysFont, nH );
        if( pEntry->mnHeight > nMaxFontHeight || pEntry->mnHeight < 2 )
            aScale_ = Fraction( pEntry->mnHeight, nH );
    }
    else
    {
        #ifdef DEBUG
        // XXX Fix me: provide a fallback for poor font installations
        // we may be reach this if no font matches the GUI font
        // MS Sans Serif;Geneva;Helv;WarpSans;Dialog;Lucida; ... */
        fprintf( stderr, "SalGraphicsData::SetFont: Invalid Font Selection\n" );
        #endif
    }
}

//--------------------------------------------------------------------------

static sal_Unicode
SwapBytes( const sal_Unicode nIn )
{
    return ((nIn >> 8) & 0x00ff) | ((nIn << 8) & 0xff00);
}


// draw string in a specific multibyte encoding
static void
ConvertTextItem16( XTextItem16* pTextItem,
        SalConverterCache* pCvt, rtl_TextEncoding nEncoding )
{
    if ( pTextItem && pTextItem->nchars > 0 )
    {
        // convert the string into the font encoding
        sal_Size  nSize;
        sal_Size  nBufferSize = pTextItem->nchars * 2;
        sal_Char *pBuffer = (sal_Char*)alloca( nBufferSize );

        nSize = ConvertStringUTF16( (sal_Unicode*)pTextItem->chars, pTextItem->nchars,
                        pBuffer, nBufferSize, pCvt->GetU2TConverter(nEncoding));

        sal_Char *pTextChars = (sal_Char*)pTextItem->chars;
        int n = 0, m = 0;

        if (   nEncoding == RTL_TEXTENCODING_GB_2312
            || nEncoding == RTL_TEXTENCODING_GBT_12345
            || nEncoding == RTL_TEXTENCODING_GBK
            || nEncoding == RTL_TEXTENCODING_BIG5 )
        {
            // GB and Big5 needs special treatment since chars can be single or
            // double byte: encoding is
            // [ 0x00 - 0x7f ] | [ 0x81 - 0xfe ] [ 0x40 - 0x7e 0x80 - 0xfe ]
            while ( n < nSize )
            {
                if ( (unsigned char)pBuffer[ n ] < 0x80 )
                {
                    pTextChars[ m++ ] = 0x0;
                    pTextChars[ m++ ] = pBuffer[ n++ ];
                }
                else
                {
                    pTextChars[ m++ ] = pBuffer[ n++ ];
                    pTextChars[ m++ ] = pBuffer[ n++ ];
                }
            }
            pTextItem->nchars = m / 2;
        }
        else
        if ( pCvt->IsSingleByteEncoding(nEncoding) )
        {
            // Single Byte encoding has to be padded
            while ( n < nSize )
            {
                pTextChars[ m++ ] = 0x0;
                pTextChars[ m++ ] = pBuffer[ n++ ];
            }
            pTextItem->nchars = nSize;
        }
        else
        {
            while ( n < nSize )
            {
                pTextChars[ m++ ] = pBuffer[ n++ ];
            }
            pTextItem->nchars = nSize / 2;
        }

        // XXX FIXME
        if (   (nEncoding == RTL_TEXTENCODING_GB_2312)
            || (nEncoding == RTL_TEXTENCODING_EUC_KR) )
        {
            for (int n_char = 0; n_char < m; n_char++ )
                pTextChars[ n_char ] &= 0x7F;
        }

    }
}

// XXX this is a hack since XPrinter is not multibyte capable
// XXX for printing this routine is called for each character
void
XPrinterDrawText16( Display* pDisplay, Drawable nDrawable, GC nGC,
        int nX, int nY, int nAngle, XTextItem16 *pTextItem16, int nItem )
{
    // convert XTextItem16 to XTextItem
    XTextItem *pTextItem = (XTextItem*)alloca( nItem * sizeof(XTextItem) );

    for ( int nCurItem = 0; nCurItem < nItem; nCurItem++ )
    {
        int      nChars      = pTextItem16[ nCurItem ].nchars;
        char*    pDstCharPtr = (char*)alloca( nChars * sizeof(char) );
        XChar2b* pSrcCharPtr = pTextItem16[ nCurItem ].chars;

        pTextItem[ nCurItem ].chars  = pDstCharPtr;
        pTextItem[ nCurItem ].nchars = nChars;
        pTextItem[ nCurItem ].delta  = pTextItem16[ nCurItem ].delta;
        pTextItem[ nCurItem ].font   = pTextItem16[ nCurItem ].font;

        for ( int nCurChar = 0;
              nCurChar < nChars;
              nCurChar++, pDstCharPtr++, pSrcCharPtr++ )
        {
            *pDstCharPtr = (char)pSrcCharPtr->byte2;
        }
    }

    if ( nAngle != 0 )
    {
        for ( int nCurItem = 0; nCurItem < nItem; nCurItem++ )
        {
            // XXX FIXME this is broken, because nX and nY is not sufficiently updated
            XSetFont( pDisplay, nGC, pTextItem[ nItem ].font );
#if !defined(USE_PSPRINT) && !defined(_USE_PRINT_EXTENSION_)
            if ( XSalCanDrawRotString(pDisplay, nGC) )
            {
                XSalDrawRotString( pDisplay, nDrawable, nGC, nX, nY,
                    pTextItem[ nCurItem ].chars, pTextItem[ nCurItem ].nchars, nAngle );
            }
            else
#endif
            {
                XDrawString( pDisplay, nDrawable, nGC, nX, nY,
                    pTextItem[ nCurItem ].chars, pTextItem[ nCurItem ].nchars );
            }
        }
    }
    else
    {
        XDrawText( pDisplay, nDrawable, nGC, nX, nY, pTextItem, nItem );
    }
}

// draw string vertically
static void
DrawVerticalString ( Display *pDisplay, Drawable nDrawable, GC nGC,
                     int nX, int nY, const sal_Unicode *pStr, int nLength,
                     ExtendedFontStruct *pFont )
{
    VerticalTextItem** pTextItems;
    int nNumItem = pFont->GetVerticalTextItems( pStr, nLength, RTL_TEXTENCODING_UNICODE,
                            pStr, pTextItems );

    for ( int nIdx = 0; nIdx < nNumItem; nIdx++ )
    {
        if ( pTextItems[nIdx]->mpXFontStruct == NULL )
            continue;

        XSetFont( pDisplay, nGC, pTextItems[nIdx]->mpXFontStruct->fid );
        for ( int nChar = 0; nChar < pTextItems[nIdx]->mnLength; nChar++ )
        {
            XDrawString16( pDisplay, nDrawable, nGC,
                       nX + pTextItems[nIdx]->mnTransX,
                       nY + pTextItems[nIdx]->mnTransY,
                       (XChar2b*)(pTextItems[nIdx]->mpString + nChar), 1 );
            nY += (pTextItems[nIdx]->mbFixed ?
                   pTextItems[nIdx]->mnFixedAdvance : pTextItems[nIdx]->mpAdvanceAry[nChar]);
        }
    }

    for (int nIdx2 = 0; nIdx2 < nNumItem; nIdx2++)
    {
        delete( pTextItems[nIdx2] );
    }
    free( pTextItems );
}

struct VTextItemExt
{
    rtl_TextEncoding mnEncoding;
    const sal_Unicode* mpString;
};

static void
DrawVerticalTextItem( Display *pDisplay, Drawable nDrawable, GC nGC,
              int nX, int nY, XTextItem16* pTextItem, int nItem,
              VTextItemExt* pVTextItemExt, ExtendedFontStruct* pFont )
{
    for ( int nItemIdx = 0; nItemIdx < nItem; nItemIdx++ )
    {
        VerticalTextItem** pVTextItems;
        int nNumItem = pFont->GetVerticalTextItems( pVTextItemExt[nItemIdx].mpString,
                                pTextItem[nItemIdx].nchars,
                                pVTextItemExt[nItemIdx].mnEncoding,
                                (sal_Unicode *)pTextItem[nItemIdx].chars,
                                pVTextItems );
        for ( int nIdx = 0; nIdx < nNumItem; nIdx++ )
        {
            if ( pVTextItems[nIdx]->mpXFontStruct == NULL )
                continue;

            XSetFont( pDisplay, nGC, pVTextItems[nIdx]->mpXFontStruct->fid );
            for (int nChar = 0; nChar < pVTextItems[nIdx]->mnLength; nChar++ )
            {
                XDrawString16( pDisplay, nDrawable, nGC,
                           nX + pVTextItems[nIdx]->mnTransX,
                           nY + pVTextItems[nIdx]->mnTransY,
                           (XChar2b*)(pVTextItems[nIdx]->mpString + nChar), 1 );
                nY += (pVTextItems[nIdx]->mbFixed ?
                       pVTextItems[nIdx]->mnFixedAdvance : pVTextItems[nIdx]->mpAdvanceAry[nChar]);
            }
        }
        for ( int nIdx2 = 0; nIdx2 < nNumItem; nIdx2++ )
        {
            delete( pVTextItems[nIdx2] );
        }
        free( pVTextItems );
    }
}

#ifdef USE_BUILTIN_RASTERIZER
#ifdef USE_XRENDER

static void DrawServerAAFontString( const ServerFont* pServerFont,
    Display *pDisplay, Drawable nDrawable,
    GC nGC, int nX, int nY, const sal_Unicode *pStr, int nLength )
{
    // translate unicode to glyph ids and make sure they are already on the server
     /*Glyph*/unsigned short* pGlyphString = (unsigned short*)alloca( 2*nLength );
    for( int i = 0; i < nLength; ++i )
    {
        const int nGlyphIndex = pServerFont->GetGlyphIndex( pStr[i] );
        pGlyphString[ i ] = aX11GlyphPeer.GetGlyphId( *pServerFont, nGlyphIndex );
    }

    Visual* pVisual = DefaultVisual( pDisplay, 0 );
    XRenderPictFormat*  pVisualFormat =  XRenderFindVisualFormat( pDisplay, pVisual );

    static Pixmap aPixmap = NULL;
    static Picture aSrc = NULL;
    if( !aPixmap )
    {
        int iDummy;
        unsigned uDummy, nDepth;
        XLIB_Window wDummy;
        XGetGeometry( pDisplay, nDrawable, &wDummy, &iDummy, &iDummy,
            &uDummy, &uDummy, &uDummy, &nDepth );
        aPixmap = XCreatePixmap( pDisplay, nDrawable, 1, 1, nDepth );

        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;
        aSrc = XRenderCreatePicture( pDisplay, aPixmap, pVisualFormat, CPRepeat, &aAttr );
    }

    // set foreground
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );
    aGCVal.clip_mask = None;
    GC tmpGC = XCreateGC( pDisplay, aPixmap, GCForeground | GCClipMask, &aGCVal );
    XDrawPoint( pDisplay, aPixmap, tmpGC, 0, 0 );
    XFreeGC( pDisplay, tmpGC );

    // notify xrender of target drawable
    XRenderPictureAttributes aAttr;
    Picture aDst = XRenderCreatePicture( pDisplay, nDrawable, pVisualFormat, 0, &aAttr );

    // set clipping
    if( tmpClipRegion && !XEmptyRegion( tmpClipRegion ) )
        XRenderSetPictureClipRegion( pDisplay, aDst, tmpClipRegion );

    // draw the string
    GlyphSet aGlyphSet = aX11GlyphPeer.GetGlyphSet( *pServerFont );
    XRenderCompositeString16( pDisplay, PictOpOver, aSrc, aDst,
        0, aGlyphSet, 0, 0, nX, nY, pGlyphString, nLength );

    // cleanup
    XRenderFreePicture( pDisplay, aDst );
}

#endif

static void DrawServerFontString( const ServerFont* pServerFont,
    Display *pDisplay, Drawable nDrawable,
    GC nGC, int nX, int nY, const sal_Unicode *pStr, int nLength )
{
#ifdef USE_XRENDER
    if( aX11GlyphPeer.GetGlyphSet( *pServerFont ) )
    {
        DrawServerAAFontString( pServerFont, pDisplay, nDrawable, nGC, nX, nY, pStr, nLength );
        return;
    }
#endif // USE_XRENDER

    Point aPos( nX, nY );

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    GC tmpGC = XCreateGC( pDisplay, nDrawable, GCFillStyle, &aGCVal );
    XCopyGC( pDisplay, nGC, ~GCFillStyle, tmpGC);

    for( int i = 0; i < nLength; ++i )
    {
        const int nGlyphIndex = pServerFont->GetGlyphIndex( pStr[i] );

        Pixmap const aStipple = aX11GlyphPeer.GetPixmap( *pServerFont, nGlyphIndex );

        const GlyphMetric& rGM  = pServerFont->GetGlyphMetric( nGlyphIndex );
        const int nDestX        = aPos.X() + rGM.GetOffset().X();
        const int nDestY        = aPos.Y() + rGM.GetOffset().Y();
        const int nWidth        = rGM.GetSize().Width();
        const int nHeight       = rGM.GetSize().Height();

        XSetStipple( pDisplay, tmpGC, aStipple );
        XSetTSOrigin( pDisplay, tmpGC, nDestX, nDestY );
        XFillRectangle( pDisplay, nDrawable, tmpGC, nDestX, nDestY, nWidth, nHeight );
#if 1
        aPos += rGM.GetDelta();
#else
        aPos = Point( nX, nY );
        aPos += pServerFont->TransformPoint( Point( pDXAry[i], 0 ) );
//###       aPos += Point( pDXAry[i], 0 );
#endif
    }

    XFreeGC( pDisplay, tmpGC );
}

#endif


// draw string in one of the fonts / encodings that are available in the
// extended font
static void
DrawUnicodeString( ServerFont* pServerFont, Display *pDisplay, Drawable nDrawable, GC nGC,
        int nX, int nY, const sal_Unicode *pStr, int nLength, int nAngle, BOOL bVertical,
        SalConverterCache *pCvt, ExtendedFontStruct *pFont )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( pServerFont != NULL )
    {
        DrawServerFontString( pServerFont, pDisplay, nDrawable, nGC, nX, nY, pStr, nLength );
        return;
    }
#endif

    // sanity check
    if ( pFont == NULL || nLength == 0 )
        return;

    rtl_TextEncoding nAsciiEnc = pFont->GetAsciiEncoding();

    if ( nAsciiEnc == RTL_TEXTENCODING_UNICODE )
    {
        if ( bVertical )
            DrawVerticalString( pDisplay, nDrawable, nGC, nX, nY, pStr, nLength, pFont );
        else
        {

            // plain Unicode, can handle all chars and can be handled straight forward
            XFontStruct* pFontStruct = pFont->GetFontStruct( nAsciiEnc );

            if ( pFontStruct == NULL )

                return;

            XSetFont( pDisplay, nGC, pFontStruct->fid );

            #ifdef OSL_LITENDIAN
            sal_Unicode *pBuffer = (sal_Unicode*)alloca( nLength * sizeof(sal_Unicode) );
            for ( int i = 0; i < nLength; i++ )
                pBuffer[ i ] = SwapBytes(pStr[ i ]) ;
            #else
            sal_Unicode *pBuffer = const_cast<sal_Unicode*>(pStr);
            #endif

            XDrawString16( pDisplay, nDrawable, nGC, nX, nY, (XChar2b*)pBuffer, nLength );
        }
    }
    else
    {
        // convert the string to a XTextItem16 with each item chars having the
        // encoding matching the font of fontid
        XTextItem16 *pTextItem = (XTextItem16*)alloca( nLength * sizeof(XTextItem16) );
        XChar2b *pMBChar = (XChar2b*)alloca( nLength * sizeof(XChar2b) );
        memcpy( pMBChar, pStr, nLength * sizeof(XChar2b) );

        rtl_TextEncoding nEncoding   = nAsciiEnc;
        XFontStruct*     pFontStruct = pFont->GetFontStruct( nEncoding );

        if ( pFontStruct == NULL )
            return;

        VTextItemExt* pVTextItemExt;
        if ( bVertical )
            pVTextItemExt = (VTextItemExt*)alloca( nLength * sizeof(VTextItemExt) );

        for ( int nChar = 0, nItem = -1; nChar < nLength; nChar++ )
        {
            rtl_TextEncoding nOldEnc = nEncoding;
            pFont->GetFontStruct( pStr[nChar], &nEncoding, &pFontStruct, pCvt );

            if ( pFontStruct == NULL )
                continue;

            if ( (nItem != -1) && (pFontStruct->fid == pTextItem[ nItem ].font) )
            {
                pTextItem[ nItem ].nchars += 1;
            }
            else
            {
                if ( nItem != -1 )
                    ConvertTextItem16( &pTextItem[ nItem ], pCvt, nOldEnc );

                ++nItem;

                pTextItem[ nItem ].chars  = pMBChar + nChar;
                pTextItem[ nItem ].delta  = 0;
                pTextItem[ nItem ].font   = pFontStruct->fid;
                pTextItem[ nItem ].nchars = 1;

                if ( bVertical )
                {
                    pVTextItemExt[ nItem ].mnEncoding = nEncoding;
                    pVTextItemExt[ nItem ].mpString = pStr + nChar;
                }
            }
        }
        ConvertTextItem16( &pTextItem[ nItem ], pCvt, nEncoding );
        ++nItem;

        if ( bVertical )
        {
            pVTextItemExt[ nItem - 1 ].mnEncoding = nEncoding;
            DrawVerticalTextItem( pDisplay, nDrawable, nGC, nX, nY,
                          pTextItem, nItem, pVTextItemExt, pFont );
        }
        else
        {
#ifndef USE_PSPRINT
            if ( XSalIsDisplay( pDisplay ) )
                XDrawText16( pDisplay, nDrawable, nGC, nX, nY, pTextItem, nItem );
            else
                XPrinterDrawText16( pDisplay, nDrawable, nGC, nX, nY, nAngle,
                        pTextItem, nItem );
#else
                XDrawText16( pDisplay, nDrawable, nGC, nX, nY, pTextItem, nItem );
#endif
        }
    }
}

//--------------------------------------------------------------------------

void
SalGraphicsData::DrawText( long nX, long nY,
        const sal_Unicode *pStr, USHORT nLen )
{
#ifdef __notdef__
    // XXX Fix me this part is not unicode / multibyte aware

    // Bug: #45670#
    // some monospace ISO8859-1 fonts have a hole between chars 128 and 159
    // some applications assume these characters have also the default width
    if( ! bPrinter_                                 &&
        PITCH_FIXED == xFont_->GetFont()->mePitch   &&
        nLen > 1 )
    {
        XFontStruct *pXFS   = GetFontInfo();
        long         nWidth = xFont_->GetDim()->GetWidth();

        if( xFont_->GetFixedWidth() != nWidth
            || xFont_->GetDefaultWidth() != nWidth )
        {
            unsigned int min_char   = pXFS->min_char_or_byte2;
            unsigned int max_char   = pXFS->max_char_or_byte2;
            XCharStruct *pXCS       = pXFS->per_char - min_char;

            for( USHORT i = 0; i < nLen-1; i++ )
            {
                unsigned int c = ((unsigned char*)pStr)[i];

                long nW = c < min_char || c > max_char || ! pXFS->per_char
                    ? xFont_->GetDefaultWidth()
                    : pXCS[c].width;

                if( nW != nWidth )
                {
                    long *pDXAry = new long[nLen];

                    for( i = 0; i < nLen; i++ )
                        pDXAry[i] = nWidth * (i+1);

                    DrawText( nX, nY, pStr, nLen, pDXAry );

                    delete pDXAry;

                    return;
                }
            }
        }
    }

#endif /* __notdef__ */

    Display             *pDisplay = GetXDisplay();
    SalConverterCache   *pCvt     = GetDisplay()->GetConverter();
    GC                  pGC       = SelectFont();

#ifdef USE_XRENDER
    tmpClipRegion = pClipRegion_;
#endif //USE_XRENDER
    DrawUnicodeString( mpServerSideFont, pDisplay, hDrawable_, pGC, nX, nY,
            pStr, nLen, nFontOrientation_ * 64 / 10, bFontVertical_, pCvt, xFont_ );
}

void
SalGraphics::DrawText( long nX, long nY, const xub_Unicode* pStr, USHORT nLen )
{
    maGraphicsData.FaxPhoneComment( pStr, nLen );
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
         maGraphicsData.m_pPrinterGfx->DrawText (Point(nX, nY), pStr, nLen);
    else
    {
#endif

    maGraphicsData.DrawText( nX, nY, pStr, nLen );

#if defined(USE_PSPRINT)
    }
#endif
}

//--------------------------------------------------------------------------

static BOOL
CheckNoNegativeCoordinateWorkaround()
{
    /* Motivation: one of our clients uses a Solaris 2.4 X86 system with an
       XServer for the Matrox Mystique graphics card. This board/server
       sometimes does not draw Text with negative x-coordinates into a
       virtual device (for unknown reasons). A stock X-server just clips the
       part in the negative area. */
    static int nCheck = -2;
    if( nCheck == -2 )
    {
        char* pCmp = getenv( "SAL_NO_NEGATIVE_TEXT_OFFSET" );
        if( pCmp && ! strncasecmp( pCmp, "true", 4 ) )
            nCheck = 1;
        else
            nCheck = 0;
    }
    return nCheck ? TRUE : FALSE;
}

void
SalGraphicsData::DrawText(
        long nX, long nY,
        const sal_Unicode* pStr, USHORT nLen, const long* pDXAry )
{
    GC pGC = SelectFont();

    // workaround for problems with negative coordinates
    long* pTmpAry = NULL;
    if( nX < 0 && CheckNoNegativeCoordinateWorkaround() )
    {
        long nOldX = nX;
        while( nX < 0 )
        {
            nX = nOldX + *pDXAry;
            pStr++, pDXAry++, nLen--;
            if( nLen < 1 )
                return;
        }
        pTmpAry = new long[ nLen ];
        for( int q = 0; q < nLen-1; q++ )
            pTmpAry[q] = pDXAry[q] - ( nX - nOldX );
        pDXAry = pTmpAry;
    }

    // draw every single character
    SalConverterCache *pCvt = GetDisplay()->GetConverter();
    int angle = nFontOrientation_ * 64 / 10;
    BOOL bVertical = bFontVertical_;
    Polygon aPolygon(1);
    Point   aOrigin( nX, nY );
    Point   aCharPos;

#ifdef USE_XRENDER
    tmpClipRegion = pClipRegion_;
#endif //USE_XRENDER
    DrawUnicodeString( mpServerSideFont, GetXDisplay(), hDrawable_, pGC,
            aOrigin.X(), aOrigin.Y(), pStr, 1, angle, bVertical, pCvt, xFont_ );

    for( int i = 1; i < nLen ; i++ )
    {
        aCharPos = Point( aOrigin.X() + pDXAry[ i - 1 ], aOrigin.Y() );
        aPolygon.SetPoint( aCharPos, 0 );
        aPolygon.Rotate( aOrigin, nFontOrientation_ );
        aCharPos = aPolygon.GetPoint( 0 );

#ifdef USE_XRENDER
        tmpClipRegion = pClipRegion_;
#endif // USE_XRENDER
        DrawUnicodeString( mpServerSideFont, GetXDisplay(), hDrawable_, pGC,
                aCharPos.X(), aCharPos.Y(), pStr + i, 1, angle, bVertical, pCvt, xFont_ );
    }

    if( pTmpAry )
        delete pTmpAry;
}

// ----------------------------------------------------------------------------
//
// SalGraphics
//
// ----------------------------------------------------------------------------

USHORT
SalGraphics::SetFont( ImplFontSelectData *pEntry )
{
    #if defined(USE_PSPRINT)
    if( (maGraphicsData.m_pPrinterGfx != NULL)
#ifdef USE_BUILTIN_RASTERIZER
    && ( !pEntry->mpFontData || (pEntry->mpFontData->mpSysData != SERVERFONT_MAGIC) )
#endif // USE_BUILTIN_RASTERIZER
    )
    {
        sal_Bool bVertical = pEntry->mbVertical;
        sal_Int32 nID = pEntry->mpFontData ? (sal_Int32)pEntry->mpFontData->mpSysData : 0;

        return maGraphicsData.m_pPrinterGfx->SetFont(nID, pEntry->mnHeight,
                                                     pEntry->mnOrientation);
    }
    else
    {
    #endif

    maGraphicsData.SetFont( pEntry );
    return _IsPrinter() ? SAL_SETFONT_USEDRAWTEXTARRAY : 0;

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ----------------------------------------------------------------------------

void
SalGraphics::DrawTextArray( long nX, long nY,
        const xub_Unicode *pStr, USHORT nLen, const long *pDXAry )
{
    maGraphicsData.FaxPhoneComment( pStr, nLen );
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
         maGraphicsData.m_pPrinterGfx->DrawText (Point(nX, nY), pStr, nLen, pDXAry);
    else
    {
#endif

    maGraphicsData.DrawText( nX, nY, pStr, nLen, pDXAry );

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ----------------------------------------------------------------------------

void
SalGraphics::SetTextColor( SalColor nSalColor )
{
    #if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
    {
        psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                                  SALCOLOR_GREEN (nSalColor),
                                  SALCOLOR_BLUE  (nSalColor));
        maGraphicsData.m_pPrinterGfx->SetTextColor (aColor);
    }
    else
    {
    #endif

    if( _GetTextColor() != nSalColor )
    {
        _GetTextColor()     = nSalColor;
        _GetTextPixel()     = _GetPixel( nSalColor );
        _IsFontGC()         = FALSE;
    }

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ----------------------------------------------------------------------------

#if defined(USE_PSPRINT)

static FontWidth
ToFontWidth (psp::width::type eWidth)
{
    switch (eWidth)
    {
        case psp::width::UltraCondensed: return WIDTH_ULTRA_CONDENSED;
        case psp::width::ExtraCondensed: return WIDTH_EXTRA_CONDENSED;
        case psp::width::Condensed:      return WIDTH_CONDENSED;
        case psp::width::SemiCondensed:  return WIDTH_SEMI_CONDENSED;
        case psp::width::Normal:         return WIDTH_NORMAL;
        case psp::width::SemiExpanded:   return WIDTH_SEMI_EXPANDED;
        case psp::width::Expanded:       return WIDTH_EXPANDED;
        case psp::width::ExtraExpanded:  return WIDTH_EXTRA_EXPANDED;
        case psp::width::UltraExpanded:  return WIDTH_ULTRA_EXPANDED;
    }
    return WIDTH_DONTKNOW;
}

static FontWeight
ToFontWeight (psp::weight::type eWeight)
{
    switch (eWeight)
    {
        case psp::weight::Thin:       return WEIGHT_THIN;
        case psp::weight::UltraLight: return WEIGHT_ULTRALIGHT;
        case psp::weight::Light:      return WEIGHT_LIGHT;
        case psp::weight::SemiLight:  return WEIGHT_SEMILIGHT;
        case psp::weight::Normal:     return WEIGHT_NORMAL;
        case psp::weight::Medium:     return WEIGHT_MEDIUM;
        case psp::weight::SemiBold:   return WEIGHT_SEMIBOLD;
        case psp::weight::Bold:       return WEIGHT_BOLD;
        case psp::weight::UltraBold:  return WEIGHT_ULTRABOLD;
        case psp::weight::Black:      return WEIGHT_BLACK;
    }
    return WEIGHT_DONTKNOW;
}

static FontPitch
ToFontPitch (psp::pitch::type ePitch)
{
    switch (ePitch)
    {
        case psp::pitch::Fixed:     return PITCH_FIXED;
        case psp::pitch::Variable:  return PITCH_VARIABLE;
    }
    return PITCH_DONTKNOW;
}

static FontItalic
ToFontItalic (psp::italic::type eItalic)
{
    switch (eItalic)
    {
        case psp::italic::Upright:  return ITALIC_NONE;
        case psp::italic::Oblique:  return ITALIC_OBLIQUE;
        case psp::italic::Italic:   return ITALIC_NORMAL;
    }
    return ITALIC_DONTKNOW;
}

static FontFamily
ToFontFamily (psp::family::type eFamily)
{
    switch (eFamily)
    {
        case psp::family::Decorative: return FAMILY_DECORATIVE;
        case psp::family::Modern:     return FAMILY_MODERN;
        case psp::family::Roman:      return FAMILY_ROMAN;
        case psp::family::Script:     return FAMILY_SCRIPT;
        case psp::family::Swiss:      return FAMILY_SWISS;
        case psp::family::System:     return FAMILY_SYSTEM;
    }
    return FAMILY_DONTKNOW;
}

#endif /* USE_PSPRINT */

// ----------------------------------------------------------------------------

void
SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
    #if defined(USE_PSPRINT)
    if (maGraphicsData.m_pJobData != NULL)
    {
        ::std::list< psp::fontID > aList;

        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        rMgr.getFontList( aList, maGraphicsData.m_pJobData->m_pParser );

        ::std::list< psp::fontID >::iterator it;
        for (it = aList.begin(); it != aList.end(); ++it)
        {
            psp::FastPrintFontInfo aInfo;
            if (rMgr.getFontFastInfo (*it, aInfo))
            {
                ImplFontData *pFontData = new ImplFontData;
                pFontData->mpSysData = (void*)*it;

                pFontData->meFamily             = ToFontFamily (aInfo.m_eFamilyStyle);
                pFontData->meWeight             = ToFontWeight (aInfo.m_eWeight);
                pFontData->meItalic                 = ToFontItalic (aInfo.m_eItalic);
                    pFontData->meWidthType          = ToFontWidth  (aInfo.m_eWidth);
                pFontData->mePitch                  = ToFontPitch  (aInfo.m_ePitch);
                pFontData->meCharSet            = aInfo.m_aEncoding;
                pFontData->maName           = aInfo.m_aFamilyName;
                pFontData->meScript         = SCRIPT_DONTKNOW;
                /* pFontData->maStyleName  = XXX */

                pFontData->mnWidth          = 0;
                pFontData->mnHeight         = 0;
                pFontData->mbOrientation        = TRUE;
                pFontData->mnQuality            = aInfo.m_eType == psp::fonttype::Builtin ? 1024 : 0;
                pFontData->mnVerticalOrientation= 0;
                pFontData->meType               = TYPE_SCALABLE;
                pFontData->mbDevice             = aInfo.m_eType == psp::fonttype::Builtin;

#ifdef DEBUG
                if( pFontData->maName.EqualsIgnoreCaseAscii( "helvetica" ) )
                {
                    fprintf( stderr, "" );
                }
#endif
                pList->Add( pFontData );
            }
        }
    }
    else
    {
    #endif

    XlfdStorage* pFonts = _GetDisplay()->GetXlfdList();

    for ( int nIdx = 0; nIdx < pFonts->GetCount(); nIdx++ )
    {
        ImplFontData *pFontData = new ImplFontData;
        pFonts->Get(nIdx)->ToImplFontData( pFontData );
        pList->Add( pFontData );
    }

#ifdef USE_BUILTIN_RASTERIZER
        aX11GlyphPeer.SetDisplay( maGraphicsData.GetXDisplay() );
#endif // USE_BUILTIN_RASTERIZER

    #if defined(USE_PSPRINT)
    }
    #endif

#ifdef USE_BUILTIN_RASTERIZER
    GlyphCache::EnsureInstance( aX11GlyphPeer );
    GlyphCache::GetInstance().FetchFontList( pList );
#endif // USE_BUILTIN_RASTERIZER
}

// ----------------------------------------------------------------------------

inline long
sal_DivideNeg( long n1, long n2 )
{
    return ( n1 < 0 ) ? (n1 - n2 / 2) / n2 : (n1 + n2 / 2) / n2;
}

// ----------------------------------------------------------------------------

void
SalGraphics::GetFontMetric( ImplFontMetricData *pMetric )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        long rDummyFactor;
        maGraphicsData.mpServerSideFont->FetchFontMetric( *pMetric,
            rDummyFactor );
        return;
    }
#endif //USE_BUILTIN_RASTERIZER

    #if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
    {
        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        psp::PrintFontInfo aInfo;

        if (rMgr.getFontInfo (maGraphicsData.m_pPrinterGfx->GetFontID(), aInfo))
        {
            sal_Int32 nTextSize = maGraphicsData.m_pPrinterGfx->GetFontSize();

            pMetric->mnOrientation  = maGraphicsData.m_pPrinterGfx->GetFontAngle();
            pMetric->mnSlant        = 0;
            pMetric->mbDevice       = sal_True;

            pMetric->meCharSet      = aInfo.m_aEncoding;

            pMetric->meFamily       = ToFontFamily (aInfo.m_eFamilyStyle);
            pMetric->meWeight       = ToFontWeight (aInfo.m_eWeight);
            pMetric->mePitch        = ToFontPitch  (aInfo.m_ePitch);
            pMetric->meItalic       = ToFontItalic (aInfo.m_eItalic);
            pMetric->meType     = TYPE_SCALABLE;

            pMetric->mnFirstChar    =   0;
            pMetric->mnLastChar = 255;

            pMetric->mnWidth        = aInfo.m_nWidth * nTextSize / 1000;
            pMetric->mnAscent       = aInfo.m_nAscend * nTextSize / 1000;
            pMetric->mnDescent      = aInfo.m_nDescend * nTextSize / 1000;
            pMetric->mnLeading      = aInfo.m_nLeading * nTextSize / 1000;
        }
    }
    else
    {
    #endif

    ExtendedFontStruct* pFont = maGraphicsData.xFont_;

    if ( pFont != NULL )
    {
        pFont->ToImplFontMetricData( pMetric );

        #ifndef USE_PSPRINT
        if( XSalCanDrawRotString( maGraphicsData.GetXDisplay(), None ) )
            pMetric->mnOrientation = maGraphicsData.nFontOrientation_;
        #endif
        if ( maGraphicsData.bFontVertical_ )
            pMetric->mnOrientation = 2700;

        long n;

        n = maGraphicsData.aScale_.GetNumerator();
        if( n != 1 )
        {
            pMetric->mnWidth    *= n;
            pMetric->mnAscent   *= n;
            pMetric->mnDescent  *= n;
            pMetric->mnLeading  *= n;
            pMetric->mnSlant    *= n;
        }

        n = maGraphicsData.aScale_.GetDenominator();
        if( n != 1 )
        {
            pMetric->mnWidth    = Divide( pMetric->mnWidth, n );
            pMetric->mnAscent   = sal_DivideNeg( pMetric->mnAscent,  n );
            pMetric->mnDescent  = sal_DivideNeg( pMetric->mnDescent, n );
            pMetric->mnLeading  = sal_DivideNeg( pMetric->mnLeading, n );
            pMetric->mnSlant    = sal_DivideNeg( pMetric->mnSlant,   n );
        }
    }

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ---------------------------------------------------------------------------

static long
InitializeWidthArray( long *pWidthArray, sal_Size nItems, int nValue = 0  )
{
    const long nPrecision = 1;

    for ( int i = 0; i < nItems; i++, pWidthArray++ )
        *pWidthArray = nValue;

    return nPrecision;
}

// ---------------------------------------------------------------------------

long
SalGraphics::GetCharWidth( USHORT nChar1, USHORT nChar2, long  *pWidthAry )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        ServerFont& rSF = *maGraphicsData.mpServerSideFont;
        for( int i = nChar1; i <= nChar2; ++i )
        {
            const int nGlyphIndex = rSF.GetGlyphIndex( i );
            const GlyphMetric& rGM = rSF.GetGlyphMetric( nGlyphIndex );
            pWidthAry[ i - nChar1 ] = rGM.GetDelta().X();
        }
        return 1;
    }
#endif // USE_BUILTIN_RASTERIZER

    #if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
        return maGraphicsData.m_pPrinterGfx->GetCharWidth(nChar1, nChar2, pWidthAry);
    else
    {
    #endif

    // return the precision of the calculated charwidth, e.g. 1000 = 3 digits
    // defaultet to 1 for now
    const long nPrecision = 1;
    int nRequestedWidth = nChar2 - nChar1 + 1;
    int nCharWidth;

    // XXX sanity check, this may happen if no font at all is installed
    // or no system font matches the requirements for the user interface
    if ( maGraphicsData.xFont_ == NULL )
        return InitializeWidthArray( pWidthAry, nRequestedWidth, 12 );

    // the font should know it's metrics best
    SalDisplay *pSalDisplay = maGraphicsData.GetDisplay();

    nCharWidth = maGraphicsData.xFont_->GetCharWidth(
            pSalDisplay->GetConverter(), nChar1, nChar2, pWidthAry,
            maGraphicsData.bFontVertical_ );

    // XXX sanity check, this may happen if the font cannot be loaded/queried
    // either because of a garbled fontpath or because of invalid fontfile
    if ( nCharWidth != nRequestedWidth )
        InitializeWidthArray( pWidthAry + nCharWidth,
                nRequestedWidth - nCharWidth );

    // handle internal scaling
    const long nNumerator   = maGraphicsData.aScale_.GetNumerator();
    const long nDenominator = maGraphicsData.aScale_.GetDenominator();
    long *pPtr;
    sal_Unicode nChar;

    if ( nNumerator != 1 )
        for( pPtr = pWidthAry, nChar = nChar1; nChar <= nChar2; nChar++, pPtr++)
            *pPtr *= nNumerator;
    if ( nDenominator != 1 )
        for( pPtr = pWidthAry, nChar = nChar1; nChar <= nChar2; nChar++, pPtr++)
            *pPtr = Divide( *pPtr, nDenominator );

    // return
    return nPrecision;

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ---------------------------------------------------------------------------

extern unsigned char TranslateCharName( char* );

ULONG
SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData *pKernPairs )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        ImplKernPairData* pTmpKernPairs;
        ULONG nGotPairs = maGraphicsData.mpServerSideFont->GetKernPairs( &pTmpKernPairs );
        if( nGotPairs > nPairs )
            nGotPairs = nPairs;
        for( int i = 0; i < nGotPairs; ++i )
            pKernPairs[ i ] = pTmpKernPairs[ i ];
        delete[] pTmpKernPairs;
        return nGotPairs;
    }
#endif //USE_BUILTIN_RASTERIZER

    if( ! _IsPrinter() )
        return 0;
#ifdef USE_PSPRINT
    return 0;
#else
    // get pair kerning table ( internal data from xprinter )
    int i, nCurPair=0;

    // XXX Fix me, improve this to be multi encoding aware: merge kern
    // pair list for all encodings available in the xfont
    rtl_TextEncoding nEncoding = maGraphicsData.xFont_->GetAsciiEncoding();
    XFontStruct *pXFS = maGraphicsData.xFont_->GetFontStruct( nEncoding );
    XExtData    *pXES = pXFS->ext_data;

    for( i = 0; pXES && i < 2; i++, pXES = pXES->next );
    if( i < 2 )
        return 0;
    XpPairKernData* pXpPKD = (XpPairKernData*)(pXES->private_data);
    PairKernData*   pPKD   = pXpPKD->pkd;

    for( i = 0, nCurPair=0; i < pXpPKD->numOfPairs; i++ )
    {
        unsigned char c1 = TranslateCharName( pPKD[i].name1 );
        unsigned char c2 = TranslateCharName( pPKD[i].name2 );
        if( c1 && c2 )
        {
            if( pKernPairs && nCurPair < nPairs )

            {
                pKernPairs[ nCurPair ].mnChar1 = c1;
                pKernPairs[ nCurPair ].mnChar2 = c2;
                pKernPairs[ nCurPair ].mnKern =
                        (long)(pPKD[i].xamt * pXpPKD->pointsize / 1000 );
            }
            nCurPair++;
        }
    }

    return nCurPair;
#endif
}

// ---------------------------------------------------------------------------

BOOL
SalGraphics::GetGlyphBoundRect( xub_Unicode cChar,
        long *pX, long *pY, long *pDX, long *pDY )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        ServerFont& rSF = *maGraphicsData.mpServerSideFont;
        const int nGlyphIndex = rSF.GetGlyphIndex( cChar );
        const GlyphMetric& rGM = rSF.GetGlyphMetric( nGlyphIndex );

        *pX = rGM.GetOffset().X();
        *pY = rGM.GetOffset().Y();
        *pDX = rGM.GetSize().Width();
        *pDY = rGM.GetSize().Height();
        return TRUE;
    }
#endif //USE_BUILTIN_RASTERIZER

    return FALSE;
}

// ---------------------------------------------------------------------------

ULONG
SalGraphics::GetGlyphOutline( xub_Unicode cChar,
        USHORT **ppPolySizes, SalPoint **ppPoints, BYTE **ppFlags )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        PolyPolygon rPolyPoly;
        ServerFont& rSF = *maGraphicsData.mpServerSideFont;
        const int nGlyphIndex = rSF.GetGlyphIndex( cChar );
        bool bRet = rSF.GetGlyphOutline( nGlyphIndex, rPolyPoly );
        if( !bRet )
            return 0;

        // translate PolyPolygon into ancient format
        const unsigned nPolygons = rPolyPoly.Count();
        unsigned nPoints = 0;
        unsigned i;
        for( i = 0; i < nPolygons; ++i )
            nPoints += rPolyPoly[ i].GetSize();

        USHORT* const pPolySizes    = new USHORT[ nPolygons ];
        SalPoint* const pPoints     = new SalPoint[ nPoints ];
        BYTE* const pFlags          = new BYTE[ nPoints ];

        *ppPolySizes = pPolySizes;
        *ppPoints   = pPoints;
        *ppFlags    = pFlags;

        unsigned nDestIndex = 0;
        for( i = 0; i < nPolygons; ++i )
        {
            const Polygon& rPolygon = rPolyPoly[ i ];
            const int nPoints = rPolygon.GetSize();
            pPolySizes[ i ] = nPoints;
            for( unsigned j = 0; j < nPoints; ++j )
            {
                const Point& aPoint = rPolygon[ j ];
                pPoints[ nDestIndex ].mnX   = aPoint.X();
                pPoints[ nDestIndex ].mnY   = aPoint.Y();
                pFlags[ nDestIndex ]        = rPolygon.GetFlags( j );
                ++nDestIndex;
            }
        }

        return nPolygons;
    }
#endif // USE_BUILTIN_RASTERIZER

    return 0;
}

// ===========================================================================
