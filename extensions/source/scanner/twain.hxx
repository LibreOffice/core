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
