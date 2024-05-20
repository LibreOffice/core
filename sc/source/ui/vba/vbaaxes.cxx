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
#include <basic/sberrors.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <ooo/vba/excel/XlAxisType.hpp>
#include <ooo/vba/excel/XlAxisGroup.hpp>
#include <ooo/vba/excel/XAxis.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace ::ooo::vba;
using namespace ::ooo::vba::excel::XlAxisType;
using namespace ::ooo::vba::excel::XlAxisGroup;

// each 'Item' in the Axes collection is  indexed via 2 indexes,  group and type.
// We need to 'flatten' this into a single index in order to be able to wrap
// iteration over the set of Axis(s) in a XIndexAccess implementation

typedef ::std::pair<sal_Int32, sal_Int32 > AxesCoordinate; // type and group combination

namespace {

class EnumWrapper : public EnumerationHelper_BASE
{
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
        explicit EnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : m_xIndexAccess(std::move( xIndexAccess )), nIndex( 0 ) {}
        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) override
        {
                if ( nIndex < m_xIndexAccess->getCount() )
                        return m_xIndexAccess->getByIndex( nIndex++ );
                throw container::NoSuchElementException();
        }
};

}

uno::Reference< excel::XAxis >
ScVbaAxes::createAxis( const uno::Reference< excel::XChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext,  sal_Int32 nType, sal_Int32 nAxisGroup )
{
    ScVbaChart* pChart = static_cast< ScVbaChart* >( xChart.get() );
    if ( !pChart )
        throw uno::RuntimeException(u"Object failure, can't access chart implementation"_ustr  );

    uno::Reference< beans::XPropertySet > xAxisPropertySet;
    if ((nType == xlCategory) || (nType == xlSeriesAxis) || (nType == xlValue))
    {
        if ((nAxisGroup != xlPrimary) && (nAxisGroup != xlSecondary))
            DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED);
        xAxisPropertySet.set( pChart->getAxisPropertySet(nType, nAxisGroup), uno::UNO_SET_THROW );
    }
    else
        DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED);
    return new ScVbaAxis( pChart, xContext, xAxisPropertySet, nType, nAxisGroup);
}

namespace {

class AxisIndexWrapper : public ::cppu::WeakImplHelper< container::XIndexAccess >
{
    // if necessary for better performance we could change this into a map and cache the
    // indices -> Axis, currently we create a new Axis object
    // on each getByIndex
    uno::Reference< uno::XComponentContext > mxContext;
    std::vector< AxesCoordinate > mCoordinates;
    rtl::Reference< ScVbaChart > mxChart;
public:
    AxisIndexWrapper( uno::Reference< uno::XComponentContext > xContext, rtl::Reference< ScVbaChart > xChart ) : mxContext(std::move( xContext )), mxChart(std::move( xChart ))
    {
        if ( !mxChart.is() )
            return;

        ScVbaChart* pChart = mxChart.get();
        // primary
        bool bBool = false;
        uno::Reference< beans::XPropertySet > xDiagramPropertySet( pChart->xDiagramPropertySet() );
        if ( ( xDiagramPropertySet->getPropertyValue(u"HasXAxis"_ustr) >>= bBool )  && bBool )
            mCoordinates.emplace_back( xlPrimary, xlCategory );
        if ( ( xDiagramPropertySet->getPropertyValue(u"HasYAxis"_ustr) >>= bBool )  && bBool )
            mCoordinates.emplace_back( xlPrimary, xlSeriesAxis );

        if (  pChart->is3D() )
            mCoordinates.emplace_back( xlPrimary, xlValue );

        // secondary
        if ( ( xDiagramPropertySet->getPropertyValue(u"HasSecondaryXAxis"_ustr) >>= bBool )  && bBool )
            mCoordinates.emplace_back( xlSecondary, xlCategory );
        if ( ( xDiagramPropertySet->getPropertyValue(u"HasSecondaryYAxis"_ustr) >>= bBool )  && bBool )
            mCoordinates.emplace_back( xlSecondary, xlSeriesAxis );

    }
    virtual ::sal_Int32 SAL_CALL getCount() override { return mCoordinates.size(); }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        try
        {
            AxesCoordinate dIndexes = mCoordinates[ Index ];
            return uno::Any( ScVbaAxes::createAxis( mxChart, mxContext, dIndexes.second, dIndexes.first ) );
        }
        catch (const css::script::BasicErrorException&)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetException(
                   u"Error Getting Index!"_ustr,
                   getXWeak(),
                   anyEx );
        }
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<excel::XAxis>::get();
    }
    virtual sal_Bool SAL_CALL hasElements( ) override
    {
        return ( !mCoordinates.empty() );
    }
};

uno::Reference< container::XIndexAccess > createIndexWrapper( const rtl::Reference< ScVbaChart >& xChart, const uno::Reference< uno::XComponentContext >& xContext )
{
    return new AxisIndexWrapper( xContext, xChart );
}

}

// #FIXME The collection semantics will never work as this object is not yet initialised correctly
ScVbaAxes::ScVbaAxes( const uno::Reference< XHelperInterface >& xParent,const uno::Reference< uno::XComponentContext > & xContext, const rtl::Reference< ScVbaChart >& xChart ) : ScVbaAxes_BASE( xParent, xContext, createIndexWrapper( xChart, xContext )), moChartParent( xChart )
{
}

uno::Type SAL_CALL
ScVbaAxes::getElementType()
{
    return  cppu::UnoType<excel::XAxes>::get();
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaAxes::createEnumeration()
{
    return new EnumWrapper( m_xIndexAccess );
}

uno::Any SAL_CALL
ScVbaAxes::Item( const css::uno::Any& _nType, const css::uno::Any& _oAxisGroup)
{
    // #TODO map the possible index combinations to a container::XIndexAccess wrapper impl
    // using a vector of valid std::pair maybe?
    // body helper api port bits
    sal_Int32 nAxisGroup = xlPrimary;
    sal_Int32 nType = -1;
    if ( !_nType.hasValue() || !( _nType >>= nType ) )
        throw uno::RuntimeException(u"Axes::Item Failed to extract type"_ustr  );

    if ( _oAxisGroup.hasValue() )
        _oAxisGroup >>= nAxisGroup ;

    return uno::Any( createAxis( moChartParent, mxContext, nType, nAxisGroup ) );
}

uno::Any
ScVbaAxes::createCollectionObject(const css::uno::Any& aSource)
{
    return aSource; // pass through ( it's already an XAxis object
}

OUString
ScVbaAxes::getServiceImplName()
{
    return u"ScVbaAxes"_ustr;
}

uno::Sequence< OUString >
ScVbaAxes::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.Axes"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
