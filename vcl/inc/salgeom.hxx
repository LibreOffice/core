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

#ifndef INCLUDED_VCL_INC_SALGEOM_HXX
#define INCLUDED_VCL_INC_SALGEOM_HXX

#include <iostream>

#include <vcl/dllapi.h>
#include <tools/long.hxx>

struct SalFrameGeometry {
    // screen position of upper left corner of drawable area in pixel
    tools::Long                nX, nY;
    // dimensions of the drawable area in pixel
    unsigned long       nWidth, nHeight;
    // thickness of the decoration in pixel
    unsigned long       nLeftDecoration,
                        nTopDecoration,
                        nRightDecoration,
                        nBottomDecoration;
    unsigned int        nDisplayScreenNumber;

    SalFrameGeometry() :
        nX( 0 ),
        nY( 0 ),
        nWidth( 1 ),
        nHeight( 1 ),
        nLeftDecoration( 0 ),
        nTopDecoration( 0 ),
        nRightDecoration( 0 ),
        nBottomDecoration( 0 ),
        nDisplayScreenNumber( 0 )
    {}
};

inline std::ostream &operator <<(std::ostream& s, const SalFrameGeometry& rGeom)
{
    s << rGeom.nWidth << "x" << rGeom.nHeight << "@(" << rGeom.nX << "," << rGeom.nY << "):{"
      << rGeom.nLeftDecoration << "," << rGeom.nTopDecoration << "," << rGeom.nRightDecoration << "," << rGeom.nBottomDecoration << "}";

    return s;
}

/// Interface used to share logic on sizing between
/// SalVirtualDevices and SalFrames
class VCL_PLUGIN_PUBLIC SalGeometryProvider {
public:
    virtual ~SalGeometryProvider() {}
    virtual tools::Long GetWidth() const = 0;
    virtual tools::Long GetHeight() const = 0;
    virtual bool IsOffScreen() const = 0;
};

#endif // INCLUDED_VCL_INC_SALGEOM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
