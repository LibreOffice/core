#include "AreaChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
//#include "chartview/servicenames_charttypes.hxx"
//#include "chartview/servicenames_coosystems.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "TransformationHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "Clipping.hxx"
#include "Splines.hxx"
#include "ChartTypeHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_SYMBOLPROPERTIES_HPP_
#include <drafts/com/sun/star/chart2/SymbolProperties.hpp>
#endif

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
// header for class Polygon3D
#ifndef _POLY3D_HXX
#include <svx/poly3d.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::drafts::com::sun::star::chart2;

class AreaPositionHelper : public PlottingPositionHelper
{
public:
    AreaPositionHelper(); //@todo
    virtual ~AreaPositionHelper();

    //double              getSlotPos( double fCategoryX ) const;

    double              getTransformedDepth() const;
    Rectangle           getTransformedClipRect() const;
    DoubleRectangle     getTransformedClipDoubleRect() const;

    double              getLogicGrounding() const;

private: //member
};

AreaPositionHelper::AreaPositionHelper()
{
}

AreaPositionHelper::~AreaPositionHelper()
{
}

double AreaPositionHelper::getTransformedDepth() const
{
    //return the depth for a logic 1
    double MinZ = getLogicMinZ();
    double MaxZ = getLogicMaxZ();
    if(m_aScales[2].Scaling.is())
        MinZ = m_aScales[2].Scaling->doScaling(MinZ);
    if(m_aScales[2].Scaling.is())
        MaxZ = m_aScales[2].Scaling->doScaling(MaxZ);

    return FIXED_SIZE_FOR_3D_CHART_VOLUME/(MaxZ-MinZ);
}

DoubleRectangle AreaPositionHelper::getTransformedClipDoubleRect() const
{
    //get logic clip values:
    double MinX = getLogicMinX();
    double MinY = getLogicMinY();
    double MinZ = getLogicMinZ();
    double MaxX = getLogicMaxX();
    double MaxY = getLogicMaxY();
    double MaxZ = getLogicMaxZ();

    //apply scaling
    doLogicScaling( &MinX, &MinY, &MinZ );
    doLogicScaling( &MaxX, &MaxY, &MaxZ);

    drawing::Position3D aMimimum( MinX, MinY, MinZ);
    drawing::Position3D aMaximum( MaxX, MaxY, MaxZ);

    //transform to screen coordinates
    aMimimum = SequenceToPosition3D( getTransformationLogicToScene()
                    ->transform( Position3DToSequence(aMimimum) ) );
    aMaximum = SequenceToPosition3D( getTransformationLogicToScene()
                    ->transform( Position3DToSequence(aMaximum) ) );

    DoubleRectangle aRet( aMimimum.PositionX
                  , aMaximum.PositionY
                  , aMaximum.PositionX
                  , aMimimum.PositionY );
    return aRet;
}

Rectangle AreaPositionHelper::getTransformedClipRect() const
{
    DoubleRectangle aDoubleRect( this->getTransformedClipDoubleRect() );

    Rectangle aRet( static_cast<long>(aDoubleRect.Left)
                  , static_cast<long>(aDoubleRect.Top)
                  , static_cast<long>(aDoubleRect.Right)
                  , static_cast<long>(aDoubleRect.Bottom) );
    return aRet;
}

double AreaPositionHelper::getLogicGrounding() const
{
    //@todo get this from model axis crosses at if that value is between min and max
    //@todo get this for other directions - so far only y
    double fRet=0.0;
    uno::Reference< lang::XServiceName > xServiceName( m_aScales[1].Scaling, uno::UNO_QUERY );
    bool bIsLogarithm = ( xServiceName.is() && (xServiceName->getServiceName()).equals(
                      C2U( "com.sun.star.chart2.LogarithmicScaling" )));
    if( bIsLogarithm )
        fRet = m_aScales[1].Minimum;
    return fRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

AreaChart::AreaChart( const uno::Reference<XChartType>& xChartTypeModel, bool bNoArea )
        : VSeriesPlotter( xChartTypeModel )
        , m_pPosHelper( new AreaPositionHelper() )
        , m_bArea(!bNoArea)
        , m_bLine(bNoArea)
        , m_bSymbol( ChartTypeHelper::isSupportingSymbolProperties(xChartTypeModel) )
        , m_eCurveStyle(CurveStyle_LINES)
        , m_nCurveResolution(20)
        , m_nSplineOrder(3)
{
    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue( C2U( "CurveStyle" ) ) >>= m_eCurveStyle;
            m_xChartTypeModelProps->getPropertyValue( C2U( "CurveResolution" ) ) >>= m_nCurveResolution;
            m_xChartTypeModelProps->getPropertyValue( C2U( "SplineOrder" ) ) >>= m_nSplineOrder;
        }
    }
    catch( uno::Exception& e )
    {
        e;
    }
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

AreaChart::~AreaChart()
{
    delete m_pPosHelper;
}

double AreaChart::getMaximumZ()
{
    if( 3!=m_nDimension )
        return VSeriesPlotter::getMaximumZ();

    return m_aXSlots.size();-0.5;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(AreaChart,CHART2_VIEW_AREACHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > AreaChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_AREACHART_SERVICE_NAME;
    return aSNS;
}
*/
/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL AreaChart
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN2D_SERVICE_NAME;
}

    void SAL_CALL AreaChart
::setScales( const uno::Sequence< ExplicitScaleData >& rScales ) throw (uno::RuntimeException)
{
}
    void SAL_CALL AreaChart
::setTransformation( const uno::Reference< XTransformation >& xTransformationToLogicTarget, const uno::Reference< XTransformation >& xTransformationToFinalPage ) throw (uno::RuntimeException)
{
}
*/

bool isPolygonEmptyOrSinglePoint(drawing::PolyPolygonShape3D& rPoly)
{
    if(!rPoly.SequenceX.getLength())
        return true;
    if(rPoly.SequenceX[0].getLength()<=1)
        return true;
    return false;
}

void closePolygon( drawing::PolyPolygonShape3D& rPoly)
{
    //add a last point == first point
    if(isPolygonEmptyOrSinglePoint(rPoly))
        return;
    drawing::Position3D aFirst(rPoly.SequenceX[0][0],rPoly.SequenceY[0][0],rPoly.SequenceZ[0][0]);
    AddPointToPoly( rPoly, aFirst );
}

void AreaChart::addSeries( VDataSeries* pSeries, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, xSlot, ySlot );
}

drawing::PolyPolygonShape3D createBorderPolygon(
                      drawing::PolyPolygonShape3D& rPoly
                    , double fDepth )
{
    drawing::PolyPolygonShape3D aRet;

    sal_Int32 nPolyCount = rPoly.SequenceX.getLength();
    sal_Int32 nBorder=0;
    for(sal_Int32 nPoly=0;nPoly<nPolyCount;nPoly++)
    {
        sal_Int32 nPointCount = rPoly.SequenceX[nPoly].getLength();
        for(sal_Int32 nPoint=0;nPoint<nPointCount-1;nPoint++)
        {
            aRet.SequenceX.realloc(nBorder+1);
            aRet.SequenceY.realloc(nBorder+1);
            aRet.SequenceZ.realloc(nBorder+1);

            aRet.SequenceX[nBorder].realloc(5);
            aRet.SequenceY[nBorder].realloc(5);
            aRet.SequenceZ[nBorder].realloc(5);

            aRet.SequenceX[nBorder][0] = aRet.SequenceX[nBorder][3] = aRet.SequenceX[nBorder][4] = rPoly.SequenceX[nPoly][nPoint];
            aRet.SequenceY[nBorder][0] = aRet.SequenceY[nBorder][3] = aRet.SequenceY[nBorder][4] = rPoly.SequenceY[nPoly][nPoint];
            aRet.SequenceZ[nBorder][0] = aRet.SequenceZ[nBorder][3] = aRet.SequenceZ[nBorder][4] = rPoly.SequenceZ[nPoly][nPoint];
            aRet.SequenceZ[nBorder][3] += fDepth;

            aRet.SequenceX[nBorder][1] = aRet.SequenceX[nBorder][2] = rPoly.SequenceX[nPoly][nPoint+1];
            aRet.SequenceY[nBorder][1] = aRet.SequenceY[nBorder][2] = rPoly.SequenceY[nPoly][nPoint+1];
            aRet.SequenceZ[nBorder][1] = aRet.SequenceZ[nBorder][2] = rPoly.SequenceZ[nPoly][nPoint+1];
            aRet.SequenceZ[nBorder][2] += fDepth;

            nBorder++;
        }
    }
    return aRet;
}

uno::Reference< drawing::XShape >
        create3DLine( const uno::Reference< drawing::XShapes >& xTarget
                    , uno::Reference< lang::XMultiServiceFactory > m_xShapeFactory
                    , drawing::PolyPolygonShape3D& rPoly
                    , double fDepth )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DExtrudeObject" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //depth
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)fDepth) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , uno::makeAny( rPoly ) );

            //NormalsKind
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALS_KIND )
                , uno::makeAny( drawing::NormalsKind_FLAT ) );

            //DoubleSided
            xProp->setPropertyValue( C2U( UNO_NAME_3D_DOUBLE_SIDED )
                , uno::makeAny( (sal_Bool)true) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

bool AreaChart::impl_createLine( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly )
{
    //return true if a line was created successfully
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xLogicTarget);

    m_pPosHelper->getTransformedClipRect();
    drawing::PolyPolygonShape3D aPoly;
    if(CurveStyle_CUBIC_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateCubicSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution );
        Clipping::clipPolygonAtRectangle( aSplinePoly, m_pPosHelper->getTransformedClipDoubleRect(), aPoly );
    }
    else if(CurveStyle_B_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateBSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution, m_nSplineOrder );
        Clipping::clipPolygonAtRectangle( aSplinePoly, m_pPosHelper->getTransformedClipDoubleRect(), aPoly );
    }
    else
    {
        Clipping::clipPolygonAtRectangle( *pSeriesPoly, m_pPosHelper->getTransformedClipDoubleRect(), aPoly );
    }

    if(isPolygonEmptyOrSinglePoint(aPoly))
        return false;

    //create line:
    uno::Reference< drawing::XShape > xShape(NULL);
    if(m_nDimension==3)
    {
        xShape = create3DLine( xSeriesGroupShape_Shapes, m_xShapeFactory
                , aPoly, m_pPosHelper->getTransformedDepth() );
        this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , m_aShapePropertyMapForArea );
        //createBorder
        {
            drawing::PolyPolygonShape3D aBorderPoly = createBorderPolygon(
                aPoly, m_pPosHelper->getTransformedDepth() );
            VLineProperties aLineProperties;
            aLineProperties.initFromPropertySet( pSeries->getPropertiesOfSeries(), true );
            uno::Reference< drawing::XShape > xBorder =
                m_pShapeFactory->createLine3D( xSeriesGroupShape_Shapes
                    , aBorderPoly, aLineProperties );
            //because of this name this line will be used for marking the axis
            m_pShapeFactory->setShapeName( xBorder, C2U("MarkHandles") );
        }
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createLine2D( xSeriesGroupShape_Shapes
                , PolyToPointSequence( aPoly ), VLineProperties() );
        this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
        //because of this name this line will be used for marking the axis
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
    return true;
}

bool AreaChart::impl_createArea( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly )
{
    //return true if an area was created successfully

    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xLogicTarget);
    double zValue = pSeries->m_fLogicZPos - 0.5;

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    //add second part to the polygon (grounding points or previous series points)
    if(!pPreviousSeriesPoly)
    {
        double fMinX = pSeries->m_fLogicMinX;
        double fMaxX = pSeries->m_fLogicMaxX;
        //clip to scale
        if(fMaxX<m_pPosHelper->getLogicMinX() || fMinX>m_pPosHelper->getLogicMaxX())
            return false;//no visible shape needed
        if(fMaxX > m_pPosHelper->getLogicMaxX())
            fMaxX = m_pPosHelper->getLogicMaxX();
        if(fMinX < m_pPosHelper->getLogicMinX())
            fMinX = m_pPosHelper->getLogicMinX();

        //apply scaling
        {
            m_pPosHelper->doLogicScaling( &fMinX, NULL, &zValue );
            m_pPosHelper->doLogicScaling( &fMaxX, NULL, NULL );
        }
        double fY = m_pPosHelper->getLogicGrounding();

        drawing::Position3D aScaledLogicPosition2( fMaxX,fY,zValue);
        drawing::Position3D aTransformedPosition2( SequenceToPosition3D( m_pPosHelper->getTransformationLogicToScene()->transform( Position3DToSequence(aScaledLogicPosition2) ) ) );
        AddPointToPoly( aPoly, aTransformedPosition2 );

        drawing::Position3D aScaledLogicPosition( fMinX,fY,zValue);
        drawing::Position3D aTransformedPosition( SequenceToPosition3D( m_pPosHelper->getTransformationLogicToScene()->transform( Position3DToSequence(aScaledLogicPosition) ) ) );
        AddPointToPoly( aPoly, aTransformedPosition );
    }
    else
    {
        appendPoly( aPoly, *pPreviousSeriesPoly );
    }
    closePolygon(aPoly);

    //apply clipping
    //(for more accurat clipping it would be better to first clip and than scale and transform,
    //but as long as we only have integer Polygon clipping we need to apply scaling and transformation first ) see QQQ
    {
        Polygon aToolsPoly = PolyToToolsPoly( aPoly );
        aToolsPoly.Clip( m_pPosHelper->getTransformedClipRect() );
        aPoly = ToolsPolyToPoly( aToolsPoly, zValue );
        closePolygon(aPoly); //again necessary after clipping
    }

    if(isPolygonEmptyOrSinglePoint(aPoly))
        return false;

    //create area:
    uno::Reference< drawing::XShape > xShape(NULL);
    if(m_nDimension==3)
    {
        xShape = m_pShapeFactory->createArea3D( xSeriesGroupShape_Shapes
                , aPoly, m_pPosHelper->getTransformedDepth() );
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createArea2D( xSeriesGroupShape_Shapes
                , aPoly );
    }
    this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , m_aShapePropertyMapForArea );
    //because of this name this line will be used for marking the axis
    m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    return true;
}

void AreaChart::impl_createSeriesShapes()
{
    //the polygon shapes for each series need to be created before

    //iterate through all series again to create the series shapes
    ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
    //for the area chart there should be at most one x slot (no side by side stacking available)
    //handle as if independent series
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

        ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
        const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
//=============================================================================

        drawing::PolyPolygonShape3D* pPreviousSeriesPoly = NULL;
        drawing::PolyPolygonShape3D* pSeriesPoly = NULL;

        //iterate through all series
        for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
        {
            pSeriesPoly = &(*aSeriesIter)->m_aPolyPolygonShape3D;

            if( m_bArea )
            {
                if( !impl_createArea( *aSeriesIter, pSeriesPoly, pPreviousSeriesPoly ) )
                    continue;
            }
            if( m_bLine )
            {
                if( !impl_createLine( *aSeriesIter, pSeriesPoly ) )
                    continue;
            }
            pPreviousSeriesPoly = pSeriesPoly;
        }//next series in x slot (next y slot)
    }//next x slot
}

void AreaChart::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"AreaChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    double fLogicZ        = -0.5;//as defined
    double fLogicBaseWidth = 1.0;//as defined
    double fLogicBaseDepth = fLogicBaseWidth;//Logic Depth and Width are identical by define ... (symmetry is not necessary anymore)

    sal_Int32 nStartIndex = 0; // inclusive       ;..todo get somehow from x scale
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount(m_aXSlots);
    if(nEndIndex<=0)
        nEndIndex=1;
//=============================================================================
    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
        //for the area chart there should be at most one x slot (no side by side stacking available)
        //attention different: xSlots are always interpreted as independent areas one behind the other: @todo this doesn't work why not???
        for( sal_Int32 nZ=0; aXSlotIter != aXSlotEnd; aXSlotIter++, nZ++ )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

            double fLogicYForNextSeries = 0.0;

            double fLogicYSum = 0.0;
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                double fAdd = (*aSeriesIter)->getY( nIndex );
                if( !::rtl::math::isNan(fAdd) )
                    fLogicYSum += fAdd;
            }
            aSeriesIter = pSeriesList->begin();
//=============================================================================
            //iterate through all series
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeFrontChild(*aSeriesIter, m_xLogicTarget);

                if(m_nDimension==3)
                    fLogicZ = nZ;
                (*aSeriesIter)->m_fLogicZPos = fLogicZ;

                //collect data point information (logic coordinates, style ):
                double fLogicX       = (*aSeriesIter)->getX(nIndex);
                double fLogicY       = (*aSeriesIter)->getY(nIndex);
                if( ::rtl::math::isNan(fLogicY) )
                    fLogicY=0;//@todo maybe there is another grounding ?? - for sum 0 is right
                fLogicY += fLogicYForNextSeries;
                fLogicYForNextSeries = fLogicY;

                bool bIsVisible = m_pPosHelper->isLogicVisible( fLogicX, fLogicY, fLogicZ );

                //remind minimal and maximal x values for area 'grounding' points
                //only for filled area
                {
                    double& rfMinX = (*aSeriesIter)->m_fLogicMinX;
                    if(!nIndex||fLogicX<rfMinX)
                        rfMinX=fLogicX;
                    double& rfMaxX = (*aSeriesIter)->m_fLogicMaxX;
                    if(!nIndex||fLogicX>rfMaxX)
                        rfMaxX=fLogicX;
                }

                //apply scaling
                //(for more accurat clipping it would be better to first clip and than scale and transform,
                //but as long as we only have integer Polygon clipping we need to apply scaling and transformation first ) see QQQ
                m_pPosHelper->doLogicScaling( &fLogicX, &fLogicY, &fLogicZ );

                //store point information for series polygon
                //for area and/or line (symbols only do not need this)
                if(    !::rtl::math::isNan(fLogicX) && !::rtl::math::isInf(fLogicX)
                    && !::rtl::math::isNan(fLogicY) && !::rtl::math::isInf(fLogicY)
                    && !::rtl::math::isNan(fLogicZ) && !::rtl::math::isInf(fLogicZ) )
                {
                    drawing::PolyPolygonShape3D& rSeriesPoly = (*aSeriesIter)->m_aPolyPolygonShape3D;
                    drawing::Position3D aScaledLogicPosition( fLogicX, fLogicY,fLogicZ);
                    //transformation 3) -> 4)
                    drawing::Position3D aTransformedPosition(
                        SequenceToPosition3D(
                            m_pPosHelper->getTransformationLogicToScene()->transform(
                                Position3DToSequence(aScaledLogicPosition) ) ) );
                    AddPointToPoly( rSeriesPoly, aTransformedPosition );
                }

                //create a single datapoint if point is visible
                //apply clipping:
                if( !bIsVisible )
                    continue;

                //create a group shape for this point and add to the series shape:
                rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                    (*aSeriesIter)->getPointCID_Stub(), nIndex );
                uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                    createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                        uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                {
                    DataPointGeometry aLogicGeom( drawing::Position3D(fLogicX,fLogicY,fLogicZ)
                                , drawing::Direction3D(1,1,1) );
                    //transformation 3) -> 4)
                    DataPointGeometry aTransformedGeom(
                        TransformationHelper::transformLogicGeom(
                            aLogicGeom,m_pPosHelper->getTransformationLogicToScene()) );

                    //create data point
                    if( m_bSymbol )
                    {
                        if(m_nDimension==3)
                        {
                            /* //no symbols for 3D
                            m_pShapeFactory->createSymbol3D( xPointGroupShape_Shapes
                                    , aTransformedGeom.m_aPosition, aTransformedGeom.m_aSize
                                    , (*aSeriesIter)->getSymbolTypeOfPoint( nIndex ) );
                                    */
                        }
                        else //m_nDimension!=3
                        {
                            SymbolProperties* pSymbolProperties = (*aSeriesIter)->getSymbolProperties( nIndex );
                            if( pSymbolProperties )
                            {
                                if( pSymbolProperties->aStyle == SymbolStyle_STANDARD )
                                {
                                    drawing::Direction3D aSize;
                                    aSize.DirectionX = pSymbolProperties->aSize.Width;
                                    aSize.DirectionY = pSymbolProperties->aSize.Height;
                                    m_pShapeFactory->createSymbol2D( xPointGroupShape_Shapes
                                            , aTransformedGeom.m_aPosition, aSize
                                            , pSymbolProperties->nStandardSymbol
                                            , pSymbolProperties->nFillColor );
                                }
                                //@todo other symbol styles
                            }
                        }
                    }
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

    impl_createSeriesShapes();

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

//.............................................................................
} //namespace chart
//.............................................................................
