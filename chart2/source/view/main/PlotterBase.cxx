/*************************************************************************
 *
 *  $RCSfile: PlotterBase.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: bm $ $Date: 2003-12-15 10:00:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "PlotterBase.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "StatisticsHelper.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATAPOINTLABEL_HPP_
#include <drafts/com/sun/star/chart2/DataPointLabel.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_ERRORBARSTYLE_HPP_
#include <drafts/com/sun/star/chart2/ErrorBarStyle.hpp>
#endif

/*
#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif
*/
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PlotterBase::PlotterBase( sal_Int32 nDimensionCount )
        : m_xLogicTarget(NULL)
        , m_xFinalTarget(NULL)
        , m_xShapeFactory(NULL)
        , m_pShapeFactory(NULL)
        , m_nDimension(nDimensionCount)
        , m_pPosHelper(NULL)
{
}

    void SAL_CALL PlotterBase
::init(  const uno::Reference< drawing::XShapes >& xLogicTarget
       , const uno::Reference< drawing::XShapes >& xFinalTarget
       , const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory )
            throw (uno::RuntimeException)
{
    DBG_ASSERT(xLogicTarget.is()&&xFinalTarget.is()&&xShapeFactory.is(),"no proper initialization parameters");
    //is only allowed to be called once
    m_xLogicTarget  = xLogicTarget;
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xShapeFactory;
    m_pShapeFactory = new ShapeFactory(xShapeFactory);
}

PlotterBase::~PlotterBase()
{
    delete m_pShapeFactory;
}

void SAL_CALL PlotterBase::setScales( const uno::Sequence< ExplicitScaleData >& rScales )
                            throw (uno::RuntimeException)
{
    DBG_ASSERT(m_nDimension<=rScales.getLength(),"Dimension of Plotter does not fit two dimension of given scale sequence");
    m_pPosHelper->setScales( rScales );
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

namespace
{
double lcl_getErrorBarLogicLength(
    const uno::Sequence< double > & rData,
    uno::Reference< beans::XPropertySet > xProp,
    ErrorBarStyle eErrorBarStyle,
    sal_Int32 nIndex,
    bool bPositive )
{
    double fResult;
    ::rtl::math::setNan( & fResult );
    try
    {
        switch( eErrorBarStyle )
        {
            case ErrorBarStyle_VARIANCE:
                fResult = StatisticsHelper::getVariance( rData );
                break;
            case ErrorBarStyle_STANDARD_DEVIATION:
                fResult = StatisticsHelper::getStandardDeviation( rData );
                break;
            case ErrorBarStyle_RELATIVE:
            {
                double fPercent;
                if( xProp->getPropertyValue( bPositive
                                             ? C2U("PositiveError")
                                             : C2U("NegativeError")) >>= fPercent )
                {
                    if( ! ::rtl::math::isNan( rData[nIndex] ) &&
                        ! ::rtl::math::isNan( fPercent ))
                    {
                        fResult = rData[nIndex] * fPercent / 100.0;
                    }
                }
            }
            break;
            case ErrorBarStyle_ABSOLUTE:
                xProp->getPropertyValue( bPositive
                                         ? C2U("PositiveError")
                                         : C2U("NegativeError")) >>= fResult;
                break;
            case ErrorBarStyle_ERROR_MARGIN:
            {
                // todo: check if this is really what's called error-margin
                double fPercent;
                if( xProp->getPropertyValue( bPositive
                                             ? C2U("PositiveError")
                                             : C2U("NegativeError")) >>= fPercent )
                {
                    double fMaxValue = *(::std::max_element(
                                             rData.getConstArray(),
                                             rData.getConstArray() + rData.getLength()));
                    if( ! ::rtl::math::isNan( fMaxValue ) &&
                        ! ::rtl::math::isNan( fPercent ))
                    {
                        fResult = fMaxValue * fPercent / 100.0;
                    }
                }
            }
            break;
            case ErrorBarStyle_STANDARD_ERROR:
                fResult = StatisticsHelper::getStandardError( rData );
                break;
            case ErrorBarStyle_FROM_DATA:
                // todo: implement
                break;

                // to avoid warning
            case ErrorBarStyle_MAKE_FIXED_SIZE:
                break;
        }
    }
    catch( uno::Exception & e )
    {
        ASSERT_EXCEPTION( e );
    }

    return fResult;
}

bool lcl_getErrorBarPosAndSize(
    double fErrorBarLength,
    ShapeFactory::tErrorBarDirection eDirection,
    const uno::Reference< XTransformation > & xTrans,
    const PlottingPositionHelper & rPositionHelper,
    drawing::Position3D & rInOutNewPos,
    drawing::Direction3D & rOutNewSize )
{
    bool bClipped = false;

    if( xTrans.is())
    {
        drawing::Position3D aUpperLeft( rInOutNewPos ), aLowerRight( rInOutNewPos );
        switch( eDirection )
        {
            case ShapeFactory::ERROR_BAR_UP:
                aUpperLeft.PositionY += fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_DOWN:
                aLowerRight.PositionY -= fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_RIGHT:
                aLowerRight.PositionX += fErrorBarLength;
                break;
            case ShapeFactory::ERROR_BAR_LEFT:
                aUpperLeft.PositionX -= fErrorBarLength;
                break;
        }

        rPositionHelper.clipLogicValues(
            & rInOutNewPos.PositionX, & rInOutNewPos.PositionY, & rInOutNewPos.PositionZ );
        rPositionHelper.doLogicScaling(
            & rInOutNewPos.PositionX, & rInOutNewPos.PositionY, & rInOutNewPos.PositionZ );
        rInOutNewPos = drawing::Position3D(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( rInOutNewPos ))));

        double fOldX = aUpperLeft.PositionX;
        double fOldY = aUpperLeft.PositionY;
        rPositionHelper.clipLogicValues(
            & aUpperLeft.PositionX, & aUpperLeft.PositionY, & aUpperLeft.PositionZ );
        bClipped = bClipped ||
            ( ( eDirection == ShapeFactory::ERROR_BAR_UP ||
                eDirection == ShapeFactory::ERROR_BAR_LEFT ) &&
              ( fOldX != aUpperLeft.PositionX ||
                fOldY != aUpperLeft.PositionY ));
        rPositionHelper.doLogicScaling(
            & aUpperLeft.PositionX, & aUpperLeft.PositionY, & aUpperLeft.PositionZ );
        drawing::Position3D aNewUpperLeft(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( aUpperLeft ))));

        fOldX = aLowerRight.PositionX;
        fOldY = aLowerRight.PositionY;
        rPositionHelper.clipLogicValues(
            & aLowerRight.PositionX, & aLowerRight.PositionY, & aLowerRight.PositionZ );
        bClipped = bClipped ||
            ( ( eDirection == ShapeFactory::ERROR_BAR_DOWN ||
                eDirection == ShapeFactory::ERROR_BAR_RIGHT ) &&
              ( fOldX != aLowerRight.PositionX ||
                fOldY != aLowerRight.PositionY ));
        rPositionHelper.doLogicScaling(
            & aLowerRight.PositionX, & aLowerRight.PositionY, & aLowerRight.PositionZ );
        drawing::Position3D aNewLowerRight(
            SequenceToPosition3D( xTrans->transform( Position3DToSequence( aLowerRight ))));

        rOutNewSize = drawing::Direction3D(
            aNewLowerRight.PositionX - aNewUpperLeft.PositionX,
            aNewLowerRight.PositionY - aNewUpperLeft.PositionY,
            rInOutNewPos.PositionZ );

        // in 100th of a mm
        double fFixedWidth = 200.0;
        if( eDirection == ShapeFactory::ERROR_BAR_LEFT ||
            eDirection == ShapeFactory::ERROR_BAR_RIGHT )
        {
            rOutNewSize.DirectionY = fFixedWidth;
        }
        else
        {
            rOutNewSize.DirectionX = fFixedWidth;
        }
    }
    return bClipped;
}

} // anonymous namespace

// virtual
void PlotterBase::createErrorBar(
      const uno::Reference< drawing::XShapes >& xTarget
    , const drawing::Position3D& rPos
    , const uno::Reference< beans::XPropertySet > & xErrorBarProperties
    , const uno::Sequence< double > & rData
    , sal_Int32 nIndex
    , bool bVertical /* = true */
    )
{
    if( ! xErrorBarProperties.is())
        return;

    try
    {
        sal_Bool bShowPos, bShowNeg;
        ErrorBarStyle eErrorBarStyle;

        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ShowPositiveError" )) >>= bShowPos ))
            bShowPos = sal_False;
        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ShowNegativeError" )) >>= bShowNeg ))
            bShowNeg = sal_False;
        if( ! (xErrorBarProperties->getPropertyValue( C2U( "ErrorBarStyle" )) >>= eErrorBarStyle ))
            eErrorBarStyle = ErrorBarStyle_VARIANCE;

        uno::Reference< XTransformation > xTrans( m_pPosHelper->getTransformationLogicToScene() );

        if( bShowPos )
        {
            ShapeFactory::tErrorBarDirection eErrorBarDir =
                bVertical
                ? ShapeFactory::ERROR_BAR_UP
                : ShapeFactory::ERROR_BAR_RIGHT;
            double fErrorBarLength = lcl_getErrorBarLogicLength(
                rData, xErrorBarProperties, eErrorBarStyle, nIndex, true /* positive */ );

            drawing::Position3D  aPos( rPos );
            drawing::Direction3D aSize;
            bool bClipped =
                lcl_getErrorBarPosAndSize( fErrorBarLength, eErrorBarDir, xTrans, *m_pPosHelper, aPos, aSize );

            m_pShapeFactory->createErrorBar2D( xTarget, aPos, aSize, eErrorBarDir, bClipped );
        }

        if( bShowNeg )
        {
            ShapeFactory::tErrorBarDirection eErrorBarDir =
                bVertical
                ? ShapeFactory::ERROR_BAR_DOWN
                : ShapeFactory::ERROR_BAR_LEFT;
            double fErrorBarLength = lcl_getErrorBarLogicLength(
                rData, xErrorBarProperties, eErrorBarStyle, nIndex, false /* negative */ );

            drawing::Position3D  aPos( rPos );
            drawing::Direction3D aSize;
            bool bClipped =
                lcl_getErrorBarPosAndSize( fErrorBarLength, eErrorBarDir, xTrans, *m_pPosHelper, aPos, aSize );

            m_pShapeFactory->createErrorBar2D( xTarget, aPos, aSize, eErrorBarDir, bClipped );
        }
    }
    catch( uno::Exception & e )
    {
        ASSERT_EXCEPTION( e );
    }

}

/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL PlotterBase
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN2D_SERVICE_NAME;
}

    void SAL_CALL PlotterBase
::setScales( const uno::Sequence< ExplicitScaleData >& rScales ) throw (uno::RuntimeException)
{
}
    void SAL_CALL PlotterBase
::setTransformation( const uno::Reference< XTransformation >& xTransformationToLogicTarget, const uno::Reference< XTransformation >& xTransformationToFinalPage ) throw (uno::RuntimeException)
{
}
*/

//e.g. for Rectangle
/*
uno::Reference< drawing::XShape > PlotterBase::createPartialPointShape(
                                    CooPoint + series dependent properties ...(create a special struct for each chart type)
                                    , uno::Reference< XThinCoordinateSystem > xCoo
                                    , sal_Bool bIsInBreak
                                    , PointStyle* pStyle )
{
    //create one here; use scaling and transformation to logic target

    //maybe do not show anything in the break //maybe read the behavior out of the configuration
    //if(bIsInBreak)
    //  return NULL;

    uno::Reference< drawing::XShape > xNewPartialPointShape(
        m_xShapeFactory->createInstance(
        rtl::OUString::createFromAscii( "com.sun.star.drawing.RectangleShape" ) )
        , uno::UNO_QUERY );
    //set size and position
    {
        //
    }
    if(pStyle||bIsInBreak)
    {
        //set style properties if any for a single point
        uno::Reference< beans::XPropertySet > xProp( xNewPartialPointShape, uno::UNO_QUERY );
        xProp->setPropertyValue( ... );

        //set special properties if point in break (e.g. additional transparency ...)
    }
}

//e.g. for PlotterBase in 2 dim cartesian coordinates:
sal_Bool ShapeFactory::isShown( const Sequence< ExplicitScaleData >& rScales, const CooPoint& rP, double dLogicalWidthBeforeScaling )
{
    ASSERT(rScales.getLength()==2)
    double dMin_x = rScales[0].Minimum;
    double dMax_x = rScales[0].Maximum;
    double dMin_y = rScales[1].Minimum;
    double dMax_y = rScales[1].Maximum;

    //we know that we have cartesian geometry
    Rectangle aSysRect( rScales[0].Minimum, rScales[1].Maximum, rScales[0].Maximum, rScales[1].Minimum );
    Rectangle aPointRect( dLogicalWidthBeforeScaling )
    if(rP)
}

//-----------------------------------------------------------------------------

class FatCoordinateSystem
{
public:
    //XCoordinateSystemType getType();
    Sequence<XThinCoordinateSystem> getCoordinateSystems();
}

class ThinCoordinateSystem
{
private:

public:
    sal_Bool        isBreak();
    Sequence< ExplicitScaleData > getScales();//SubScales without beak


}
*/
//.............................................................................
} //namespace chart
//.............................................................................
