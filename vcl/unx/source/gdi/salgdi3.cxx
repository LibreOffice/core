/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.106 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:57:39 $
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !( defined(FREEBSD) || defined(NETBSD) || defined(MACOSX) )
#include <alloca.h>
#endif

#ifdef USE_BUILTIN_RASTERIZER
#include <gcach_xpeer.hxx>
#endif // USE_BUILTIN_RASTERIZER

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
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
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
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
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef EXTENDED_FONTSTRUCT_HXX
#include "xfont.hxx"
#endif


#include <tools/debug.hxx>
#include <tools/stream.hxx>

#ifndef _USE_PRINT_EXTENSION_
#include <psprint/printergfx.hxx>
#include <psprint/fontmanager.hxx>
#include <psprint/jobdata.hxx>
#include <psprint/printerinfomanager.hxx>
#include <svapp.hxx>
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


#ifdef MACOSX
#include <hash_set>
#endif

// -----------------------------------------------------------------------

#ifdef USE_BUILTIN_RASTERIZER
static X11GlyphPeer aX11GlyphPeer;
#endif // USE_BUILTIN_RASTERIZER

using namespace rtl;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
 *  returns:
 *  true: cut out positions rStart to rStop from output because fax number was swallowed
 *  false: do nothing
 */

String SalGraphicsData::FaxPhoneComment( const String& rOrig, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop ) const
{
    rCutStop = rCutStart = STRING_NOTFOUND;

#ifdef _USE_PRINT_EXTENSION_
    return String( rOrig );
#else
    if( ! m_pPhoneNr )
        return String( rOrig );
#endif

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"
#define FAX_END_TOKEN_LENGTH     2

    bool bRet = false;
    bool bStarted = false;
    bool bStopped = false;
    USHORT nPos;
    USHORT nStart = 0;
    USHORT nStop = rLen;
    String aPhone = rOrig.Copy( nIndex, rLen );

    static String aPhoneNumber;
    static bool bIsCollecting = false;

    if( ! bIsCollecting )
    {
        if( ( nPos = aPhone.SearchAscii( FAX_PHONE_TOKEN ) ) != STRING_NOTFOUND )
        {
            nStart = nPos;
            bIsCollecting = true;
            aPhoneNumber.Erase();
            bRet = true;
            bStarted = true;
        }
    }
    if( bIsCollecting )
    {
        bRet = true;
        nPos = bStarted ? nStart + FAX_PHONE_TOKEN_LENGTH : 0;
        if( ( nPos = aPhone.SearchAscii( FAX_END_TOKEN, nPos ) ) != STRING_NOTFOUND )
        {
            bIsCollecting = false;
            nStop = nPos + FAX_END_TOKEN_LENGTH;
            bStopped = true;
        }
        int nTokenStart = nStart + (bStarted ? FAX_PHONE_TOKEN_LENGTH : 0);
        int nTokenStop = nStop - (bStopped ? FAX_END_TOKEN_LENGTH : 0);
        aPhoneNumber += aPhone.Copy( nTokenStart, nTokenStop - nTokenStart );
        if( ! bIsCollecting )
        {
#ifndef PRINTER_DUMMY
            *m_pPhoneNr = aPhoneNumber;
#endif
            aPhoneNumber.Erase();
        }
    }
    if( aPhoneNumber.Len() > 1024 )
    {
        bIsCollecting = false;
        aPhoneNumber.Erase();
        bRet = false;
    }

    String aRet;
    if( bRet
#ifndef MACOSX
        && m_bSwallowFaxNo
#endif
        )
    {
        rLen -= nStop - nStart;
        rCutStart = nStart+nIndex;
        rCutStop = nStop+nIndex;
        if( rCutStart )
            aRet = rOrig.Copy( 0, rCutStart );
        aRet += rOrig.Copy( rCutStop );
    }
    else
        aRet = rOrig;

    return aRet;
}

// ----------------------------------------------------------------------------
//
// manage X11 fonts and self rastered fonts
//
// ----------------------------------------------------------------------------

#ifndef _USE_PRINT_EXTENSION_

static FontItalic ToFontItalic (psp::italic::type eItalic);
static FontWeight ToFontWeight (psp::weight::type eWeight);

class FontLookup
{
    public:

        struct hash;
        struct equal;
        typedef ::std::hash_set< FontLookup,
                                 FontLookup::hash,
                                 FontLookup::equal > fl_hashset;

    private:

        FontWeight          mnWeight;
        FontItalic          mnItalic;
        sal_Bool            mbDisplay;

        rtl::OString        maName;

    public:

                            FontLookup ( ::std::list< psp::fontID >::iterator& it,
                                          const psp::PrintFontManager& rMgr );
                            FontLookup (const Xlfd& rFont);
                            FontLookup (const FontLookup &rRef) :
                                    mnWeight (rRef.mnWeight),
                                    mnItalic (rRef.mnItalic),
                                    maName   (rRef.maName),
                                    mbDisplay(rRef.mbDisplay)
                            {}
                            ~FontLookup ()
                            {}

        static void         BuildSet (fl_hashset& rSet);
        static bool         InSet (const fl_hashset& rSet, const Xlfd& rXfld);
        bool                InSet (const fl_hashset& rSet) const;

        bool                operator== (const FontLookup &rRef) const
                            {
                                return     (abs(mnWeight - rRef.mnWeight) < 2)
                                        && (mnItalic == rRef.mnItalic)
                                        && (maName   == rRef.maName)
                                        && (mbDisplay== rRef.mbDisplay);
                            }
        FontLookup&         operator= (const FontLookup &rRef)
                            {
                                mnWeight = rRef.mnWeight;
                                mnItalic = rRef.mnItalic;
                                maName   = rRef.maName;
                                mbDisplay= rRef.mbDisplay;

                                return *this;
                            }
        size_t              Hash() const
                            {
                                return maName.hashCode ();
                            }

        struct equal
        {
            bool operator()(const FontLookup &r1, const FontLookup &r2) const
            {
                return r1 == r2;
            }
        };
        struct hash
        {
            size_t operator()(const FontLookup &rArg) const
            {
                return rArg.Hash();
            }
        };
};

FontLookup::FontLookup ( ::std::list< psp::fontID >::iterator& it,
                         const psp::PrintFontManager& rMgr )
{
    psp::FastPrintFontInfo aInfo;
    if (rMgr.getFontFastInfo (*it, aInfo))
    {
        mnItalic = ToFontItalic (aInfo.m_eItalic);
        mnWeight = ToFontWeight (aInfo.m_eWeight);
        mbDisplay=    aInfo.m_eType == psp::fonttype::Builtin
                   || aInfo.m_eType == psp::fonttype::Unknown ? False : True;
        maName   = rtl::OUStringToOString
            ( aInfo.m_aFamilyName,
              RTL_TEXTENCODING_ISO_8859_1).toAsciiLowerCase();

        sal_Int32       n_length = maName.getLength();
        const sal_Char* p_from   = maName.getStr();
        sal_Char*       p_to     = (sal_Char*)alloca (n_length + 1);

        sal_Int32 i, j;
        for (i = 0, j = 0; i < n_length; i++)
        {
            if ( p_from[i] != ' ' )
                p_to[j++] = p_from[i];
        }
        maName = rtl::OString (p_to, j);
        if (mnItalic == ITALIC_OBLIQUE)
            mnItalic = ITALIC_NORMAL;

    }
    else
    {
        mnItalic = ITALIC_DONTKNOW;
        mnWeight = WEIGHT_DONTKNOW;
        mbDisplay= False;
    }
}

FontLookup::FontLookup (const Xlfd& rFont)
{
    AttributeProvider* pFactory = rFont.mpFactory;
    Attribute*         pAttr;

    pAttr    = pFactory->RetrieveSlant  (rFont.mnSlant);
    mnItalic = (FontItalic)pAttr->GetValue();
    pAttr    = pFactory->RetrieveWeight (rFont.mnWeight);
    mnWeight = (FontWeight)pAttr->GetValue();
    pAttr    = pFactory->RetrieveFamily (rFont.mnFamily);
    maName   = pAttr->GetKey();

    if (mnItalic == ITALIC_OBLIQUE)
        mnItalic = ITALIC_NORMAL;

    mbDisplay = True;
}

void
FontLookup::BuildSet (FontLookup::fl_hashset &rSet)
{
    ::std::list< psp::fontID > aIdList;

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aIdList, NULL );

    ::std::list< psp::fontID >::iterator it;
    for (it = aIdList.begin(); it != aIdList.end(); ++it)
    {
        FontLookup aItem (it, rMgr);
        rSet.insert (aItem);
    }
}

bool
FontLookup::InSet (const FontLookup::fl_hashset& rSet) const
{
      fl_hashset::const_iterator it = rSet.find(*this);
      return it == rSet.end() ? false : true;
}

bool
FontLookup::InSet (const FontLookup::fl_hashset& rSet, const Xlfd& rXlfd)
{
    FontLookup aNeedle (rXlfd);
    return aNeedle.InSet (rSet);
}

#endif

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
        mpFactory  = new AttributeProvider;
        mpFontList = new XlfdStorage;
        mpFallbackFactory = new VirtualXlfd;

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



        mpFactory->AddClassification();
        // add some pretty print description
        mpFactory->AddAnnotation();
        // misc feature checking
        mpFactory->TagFeature();

        // sort according to font style
        qsort( pXlfdList, nXlfdCount, sizeof(Xlfd), XlfdCompare );

#ifndef _USE_PRINT_EXTENSION_
        // create a list of fonts already managed by the fontmanager
        FontLookup::fl_hashset aSet;
        FontLookup::BuildSet (aSet);
#endif

        //
        // create a font list with merged encoding information
        //

        BitmapXlfdStorage   aBitmapList;
        ScalableXlfd       *pScalableFont = NULL;

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
            // exclude "interface system" and "interface user"
            if (pAttr->HasFeature( XLFD_FEATURE_APPLICATION_FONT ) )

            {
                continue;
            }
            // exclude fonts already managed by fontmanager, anyway keep
            // gui fonts: they are candidates for GetInterfaceFont ()
            if (pXlfdList[i].Fonttype() == eTypeScalable)
                ((VirtualXlfd*)mpFallbackFactory)->FilterInterfaceFont (pXlfdList + i);
#ifndef _USE_PRINT_EXTENSION_
            if (FontLookup::InSet (aSet, pXlfdList[i]))
                 continue;
#endif
            Bool bSameOutline = pXlfdList[i].SameFontoutline(pXlfdList + nFrom);
            XlfdFonttype eType = pXlfdList[i].Fonttype();

            // flush the old merged font list if the name doesn't match any more
            if ( !bSameOutline )
            {
                mpFontList->Add( pScalableFont );
                mpFontList->Add( &aBitmapList );

                pScalableFont = NULL;
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

                case eTypeScalableBitmap:
                default:

                    break;
            }

            nFrom = i;
        }

        // flush the merged list into the global list
        mpFontList->Add( pScalableFont );
        mpFontList->Add( &aBitmapList );
        if (mpFallbackFactory->NumEncodings() > 0)
            mpFontList->Add( mpFallbackFactory );
        // cleanup the list of simple font information
        if ( pXlfdList != NULL )
            free( pXlfdList );

        return mpFontList;
    }
}

// ---------------------------------------------------------------------------

ExtendedFontStruct*
SalDisplay::GetFont( const ExtendedXlfd *pRequestedFont, int nPixelSize, sal_Bool bVertical )
{
    if( !pFontCache_ )
    {
        pFontCache_ = new SalFontCache( 64, 64, 16 ); // ???
    }
    else
    {
        ExtendedFontStruct *pItem;
        for ( pItem  = pFontCache_->First();
              pItem != NULL;
              pItem  = pFontCache_->Next() )
        {
            if ( pItem->Match(pRequestedFont, nPixelSize, bVertical) )
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
                                        nPixelSize, bVertical,
                                        const_cast<ExtendedXlfd*>(pRequestedFont) );
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

    pFontCache_ = (SalFontCache*)NULL;
    mpFontList = (XlfdStorage*)NULL;
    mpFactory  = (AttributeProvider*)NULL;
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
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = True;
        values.foreground           = nTextPixel_;
#ifdef _USE_PRINT_EXTENSION_
        values.background = xColormap_->GetWhitePixel();
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCBackground | GCForeground,
                              &values );
#else
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCForeground,
                              &values );
#endif
    }
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

bool SalGraphicsData::SetFont( const ImplFontSelectData *pEntry, int nFallbackLevel )
{
    nFontOrientation_   = pEntry->mnOrientation;
    bFontVertical_      = pEntry->mbVertical;

#ifdef HDU_DEBUG
    ByteString aName( pEntry->maName, osl_getThreadTextEncoding() );
    fprintf( stderr, "SetFont(lvl=%d,\"%s\",naa=%d,b=%d,i=%d)\n", nFallbackLevel, aName.GetBuffer(), pEntry->mbNonAntialiased, pEntry->meWeight, pEntry->meItalic );
    if( pEntry->mpFontData )
    {
        aName = ByteString( pEntry->mpFontData->maName, osl_getThreadTextEncoding() );
        fprintf( stderr, "\t(pFD=\"%s\",b=%d,i=%d)\n",aName.GetBuffer(),pEntry->mpFontData->meWeight,pEntry->mpFontData->meItalic);
    }
#endif

    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        mXFont[i] = NULL; // ->ReleaseRef()

        if( mpServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *mpServerFont[i] );
            mpServerFont[i] = NULL;
        }
    }

    if( !pEntry->mpFontData )
        return false;

    // requesting a font provided by builtin rasterizer
    ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pServerFont != NULL )
    {
        if( !pServerFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pServerFont );
            return false;
        }
        mpServerFont[ nFallbackLevel ] = pServerFont;
        return true;
    }

    if( m_pPrinterGfx != NULL )
        return false;

    bFontGC_    = FALSE;
    aScale_     = Fraction( 1, 1 );
    ExtendedXlfd *pSysFont = (ExtendedXlfd*)pEntry->mpFontData->mpSysData;
    if( !pSysFont )
        return false;
     static int nMaxFontHeight = GetMaxFontHeight();
     USHORT nH, nW;
     if( bWindow_ )
     {
         // see BugId #44528# FontWork (-> #45038#) and as well Bug #47127#
         if( pEntry->mnHeight > nMaxFontHeight )
             nH = nMaxFontHeight;
         else if( pEntry->mnHeight > 2 )
             nH = pEntry->mnHeight;
         else
             nH = 2;
         nW = 0; // should be "pEntry->mnWidth", but for X11 fonts default looks better
     }
     else
     {
         nH = pEntry->mnHeight;
         nW = pEntry->mnWidth;
     }

     mXFont[ nFallbackLevel ] = GetDisplay()->GetFont( pSysFont, nH, bFontVertical_ );
     if( pEntry->mnHeight > nMaxFontHeight || pEntry->mnHeight < 2 )
         aScale_ = Fraction( pEntry->mnHeight, nH );

    return true;
}

//--------------------------------------------------------------------------

static sal_Unicode
SwapBytes( const sal_Unicode nIn )
{
    return ((nIn >> 8) & 0x00ff) | ((nIn & 0x00ff) << 8);
}

// draw string in a specific multibyte encoding
static void
ConvertTextItem16( XTextItem16* pTextItem, rtl_TextEncoding nEncoding )
{
    if ( (pTextItem == NULL) || (pTextItem->nchars <= 0) )
        return;

    SalConverterCache* pCvt = SalConverterCache::GetInstance();
    // convert the string into the font encoding
    sal_Size  nSize;
    sal_Size  nBufferSize = pTextItem->nchars * 2;
    sal_Char *pBuffer = (sal_Char*)alloca( nBufferSize );

    nSize = pCvt->ConvertStringUTF16( (sal_Unicode*)pTextItem->chars, pTextItem->nchars,
                    pBuffer, nBufferSize, nEncoding);

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

//--------------------------------------------------------------------------

#ifdef USE_BUILTIN_RASTERIZER
void SalGraphicsData::DrawServerAAFontString( const ServerFontLayout& rLayout )
{
    Display* pDisplay = GetXDisplay();
    Visual* pVisual = GetDisplay()->GetVisual()->GetVisual();
    XRenderPictFormat* pVisualFormat = (*aX11GlyphPeer.pXRenderFindVisualFormat)( pDisplay, pVisual );

    // create xrender Picture for font foreground
    static Pixmap aPixmap;
    static Picture aSrc = NULL;
    if( !aSrc )
    {
        int iDummy;
        unsigned uDummy, nDepth;
        XLIB_Window wDummy;
        XGetGeometry( pDisplay, hDrawable_, &wDummy, &iDummy, &iDummy,
            &uDummy, &uDummy, &uDummy, &nDepth );
        aPixmap = XCreatePixmap( pDisplay, hDrawable_, 1, 1, nDepth );

        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;
        aSrc = (*aX11GlyphPeer.pXRenderCreatePicture)( pDisplay, aPixmap, pVisualFormat, CPRepeat, &aAttr );
    }

    // set font foreground
    GC nGC = SelectFont();
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );
    aGCVal.clip_mask = None;
    GC tmpGC = XCreateGC( pDisplay, aPixmap, GCForeground | GCClipMask, &aGCVal );
    XDrawPoint( pDisplay, aPixmap, tmpGC, 0, 0 );
    XFreeGC( pDisplay, tmpGC );

    // notify xrender of target drawable
    XRenderPictureAttributes aAttr;
    Picture aDst = (*aX11GlyphPeer.pXRenderCreatePicture)( pDisplay, hDrawable_, pVisualFormat, 0, &aAttr );
    // set clipping
    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
        (*aX11GlyphPeer.pXRenderSetPictureClipRegion)( pDisplay, aDst, pClipRegion_ );

    ServerFont& rFont = rLayout.GetServerFont();
    GlyphSet aGlyphSet = aX11GlyphPeer.GetGlyphSet( rFont );

    Point aPos;
    static const int MAXGLYPHS = 160;
    long aGlyphAry[ MAXGLYPHS ];
    int nMaxGlyphs = rLayout.GetOrientation() ? 1 : MAXGLYPHS;
    for( int nStart = 0;;)
    {
        int nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart );
        if( !nGlyphs )
            break;

        unsigned int aRenderAry[ MAXGLYPHS ];
        for( int i = 0; i < nGlyphs; ++i )
             aRenderAry[ i ] = aX11GlyphPeer.GetGlyphId( rFont, aGlyphAry[i] );
        (*aX11GlyphPeer.pXRenderCompositeString32)( pDisplay, PictOpOver,
            aSrc, aDst, 0, aGlyphSet, 0, 0, aPos.X(), aPos.Y(), aRenderAry, nGlyphs );
    }

    // cleanup
    (*aX11GlyphPeer.pXRenderFreePicture)( pDisplay, aDst );
}

//--------------------------------------------------------------------------

bool SalGraphicsData::DrawServerAAForcedString( const ServerFontLayout& rLayout )
{
    ServerFont& rFont = rLayout.GetServerFont();

    // prepare glyphs and get extent of operation
    int nXmin, nXmax, nYmin, nYmax;
    int nStart = 0;
    Point aPos;
    long nGlyph;
    for( bool bFirst=true; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;
        const int nX2 = nX1 + pRawBitmap->mnWidth;
        const int nY2 = nY1 + pRawBitmap->mnHeight;

        if( bFirst )
        {
            bFirst = false;
            nXmin = nX1;
            nXmax = nX2;
            nYmin = nY1;
            nYmax = nY2;
        }
        else
        {
            if( nXmin > nX1 ) nXmin = nX1;
            if( nXmax < nX2 ) nXmax = nX2;
            if( nYmin > nY1 ) nYmin = nY1;
            if( nYmax < nY2 ) nYmax = nY2;
        }
    }

    // get XImage
    bool bOldXErrorEnabled = GetDisplay()->GetXLib()->GetIgnoreXErrors();
    GetDisplay()->GetXLib()->SetIgnoreXErrors( true );
    Display* pDisplay = GetXDisplay();

    XRectangle aXRect;
    unsigned long nWidth = 1, nHeight = 1;
    if( m_pFrame )
        nWidth = m_pFrame->maGeometry.nWidth, nHeight = m_pFrame->maGeometry.nHeight;
    else if( m_pVDev )
        nWidth = m_pVDev->maVirDevData.GetWidth(), nHeight = m_pVDev->maVirDevData.GetHeight();

    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
    {
        // get bounding box
        XClipBox( pClipRegion_, &aXRect );
        // clip with window
        if( aXRect.x < 0 ) aXRect.x = 0;

        if( aXRect.y < 0 ) aXRect.y = 0;
        if( aXRect.width+aXRect.x > nWidth ) aXRect.width = nWidth-aXRect.x;
        if( aXRect.height+aXRect.y > nHeight ) aXRect.height = nHeight-aXRect.y;
    }
    else
    {
        aXRect.x = 0;
        aXRect.y = 0;
        aXRect.width = nWidth;
        aXRect.height = nHeight;
    }
    if( m_pFrame )
    {
        // clip with screen
        int nScreenX = m_pFrame->maGeometry.nX+aXRect.x;
        int nScreenY = m_pFrame->maGeometry.nY+aXRect.y;
        int nScreenW = GetDisplay()->GetScreenSize().Width();
        int nScreenH = GetDisplay()->GetScreenSize().Height();
        if( nScreenX < 0 )
            aXRect.x -= nScreenX, aXRect.width += nScreenX;
        if( nScreenX+aXRect.width > nScreenW )
            aXRect.width = nScreenW-nScreenX;
        if( nScreenY < 0 )
            aXRect.y -= nScreenY, aXRect.height += nScreenY;
        if( nScreenY+aXRect.height > nScreenH )
            aXRect.height = nScreenH-nScreenY;
    }


    if( nXmin < aXRect.x )  nXmin = aXRect.x;
    if( nYmin < aXRect.y )  nYmin = aXRect.y;
    if( nXmax >= aXRect.x+aXRect.width )    nXmax = aXRect.x + aXRect.width - 1;
    if( nYmax >= aXRect.y+aXRect.height )   nYmax = aXRect.y + aXRect.height - 1;

    if( nXmin > nXmax )
        return false;
    if( nYmin > nYmax )
        return false;

    XImage* const pImg = XGetImage( pDisplay, hDrawable_,
        nXmin, nYmin, (nXmax-nXmin+1), (nYmax-nYmin+1), ~0, ZPixmap );
    if( pImg == NULL )
        return false;

    // prepare context
    GC nGC = SelectFont();
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );

    unsigned long nOrigColor = XGetPixel( pImg, 0, 0 );
    XPutPixel( pImg, 0, 0, aGCVal.foreground );
    unsigned char aColor[4];
    aColor[0] = pImg->data[0];
    aColor[1] = pImg->data[1];
    aColor[2] = pImg->data[2];
    aColor[3] = pImg->data[3];
    XPutPixel( pImg, 0, 0, nOrigColor );

    // work on XImage
    const int bpp = pImg->bits_per_pixel >> 3;
    for( nStart = 0; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;

        if( (nX1 <= nXmax) && ((nX1 + pRawBitmap->mnWidth) > nXmin)
        &&  (nY1 <= nYmax) && ((nY1 + pRawBitmap->mnHeight) > nYmin) )
        {
            const unsigned char* p10 = pRawBitmap->mpBits;
            unsigned char* p20 = (unsigned char*)pImg->data;                // dest left limit
            p20 += (nY1 - nYmin) * pImg->bytes_per_line;
            unsigned char* p21 = p20 + (nX1 - nXmin + pImg->xoffset) * bpp;
            int y = pRawBitmap->mnHeight;
            if( y > nYmax - nY1 )
                y = nYmax - nY1 + 1;
            while( --y >= 0 )
            {
                if( p20 >= (unsigned char*)pImg->data )
                {
                    unsigned char* const p22 = p20 + pImg->width * bpp; // dest right limit
                    unsigned char* pDst = p21;
                    const unsigned char* pSrc = p10;
                    for( int x = pRawBitmap->mnWidth; (--x >= 0) && (p22 > pDst); ++pSrc )
                    {
                        if( (*pSrc == 0) || (p20 > pDst) )          // keep background
                            pDst += bpp;
                        else if( *pSrc == 0xFF )                    // paint foreground
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                *pDst = *pColor;
                        }
                        else                                        // blend fg into bg
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                // theoretically it should be *257) >> 16
                                // but the error is <0.4% worst case and we are in
                                // the innermost loop of very perf-sensitive code

                                *pDst += (*pSrc * ((int)*pColor - *pDst)) >> 8;
                        }
                    }
                }
                p10 += pRawBitmap->mnScanlineSize;
                p20 += pImg->bytes_per_line;
                p21 += pImg->bytes_per_line;
            }
        }
    }

    // put XImage
    XPutImage( pDisplay, hDrawable_, nGC, pImg,
        0, 0, nXmin, nYmin, (nXmax - nXmin + 1), (nYmax - nYmin + 1) );
    XDestroyImage( pImg );

    GetDisplay()->GetXLib()->SetIgnoreXErrors( bOldXErrorEnabled );
    return true;
}

//--------------------------------------------------------------------------

void SalGraphicsData::DrawServerSimpleFontString( const ServerFontLayout& rSalLayout )
{
    ServerFont& rFont = rSalLayout.GetServerFont();

    Display* pDisplay = GetXDisplay();
    GC nGC = SelectFont();

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    aGCVal.line_width = 0;
    GC tmpGC = XCreateGC( pDisplay, hDrawable_, GCFillStyle|GCLineWidth, &aGCVal );
    XCopyGC( pDisplay, nGC, (1<<GCLastBit)-(1+GCFillStyle+GCLineWidth), tmpGC );

    Point aPos;
    long nGlyph;
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        Pixmap aStipple = aX11GlyphPeer.GetPixmap( rFont, nGlyph );
        const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyph );

        if( aStipple != None )
        {
            const int nDestX    = aPos.X() + rGM.GetOffset().X();
            const int nDestY    = aPos.Y() + rGM.GetOffset().Y();

            aGCVal.stipple      = aStipple;
            aGCVal.ts_x_origin  = nDestX;
            aGCVal.ts_y_origin  = nDestY;
            XChangeGC( pDisplay, tmpGC, GCStipple|GCTileStipXOrigin|GCTileStipYOrigin, &aGCVal );

            const int nWidth    = rGM.GetSize().Width();
            const int nHeight   = rGM.GetSize().Height();
            XFillRectangle( pDisplay, hDrawable_, tmpGC, nDestX, nDestY, nWidth, nHeight );
        }

    }

    XFreeGC( pDisplay, tmpGC );
}

//--------------------------------------------------------------------------

#ifndef _USE_PRINT_EXTENSION_

class PspFontLayout : public GenericSalLayout
{
public:
                        PspFontLayout( ::psp::PrinterGfx& );
    virtual bool        LayoutText( ImplLayoutArgs& );
    virtual void        InitFont() const;
    virtual void        DrawText( SalGraphics& ) const;
private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    int                 mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

//--------------------------------------------------------------------------

PspFontLayout::PspFontLayout( ::psp::PrinterGfx& rGfx )
:   mrPrinterGfx( rGfx )
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

//--------------------------------------------------------------------------

bool PspFontLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    mbVertical = ((rArgs.mnFlags & SAL_LAYOUT_VERTICAL) != 0);

    long nUnitsPerPixel = 1;
    int nOldGlyphId = -1;
    long nGlyphWidth = 0;
    int nCharPos = -1;
    Point aNewPos( 0, 0 );
    GlyphItem aPrevItem;
    for(;;)
    {
        bool bRightToLeft;
        if( !rArgs.GetNextPos( &nCharPos, &bRightToLeft ) )
            break;

        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
        int nGlyphIndex = cChar;  // printer glyphs = unicode

        // update fallback_runs if needed
        psp::CharacterMetric aMetric;
        mrPrinterGfx.GetFontMgr().getMetrics( mnFontID, cChar, cChar, &aMetric, mbVertical );
        if( aMetric.width == -1 && aMetric.height == -1 )
            rArgs.NeedFallback( nCharPos, bRightToLeft );

        // apply pair kerning to prev glyph if requested
        if( SAL_LAYOUT_KERNING_PAIRS & rArgs.mnFlags )
        {
            // TODO: get kerning value from printer
            int nKern = 0; //GetGlyphKernValue( nOldGlyphId, nGlyphIndex );
            nGlyphWidth += nKern;
            aPrevItem.mnNewWidth = nGlyphWidth;
        }

        // finish previous glyph
        if( nOldGlyphId >= 0 )
            AppendGlyph( aPrevItem );
        nOldGlyphId = nGlyphIndex;
        aNewPos.X() += nGlyphWidth;

        // prepare GlyphItem for appending it in next round
        nUnitsPerPixel = mrPrinterGfx.GetCharWidth( cChar, cChar, &nGlyphWidth );
        int nGlyphFlags = bRightToLeft ? GlyphItem::IS_RTL_GLYPH : 0;
        nGlyphIndex |= GF_ISCHAR;
        aPrevItem = GlyphItem( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
    }

    // append last glyph item if any
    if( nOldGlyphId >= 0 )
        AppendGlyph( aPrevItem );

    SetOrientation( mrPrinterGfx.GetFontAngle() );
    SetUnitsPerPixel( nUnitsPerPixel );
    return (nOldGlyphId >= 0);
}

class PspServerFontLayout : public ServerFontLayout
{
public:
    PspServerFontLayout( ::psp::PrinterGfx&, ServerFont& rFont );

    virtual void        InitFont() const;
private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    int                 mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

PspServerFontLayout::PspServerFontLayout( ::psp::PrinterGfx& rGfx, ServerFont& rFont )
        :   ServerFontLayout( rFont ),
            mrPrinterGfx( rGfx )
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

void PspServerFontLayout::InitFont() const
{
    mrPrinterGfx.SetFont( mnFontID, mnFontHeight, mnFontWidth,
                          mnOrientation, mbVertical, mbArtItalic, mbArtBold );
}

//--------------------------------------------------------------------------

void DrawPrinterLayout( const SalLayout& rLayout, ::psp::PrinterGfx& rGfx )
{
    const int nMaxGlyphs = 200;
    long        aGlyphAry[ nMaxGlyphs ];
    long        aWidthAry[ nMaxGlyphs ];
    sal_Int32   aIdxAry  [ nMaxGlyphs ];
    sal_Unicode aUnicodes[ nMaxGlyphs ];
    Point aPos;
    long nUnitsPerPixel = rLayout.GetUnitsPerPixel();
    for( int nStart = 0;; )
    {
        int nGlyphCount = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart, aWidthAry );
        if( !nGlyphCount )
            break;

        long nXOffset = 0;
        for( int i = 0; i < nGlyphCount; ++i )
        {
            nXOffset += aWidthAry[ i ];
            aIdxAry[ i ] = nXOffset / nUnitsPerPixel;
            long nGlyphIdx = aGlyphAry[i] & (GF_IDXMASK | GF_ROTMASK);
            aUnicodes[i] = (aGlyphAry[i] & GF_ISCHAR) ? nGlyphIdx : 0;
            aGlyphAry[i] = nGlyphIdx;
        }

        rGfx.DrawGlyphs( aPos, (unsigned long*)aGlyphAry, aUnicodes, nGlyphCount, aIdxAry );
    }
}

//--------------------------------------------------------------------------

void PspFontLayout::InitFont() const
{
    mrPrinterGfx.SetFont( mnFontID, mnFontHeight, mnFontWidth,
        mnOrientation, mbVertical, mbArtItalic, mbArtBold );
}

//--------------------------------------------------------------------------

void PspFontLayout::DrawText( SalGraphics& ) const
{
    DrawPrinterLayout( *this, mrPrinterGfx );
}

#endif // _USE_PRINT_EXTENSION_

//--------------------------------------------------------------------------

void SalGraphicsData::DispatchServerFontString( const ServerFontLayout& rLayout )
{
    if( m_pPrinterGfx != NULL )
    {
        // print complex text
        DrawPrinterLayout( rLayout, *m_pPrinterGfx );
    }
    else
    {
        // draw complex text
        ServerFont& rFont = rLayout.GetServerFont();

        if( aX11GlyphPeer.GetGlyphSet( rFont ) )
            DrawServerAAFontString( rLayout );
        else if( aX11GlyphPeer.ForcedAntialiasing( rFont ) )
            DrawServerAAForcedString( rLayout );
        else
            DrawServerSimpleFontString( rLayout );
    }
}

//--------------------------------------------------------------------------

void ServerFontLayout::DrawText( SalGraphics& rSalGraphics ) const
{
    rSalGraphics.maGraphicsData.DispatchServerFontString( *this );
}

//--------------------------------------------------------------------------

static Point
RotatedPoint( Point &rOrigin, int nDx, int nAngle )
{
    Point   aPos( rOrigin.X() + nDx, rOrigin.Y() );
    Polygon aPolygon(1);
    aPolygon.SetPoint( aPos, 0 );
    aPolygon.Rotate( rOrigin, nAngle );
    return aPolygon.GetPoint( 0 );
}

//--------------------------------------------------------------------------

void SalGraphicsData::DrawServerFontString( const ServerFontLayout& rLayout )
{
    DispatchServerFontString( rLayout );
}

#endif // USE_BUILTIN_RASTERIZER

//--------------------------------------------------------------------------

void SalGraphicsData::DrawStringUCS2MB( ExtendedFontStruct& rFont,
    const Point& rPoint, const sal_Unicode* pStr, int nLength )
{
    Display* pDisplay   = GetXDisplay();
    GC       nGC        = SelectFont();

    if( rFont.GetAsciiEncoding() == RTL_TEXTENCODING_UNICODE )
    {
        // plain Unicode, can handle all chars and can be handled straight forward
        XFontStruct* pFontStruct = rFont.GetFontStruct( RTL_TEXTENCODING_UNICODE );
        if( !pFontStruct )
            return;

        XSetFont( pDisplay, nGC, pFontStruct->fid );

        #ifdef OSL_LITENDIAN
        sal_Unicode *pBuffer = (sal_Unicode*)alloca( nLength * sizeof(sal_Unicode) );
        for ( int i = 0; i < nLength; i++ )
            pBuffer[ i ] = SwapBytes(pStr[ i ]) ;
        #else
        sal_Unicode *pBuffer = const_cast<sal_Unicode*>(pStr);
        #endif

        XDrawString16( pDisplay, hDrawable_, nGC, rPoint.X(), rPoint.Y(), (XChar2b*)pBuffer, nLength );
    }
    else
    {
        XTextItem16 *pTextItem = (XTextItem16*)alloca( nLength * sizeof(XTextItem16) );
        XChar2b     *pMBChar   = (XChar2b*)pStr;

        for( int nChar = 0, nItem = 0; nChar < nLength; ++nChar )
        {
            rtl_TextEncoding  nEnc;
            XFontStruct* pFontStruct = rFont.GetFontStruct( pStr[nChar], &nEnc );
            if( !pFontStruct )
                continue;

            pTextItem[ nItem ].chars  = pMBChar + nChar;
            pTextItem[ nItem ].delta  = 0;
            pTextItem[ nItem ].font   = pFontStruct->fid;
            pTextItem[ nItem ].nchars = 1;

            ConvertTextItem16( &pTextItem[ nItem ], nEnc );
            ++nItem;
        }

        XDrawText16( pDisplay, hDrawable_, nGC, rPoint.X(), rPoint.Y(), pTextItem, nItem );
    }
}

//--------------------------------------------------------------------------

ULONG SalGraphicsData::GetFontCodeRanges( sal_uInt32* pCodePairs ) const

{
    ULONG nPairs = 0;
#ifdef USE_BUILTIN_RASTERIZER
    if( mpServerFont[0] )
        nPairs = mpServerFont[0]->GetFontCodeRanges( pCodePairs );
    else
#endif //USE_BUILTIN_RASTERIZER
    if( mXFont[0] )
        nPairs = mXFont[0]->GetFontCodeRanges( pCodePairs );
    return nPairs;
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

// ----------------------------------------------------------------------------
//
// SalGraphics
//
// ----------------------------------------------------------------------------

USHORT SalGraphics::SetFont( ImplFontSelectData *pEntry, int nFallbackLevel )
{
#ifndef _USE_PRINT_EXTENSION_
    if( (maGraphicsData.m_pPrinterGfx != NULL) )
    {
        sal_Bool bVertical = pEntry->mbVertical;
        sal_Int32 nID = pEntry->mpFontData ? (sal_Int32)pEntry->mpFontData->mpSysData : 0;

        bool bArtItalic = false;
        bool bArtBold = false;
        if( pEntry->meItalic == ITALIC_OBLIQUE || pEntry->meItalic == ITALIC_NORMAL )
        {
            psp::italic::type eItalic = maGraphicsData.m_pPrinterGfx->GetFontMgr().getFontItalic( nID );
            if( eItalic != psp::italic::Italic && eItalic != psp::italic::Oblique )
                bArtItalic = true;
        }
        int nWeight = (int)pEntry->meWeight;
        int nRealWeight = (int)maGraphicsData.m_pPrinterGfx->GetFontMgr().getFontWeight( nID );
        if( nRealWeight < nWeight-2 ||
            ( nRealWeight < (int)psp::weight::SemiBold && nWeight > (int)WEIGHT_NORMAL) )
        {
            bArtBold = true;
        }

        // also set the serverside font for layouting
        maGraphicsData.SetFont( pEntry, nFallbackLevel );

        // set the printer font
        return maGraphicsData.m_pPrinterGfx->SetFont( nID,
                                                      pEntry->mnHeight,
                                                      pEntry->mnWidth,
                                                      pEntry->mnOrientation,
                                                      pEntry->mbVertical,
                                                      bArtItalic,
                                                      bArtBold
                                                      );
    }
    else
#endif
    {
        USHORT nRetVal = 0;
        if( !maGraphicsData.SetFont( pEntry, nFallbackLevel ) )
            nRetVal |= SAL_SETFONT_BADFONT;
        if( _IsPrinter() || (maGraphicsData.mpServerFont[ nFallbackLevel ] != NULL) )
            nRetVal |= SAL_SETFONT_USEDRAWTEXTARRAY;
        return nRetVal;
    }
}

// ----------------------------------------------------------------------------

void SalGraphics::RemovingFont( ImplFontData* pFontData )
{
    // ImplFontData gets removed => notify all caches
    GlyphCache::GetInstance().RemoveFont( pFontData );
}

// ----------------------------------------------------------------------------

void
SalGraphics::SetTextColor( SalColor nSalColor )
{
#ifndef _USE_PRINT_EXTENSION_
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

#ifndef _USE_PRINT_EXTENSION_
    }
#endif
}

// ----------------------------------------------------------------------------

#ifndef _USE_PRINT_EXTENSION_

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

static void SetImplFontData( const psp::FastPrintFontInfo& aInfo, ImplFontData& rData )
{
    rData.meFamily      = ToFontFamily (aInfo.m_eFamilyStyle);
    rData.meWeight      = ToFontWeight (aInfo.m_eWeight);

    rData.meItalic      = ToFontItalic (aInfo.m_eItalic);
    rData.meWidthType   = ToFontWidth  (aInfo.m_eWidth);
    rData.mePitch       = ToFontPitch  (aInfo.m_ePitch);
    rData.meCharSet     = aInfo.m_aEncoding;
    rData.maName        = aInfo.m_aFamilyName;
    // rData.meScript       = SCRIPT_DONTKNOW;
    /*rData.maStyleName = XXX */

    rData.mnWidth       = 0;
    rData.mnHeight      = 0;
    rData.mbOrientation = TRUE;
    rData.mnQuality     = (aInfo.m_eType == psp::fonttype::Builtin ? 1024 : 0);
    rData.mnVerticalOrientation= 0;
    rData.meType        = TYPE_SCALABLE;
    rData.mbDevice      = (aInfo.m_eType == psp::fonttype::Builtin);
    String aMapNames;
    for( ::std::list< OUString >::const_iterator it = aInfo.m_aAliases.begin(); it != aInfo.m_aAliases.end(); ++it )
    {
        if( it != aInfo.m_aAliases.begin() )
            aMapNames.Append(';');
        aMapNames.Append( String( *it ) );
    }
    rData.maMapNames    = aMapNames;
    switch( aInfo.m_eType )
    {
        case psp::fonttype::TrueType:
            rData.mbSubsettable = TRUE;
            rData.mbEmbeddable  = FALSE;
            break;
        case psp::fonttype::Type1:
            rData.mbSubsettable = FALSE;
            rData.mbEmbeddable  = TRUE;
            break;
        default:
            rData.mbSubsettable = FALSE;
            rData.mbEmbeddable  = FALSE;
            break;
    }
}

#endif

// ----------------------------------------------------------------------------

ImplFontData* SalGraphics::AddTempDevFont( const String& rFileURL, const String& rFontName )
{
    // inform font manager
    rtl::OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFileURL, aUSystemPath ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aOFileName( OUStringToOString( aUSystemPath, aEncoding ) );
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    int nFontId = rMgr.addFontFile( aOFileName, 0 );
    if( !nFontId )
        return NULL;

    // prepare font data
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontFastInfo( nFontId, aInfo );
    ImplFontData* pFontData = new ImplFontData;
    SetImplFontData( aInfo, *pFontData );
    pFontData->maName = rFontName;
    pFontData->mnQuality += 5800;

    // inform glyph cache
    GlyphCache& rGC = GlyphCache::GetInstance();
    rGC.AddFontFile( rMgr.getFontFileSysPath( nFontId ), 0, nFontId, pFontData );
    pFontData->mpSysData = (void*)nFontId; // NOTE: don't do this at home

    return pFontData;
}

// ----------------------------------------------------------------------------

void SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
#ifndef _USE_PRINT_EXTENSION_
    if (maGraphicsData.m_pJobData != NULL)
    {
        const char* pLangBoost = NULL;
        const LanguageType aLang = Application::GetSettings().GetUILanguage();
        switch( aLang )
        {
            case LANGUAGE_JAPANESE:
                pLangBoost = "jan";    // japanese is default
                break;
            case LANGUAGE_CHINESE:
            case LANGUAGE_CHINESE_SIMPLIFIED:
            case LANGUAGE_CHINESE_SINGAPORE:
                pLangBoost = "zhs";
                break;
            case LANGUAGE_CHINESE_TRADITIONAL:
            case LANGUAGE_CHINESE_HONGKONG:
            case LANGUAGE_CHINESE_MACAU:
                pLangBoost = "zht";
                break;
            case LANGUAGE_KOREAN:
            case LANGUAGE_KOREAN_JOHAB:
                pLangBoost = "kor";
                break;
        }

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
                SetImplFontData( aInfo, *pFontData );
                pFontData->mpSysData = (void*)*it;
                if( pFontData->maName.CompareIgnoreCaseToAscii( "itc ", 4 ) == COMPARE_EQUAL )
                    pFontData->maName = pFontData->maName.Copy( 4 );
                if( aInfo.m_eType == psp::fonttype::TrueType )
                {
                    // prefer truetype fonts
                    pFontData->mnQuality += 10;
                    // asian type 1 fonts are not known
                    ByteString aFileName( rMgr.getFontFileSysPath( *it ) );
                    int nPos = aFileName.SearchBackward( '_' );
                    if( nPos == STRING_NOTFOUND || aFileName.GetChar( nPos+1 ) == '.' )
                        pFontData->mnQuality += 5;
                    else
                    {
                        if( pLangBoost && aFileName.Copy( nPos+1, 3 ).EqualsIgnoreCaseAscii( pLangBoost ) )
                            pFontData->mnQuality += 10;
                    }
                }
                pList->Add( pFontData );
            }
        }
    }
    else
#endif
    {
        XlfdStorage* pFonts = _GetDisplay()->GetXlfdList();

        for ( int nIdx = 0; nIdx < pFonts->GetCount(); nIdx++ )
        {
            ImplFontData *pFontData = new ImplFontData;
            pFonts->Get(nIdx)->ToImplFontData( pFontData );
            if( pFontData->maName.CompareIgnoreCaseToAscii( "itc ", 4 ) == COMPARE_EQUAL )
                pFontData->maName = pFontData->maName.Copy( 4 );
            pFontData->mbSubsettable    = FALSE;
            pFontData->mbEmbeddable     = FALSE;
            pList->Add( pFontData );
        }

#ifdef USE_BUILTIN_RASTERIZER
        aX11GlyphPeer.SetDisplay( maGraphicsData.GetXDisplay(),
            maGraphicsData.GetDisplay()->GetVisual()->GetVisual() );
#ifdef MACOSX
        GlyphCache::EnsureInstance( aX11GlyphPeer, true );
#else
        GlyphCache::EnsureInstance( aX11GlyphPeer, false );
#endif
        GlyphCache& rGC = GlyphCache::GetInstance();

        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        ::std::list< psp::fontID > aList;
        ::std::list< psp::fontID >::iterator it;
        rMgr.getFontList( aList );
        for( it = aList.begin(); it != aList.end(); ++it )
        {
            psp::FastPrintFontInfo aInfo;
            if( rMgr.getFontFastInfo( *it, aInfo ) )
            {
                if( aInfo.m_eType == psp::fonttype::Builtin )
                    continue;
                ImplFontData aFontData;
                SetImplFontData( aInfo, aFontData );
                // prefer builtin_rasterizer fonts
                aFontData.mnQuality += 4096;
                // prefer truetype fonts
                if( aInfo.m_eType == psp::fonttype::TrueType )
                    aFontData.mnQuality += 1000;
                int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
                if( aFontData.maName.CompareIgnoreCaseToAscii( "itc ", 4 ) == COMPARE_EQUAL )
                    aFontData.maName = aFontData.maName.Copy( 4 );
                if( nFaceNum < 0 )
                    nFaceNum = 0;
                rGC.AddFontFile( rMgr.getFontFileSysPath( aInfo.m_nID ), nFaceNum,
                    aInfo.m_nID, &aFontData );
            }
        }

        rGC.FetchFontList( pList );
#endif // USE_BUILTIN_RASTERIZER
    }
}

// ----------------------------------------------------------------------------

void SalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
#ifndef _USE_PRINT_EXTENSION_
    if( maGraphicsData.m_pPrinterGfx != NULL )
    {
        const psp::PrinterInfo& rInfo = psp::PrinterInfoManager::get().getPrinterInfo( maGraphicsData.m_pJobData->m_aPrinterName );
        if( rInfo.m_bPerformFontSubstitution )
        {
            for( std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator it = rInfo.m_aFontSubstitutes.begin(); it != rInfo.m_aFontSubstitutes.end(); ++it )
                pOutDev->ImplAddDevFontSubstitute( it->first, it->second, FONT_SUBSTITUTE_ALWAYS );
        }
    }
#endif
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
#ifndef _USE_PRINT_EXTENSION_
    if (maGraphicsData.m_pPrinterGfx != NULL)
    {
        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        psp::PrintFontInfo aInfo;

        if (rMgr.getFontInfo (maGraphicsData.m_pPrinterGfx->GetFontID(), aInfo))
        {
            sal_Int32 nTextHeight   = maGraphicsData.m_pPrinterGfx->GetFontHeight();
            sal_Int32 nTextWidth    = maGraphicsData.m_pPrinterGfx->GetFontWidth();
            if( ! nTextWidth )
                nTextWidth = nTextHeight;

            pMetric->mnOrientation  = maGraphicsData.m_pPrinterGfx->GetFontAngle();
            pMetric->mnSlant        = 0;
            pMetric->mbDevice       = aInfo.m_eType == psp::fonttype::Builtin ? sal_True : sal_False;

            pMetric->meCharSet      = aInfo.m_aEncoding;

            pMetric->meFamily       = ToFontFamily (aInfo.m_eFamilyStyle);
            pMetric->meWeight       = ToFontWeight (aInfo.m_eWeight);
            pMetric->mePitch        = ToFontPitch  (aInfo.m_ePitch);
            pMetric->meItalic       = ToFontItalic (aInfo.m_eItalic);
            pMetric->meType         = TYPE_SCALABLE;

            pMetric->mnFirstChar    =   0;
            pMetric->mnLastChar     = 255;

            pMetric->mnWidth        = nTextWidth;
            pMetric->mnAscent       = ( aInfo.m_nAscend * nTextHeight + 500 ) / 1000;
            pMetric->mnDescent      = ( aInfo.m_nDescend * nTextHeight + 500 ) / 1000;
            pMetric->mnLeading      = ( aInfo.m_nLeading * nTextHeight + 500 ) / 1000;
        }
        return;
    }
#endif

    if( maGraphicsData.mpServerFont[0] != NULL )
    {
        long rDummyFactor;
        maGraphicsData.mpServerFont[0]->FetchFontMetric( *pMetric, rDummyFactor );
        return;
    }

    ExtendedFontStruct* pFont = maGraphicsData.mXFont[0];
    if ( pFont != NULL )
    {
        pFont->ToImplFontMetricData( pMetric );
        if ( maGraphicsData.bFontVertical_ )
            pMetric->mnOrientation = 0;

        long n = maGraphicsData.aScale_.GetNumerator();
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

ULONG
SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData *pKernPairs )
{
    if( ! _IsPrinter() )
    {
        if( maGraphicsData.mpServerFont[0] != NULL )
        {
            ImplKernPairData* pTmpKernPairs;
            ULONG nGotPairs = maGraphicsData.mpServerFont[0]->GetKernPairs( &pTmpKernPairs );
            for( int i = 0; i < nPairs && i < nGotPairs; ++i )
                pKernPairs[ i ] = pTmpKernPairs[ i ];
            delete[] pTmpKernPairs;
            return nGotPairs;
        }
    }
    else
    {
#ifndef _USE_PRINT_EXTENSION_
        const ::std::list< ::psp::KernPair >& rPairs( maGraphicsData.m_pPrinterGfx->getKernPairs() );
        ULONG nHavePairs = rPairs.size();
        if( pKernPairs && nPairs )
        {
            ::std::list< ::psp::KernPair >::const_iterator it;
            int i;
            int nTextScale = maGraphicsData.m_pPrinterGfx->GetFontWidth();
            if( ! nTextScale )
                nTextScale = maGraphicsData.m_pPrinterGfx->GetFontHeight();
            for( i = 0, it = rPairs.begin(); i < nPairs && i < nHavePairs; i++, ++it )
            {
                pKernPairs[i].mnChar1   = it->first;
                pKernPairs[i].mnChar2   = it->second;
                pKernPairs[i].mnKern    = it->kern_x * nTextScale / 1000;
            }

        }
        return nHavePairs;
#endif
    }

    return 0;
}

// ---------------------------------------------------------------------------

ULONG SalGraphics::GetFontCodeRanges( sal_uInt32* pCodePairs ) const
{
    return maGraphicsData.GetFontCodeRanges( pCodePairs );
}

// ---------------------------------------------------------------------------

BOOL SalGraphics::GetGlyphBoundRect( long nGlyphIndex, Rectangle& rRect, const OutputDevice* )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = maGraphicsData.mpServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( nGlyphIndex );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return TRUE;
}

// ---------------------------------------------------------------------------

BOOL SalGraphics::GetGlyphOutline( long nGlyphIndex, PolyPolygon& rPolyPoly, const OutputDevice* )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = maGraphicsData.mpServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rPolyPoly ) )
        return TRUE;

    return FALSE;
}

//--------------------------------------------------------------------------

SalLayout* SalGraphicsData::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
#if !defined(_USE_PRINT_EXTENSION_)
    // workaround for printers not handling glyph indexing for non-TT fonts
    if( m_pPrinterGfx != NULL )
    {
        int nFontId = m_pPrinterGfx->GetFontID();
        if( psp::fonttype::TrueType != psp::PrintFontManager::get().getFontType( nFontId ) )
            rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
        else if( nFallbackLevel > 0 )
            rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
    }
#endif // !defined(_USE_PRINT_EXTENSION_)

    GenericSalLayout* pLayout = NULL;

    if( mpServerFont[ nFallbackLevel ]
    && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
        pLayout = m_pPrinterGfx ? new PspServerFontLayout( *m_pPrinterGfx, *mpServerFont[nFallbackLevel] ) : new ServerFontLayout( *mpServerFont[ nFallbackLevel ] );
#if !defined(_USE_PRINT_EXTENSION_)
    else if( m_pPrinterGfx != NULL )
        pLayout = new PspFontLayout( *m_pPrinterGfx );
#endif // !defined(_USE_PRINT_EXTENSION_)
    else if( mXFont[ nFallbackLevel ] )
        pLayout = new X11FontLayout( *mXFont[ nFallbackLevel ] );
    else
        pLayout = NULL;

    return pLayout;
}

//--------------------------------------------------------------------------

SalLayout* SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    SalLayout* pSalLayout = maGraphicsData.GetTextLayout( rArgs, nFallbackLevel );
    return pSalLayout;
}

//--------------------------------------------------------------------------

BOOL SalGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   ImplFontData* pFont,
                                   long* pGlyphIDs,
                                   sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphs,
                                   FontSubsetInfo& rInfo
                                   )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the sysdata member of pFont should
    // contain a fontID as the X fonts should be filtered
    // out of the font list available to PDF export (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = (psp::fontID)pFont->mpSysData;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;

    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
        return FALSE;

    // fill in font info
    switch( aFontInfo.m_eType )
    {
        case psp::fonttype::TrueType: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;break;
        case psp::fonttype::Type1: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;break;
        default:
            return FALSE;
    }
    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = rMgr.getPSName( aFont );

    int xMin, yMin, xMax, yMax;
    rMgr.getFontBoundingBox( aFont, xMin, yMin, xMax, yMax );

    if( ! rMgr.createFontSubset( aFont,
                                 rToFile,
                                 pGlyphIDs,
                                 pEncoding,
                                 pWidths,
                                 nGlyphs
                                 ) )
        return FALSE;

    rInfo.m_aFontBBox   = Rectangle( Point( xMin, yMin ), Size( xMax-xMin, yMax-yMin ) );
    rInfo.m_nCapHeight  = yMax; // Well ...

    return TRUE;
#else
    return FALSE;
#endif
}

//--------------------------------------------------------------------------

const void* SalGraphics::GetEmbedFontData( ImplFontData* pFont, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the sysdata member of pFont should
    // contain a fontID as the X fonts should be filtered
    // out of the font list available to PDF export (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = (psp::fontID)pFont->mpSysData;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;
    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
        return NULL;

    // fill in font info
    switch( aFontInfo.m_eType )
    {
        case psp::fonttype::TrueType: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;break;
        case psp::fonttype::Type1: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;break;
        default:
            return NULL;
    }
    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = rMgr.getPSName( aFont );

    int xMin, yMin, xMax, yMax;
    rMgr.getFontBoundingBox( aFont, xMin, yMin, xMax, yMax );

    psp::CharacterMetric aMetrics[256];
    sal_Unicode nFirstChar = 0;
    sal_Unicode nLastChar = 255;
    if( aFontInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL && aFontInfo.m_eType == psp::fonttype::Type1 )
    {
        nFirstChar = 0xf000;
        nLastChar = 0xf0ff;
    }
    if( ! rMgr.getMetrics( aFont, nFirstChar, nLastChar, aMetrics ) )
        return NULL;

    OString aSysPath = rMgr.getFontFileSysPath( aFont );
    struct stat aStat;
    if( stat( aSysPath.getStr(), &aStat ) )
        return NULL;
    int fd = open( aSysPath.getStr(), O_RDONLY );
    if( fd < 0 )
        return NULL;
    void* pFile = mmap( NULL, aStat.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );
    if( pFile == MAP_FAILED )
        return NULL;

    *pDataLen = aStat.st_size;

    rInfo.m_aFontBBox   = Rectangle( Point( xMin, yMin ), Size( xMax-xMin, yMax-yMin ) );
    rInfo.m_nCapHeight  = yMax; // Well ...

    for( int i = 0; i < 256; i++ )

        pWidths[i] = (aMetrics[i].width > 0 ? aMetrics[i].width : 0);

    return pFile;

#else
    return NULL;
#endif
}

//--------------------------------------------------------------------------

void SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    munmap( (char*)pData, nLen );
}

//--------------------------------------------------------------------------

const std::map< sal_Unicode, sal_Int32 >* SalGraphics::GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the sysdata member of pFont should
    // contain a fontID as the X fonts should be filtered
    // out of the font list available to PDF export (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = (psp::fontID)pFont->mpSysData;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;
    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
    {
        if( pNonEncoded )
            *pNonEncoded = NULL;
        return NULL;
    }

    return rMgr.getEncodingMap( aFont, pNonEncoded );
#else
    return NULL;
#endif
}

// ===========================================================================
