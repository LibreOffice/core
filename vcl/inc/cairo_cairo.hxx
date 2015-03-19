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

#ifndef INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_CAIRO_HXX
#define INCLUDED_CANVAS_SOURCE_CAIRO_CAIRO_CAIRO_HXX

#include <sal/config.h>
#include <boost/shared_ptr.hpp>
#include <vcl/cairo.hxx>
#include "vcl/dllapi.h"
#include <cairo.h>

struct SystemEnvData;
struct BitmapSystemData;
class  OutputDevice;
namespace vcl { class Window; }
class  Size;

namespace cairo {
    typedef cairo_t Cairo;
    typedef cairo_matrix_t Matrix;
    typedef cairo_format_t Format;
    typedef cairo_content_t Content;
    typedef cairo_pattern_t Pattern;

    VCL_PLUGIN_PUBLIC const SystemEnvData* GetSysData(const vcl::Window *pOutputWindow);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
