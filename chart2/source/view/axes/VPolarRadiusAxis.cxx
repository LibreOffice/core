/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarRadiusAxis.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:13:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "VPolarRadiusAxis.hxx"
#include "VCartesianAxis.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;

VPolarRadiusAxis::VPolarRadiusAxis( const AxisProperties& rAxisProperties
            , const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier
            , sal_Int32 nDimensionCount )
            : VPolarAxis( rAxisProperties, xNumberFormatsSupplier, 1/*nDimensionIndex*/, nDimensionCount )
{
    m_aAxisProperties.m_fInnerDirectionSign=0.0;
    m_aAxisProperties.m_bLabelsOutside=true;
    m_aAxisProperties.m_bIsMainAxis=false;
    m_aAxisProperties.m_aLabelAlignment=LABEL_ALIGN_RIGHT;
    m_aAxisProperties.init();

    m_apAxisWithLabels = std::auto_ptr<VCartesianAxis>( new VCartesianAxis(
        m_aAxisProperties,xNumberFormatsSupplier,1/*nDimensionIndex*/,nDimensionCount
        ,new PolarPlottingPositionHelper() ) );
}

VPolarRadiusAxis::~VPolarRadiusAxis()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarRadiusAxis::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    VPolarAxis::setTransformationSceneToScreen( rMatrix );
    m_apAxisWithLabels->setTransformationSceneToScreen( rMatrix );
}

void SAL_CALL VPolarRadiusAxis::setExplicitScaleAndIncrement(
              const ExplicitScaleData& rScale
            , const ExplicitIncrementData& rIncrement )
            throw (uno::RuntimeException)
{
    VPolarAxis::setExplicitScaleAndIncrement( rScale, rIncrement );
    m_apAxisWithLabels->setExplicitScaleAndIncrement( rScale, rIncrement );
}

void SAL_CALL VPolarRadiusAxis::initPlotter(  const uno::Reference< drawing::XShapes >& xLogicTarget
       , const uno::Reference< drawing::XShapes >& xFinalTarget
       , const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory
       , const rtl::OUString& rCID )
            throw (uno::RuntimeException)
{
    VPolarAxis::initPlotter(  xLogicTarget, xFinalTarget, xShapeFactory, rCID );
    m_apAxisWithLabels->initPlotter(  xLogicTarget, xFinalTarget, xShapeFactory, rCID );
}

void SAL_CALL VPolarRadiusAxis::setScales( const uno::Sequence< ExplicitScaleData >& rScales
                                     , sal_Bool bSwapXAndYAxis )
                            throw (uno::RuntimeException)
{
    VPolarAxis::setScales( rScales, bSwapXAndYAxis );
    m_apAxisWithLabels->setScales( rScales, bSwapXAndYAxis );
}

void SAL_CALL VPolarRadiusAxis::initAxisLabelProperties( const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels )
{
    VPolarAxis::initAxisLabelProperties( rFontReferenceSize, rMaximumSpaceForLabels );
    m_apAxisWithLabels->initAxisLabelProperties( rFontReferenceSize, rMaximumSpaceForLabels );
}

sal_Int32 VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount()
{
    return 2;
}

bool VPolarRadiusAxis::prepareShapeCreation()
{
    //returns true if all is ready for further shape creation and any shapes need to be created
    if( !isAnythingToDraw() )
        return false;

    if( m_xGroupShape_Shapes.is() )
        return true;

    return true;
}

void SAL_CALL VPolarRadiusAxis::createMaximumLabels()
{
    m_apAxisWithLabels->createMaximumLabels();
}

void SAL_CALL VPolarRadiusAxis::updatePositions()
{
    m_apAxisWithLabels->updatePositions();
}

void SAL_CALL VPolarRadiusAxis::createLabels()
{
    m_apAxisWithLabels->createLabels();
}

void SAL_CALL VPolarRadiusAxis::createShapes()
{
    if( !prepareShapeCreation() )
        return;

    const ExplicitScaleData& rAngleScale         = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];

    ::std::vector< ::std::vector< TickInfo > > aAngleTickInfos;
    TickmarkHelper aAngleTickmarkHelper( rAngleScale, rAngleIncrement );
    aAngleTickmarkHelper.getAllTicks( aAngleTickInfos );

    uno::Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    AxisProperties aAxisProperties(m_aAxisProperties);

    sal_Int32 nTick = 0;
    TickIter aIter( aAngleTickInfos, rAngleIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if( nTick == 0 )
        {
            m_apAxisWithLabels->createShapes();
            continue;
        }

        pTickInfo->updateUnscaledValue( xInverseScaling );
        aAxisProperties.m_pfMainLinePositionAtOtherAxis = new double( pTickInfo->fUnscaledTickValue );
        aAxisProperties.m_bDisplayLabels=false;

        //-------------------
        VCartesianAxis aAxis(aAxisProperties,m_xNumberFormatsSupplier
            ,1,2,new PolarPlottingPositionHelper());
        aAxis.setExplicitScaleAndIncrement( m_aScale, m_aIncrement );
        aAxis.initPlotter(m_xLogicTarget,m_xFinalTarget,m_xShapeFactory, m_aCID );
        aAxis.setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix( m_aMatrixScreenToScene ) );
        aAxis.setScales( m_pPosHelper->getScales(), false );
        aAxis.initAxisLabelProperties(m_aAxisLabelProperties.m_aFontReferenceSize,m_aAxisLabelProperties.m_aMaximumSpaceForLabels);
        aAxis.createShapes();
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
