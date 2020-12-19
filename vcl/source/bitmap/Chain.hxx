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

#include <tools/solar.h>
#include <tools/gen.hxx>
#include <tools/poly.hxx>

namespace vcl::Vectorizer
{
class PointArray;

class Chain
{
private:
    tools::Polygon maPoly;
    Point maStartPt;
    sal_uLong mnArraySize;
    sal_uLong mnCount;
    std::unique_ptr<sal_uInt8[]> mpCodes;

    void GetSpace();

    void PostProcess(const vcl::Vectorizer::PointArray& rArr);

    Chain(const Chain&) = delete;
    Chain& operator=(const Chain&) = delete;

public:
    Chain();

    void BeginAdd(const Point& rStartPt);
    inline void Add(sal_uInt8 nCode);
    void EndAdd(sal_uLong nTypeFlag);

    const tools::Polygon& GetPoly() const { return maPoly; }
};

inline void Chain::Add(sal_uInt8 nCode)
{
    if (mnCount == mnArraySize)
        GetSpace();

    mpCodes[mnCount++] = nCode;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
