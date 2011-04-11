/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART_DATASERIES_HXX
#define _CHART_DATASERIES_HXX

// UNO types
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

// helper classes
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase8.hxx>
#include <comphelper/uno3.hxx>
#include <osl/mutex.hxx>

// STL
#include <vector>
#include <map>

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"

namespace com { namespace sun { namespace star { namespace style {
    class XStyle;
}}}}

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper8<
        ::com::sun::star::chart2::XDataSeries,
        ::com::sun::star::chart2::data::XDataSink,
        ::com::sun::star::chart2::data::XDataSource,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::chart2::XRegressionCurveContainer,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    DataSeries_Base;
}

class DataSeries :
    public MutexContainer,
    public impl::DataSeries_Base,
    public ::property::OPropertySet
{
public:
    explicit DataSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSeries();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSeries )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    explicit DataSeries( const DataSeries & rOther );

    // late initialization to call after copy-constructing
    void Init( const DataSeries & rOther );

    // ____ XDataSeries ____
    // _____________________
    /// @see ::com::sun::star::chart2::XDataSeries
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        SAL_CALL getDataPointByIndex( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetDataPoint( sal_Int32 nIndex )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetAllDataPoints()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSink ____
    // ___________________
    /// @see ::com::sun::star::chart2::data::XDataSink
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aData )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSource ____
    // _____________________
    /// @see ::com::sun::star::chart2::data::XDataSource
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    // ______________________
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    // ______________________
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    // ____ XRegressionCurveContainer ____
    // ___________________________________
    /// @see ::com::sun::star::chart2::XRegressionCurveContainer
    virtual void SAL_CALL addRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XRegressionCurve > > SAL_CALL getRegressionCurves()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRegressionCurves(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurve > >& aRegressionCurves )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;
    typedef ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer        m_aDataSequences;

    typedef ::std::map< sal_Int32,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > > tDataPointAttributeContainer;
    tDataPointAttributeContainer  m_aAttributedDataPoints;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > >
        tRegressionCurveContainerType;
    tRegressionCurveContainerType m_aRegressionCurves;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

}  // namespace chart

// _CHART_DATASERIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
