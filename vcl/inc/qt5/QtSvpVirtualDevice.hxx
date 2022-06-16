/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <headless/svpvd.hxx>

class QtSvpGraphics;

class VCL_DLLPUBLIC QtSvpVirtualDevice final : public SvpSalVirtualDevice
{
    sal_Int32 m_nScalePercentage;

public:
    QtSvpVirtualDevice(QtSvpGraphics& rGraphics, cairo_surface_t* pPreExistingTarget);
    SalGraphics* AcquireGraphics() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
