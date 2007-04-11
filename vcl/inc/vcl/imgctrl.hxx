/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:57:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_IMGCTRL_HXX
#define _SV_IMGCTRL_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#include <vcl/fixed.hxx>
#include <vcl/bitmapex.hxx>

// ----------------
// - ImageControl -
// ----------------

class VCL_DLLPUBLIC ImageControl : public FixedImage
{
private:
    BitmapEx        maBmp;
    BitmapEx        maBmpHC;
    BOOL            mbScaleImage;

public:
                    ImageControl( Window* pParent, WinBits nStyle = 0 );

    void            SetScaleImage( BOOL bScale );
    BOOL            IsScaleImage() const;

    virtual void    Resize();
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    GetFocus();
    virtual void    LoseFocus();

    void            SetBitmap( const BitmapEx& rBmp );
    using OutputDevice::GetBitmap;
    const BitmapEx& GetBitmap() const { return maBmp; }
    BOOL            SetModeBitmap( const BitmapEx& rBitmap, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const BitmapEx& GetModeBitmap( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;
};

#endif  // _SV_IMGCTRL_HXX
