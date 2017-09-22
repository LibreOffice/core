/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_BASEGFX_UTILS_ZOOMTOOLS_HXX
#define INCLUDED_BASEGFX_UTILS_ZOOMTOOLS_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** This namespace provides functions for optimized geometric zooming
    */
    namespace zoomtools
    {
        BASEGFX_DLLPUBLIC long zoomOut(long nCurrent);
        BASEGFX_DLLPUBLIC long zoomIn(long nCurrent);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
