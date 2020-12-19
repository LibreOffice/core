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

#include <rtl/alloc.h>

#include "VectorMap.hxx"

namespace vcl::Vectorizer
{
void VECT_MAP(const std::unique_ptr<tools::Long[]>& pMapIn,
              const std::unique_ptr<tools::Long[]>& pMapOut, tools::Long nVal)
{
    pMapIn[nVal] = (nVal * 4) + 1;
    pMapOut[nVal] = pMapIn[nVal] + 5;
}

void VECT_PROGRESS(const Link<tools::Long, void>* pProgress, tools::Long _def_nVal)
{
    if (pProgress)
        pProgress->Call(_def_nVal);
}

VectorMap::VectorMap(tools::Long nWidth, tools::Long nHeight)
    : mpBuf(static_cast<Scanline>(rtl_allocateZeroMemory(nWidth * nHeight)))
    , mpScan(static_cast<Scanline*>(std::malloc(nHeight * sizeof(Scanline))))
    , mnWidth(nWidth)
    , mnHeight(nHeight)
{
    const tools::Long nWidthAl = (nWidth >> 2) + 1;
    Scanline pTmp = mpBuf;

    for (tools::Long nY = 0; nY < nHeight; pTmp += nWidthAl)
        mpScan[nY++] = pTmp;
}

VectorMap::~VectorMap()
{
    std::free(mpBuf);
    std::free(mpScan);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
