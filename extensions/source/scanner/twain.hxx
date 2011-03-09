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

#ifndef _TWAIN_HXX
#define _TWAIN_HXX

#include <vos/module.hxx>
#include <vcl/bitmap.hxx>
#include "twain/twain.h"


// -----------
// - Defines -
// -----------

#define TWAIN_SELECT            0x00000001UL
#define TWAIN_ACQUIRE           0x00000002UL
#define TWAIN_TERMINATE         0xFFFFFFFFUL

#define TWAIN_EVENT_NONE        0x00000000UL
#define TWAIN_EVENT_QUIT        0x00000001UL
#define TWAIN_EVENT_SCANNING    0x00000002UL
#define TWAIN_EVENT_XFER        0x00000004UL

// ------------
// - ImpTwain -
// ------------

class ImpTwain
{
    TW_IDENTITY                 aAppIdent;
    TW_IDENTITY                 aSrcIdent;
    Link                        aNotifyLink;
    Bitmap                      aBitmap;
    DSMENTRYPROC                pDSM;
    vos:: OModule * pMod;
    sal_uLong                       nCurState;

    void                        ImplCreate();
    void                        ImplOpenSourceManager();
    void                        ImplOpenSource();
    sal_Bool                        ImplEnableSource();
    void                        ImplXfer();
    void                        ImplFallback( sal_uLong nEvent );

                                DECL_LINK( ImplFallbackHdl, void* );
                                DECL_LINK( ImplDestroyHdl, void* );

public:

    sal_Bool                        ImplHandleMsg( void* pMsg );

#ifdef OS2
    HAB                         hAB;
    HWND                        hTwainWnd;
    long                        hTwainHook;
#else
    HWND                        hTwainWnd;
    HHOOK                       hTwainHook;
#endif

public:

                                ImpTwain( const Link& rNotifyLink );
                                ~ImpTwain();

    void                        Destroy();

    sal_Bool                        SelectSource();
    sal_Bool                        InitXfer();
    Bitmap                      GetXferBitmap();
};

#endif // _TWAIN_HXX
