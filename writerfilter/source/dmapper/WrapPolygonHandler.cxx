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

#include "ConversionHelper.hxx"
#include "WrapPolygonHandler.hxx"
#include "dmapperLoggers.hxx"

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper {

WrapPolygon::WrapPolygon()
{
}

WrapPolygon::~WrapPolygon()
{
}

void WrapPolygon::addPoint(const awt::Point & rPoint)
{
    mPoints.push_back(rPoint);
}

WrapPolygon::Points_t::const_iterator WrapPolygon::begin() const
{
    return mPoints.begin();
}

WrapPolygon::Points_t::const_iterator WrapPolygon::end() const
{
    return mPoints.end();
}

WrapPolygon::Points_t::iterator WrapPolygon::begin()
{
    return mPoints.begin();
}

WrapPolygon::Points_t::iterator WrapPolygon::end()
{
    return mPoints.end();
}

size_t WrapPolygon::size() const
{
    return mPoints.size();
}

WrapPolygon::Pointer_t WrapPolygon::move(const awt::Point & rPoint)
{
    WrapPolygon::Pointer_t pResult(new WrapPolygon);

    Points_t::iterator aIt = begin();
    Points_t::iterator aItEnd = end();

    while (aIt != aItEnd)
    {
        awt::Point aPoint(aIt->X + rPoint.X, aIt->Y + rPoint.Y);
        pResult->addPoint(aPoint);
        aIt++;
    }

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::scale(const Fraction & rFractionX, const Fraction & rFractionY)
{
    WrapPolygon::Pointer_t pResult(new WrapPolygon);

    Points_t::iterator aIt = begin();
    Points_t::iterator aItEnd = end();

    while (aIt != aItEnd)
    {
        awt::Point aPoint(Fraction(aIt->X) * rFractionX, Fraction(aIt->Y) * rFractionY);
        pResult->addPoint(aPoint);
        aIt++;
    }

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctWordWrapPolygon(const awt::Size & rSrcSize, const awt::Size & rDstSize)
{
    WrapPolygon::Pointer_t pResult;

    const sal_uInt32 nWrap100Percent = 21600;

    Fraction aMove(nWrap100Percent, rSrcSize.Width);
    aMove = aMove * Fraction(15, 1);
    awt::Point aMovePoint(aMove, 0);
    pResult = move(aMovePoint);

    Fraction aScaleX(nWrap100Percent, Fraction(nWrap100Percent) + aMove);
    Fraction aScaleY(nWrap100Percent, Fraction(nWrap100Percent) - aMove);
    pResult = pResult->scale(aScaleX, aScaleY);

    Fraction aScaleDestX(rDstSize.Width, nWrap100Percent);
    Fraction aScaleDestY(rDstSize.Height, nWrap100Percent);
    pResult = pResult->scale(aScaleDestX, aScaleDestY);

    return pResult;
}

drawing::PointSequenceSequence WrapPolygon::getPointSequenceSequence() const
{
    drawing::PointSequenceSequence aPolyPolygon(1L);
    drawing::PointSequence * pPolygon = aPolyPolygon.getArray();
    pPolygon->realloc(size());

    sal_uInt32 n = 0;
    Points_t::const_iterator aIt = begin();
    Points_t::const_iterator aItEnd = end();

    while (aIt != aItEnd)
    {
        (*pPolygon)[n] = *aIt;
        ++n;
        aIt++;
    }

    return aPolyPolygon;
}

WrapPolygonHandler::WrapPolygonHandler()
: LoggedProperties(dmapper_logger, "WrapPolygonHandler")
, mpPolygon(new WrapPolygon)
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

void WrapPolygonHandler::lcl_sprm(Sprm & _sprm)
{
    switch (_sprm.getId())
    {
    case NS_ooxml::LN_CT_WrapPath_lineTo:
    case NS_ooxml::LN_CT_WrapPath_start:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            resolveSprmProps(*this, _sprm);

            awt::Point aPoint(mnX, mnY);
            mpPolygon->addPoint(aPoint);
        }
        break;
    default:
#ifdef DEBUG_WRAP_POLYGON_HANDLER
        dmapper_logger->element("unhandled");
#endif
        break;
    }
}

WrapPolygon::Pointer_t WrapPolygonHandler::getPolygon()
{
    return mpPolygon;
}

}}
