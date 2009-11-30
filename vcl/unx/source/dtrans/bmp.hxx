/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bmp.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _DTRANS_BMP_HXX_
#define _DTRANS_BMP_HXX_

#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

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
