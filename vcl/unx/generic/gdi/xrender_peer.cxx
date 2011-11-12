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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <stdio.h>

#include <rtl/ustring.hxx>
#include <osl/module.h>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>

using ::rtl::OUString;
using ::rtl::OUStringToOString;
#include <xrender_peer.hxx>

using namespace rtl;

// ---------------------------------------------------------------------------

XRenderPeer::XRenderPeer()
    : mpDisplay( GetGenericData()->GetSalDisplay()->GetDisplay() )
    , mpStandardFormatA8( NULL )
    , mnRenderVersion( 0 )
{
    InitRenderLib();
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
    // needed to initialize libXrender internals
    XRenderQueryExtension( mpDisplay, &nDummy, &nDummy );

    int nMajor, nMinor;
    XRenderQueryVersion( mpDisplay, &nMajor, &nMinor );
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
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
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

    return nRetMask;
}

// ---------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
