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
        css::chart2::XAxis,
        css::chart2::XTitled,
        css::lang::XServiceInfo,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    Axis_Base;
}

class Axis :
    public MutexContainer,
    public impl::Axis_Base,
    public ::property::OPropertySet
{
public:
    explicit Axis( css::uno::Reference< css::uno::XComponentContext > const & xContext );
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
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XAxis ____
    virtual void SAL_CALL setScaleData( const css::chart2::ScaleData& rScaleData )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::chart2::ScaleData SAL_CALL getScaleData()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getGridProperties()
                    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > > SAL_CALL getSubGridProperties()
                    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > > SAL_CALL getSubTickProperties()
                    throw (css::uno::RuntimeException, std::exception) override;

    // ____ XTitled ____
    virtual css::uno::Reference< css::chart2::XTitle > SAL_CALL getTitleObject()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitleObject(
        const css::uno::Reference< css::chart2::XTitle >& Title )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    // Note: the coordinate systems are not cloned!
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source )
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

private: //methods
    void AllocateSubGrids();

private: //member

    css::uno::Reference< css::util::XModifyListener >   m_xModifyEventForwarder;

    css::chart2::ScaleData             m_aScaleData;

    css::uno::Reference< css::beans::XPropertySet >     m_xGrid;

    css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > >     m_aSubGridProperties;

    css::uno::Reference< css::chart2::XTitle >          m_xTitle;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_MAIN_AXIS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
