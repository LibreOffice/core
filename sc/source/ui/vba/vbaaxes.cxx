/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "vbaaxes.hxx"
#include "vbaaxis.hxx"
#include "vbachart.hxx"
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XlAxisType.hpp>
#include <ooo/vba/excel/XlAxisGroup.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlAxisType;
using namespace ::ooo::vba::excel::XlAxisGroup;

// each 'Item' in the Axes collection is  indexed via 2 indexes,  group and type.
// We need to 'flatten' this into a single index in order to be able to wrap
// iteration over the set of Axis(s) in a XIndexAccess implementation

typedef ::std::pair<sal_Int32, sal_Int32 > AxesCoordinate; // type and group combination
typedef ::std::vector< AxesCoordinate > vecAxesIndices;

namespace {

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
        EnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return m_xIndexAccess->getByIndex( nIndex++ );
                throw container::NoSuchElementException();
        }
};

}

uno::Reference< excel::XAxis >
ScVbaAxes::createAxis( const uno::Reference< excel::XChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext,  sal_Int32 nType, sal_Int32 nAxisGroup ) throw ( uno::RuntimeException, script::BasicErrorException )
{
    ScVbaChart* pChart = static_cast< ScVbaChart* >( xChart.get() );
    if ( !pChart )
        throw uno::RuntimeException("Object failure, can't access chart implementation"  );

    uno::Reference< beans::XPropertySet > xAxisPropertySet;
    if (((nType == xlCategory) || (nType == xlSeriesAxis) || (nType == xlValue)))
    {
        if ((nAxisGroup != xlPrimary) && (nAxisGroup != xlSecondary))
            DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
        xAxisPropertySet.set( pChart->getAxisPropertySet(nType, nAxisGroup), uno::UNO_QUERY_THROW );
    }
    else
        DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
    uno::Reference< XHelperInterface > xParent( xChart, uno::UNO_QUERY_THROW );
    return new ScVbaAxis( xParent, xContext, xAxisPropertySet, nType, nAxisGroup);
}

namespace {

class AxisIndexWrapper : public ::cppu::WeakImplHelper< container::XIndexAccess >
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
            bool bBool = false;
            uno::Reference< beans::XPropertySet > xDiagramPropertySet( pChart->xDiagramPropertySet() );
            if ( ( xDiagramPropertySet->getPropertyValue("HasXAxis") >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlCategory ) );
            if ( ( xDiagramPropertySet->getPropertyValue("HasYAxis") >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlSeriesAxis ) );

            if (  pChart->is3D() )
                mCoordinates.push_back( AxesCoordinate( xlPrimary, xlValue ) );

            // secondary
            if ( ( xDiagramPropertySet->getPropertyValue("HasSecondaryXAxis") >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlSecondary, xlCategory ) );
            if ( ( xDiagramPropertySet->getPropertyValue("HasSecondaryYAxis") >>= bBool )  && bBool )
                mCoordinates.push_back( AxesCoordinate( xlSecondary, xlSeriesAxis ) );
        }

    }
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException, std::exception) SAL_OVERRIDE { return mCoordinates.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, ::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        try
        {
            AxesCoordinate dIndexes = mCoordinates[ Index ];
            return uno::makeAny( ScVbaAxes::createAxis( mxChart, mxContext, dIndexes.second, dIndexes.first ) );
        }
        catch (const css::script::BasicErrorException& e)
        {
            throw css::lang::WrappedTargetException(
                   "Error Getting Index!",
                   static_cast < OWeakObject * > ( this ),
                   makeAny( e ) );
        }
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return cppu::UnoType<excel::XAxis>::get();
    }
    virtual sal_Bool SAL_CALL hasElements( ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return ( mCoordinates.size() > 0 );
    }
};

uno::Reference< container::XIndexAccess > createIndexWrapper( const uno::Reference< excel::XChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext )
{
    return new AxisIndexWrapper( xContext, xChart );
}

}

// #FIXME The collection semantics will never work as this object is not yet initialised correctly
ScVbaAxes::ScVbaAxes( const uno::Reference< XHelperInterface >& xParent,const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< excel::XChart >& xChart ) : ScVbaAxes_BASE( xParent, xContext, createIndexWrapper( xChart, xContext )), moChartParent( xChart )
{
}

uno::Type SAL_CALL
ScVbaAxes::getElementType() throw (css::uno::RuntimeException)
{
    return  cppu::UnoType<excel::XAxes>::get();
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaAxes::createEnumeration() throw (css::uno::RuntimeException)
{
    return new EnumWrapper( m_xIndexAccess );
}

uno::Any SAL_CALL
ScVbaAxes::Item( const css::uno::Any& _nType, const css::uno::Any& _oAxisGroup) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    // #TODO map the possible index combinations to a container::XIndexAccess wrapper impl
    // using a vector of valid std::pair maybe?
    // body helper api port bits
    sal_Int32 nAxisGroup = xlPrimary;
    sal_Int32 nType = -1;
    if ( !_nType.hasValue() || !( _nType >>= nType ) )
        throw uno::RuntimeException("Axes::Item Failed to extract type"  );

    if ( _oAxisGroup.hasValue() )
        _oAxisGroup >>= nAxisGroup ;

    return uno::makeAny( createAxis( moChartParent, mxContext, nType, nAxisGroup ) );
}

uno::Any
ScVbaAxes::createCollectionObject(const css::uno::Any& aSource)
{
    return aSource; // pass through ( it's already an XAxis object
}

OUString
ScVbaAxes::getServiceImplName()
{
    return OUString("ScVbaAxes");
}

uno::Sequence< OUString >
ScVbaAxes::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.Axes";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
