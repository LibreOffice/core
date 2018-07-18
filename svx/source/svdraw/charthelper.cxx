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

#include <svx/charthelper.hxx>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XUpdatable2.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/graphic/PrimitiveFactory2D.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

using namespace ::com::sun::star;

void ChartHelper::updateChart( const uno::Reference< ::frame::XModel >& rXModel )
{
    if (!rXModel.is())
        return;

    try
    {
        const uno::Reference< lang::XMultiServiceFactory > xChartFact(rXModel, uno::UNO_QUERY_THROW);
        const uno::Reference< lang::XUnoTunnel > xChartView(xChartFact->createInstance("com.sun.star.chart2.ChartView"), uno::UNO_QUERY_THROW);
        const uno::Reference<util::XUpdatable2> xUpdatable(xChartView, uno::UNO_QUERY_THROW);

        xUpdatable->updateHard();
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(false, "Unexpected exception!");
    }
}

drawinglayer::primitive2d::Primitive2DContainer ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
    const uno::Reference< ::frame::XModel >& rXModel,
    basegfx::B2DRange& rRange)
{
    drawinglayer::primitive2d::Primitive2DContainer aRetval;

    if (!rXModel.is())
        return aRetval;

    updateChart(rXModel);

    try
    {
        const uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier(rXModel, uno::UNO_QUERY_THROW);
        const uno::Reference< container::XIndexAccess > xShapeAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);

        if(xShapeAccess->getCount())
        {
            const sal_Int32 nShapeCount(xShapeAccess->getCount());
            const uno::Reference< uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
            const uno::Reference< graphic::XPrimitiveFactory2D > xPrimitiveFactory =
                graphic::PrimitiveFactory2D::create( xContext );

            const uno::Sequence< beans::PropertyValue > aParams;
            uno::Reference< drawing::XShape > xShape;

            for(sal_Int32 a(0); a < nShapeCount; a++)
            {
                xShapeAccess->getByIndex(a) >>= xShape;

                if(xShape.is())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence aNew(
                            xPrimitiveFactory->createPrimitivesFromXShape(
                                xShape,
                                aParams));

                    aRetval.append(aNew);
                }
            }
        }
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(false, "Unexpected exception!");
    }

    if(!aRetval.empty())
    {
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

        rRange = aRetval.getB2DRange(aViewInformation2D);
    }

    return aRetval;
}

void ChartHelper::AdaptDefaultsForChart(
    const uno::Reference < embed::XEmbeddedObject > & xEmbObj)
{
    if( xEmbObj.is())
    {
        uno::Reference< chart2::XChartDocument > xChartDoc( xEmbObj->getComponent(), uno::UNO_QUERY );
        OSL_ENSURE( xChartDoc.is(), "Trying to set chart property to non-chart OLE" );
        if( !xChartDoc.is())
            return;

        try
        {
            // set background to transparent (none)
            uno::Reference< beans::XPropertySet > xPageProp( xChartDoc->getPageBackground());
            if( xPageProp.is())
                xPageProp->setPropertyValue( "FillStyle",
                                             uno::makeAny( drawing::FillStyle_NONE ));
            // set no border
            if( xPageProp.is())
                xPageProp->setPropertyValue( "LineStyle",
                                             uno::makeAny( drawing::LineStyle_NONE ));
        }
        catch( const uno::Exception & )
        {
            OSL_FAIL( "Exception caught in AdaptDefaultsForChart" );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
