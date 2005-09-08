/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: twain.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:40:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    NAMESPACE_VOS( OModule )*   pMod;
    ULONG                       nCurState;

    void                        ImplCreate();
    void                        ImplOpenSourceManager();
    void                        ImplOpenSource();
    BOOL                        ImplEnableSource();
    void                        ImplXfer();
    void                        ImplFallback( ULONG nEvent );

                                DECL_LINK( ImplFallbackHdl, void* );
                                DECL_LINK( ImplDestroyHdl, void* );

public:

    BOOL                        ImplHandleMsg( void* pMsg );

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

    BOOL                        SelectSource();
    BOOL                        InitXfer();
    Bitmap                      GetXferBitmap();
};

#endif // _TWAIN_HXX
