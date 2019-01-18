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

#include "VAxisBase.hxx"
#include <ShapeFactory.hxx>
#include <ExplicitCategoriesProvider.hxx>
#include "Tickmarks.hxx"
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <osl/diagnose.h>

#include <memory>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VAxisBase::VAxisBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
                     , const AxisProperties& rAxisProperties
                     , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
            : VAxisOrGridBase( nDimensionIndex, nDimensionCount )
            , m_xNumberFormatsSupplier( xNumberFormatsSupplier )
            , m_aAxisProperties( rAxisProperties )
            , m_bUseTextLabels( false )
            , m_bReCreateAllTickInfos( true )
            , m_bRecordMaximumTextSize(false)
            , m_nMaximumTextWidthSoFar(0)
            , m_nMaximumTextHeightSoFar(0)
{
}

VAxisBase::~VAxisBase()
{
}

void VAxisBase::initAxisLabelProperties( const css::awt::Size& rFontReferenceSize
                  , const css::awt::Rectangle& rMaximumSpaceForLabels )
{
    m_aAxisLabelProperties.m_aFontReferenceSize = rFontReferenceSize;
    m_aAxisLabelProperties.m_aMaximumSpaceForLabels = rMaximumSpaceForLabels;

    if( !m_aAxisProperties.m_bDisplayLabels )
        return;

    if( m_aAxisProperties.m_nAxisType==AxisType::SERIES )
    {
        if( m_aAxisProperties.m_xAxisTextProvider.is() )
            m_aTextLabels = m_aAxisProperties.m_xAxisTextProvider->getTextualData();

        m_bUseTextLabels = true;
        if( m_aTextLabels.getLength() == 1 )
        {
            //don't show a single series name
            m_aAxisProperties.m_bDisplayLabels = false;
            return;
        }
    }
    else if( m_aAxisProperties.m_nAxisType==AxisType::CATEGORY )
    {
        if( m_aAxisProperties.m_pExplicitCategoriesProvider )
            m_aTextLabels = m_aAxisProperties.m_pExplicitCategoriesProvider->getSimpleCategories();

        m_bUseTextLabels = true;
    }

    m_aAxisLabelProperties.nNumberFormatKey = m_aAxisProperties.m_nNumberFormatKey;
    m_aAxisLabelProperties.init(m_aAxisProperties.m_xAxisModel);
    if( m_aAxisProperties.m_bComplexCategories && m_aAxisProperties.m_nAxisType == AxisType::CATEGORY )
        m_aAxisLabelProperties.eStaggering = AxisLabelStaggering::SideBySide;
}

bool VAxisBase::isDateAxis() const
{
    return m_aScale.AxisType == AxisType::DATE;
}
bool VAxisBase::isComplexCategoryAxis() const
{
    return m_aAxisProperties.m_bComplexCategories && m_bUseTextLabels;
}

void VAxisBase::recordMaximumTextSize( const Reference< drawing::XShape >& xShape, double fRotationAngleDegree )
{
    if( m_bRecordMaximumTextSize && xShape.is() )
    {
        awt::Size aSize( ShapeFactory::getSizeAfterRotation(
                            xShape, fRotationAngleDegree ) );

        m_nMaximumTextWidthSoFar = std::max( m_nMaximumTextWidthSoFar, aSize.Width );
        m_nMaximumTextHeightSoFar = std::max( m_nMaximumTextHeightSoFar, aSize.Height );
    }
}

sal_Int32 VAxisBase::estimateMaximumAutoMainIncrementCount()
{
    return 10;
}

void VAxisBase::setExtraLinePositionAtOtherAxis( double fCrossingAt )
{
    m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis.reset(fCrossingAt);
}

sal_Int32 VAxisBase::getDimensionCount() const
{
    return m_nDimension;
}

bool VAxisBase::isAnythingToDraw()
{
    if( !m_aAxisProperties.m_xAxisModel.is() )
        return false;

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return false;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    if( xProps.is() )
    {
        bool bShow = false;
        xProps->getPropertyValue( "Show" ) >>= bShow;
        if( !bShow )
            return false;
    }
    return true;
}

void VAxisBase::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
{
    m_bReCreateAllTickInfos = true;
    m_aScale = rScale;
    m_aIncrement = rIncrement;
}

void VAxisBase::createAllTickInfos( TickInfoArraysType& rAllTickInfos )
{
    std::unique_ptr< TickFactory > apTickFactory( createTickFactory() );
    if( m_aScale.ShiftedCategoryPosition )
        apTickFactory->getAllTicksShifted( rAllTickInfos );
    else
        apTickFactory->getAllTicks( rAllTickInfos );
}

bool VAxisBase::prepareShapeCreation()
{
    //returns true if all is ready for further shape creation and any shapes need to be created
    if( !isAnythingToDraw() )
        return false;

    if( m_bReCreateAllTickInfos )
    {
        //create all scaled tickmark values
        removeTextShapesFromTicks();

        createAllTickInfos(m_aAllTickInfos);
        m_bReCreateAllTickInfos = false;
    }

    if( m_xGroupShape_Shapes.is() )
        return true;

    //create named group shape
    m_xGroupShape_Shapes = createGroupShape( m_xLogicTarget, m_nDimension==2 ? m_aCID : "");

    if( m_aAxisProperties.m_bDisplayLabels )
        m_xTextTarget = m_pShapeFactory->createGroup2D( m_xFinalTarget, m_aCID );

    return true;
}

size_t VAxisBase::getIndexOfLongestLabel( const uno::Sequence<OUString>& rLabels )
{
    sal_Int32 nRet = 0;
    sal_Int32 nLength = 0;
    sal_Int32 nN = 0;
    for( nN=0; nN<rLabels.getLength(); nN++ )
    {
        //todo: get real text width (without creating shape) instead of character count
        if( rLabels[nN].getLength() > nLength )
        {
            nLength = rLabels[nN].getLength();
            nRet = nN;
        }
    }

    assert(nRet >= 0);
    return nRet;
}

void VAxisBase::removeTextShapesFromTicks()
{
    if( m_xTextTarget.is() )
    {
        for (auto & tickInfos : m_aAllTickInfos)
        {
            for (auto & tickInfo : tickInfos)
            {
                if(tickInfo.xTextShape.is())
                {
                    m_xTextTarget->remove(tickInfo.xTextShape);
                    tickInfo.xTextShape = nullptr;
                }
            }
        }
    }
}

void VAxisBase::updateUnscaledValuesAtTicks( TickIter& rIter )
{
    Reference< XScaling > xInverseScaling;
    if( m_aScale.Scaling.is() )
        xInverseScaling = m_aScale.Scaling->getInverseScaling();

    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo; pTickInfo = rIter.nextInfo() )
    {
        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
