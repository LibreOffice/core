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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_CHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_CHARTTYPE_HXX

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include "ModifyListenerHelper.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        ::com::sun::star::chart2::XChartType,
        ::com::sun::star::chart2::XDataSeriesContainer,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    ChartType_Base;
}

class ChartType :
    public MutexContainer,
    public impl::ChartType_Base,
    public ::property::OPropertySet
{
public:
    explicit ChartType(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ChartType();

    /// merge XInterface implementations
     DECLARE_XINTERFACE()

protected:
    explicit ChartType( const ChartType & rOther );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        GetComponentContext() const { return m_xContext;}

    // ____ XChartType ____
    // still abstract ! implement !
    virtual OUString SAL_CALL getChartType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override = 0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem > SAL_CALL
        createCoordinateSystem( ::sal_Int32 DimensionCount )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedMandatoryRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedOptionalRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getRoleOfSequenceForSeriesLabel()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedPropertyRoles()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XDataSeriesContainer ____
    virtual void SAL_CALL addDataSeries(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& aDataSeries )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDataSeries(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& aDataSeries )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > > SAL_CALL getDataSeries()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDataSeries(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >& aDataSeries )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    void fireModifyEvent();

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) override;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >
        const m_xModifyEventForwarder;

private:
    void impl_addDataSeriesWithoutNotification(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >& aDataSeries );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        const m_xContext;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > >
        tDataSeriesContainerType;

    // --- mutable members: the following members need mutex guard ---

    tDataSeriesContainerType  m_aDataSeries;

    bool  m_bNotifyChanges;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_CHARTTYPE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
