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

#include <com/sun/star/drawing/PointSequence.hpp>
#include <comphelper/sequence.hxx>

#include <ooxml/resourceids.hxx>

#include "ConversionHelper.hxx"
#include "WrapPolygonHandler.hxx"
#include "util.hxx"

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

WrapPolygon::Points_t::iterator WrapPolygon::begin()
{
    return mPoints.begin();
}

WrapPolygon::Points_t::iterator WrapPolygon::end()
{
    return mPoints.end();
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
        ++aIt;
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
        awt::Point aPoint( double(aIt->X) * rFractionX.GetNumerator() / rFractionX.GetDenominator()
                           , double(aIt->Y) * rFractionY.GetNumerator() / rFractionY.GetDenominator() );
        pResult->addPoint(aPoint);
        ++aIt;
    }

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctWordWrapPolygon(const awt::Size & rSrcSize)
{
    WrapPolygon::Pointer_t pResult;

    const sal_Int32 nWrap100Percent = 21600;

    Fraction aMove(nWrap100Percent, rSrcSize.Width);
    aMove *= Fraction(15, 1);
    awt::Point aMovePoint( double(aMove), 0 );
    pResult = move(aMovePoint);

    Fraction aScaleX(nWrap100Percent, Fraction(nWrap100Percent) + aMove);
    Fraction aScaleY(nWrap100Percent, Fraction(nWrap100Percent) - aMove);
    pResult = pResult->scale(aScaleX, aScaleY);

    Fraction aScaleSrcX(rSrcSize.Width, nWrap100Percent);
    Fraction aScaleSrcY(rSrcSize.Height, nWrap100Percent);
    pResult = pResult->scale(aScaleSrcX, aScaleSrcY);

    return pResult;
}

WrapPolygon::Pointer_t WrapPolygon::correctWordWrapPolygonPixel(const awt::Size & rSrcSize)
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
    const sal_Int32 nWrap100Percent = 21600;

    Fraction aScaleX(rSrcSize.Width, nWrap100Percent);
    Fraction aScaleY(rSrcSize.Height, nWrap100Percent);
    pResult = scale(aScaleX, aScaleY);

    return pResult;
}

drawing::PointSequenceSequence WrapPolygon::getPointSequenceSequence() const
{
    drawing::PointSequenceSequence aPolyPolygon(1L);
    drawing::PointSequence aPolygon = comphelper::containerToSequence(mPoints);
    aPolyPolygon[0] = aPolygon;
    return aPolyPolygon;
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

void WrapPolygonHandler::lcl_attribute(Id Name, Value & val)
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
