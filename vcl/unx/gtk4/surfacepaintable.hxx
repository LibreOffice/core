/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

/*
   Provide a mechanism to allow continuing to use cairo surface where a GdkPaintable
   is required
*/

G_DECLARE_FINAL_TYPE(SurfacePaintable, surface_paintable, SURFACE, PAINTABLE, GObject)

/*
   Set the surface to paint, takes ownership of pSource
*/
void surface_paintable_set_source(SurfacePaintable* pPaintable, cairo_surface_t* pSource,
                                  int nWidth, int nHeight);

G_END_DECLS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
