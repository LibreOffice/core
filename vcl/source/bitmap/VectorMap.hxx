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

#pragma once

#include <tools/link.hxx>
#include <tools/long.hxx>

#include <vcl/Scanline.hxx>

#include "vecthelper.hxx"

#include <memory>

namespace vcl::Vectorizer
{
class VectorMap
{
private:
    Scanline mpBuf;
    Scanline* mpScan;
    tools::Long mnWidth;
    tools::Long mnHeight;

public:
    VectorMap(tools::Long nWidth, tools::Long nHeight);
    ~VectorMap();

    tools::Long Width() const { return mnWidth; }
    tools::Long Height() const { return mnHeight; }

    inline void Set(tools::Long nY, tools::Long nX, sal_uInt8 cVal);
    inline sal_uInt8 Get(tools::Long nY, tools::Long nX) const;

    inline bool IsFree(tools::Long nY, tools::Long nX) const;
    inline bool IsCont(tools::Long nY, tools::Long nX) const;
    inline bool IsDone(tools::Long nY, tools::Long nX) const;
};

inline void VectorMap::Set(tools::Long nY, tools::Long nX, sal_uInt8 cVal)
{
    const sal_uInt8 cShift = sal::static_int_cast<sal_uInt8>(6 - ((nX & 3) << 1));
    auto& rPixel = mpScan[nY][nX >> 2];
    rPixel = (rPixel & ~(3 << cShift)) | (cVal << cShift);
}

inline sal_uInt8 VectorMap::Get(tools::Long nY, tools::Long nX) const
{
    return sal::static_int_cast<sal_uInt8>(((mpScan[nY][nX >> 2]) >> (6 - ((nX & 3) << 1))) & 3);
}

inline bool VectorMap::IsFree(tools::Long nY, tools::Long nX) const
{
    return (VECT_FREE_INDEX == Get(nY, nX));
}

inline bool VectorMap::IsCont(tools::Long nY, tools::Long nX) const
{
    return (VECT_CONT_INDEX == Get(nY, nX));
}

inline bool VectorMap::IsDone(tools::Long nY, tools::Long nX) const
{
    return (VECT_DONE_INDEX == Get(nY, nX));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
