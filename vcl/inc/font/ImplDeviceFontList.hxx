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

#include <rtl/ref.hxx>

#include <PhysicalFontFace.hxx>

#include <vector>

// an ImplDeviceFontList is created by a PhysicalFontCollection
// it becomes invalid when original PhysicalFontCollection is modified
class ImplDeviceFontList
{
private:
    std::vector<rtl::Reference<PhysicalFontFace>> maDevFontVector;

public:
    ImplDeviceFontList() { maDevFontVector.reserve(1024); }
    void Add(PhysicalFontFace* pFace) { maDevFontVector.push_back(pFace); }
    PhysicalFontFace* Get(int nIndex) const { return maDevFontVector[nIndex].get(); }
    int Count() const { return maDevFontVector.size(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
