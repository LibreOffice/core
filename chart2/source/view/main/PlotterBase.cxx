/*************************************************************************
 *
 *  $RCSfile: PlotterBase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-17 15:29:44 $
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

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_DATACAPTIONSTYLE_HPP_
#include <drafts/com/sun/star/chart2/DataCaptionStyle.hpp>
#endif

/*
#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif
*/
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
