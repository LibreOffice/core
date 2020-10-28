/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_DEVICE_COORDINATE_HXX
#define INCLUDED_VCL_DEVICE_COORDINATE_HXX

#include <config_vcl.h>
#include <tools/long.hxx>

#if VCL_FLOAT_DEVICE_PIXEL
#include <basegfx/point/b2dpoint.hxx>
typedef double DeviceCoordinate;

#else /* !VCL_FLOAT_DEVICE_PIXEL */

#include <basegfx/point/b2ipoint.hxx>
typedef tools::Long DeviceCoordinate;

#endif /* ! Carpet Cushion */

#endif /* NDef INCLUDED_VCL_DEVICE_COORDINATE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
