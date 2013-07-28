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

#include "VAxisOrGridBase.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "Tickmarks.hxx"

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VAxisOrGridBase::VAxisOrGridBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount )
            : PlotterBase( nDimensionCount )
            , m_nDimensionIndex( nDimensionIndex )
            , m_eLeftWallPos(CuboidPlanePosition_Left)
            , m_eBackWallPos(CuboidPlanePosition_Back)
            , m_eBottomPos(CuboidPlanePosition_Bottom)
{
}

VAxisOrGridBase::~VAxisOrGridBase()
{
}

void VAxisOrGridBase::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
            throw (uno::RuntimeException)
{
    m_aScale = rScale;
    m_aIncrement = rIncrement;
}

void VAxisOrGridBase::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix )
{
    m_aMatrixScreenToScene = HomogenMatrixToB3DHomMatrix(rMatrix);
    PlotterBase::setTransformationSceneToScreen( rMatrix);
}

void VAxisOrGridBase::set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos )
{
    m_eLeftWallPos = eLeftWallPos;
    m_eBackWallPos = eBackWallPos;
    m_eBottomPos = eBottomPos;
}

TickFactory* VAxisOrGridBase::createTickFactory()
{
    return new TickFactory( m_aScale, m_aIncrement );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
