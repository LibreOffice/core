/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

//////////////////////////////////////////////////////////////////////////////
// eof
