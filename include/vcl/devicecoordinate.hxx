/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_vcl.h>
#include <tools/long.hxx>

#include <basegfx/point/b2dpoint.hxx>
typedef basegfx::B2DPoint DevicePoint;

#if VCL_FLOAT_DEVICE_PIXEL

typedef double DeviceCoordinate;

#else /* !VCL_FLOAT_DEVICE_PIXEL */

typedef sal_Int32 DeviceCoordinate;

#endif /* ! Carpet Cushion */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
