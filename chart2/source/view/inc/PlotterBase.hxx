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
#ifndef _CHART2_PLOTTERBASE_HXX
#define _CHART2_PLOTTERBASE_HXX

#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XTransformation.hpp>

#include <cppuhelper/implbase1.hxx>
#include <vector>

namespace chart
{

class ShapeFactory;
class PlottingPositionHelper;

class PlotterBase
{
public:
    PlotterBase( sal_Int32 nDimension );
    virtual ~PlotterBase();

    virtual void initPlotter(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xLogicTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xFinalTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& xFactory
        , const OUString& rCID
                ) throw (::com::sun::star::uno::RuntimeException );

    virtual void setScales( const ::std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );

    virtual void createShapes() = 0;

    static bool isValidPosition( const ::com::sun::star::drawing::Position3D& rPos );

private: //methods
    //no default constructor
    PlotterBase();

protected: //methods
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroupShape( const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , OUString rName=OUString() );

protected: //member
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xLogicTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xFinalTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>       m_xShapeFactory;
    ShapeFactory*                                                       m_pShapeFactory;
    OUString   m_aCID;

    sal_Int32                                           m_nDimension;
    // needs to be created and deleted by the derived class
    PlottingPositionHelper*                             m_pPosHelper;
};
} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
