/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#include <rtl/ustring.hxx>
#include <osl/module.h>
//#include <osl/thread.h>
using namespace rtl;

#include <xrender_peer.hxx>

#include <salunx.h>
#include <saldata.hxx>
#include <saldisp.hxx>

// ---------------------------------------------------------------------------

XRenderPeer::XRenderPeer()
:   mpDisplay( GetX11SalData()->GetDisplay()->GetDisplay() ),
    mpGlyphFormat( NULL ),
    mnRenderVersion( 0 ),
    mpRenderLib( NULL )
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
#ifdef MACOSX
    OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libXrender.dylib" ));
#else
    OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libXrender.so.1" ));
#endif
    mpRenderLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_DEFAULT );
    if( !mpRenderLib ) {
#ifdef DEBUG
        fprintf( stderr, "Display can do XRender, but no %s installed.\n"
            "Please install for improved display performance\n", OUStringToOString( aLibName.getStr(),
                                                                                    osl_getThreadTextEncoding() ).getStr() );
#endif
        return;
    }

    OUString aQueryExtensionFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderQueryExtension"));
    oslGenericFunction pFunc;
    pFunc = osl_getFunctionSymbol( mpRenderLib, aQueryExtensionFuncName.pData);
    if( !pFunc ) return;
    mpXRenderQueryExtension = (Bool(*)(Display*,int*,int*))pFunc;

    OUString aQueryVersionFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderQueryVersion"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aQueryVersionFuncName.pData);
    if( !pFunc ) return;
    mpXRenderQueryVersion = (void(*)(Display*,int*,int*))pFunc;

    OUString aVisFormatFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFindVisualFormat"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aVisFormatFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFindVisualFormat = (XRenderPictFormat*(*)(Display*,Visual*))pFunc;

    OUString aFmtFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFindFormat"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aFmtFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFindFormat = (XRenderPictFormat*(*)(Display*,unsigned long,
        const XRenderPictFormat*,int))pFunc;

    OUString aCreatGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCreateGlyphSet"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aCreatGlyphFuncName.pData);
    if( !pFunc ) return;
    mpXRenderCreateGlyphSet = (GlyphSet(*)(Display*,XRenderPictFormat*))pFunc;

    OUString aFreeGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreeGlyphSet"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aFreeGlyphFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFreeGlyphSet = (void(*)(Display*,GlyphSet))pFunc;

    OUString aAddGlyphFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderAddGlyphs"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aAddGlyphFuncName.pData);
    if( !pFunc ) return;
    mpXRenderAddGlyphs = (void(*)(Display*,GlyphSet,Glyph*,const XGlyphInfo*,
        int,const char*,int))pFunc;

    OUString aFreeGlyphsFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreeGlyphs"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aFreeGlyphsFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFreeGlyphs = (void(*)(Display*,GlyphSet,Glyph*,int))pFunc;

    OUString aCompStringFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCompositeString32"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aCompStringFuncName.pData);
    if( !pFunc ) return;
    mpXRenderCompositeString32 = (void(*)(Display*,int,Picture,Picture,
        XRenderPictFormat*,GlyphSet,int,int,int,int,const unsigned*,int))pFunc;

    OUString aCreatPicFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderCreatePicture"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aCreatPicFuncName.pData);
    if( !pFunc ) return;
    mpXRenderCreatePicture = (Picture(*)(Display*,Drawable,XRenderPictFormat*,
        unsigned long,const XRenderPictureAttributes*))pFunc;

    OUString aSetClipFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderSetPictureClipRegion"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aSetClipFuncName.pData);
    if( !pFunc ) return;
    mpXRenderSetPictureClipRegion = (void(*)(Display*,Picture,XLIB_Region))pFunc;

    OUString aFreePicFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFreePicture"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aFreePicFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFreePicture = (void(*)(Display*,Picture))pFunc;

    OUString aRenderCompositeFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderComposite"));
    pFunc = osl_getFunctionSymbol( mpRenderLib, aRenderCompositeFuncName.pData);
    if( !pFunc ) return;
    mpXRenderComposite = (void(*)(Display*,int,Picture,Picture,Picture,
        int,int,int,int,int,int,unsigned,unsigned))pFunc;

    OUString aFillRectangleFuncName(RTL_CONSTASCII_USTRINGPARAM("XRenderFillRectangle"));
    pFunc=osl_getFunctionSymbol( mpRenderLib, aFillRectangleFuncName.pData);
    if( !pFunc ) return;
    mpXRenderFillRectangle = (void(*)(Display*,int,Picture,const XRenderColor*,
        int,int,unsigned int,unsigned int))pFunc;
#endif

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
}

// ---------------------------------------------------------------------------

// return mask of screens capable of XRENDER text
sal_uInt32 XRenderPeer::InitRenderText( int nMaxDepth )
{
    if( mnRenderVersion < 0x01 )
        return 0;

    // #93033# disable XRENDER for old RENDER versions if XINERAMA is present
    int nDummy;
    if( XQueryExtension( mpDisplay, "XINERAMA", &nDummy, &nDummy, &nDummy ) )
        if( mnRenderVersion < 0x02 )
            return 0;

    // the 8bit alpha mask format must be there
    XRenderPictFormat aPictFormat={0,0,8,{0,0,0,0,0,0,0,0xFF},0};
    mpGlyphFormat = FindPictureFormat( PictFormatAlphaMask|PictFormatDepth, aPictFormat );
    if( !mpGlyphFormat )
        return 0;

    // and the visual must be supported too on at least one screen
    sal_uInt32 nRetMask = 0;
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    const int nScreenCount = pSalDisp->GetScreenCount();
    XRenderPictFormat* pVisualFormat = NULL;
    for( int nScreen = 0; nScreen < nScreenCount; ++nScreen )
    {
        Visual* pXVisual = pSalDisp->GetVisual( nScreen ).GetVisual();
        pVisualFormat = FindVisualFormat( pXVisual );
        if( pVisualFormat != NULL )
            nRetMask |= 1U << nScreen;
    }

    // #97763# disable XRENDER on <15bit displays for XFree<=4.2.0
    if( mnRenderVersion <= 0x02 )
        if( nMaxDepth < 15 )
            return 0;

    return nRetMask;
}

// ---------------------------------------------------------------------------
