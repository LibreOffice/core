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

#ifndef _BORDERHELPER_HXX
#define _BORDERHELPER_HXX

#include "svtools/svtdllapi.h"

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <vcl/outdev.hxx>

namespace svtools {

SVT_DLLPUBLIC std::vector<double> GetLineDashing( sal_uInt16 nDashing, double fScale );

SVT_DLLPUBLIC basegfx::B2DPolyPolygon ApplyLineDashing(
    const basegfx::B2DPolygon& rPolygon, sal_uInt16 nDashing, double fScale );

SVT_DLLPUBLIC void DrawLine( OutputDevice& rDev, const basegfx::B2DPoint& rBeg,
        const basegfx::B2DPoint& rEnd, sal_uInt32 nWidth, sal_uInt16 nDashing );

SVT_DLLPUBLIC void DrawLine( OutputDevice& rDev, const Point& rBeg,
        const Point& rEnd, sal_uInt32 nWidth, sal_uInt16 nDashing );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
