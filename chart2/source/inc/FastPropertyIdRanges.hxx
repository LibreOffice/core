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

namespace chart
{

enum FastPropertyIdRanges
{
    FAST_PROPERTY_ID_START = 10000,
    FAST_PROPERTY_ID_START_DATA_SERIES = FAST_PROPERTY_ID_START + 1000,
    FAST_PROPERTY_ID_START_DATA_POINT  = FAST_PROPERTY_ID_START + 2000,
    FAST_PROPERTY_ID_START_CHAR_PROP   = FAST_PROPERTY_ID_START + 3000,
    FAST_PROPERTY_ID_START_LINE_PROP   = FAST_PROPERTY_ID_START + 4000,
    FAST_PROPERTY_ID_START_FILL_PROP   = FAST_PROPERTY_ID_START + 5000,
    FAST_PROPERTY_ID_START_USERDEF_PROP= FAST_PROPERTY_ID_START + 6000,
    FAST_PROPERTY_ID_START_SCENE_PROP  = FAST_PROPERTY_ID_START + 7000,
//     FAST_PROPERTY_ID_START_NAMED_FILL_PROP = FAST_PROPERTY_ID_START + 8000,
//     FAST_PROPERTY_ID_START_NAMED_LINE_PROP = FAST_PROPERTY_ID_START + 9000,
    FAST_PROPERTY_ID_START_CHART_STATISTIC_PROP = FAST_PROPERTY_ID_START + 12000,
    FAST_PROPERTY_ID_START_CHART_SYMBOL_PROP = FAST_PROPERTY_ID_START + 13000,
    FAST_PROPERTY_ID_START_CHART_DATACAPTION_PROP = FAST_PROPERTY_ID_START + 14000,
    FAST_PROPERTY_ID_START_CHART_SPLINE_PROP = FAST_PROPERTY_ID_START + 15000,
    FAST_PROPERTY_ID_START_CHART_STOCK_PROP = FAST_PROPERTY_ID_START + 16000,
    FAST_PROPERTY_ID_START_CHART_AUTOPOSITION_PROP = FAST_PROPERTY_ID_START + 17000,
    FAST_PROPERTY_ID_START_SCALE_TEXT_PROP = FAST_PROPERTY_ID_START + 18000,
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
