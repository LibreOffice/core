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

#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        ChartPrimitive2D::ChartPrimitive2D(
            const uno::Reference< frame::XModel >& rxChartModel,
            const basegfx::B2DHomMatrix& rTransformation,
            const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren),
            mxChartModel(rxChartModel),
            maTransformation(rTransformation)
        {
        }

        bool ChartPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const ChartPrimitive2D& rCompare = static_cast< const ChartPrimitive2D& >(rPrimitive);

                return (getChartModel() == rCompare.getChartModel()
                    && getTransformation() == rCompare.getTransformation());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(ChartPrimitive2D, PRIMITIVE2D_ID_CHARTPRIMITIVE2D)

        basegfx::B2DRange ChartPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransformation());
            return aRetval;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
