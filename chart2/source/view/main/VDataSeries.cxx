/*************************************************************************
 *
 *  $RCSfile: VDataSeries.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:34 $
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
#include "VDataSeries.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"

//#include "CommonConverters.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XNUMERICALDATASEQUENCE_HPP_
#include <drafts/com/sun/star/chart2/XNumericalDataSequence.hpp>
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
using namespace ::drafts::com::sun::star::chart2;
/*
void PlottingPositionHelper::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    m_aMatrixScreenToScene = HomogenMatrixToMatrix4D(rMatrix);
}

void PlottingPositionHelper::setScales( const uno::Sequence< ExplicitScaleData >& rScales )
{
    m_aScales = rScales;
}
*/
VDataSeries::VDataSeries()
    : m_xShape(NULL)
    , m_xLabelsShape(NULL)
    , m_aAppearanceOfSeries(12632256) //LIGHT_GRAY is default color for dataseries for testing
    , m_xDataSeries(NULL)
    , m_aDataSequences()
    , m_nPointCount(0)
    , m_xData_XValues(NULL)
    , m_xData_YValues(NULL)
    , m_xData_ZValues(NULL)

    , m_apCaption_Series(NULL)
    , m_apLabelPropNames_Series(NULL)
    , m_apLabelPropValues_Series(NULL)
    , m_apCaption_AttributedPoint(NULL)
    , m_apLabelPropNames_AttributedPoint(NULL)
    , m_apLabelPropValues_AttributedPoint(NULL)
{

}

void initDoubleValues( uno::Sequence< double >& rDoubleValues,
                      const uno::Reference< XDataSequence >& xDataSequence )
{
    OSL_ASSERT( xDataSequence.is());
    if(!xDataSequence.is())
        return;

    uno::Reference< XNumericalDataSequence > xNumericalDataSequence
        = uno::Reference< XNumericalDataSequence >( xDataSequence, uno::UNO_QUERY );
    if( xNumericalDataSequence.is() )
    {
        rDoubleValues = xNumericalDataSequence->getNumericalData();
    }
    else
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        rDoubleValues.realloc(aValues.getLength());
        for(sal_Int32 nN=aValues.getLength();nN--;)
        {
            if( !(aValues[nN] >>= rDoubleValues[nN]) )
            {
                ::rtl::math::setNan( &rDoubleValues[nN] );
            }
        }
    }
}

VDataSeries::VDataSeries( uno::Reference< XDataSeries > xDataSeries, const ShapeAppearance& rDefaultAppearence )
    : m_xShape(NULL)
    , m_xLabelsShape(NULL)
    , m_aAppearanceOfSeries(rDefaultAppearence) //LIGHT_GRAY is default color for dataseries for testing
    , m_xDataSeries(xDataSeries)
    , m_aDataSequences()
    , m_nPointCount(0)
    , m_xData_XValues(NULL)
    , m_xData_YValues(NULL)
    , m_xData_ZValues(NULL)

    , m_apCaption_Series(NULL)
    , m_apLabelPropNames_Series(NULL)
    , m_apLabelPropValues_Series(NULL)
    , m_apCaption_AttributedPoint(NULL)
    , m_apLabelPropNames_AttributedPoint(NULL)
    , m_apLabelPropValues_AttributedPoint(NULL)
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
                }
                else if( aRole.equals(C2U("values")) )
                {
                    m_xData_YValues = xDataSequence;
                    initDoubleValues( m_YValues_Double, m_xData_YValues );
                    m_nPointCount = xDataSequence->getData().getLength(); //@todo determination of m_nPointCount  may needs to be improved (e.g. max of x,y,z or something)
                }
                //@todo assign the other roles (+ error for unknown?)
            }
            catch( uno::Exception& e )
            {
                e;
            }
        }
    }


    uno::Reference<beans::XPropertySet> xProp(xDataSeries, uno::UNO_QUERY );
    if( xProp.is())
    {
        try
        {
            //get style
            uno::Any aAFirstColor = xProp->getPropertyValue( C2U( "Color" ) );
            sal_Int32 nFirstColor;
            if(aAFirstColor>>=nFirstColor)
                m_aAppearanceOfSeries.m_nColorData = nFirstColor;

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
            e;
        }
    }
}

VDataSeries::~VDataSeries()
{
}

rtl::OUString VDataSeries::getCID() const
{
    return m_aCID;
}
rtl::OUString VDataSeries::getPointCID_Stub() const
{
    return m_aPointCID_Stub;
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

sal_Int32 VDataSeries::getTotalPointCount() const
{
    return m_nPointCount;
}

double VDataSeries::getX( sal_Int32 index ) const
{
    if(m_xData_XValues.is())
    {
        return 1; //@todo
    }
    else
    {
        return index;
    }
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

rtl::OUString VDataSeries::getCategoryString( sal_Int32 index ) const
{
    //@todo get real Category string if any - else return empty string
    rtl::OUString aRet;
    if( 0<=index && index<m_nPointCount )
    {
        aRet+=C2U("index ");
        aRet+=rtl::OUString::valueOf(index);
    }
    return aRet;
}

ShapeAppearance VDataSeries::getAppearanceOfPoint( sal_Int32 index ) const
{
    return m_aAppearanceOfSeries;
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

::std::auto_ptr< DataCaptionStyle > getDataCaptionStyleFromPropertySet(
        const uno::Reference< beans::XPropertySet >& xProp )
{
    ::std::auto_ptr< DataCaptionStyle > apCaption( new DataCaptionStyle() );
    try
    {
        if( !(xProp->getPropertyValue( C2U( "DataCaption" ) ) >>= *apCaption) )
            apCaption.reset();
    }
    catch( uno::Exception &e)
    {
        e;
    }
    return apCaption;
}

DataCaptionStyle* VDataSeries::getDataCaptionStyle( sal_Int32 index ) const
{
    DataCaptionStyle* pRet = NULL;
    if( isAttributedDataPoint( index ) )
    {
        if(!m_apCaption_AttributedPoint.get() || m_nCurrentAttributedPoint!=index)
        {
            m_apCaption_AttributedPoint = getDataCaptionStyleFromPropertySet( this->getPropertiesOfPoint( index ) );
            m_nCurrentAttributedPoint = index;
        }
        pRet = m_apCaption_AttributedPoint.get();
    }
    else
    {
        if(!m_apCaption_Series.get())
            m_apCaption_Series = getDataCaptionStyleFromPropertySet( this->getPropertiesOfPoint( index ) );
        pRet = m_apCaption_Series.get();
    }
    return pRet;
}


void createTextLabelMultiPropertyListsFromPropertySet(
            const uno::Reference< beans::XPropertySet >& xSourceProp
            , ::std::auto_ptr< tNameSequence >& rLabelPropNames
            , ::std::auto_ptr< tAnySequence >& rLabelPropValues
            )
{
    tPropertyNameValueMap aValueMap;

    //fill character properties into the ValueMap
    PropertyMapper::getValueMap( aValueMap
            , PropertyMapper::getPropertyNameMapForCharacterProperties()
            , uno::Reference< beans::XPropertySet >::query( xSourceProp ) //Text Properties source
            );

    //-------------------------
    //some more shape properties apart from character properties, position-matrix and label string

    //@todo get correct horizontal and vertical adjust and writing mode
    drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
    drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;
    //text::WritingMode eWritingMode = text::WritingMode_LR_TB;//@todo get correct one

    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("LineStyle"), uno::makeAny(drawing::LineStyle_NONE) ) ); // drawing::LineStyle
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextHorizontalAdjust"), uno::makeAny(eHorizontalAdjust) ) ); // drawing::TextHorizontalAdjust
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextVerticalAdjust"), uno::makeAny(eVerticalAdjust) ) ); //drawing::TextVerticalAdjust
    //aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextWritingMode"), uno::makeAny(eWritingMode) ) ); //text::WritingMode
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowHeight"), uno::makeAny(sal_True) ) ); // sal_Bool
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("TextAutoGrowWidth"), uno::makeAny(sal_True) ) ); // sal_Bool
    aValueMap.insert( tPropertyNameValueMap::value_type( C2U("Name"), uno::makeAny( rtl::OUString() ) ) ); //CID rtl::OUString - needs to be overwritten for each point

    //-------------------------
    tNameSequence* pPropNames = new tNameSequence();
    tAnySequence* pPropValues = new tAnySequence();
    PropertyMapper::getMultiPropertyListsFromValueMap( *pPropNames, *pPropValues, aValueMap );

    rLabelPropNames = ::std::auto_ptr< tNameSequence >(pPropNames);
    rLabelPropValues = ::std::auto_ptr< tAnySequence >(pPropValues);
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
            createTextLabelMultiPropertyListsFromPropertySet(
                this->getPropertiesOfPoint( index )
                , m_apLabelPropNames_AttributedPoint
                , m_apLabelPropValues_AttributedPoint );
            m_nCurrentAttributedPoint = index;
        }
        pPropNames = m_apLabelPropNames_AttributedPoint.get();
        pPropValues = m_apLabelPropValues_AttributedPoint.get();
    }
    else
    {
        if(!m_apLabelPropValues_Series.get())
        {
            createTextLabelMultiPropertyListsFromPropertySet(
                this->getPropertiesOfPoint( index )
                , m_apLabelPropNames_Series
                , m_apLabelPropValues_Series );
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
