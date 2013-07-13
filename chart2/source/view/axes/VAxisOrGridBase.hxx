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
#ifndef _CHART2_VAXISORGRIDBASE_HXX
#define _CHART2_VAXISORGRIDBASE_HXX

#include "PlotterBase.hxx"
#include "ThreeDHelper.hxx"
#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <basegfx/matrix/b3dhommatrix.hxx>

namespace chart
{

class TickFactory;

class VAxisOrGridBase : public PlotterBase
{
public:
    VAxisOrGridBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount );
    virtual ~VAxisOrGridBase();

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );
    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);
    void set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos );

    virtual TickFactory* createTickFactory();

protected: //member
    ExplicitScaleData       m_aScale;
    ExplicitIncrementData   m_aIncrement;
    sal_Int32               m_nDimensionIndex;

    ::basegfx::B3DHomMatrix m_aMatrixScreenToScene;

    CuboidPlanePosition m_eLeftWallPos;
    CuboidPlanePosition m_eBackWallPos;
    CuboidPlanePosition m_eBottomPos;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
