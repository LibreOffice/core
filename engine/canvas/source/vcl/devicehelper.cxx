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
#include <canvastools.hxx>
#include <rtl/ref.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/outdev.hxx>

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
        const uno::Reference< vclcanvas::XGraphicDevice >&              ,
        const cpo::uno::Sequence< cpo::uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        if( !mpOutDev )
            return {}; // we're disposed

        // vcl only handles even_odd polygons
        rtl::Reference<canvastools::UnoPolyPolygon> xPoly( new ::canvastools::UnoPolyPolygon(
                       ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points ), rendering::FillRule_EVEN_ODD ) );

        return xPoly;
    }

    void DeviceHelper::disposing()
    {
        // release all references
        mpOutDev.reset();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
