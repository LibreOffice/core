/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_VULKAN_HXX
#define INCLUDED_VCL_INC_SKIA_VULKAN_HXX

#include <GrContext.h>

#include <vcl/dllapi.h>

// Create and handle GrContext for Vulkan drawing to offscreen surfaces.
// Skia already provides WindowContext class that does this for surfaces
// used for drawing to windows, but it does not seem to provide a simple
// way to get GrContext without a window.
class VCL_PLUGIN_PUBLIC SkiaVulkanGrContext
{
public:
    static GrContext* getGrContext();
};

#endif // INCLUDED_VCL_INC_SKIA_VULKAN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
