#include "VSeriesPlotter.hxx"
#include "ShapeFactory.hxx"

#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "chartview/ObjectIdentifier.hxx"

//only for creation: @todo remove if all plotter are uno components and instanciated via servicefactory
#include "BarChart.hxx"
#include "PieChart.hxx"
#include "AreaChart.hxx"
//

#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
// header for class OUStringBuffer
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
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

VDataSeriesGroup::VDataSeriesGroup()
        : m_aSeriesVector()
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)

{
}

VDataSeriesGroup::VDataSeriesGroup( VDataSeries* pSeries )
        : m_aSeriesVector(1,pSeries)
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)
{
}

VDataSeriesGroup::VDataSeriesGroup( const ::std::vector< VDataSeries* >& rSeriesVector )
        : m_aSeriesVector(rSeriesVector)
        , m_bSumValuesDirty(true)
        , m_fPositiveSum(0.0)
        , m_fNegativeSum(0.0)
        , m_bMaxPointCountDirty(true)
        , m_nMaxPointCount(0)
{
}
VDataSeriesGroup::~VDataSeriesGroup()
{
}

void VDataSeriesGroup::deleteSeries()
{
    //delete all data series help objects:
    ::std::vector< VDataSeries* >::const_iterator       aIter = m_aSeriesVector.begin();
    const ::std::vector< VDataSeries* >::const_iterator aEnd  = m_aSeriesVector.end();
    for( ; aIter != aEnd; aIter++ )
    {
        delete *aIter;
    }
    m_aSeriesVector.clear();
}

void VDataSeriesGroup::addSeries( VDataSeries* pSeries )
{
    m_aSeriesVector.push_back(pSeries);
    m_bSumValuesDirty=true;
    m_bMaxPointCountDirty=true;
}

sal_Int32 VDataSeriesGroup::getSeriesCount() const
{
    return m_aSeriesVector.size();
}

void VDataSeriesGroup::setSums( double fPositiveSum, double fNegativeSum )
{
    m_fPositiveSum = fPositiveSum;
    m_fNegativeSum = fNegativeSum;
    m_bSumValuesDirty = false;
}

bool VDataSeriesGroup::getSums( double& rfPositiveSum, double& rfNegativeSum ) const
{
    if(m_bSumValuesDirty)
        return false;
    rfPositiveSum = m_fPositiveSum;
    rfNegativeSum = m_fNegativeSum;
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

VSeriesPlotter::VSeriesPlotter( sal_Int32 nDimension )
        : PlotterBase( nDimension )
        , m_aXSlots()
{
}

VSeriesPlotter::~VSeriesPlotter()
{
    //delete all data series help objects:
    ::std::vector< VDataSeriesGroup >::iterator            aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd =  m_aXSlots.end();
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        aXSlotIter->deleteSeries();
    }
    m_aXSlots.clear();
}

//shape property -- chart model object property
tMakePropertyNameMap VSeriesPlotter::m_aShapePropertyMapForArea =
        tMakePropertyNameMap
        //area properties
        ( C2U( UNO_NAME_FILLSTYLE ), C2U("FillStyle") )
        ( C2U( UNO_NAME_FILLCOLOR ), C2U("Color") )
        ( C2U(UNO_NAME_FILLGRADIENT), C2U("Gradient") )
        ( C2U(UNO_NAME_FILL_TRANSPARENCE), C2U("Transparency") )
        ( C2U("FillTransparenceGradient"), C2U("TransparencyGradient") )
        ( C2U(UNO_NAME_FILLHATCH), C2U("Hatch") )
        //line properties
        ( C2U(UNO_NAME_LINESTYLE), C2U("BorderStyle") )
        ( C2U(UNO_NAME_LINECOLOR), C2U("BorderColor") )
        ( C2U(UNO_NAME_LINEWIDTH), C2U("BorderWidth") )
        ( C2U(UNO_NAME_LINEDASH), C2U("BorderDash") )
        ( C2U(UNO_NAME_LINETRANSPARENCE), C2U("BorderTransparency") )
    ;

void VSeriesPlotter::addSeries( VDataSeries* pSeries, sal_Int32 xSlot,sal_Int32 ySlot )
{
    //take ownership of pSeries

    DBG_ASSERT( pSeries, "series to add is NULL" );
    if(!pSeries)
        return;

    if(xSlot<0 || xSlot>=static_cast<sal_Int32>(m_aXSlots.size()))
    {
        //append the series to already existing x series
        m_aXSlots.push_back( VDataSeriesGroup(pSeries) );
    }
    else
    {
        //x slot is already occupied
        //y slot decides what to do:

        VDataSeriesGroup& rYSlots = m_aXSlots[xSlot];
        sal_Int32 nYSlotCount = rYSlots.getSeriesCount();

        if( ySlot < -1 )
        {
            //move all existing series in the xSlot to next slot
            //@todo
            DBG_ASSERT(0,"Not implemented yet");
        }
        else if( ySlot == -1 || ySlot >= nYSlotCount)
        {
            //append the series to already existing y series
            rYSlots.addSeries(pSeries);
        }
        else
        {
            //y slot is already occupied
            //insert at given y and x position

            //@todo
            DBG_ASSERT(0,"Not implemented yet");
        }
    }
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getSeriesGroupShape( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(pDataSeries->m_xShape.is())
    {
        return uno::Reference<drawing::XShapes>( pDataSeries->m_xShape, uno::UNO_QUERY );
    }

    //create a group shape for this series and add to logic target:
    uno::Reference< drawing::XShapes > xShapes(
        createGroupShape( xTarget,pDataSeries->getCID() ));
    uno::Reference<drawing::XShape> xShape =
                uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    pDataSeries->m_xShape.set(xShape);
    return xShapes;
}

uno::Reference< drawing::XShapes > VSeriesPlotter::getLabelsGroupShape( VDataSeries* pDataSeries
                                        , const uno::Reference< drawing::XShapes >& xTarget )
{
    if(pDataSeries->m_xLabelsShape.is())
    {
        return uno::Reference<drawing::XShapes>( pDataSeries->m_xLabelsShape, uno::UNO_QUERY );
    }
    //create a group shape for this series and add to logic target:
    uno::Reference< drawing::XShapes > xShapes(
        createGroupShape( xTarget,pDataSeries->getLabelsCID() ));
    uno::Reference<drawing::XShape> xShape =
                uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    pDataSeries->m_xLabelsShape.set(xShape);
    return xShapes;

}

void VSeriesPlotter::createDataLabel( const uno::Reference< drawing::XShapes >& xTarget
                    , const VDataSeries& rDataSeries
                    , sal_Int32 nPointIndex
                    , double fValue
                    , double fSumValue
                    , const awt::Point& rScreenPosition2D )
{
    //check wether the label needs to be created and how:
    DataCaptionStyle* pCaption = rDataSeries.getDataCaptionStyle( nPointIndex );

    if( !pCaption || (!pCaption->ShowNumber && !pCaption->ShowNumberInPercent
        && !pCaption->ShowCategoryName && !pCaption->ShowLegendSymbol ) )
        return;

    //------------------------------------------------
    //prepare text
    ::rtl::OUStringBuffer aText;
    {
        if(pCaption->ShowNumberInPercent)
        {
            fValue = fValue*100.0/fSumValue;
            sal_Int32 nRound = static_cast< sal_Int32 >( fValue * 100.0 );
            fValue = nRound/100.0;
            if( fValue < 0 )
                fValue*=-1.0;
        }
        if(pCaption->ShowCategoryName)
            aText.append( rDataSeries.getCategoryString(nPointIndex) );

        if(pCaption->ShowNumber || pCaption->ShowNumberInPercent)
        {
            if(aText.getLength())
                aText.append(sal_Unicode(' '));

            //@todo as default use numberformat and formatter from calc fro this range
            sal_Unicode cDecSeparator = '.';//@todo get this locale dependent
            aText.append( ::rtl::math::doubleToUString( fValue
                            , rtl_math_StringFormat_G //rtl_math_StringFormat
                            , 3// DecPlaces
                            , cDecSeparator
                            //,sal_Int32 const * pGroups
                            //,sal_Unicode cGroupSeparator
                            ,false //bEraseTrailingDecZeros
                            ) );
        }
        if(pCaption->ShowNumberInPercent)
            aText.append(sal_Unicode('%'));
    }
    //------------------------------------------------
    //prepare properties for multipropertyset-interface of shape
    tNameSequence* pPropNames;
    tAnySequence* pPropValues;
    if( !rDataSeries.getTextLabelMultiPropertyLists( nPointIndex, pPropNames, pPropValues ) )
        return;
    uno::Any* pCIDAny = PropertyMapper::getValuePointer(*pPropValues,*pPropNames,C2U("Name"));
    if(pCIDAny)
    {
        rtl::OUString aCID = ObjectIdentifier::createPointCID( rDataSeries.getLabelCID_Stub(),nPointIndex );
        *pCIDAny = uno::makeAny(aCID);
    }
    //------------------------------------------------

    //create text shape
    uno::Reference< drawing::XShape > xTextShape = ShapeFactory(m_xShapeFactory).
        createText( xTarget, aText.makeStringAndClear()
                    , *pPropNames, *pPropValues
                    , ShapeFactory::makeTransformation( rScreenPosition2D ) );
}

void VSeriesPlotter::setMappedProperties(
          const uno::Reference< drawing::XShape >& xTargetShape
        , const uno::Reference< beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap )
{
    uno::Reference< beans::XPropertySet > xTargetProp( xTargetShape, uno::UNO_QUERY );
    PropertyMapper::setMappedProperties(xTargetProp,xSource,rMap);
}

//-------------------------------------------------------------------------
// MinimumAndMaximumSupplier
//-------------------------------------------------------------------------

double VSeriesPlotter::getMinimumX()
{
    //this is the default for all category charts
    //@todo for other types
    return -0.5;
}
double VSeriesPlotter::getMaximumX()
{
    //this is the default for all category charts
    //@todo for other types

    //return category count
    sal_Int32 nPointCount = getPointCount( m_aXSlots );
    return nPointCount-0.5;
}

double VSeriesPlotter::getMinimumYInRange( double fMinimumX, double fMaximumX )
{
    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for(size_t nN =0; nN<m_aXSlots.size();nN++ )
    {
        double fLocalMinimum, fLocalMaximum;
        if( m_aXSlots[nN].calculateYMinAndMaxForCategoryRange(
                            static_cast<sal_Int32>(fMinimumX+0.5)
                            , static_cast<sal_Int32>(fMaximumX+0.5)
                            , fLocalMinimum, fLocalMaximum ) )
        {
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMinimum))
        ::rtl::math::setNan(&fMinimum);
    return fMinimum;
}

double VSeriesPlotter::getMaximumYInRange( double fMinimumX, double fMaximumX )
{
    double fMinimum, fMaximum;
    ::rtl::math::setInf(&fMinimum, false);
    ::rtl::math::setInf(&fMaximum, true);
    for(size_t nN =0; nN<m_aXSlots.size();nN++ )
    {
        double fLocalMinimum, fLocalMaximum;
        if( m_aXSlots[nN].calculateYMinAndMaxForCategoryRange(
                            static_cast<sal_Int32>(fMinimumX+0.5)
                            , static_cast<sal_Int32>(fMaximumX+0.5)
                            , fLocalMinimum, fLocalMaximum ) )
        {
            if(fMaximum<fLocalMaximum)
                fMaximum=fLocalMaximum;
            if(fMinimum>fLocalMinimum)
                fMinimum=fLocalMinimum;
        }
    }
    if(::rtl::math::isInf(fMaximum))
        ::rtl::math::setNan(&fMaximum);
    return fMaximum;
}

//static
sal_Int32 VSeriesPlotter::getPointCount( const ::std::vector< VDataSeriesGroup >& rSlots )
{
    sal_Int32 nRet = 0;

    ::std::vector< VDataSeriesGroup >::const_iterator       aXSlotIter = rSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = rSlots.end();

    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        sal_Int32 nPointCount = getPointCount( *aXSlotIter );
        if( nPointCount>nRet )
            nRet = nPointCount;
    }
    return nRet;
}

//static
sal_Int32 VSeriesPlotter::getPointCount( const VDataSeriesGroup& rSeriesGroup )
{
    sal_Int32 nRet = 0;

    const ::std::vector< VDataSeries* >* pSeriesList = &(rSeriesGroup.m_aSeriesVector);

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd = pSeriesList->end();

    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        sal_Int32 nPointCount = (*aSeriesIter)->getTotalPointCount();
        if( nPointCount>nRet )
            nRet = nPointCount;
    }
    return nRet;
}

void VDataSeriesGroup::calculateYSumsForCategory( sal_Int32 nCategoryIndex
        , double& rfPositiveSum, double& rfNegativeSum )
{
    //@todo maybe cach these values

    ::std::vector< VDataSeries* >* pSeriesList = &this->m_aSeriesVector;

    ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
    const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

    double fLogicPositiveYSum = 0.0;
    double fLogicNegativeYSum = 0.0;
    for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
    {
        double fY = (*aSeriesIter)->getY( nCategoryIndex );
        if( ::rtl::math::isNan(fY) )
            continue;
        if(fY>=0)
            fLogicPositiveYSum+=fY;
        else
            fLogicNegativeYSum+=fY;
    }

    rfPositiveSum = fLogicPositiveYSum;
    rfNegativeSum = fLogicNegativeYSum;
}

bool VDataSeriesGroup::calculateYMinAndMaxForCategoryRange(
        sal_Int32 nStartCategoryIndex, sal_Int32 nEndCategoryIndex
        , double& rfMinimum, double& rfMaximum )
{
    //return true if valid values were found otherwise false

    //@todo maybe cache these values
    ::rtl::math::setInf(&rfMinimum, false);
    ::rtl::math::setInf(&rfMaximum, true);

    double fPositiveValue=0.0, fNegativeValue=0.0;
    //iterate through the given categories
    if(nStartCategoryIndex<0)
        nStartCategoryIndex=0;
    if(nEndCategoryIndex<0)
        nEndCategoryIndex=0;
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex < nEndCategoryIndex; nCatIndex++ )
    {
        this->calculateYSumsForCategory( nCatIndex, fPositiveValue, fNegativeValue );
        if(rfMaximum<fPositiveValue)
            rfMaximum=fPositiveValue;
        if(rfMinimum>fNegativeValue)
            rfMinimum=fNegativeValue;
    }
    return !::rtl::math::isInf( rfMinimum ) && !::rtl::math::isInf( rfMaximum );
}

//static
VSeriesPlotter* VSeriesPlotter::createSeriesPlotter( const rtl::OUString& rChartType, sal_Int32 nDimension )
{
    //@todo: in future the plotter should be instanciated via service factory
    VSeriesPlotter* pRet=NULL;
    if( rChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.BarChart")) )
        pRet = new BarChart(nDimension);
    else if( rChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.AreaChart")) )
        pRet = new AreaChart(nDimension,true,false,false);
    else if( rChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.LineChart")) )
        pRet = new AreaChart(nDimension,false,true,true);
    else if( rChartType.equalsIgnoreAsciiCase(C2U("com.sun.star.chart2.PieChart")) )
        pRet = new PieChart(nDimension);
    else
    {
        //@todo create other charttypes
        //com.sun.star.chart2.NetChart?
        //com.sun.star.chart2.ScatterChart?
        pRet = new BarChart(nDimension);
    }

    return pRet;
}

//.............................................................................
} //namespace chart
//.............................................................................
