/*************************************************************************
 *
 *  $RCSfile: gcach_xpeer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hdu $ $Date: 2001-02-27 18:39:01 $
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

#include <X11/Xlib.h>
#include <gcach_xpeer.hxx>
#include <stdlib.h>

#ifdef USE_XRENDER
    #include <dlfcn.h>
#endif // USE_XRENDER

// ---------------------------------------------------------------------------

X11GlyphPeer::X11GlyphPeer()
:   mpDisplay(NULL)
#ifdef USE_XRENDER
,   mbUsingXRender(false)
,   mpGlyphFormat(NULL)
#endif // USE_XRENDER
{
    maRawBitmap.mnAllocated = 0;
    maRawBitmap.mpBits = NULL;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::SetDisplay( Display* _pDisplay )
{
    if( mpDisplay == _pDisplay )
        return;

    mpDisplay = _pDisplay;

#ifdef USE_XRENDER
    if( getenv("SAL_ANTIALIAS_DISABLE") )
        return;

    int nDummy;
    if( !XQueryExtension( mpDisplay, "RENDER", &nDummy, &nDummy, &nDummy ) )
        return;

    // we don't know if we are running on a system with xrender library
    // we don't want to install system libraries ourselves
    // => load them dynamically when they are there
    void* pRenderLib = dlopen( "libXrender.so", RTLD_GLOBAL | RTLD_LAZY );
    if( !pRenderLib ) {
        printf( "XRender extension but no libXrender.so installed. Please install for improved display quality\n" );
        return;
    }

    void* pFunc;
    pFunc = dlsym( pRenderLib, "XRenderQueryExtension" );
    if( !pFunc ) return;
    pXRenderQueryExtension          = (Bool(*)(Display*,int*,int*))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderQueryVersion" );
    if( !pFunc ) return;
    pXRenderQueryVersion            = (void(*)(Display*,int*,int*))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderFindVisualFormat" );
    if( !pFunc ) return;
    pXRenderFindVisualFormat    = (XRenderPictFormat*(*)(Display*,Visual*))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderFindFormat" );
    if( !pFunc ) return;
    pXRenderFindFormat          = (XRenderPictFormat*(*)(Display*,unsigned long,XRenderPictFormat*,int))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderCreateGlyphSet" );
    if( !pFunc ) return;
    pXRenderCreateGlyphSet          = (GlyphSet(*)(Display*,XRenderPictFormat*))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderFreeGlyphSet" );
    if( !pFunc ) return;
    pXRenderFreeGlyphSet            = (void(*)(Display*,GlyphSet))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderAddGlyphs" );
    if( !pFunc ) return;
    pXRenderAddGlyphs               = (void(*)(Display*,GlyphSet,Glyph*,XGlyphInfo*,int,char*,int))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderCompositeString16" );
    if( !pFunc ) return;
    pXRenderCompositeString16       = (void(*)(Display*,int,Picture,Picture,XRenderPictFormat*,GlyphSet,int,int,int,int,unsigned short*,int))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderCreatePicture" );
    if( !pFunc ) return;
    pXRenderCreatePicture           = (Picture(*)(Display*,Drawable,XRenderPictFormat*,unsigned long,XRenderPictureAttributes*))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderSetPictureClipRegion" );
    if( !pFunc ) return;
    pXRenderSetPictureClipRegion    = (void(*)(Display*,Picture,XLIB_Region))pFunc;
    pFunc = dlsym( pRenderLib, "XRenderFreePicture" );
    if( !pFunc ) return;
    pXRenderFreePicture             = (void(*)(Display*,Picture))pFunc;

    // needed to initialize libXrender internals, we already know its there
    (*pXRenderQueryExtension)( mpDisplay, &nDummy, &nDummy );

    int nMajor, nMinor;
    (*pXRenderQueryVersion)( mpDisplay, &nMajor, &nMinor );
    // TODO: enabling/disabling things depending on version

    // the 8bit alpha mask format must be there
    XRenderPictFormat aPictFormat={0,0,8,{0,0,0,0,0,0,0,0xFF},0};
    mpGlyphFormat = (*pXRenderFindFormat)( mpDisplay, 0, &aPictFormat, 1 );

    // and support for the visual
    Visual* pVisual = DefaultVisual( mpDisplay, 0 );
    XRenderPictFormat*  pVisualFormat =  (*pXRenderFindVisualFormat)( mpDisplay, pVisual );
    if( pVisualFormat != NULL )
        mbUsingXRender = true;
#endif // USE_XRENDER
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::RemovingFont( ServerFont& rServerFont )
{
    switch( rServerFont.GetExtInfo() )
    {
        case PIXMAP_KIND:
            break;

#ifdef USE_XRENDER
        case XRENDER_KIND:
            (*pXRenderFreeGlyphSet)( mpDisplay,(GlyphSet)rServerFont.GetExtPointer() );
            break;
#endif // USE_XRENDER
    }

    rServerFont.SetExtended( EMPTY_KIND, NULL );
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::RemovingGlyph( ServerFont& rServerFont, GlyphData& rGlyphData, int nGlyphIndex )
{
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

#ifdef USE_XRENDER
        case XRENDER_KIND:
        {
            GlyphSet aGlyphSet = GetGlyphSet( rServerFont );
            Glyph nGlyphId = GetGlyphId( rServerFont, nGlyphIndex );
            // current version of XRENDER does not implement XRenderFreeGlyphs()
//###       (*pXRenderFreeGlyphs)( mpDisplay, aGlyphSet, &nGlyphId, 1 );
            mnBytesUsed -= nHeight * ((nWidth + 3) & ~3);
            break;
        }
#endif // USE_XRENDER
    }

    if( mnBytesUsed < 0 )   // TODO: eliminate nBytesUsed calc mismatch
        mnBytesUsed = 0;

    rGlyphData.SetExtended( EMPTY_KIND, NULL );
}

// ---------------------------------------------------------------------------

#ifdef USE_XRENDER
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
                const ImplFontSelectData& rFSD = rServerFont.GetFontSelData();
                if( rFSD.mnHeight<250  && (rFSD.mnHeight>=12 || rFSD.mnHeight<8) )
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
#endif // USE_XRENDER

// ---------------------------------------------------------------------------

Pixmap X11GlyphPeer::GetPixmap( ServerFont& rServerFont, int nGlyphIndex )
{
    Pixmap aPixmap = None;
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    if( ( rGlyphData.GetExtInfo() == EMPTY_KIND ) )
    {
        if( rServerFont.GetGlyphBitmap1( nGlyphIndex, maRawBitmap ) )
        {
            rGlyphData.SetSize( Size( maRawBitmap.mnWidth, maRawBitmap.mnHeight ) );
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
                    maRawBitmap.mnWidth, maRawBitmap.mnHeight, 0, 1, 1 );
                mnBytesUsed += nBytes;
            }
        }

        rGlyphData.SetExtended( PIXMAP_KIND, (void*)aPixmap );
    }
    else
        aPixmap = (Pixmap)rGlyphData.GetExtPointer();

    return aPixmap;
}

// ---------------------------------------------------------------------------

#ifdef USE_XRENDER
Glyph X11GlyphPeer::GetGlyphId( ServerFont& rServerFont, int nGlyphIndex )
{
    Glyph aGlyphId;
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    if( rGlyphData.GetExtInfo() == XRENDER_KIND )
        aGlyphId = (Glyph)rGlyphData.GetExtPointer();
    else
    {
        // prepare GlyphInfo and Bitmap
        if( !rServerFont.GetGlyphBitmap8( nGlyphIndex, maRawBitmap ) )
            return GetGlyphId( rServerFont, 0 );

        XGlyphInfo aGlyphInfo;
        aGlyphInfo.width    = maRawBitmap.mnWidth;
        aGlyphInfo.height   = maRawBitmap.mnHeight;
        aGlyphInfo.x        = -maRawBitmap.mnXOffset;
        aGlyphInfo.y        = -maRawBitmap.mnYOffset;

        const GlyphMetric& rGM  = rGlyphData.GetMetric();

        aGlyphInfo.xOff     = +rGM.GetDelta().X();
        aGlyphInfo.yOff     = +rGM.GetDelta().Y();

        // upload glyph bitmap to server
        GlyphSet aGlyphSet = GetGlyphSet( rServerFont );

        aGlyphId = nGlyphIndex;
        const ULONG nBytes = maRawBitmap.mnScanlineSize * maRawBitmap.mnHeight;
        (*pXRenderAddGlyphs)( mpDisplay, aGlyphSet, &aGlyphId, &aGlyphInfo, 1,
            maRawBitmap.mpBits,  nBytes );

        mnBytesUsed += nBytes;
        rGlyphData.SetExtended( XRENDER_KIND, (void*)aGlyphId );
    }

    return aGlyphId;
}
#endif // USE_XRENDER
