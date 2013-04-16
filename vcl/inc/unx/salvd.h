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

#ifndef _SV_SALVD_H
#define _SV_SALVD_H

#include <prex.h>
#include <postx.h>

#include <unx/saltype.h>
#include <salvd.hxx>

class SalDisplay;
class X11SalGraphics;

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
