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

#ifndef INCLUDED_VCL_OUTDEVMAP_HXX
#define INCLUDED_VCL_OUTDEVMAP_HXX

struct ImplMapRes
{
    tools::Long                mnMapOfsX;          // Offset in X direction
    tools::Long                mnMapOfsY;          // Offset in Y direction
    tools::Long                mnMapScNumX;        // Scaling factor - numerator in X direction
    tools::Long                mnMapScNumY;        // Scaling factor - numerator in Y direction
    tools::Long                mnMapScDenomX;      // Scaling factor - denominator in X direction
    tools::Long                mnMapScDenomY;      // Scaling factor - denominator in Y direction
};

struct ImplThresholdRes
{
    tools::Long                mnThresLogToPixX;   // Thresholds for calculation
    tools::Long                mnThresLogToPixY;   // with BigInts
    tools::Long                mnThresPixToLogX;   // ""
    tools::Long                mnThresPixToLogY;   // ""
};

#endif // INCLUDED_VCL_OUTDEVMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
