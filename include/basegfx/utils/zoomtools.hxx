/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/basegfxdllapi.h>

namespace basegfx::zoomtools
{
/** This namespace provides functions for optimized geometric zooming
*/
BASEGFX_DLLPUBLIC sal_uInt64 zoomOut(sal_uInt64 nCurrent);
BASEGFX_DLLPUBLIC sal_uInt64 zoomIn(sal_uInt64 nCurrent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
