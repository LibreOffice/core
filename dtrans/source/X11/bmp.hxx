/*************************************************************************
 *
 *  $RCSfile: bmp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:05:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DTRANS_BMP_HXX_
#define _DTRANS_BMP_HXX_

#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <sal/types.h>

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif



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
