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

#include <tools/gen.hxx>
#include <tools/long.hxx>
#include <vcl/dllapi.h>

class VCL_DLLPUBLIC Scrollable
{
public:
    virtual tools::Long DoScroll(tools::Long nNewPos) = 0;

    virtual void SetRangeMin(tools::Long nNewRange) = 0;
    virtual tools::Long GetRangeMin() const = 0;
    virtual void SetRangeMax(tools::Long nNewRange) = 0;
    virtual tools::Long GetRangeMax() const = 0;
    virtual void SetRange(const Range& rRange) = 0;
    virtual Range GetRange() const = 0;
    virtual void SetThumbPos(tools::Long nThumbPos) = 0;
    virtual tools::Long GetThumbPos() const = 0;
    virtual void SetLineSize(tools::Long nNewSize) = 0;
    virtual tools::Long GetLineSize() const = 0;
    virtual void SetPageSize(tools::Long nNewSize) = 0;
    virtual tools::Long GetPageSize() const = 0;
    virtual void SetVisibleSize(tools::Long nNewSize) = 0;
    virtual tools::Long GetVisibleSize() const = 0;

    virtual bool Inactive() const = 0;

    virtual ~Scrollable() {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
