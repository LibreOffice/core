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

#pragma once

#include <sfx2/viewfrm.hxx>

#include <tools/svborder.hxx>
#include <vcl/window.hxx>

struct SfxViewFrame_Impl
{
    SvBorder            aBorder;
    Size                aMargin;
    Size                aSize;
    OUString            aActualURL;
    SfxFrame&           rFrame;
    VclPtr<vcl::Window> pWindow;
    sal_uInt16          nDocViewNo;
    SfxInterfaceId      nCurViewId;
    bool                bResizeInToOut:1;
    bool                bObjLocked:1;
    bool                bReloading:1;
    bool                bIsDowning:1;
    bool                bModal:1;
    bool                bEnabled:1;
    bool                bWindowWasEnabled:1;
    OUString            aFactoryName;

    explicit SfxViewFrame_Impl(SfxFrame& i_rFrame)
        : rFrame(i_rFrame)
        , pWindow(nullptr)
        , nDocViewNo(0)
        , nCurViewId(0)
        , bResizeInToOut(false)
        , bObjLocked(false)
        , bReloading(false)
        , bIsDowning(false)
        , bModal(false)
        , bEnabled(false)
        , bWindowWasEnabled(true)
    {
    }
};

class SfxFrameViewWindow_Impl : public vcl::Window
{
    SfxViewFrame*   pFrame;

public:
                        SfxFrameViewWindow_Impl( SfxViewFrame* p, vcl::Window& rParent ) :
                            Window( &rParent, WB_CLIPCHILDREN ),
                            pFrame( p )
                        {
                            p->GetFrame().GetWindow().SetBorderStyle( WindowBorderStyle::NOBORDER );
                        }

    virtual void        Resize() override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
