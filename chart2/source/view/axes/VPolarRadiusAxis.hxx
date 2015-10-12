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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARRADIUSAXIS_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_AXES_VPOLARRADIUSAXIS_HXX

#include "VPolarAxis.hxx"
#include <memory>

namespace chart
{

/**
*/

class VCartesianAxis;

class VPolarRadiusAxis : public VPolarAxis
{
public:
    VPolarRadiusAxis( const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
           , sal_Int32 nDimensionCount );
    virtual ~VPolarRadiusAxis();

    virtual void initPlotter(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xLogicTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xFinalTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& xFactory
        , const OUString& rCID
                ) throw (::com::sun::star::uno::RuntimeException ) override;

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix ) override;

    virtual void setScales( const ::std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis ) override;

    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException) override;

    virtual void initAxisLabelProperties(
                    const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels ) override;

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount() override;

    virtual void createMaximumLabels() override;
    virtual void createLabels() override;
    virtual void updatePositions() override;

    virtual void createShapes() override;

protected: //methods
    virtual bool prepareShapeCreation() override;

private: //member
    std::unique_ptr<VCartesianAxis>  m_apAxisWithLabels;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
