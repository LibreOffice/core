/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VAxisBase.cxx,v $
 * $Revision: 1.3 $
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

#include "VAxisBase.hxx"
#include "ShapeFactory.hxx"
#include "CommonConverters.hxx"
#include "TickmarkHelper.hxx"
#include "macros.hxx"

// header for define DBG_ASSERT
#include <tools/debug.hxx>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
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

sal_Int32 VAxisBase::getDimensionCount()
{
    return m_nDimension;
}

void SAL_CALL VAxisBase::initAxisLabelProperties( const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels )
{
    m_aAxisLabelProperties.m_aFontReferenceSize = rFontReferenceSize;
    m_aAxisLabelProperties.m_aMaximumSpaceForLabels = rMaximumSpaceForLabels;

    if( !m_aAxisProperties.m_bDisplayLabels )
        return;

    if( AxisType::CATEGORY==m_aAxisProperties.m_nAxisType
        || AxisType::SERIES==m_aAxisProperties.m_nAxisType )
    {
        if( m_aAxisProperties.m_xAxisTextProvider.is() )
            m_aTextLabels = m_aAxisProperties.m_xAxisTextProvider->getTextualData();

        m_bUseTextLabels = true;
        if( m_aTextLabels.getLength() == 1 && AxisType::SERIES==m_aAxisProperties.m_nAxisType )
        {
            //don't show a single series name
            m_aAxisProperties.m_bDisplayLabels = false;
            return;
        }
    }

    m_aAxisLabelProperties.nNumberFormatKey = m_aAxisProperties.m_nNumberFormatKey;
    m_aAxisLabelProperties.init(m_aAxisProperties.m_xAxisModel);
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

void VAxisBase::setExrtaLinePositionAtOtherAxis( const double& fCrossingAt )
{
    if( m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis )
        delete m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis;
    m_aAxisProperties.m_pfExrtaLinePositionAtOtherAxis = new double(fCrossingAt);
}

sal_Bool SAL_CALL VAxisBase::isAnythingToDraw()
{
    if( !m_aAxisProperties.m_xAxisModel.is() )
        return false;

    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return false;

    uno::Reference< beans::XPropertySet > xProps( m_aAxisProperties.m_xAxisModel, uno::UNO_QUERY );
    if( xProps.is() )
    {
        sal_Bool bShow = sal_False;
        xProps->getPropertyValue( C2U( "Show" ) ) >>= bShow;
        if( !bShow )
            return false;
    }
    return true;
}

void SAL_CALL VAxisBase::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
            throw (uno::RuntimeException)
{
    m_bReCreateAllTickInfos = true;
    m_aScale = rScale;
    m_aIncrement = rIncrement;
}

bool VAxisBase::prepareShapeCreation()
{
    //returns true if all is ready for further shape creation and any shapes need to be created
    if( !isAnythingToDraw() )
        return false;

    if( m_bReCreateAllTickInfos )
    {
        //-----------------------------------------
        //create all scaled tickmark values
        if( m_xTextTarget.is() )
        {
            TickIter aRemoveIter( m_aAllTickInfos, m_aIncrement, 0, 0 );
            removeTextShapesFromTicks( aRemoveIter, m_xTextTarget );
        }

        std::auto_ptr< TickmarkHelper > apTickmarkHelper( this->createTickmarkHelper() );
        apTickmarkHelper->getAllTicks( m_aAllTickInfos );
        m_bReCreateAllTickInfos = false;
    }

    if( m_xGroupShape_Shapes.is() )
        return true;

    //-----------------------------------------
    //create named group shape
    m_xGroupShape_Shapes = this->createGroupShape( m_xLogicTarget, m_nDimension==2 ? m_aCID : C2U(""));

    if( m_aAxisProperties.m_bDisplayLabels )
        m_xTextTarget = m_pShapeFactory->createGroup2D( m_xFinalTarget, m_aCID );

    return true;
}

sal_Int32 VAxisBase::getIndexOfLongestLabel( const uno::Sequence< rtl::OUString >& rLabels )
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
    return nRet;
}

void VAxisBase::removeTextShapesFromTicks( TickIter& rIter, const Reference< drawing::XShapes >& xTarget )
{
    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo; pTickInfo = rIter.nextInfo() )
    {
        if(pTickInfo->xTextShape.is())
        {
            xTarget->remove(pTickInfo->xTextShape);
            pTickInfo->xTextShape = NULL;
        }
    }
}

void VAxisBase::updateUnscaledValuesAtTicks( TickIter& rIter )
{
    Reference< XScaling > xInverseScaling( NULL );
    if( m_aScale.Scaling.is() )
        xInverseScaling = m_aScale.Scaling->getInverseScaling();

    for( TickInfo* pTickInfo = rIter.firstInfo()
        ; pTickInfo; pTickInfo = rIter.nextInfo() )
    {
        pTickInfo->updateUnscaledValue( xInverseScaling );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
