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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SURFACEGRAPHICS_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SURFACEGRAPHICS_HXX

#include "dx_graphicsprovider.hxx"

namespace dxcanvas
{
    /** Container providing a Gdiplus::Graphics for a Surface

        This wrapper class transparently handles allocation and
        release of surface resources the RAII way (the
        GraphicsSharedPtr returned has a deleter that does all the
        necessary DX cleanup work).
     */
    GraphicsSharedPtr createSurfaceGraphics(const COMReference<surface_type>& rSurface );
}

#endif // INCLUDED_CANVAS_SOURCE_DIRECTX_DX_SURFACEGRAPHICS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
