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



#ifndef _GOODIES_GRAPHICTRANSFORMER_HXX
#define _GOODIES_GRAPHICTRANSFORMER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

using namespace com::sun::star;

namespace unographic {

// ----------------------
// - GraphicTransformer -
// ----------------------

typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::graphic::XGraphicTransformer
    > GraphicTransformer_UnoImplHelper1;
class GraphicTransformer : public GraphicTransformer_UnoImplHelper1
{
    public:

    GraphicTransformer();
    ~GraphicTransformer();

    // XGraphicTransformer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL colorChange(
        const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rGraphic,
            sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

};

}

#endif
