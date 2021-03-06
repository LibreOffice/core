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

#include <sal/types.h>
#include <tools/gen.hxx>

namespace sd::slidesorter::model
{
/** This class gives access to values related to the visualization of page
    objects.  This includes animation state when blending from one state to
    another.
*/
class VisualState
{
public:
    VisualState(const sal_Int32 nPageId);
    ~VisualState();

    const Point& GetLocationOffset() const { return maLocationOffset; }
    void SetLocationOffset(const Point& rPoint);

    sal_Int32 mnPageId; // For debugging

private:
    Point maLocationOffset;
};

} // end of namespace ::sd::slidesorter::model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
