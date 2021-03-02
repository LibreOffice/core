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

#ifndef INCLUDED_VCL_INC_SCRWND_HXX
#define INCLUDED_VCL_INC_SCRWND_HXX

#include <vcl/toolkit/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>

class Timer;

enum class WheelMode {
    NONE      = 0x0000,
    VH        = 0x0001,
    V         = 0x0002,
    H         = 0x0004,
    ScrollVH  = 0x0008,
    ScrollV   = 0x0010,
    ScrollH   = 0x0020
};
namespace o3tl {
    template<> struct typed_flags<WheelMode> : is_typed_flags<WheelMode, 0x003f> {};
}

class ImplWheelWindow final : public FloatingWindow
{
private:

    std::vector<Image>  maImgList;
    Point               maLastMousePos;
    Point               maCenter;
    std::unique_ptr<Timer> mpTimer;
    sal_uInt64          mnRepaintTime;
    sal_uInt64          mnTimeout;
    WheelMode           mnWheelMode;
    sal_uLong           mnMaxWidth;
    sal_uLong           mnActDist;
    tools::Long                mnActDeltaX;
    tools::Long                mnActDeltaY;
    void                ImplCreateImageList();
    void                ImplSetRegion(const Bitmap& rRegionBmp);
    using Window::ImplGetMousePointer;
    PointerStyle        ImplGetMousePointer( tools::Long nDistX, tools::Long nDistY );
    void                ImplDrawWheel(vcl::RenderContext& rRenderContext);
    void                ImplRecalcScrollValues();

                        DECL_LINK(ImplScrollHdl, Timer *, void);

    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;

public:

    explicit ImplWheelWindow( vcl::Window* pParent );
    virtual             ~ImplWheelWindow() override;
    virtual void        dispose() override;

    void                ImplStop();
    void                ImplSetWheelMode( WheelMode nWheelMode );
};

#endif // INCLUDED_VCL_INC_SCRWND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
