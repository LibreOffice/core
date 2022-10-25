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

#include <vcl/dllapi.h>
#include <boost/container/small_vector.hpp>

// used for managing runs e.g. for BiDi, glyph and script fallback
class VCL_DLLPUBLIC ImplLayoutRuns
{
private:
    int mnRunIndex;
    boost::container::small_vector<int, 8> maRuns;

public:
    ImplLayoutRuns() { mnRunIndex = 0; }

    void Clear() { maRuns.clear(); }
    void AddPos(int nCharPos, bool bRTL);
    void AddRun(int nMinRunPos, int nEndRunPos, bool bRTL);

    bool IsEmpty() const { return maRuns.empty(); }
    void ResetPos() { mnRunIndex = 0; }
    void NextRun() { mnRunIndex += 2; }
    bool GetRun(int* nMinRunPos, int* nEndRunPos, bool* bRTL) const;
    bool GetNextPos(int* nCharPos, bool* bRTL);
    bool PosIsInRun(int nCharPos) const;
    bool PosIsInAnyRun(int nCharPos) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
