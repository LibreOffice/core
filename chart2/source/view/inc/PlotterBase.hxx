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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_PLOTTERBASE_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_PLOTTERBASE_HXX

#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XTransformation.hpp>

#include <vector>

namespace chart
{

class AbstractShapeFactory;
class PlottingPositionHelper;

/** This class provides methods for setting axis scales and for performing
 *  scene to screen transformations. It is used as the base class for all
 *  plotter classes.
 */
class PlotterBase
{
public:
    PlotterBase( sal_Int32 nDimension );
    virtual ~PlotterBase();

    /// @throws css::uno::RuntimeException
    virtual void initPlotter(
          const css::uno::Reference< css::drawing::XShapes >& xLogicTarget
        , const css::uno::Reference< css::drawing::XShapes >& xFinalTarget
        , const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory
        , const OUString& rCID
                );

    virtual void setScales( const std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis );

    virtual void setTransformationSceneToScreen( const css::drawing::HomogenMatrix& rMatrix );

    virtual void createShapes() = 0;

    static bool isValidPosition( const css::drawing::Position3D& rPos );

protected: //methods
    css::uno::Reference< css::drawing::XShapes >
        createGroupShape( const css::uno::Reference<
                css::drawing::XShapes >& xTarget
                , const OUString& rName=OUString() );

protected: //member
    css::uno::Reference< css::drawing::XShapes >                m_xLogicTarget;
    css::uno::Reference< css::drawing::XShapes >                m_xFinalTarget;
    css::uno::Reference< css::lang::XMultiServiceFactory>       m_xShapeFactory;
    AbstractShapeFactory* m_pShapeFactory;
    OUString   m_aCID;

    const sal_Int32 m_nDimension;
    // needs to be created and deleted by the derived class
    PlottingPositionHelper*                             m_pPosHelper;
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
