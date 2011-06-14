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
#include "PlotterBase.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include <rtl/math.hxx>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlotterBase::PlotterBase( sal_Int32 nDimensionCount )
        : m_xLogicTarget(NULL)
        , m_xFinalTarget(NULL)
        , m_xShapeFactory(NULL)
        , m_pShapeFactory(NULL)
        , m_aCID()
        , m_nDimension(nDimensionCount)
        , m_pPosHelper(NULL)
{
}

void PlotterBase::initPlotter(  const uno::Reference< drawing::XShapes >& xLogicTarget
       , const uno::Reference< drawing::XShapes >& xFinalTarget
       , const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory
       , const rtl::OUString& rCID )
            throw (uno::RuntimeException)
{
    DBG_ASSERT(xLogicTarget.is()&&xFinalTarget.is()&&xShapeFactory.is(),"no proper initialization parameters");
    //is only allowed to be called once
    m_xLogicTarget  = xLogicTarget;
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xShapeFactory;
    m_pShapeFactory = new ShapeFactory(xShapeFactory);
    m_aCID = rCID;
}

PlotterBase::~PlotterBase()
{
    delete m_pShapeFactory;
}

void PlotterBase::setScales( const std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis )
{
    DBG_ASSERT(m_nDimension<=static_cast<sal_Int32>(rScales.size()),"Dimension of Plotter does not fit two dimension of given scale sequence");
    m_pPosHelper->setScales( rScales, bSwapXAndYAxis );
}


void PlotterBase::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    DBG_ASSERT(m_nDimension==2,"Set this transformation only in case of 2D");
    if(m_nDimension!=2)
        return;
    m_pPosHelper->setTransformationSceneToScreen( rMatrix );
}

uno::Reference< drawing::XShapes > PlotterBase::createGroupShape(
            const uno::Reference< drawing::XShapes >& xTarget
            , ::rtl::OUString rName )
{
    if(!m_xShapeFactory.is())
        return NULL;

    if(m_nDimension==2)
    {
        //create and add to target
        return m_pShapeFactory->createGroup2D( xTarget, rName );
    }
    else
    {
        //create and added to target
        return m_pShapeFactory->createGroup3D( xTarget, rName );
    }
}

bool PlotterBase::isValidPosition( const drawing::Position3D& rPos )
{
    if( ::rtl::math::isNan(rPos.PositionX) )
        return false;
    if( ::rtl::math::isNan(rPos.PositionY) )
        return false;
    if( ::rtl::math::isNan(rPos.PositionZ) )
        return false;
    if( ::rtl::math::isInf(rPos.PositionX) )
        return false;
    if( ::rtl::math::isInf(rPos.PositionY) )
        return false;
    if( ::rtl::math::isInf(rPos.PositionZ) )
        return false;
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
