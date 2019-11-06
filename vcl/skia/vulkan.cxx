/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Some of this code is based on Skia source code, covered by the following
 * license notice (see readlicense_oo for the full license):
 *
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <skia/vulkan.hxx>

#include <GrContextFactory.h>

#include <vcl/lazydelete.hxx>

static GrContext* createGrContext()
{
    static vcl::DeleteOnDeinit<sk_gpu_test::GrContextFactory> factory(
        new sk_gpu_test::GrContextFactory);
    // The factory owns the context.
    return factory.get()->get(sk_gpu_test::GrContextFactory::kVulkan_ContextType);
}

GrContext* SkiaVulkanGrContext::getGrContext()
{
    static GrContext* context = createGrContext();
    return context;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
