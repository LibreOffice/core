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

#include <sal/log.hxx>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/poly.hxx>

#include <memory>

namespace vcl::Vectorizer
{
class PointArray
{
    std::unique_ptr<Point[]> mpArray;
    sal_uLong mnSize;
    sal_uLong mnRealSize;

public:
    PointArray();

    void SetSize(sal_uLong nSize);
    sal_uLong GetRealSize() const { return mnRealSize; }
    void SetRealSize(sal_uLong nRealSize) { mnRealSize = nRealSize; }
    void CreatePoly(tools::Polygon& rPoly) const;

    inline Point& operator[](sal_uLong nPos);
    inline const Point& operator[](sal_uLong nPos) const;
};

inline Point& PointArray::operator[](sal_uLong nPos)
{
    SAL_WARN_IF(nPos >= mnSize, "vcl", "PointArray::operator[]: nPos out of range!");
    return mpArray[nPos];
}

inline const Point& PointArray::operator[](sal_uLong nPos) const
{
    SAL_WARN_IF(nPos >= mnSize, "vcl", "PointArray::operator[]: nPos out of range!");
    return mpArray[nPos];
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
