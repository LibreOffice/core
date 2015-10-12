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

#ifndef INCLUDED_VCL_SOURCE_WINDOW_SCRWND_HXX
#define INCLUDED_VCL_SOURCE_WINDOW_SCRWND_HXX

#include <vcl/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>

#define WHEELMODE_NONE      0x00000000UL
#define WHEELMODE_VH        0x00000001UL
#define WHEELMODE_V         0x00000002UL
#define WHEELMODE_H         0x00000004UL
#define WHEELMODE_SCROLL_VH 0x00000008UL
#define WHEELMODE_SCROLL_V  0x00000010UL
#define WHEELMODE_SCROLL_H  0x00000020UL

class Timer;

class ImplWheelWindow : public FloatingWindow
{
private:

    ImageList           maImgList;
    Bitmap              maWheelBmp;
    CommandScrollData   maCommandScrollData;
    Point               maLastMousePos;
    Point               maCenter;
    Timer*              mpTimer;
    sal_uInt64          mnRepaintTime;
    sal_uInt64          mnTimeout;
    sal_uLong               mnWheelMode;
    sal_uLong               mnMaxWidth;
    sal_uLong               mnActDist;
    long                mnActDeltaX;
    long                mnActDeltaY;

    void                ImplCreateImageList();
    void                ImplSetRegion( const Bitmap& rRegionBmp );
    using Window::ImplGetMousePointer;
    PointerStyle        ImplGetMousePointer( long nDistX, long nDistY );
    void                ImplDrawWheel(vcl::RenderContext& rRenderContext);
    void                ImplRecalcScrollValues();

                        DECL_LINK_TYPED(ImplScrollHdl, Timer *, void);

protected:

    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;

public:

    explicit ImplWheelWindow( vcl::Window* pParent );
    virtual             ~ImplWheelWindow();
    virtual void        dispose() override;

    void                ImplStop();
    void                ImplSetWheelMode( sal_uLong nWheelMode );
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_SCRWND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
