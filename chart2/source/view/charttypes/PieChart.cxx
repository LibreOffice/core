#include "PieChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
//#include "chartview/servicenames_charttypes.hxx"
//#include "chartview/servicenames_coosystems.hxx"

#include "CommonConverters.hxx"
#include "Linear3DTransformation.hxx"
#include "ViewDefines.hxx"
#include "chartview/ObjectIdentifier.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

class PiePositionHelper : public PlottingPositionHelper
{
public:
    PiePositionHelper();
    virtual ~PiePositionHelper();

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation >
                        getTransformationLogicToScene( sal_Int32 nDim ) const;

    double              getCatCount() const {
                            double fMin = getLogicMinX();
                            double fMax = getLogicMaxX();
                            //categories are defined to be at positive whole numbers only
                            if(fMax<0.0)
                                return 0.0;
                            if(fMin<0.0)
                                fMin=0.0;
                            //don't remove cast
                            return (long)(fMax - fMin)+1; }

    double              getInnerRadius( double fCategoryX, bool& bIsVisible ) const;
    double              getOuterRadius( double fCategoryX ) const;

    DataPointGeometry   transformLogicGeom( const DataPointGeometry& rGeom, sal_Int32 nDim  ) const;

    sal_Int32           getStartCategoryIndex() const {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nStart = static_cast<sal_Int32>(getLogicMinX() - 0.5);
                            if( nStart < 0 )
                                nStart = 0;
                            return nStart;
                        }
    sal_Int32           getEndCategoryIndex() const  {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nEnd = static_cast<sal_Int32>(getLogicMaxX() - 0.5);
                            if( nEnd < 0 )
                                nEnd = 0;
                            return nEnd;
                        }

private: //member
    double      m_fDepth;

public:
    //Radius Offset for all rings in absolute logic values (1.0 == 1 category)
    double      m_fRingOffset;

    //Distance between different category rings, seen relative to width of a ring:
    double      m_fRingDistance; //>=0 m_fRingDistance=1 --> distance == width
};

PiePositionHelper::PiePositionHelper()
        : m_fDepth(1.0)
        , m_fRingOffset(0.5)
        , m_fRingDistance(0.2)
{
}

PiePositionHelper::~PiePositionHelper()
{
}

double PiePositionHelper::getInnerRadius( double fCategoryX, bool& bIsVisible ) const
{
    bIsVisible = true;
    double fRet = m_fRingOffset + fCategoryX-0.5+m_fRingDistance/(2.0*(1+m_fRingDistance)) - getLogicMinX();
    if(fRet<m_fRingOffset)
        fRet=m_fRingOffset;
    if(fRet>(m_fRingOffset+getLogicMaxX()-getLogicMinX()))
    {
        fRet=m_fRingOffset+getLogicMaxX()- getLogicMinX();
        bIsVisible = false;
    }
    doLogicScaling(&fRet,NULL,NULL);
    return fRet;
}

double PiePositionHelper::getOuterRadius( double fCategoryX ) const
{
    double fRet = m_fRingOffset + fCategoryX+0.5-m_fRingDistance/(2.0*(1+m_fRingDistance)) - getLogicMinX();
    if(fRet<m_fRingOffset)
        fRet=m_fRingOffset;
    if(fRet>(m_fRingOffset+getLogicMaxX()-getLogicMinX()))
        fRet=m_fRingOffset+getLogicMaxX()- getLogicMinX();
    doLogicScaling(&fRet,NULL,NULL);
    return fRet;
}


uno::Reference< XTransformation > PiePositionHelper::getTransformationLogicToScene(
            sal_Int32 nDim ) const
{
    //transformation from 2) to 4) //@todo 2) and 4) need a ink to a document

    //?? need to apply this transformation to each geometric object, or would group be sufficient??

    if( !m_xTransformationLogicToScene.is() )
    {
        double fLogicDiameter = 2*(getLogicMaxX() - getLogicMinX() + m_fRingOffset);

        Matrix4D aMatrix;
        //the middle of the pie circle is the middle of the diagram
        aMatrix.TranslateX(fLogicDiameter/2.0);
        aMatrix.ScaleX(FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter);

        aMatrix.TranslateY(fLogicDiameter/2.0);
        aMatrix.ScaleY(FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter);

        aMatrix.ScaleZ(FIXED_SIZE_FOR_3D_CHART_VOLUME/m_fDepth);

//        if(nDim==2)
            aMatrix = m_aMatrixScreenToScene*aMatrix;

        m_xTransformationLogicToScene = new Linear3DTransformation(Matrix4DToHomogenMatrix( aMatrix ));
    }
    return m_xTransformationLogicToScene;
}

DataPointGeometry PiePositionHelper::transformLogicGeom( const DataPointGeometry& rGeom, sal_Int32 nDim  ) const
{
    uno::Reference< XTransformation > xTransformation = getTransformationLogicToScene( nDim );
    DataPointGeometry aTransformedGeom( rGeom );

    /*
    DataPointGeometry aLogicGeom( drawing::Position3D(0.0,0.0,0.0)
                                , drawing::Direction3D(fOuterXDiameter,fOuterYDiameter,fDepth)
                                , drawing::Direction3D(fInnerXDiameter,fStartAngleDegree,fWidthAngleDegree) );
    */
    aTransformedGeom.m_aPosition = SequenceToPosition3D( xTransformation->transform( Position3DToSequence(rGeom.m_aPosition) ) );

    drawing::Position3D aLogicPos1;
    drawing::Position3D aLogicPos2( aLogicPos1 );
    aLogicPos2.PositionX += rGeom.m_aSize2.DirectionX;//fInnerXDiameter;
    aLogicPos2.PositionZ += rGeom.m_aSize.DirectionZ;
    drawing::Position3D aLogicPos3( aLogicPos1 );
    aLogicPos3.PositionX += rGeom.m_aSize.DirectionX; //fOuterXDiameter;
    aLogicPos3.PositionY += rGeom.m_aSize.DirectionY; //fOuterYDiameter;

    drawing::Position3D aTransformedPos1( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos1) ) ) );
    drawing::Position3D aTransformedPos2( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos2) ) ) );
    drawing::Position3D aTransformedPos3( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos3) ) ) );


    double fTransformedInnerXDiameter = aTransformedPos2.PositionX - aTransformedPos1.PositionX;
    double fTransformedOuterXDiameter = aTransformedPos3.PositionX - aTransformedPos1.PositionX;
    double fTransformedOuterYDiameter = aTransformedPos3.PositionY - aTransformedPos1.PositionY;
    double fTransformedDepth = aTransformedPos2.PositionZ - aTransformedPos1.PositionZ;

    aTransformedGeom.m_aSize2.DirectionX = fTransformedInnerXDiameter;
    aTransformedGeom.m_aSize.DirectionX = fTransformedOuterXDiameter;
    aTransformedGeom.m_aSize.DirectionY = fTransformedOuterYDiameter;
    aTransformedGeom.m_aSize.DirectionZ = fTransformedDepth;

    return aTransformedGeom;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PieChart::PieChart( const uno::Reference<XChartType>& xChartTypeModel )
        : VSeriesPlotter( xChartTypeModel )
        , m_pPosHelper( new PiePositionHelper() )
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

PieChart::~PieChart()
{
    delete m_pPosHelper;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(PieChart,CHART2_VIEW_PIECHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > PieChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_PIECHART_SERVICE_NAME;
    return aSNS;
}
*/

/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL PieChart
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN2D_SERVICE_NAME;
}

    void SAL_CALL PieChart
::setScales( const uno::Sequence< ExplicitScaleData >& rScales ) throw (uno::RuntimeException)
{
}
    void SAL_CALL PieChart
::setTransformation( const uno::Reference< XTransformation >& xTransformationToLogicTarget, const uno::Reference< XTransformation >& xTransformationToFinalPage ) throw (uno::RuntimeException)
{
}
*/

bool PieChart::isSingleRingChart() const
{
    return m_pPosHelper->getEndCategoryIndex()==1 ;
}

awt::Point PieChart::getLabelScreenPositionAndAlignment( LabelAlignment& rAlignment, bool bOutsidePosition
        , double fAngleDegree, double fOuterRadius, double fInnerRadius, double fLogicZ) const
{
    //outer positions only for pure pie chart(one ring only)

    double fAnglePi = fAngleDegree*F_PI/180.0;
    double fRadius = 0.0;
    if( bOutsidePosition )
    {
        fRadius = fOuterRadius + 0.1*fOuterRadius;
        if(fAngleDegree<=22.5)
            rAlignment = LABEL_ALIGN_RIGHT;
        else if(fAngleDegree<67.5)
            rAlignment = LABEL_ALIGN_RIGHT_TOP;
        else if(fAngleDegree<112.5)
            rAlignment = LABEL_ALIGN_TOP;
        else if(fAngleDegree<=157.5)
            rAlignment = LABEL_ALIGN_LEFT_TOP;
        else if(fAngleDegree<=202.5)
            rAlignment = LABEL_ALIGN_LEFT;
        else if(fAngleDegree<247.5)
            rAlignment = LABEL_ALIGN_LEFT_BOTTOM;
        else if(fAngleDegree<292.5)
            rAlignment = LABEL_ALIGN_BOTTOM;
        else if(fAngleDegree<337.5)
            rAlignment = LABEL_ALIGN_RIGHT_BOTTOM;
        else
            rAlignment = LABEL_ALIGN_RIGHT;
    }
    else
    {
        fRadius = fInnerRadius + (fOuterRadius-fInnerRadius)/2.0 ;
        rAlignment = LABEL_ALIGN_CENTER;
    }
    drawing::Position3D aLogicPos(fRadius*cos(fAnglePi),fRadius*sin(fAnglePi),fLogicZ);

    uno::Reference< XTransformation > xTransformation = m_pPosHelper->getTransformationLogicToScene( m_nDimension );
    drawing::Position3D aScreenPosition3D( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos) ) ) );
    return awt::Point( aScreenPosition3D.PositionX, aScreenPosition3D.PositionY );
}

uno::Reference< drawing::XShape > PieChart::createDataPoint2D(
          const uno::Reference< drawing::XShapes >& xTarget
        , const DataPointGeometry& rGeometry
        , const uno::Reference< beans::XPropertySet >& xObjectProperties )
{
    uno::Reference< drawing::XShape > xShape =
        m_pShapeFactory->createPieSegment2D( xTarget, rGeometry );

    this->setMappedProperties( xShape, xObjectProperties, m_aShapePropertyMapForArea );
    return xShape;
}

uno::Reference< drawing::XShape > PieChart::createDataPoint3D(
        const uno::Reference< drawing::XShapes >& xTarget
        , const DataPointGeometry& rGeometry
        , const uno::Reference< beans::XPropertySet >& xObjectProperties )
{
    uno::Reference< drawing::XShape > xShape =
        m_pShapeFactory->createPieSegment( xTarget, rGeometry );

    this->setMappedProperties( xShape, xObjectProperties, m_aShapePropertyMapForArea );
    return xShape;
}

void PieChart::addSeries( VDataSeries* pSeries, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, 0, ySlot );
}

void PieChart::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"PieChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));
    uno::Reference< drawing::XShapes > xTextTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));

    if( this->isSingleRingChart() )
    {
        m_pPosHelper->m_fRingOffset = 0.0;
        m_pPosHelper->m_fRingDistance = 0.0;
    }

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    double fLogicZ        = -0.5;//as defined

    //(@todo maybe different iteration for breaks in axis ?)
    sal_Int32 nStartCategoryIndex = m_pPosHelper->getStartCategoryIndex(); // inclusive
    sal_Int32 nEndCategoryIndex   = m_pPosHelper->getEndCategoryIndex(); //inclusive
//=============================================================================
    //iterate through all shown categories
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex <= nEndCategoryIndex; nCatIndex++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
        //for the pie chart there should be at most one x slot per category (one ring is one x slot and there is only one ring per category)
        if( aXSlotIter != aXSlotEnd )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

            double fLogicYForNextSeries = 0.0;

            double fLogicYSum = 0.0;
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                double fY = (*aSeriesIter)->getY( nCatIndex );
                if(fY<0.0)
                {
                    //@todo warn somehow that negative values are treated as positive
                }
                if( ::rtl::math::isNan(fY) )
                    continue;
                fLogicYSum += fabs(fY);
            }
            if(fLogicYSum==0.0)
                continue;
            aSeriesIter = pSeriesList->begin();

            bool bIsVisible;
            double fInnerXRadius      = m_pPosHelper->getInnerRadius( (double)nCatIndex+1.0, bIsVisible );
            if( !bIsVisible )
                continue;
            double fOuterXRadius      = m_pPosHelper->getOuterRadius( (double)nCatIndex+1.0 );
            double fOuterYRadius      = fOuterXRadius;
            //the radii are already clipped and had scaling now

            double fDepth           = 1.0;
//=============================================================================
            //iterate through all series in this x slot (in this ring)
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(*aSeriesIter, xSeriesTarget);
                //collect data point information (logic coordinates, style ):
                double fLogicYValue = fabs((*aSeriesIter)->getY( nCatIndex ));
                if( ::rtl::math::isNan(fLogicYValue) )
                    continue;
                if(fLogicYValue==0.0)//@todo: continue also if the resolution to small
                    continue;
                double fLogicYPos = fLogicYForNextSeries;
                fLogicYForNextSeries += fLogicYValue;

                rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                    (*aSeriesIter)->getPointCID_Stub(), nCatIndex );
                //each subsystem may add an additional shape to form the whole point
                //create a group shape for this point and add to the series shape:
                uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                    createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                        uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                //iterate through all subsystems to create partial points
                {
                    //clip logic coordinates:
                    //check fLogicX only necessary if we will allow breaks in category axes (see above for-loop)
                    //check fLogicZ hm ...
                    //check fLogicYValue    get Y-scale for this series/stack group
                    //@todo fLogicYValue =... fInnerRadius = ...

                    //----------------------------------
                    double fStartAngleDegree = fLogicYPos/fLogicYSum*360.0;
                    double fWidthAngleDegree = fLogicYValue/fLogicYSum*360.0;

                    DataPointGeometry aLogicGeom( drawing::Position3D(0.0,0.0,0.0)
                                , drawing::Direction3D(2.0*fOuterXRadius,2.0*fOuterYRadius,fDepth)
                                , drawing::Direction3D(2.0*fInnerXRadius,fStartAngleDegree,fWidthAngleDegree) );

                    //@todo: consider scaling here (transformation logic to logic)
                    //assumed scaling here: linear

                    //transformation 3) -> 4)
                    DataPointGeometry aTransformedGeom( m_pPosHelper->transformLogicGeom(aLogicGeom, m_nDimension) );

                    if(m_nDimension==3)
                    {
                        uno::Reference< drawing::XShape >  xShape = createDataPoint3D(
                            xPointGroupShape_Shapes
                            , aTransformedGeom
                            ,(*aSeriesIter)->getPropertiesOfPoint( nCatIndex ));
                    }
                    else //m_nDimension!=3
                    {
                        uno::Reference< drawing::XShape >  xShape = createDataPoint2D(
                            xPointGroupShape_Shapes
                            , aTransformedGeom
                            ,(*aSeriesIter)->getPropertiesOfPoint( nCatIndex ));
                    }
                    //create data point label
                    LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
                    awt::Point aScreenPosition2D( this->getLabelScreenPositionAndAlignment(eAlignment, this->isSingleRingChart()
                        , fStartAngleDegree + fWidthAngleDegree/2.0, fOuterXRadius, fInnerXRadius, fLogicZ ));
                    this->createDataLabel( xTextTarget, **aSeriesIter, nCatIndex
                                    , fLogicYValue, fLogicYSum, aScreenPosition2D, eAlignment );
                }

                //remove PointGroupShape if empty
                if(!xPointGroupShape_Shapes->getCount())
                    xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

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
uno::Reference< drawing::XShape > PieChart::createPartialPointShape(
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

//e.g. for PieChart in 2 dim cartesian coordinates:
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
