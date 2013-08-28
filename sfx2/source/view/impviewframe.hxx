/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SFX2_IMPVIEWFRAME_HXX
#define SFX2_IMPVIEWFRAME_HXX

#include "sfx2/viewfrm.hxx"

#include <svtools/asynclink.hxx>
#include <vcl/window.hxx>

#include <boost/optional.hpp>

struct SfxViewFrame_Impl
{
    SvBorder            aBorder;
    Size                aMargin;
    Size                aSize;
    TypeId              aLastType;
    OUString            aActualURL;
    SfxFrame&           rFrame;
    svtools::AsynchronLink* pReloader;
    Window*             pWindow;
    SfxViewFrame*       pActiveChild;
    Window*             pFocusWin;
    sal_uInt16          nDocViewNo;
    sal_uInt16          nCurViewId;
    sal_Bool            bResizeInToOut:1;
    sal_Bool            bDontOverwriteResizeInToOut:1;
    sal_Bool            bObjLocked:1;
    sal_Bool            bReloading:1;
    sal_Bool            bIsDowning:1;
    sal_Bool            bModal:1;
    sal_Bool            bEnabled:1;
    sal_Bool            bWindowWasEnabled:1;
    sal_Bool            bActive;
    OUString            aFactoryName;
    ::boost::optional< bool >
                        aHasToolPanels;

                        SfxViewFrame_Impl( SfxFrame& i_rFrame )
                        : rFrame( i_rFrame )
                        , pReloader(0 )
                        , pWindow( 0 )
                        , pActiveChild(0)
                        , pFocusWin(0)
                        , bWindowWasEnabled(sal_True)
                        , bActive( sal_False )
                        {
                        }

                        ~SfxViewFrame_Impl()
                        {
                            delete pReloader;
                        }
};

class SfxFrameViewWindow_Impl : public Window
{
    SfxViewFrame*   pFrame;

public:
                        SfxFrameViewWindow_Impl( SfxViewFrame* p, Window& rParent, WinBits nBits=0 ) :
                            Window( &rParent, nBits | WB_CLIPCHILDREN ),
                            pFrame( p )
                        {
                            p->GetFrame().GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
                        }

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
};

#endif // SFX2_IMPVIEWFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
