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
    double MinZ = getLogicMinZ();
    double MaxZ = getLogicMaxZ();
    if(m_aScales[2].Scaling.is())
        MinZ = m_aScales[2].Scaling->doScaling(MinZ);
    if(m_aScales[2].Scaling.is())
        MaxZ = m_aScales[2].Scaling->doScaling(MaxZ);
    return FIXED_SIZE_FOR_3D_CHART_VOLUME/(MaxZ-MinZ);
}

Rectangle AreaPositionHelper::getTransformedClipRect() const
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

    Rectangle aRet( static_cast<long>(aMimimum.PositionX)
                  , static_cast<long>(aMaximum.PositionY)
                  , static_cast<long>(aMaximum.PositionX)
                  , static_cast<long>(aMimimum.PositionY) );
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

AreaChart::AreaChart( sal_Int32 nDimension, sal_Bool bArea, sal_Bool bLine, sal_Bool bSymbol )
        : VSeriesPlotter( nDimension )
        , m_pPosHelper( new AreaPositionHelper() )
        , m_bArea(bArea)
        , m_bLine(bLine)
        , m_bSymbol(bSymbol)
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

AreaChart::~AreaChart()
{
    delete m_pPosHelper;
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

            /*
            //Normals Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALSPOLYGON3D )
                , uno::makeAny( aNewPoly ) );
                */
                /*
            //NormalsKind
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALS_KIND )
                , uno::makeAny( drawing::NormalsKind_FLAT ) );

            //LineOnly
            xProp->setPropertyValue( C2U( UNO_NAME_3D_LINEONLY )
                , uno::makeAny( (sal_Bool)false) );

            //DoubleSided
            xProp->setPropertyValue( C2U( UNO_NAME_3D_DOUBLE_SIDED )
                , uno::makeAny( (sal_Bool)true) );
                */
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
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries, m_xLogicTarget);

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    double fMinZ = m_pPosHelper->getLogicMinZ();
    m_pPosHelper->doLogicScaling( NULL, NULL, &fMinZ );

    Polygon aToolsPoly = PolyToToolsPoly( aPoly );
    aToolsPoly.Clip( m_pPosHelper->getTransformedClipRect() );
    aPoly = ToolsPolyToPoly( aToolsPoly, fMinZ );

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
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createLine2D( xSeriesGroupShape_Shapes
                , PolyToPointSequence( aPoly ), LineProperties() );
        this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
    }
    return true;
}

bool AreaChart::impl_createArea( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly )
{
    //return true if an area was created successfully

    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries, m_xLogicTarget);
    double zValue = m_pPosHelper->getLogicMinZ();

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    //add second part to the polygon (grounding points or previous series points)
    if(!pPreviousSeriesPoly)
    {
        double fMinX = pSeries->m_fMinX;
        double fMaxX = pSeries->m_fMaxX;
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
    if( aXSlotIter != aXSlotEnd )
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
    double fLogicZ        = 0.0;//as defined
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
        for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
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
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(*aSeriesIter, m_xLogicTarget);

                //set z order for series group shape
                {
                    uno::Reference< beans::XPropertySet > xProp( xSeriesGroupShape_Shapes, uno::UNO_QUERY );
                    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
                    if( xProp.is())
                    {
                        try
                        {
                            uno::Any aAZOrder = xProp->getPropertyValue( C2U( UNO_NAME_MISC_OBJ_ZORDER ) );
                            sal_Int32 nZOrder=33;
                            aAZOrder >>= nZOrder;
                            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_ZORDER )
                                , uno::makeAny( sal_Int32(0) ) );
                        }
                        catch( uno::Exception& e )
                        {
                            e;
                        }
                    }
                }

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
                    double& rfMinX = (*aSeriesIter)->m_fMinX;
                    if(!nIndex||fLogicX<rfMinX)
                        rfMinX=fLogicX;
                    double& rfMaxX = (*aSeriesIter)->m_fMaxX;
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
                            m_pShapeFactory->createSymbol3D( xPointGroupShape_Shapes
                                    , aTransformedGeom
                                    , (*aSeriesIter)->getAppearanceOfPoint( nIndex ) );
                        }
                        else //m_nDimension!=3
                        {
                            m_pShapeFactory->createSymbol2D( xPointGroupShape_Shapes
                                    , aTransformedGeom
                                    , (*aSeriesIter)->getAppearanceOfPoint( nIndex ) );
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
