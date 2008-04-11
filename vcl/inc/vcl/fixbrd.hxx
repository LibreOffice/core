/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fixbrd.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_FIXBRD_HXX
#define _SV_FIXBRD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/decoview.hxx>
#include <vcl/ctrl.hxx>

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
