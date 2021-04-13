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

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>

namespace x11 {

// helper methods
sal_uInt8* X11_getBmpFromPixmap( Display* pDisplay,
                                 Drawable aDrawable,
                                 Colormap aColormap,
                                 sal_Int32& rOutSize );

class PixmapHolder
{
    Display*        m_pDisplay;
    Colormap        m_aColormap;
    Pixmap          m_aPixmap;
    Pixmap          m_aBitmap;
    XVisualInfo     m_aInfo;

    int             m_nRedShift;
    int             m_nGreenShift;
    int             m_nBlueShift;
    tools::ULong    m_nBlueShift2Mask, m_nRedShift2Mask, m_nGreenShift2Mask;

    // these expect data pointers to bitmapinfo header
    void setBitmapDataTC( const sal_uInt8* pData, XImage* pImage );
    void setBitmapDataTCDither( const sal_uInt8* pData, XImage* pImage );
    void setBitmapDataPalette( const sal_uInt8* pData, XImage* pImage );

    tools::ULong getTCPixel( sal_uInt8 r, sal_uInt8 g, sal_uInt8 b ) const;
public:
    PixmapHolder( Display* pDisplay );
    ~PixmapHolder();

    // accepts bitmap file (including bitmap file header)
    Pixmap setBitmapData( const sal_uInt8* pData );
    bool needsConversion( const sal_uInt8* pData );

    Colormap getColormap() const { return m_aColormap; }
    Pixmap getPixmap() const { return m_aPixmap; }
    Pixmap getBitmap() const { return m_aBitmap; }
    VisualID getVisualID() const { return m_aInfo.visualid; }
    int getDepth() const { return m_aInfo.depth; }
};

css::uno::Sequence<sal_Int8> convertBitmapDepth(
    css::uno::Sequence<sal_Int8> const & data, int depth);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
