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

#include <vcl/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>

// -----------
// - Defines -
// -----------

#define WHEELMODE_NONE      0x00000000UL
#define WHEELMODE_VH        0x00000001UL
#define WHEELMODE_V         0x00000002UL
#define WHEELMODE_H         0x00000004UL
#define WHEELMODE_SCROLL_VH 0x00000008UL
#define WHEELMODE_SCROLL_V  0x00000010UL
#define WHEELMODE_SCROLL_H  0x00000020UL

// -------------------
// - ImplWheelWindow -
// -------------------

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
    sal_uLong               mnRepaintTime;
    sal_uLong               mnTimeout;
    sal_uLong               mnWheelMode;
    sal_uLong               mnMaxWidth;
    sal_uLong               mnActDist;
    long                mnActDeltaX;
    long                mnActDeltaY;

    void                ImplCreateImageList();
    void                ImplSetRegion( const Bitmap& rRegionBmp );
    using Window::ImplGetMousePointer;
    PointerStyle        ImplGetMousePointer( long nDistX, long nDistY );
    void                ImplDrawWheel();
    void                ImplRecalcScrollValues();

                        DECL_LINK(ImplScrollHdl, void *);

protected:

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );

public:

                        ImplWheelWindow( Window* pParent );
                        ~ImplWheelWindow();

    void                ImplStop();
    void                ImplSetWheelMode( sal_uLong nWheelMode );
    sal_uLong               ImplGetWheelMode() const { return mnWheelMode; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
