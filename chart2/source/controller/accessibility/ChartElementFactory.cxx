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

#include "ChartElementFactory.hxx"
#include "ObjectIdentifier.hxx"
#include "AccessibleChartElement.hxx"

namespace chart
{

AccessibleBase* ChartElementFactory::CreateChartElement( const AccessibleElementInfo& rAccInfo )
{
    ObjectIdentifier aOID( rAccInfo.m_aOID );
    ObjectType eType( aOID.getObjectType() );

    switch( eType )
    {
        case OBJECTTYPE_DATA_POINT:
        case OBJECTTYPE_LEGEND_ENTRY:
            return new AccessibleChartElement( rAccInfo, false, false );
        case OBJECTTYPE_PAGE:
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM_FLOOR:
        case OBJECTTYPE_AXIS:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_CURVE: // e.g. a statistical method printed as line
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_STOCK_RANGE:
        case OBJECTTYPE_DATA_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            return new AccessibleChartElement( rAccInfo, true, false );
        case OBJECTTYPE_UNKNOWN:
            break;
        default:
            break;
    }

    return 0;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
