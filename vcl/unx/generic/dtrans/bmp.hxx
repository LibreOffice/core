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

#ifndef _DTRANS_BMP_HXX_
#define _DTRANS_BMP_HXX_

#include <prex.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <postx.h>

#include <sal/types.h>
#include <com/sun/star/awt/XBitmap.hpp>
#include <cppuhelper/compbase1.hxx>



namespace x11 {

// helper methods
sal_uInt8* X11_getBmpFromPixmap( Display* pDisplay,
                                 Drawable aDrawable,
                                 Colormap aColormap,
                                 sal_Int32& rOutSize );

void X11_freeBmp( sal_uInt8* pBmp );

class PixmapHolder
{
    Display*        m_pDisplay;
    Colormap        m_aColormap;
    Pixmap          m_aPixmap;
    Pixmap          m_aBitmap;
    XVisualInfo     m_aInfo;

    int             m_nRedShift, m_nRedShift2;
    int             m_nGreenShift, m_nGreenShift2;
    int             m_nBlueShift, m_nBlueShift2;
    unsigned long   m_nBlueShift2Mask, m_nRedShift2Mask, m_nGreenShift2Mask;

    // these expect data pointers to bitmapinfo header
    void setBitmapDataTC( const sal_uInt8* pData, XImage* pImage );
    void setBitmapDataTCDither( const sal_uInt8* pData, XImage* pImage );
    void setBitmapDataPalette( const sal_uInt8* pData, XImage* pImage );

    unsigned long getTCPixel( sal_uInt8 r, sal_uInt8 g, sal_uInt8 b ) const;
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
    int getClass() const { return m_aInfo.c_class; }
    int getDepth() const { return m_aInfo.depth; }
};

class BmpTransporter :
        public cppu::WeakImplHelper1< com::sun::star::awt::XBitmap >
{
    com::sun::star::uno::Sequence<sal_Int8>         m_aBM;
    com::sun::star::awt::Size                       m_aSize;
public:
    BmpTransporter( const com::sun::star::uno::Sequence<sal_Int8>& rBmp );
    virtual  ~BmpTransporter();

    virtual com::sun::star::awt::Size SAL_CALL getSize() throw();
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getDIB() throw();
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getMaskDIB() throw();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
