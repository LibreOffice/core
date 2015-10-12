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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_MAIN_AXIS_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_MAIN_AXIS_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>

#include "ModifyListenerHelper.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
        ::com::sun::star::chart2::XAxis,
        ::com::sun::star::chart2::XTitled,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    Axis_Base;
}

class Axis :
    public MutexContainer,
    public impl::Axis_Base,
    public ::property::OPropertySet
{
public:
    explicit Axis( ::com::sun::star::uno::Reference<
          ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Axis();

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    explicit Axis( const Axis & rOther );

    // late initialization to call after copy-constructing
    void Init( const Axis & rOther );

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XAxis ____
    virtual void SAL_CALL setScaleData( const ::com::sun::star::chart2::ScaleData& rScaleData )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::chart2::ScaleData SAL_CALL getScaleData()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > SAL_CALL getGridProperties()
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > > SAL_CALL getSubGridProperties()
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > > SAL_CALL getSubTickProperties()
                    throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitleObject(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    // Note: the coordinate systems are not cloned!
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

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

private: //methods
    void AllocateSubGrids();

private: //member

    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   m_xModifyEventForwarder;

    ::com::sun::star::chart2::ScaleData             m_aScaleData;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >     m_xGrid;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > >     m_aSubGridProperties;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >          m_xTitle;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_MAIN_AXIS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
