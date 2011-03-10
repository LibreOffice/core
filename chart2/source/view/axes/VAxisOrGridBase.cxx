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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "VAxisOrGridBase.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "Tickmarks.hxx"

// header for define DBG_ASSERT
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
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

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
