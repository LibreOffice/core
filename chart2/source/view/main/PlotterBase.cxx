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

#include <PlotterBase.hxx>
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <osl/diagnose.h>

namespace chart
{
using namespace ::com::sun::star;

PlotterBase::PlotterBase( sal_Int32 nDimensionCount )
        : m_nDimension(nDimensionCount)
        , m_pPosHelper(nullptr)
{
}

void PlotterBase::initPlotter(  const rtl::Reference<SvxShapeGroupAnyD>& xLogicTarget
       , const rtl::Reference<SvxShapeGroupAnyD>& xFinalTarget
       , const OUString& rCID )
{
    OSL_PRECOND(xLogicTarget.is()&&xFinalTarget.is(),"no proper initialization parameters");
    //is only allowed to be called once
    m_xLogicTarget  = xLogicTarget;
    m_xFinalTarget  = xFinalTarget;
    m_aCID = rCID;
}

PlotterBase::~PlotterBase()
{
}

void PlotterBase::setScales( std::vector< ExplicitScaleData >&& rScales, bool bSwapXAndYAxis )
{
    if (!m_pPosHelper)
        return;

    OSL_PRECOND(m_nDimension<=static_cast<sal_Int32>(rScales.size()),"Dimension of Plotter does not fit two dimension of given scale sequence");
    m_pPosHelper->setScales( std::move(rScales), bSwapXAndYAxis );
}

void PlotterBase::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    if (!m_pPosHelper)
        return;

    OSL_PRECOND(m_nDimension==2,"Set this transformation only in case of 2D");
    if(m_nDimension!=2)
        return;
    m_pPosHelper->setTransformationSceneToScreen( rMatrix );
}

rtl::Reference<SvxShapeGroupAnyD> PlotterBase::createGroupShape(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
            , const OUString& rName )
{
    if(m_nDimension==2)
    {
        //create and add to target
        return ShapeFactory::createGroup2D( xTarget, rName );
    }
    else
    {
        //create and added to target
        return ShapeFactory::createGroup3D( xTarget, rName );
    }
}

bool PlotterBase::isValidPosition( const drawing::Position3D& rPos )
{
    if( std::isnan(rPos.PositionX) )
        return false;
    if( std::isnan(rPos.PositionY) )
        return false;
    if( std::isnan(rPos.PositionZ) )
        return false;
    if( std::isinf(rPos.PositionX) )
        return false;
    if( std::isinf(rPos.PositionY) )
        return false;
    if( std::isinf(rPos.PositionZ) )
        return false;
    return true;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
