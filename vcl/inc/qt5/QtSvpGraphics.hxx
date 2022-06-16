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

#pragma once

#include <vclpluginapi.h>
#include <headless/svpgdi.hxx>

class QtFrame;

class VCLPLUG_QT_PUBLIC QtSvpGraphics final : public SvpSalGraphics
{
    QtFrame* const m_pFrame;
    sal_Int32 m_nScalePercentage;

    void handleDamage(const tools::Rectangle&) override;

public:
    QtSvpGraphics(QtFrame* rFrame, sal_Int32 nScalePercentage = -1);
    ~QtSvpGraphics() override;

    void updateQWidget() const;
    QtFrame* frame() const { return m_pFrame; }

#if ENABLE_CAIRO_CANVAS
    bool SupportsCairo() const override;
    cairo::SurfaceSharedPtr
    CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width,
                                          int height) const override;
#endif // ENABLE_CAIRO_CANVAS

    virtual sal_Int32 GetSgpMetric(vcl::SGPmetric eMetric) const override;

    virtual OUString getRenderBackendName() const override { return "qt5svp"; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
