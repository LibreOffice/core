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
#include <vcl/salgtype.hxx>
#include <basegfx/vector/b2ivector.hxx>

#include <vector>

class SvpSalGraphics;
typedef struct _cairo_surface cairo_surface_t;

class VCL_DLLPUBLIC SvpSalVirtualDevice : public SalVirtualDevice
{
    cairo_surface_t*                    m_pRefSurface;
    cairo_surface_t*                    m_pSurface;
    bool                                m_bOwnsSurface; // nearly always true, except for edge case of tdf#127529
    basegfx::B2IVector                  m_aFrameSize;
    std::vector< SvpSalGraphics* >      m_aGraphics;

    void CreateSurface(tools::Long nNewDX, tools::Long nNewDY, sal_uInt8 *const pBuffer);

protected:
    SvpSalGraphics* AddGraphics(SvpSalGraphics* aGraphics);

public:
    SvpSalVirtualDevice(cairo_surface_t* pRefSurface, cairo_surface_t* pPreExistingTarget);
    virtual ~SvpSalVirtualDevice() override;

    // SalVirtualDevice
    virtual SalGraphics*    AcquireGraphics() override;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) override;

    virtual bool        SetSize( tools::Long nNewDX, tools::Long nNewDY ) override;
    virtual bool        SetSizeUsingBuffer( tools::Long nNewDX, tools::Long nNewDY,
                                            sal_uInt8 * pBuffer
                                          ) override;

    cairo_surface_t* GetSurface() const { return m_pSurface; }

    // SalGeometryProvider
    virtual tools::Long GetWidth() const override;
    virtual tools::Long GetHeight() const override;
};

#endif // INCLUDED_VCL_INC_HEADLESS_SVPVD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
