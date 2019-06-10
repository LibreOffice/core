/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <config_vclplug.h>

#if ENABLE_GSTREAMER_1_0
#include <gst/gst.h>
#include <dlfcn.h>

typedef GstElement* (*GstElementFactoryName)(const gchar*, const gchar*);

static GstElementFactoryName gstElementFactoryNameSymbol()
{
    return reinterpret_cast<GstElementFactoryName>(dlsym(nullptr, "gst_element_factory_make"));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
