/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART_FASTPROPERTYIDRANGES_HXX
#define CHART_FASTPROPERTYIDRANGES_HXX

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
    FAST_PROPERTY_ID_START_SCALE_TEXT_PROP = FAST_PROPERTY_ID_START + 18000
};

} //  namespace chart

// CHART_FASTPROPERTYIDRANGES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
