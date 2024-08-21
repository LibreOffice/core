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

#include <sal/config.h>

#include <cmath>

#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <comphelper/sequence.hxx>
#include <tools/UnitConversion.hxx>

#include <ooxml/resourceids.hxx>

#include "WrapPolygonHandler.hxx"
#include "util.hxx"

#include <sal/log.hxx>

namespace writerfilter {

using namespace com::sun::star;

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

WrapPolygon::Pointer_t WrapPolygon::move(const awt::Point & rPoint) const
{
    WrapPolygon::Pointer_t pResult(new WrapPolygon);

    Points_t::const_iterator aIt = begin();
    Points_t::const_iterator aItEnd = end();

    while (aIt != aItEnd)
    {
        awt::Point aPoint(aIt->X + rPoint.X, aIt->Y + rPoint.Y);
        pResult->addPoint(aPoint);
        ++aIt;
    }

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::scale(double scaleX, double scaleY) const
{
    WrapPolygon::Pointer_t pResult(new WrapPolygon);

    Points_t::const_iterator aIt = begin();
    Points_t::const_iterator aItEnd = end();

    while (aIt != aItEnd)
    {
        awt::Point aPoint(std::round(aIt->X * scaleX), std::round(aIt->Y * scaleY));
        pResult->addPoint(aPoint);
        ++aIt;
    }

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctWordWrapPolygon(const awt::Size & rSrcSize) const
{
    WrapPolygon::Pointer_t pResult;

    const double nWrap100Percent = 21600;

    double nMove(convertTwipToMm100(nWrap100Percent * 15) / rSrcSize.Width);
    awt::Point aMovePoint(std::round(nMove), 0);
    pResult = move(aMovePoint);

    double nScaleX = rSrcSize.Width / (nWrap100Percent + nMove);
    double nScaleY = rSrcSize.Height / (nWrap100Percent - nMove);
    pResult = pResult->scale(nScaleX, nScaleY);

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctWordWrapPolygonPixel(const awt::Size & rSrcSize) const
{
    WrapPolygon::Pointer_t pResult;

    /*
    * https://msdn.microsoft.com/en-us/library/ee342530.aspx
    *
    * Image wrapping polygons in Microsoft Word use a fixed coordinate space
    * that is 21600 units x 21600 units. Coordinate (0,0) is the upper left
    * corner of the image and coordinate (21600,21600) is the lower right
    * corner of the image. Microsoft Word scales the size of the wrapping
    * polygon units to fit the size of the image. The 21600 value is a legacy
    * artifact from the drawing layer of early versions of Microsoft Office.
    */
    const double nWrap100Percent = 21600;

    double aScaleX(rSrcSize.Width / nWrap100Percent);
    double aScaleY(rSrcSize.Height / nWrap100Percent);
    pResult = scale(aScaleX, aScaleY);

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctCrop(const awt::Size& rGraphicSize,
                                                const text::GraphicCrop& rGraphicCrop) const
{
    WrapPolygon::Pointer_t pResult;

    double aScaleX(double(rGraphicSize.Width - rGraphicCrop.Left - rGraphicCrop.Right)
                   / rGraphicSize.Width);
    double aScaleY(double(rGraphicSize.Height - rGraphicCrop.Top - rGraphicCrop.Bottom)
                   / rGraphicSize.Height);
    pResult = scale(aScaleX, aScaleY);

    awt::Point aMove(rGraphicCrop.Left, rGraphicCrop.Top);
    pResult = pResult->move(aMove);

    return pResult;
}

drawing::PointSequenceSequence WrapPolygon::getPointSequenceSequence() const
{
    return { comphelper::containerToSequence(mPoints) };
}

WrapPolygonHandler::WrapPolygonHandler()
    : LoggedProperties("WrapPolygonHandler")
    , mpPolygon(new WrapPolygon)
    , mnX(0)
    , mnY(0)
{
}

WrapPolygonHandler::~WrapPolygonHandler()
{
}

void WrapPolygonHandler::lcl_attribute(Id Name, const Value & val)
{
    sal_Int32 nIntValue = val.getInt();

    switch(Name)
    {
    case NS_ooxml::LN_CT_Point2D_x:
        mnX = nIntValue;
        break;
    case NS_ooxml::LN_CT_Point2D_y:
        mnY = nIntValue;
        break;
    default:
        SAL_WARN("writerfilter", "WrapPolygonHandler::lcl_attribute: unhandled token: " << Name);
        break;
    }
}

void WrapPolygonHandler::lcl_sprm(Sprm & _sprm)
{
    switch (_sprm.getId())
    {
    case NS_ooxml::LN_CT_WrapPath_lineTo:
    case NS_ooxml::LN_CT_WrapPath_start:
        {
            resolveSprmProps(*this, _sprm);

            awt::Point aPoint(mnX, mnY);
            mpPolygon->addPoint(aPoint);
        }
        break;
    default:
        SAL_WARN("writerfilter", "WrapPolygonHandler::lcl_sprm: unhandled token: " << _sprm.getId());
        break;
    }
}


}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
