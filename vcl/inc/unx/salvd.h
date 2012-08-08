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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <unx/salstd.hxx>
#include <unx/saltype.h>
#include <salvd.hxx>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalDisplay;
class X11SalGraphics;

// -=-= SalVirDevData -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class X11SalVirtualDevice : public SalVirtualDevice
{
    SalDisplay      *pDisplay_;
    X11SalGraphics  *pGraphics_;


    Pixmap          hDrawable_;
    SalX11Screen    m_nXScreen;

    int             nDX_;
    int             nDY_;
    sal_uInt16      nDepth_;
    sal_Bool        bGraphics_;         // is Graphics used
    sal_Bool        bExternPixmap_;

public:
    X11SalVirtualDevice();
    virtual ~X11SalVirtualDevice();

    sal_Bool            Init( SalDisplay *pDisplay,
                              long nDX, long nDY,
                              sal_uInt16 nBitCount,
                              SalX11Screen nXScreen,
                              Pixmap hDrawable = None,
                              XRenderPictFormat* pXRenderFormat = NULL );
    inline  void            InitGraphics( X11SalVirtualDevice *pVD );

    inline  Display        *GetXDisplay() const;
    inline  SalDisplay     *GetDisplay() const;
    inline  sal_Bool        IsDisplay() const;
    inline  Pixmap          GetDrawable() const { return hDrawable_; }
    inline  sal_uInt16      GetDepth() const { return nDepth_; }
    int                     GetWidth() const { return nDX_; }
    int                     GetHeight() const { return nDY_; }
    SalX11Screen            GetXScreenNumber() const { return m_nXScreen; }

    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

                            // Set new size, without saving the old contents
    virtual sal_Bool        SetSize( long nNewDX, long nNewDY );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#ifdef _SV_SALDISP_HXX

inline void X11SalVirtualDevice::InitGraphics( X11SalVirtualDevice *pVD )
{ pGraphics_->Init( pVD ); }

inline Display *X11SalVirtualDevice::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline SalDisplay *X11SalVirtualDevice::GetDisplay() const
{ return pDisplay_; }

inline sal_Bool X11SalVirtualDevice::IsDisplay() const
{ return pDisplay_->IsDisplay(); }

#endif

#endif // _SV_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
