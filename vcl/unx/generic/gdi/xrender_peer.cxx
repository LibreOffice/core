/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdio.h>

#include <rtl/ustring.hxx>
#include <osl/module.h>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>

#include <xrender_peer.hxx>

using namespace rtl;

// ---------------------------------------------------------------------------

XRenderPeer::XRenderPeer()
:   mpDisplay( GetX11SalData()->GetDisplay()->GetDisplay() ),
    mpStandardFormatA8( NULL ),
    mnRenderVersion( 0 ),
    mpRenderLib( NULL )
#ifndef XRENDER_LINK
,   mpXRenderCompositeTrapezoids( NULL )
,   mpXRenderAddTraps( NULL )
#endif // XRENDER_LINK
{
    InitRenderLib();
}

// ---------------------------------------------------------------------------

XRenderPeer::~XRenderPeer()
{
    osl_unloadModule( mpRenderLib );
}

// ---------------------------------------------------------------------------

XRenderPeer& XRenderPeer::GetInstance()
{
    static XRenderPeer aPeer;
    return aPeer;
}

// ---------------------------------------------------------------------------

void XRenderPeer::InitRenderLib()
{
    int nDummy;
    if( !XQueryExtension( mpDisplay, "RENDER", &nDummy, &nDummy, &nDummy ) )
        return;

#ifndef XRENDER_LINK
    // we don't know if we are running on a system with xrender library
    // we don't want to install system libraries ourselves
    // => load them dynamically when they are there
    const OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libXrender.so.1" ));
    mpRenderLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_DEFAULT );
    if( !mpRenderLib ) {
#ifdef DEBUG
        fprintf( stderr, "Display can do XRender, but no %s installed.\n"
            "Please install for improved display performance\n", OUStringToOString( aLibName.getStr(),
                                                                                    osl_getThreadTextEncoding() ).getStr() );
#endif
        return;
    }

    oslGenericFunction pFunc;
    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderQueryExtension" );
    if( !pFunc ) return;
    mpXRenderQueryExtension = (Bool(*)(Display*,int*,int*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderQueryVersion" );
    if( !pFunc ) return;
    mpXRenderQueryVersion = (void(*)(Display*,int*,int*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFindVisualFormat" );
    if( !pFunc ) return;
    mpXRenderFindVisualFormat = (XRenderPictFormat*(*)(Display*,Visual*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFindStandardFormat" );
    if( !pFunc ) return;
    mpXRenderFindStandardFormat = (XRenderPictFormat*(*)(Display*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFindFormat" );
    if( !pFunc ) return;
    mpXRenderFindFormat = (XRenderPictFormat*(*)(Display*,unsigned long,
        const XRenderPictFormat*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderCreateGlyphSet" );
    if( !pFunc ) return;
    mpXRenderCreateGlyphSet = (GlyphSet(*)(Display*,const XRenderPictFormat*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFreeGlyphSet" );
    if( !pFunc ) return;
    mpXRenderFreeGlyphSet = (void(*)(Display*,GlyphSet))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderAddGlyphs" );
    if( !pFunc ) return;
    mpXRenderAddGlyphs = (void(*)(Display*,GlyphSet,Glyph*,const XGlyphInfo*,
        int,const char*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFreeGlyphs" );
    if( !pFunc ) return;
    mpXRenderFreeGlyphs = (void(*)(Display*,GlyphSet,Glyph*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderCompositeString32" );
    if( !pFunc ) return;
    mpXRenderCompositeString32 = (void(*)(Display*,int,Picture,Picture,
        const XRenderPictFormat*,GlyphSet,int,int,int,int,const unsigned*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderCreatePicture" );
    if( !pFunc ) return;
    mpXRenderCreatePicture = (Picture(*)(Display*,Drawable,const XRenderPictFormat*,
        unsigned long,const XRenderPictureAttributes*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderChangePicture" );
    if( !pFunc ) return;
    mpXRenderChangePicture = (void(*)(Display*,Picture,unsigned long,const XRenderPictureAttributes*))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderSetPictureClipRegion" );
    if( !pFunc ) return;
    mpXRenderSetPictureClipRegion = (void(*)(Display*,Picture,XLIB_Region))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFreePicture" );
    if( !pFunc ) return;
    mpXRenderFreePicture = (void(*)(Display*,Picture))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderComposite" );
    if( !pFunc ) return;
    mpXRenderComposite = (void(*)(Display*,int,Picture,Picture,Picture,
        int,int,int,int,int,int,unsigned,unsigned))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderFillRectangle" );
    if( !pFunc ) return;
    mpXRenderFillRectangle = (void(*)(Display*,int,Picture,const XRenderColor*,
        int,int,unsigned int,unsigned int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderCompositeTrapezoids" );
#if 0 // not having trapezoid support is supported
    if( !pFunc ) return;
#endif
    mpXRenderCompositeTrapezoids = (void(*)(Display*,int,Picture,Picture,
        const XRenderPictFormat*,int,int,const XTrapezoid*,int))pFunc;

    pFunc = osl_getAsciiFunctionSymbol( mpRenderLib, "XRenderAddTraps" );
#if 0 // not having trapezoid support is supported
    if( !pFunc ) return;
#endif
    mpXRenderAddTraps = (void(*)(Display*,Picture,int,int,const _XTrap*,int))pFunc;

#endif // XRENDER_LINK

    // needed to initialize libXrender internals, we already know its there
#ifdef XRENDER_LINK
    XRenderQueryExtension( mpDisplay, &nDummy, &nDummy );
#else
    (*mpXRenderQueryExtension)( mpDisplay, &nDummy, &nDummy );
#endif

    int nMajor, nMinor;
#ifdef XRENDER_LINK
    XRenderQueryVersion( mpDisplay, &nMajor, &nMinor );
#else
    (*mpXRenderQueryVersion)( mpDisplay, &nMajor, &nMinor );
#endif
    mnRenderVersion = 16*nMajor + nMinor;

    // the 8bit alpha mask format must be there
    XRenderPictFormat aPictFormat={0,0,8,{0,0,0,0,0,0,0,0xFF},0};
    mpStandardFormatA8 = FindPictureFormat( PictFormatAlphaMask|PictFormatDepth, aPictFormat );
}

// ---------------------------------------------------------------------------

// return mask of screens capable of XRENDER text
sal_uInt32 XRenderPeer::InitRenderText()
{
    if( mnRenderVersion < 0x01 )
        return 0;

    // #93033# disable XRENDER for old RENDER versions if XINERAMA is present
    int nDummy;
    if( XQueryExtension( mpDisplay, "XINERAMA", &nDummy, &nDummy, &nDummy ) )
        if( mnRenderVersion < 0x02 )
            return 0;

    if( !mpStandardFormatA8 )
        return 0;

    // and the visual must be supported too on at least one screen
    sal_uInt32 nRetMask = 0;
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    const int nScreenCount = pSalDisp->GetScreenCount();
    XRenderPictFormat* pVisualFormat = NULL;
    int nMaxDepth = 0;
    for( int nScreen = 0; nScreen < nScreenCount; ++nScreen )
    {
        Visual* pXVisual = pSalDisp->GetVisual( nScreen ).GetVisual();
        pVisualFormat = FindVisualFormat( pXVisual );
        if( pVisualFormat != NULL )
        {
            int nVDepth = pSalDisp->GetVisual( nScreen ).GetDepth();
            if( nVDepth > nMaxDepth )
                nMaxDepth = nVDepth;
            nRetMask |= 1U << nScreen;
        }
    }

    // #97763# disable XRENDER on <15bit displays for XFree<=4.2.0
    if( mnRenderVersion <= 0x02 )
        if( nMaxDepth < 15 )
            nRetMask = 0;

    return nRetMask;
}

// ---------------------------------------------------------------------------
