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

#include <helperchartrenderer.hxx>
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <svtools/chartprettypainter.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    bool renderChartPrimitive2D(
        const primitive2d::ChartPrimitive2D& rChartCandidate,
        OutputDevice& rOutputDevice,
        const geometry::ViewInformation2D& rViewInformation2D)
    {
        bool bChartRendered(false);

        // code from chart PrettyPrinter
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFact( rChartCandidate.getChartModel(), uno::UNO_QUERY );
            OSL_ENSURE( xFact.is(), "Chart cannot be painted pretty!\n" );

            if( xFact.is() )
            {
                uno::Reference< lang::XUnoTunnel > xChartRenderer( xFact->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ChartRenderer" ) ) ), uno::UNO_QUERY );
                OSL_ENSURE( xChartRenderer.is(), "Chart cannot be painted pretty!\n" );

                if( xChartRenderer.is() )
                {
                    ChartPrettyPainter* pPrettyPainter = reinterpret_cast<ChartPrettyPainter*>(
                        xChartRenderer->getSomething( ChartPrettyPainter::getUnoTunnelId() ));

                    if( pPrettyPainter )
                    {
                        // create logic object range; do NOT use ObjectTransformation for this
                        // (rViewInformation2D.getObjectTransformation()), only the logic object
                        // size is wanted
                        basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);
                        aObjectRange.transform(rChartCandidate.getTransformation());
                        const Rectangle aRectangle(
                                (sal_Int32)aObjectRange.getMinX(), (sal_Int32)aObjectRange.getMinY(),
                                (sal_Int32)aObjectRange.getMaxX(), (sal_Int32)aObjectRange.getMaxY());

                        // #i101811#
                        if(rViewInformation2D.getObjectTransformation().isIdentity())
                        {
                            // no embedding in another transfromation, just paint with existing
                            // MapMode. This is just a shortcut; using the below code will also
                            // work; it has just a neutral ObjectTransformation
                            bChartRendered = pPrettyPainter->DoPaint(&rOutputDevice, aRectangle);
                        }
                        else
                        {
                            // rViewInformation2D.getObjectTransformation() is used and
                            // needs to be expressed in the MapMode for the PrettyPainter;
                            // else it would call ChartModelHelper::setPageSize(...) with the
                            // changed size what really will change the chart model and leads
                            // to re-layouts and re-formatting
                            const MapMode aOldMapMode(rOutputDevice.GetMapMode());
                            basegfx::B2DVector aVTScale, aScale, aTranslate;
                            double fRotate, fShearX;

                            // get basic scaling with current MapMode (aVTScale), containing
                            // mapping for set MapUnit (e.g. for 100th mm, the basic scale is
                            // not 1.0, 1.0). This is needed since this scale is included in
                            // the ObjectToView Transformation and needs to be removed (see
                            // correction below) to re-create a MapMode
                            rOutputDevice.SetMapMode(aOldMapMode.GetMapUnit());
                            rOutputDevice.GetViewTransformation().decompose(aVTScale, aTranslate, fRotate, fShearX);

                            // get complete ObjectToView Transformation scale and translate from current
                            // transformation chain (combined view and object transform)
                            rViewInformation2D.getObjectToViewTransformation().decompose(aScale, aTranslate, fRotate, fShearX);

                            // assert when shear and/or rotation is used
                            OSL_ENSURE(basegfx::fTools::equalZero(fRotate), "Chart PrettyPrinting with unsupportable rotation (!)");
                            OSL_ENSURE(basegfx::fTools::equalZero(fShearX), "Chart PrettyPrinting with unsupportable shear (!)");

                            // clean scale and translate from basic scaling (DPI, etc...)
                            // since this will implicitely be part of the to-be-created MapMode
                            const basegfx::B2DTuple aBasicCleaner(
                                basegfx::fTools::equalZero(aVTScale.getX()) ? 1.0 : 1.0 / aVTScale.getX(),
                                basegfx::fTools::equalZero(aVTScale.getY()) ? 1.0 : 1.0 / aVTScale.getY());
                            aScale *= aBasicCleaner;
                            aTranslate *= aBasicCleaner;

                            // for MapMode, take scale out of translation
                            const basegfx::B2DTuple aScaleRemover(
                                basegfx::fTools::equalZero(aScale.getX()) ? 1.0 : 1.0 / aScale.getX(),
                                basegfx::fTools::equalZero(aScale.getY()) ? 1.0 : 1.0 / aScale.getY());
                            aTranslate *= aScaleRemover;

                            // build new MapMode
                            const MapMode aNewMapMode(aOldMapMode.GetMapUnit(),
                                Point(basegfx::fround(aTranslate.getX()), basegfx::fround(aTranslate.getY())),
                                Fraction(aScale.getX()), Fraction(aScale.getY()));

                            // use, paint, restore
                            rOutputDevice.SetMapMode(aNewMapMode);
                            bChartRendered = pPrettyPainter->DoPaint(&rOutputDevice, aRectangle);
                            rOutputDevice.SetMapMode(aOldMapMode);
                        }
                    }
                }
            }
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Chart cannot be painted pretty!" );
        }

        return bChartRendered;
    }
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
