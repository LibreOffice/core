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

#ifndef INCLUDED_VCL_INC_SKIA_UTILS_H
#define INCLUDED_VCL_INC_SKIA_UTILS_H

#include <vcl/skia/SkiaHelper.hxx>

#include <tools/gen.hxx>

#include <tools/sk_app/VulkanWindowContext.h>

namespace SkiaHelper
{
// Get the one shared GrContext instance.
GrContext* getSharedGrContext();

void disableRenderMethod(RenderMethod method);

// Create SkSurface, GPU-backed if possible.
VCL_DLLPUBLIC sk_sp<SkSurface> createSkSurface(int width, int height,
                                               SkColorType type = kN32_SkColorType);

inline sk_sp<SkSurface> createSkSurface(const Size& size, SkColorType type = kN32_SkColorType)
{
    return createSkSurface(size.Width(), size.Height(), type);
}

/// This function is in the X11/Win backend libs, but needs to be used in SkiaHelper in the vcl lib.
VCL_DLLPUBLIC void
    setCreateVulkanWindowContext(std::unique_ptr<sk_app::WindowContext> (*function)());

#ifdef DBG_UTIL
void prefillSurface(sk_sp<SkSurface>& surface);
VCL_DLLPUBLIC void dump(const SkBitmap& bitmap, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkImage>& image, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkSurface>& surface, const char* file);
#endif

} // namespace

#endif // INCLUDED_VCL_INC_SKIA_UTILS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
