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

#ifndef INCLUDED_VCL_INC_HEADLESS_SVPVD_HXX
#define INCLUDED_VCL_INC_HEADLESS_SVPVD_HXX

#include <salvd.hxx>

#include <list>

class SvpSalGraphics;
typedef struct _cairo_surface cairo_surface_t;

class VCL_DLLPUBLIC SvpSalVirtualDevice : public SalVirtualDevice
{
    DeviceFormat                        m_eFormat;
    cairo_surface_t*                    m_pSurface;
    basegfx::B2IVector                  m_aFrameSize;
    double                              m_fScale;
    std::list< SvpSalGraphics* >        m_aGraphics;

public:
    SvpSalVirtualDevice(DeviceFormat eFormat, double fScale)
        : m_eFormat(eFormat)
        , m_pSurface(nullptr)
        , m_fScale(fScale)
    {
    }
    virtual ~SvpSalVirtualDevice() override;

    // SalVirtualDevice
    virtual SalGraphics*    AcquireGraphics() override;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) override;

    virtual bool        SetSize( long nNewDX, long nNewDY ) override;
    virtual bool        SetSizeUsingBuffer( long nNewDX, long nNewDY,
                                            sal_uInt8 * pBuffer
                                          ) override;

    // SalGeometryProvider
    virtual long GetWidth() const override;
    virtual long GetHeight() const override;
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPVD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
