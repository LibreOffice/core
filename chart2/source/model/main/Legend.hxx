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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_MAIN_LEGEND_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_MAIN_LEGEND_HXX

#include "ModifyListenerHelper.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::chart2::XLegend,
        css::lang::XServiceInfo,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener >
    Legend_Base;
}

class Legend :
    public MutexContainer,
    public impl::Legend_Base,
    public ::property::OPropertySet
{
public:
    explicit Legend( css::uno::Reference< css::uno::XComponentContext > const & xContext );
    virtual ~Legend();

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
    explicit Legend( const Legend & rOther );

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw (css::beans::UnknownPropertyException, css::uno::RuntimeException) override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
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

private:
    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_MAIN_LEGEND_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
