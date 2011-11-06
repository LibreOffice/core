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


#ifndef _CHART_VPOLARTRANSFORMATION_HXX
#define _CHART_VPOLARTRANSFORMATION_HXX

#include "PlottingPositionHelper.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/chart2/XTransformation.hpp>

namespace chart
{

class VPolarTransformation : public ::cppu::WeakImplHelper1<
    ::com::sun::star::chart2::XTransformation
    >
{
public:
    VPolarTransformation( const PolarPlottingPositionHelper& rPositionHelper );
    virtual ~VPolarTransformation();

    // ____ XTransformation ____
    /// @see ::com::sun::star::chart2::XTransformation
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL transform(
        const ::com::sun::star::uno::Sequence< double >& rSourceValues )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension()
        throw (::com::sun::star::uno::RuntimeException);

private:
    PolarPlottingPositionHelper     m_aPositionHelper;
    ::basegfx::B3DHomMatrix         m_aUnitCartesianToScene;
};

}  // namespace chart

// _CHART_VPOLARTRANSFORMATION_HXX
#endif

