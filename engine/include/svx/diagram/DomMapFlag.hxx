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

#ifndef INCLUDED_SVX_DIAGRAM_DOMMAPFLAG_HXX
#define INCLUDED_SVX_DIAGRAM_DOMMAPFLAG_HXX

#include <sal/types.h>
#include <limits.h>

namespace svx::diagram
{
enum class SVXCORE_DLLPUBLIC DomMapFlag : sal_uInt16
{
    OOXData = 0,
    OOXDataImageRels = 1,
    OOXDataHlinkRels = 2,
    OOXLayout = 3,
    OOXStyle = 4,
    OOXColor = 5,
    OOXDrawing = 6,
    OOXDrawingImageRels = 7,
    OOXDrawingHlinkRels = 8
};

typedef std::vector<DomMapFlag> DomMapFlags;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
