#include "BarChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
//#include "chartview/servicenames_charttypes.hxx"
//#include "chartview/servicenames_coosystems.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "Linear3DTransformation.hxx"
#include "ViewDefines.hxx"
#include "CategoryPositionHelper.hxx"
#include "TransformationHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"

#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

// #define TRANSP_GRADIENT_HACK 1

#ifdef TRANSP_GRADIENT_HACK
# ifndef _DRAFTS_COM_SUN_STAR_CHART2_TRANSPARENCYSTYLE_HPP_
# include <drafts/com/sun/star/chart2/TransparencyStyle.hpp>
# endif
# ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
# include <com/sun/star/container/XNameContainer.hpp>
# endif
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::drafts::com::sun::star::chart2;

class BarPositionHelper : public CategoryPositionHelper, public PlottingPositionHelper
{
public:
    BarPositionHelper( double fSeriesCount=0 ); //@todo
    virtual ~BarPositionHelper();

    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation >
                        getTransformationLogicToScene() const;

    void                updateSeriesCount( double fSeriesCount ); /*only enter the size of x stacked series*/

    double              getStartCategoryIndex() const {
                            sal_Int32 nStart = static_cast<sal_Int32>(getLogicMinX() + 1.49);//@todo +0.5 if clipping of half categories is implemented
                            if( nStart < 0 )
                                nStart = 0;
                            return nStart;
                        }
    double              getEndCategoryIndex() const  {
                            sal_Int32 nEnd = static_cast<sal_Int32>(getLogicMaxX() + 0.5);
                            if( nEnd < 0 )
                                nEnd = 0;
                            return nEnd;
                        }
};

BarPositionHelper::BarPositionHelper( double fSeriesCount )
        : CategoryPositionHelper( fSeriesCount)
{
}

BarPositionHelper::~BarPositionHelper()
{
}

void BarPositionHelper::updateSeriesCount( double fSeriesCount )
{
    m_fSeriesCount = fSeriesCount;
}

uno::Reference< XTransformation > BarPositionHelper::getTransformationLogicToScene() const
{
    //transformation from 2) to 4) //@todo 2) and 4) need a link to a document

    //we need to apply this transformation to each geometric object because of a bug/problem
    //of the old drawing layer (the UNO_NAME_3D_EXTRUDE_DEPTH is an integer value instead of an double )

    if( !m_xTransformationLogicToScene.is() )
    {
        Matrix4D aMatrix;

        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MinZ = getLogicMinZ();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MaxZ = getLogicMaxZ();

        //apply scaling
        //scaling of x axis is refused/ignored
        doLogicScaling( NULL, &MinY, &MinZ );
        doLogicScaling( NULL, &MaxY, &MaxZ);

        if( AxisOrientation_MATHEMATICAL==m_aScales[0].Orientation )
            aMatrix.TranslateX(-MinX);
        else
            aMatrix.TranslateX(-MaxX);
        if( AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation )
            aMatrix.TranslateY(-MinY);
        else
            aMatrix.TranslateY(-MaxY);
        if( AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation )
            aMatrix.TranslateZ(-MaxZ);//z direction in draw is reverse mathematical direction
            //aMatrix.TranslateZ(-MinZ+0.5*getSlotWidth());
        else
            aMatrix.TranslateZ(-MinZ);
            //aMatrix.TranslateZ(-MinZ+0.5*getSlotWidth());

        double fWidthX = MaxX - MinX;
        double fWidthY = MaxY - MinY;
        double fWidthZ = MaxZ - MinZ;

        double fScaleDirectionX = AxisOrientation_MATHEMATICAL==m_aScales[0].Orientation ? 1.0 : -1.0;
        double fScaleDirectionY = AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation ? 1.0 : -1.0;
        double fScaleDirectionZ = AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation ? -1.0 : 1.0;

        aMatrix.ScaleX(fScaleDirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthX);
        aMatrix.ScaleY(fScaleDirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY);
        aMatrix.ScaleZ(fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ);
        //aMatrix.ScaleZ(fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ/getSlotWidth());

        //if(nDim==2)
            aMatrix = aMatrix*m_aMatrixScreenToScene;

        m_xTransformationLogicToScene = new Linear3DTransformation(Matrix4DToHomogenMatrix( aMatrix ));
    }
    return m_xTransformationLogicToScene;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BarChart::BarChart( sal_Int32 nDimension )
        : VSeriesPlotter( nDimension )
        , m_pPosHelper( new BarPositionHelper() )
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

BarChart::~BarChart()
{
    delete m_pPosHelper;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(BarChart,CHART2_VIEW_BARCHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > BarChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_BARCHART_SERVICE_NAME;
    return aSNS;
}
*/
/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL BarChart
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN2D_SERVICE_NAME;
}

    void SAL_CALL BarChart
::setScales( const uno::Sequence< ExplicitScaleData >& rScales ) throw (uno::RuntimeException)
{
}
    void SAL_CALL BarChart
::setTransformation( const uno::Reference< XTransformation >& xTransformationToLogicTarget, const uno::Reference< XTransformation >& xTransformationToFinalPage ) throw (uno::RuntimeException)
{
}
*/

uno::Reference< drawing::XShape > BarChart::createDataPoint2D_Bar(
          const uno::Reference< drawing::XShapes >& xTarget
        , const DataPointGeometry& rGeometry
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        )
{
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.drawing.RectangleShape" ) ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //@todo check wether this needs to be done before transformation to avoid rounding errors
    double fLeftCorner = rGeometry.m_aPosition.PositionX - rGeometry.m_aSize.DirectionX/2.0;
    xShape->setPosition(awt::Point(
                                static_cast<sal_Int32>(fLeftCorner)
                                ,static_cast<sal_Int32>(rGeometry.m_aPosition.PositionY)));
    xShape->setSize(rGeometry.getSize2D());
    this->setMappedProperties( xShape, xObjectProperties, m_aShapePropertyMapForArea );
/*
#ifdef TRANSP_GRADIENT_HACK
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    //HACK: Setting a transparency gradient currently does not work well.  With
    //this hack you can create one gradient that is the only one used.
    // see bug #110940#.

    // if transparency style is not gradient, disable it by setting
    // FillTransparenceGradientName to an empty string
    // Note: TransparencyStyle should become a new property in FillProperties
    //       (FillTransparenceStyle)
    ::drafts::com::sun::star::chart2::TransparencyStyle aTransStyle;
    xObjectProperties->getPropertyValue( C2U( "TransparencyStyle" )) >>= aTransStyle;
    if( aTransStyle != ::drafts::com::sun::star::chart2::TransparencyStyle_GRADIENT )
    {
        xProp->setPropertyValue( C2U( "FillTransparenceGradientName" ),
                                uno::makeAny( ::rtl::OUString()));
    }
    else
    {
        uno::Reference< container::XNameContainer > xGradientTable(
            m_xShapeFactory->createInstance(
                C2U( "com.sun.star.drawing.TransparencyGradientTable" )), uno::UNO_QUERY );
        // note: generate unique name
        if( ! xGradientTable->hasByName( C2U( "Test" ) ) )
        {
            xGradientTable->insertByName( C2U( "Test" ),
                                          xObjectProperties->getPropertyValue( C2U( "TransparencyGradient")));
        }
        xProp->setPropertyValue( C2U( "FillTransparenceGradientName" ),
                                uno::makeAny( C2U( "Test" )));
    }
#endif
*/
    return xShape;
}

uno::Reference< drawing::XShape > BarChart::createDataPoint3D_Bar(
          const uno::Reference< drawing::XShapes >& xTarget
        , const DataPointGeometry& rGeometry
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        , Geometry3D eGeometry )
{
    uno::Reference< drawing::XShape > xShape(NULL);
    //test @todo remove
    static sal_Int32 nTest = 0;
    eGeometry = Geometry3D(nTest%4+1);
    nTest++;

    switch( eGeometry )
    {
        case GEOMETRY_CYLINDER:
            xShape = m_pShapeFactory->createCylinder( xTarget, rGeometry );
            break;
        case GEOMETRY_CONE:
            xShape = m_pShapeFactory->createCone( xTarget, rGeometry );
            break;
        case GEOMETRY_PYRAMID:
            xShape = m_pShapeFactory->createPyramid( xTarget, rGeometry );
            break;
        case GEOMETRY_CUBOID:
        default:
            xShape = m_pShapeFactory->createRoundedCube( xTarget, rGeometry );
    }
    this->setMappedProperties( xShape, xObjectProperties, m_aShapePropertyMapForArea );
    return xShape;
}

void BarChart::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"BarChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));
    uno::Reference< drawing::XShapes > xTextTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    m_pPosHelper->updateSeriesCount( m_aXSlots.size() );
    double fLogicZ        = 0.0;//as defined
    double fLogicBaseWidth = m_pPosHelper->getSlotWidth();

    //(@todo maybe different iteration for breaks in axis ?)
    sal_Int32 nStartCategoryIndex = static_cast<sal_Int32>(m_pPosHelper->getStartCategoryIndex()); // inclusive
    sal_Int32 nEndCategoryIndex   = static_cast<sal_Int32>(m_pPosHelper->getEndCategoryIndex()); //inclusive
//=============================================================================
    //iterate through all shown categories
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex < nEndCategoryIndex; nCatIndex++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
        //iterate through all x slots in this category
        for( double fSlotX=0; aXSlotIter != aXSlotEnd; aXSlotIter++, fSlotX+=1.0 )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            double fLogicPositiveYSum = 0.0, fLogicNegativeYSum = 0.0;
            aXSlotIter->calculateYSumsForCategory( nCatIndex, fLogicPositiveYSum, fLogicNegativeYSum );
            if(fLogicPositiveYSum==0.0 && fLogicNegativeYSum==0.0)
                continue;

            double fScaledLogicPositiveYSum = fLogicPositiveYSum, fScaledLogicNegativeYSum = fLogicNegativeYSum;
            m_pPosHelper->doLogicScaling(NULL,&fScaledLogicPositiveYSum,NULL);
            m_pPosHelper->doLogicScaling(NULL,&fScaledLogicNegativeYSum,NULL);

            double fPositiveLogicYForNextSeries = 0.0;
            double fNegativeLogicYForNextSeries = 0.0;

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
            aSeriesIter = pSeriesList->begin();
//=============================================================================
            //iterate through all series in this x slot
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes(
                    getSeriesGroupShape(*aSeriesIter, xSeriesTarget) );
                uno::Reference< drawing::XShapes > xLabelGroupShape_Shapes(
                    getLabelsGroupShape(*aSeriesIter, xTextTarget) );

                //collect data point information (logic coordinates, style ):
                double fLogicX = m_pPosHelper->getSlotPos( (double)nCatIndex, fSlotX );
                double fLogicBarHeight = (*aSeriesIter)->getY( nCatIndex );
                if( ::rtl::math::isNan( fLogicBarHeight )) //no value at this category
                    continue;
                if(fLogicBarHeight==0.0)//@todo: continue also if the resolution to small
                    continue;

                //todo sort negative and positive values, to display them on different sides of the Y axis
                bool bPositive = fLogicBarHeight > 0.0;
                double fLowerYValue = bPositive ? fPositiveLogicYForNextSeries : fNegativeLogicYForNextSeries;
                double fUpperYValue = fLowerYValue+fLogicBarHeight;
                if( bPositive )
                    fPositiveLogicYForNextSeries += fLogicBarHeight;
                else
                    fNegativeLogicYForNextSeries += fLogicBarHeight;
                /*
                if( !bPositive )
                {
                    double fHelp = fLowerYValue;
                    fLowerYValue = fUpperYValue;
                    fUpperYValue = fHelp;
                }
                */

                //@todo ... start an iteration over the different breaks of the axis
                //each subsystem may add an additional shape to form the whole point
                //create a group shape for this point and add to the series shape:
//              uno::Reference< drawing::XShapes > xPointGroupShape_Shapes( createGroupShape(xSeriesGroupShape_Shapes) );
//              uno::Reference<drawing::XShape> xPointGroupShape_Shape =
//                      uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );
                //as long as we do not iterate we do not need to create an additional group for each point
                uno::Reference< drawing::XShapes > xPointGroupShape_Shapes = xSeriesGroupShape_Shapes;

                //@todo iterate through all subsystems to create partial points
                {
                    //@todo select a suiteable PositionHelper for this subsystem
                    BarPositionHelper* pPosHelper = m_pPosHelper;

                    //apply clipping to Y
                    //@todo clipping of X and Z is not fully integrated so far, as there is a need to create different objects
                    {
                        double fLower = bPositive ? fLowerYValue : fUpperYValue;
                        double fUpper = bPositive ? fUpperYValue : fLowerYValue;
                        if( fLower > pPosHelper->getLogicMaxY() )
                            continue;
                        if( fUpper < pPosHelper->getLogicMinY() )
                            continue;
                        if( fLower < pPosHelper->getLogicMinY() )
                            fLower = pPosHelper->getLogicMinY();
                        if( fUpper > pPosHelper->getLogicMaxY() )
                            fUpper = pPosHelper->getLogicMaxY();
                        if( approxEqual(fLower, fUpper) )
                            continue;
                        fLowerYValue = bPositive ? fLower : fUpper;
                        fUpperYValue = bPositive ? fUpper : fLower;
                    }

                    //apply scaling to Y before calculating width (necessary to maintain gradient in clipped objects)
                    pPosHelper->doLogicScaling(NULL,&fLowerYValue,NULL);
                    pPosHelper->doLogicScaling(NULL,&fUpperYValue,NULL);
                    //scaling of X and Z is not provided as the created objects should be symmetric in that dimensions

                    //calculate resulting width
//                    double fCompleteHeight = bPositive ? fScaledLogicPositiveYSum : fScaledLogicNegativeYSum;
                    double fCompleteHeight = bPositive ? fLogicPositiveYSum : fLogicNegativeYSum;
                    double fLogicBarWidth = fLogicBaseWidth;
                    if(m_nDimension==3)
                    {
                        //@todo? modify depth and width only dependend on the geomtric form ... :
                        if(fCompleteHeight!=0.0)
                            fLogicBarWidth = fLogicBaseWidth*(fCompleteHeight-fLowerYValue)/(fCompleteHeight);
                    }
                    //double fLogicBarDepth = fLogicBarWidth; //Logic Depth and Width are identical by define ... (symmetry is not necessary anymore)
                    double fLogicBarDepth = 1.0;

                    DataPointGeometry aLogicGeom( drawing::Position3D(fLogicX,fLowerYValue,fLogicZ)
                                , drawing::Direction3D(fLogicBarWidth,fUpperYValue-fLowerYValue,fLogicBarDepth)
                                , drawing::Direction3D(0,approxSub(fCompleteHeight,fUpperYValue),0) );
                    //transformation 3) -> 4)
                    DataPointGeometry aTransformedGeom(
                        TransformationHelper::transformLogicGeom(
                            aLogicGeom,pPosHelper->getTransformationLogicToScene()) );

                    //create partial point
                    uno::Reference< drawing::XShape >  xShape;
                    if(m_nDimension==3)
                    {
                        xShape = createDataPoint3D_Bar(
                            xPointGroupShape_Shapes
                            , aTransformedGeom
                            ,(*aSeriesIter)->getPropertiesOfPoint( nCatIndex )
                            ,GEOMETRY_CUBOID );
                    }
                    else //m_nDimension!=3
                    {
                        xShape = createDataPoint2D_Bar(
                            xPointGroupShape_Shapes
                            , aTransformedGeom
                            , (*aSeriesIter)->getPropertiesOfPoint( nCatIndex ));
                    }
                    //set name/classified ObjectID (CID)
                    ShapeFactory::setShapeName(xShape
                        , ObjectIdentifier::createPointCID(
                            (*aSeriesIter)->getPointCID_Stub(),nCatIndex) );

                    //------------
                    //create data point label

                    bool bMiddlePosition = false;
                    if( pSeriesList->begin() != pSeriesList->end() )
                        bMiddlePosition = true;

                    awt::Point aScreenPosition2D = awt::Point(
                            static_cast<sal_Int32>(aTransformedGeom.m_aPosition.PositionX)
                            ,static_cast<sal_Int32>(aTransformedGeom.m_aPosition.PositionY
                                                    +aTransformedGeom.m_aSize.DirectionY )
                            );
                    if(bMiddlePosition)
                        aScreenPosition2D.Y -= static_cast<sal_Int32>(aTransformedGeom.m_aSize.DirectionY/2.0);

                    double fLogicSum = bPositive ? fLogicPositiveYSum : fLogicNegativeYSum;
                    this->createDataLabel( xLabelGroupShape_Shapes, *(*aSeriesIter), nCatIndex
                                    , fLogicBarHeight, fLogicSum, aScreenPosition2D );
                }//end iteration through partial points

                //remove PointGroupShape if empty
//                if(!xPointGroupShape_Shapes->getCount())
//                    xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

            }//next series in x slot (next y slot)
        }//next x slot
    }//next category
//=============================================================================
//=============================================================================
//=============================================================================
    /* @todo remove series shapes if empty
    //remove and delete point-group-shape if empty
    if(!xSeriesGroupShape_Shapes->getCount())
    {
        (*aSeriesIter)->m_xShape.set(NULL);
        m_xLogicTarget->remove(xSeriesGroupShape_Shape);
    }
    */

    //remove and delete series-group-shape if empty

    //... todo
}

//e.g. for Rectangle
/*
uno::Reference< drawing::XShape > BarChart::createPartialPointShape(
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

//e.g. for BarChart in 2 dim cartesian coordinates:
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
