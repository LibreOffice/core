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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_INC_BASECOORDINATESYSTEM_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_INC_BASECOORDINATESYSTEM_HXX

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper
    < ::com::sun::star::lang::XServiceInfo,
      ::com::sun::star::chart2::XCoordinateSystem,
      ::com::sun::star::chart2::XChartTypeContainer,
      ::com::sun::star::util::XCloneable,
      ::com::sun::star::util::XModifyBroadcaster,
      ::com::sun::star::util::XModifyListener >
    BaseCoordinateSystem_Base;
}

class BaseCoordinateSystem :
        public impl::BaseCoordinateSystem_Base,
        public MutexContainer,
        public ::property::OPropertySet
{
public:
    BaseCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        sal_Int32 nDimensionCount = 2,
        bool bSwapXAndYAxis = false );
    explicit BaseCoordinateSystem( const BaseCoordinateSystem & rSource );
    virtual ~BaseCoordinateSystem();

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ XCoordinateSystem ____
    virtual ::sal_Int32 SAL_CALL getDimension()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAxisByDimension(
        ::sal_Int32 nDimension,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis,
        ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > SAL_CALL getAxisByDimension(
        ::sal_Int32 nDimension, ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getMaximumAxisIndexByDimension( ::sal_Int32 nDimension )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XChartTypeContainer ____
    virtual void SAL_CALL addChartType(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& aChartType )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChartType(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& aChartType )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > > SAL_CALL getChartTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setChartTypes(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > >& aChartTypes )
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

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

protected:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >        m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;

private:
    sal_Int32                                             m_nDimensionCount;
    typedef ::std::vector< ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XAxis > > > tAxisVecVecType;
    tAxisVecVecType m_aAllAxis; //outer sequence is the dimension; inner sequence is the axis index that indicates main or secondary axis
    ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any >                  m_aOrigin;
    ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType > >          m_aChartTypes;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_INC_BASECOORDINATESYSTEM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
