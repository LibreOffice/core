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

#ifndef INCLUDED_VCL_INC_UNX_SALVD_H
#define INCLUDED_VCL_INC_UNX_SALVD_H

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
    bool        bGraphics_;         // is Graphics used
    bool        bExternPixmap_;

public:
    X11SalVirtualDevice();
    virtual ~X11SalVirtualDevice();

    bool            Init( SalDisplay *pDisplay,
                              long nDX, long nDY,
                              sal_uInt16 nBitCount,
                              SalX11Screen nXScreen,
                              Pixmap hDrawable = None,
                              XRenderPictFormat* pXRenderFormat = NULL );
    void InitGraphics( X11SalVirtualDevice *pVD )
    {
        pGraphics_->Init( pVD );
    }
    Display *GetXDisplay() const
    {
        return pDisplay_->GetDisplay();
    }
    SalDisplay *GetDisplay() const
    {
        return pDisplay_;
    }
    bool IsDisplay() const
    {
        return pDisplay_->IsDisplay();
    }
    Pixmap          GetDrawable() const { return hDrawable_; }
    sal_uInt16      GetDepth() const { return nDepth_; }
    int                     GetWidth() const { return nDX_; }
    int                     GetHeight() const { return nDY_; }
    SalX11Screen            GetXScreenNumber() const { return m_nXScreen; }

    virtual SalGraphics*    AcquireGraphics() SAL_OVERRIDE;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;

                            // Set new size, without saving the old contents
    virtual bool        SetSize( long nNewDX, long nNewDY ) SAL_OVERRIDE;
    virtual void            GetSize( long& rWidth, long& rHeight ) SAL_OVERRIDE;
};


#endif // INCLUDED_VCL_INC_UNX_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
