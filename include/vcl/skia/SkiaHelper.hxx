/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SKIA_SKIAHELPER_HXX
#define INCLUDED_VCL_SKIA_SKIAHELPER_HXX

#include <vcl/dllapi.h>

#include <config_features.h>

#include <rtl/ustring.hxx>

namespace SkiaHelper
{
VCL_DLLPUBLIC bool isVCLSkiaEnabled();

#if HAVE_FEATURE_SKIA

// Which Skia backend to use.
enum RenderMethod
{
    RenderRaster,
    RenderVulkan
};

VCL_DLLPUBLIC RenderMethod renderMethodToUse();

VCL_DLLPUBLIC rtl::OUString driverInfoString();

// Clean up before exit.
VCL_DLLPUBLIC void cleanup();

#endif // HAVE_FEATURE_SKIA

} // namespace

#endif // INCLUDED_VCL_SKIA_SKIAHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
