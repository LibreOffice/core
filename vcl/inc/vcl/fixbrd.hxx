/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fixbrd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:53:06 $
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

#ifndef _SV_FIXBRD_HXX
#define _SV_FIXBRD_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

// ---------------------
// - FixedBorder-Types -
// ---------------------

#define FIXEDBORDER_TYPE_IN                     (FRAME_DRAW_IN)
#define FIXEDBORDER_TYPE_OUT                    (FRAME_DRAW_OUT)
#define FIXEDBORDER_TYPE_GROUP                  (FRAME_DRAW_GROUP)
#define FIXEDBORDER_TYPE_DOUBLEIN               (FRAME_DRAW_DOUBLEIN)
#define FIXEDBORDER_TYPE_DOUBLEOUT              (FRAME_DRAW_DOUBLEOUT)

// ---------------
// - FixedBorder -
// ---------------

class VCL_DLLPUBLIC FixedBorder : public Control
{
private:
    USHORT          mnType;
    BOOL            mbTransparent;

private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                              const Point& rPos, const Size& rSize );

public:
                    FixedBorder( Window* pParent, WinBits nStyle = 0 );
                    FixedBorder( Window* pParent, const ResId& rResId );
                    ~FixedBorder();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetTransparent( BOOL bTransparent );
    BOOL            IsTransparent() const { return mbTransparent; }
    void            SetBorderType( USHORT nType );
    USHORT          GetBorderType() const { return mnType; }
};

#endif  // _SV_FIXBRD_HXX
