/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaaxes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:43:28 $
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

#include "vbaaxes.hxx"
#include "vbaaxis.hxx"
#include "vbachart.hxx"
#include <org/openoffice/excel/XlAxisType.hpp>
#include <org/openoffice/excel/XlAxisGroup.hpp>
#include <org/openoffice/excel/XAxis.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::org::openoffice;
using namespace ::org::openoffice::excel::XlAxisType;
using namespace ::org::openoffice::excel::XlAxisGroup;

// each 'Item' in the Axes collection is  indexed via 2 indexes,  group and type.
// We need to 'flatten' this into a single index in order to be able to wrap
// iteration over the set of Axis(s) in a XIndexAccess implementation
//
typedef ::std::pair<sal_Int32, sal_Int32 > AxesCoordinate; // type and group combination
typedef ::std::vector< AxesCoordinate > vecAxesIndices;

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > AxisIndexWrapper_BASE;

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
        EnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return m_xIndexAccess->getByIndex( nIndex++ );
                throw container::NoSuchElementException();
        }
};


uno::Reference< excel::XAxis >
ScVbaAxes::createAxis( const uno::Reference< excel::XChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext,  sal_Int32 nType, sal_Int32 nAxisGroup ) throw ( uno::RuntimeException )
{
    ScVbaChart* pChart = static_cast< ScVbaChart* >( xChart.get() );
    if ( !pChart )
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Object failure, can't access chart implementation" ), uno::Reference< uno::XInterface >()  );

    uno::Reference< beans::XPropertySet > xAxisPropertySet;
    if (((nType == xlCategory) || (nType == xlSeriesAxis) || (nType == xlValue)))
    {
        if ((nAxisGroup != xlPrimary) && (nAxisGroup != xlSecondary))
            throw script::BasicErrorException( rtl::OUString(), NULL, SbERR_METHOD_FAILED, rtl::OUString());
        xAxisPropertySet.set( pChart->getAxisPropertySet(nType, nAxisGroup), uno::UNO_QUERY_THROW );
    }
    else
        throw script::BasicErrorException( rtl::OUString(), NULL, SbERR_METHOD_FAILED, rtl::OUString());
    uno::Reference< vba::XHelperInterface > xParent( xChart, uno::UNO_QUERY_THROW );
    return new ScVbaAxis( xParent, xContext, xAxisPropertySet, nType, nAxisGroup);
}

class AxisIndexWrapper : public AxisIndexWrapper_BASE
{
    // if necessary for better performance we could change this into a map and cache the
    // indices -> Axis, currently we create a new Axis object
    // on each getByIndex
    uno::Reference< uno::XComponentContext > mxContext;
    vecAxesIndices mCoordinates;
    uno::Reference< excel::XChart > mxChart;
public:
    AxisIndexWrapper( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< excel::XChart >& xChart ) : mxContext( xContext ), mxChart( xChart )
    {
        if ( mxChart.is() )
        {
            ScVbaChart* pChart = static_cast< ScVbaChart* >( mxChart.get() );
            // primary
            sal_Bool bBool = false;
            uno::Reference< beans::XPropertySet > xDiagramPropertySet( pChart->xDiagramPropertySet() );
            if ( ( xDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasXAxis" ) ) ) >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlCategory ) );
            if ( ( xDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasYAxis" ) ) ) >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlSeriesAxis ) );

            if (  pChart->is3D() )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlValue ) );

            // secondary
            if ( ( xDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryXAxis" ) ) ) >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlSecondary, xlCategory ) );
            if ( ( xDiagramPropertySet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HasSecondaryYAxis" ) ) ) >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlSecondary, xlSeriesAxis ) );
        }

    }
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException) { return mCoordinates.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, ::uno::RuntimeException)
    {
            AxesCoordinate dIndexes = mCoordinates[ Index ];
            return uno::makeAny( ScVbaAxes::createAxis( mxChart, mxContext, dIndexes.second, dIndexes.first ) );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        return excel::XAxis::static_type(0);
    }
    virtual ::sal_Bool SAL_CALL hasElements( ) throw (uno::RuntimeException)
    {
        return ( mCoordinates.size() > 0 );
    }
};

uno::Reference< container::XIndexAccess > createIndexWrapper( const uno::Reference< excel::XChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext )
{
    return new AxisIndexWrapper( xContext, xChart );
}

// #FIXME The collection semantics will never work as this object is not yet initialised correctly
ScVbaAxes::ScVbaAxes( const uno::Reference< vba::XHelperInterface >& xParent,const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< excel::XChart >& xChart ) : ScVbaAxes_BASE( xParent, xContext, createIndexWrapper( xChart, xContext )), moChartParent( xChart )
{
}

uno::Type SAL_CALL
ScVbaAxes::getElementType() throw (css::uno::RuntimeException)
{
    return  excel::XAxes::static_type(0);
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaAxes::createEnumeration() throw (css::uno::RuntimeException)
{
    return new EnumWrapper( m_xIndexAccess );
}

uno::Any SAL_CALL
ScVbaAxes::Item( const css::uno::Any& _nType, const css::uno::Any& _oAxisGroup) throw (css::uno::RuntimeException)
{
    // #TODO map the possible index combinations to a container::XIndexAccess wrapper impl
    // using a vector of valid std::pair maybe?
    // bodgy helperapi port bits
    sal_Int32 nAxisGroup = xlPrimary;
    sal_Int32 nType = -1;
    if ( !_nType.hasValue() || ( ( _nType >>= nType ) == sal_False )  )
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "Axes::Item Failed to extract type" ), uno::Reference< uno::XInterface >()  );

    if ( _oAxisGroup.hasValue() )
        _oAxisGroup >>= nAxisGroup ;

    return uno::makeAny( createAxis( moChartParent, mxContext, nType, nAxisGroup ) );
}

uno::Any
ScVbaAxes::createCollectionObject(const css::uno::Any& aSource)
{
    return aSource; // pass through ( it's already an XAxis object
}

rtl::OUString&
ScVbaAxes::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAxes") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaAxes::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Axes" ) );
    }
    return aServiceNames;
}

