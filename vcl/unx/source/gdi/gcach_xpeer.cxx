/*************************************************************************
 *
 *  $RCSfile: gcach_xpeer.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 13:57:11 $
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

#include <rtl/ustring.hxx>
#include <osl/module.h>
using namespace rtl;

#include <gcach_xpeer.hxx>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------

static int nRenderVersion = 0x00;

X11GlyphPeer::X11GlyphPeer()
:   mpDisplay(NULL)
,   mbForcedAA(false)
,   mbUsingXRender(false)
,   mpGlyphFormat(NULL)
{
    maRawBitmap.mnAllocated = 0;
    maRawBitmap.mpBits = NULL;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::SetDisplay( Display* _pDisplay, Visual* _pVisual )
{
    if( mpDisplay == _pDisplay )
        return;

    mpDisplay = _pDisplay;

    int nEnvAntiAlias = 0;
    const char* pEnvAntiAlias = getenv( "SAL_ANTIALIAS_DISABLE" );
    if( pEnvAntiAlias )
    {
        nEnvAntiAlias = atoi( pEnvAntiAlias );
        if( nEnvAntiAlias == 0 )
            return;
    }

    // we can do anti aliasing on the client side
    // when the display's visuals are suitable
    mbForcedAA = true;
    XVisualInfo aXVisualInfo;
    aXVisualInfo.visualid = _pVisual->visualid;
    int nVisuals = 0;
    XVisualInfo* pXVisualInfo = XGetVisualInfo( mpDisplay, VisualIDMask, &aXVisualInfo, &nVisuals );
    int nMaxDepth = 0;
    for( int i = nVisuals; --i >= 0; )
    {
        if( nMaxDepth < pXVisualInfo[i].depth )
            nMaxDepth = pXVisualInfo[i].depth;
        if( ((pXVisualInfo[i].c_class==PseudoColor) || (pXVisualInfo[i].depth<24))
        && ((pXVisualInfo[i].c_class>GrayScale) || (pXVisualInfo[i].depth!=8) ) )
            mbForcedAA = false;
    }
    if( pXVisualInfo != NULL )
        XFree( pXVisualInfo );

    if( (nEnvAntiAlias & 1) != 0 )
        mbForcedAA = false;

    // but we prefer the hardware accelerated solution
    int nDummy;
    if( !XQueryExtension( mpDisplay, "RENDER", &nDummy, &nDummy, &nDummy ) )
        return;

    // we don't know if we are running on a system with xrender library
    // we don't want to install system libraries ourselves
    // => load them dynamically when they are there
#ifdef MACOSX
    OUString xrenderLibraryName( RTL_CONSTASCII_USTRINGPARAM( "libXrender.dylib" ));
#else
    OUString xrenderLibraryName( RTL_CONSTASCII_USTRINGPARAM( "libXrender.so.1" ));
#endif
    oslModule pRenderLib=osl_loadModule(xrenderLibraryName.pData, SAL_LOADMODULE_DEFAULT);
    if( !pRenderLib ) {
#ifdef DEBUG
        fprintf( stderr, "Display can do XRender, but no %s installed.\n"
            "Please install for improved display performance\n", xrenderLibraryName.getStr() );
#endif
        return;
    }

    OUString queryExtensionFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderQueryExtension"));
    void *pFunc;
    pFunc=osl_getSymbol(pRenderLib, queryExtensionFuncName.pData);
    if( !pFunc ) return;
    pXRenderQueryExtension          = (Bool(*)(Display*,int*,int*))pFunc;

    OUString queryVersionFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderQueryVersion"));
    pFunc=osl_getSymbol(pRenderLib, queryVersionFuncName.pData);
    if( !pFunc ) return;
    pXRenderQueryVersion            = (void(*)(Display*,int*,int*))pFunc;

    OUString visFormatFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFindVisualFormat"));
    pFunc=osl_getSymbol(pRenderLib, visFormatFuncName.pData);
    if( !pFunc ) return;
    pXRenderFindVisualFormat        = (XRenderPictFormat*(*)(Display*,Visual*))pFunc;

    OUString fmtFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFindFormat"));
    pFunc=osl_getSymbol(pRenderLib, fmtFuncName.pData);
    if( !pFunc ) return;
    pXRenderFindFormat              = (XRenderPictFormat*(*)(Display*,unsigned long,XRenderPictFormat*,int))pFunc;

    OUString creatGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCreateGlyphSet"));
    pFunc=osl_getSymbol(pRenderLib, creatGlyphFuncName.pData);
    if( !pFunc ) return;
    pXRenderCreateGlyphSet          = (GlyphSet(*)(Display*,XRenderPictFormat*))pFunc;

    OUString freeGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreeGlyphSet"));
    pFunc=osl_getSymbol(pRenderLib, freeGlyphFuncName.pData);
    if( !pFunc ) return;
    pXRenderFreeGlyphSet            = (void(*)(Display*,GlyphSet))pFunc;

    OUString addGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderAddGlyphs"));
    pFunc=osl_getSymbol(pRenderLib, addGlyphFuncName.pData);
    if( !pFunc ) return;
    pXRenderAddGlyphs               = (void(*)(Display*,GlyphSet,Glyph*,XGlyphInfo*,int,char*,int))pFunc;

    OUString freeGlyphsFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreeGlyphs"));
    pFunc=osl_getSymbol(pRenderLib, freeGlyphsFuncName.pData);
    if( !pFunc ) return;
    pXRenderFreeGlyphs              = (void(*)(Display*,GlyphSet,Glyph*,int))pFunc;

    OUString compStringFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCompositeString32"));
    pFunc=osl_getSymbol(pRenderLib, compStringFuncName.pData);
    if( !pFunc ) return;
    pXRenderCompositeString32       = (void(*)(Display*,int,Picture,Picture,XRenderPictFormat*,GlyphSet,int,int,int,int,unsigned*,int))pFunc;

    OUString creatPicFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCreatePicture"));
    pFunc=osl_getSymbol(pRenderLib, creatPicFuncName.pData);
    if( !pFunc ) return;
    pXRenderCreatePicture           = (Picture(*)(Display*,Drawable,XRenderPictFormat*,unsigned long,XRenderPictureAttributes*))pFunc;

    OUString setClipFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderSetPictureClipRegion"));
    pFunc=osl_getSymbol(pRenderLib, setClipFuncName.pData);
    if( !pFunc ) return;
    pXRenderSetPictureClipRegion    = (void(*)(Display*,Picture,XLIB_Region))pFunc;

    OUString freePicFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreePicture"));
    pFunc=osl_getSymbol(pRenderLib, freePicFuncName.pData);
    if( !pFunc ) return;
    pXRenderFreePicture             = (void(*)(Display*,Picture))pFunc;

    // needed to initialize libXrender internals, we already know its there
    (*pXRenderQueryExtension)( mpDisplay, &nDummy, &nDummy );

    int nMajor, nMinor;
    (*pXRenderQueryVersion)( mpDisplay, &nMajor, &nMinor );
    nRenderVersion = 16*nMajor + nMinor;
    // TODO: enable/disable things depending on version

    // the 8bit alpha mask format must be there
    XRenderPictFormat aPictFormat={0,0,8,{0,0,0,0,0,0,0,0xFF},0};
    mpGlyphFormat = (*pXRenderFindFormat)( mpDisplay,
        PictFormatAlphaMask|PictFormatDepth, &aPictFormat, 0 );

    if( mpGlyphFormat != NULL )
    {
        // and the visual must be supported too
        XRenderPictFormat* pVisualFormat = (*pXRenderFindVisualFormat)( mpDisplay, _pVisual );
        if( pVisualFormat != NULL )
            mbUsingXRender = true;
    }

    // #97763# disable XRENDER on <15bit displays for XFree<=4.2.0
    if( (nMaxDepth < 15) && (nRenderVersion <= 0x02) )
        mbUsingXRender = false;

    // #93033# disable XRENDER for old RENDER versions if XINERAMA is present
    if( (nRenderVersion < 0x02)
    &&  XQueryExtension( mpDisplay, "XINERAMA", &nDummy, &nDummy, &nDummy ) )
        mbUsingXRender = false;

    if( (nEnvAntiAlias & 2) != 0 )
        mbUsingXRender = false;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::RemovingFont( ServerFont& rServerFont )
{
    switch( rServerFont.GetExtInfo() )
    {
        case PIXMAP_KIND:
        case AAFORCED_KIND:
            break;

        case XRENDER_KIND:
            (*pXRenderFreeGlyphSet)( mpDisplay,(GlyphSet)rServerFont.GetExtPointer() );
            break;
    }

    rServerFont.SetExtended( EMPTY_KIND, NULL );
}

// ---------------------------------------------------------------------------

// notification to clean up GlyphPeer resources for this glyph
void X11GlyphPeer::RemovingGlyph( ServerFont& rServerFont, GlyphData& rGlyphData, int nGlyphIndex )
{
    // nothing to do if the GlyphPeer hasn't allocated resources for the glyph
    if( rGlyphData.GetExtInfo() == EMPTY_KIND )
        return;

    const GlyphMetric& rGM = rGlyphData.GetMetric();
    const int nWidth = rGM.GetSize().Width();
    const int nHeight = rGM.GetSize().Height();

    switch( rServerFont.GetExtInfo() )
    {
        case PIXMAP_KIND:
            {
                Pixmap aPixmap = (Pixmap)rServerFont.GetExtPointer();
                if( aPixmap != None )
                {
                    XFreePixmap( mpDisplay, aPixmap );
                    mnBytesUsed -= nHeight * ((nWidth + 7) >> 3);
                }
            }
            break;

        case AAFORCED_KIND:
            {
                RawBitmap* pRawBitmap = (RawBitmap*)rGlyphData.GetExtPointer();
                if( pRawBitmap != NULL )
                {
                    mnBytesUsed -= pRawBitmap->mnScanlineSize * pRawBitmap->mnHeight;
                    mnBytesUsed -= sizeof(RawBitmap);
                    delete pRawBitmap;
                }
            }
            break;

        case XRENDER_KIND:
            {
                Glyph nGlyphId = (Glyph)rGlyphData.GetExtPointer();
                // XRenderFreeGlyphs not implemented yet for version<=0.2
                // #108209# disabled because of crash potential,
                // the glyph leak is not too bad because they will
                // be cleaned up when the glyphset is released
#if 0   // TODO: reenable when it works without problems
                if( nRenderVersion >= 0x05 )
                    (*pXRenderFreeGlyphs)( mpDisplay, aGlyphSet, &nGlyphId, 1 );
#endif
                mnBytesUsed -= nHeight * ((nWidth + 3) & ~3);
            }
            break;
    }

    if( mnBytesUsed < 0 )   // TODO: eliminate nBytesUsed calc mismatch
        mnBytesUsed = 0;

    rGlyphData.SetExtended( EMPTY_KIND, NULL );
}

// ---------------------------------------------------------------------------

bool X11GlyphPeer::ForcedAntialiasing( const ServerFont& rServerFont ) const
{
    bool bForceOk = rServerFont.GetAntialiasAdvice();
    // maximum size for antialiasing is 250 pixels
    bForceOk &= (rServerFont.GetFontSelData().mnHeight < 250);
    return (bForceOk && mbForcedAA);
}

// ---------------------------------------------------------------------------

GlyphSet X11GlyphPeer::GetGlyphSet( ServerFont& rServerFont )
{
    if( !mbUsingXRender )
        return NULL;

    GlyphSet aGlyphSet;

    switch( rServerFont.GetExtInfo() )
    {
        case XRENDER_KIND:
            aGlyphSet = (GlyphSet)rServerFont.GetExtPointer();
            break;

        case EMPTY_KIND:
            {
                // antialiasing for reasonable font heights only
                // => prevents crashes caused by X11 requests >= 256k
                // => prefer readablity of hinted glyphs at small sizes
                // => prefer "grey clouds" to "black clouds" at very small sizes
                int nHeight = rServerFont.GetFontSelData().mnHeight;
                if( nHeight<250 && rServerFont.GetAntialiasAdvice() )
                {
                    aGlyphSet = (*pXRenderCreateGlyphSet)( mpDisplay, mpGlyphFormat );
                    rServerFont.SetExtended( XRENDER_KIND, (void*)aGlyphSet );
                }
                else
                    aGlyphSet = NULL;
            }
            break;

        default:
            aGlyphSet = NULL;
            break;
    }

    return aGlyphSet;
}

// ---------------------------------------------------------------------------

Pixmap X11GlyphPeer::GetPixmap( ServerFont& rServerFont, int nGlyphIndex )
{
    Pixmap aPixmap = None;
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    if( rGlyphData.GetExtInfo() == PIXMAP_KIND )
        aPixmap = (Pixmap)rGlyphData.GetExtPointer();
    else
    {
        if( rServerFont.GetGlyphBitmap1( nGlyphIndex, maRawBitmap ) )
        {
            // #94666# circumvent bug in some X11 systems, e.g. XF410.LynxEM.v163
            ULONG nPixmapWidth = 8 * maRawBitmap.mnScanlineSize - 1;
            nPixmapWidth = std::max( nPixmapWidth, maRawBitmap.mnWidth );

            rGlyphData.SetSize( Size( nPixmapWidth, maRawBitmap.mnHeight ) );
            rGlyphData.SetOffset( +maRawBitmap.mnXOffset, +maRawBitmap.mnYOffset );

            const ULONG nBytes = maRawBitmap.mnHeight * maRawBitmap.mnScanlineSize;
            if( nBytes > 0 )
            {
                // conversion table LSB<->MSB (for XCreatePixmapFromData)
                static const unsigned char lsb2msb[256] =
                {
                    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
                    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
                    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
                    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
                    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
                    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
                    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
                    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
                    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
                    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
                    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
                    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
                    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
                    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
                    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
                    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
                    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
                };

                unsigned char* pTemp = maRawBitmap.mpBits;
                for( int i = nBytes; --i >= 0; ++pTemp )
                    *pTemp = lsb2msb[ *pTemp ];

                 aPixmap = XCreatePixmapFromBitmapData( mpDisplay,
                     DefaultRootWindow( mpDisplay ), (char*)maRawBitmap.mpBits,
                     nPixmapWidth, maRawBitmap.mnHeight, 1, 0, 1 );
                 mnBytesUsed += nBytes;
            }
        }
        else
        {
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                aPixmap = GetPixmap( rServerFont, 0 );
        }

        rGlyphData.SetExtended( PIXMAP_KIND, (void*)aPixmap );
    }

    return aPixmap;
}

// ---------------------------------------------------------------------------

const RawBitmap* X11GlyphPeer::GetRawBitmap( ServerFont& rServerFont,
    int nGlyphIndex )
{
    const RawBitmap* pRawBitmap = NULL;
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    if( rGlyphData.GetExtInfo() == AAFORCED_KIND )
        pRawBitmap = (RawBitmap*)rGlyphData.GetExtPointer();
    else
    {
        RawBitmap* pNewBitmap = new RawBitmap;
        if( rServerFont.GetGlyphBitmap8( nGlyphIndex, *pNewBitmap ) )
        {
            pRawBitmap = pNewBitmap;
            mnBytesUsed += pRawBitmap->mnScanlineSize * pRawBitmap->mnHeight;
            mnBytesUsed += sizeof(RawBitmap);
        }
        else
        {
            delete pNewBitmap;
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                pRawBitmap = GetRawBitmap( rServerFont, 0 );
        }

        rGlyphData.SetExtended( AAFORCED_KIND, (void*)pRawBitmap );
    }

    return pRawBitmap;
}

// ---------------------------------------------------------------------------

Glyph X11GlyphPeer::GetGlyphId( ServerFont& rServerFont, int nGlyphIndex )
{
    Glyph aGlyphId = 0;
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    if( rGlyphData.GetExtInfo() == XRENDER_KIND )
        aGlyphId = (Glyph)rGlyphData.GetExtPointer();
    else
    {
        // prepare GlyphInfo and Bitmap
        if( rServerFont.GetGlyphBitmap8( nGlyphIndex, maRawBitmap ) )
        {
            XGlyphInfo aGlyphInfo;
            aGlyphInfo.width    = maRawBitmap.mnWidth;
            aGlyphInfo.height   = maRawBitmap.mnHeight;
            aGlyphInfo.x        = -maRawBitmap.mnXOffset;
            aGlyphInfo.y        = -maRawBitmap.mnYOffset;

            rGlyphData.SetSize( Size( maRawBitmap.mnWidth, maRawBitmap.mnHeight ) );
            rGlyphData.SetOffset( +maRawBitmap.mnXOffset, +maRawBitmap.mnYOffset );

            const GlyphMetric& rGM = rGlyphData.GetMetric();
            aGlyphInfo.xOff     = +rGM.GetDelta().X();
            aGlyphInfo.yOff     = +rGM.GetDelta().Y();

            // upload glyph bitmap to server
            GlyphSet aGlyphSet = GetGlyphSet( rServerFont );

            aGlyphId = nGlyphIndex & 0x00FFFFFF;
            const ULONG nBytes = maRawBitmap.mnScanlineSize * maRawBitmap.mnHeight;
            (*pXRenderAddGlyphs)( mpDisplay, aGlyphSet, &aGlyphId, &aGlyphInfo, 1,
                (char*)maRawBitmap.mpBits, nBytes );
            mnBytesUsed += nBytes;
        }
        else
        {
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                aGlyphId = GetGlyphId( rServerFont, 0 );
        }

        rGlyphData.SetExtended( XRENDER_KIND, (void*)aGlyphId );
    }

    return aGlyphId;
}
