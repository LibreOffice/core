/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <tools/stream.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/dibtools.hxx>

#include "canvasbitmap.hxx"
#include <devicehelper.hxx>
#include <unopolypolygon.hxx>

using namespace ::com::sun::star;

namespace vclcanvas
{
    DeviceHelper::DeviceHelper()
    {}

    void DeviceHelper::init( const OutDevProviderSharedPtr& rOutDev )
    {
        mpOutDev = rOutDev;
    }

    uno::Reference< rendering::XLinePolyPolygon2D > DeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              ,
        const cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        uno::Reference< rendering::XLinePolyPolygon2D > xPoly;
        if( !mpOutDev )
            return xPoly; // we're disposed

        xPoly.set( new ::canvastools::UnoPolyPolygon(
                       ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ) ) );
        // vcl only handles even_odd polygons
        xPoly->setFillRule(rendering::FillRule_EVEN_ODD);

        return xPoly;
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > DeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      ,
        const cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        uno::Reference< rendering::XBezierPolyPolygon2D > xPoly;
        if( !mpOutDev )
            return xPoly; // we're disposed

        xPoly.set( new ::canvastools::UnoPolyPolygon(
                       ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
        // vcl only handles even_odd polygons
        xPoly->setFillRule(rendering::FillRule_EVEN_ODD);

        return xPoly;
    }

    uno::Reference< rendering::XBitmap > DeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >& rDevice,
        const geometry::IntegerSize2D&                     size )
    {
        if( !mpOutDev )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( vcl::unotools::sizeFromIntegerSize2D(size),
                              true,
                              *rDevice,
                              mpOutDev ) );
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpOutDev.reset();
    }

    cpo::uno::Any DeviceHelper::isAccelerated() const
    {
        return cpo::uno::Any(false);
    }

    cpo::uno::Any DeviceHelper::getDeviceHandle() const
    {
        if( !mpOutDev )
            return cpo::uno::Any();

        return cpo::uno::Any(
            reinterpret_cast< sal_Int64 >(&mpOutDev->getOutDev()) );
    }

    cpo::uno::Any DeviceHelper::getSurfaceHandle() const
    {
        return getDeviceHandle();
    }

    void DeviceHelper::dumpScreenContent() const
    {
        static sal_Int32 nFilePostfixCount(0);

        if( !mpOutDev )
            return;

        OUString aFilename = "dbg_frontbuffer" + OUString::number(nFilePostfixCount) + ".bmp";

        SvFileStream aStream( aFilename, StreamMode::STD_READWRITE );

        const ::Point aEmptyPoint;
        OutputDevice& rOutDev = mpOutDev->getOutDev();
        bool bOldMap( rOutDev.IsMapModeEnabled() );
        rOutDev.EnableMapMode( false );
        WriteDIB(rOutDev.GetBitmap(aEmptyPoint, rOutDev.GetOutputSizePixel()), aStream, false);
        rOutDev.EnableMapMode( bOldMap );

        ++nFilePostfixCount;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
