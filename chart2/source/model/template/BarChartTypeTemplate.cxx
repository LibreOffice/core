/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BarChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:02:50 $
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
#include "BarChartTypeTemplate.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "DataSeriesHelper.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATAPOINTGEOMETRY3D_HPP_
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::rtl::OUString;

namespace
{

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.BarChartTypeTemplate" ));

enum
{
    PROP_BAR_TEMPLATE_DIMENSION,
    PROP_BAR_TEMPLATE_GEOMETRY3D
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Dimension" ),
                  PROP_BAR_TEMPLATE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Geometry3D" ),
                  PROP_BAR_TEMPLATE_GEOMETRY3D,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_BAR_TEMPLATE_DIMENSION, 2 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BAR_TEMPLATE_GEOMETRY3D, ::chart2::DataPointGeometry3D::CUBOID );
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

} // anonymous namespace

namespace chart
{

BarChartTypeTemplate::BarChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    BarDirection eDirection,
    sal_Int32 nDim         /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode ),
        m_eBarDirection( eDirection ),
        m_nDim( nDim )
{}

BarChartTypeTemplate::~BarChartTypeTemplate()
{}

sal_Int32 BarChartTypeTemplate::getDimension() const
{
    return m_nDim;
}

StackMode BarChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

bool BarChartTypeTemplate::isSwapXAndY() const
{
    return (m_eBarDirection == HORIZONTAL);
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL BarChartTypeTemplate::matchesTemplate(
    const Reference< chart2::XDiagram >& xDiagram,
    sal_Bool bAdaptProperties )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram, bAdaptProperties );

    //check BarDirection
    if( bResult )
    {
        bool bFound = false;
        bool bAmbiguous = false;
        bool bVertical = DiagramHelper::getVertical( xDiagram, bFound, bAmbiguous );
        if( m_eBarDirection == HORIZONTAL )
            bResult = sal_Bool( bVertical );
        else if( m_eBarDirection == VERTICAL )
            bResult = sal_Bool( !bVertical );
    }

    // adapt solid-type of template according to values in series
    if( bAdaptProperties &&
        bResult &&
        getDimension() == 3 )
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));

        bool bGeomFound = false, bGeomAmbiguous = false;
        sal_Int32 aCommonGeom = DiagramHelper::getGeometry3D( xDiagram, bGeomFound, bGeomAmbiguous );

        if( !bGeomAmbiguous )
        {
            setFastPropertyValue_NoBroadcast(
                PROP_BAR_TEMPLATE_GEOMETRY3D, uno::makeAny( aCommonGeom ));
        }
    }

    return bResult;
}
Reference< chart2::XChartType > BarChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    Reference< chart2::XChartType > xResult;

    try
    {
        Reference< lang::XMultiServiceFactory > xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW );
        xResult.set( xFact->createInstance(
                         CHART2_SERVICE_NAME_CHARTTYPE_COLUMN ), uno::UNO_QUERY_THROW );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

Reference< chart2::XChartType > SAL_CALL BarChartTypeTemplate::getChartTypeForNewSeries(
        const uno::Sequence< Reference< chart2::XChartType > >& aFormerlyUsedChartTypes )
    throw (uno::RuntimeException)
{
    Reference< chart2::XChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}


// ____ OPropertySet ____
uno::Any BarChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL BarChartTypeTemplate::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    BarChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

void SAL_CALL BarChartTypeTemplate::applyStyle(
    const Reference< chart2::XDataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    if( getDimension() == 3 )
    {
        try
        {
            //apply Geometry3D
            uno::Any aAGeometry3D;
            this->getFastPropertyValue( aAGeometry3D, PROP_BAR_TEMPLATE_GEOMETRY3D );
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "Geometry3D" ), aAGeometry3D );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void SAL_CALL BarChartTypeTemplate::resetStyles(
    const Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    ChartTypeTemplate::resetStyles( xDiagram );
    if( getDimension() == 3 )
    {
        ::std::vector< Reference< chart2::XDataSeries > > aSeriesVec(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ));
        uno::Any aLineStyleAny( uno::makeAny( drawing::LineStyle_NONE ));
        for( ::std::vector< Reference< chart2::XDataSeries > >::iterator aIt( aSeriesVec.begin());
             aIt != aSeriesVec.end(); ++aIt )
        {
            Reference< beans::XPropertyState > xState( *aIt, uno::UNO_QUERY );
            if( xState.is())
            {
                xState->setPropertyToDefault( C2U("Geometry3D"));
                Reference< beans::XPropertySet > xProp( xState, uno::UNO_QUERY );
                if( xProp.is() &&
                    xProp->getPropertyValue( C2U("BorderStyle")) == aLineStyleAny )
                {
                    xState->setPropertyToDefault( C2U("BorderStyle"));
                }
            }
        }
    }

    DiagramHelper::setVertical( xDiagram, false );
}


void BarChartTypeTemplate::createCoordinateSystems(
    const Reference< chart2::XCoordinateSystemContainer > & xCooSysCnt )
{
    ChartTypeTemplate::createCoordinateSystems( xCooSysCnt );

    Reference< chart2::XDiagram > xDiagram( xCooSysCnt, uno::UNO_QUERY );
    DiagramHelper::setVertical( xDiagram, m_eBarDirection == HORIZONTAL );
}

// ----------------------------------------

Sequence< OUString > BarChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( BarChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BarChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart
