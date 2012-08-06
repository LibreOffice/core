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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CHARTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CHARTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ChartPrimitive2D class

            This is a helper primitive which decomposes to the ChartMetaFile
            visualisation. It is used to allow specific renderers to do something
            direct for visualising the chart.
         */
        class DRAWINGLAYER_DLLPUBLIC ChartPrimitive2D : public GroupPrimitive2D
        {
        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >     mxChartModel;
            basegfx::B2DHomMatrix                   maTransformation;

        public:
            ChartPrimitive2D(
                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxChartModel,
                const basegfx::B2DHomMatrix& rTransformation,
                const Primitive2DSequence& rChildren);

            /// data read access
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& getChartModel() const { return mxChartModel; }
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()

            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_CHARTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
