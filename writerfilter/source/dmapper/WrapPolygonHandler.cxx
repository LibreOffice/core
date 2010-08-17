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

#include <com/sun/star/drawing/PointSequence.hpp>

#include <ooxml/resourceids.hxx>
#include <resourcemodel/ResourceModelHelper.hxx>

#include "ConversionHelper.hxx"ConversionHelper::convertTwipToMM100(
#include "WrapPolygonHandler.hxx"
#include "dmapperLoggers.hxx"

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper {

WrapPolygonHandler::WrapPolygonHandler()
: LoggedProperties(dmapper_logger, "WrapPolygonHandler")
{
}

WrapPolygonHandler::~WrapPolygonHandler()
{
}

void WrapPolygonHandler::lcl_attribute(Id Name, Value & val)
{
    sal_Int32 nIntValue = val.getInt();

    switch(Name)
    {
    case NS_ooxml::LN_CT_Point2D_x:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        mnX = nIntValue;
        break;
    case NS_ooxml::LN_CT_Point2D_y:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        mnY = nIntValue;
        break;
    default:
#ifdef DEBUG_WRAP_POLYGON_HANDLER
        dmapper_logger->element("unhandled");
#endif
        break;
    }
}

void WrapPolygonHandler::lcl_sprm(Sprm & sprm)
{
    switch (sprm.getId())
    {
    case NS_ooxml::LN_CT_WrapPath_lineTo:
    case NS_ooxml::LN_CT_WrapPath_start:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            resolveSprmProps(*this, sprm);

            awt::Point aPoint(mnX, mnY);
            mPoints.push_back(aPoint);
        }
        break;
    default:
#ifdef DEBUG_WRAP_POLYGON_HANDLER
        dmapper_logger->element("unhandled");
#endif
        break;
    }
}

drawing::PointSequenceSequence WrapPolygonHandler::getPolygon()
{
    drawing::PointSequenceSequence aPolyPolygon(1L);
    drawing::PointSequence * pPolygon = aPolyPolygon.getArray();
    pPolygon->realloc(mPoints.size());

    sal_uInt32 n = 0;
    for (Points_t::const_iterator aIt = mPoints.begin(); aIt != mPoints.end(); aIt++)
    {
        (*pPolygon)[n] = *aIt;
        ++n;
    }

    return aPolyPolygon;
}

}}
