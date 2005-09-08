/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDataSeries.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:50:51 $
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
#include "VDataSeries.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif

//#include "CommonConverters.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XNUMERICALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XNumericalDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VDataSeries::VDataSeries()
{
    DBG_ERROR("not implemented");
}

VDataSeries::VDataSeries( const uno::Reference< XDataSeries >& xDataSeries )
    : m_xShape(NULL)
    , m_xLabelsShape(NULL)
    , m_xErrorBarsShape(NULL)
    , m_xShapeFrontChild(NULL)
    , m_xShapeBackChild(NULL)
    , m_xDataSeries(xDataSeries)
    , m_aDataSequences()
    , m_nPointCount(0)
    , m_xData_XValues(NULL)
    , m_xData_YValues(NULL)
    , m_xData_ZValues(NULL)

    , m_apLabel_Series(NULL)
    , m_apLabelPropNames_Series(NULL)
    , m_apLabelPropValues_Series(NULL)
    , m_apLabel_AttributedPoint(NULL)
    , m_apLabelPropNames_AttributedPoint(NULL)
    , m_apLabelPropValues_AttributedPoint(NULL)

    , m_nCurrentAttributedPoint(-1)
    , m_apSymbolProperties_Series(NULL)
    , m_apSymbolProperties_AttributedPoint(NULL)
{
    uno::Reference<XDataSource> xDataSource =
            uno::Reference<XDataSource>( xDataSeries, uno::UNO_QUERY );

    m_aDataSequences = xDataSource->getDataSequences();

    for(sal_Int32 nN = m_aDataSequences.getLength();nN--;)
    {
        uno::Reference<XDataSequence>  xDataSequence = m_aDataSequences[nN];
        uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                uno::Any aARole = xProp->getPropertyValue( C2U( "Role" ) );
                rtl::OUString aRole;
                aARole >>= aRole;

                if( aRole.equals(C2U("x-values")) )
                {
                    m_xData_XValues = xDataSequence;
                    m_XValues_Double = DataSequenceToDoubleSequence( m_xData_XValues );
                }
                else if( aRole.equals(C2U("y-values")) )
                {
                    m_xData_YValues = xDataSequence;
                    m_YValues_Double = DataSequenceToDoubleSequence( m_xData_YValues );
                    m_nPointCount = xDataSequence->getData().getLength(); //@todo determination of m_nPointCount  may needs to be improved (e.g. max of x,y,z or something)
                }
                //@todo assign the other roles (+ error for unknown?)
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }

    uno::Reference<beans::XPropertySet> xProp(xDataSeries, uno::UNO_QUERY );
    if( xProp.is())
    {
        try
        {
            //get CID
            uno::Any aAIdentifier = xProp->getPropertyValue( C2U( "Identifier" ) );
            aAIdentifier >>= m_aIdentifier;

            m_aCID = ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DATA_SERIES, m_aIdentifier );
            m_aPointCID_Stub = ObjectIdentifier::createPointCID_Stub( m_aCID );

            m_aLabelCID_Stub = ObjectIdentifier::createClassifiedIdentifier(
                                OBJECTTYPE_DATA_LABEL, ::rtl::OUString(), getLabelsCID() );

            //get AttributedDataPoints
            xProp->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= m_aAttributedDataPointIndexList;
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

VDataSeries::~VDataSeries()
{
}

uno::Reference< XDataSeries > VDataSeries::getModel() const
{
    return m_xDataSeries;
}

void VDataSeries::setCategoryXAxis()
{
    m_xData_XValues = NULL;
    m_XValues_Double.realloc(0);
}

rtl::OUString VDataSeries::getCID() const
{
    return m_aCID;
}
rtl::OUString VDataSeries::getPointCID_Stub() const
{
    return m_aPointCID_Stub;
}
rtl::OUString VDataSeries::getErrorBarsCID() const
{
    return ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DATA_ERRORS, m_aIdentifier );
}
rtl::OUString VDataSeries::getLabelsCID() const
{
    return ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_DATA_LABELS, m_aIdentifier );
}
rtl::OUString VDataSeries::getLabelCID_Stub() const
{
    return m_aLabelCID_Stub;
}
rtl::OUString VDataSeries::getDataCurveCID( const uno::Reference< beans::XPropertySet >& xDataCurveModelProp ) const
{
    rtl::OUString aRet;
    uno::Reference< XIdentifiable > xCurveIdentifier( xDataCurveModelProp, uno::UNO_QUERY );
    if(xCurveIdentifier.is())
        aRet = ObjectIdentifier::createDataCurveCID( m_aIdentifier, xCurveIdentifier->getIdentifier() );
    return aRet;
}

sal_Int32 VDataSeries::getTotalPointCount() const
{
    return m_nPointCount;
}

double VDataSeries::getX( sal_Int32 index ) const
{
    if(m_xData_XValues.is())
    {
        if( 0<=index && index<m_XValues_Double.getLength() )
            return m_XValues_Double[index];
    }
    else
    {
        if( 0<=index && index < m_YValues_Double.getLength() )
            return index+1;//first category (index 0) matches with real number 1.0
    }
    double fNan;
    ::rtl::math::setNan( & fNan );
    return fNan;
}

double VDataSeries::getY( sal_Int32 index ) const
{
    if( 0<=index && index<m_YValues_Double.getLength() )
        return m_YValues_Double[index];
    else
    {
        double fNan;
        ::rtl::math::setNan( & fNan );
        return fNan;
    }
}

uno::Sequence< double > VDataSeries::getAllX() const
{
    if(!m_xData_XValues.is() && !m_XValues_Double.getLength() && m_YValues_Double.getLength())
    {
        //init x values from category indexes
        //first category (index 0) matches with real number 1.0
        m_XValues_Double.realloc( m_YValues_Double.getLength() );
        for(sal_Int32 nN=m_XValues_Double.getLength();nN--;)
            m_XValues_Double[nN] = nN+1;
    }
    return m_XValues_Double;
}

uno::Sequence< double > VDataSeries::getAllY() const
{
    return m_YValues_Double;
}

rtl::OUString VDataSeries::getCategoryString( sal_Int32 index ) const
{
    //@todo get real Category string if any - else return empty string
    rtl::OUString aRet;
    if( 0<=index && index<m_nPointCount )
    {
        aRet+=C2U("index ");
        aRet+=rtl::OUString::valueOf(index+1);//first category (index 0) matches with real number 1.0
    }
    return aRet;
}

::std::auto_ptr< Symbol > getSymbolPropertiesFromPropertySet(
        const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< Symbol > apSymbolProps( new Symbol() );
    try
    {
        if( xProp->getPropertyValue( C2U( "Symbol" ) ) >>= *apSymbolProps )
        {
            //use main color to fill symbols
            xProp->getPropertyValue( C2U( "Color" ) ) >>= apSymbolProps->nFillColor;
        }
        else
            apSymbolProps.reset();
    }
    catch( uno::Exception &e)
    {
        ASSERT_EXCEPTION( e );
    }
    return apSymbolProps;
}

Symbol* VDataSeries::getSymbolProperties( sal_Int32 index ) const
{
    Symbol* pRet=NULL;
    if( isAttributedDataPoint( index ) )
    {
        if(!m_apSymbolProperties_AttributedPoint.get() || m_nCurrentAttributedPoint!=index)
        {
            m_apSymbolProperties_AttributedPoint = getSymbolPropertiesFromPropertySet( this->getPropertiesOfPoint( index ) );
            m_nCurrentAttributedPoint = index;
        }
        pRet = m_apSymbolProperties_AttributedPoint.get();
    }
    else
    {
        if(!m_apSymbolProperties_Series.get())
            m_apSymbolProperties_Series = getSymbolPropertiesFromPropertySet( this->getPropertiesOfPoint( index ) );
        pRet = m_apSymbolProperties_Series.get();
    }
    return pRet;
}

bool VDataSeries::isAttributedDataPoint( sal_Int32 index ) const
{
    //returns true if the data point assigned by the given index has set it's own properties
    if( index>=m_nPointCount || m_nPointCount==0)
        return false;
    for(sal_Int32 nN=m_aAttributedDataPointIndexList.getLength();nN--;)
    {
        if(index==m_aAttributedDataPointIndexList[nN])
            return true;
    }
    return false;
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfPoint( sal_Int32 index ) const
{
    if( isAttributedDataPoint( index ) )
        return m_xDataSeries->getDataPointByIndex(index);
    return this->getPropertiesOfSeries();
}

uno::Reference< beans::XPropertySet > VDataSeries::getPropertiesOfSeries() const
{
    return  uno::Reference<beans::XPropertySet>(m_xDataSeries, uno::UNO_QUERY );
}

::std::auto_ptr< DataPointLabel > getDataPointLabelFromPropertySet(
        const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< DataPointLabel > apLabel( new DataPointLabel() );
    try
    {
        if( !(xProp->getPropertyValue( C2U( "Label" ) ) >>= *apLabel) )
            apLabel.reset();
    }
    catch( uno::Exception &e)
    {
        ASSERT_EXCEPTION( e );
    }
    return apLabel;
}

DataPointLabel* VDataSeries::getDataPointLabel( sal_Int32 index ) const
{
    DataPointLabel* pRet = NULL;
    if( isAttributedDataPoint( index ) )
    {
        if(!m_apLabel_AttributedPoint.get() || m_nCurrentAttributedPoint!=index)
        {
            m_apLabel_AttributedPoint = getDataPointLabelFromPropertySet( this->getPropertiesOfPoint( index ) );
            m_nCurrentAttributedPoint = index;
        }
        pRet = m_apLabel_AttributedPoint.get();
    }
    else
    {
        if(!m_apLabel_Series.get())
            m_apLabel_Series = getDataPointLabelFromPropertySet( this->getPropertiesOfPoint( index ) );
        pRet = m_apLabel_Series.get();
    }
    return pRet;
}

DataPointLabel* VDataSeries::getDataPointLabelIfLabel( sal_Int32 index ) const
{
    DataPointLabel* pLabel = this->getDataPointLabel( index );
    if( !pLabel || (!pLabel->ShowNumber && !pLabel->ShowNumberInPercent
        && !pLabel->ShowCategoryName && !pLabel->ShowLegendSymbol ) )
        return 0;
    return pLabel;
}

bool VDataSeries::getTextLabelMultiPropertyLists( sal_Int32 index
    , tNameSequence*& pPropNames
    , tAnySequence*& pPropValues ) const
{
    pPropNames = NULL; pPropValues = NULL;
    if( isAttributedDataPoint( index ) )
    {
        if(!m_apLabelPropValues_AttributedPoint.get() || m_nCurrentAttributedPoint!=index)
        {
            pPropNames = new tNameSequence();
            pPropValues = new tAnySequence();
            PropertyMapper::getTextLabelMultiPropertyLists(
                this->getPropertiesOfPoint( index ), *pPropNames, *pPropValues );
            m_apLabelPropNames_AttributedPoint = ::std::auto_ptr< tNameSequence >(pPropNames);
            m_apLabelPropValues_AttributedPoint = ::std::auto_ptr< tAnySequence >(pPropValues);

            m_nCurrentAttributedPoint = index;
        }
        pPropNames = m_apLabelPropNames_AttributedPoint.get();
        pPropValues = m_apLabelPropValues_AttributedPoint.get();
    }
    else
    {
        if(!m_apLabelPropValues_Series.get())
        {
            pPropNames = new tNameSequence();
            pPropValues = new tAnySequence();
            PropertyMapper::getTextLabelMultiPropertyLists(
                this->getPropertiesOfPoint( index ), *pPropNames, *pPropValues );
            m_apLabelPropNames_Series = ::std::auto_ptr< tNameSequence >(pPropNames);
            m_apLabelPropValues_Series = ::std::auto_ptr< tAnySequence >(pPropValues);
        }
        pPropNames = m_apLabelPropNames_Series.get();
        pPropValues = m_apLabelPropValues_Series.get();
    }
    if(pPropNames&&pPropValues)
        return true;
    return false;
}

//-----------------------------------------------------------------------------

/*
//static
sal_Int32 VDataSeries::getCategoryCount(
    const uno::Sequence< uno::Reference< XDataSeries > >& rSeriesList )
{
    sal_Int32 nRet = 1;
    for( sal_Int32 nN = rSeriesList.getLength(); nN--; )
    {
        uno::Reference<XDataSource> xDataSource =
            uno::Reference<XDataSource>( rSeriesList[nN], uno::UNO_QUERY );

        uno::Sequence< uno::Reference< XDataSequence > > aDataSequences
            = xDataSource->getDataSequences();

        for(sal_Int32 nN = aDataSequences.getLength();nN--;)
        {
            uno::Reference<XDataSequence>  xDataSequence = aDataSequences[nN];
            uno::Reference<beans::XPropertySet> xProp(xDataSequence, uno::UNO_QUERY );
            if( xProp.is())
            {
                try
                {
                    uno::Any aARole = xProp->getPropertyValue( C2U( "Role" ) );
                    rtl::OUString aRole;
                    aARole >>= aRole;

                    if( aRole.equals(C2U("categories")) )
                    {
                        sal_Int32 nCount = xDataSequence->getData().getLength();
                        if(nRet<nCount)
                            nRet = nCount;
                    }
                }
                catch( uno::Exception& e )
                {
                    e;
                }
            }
        }

    }
    return nRet;
}
*/

//.............................................................................
} //namespace chart
//.............................................................................
