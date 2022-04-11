/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// make gtk4 plug advertise correctly as gtk4
#define GTK_TOOLKIT_NAME "gtk4"

#include "convert3to4.hxx"
#include "notifyinglayout.hxx"
#include "surfacecellrenderer.hxx"
#include "surfacepaintable.hxx"
#include "transferableprovider.hxx"

#include "../gtk3/gtkinst.cxx"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
