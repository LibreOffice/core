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
#ifndef CHART2_REGRESSIONEQUATION_HXX
#define CHART2_REGRESSIONEQUATION_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/chart2/XTitle.hpp>

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"

#include <cppuhelper/implbase4.hxx>
#include <comphelper/uno3.hxx>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper4<
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener,
        ::com::sun::star::chart2::XTitle >
    RegressionEquation_Base;
}

class RegressionEquation :
        public MutexContainer,
        public impl::RegressionEquation_Base,
        public ::property::OPropertySet
{
public:
    explicit RegressionEquation(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~RegressionEquation();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( RegressionEquation )

protected:
    explicit RegressionEquation( const RegressionEquation & rOther );

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
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

    // ____ XTitle ____
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XFormattedString > > SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Reference<
                                   ::com::sun::star::chart2::XFormattedString > >& Strings )
        throw (::com::sun::star::uno::RuntimeException);

    using ::cppu::OPropertySetHelper::disposing;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();

    void fireModifyEvent();

private:
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XFormattedString > > m_aStrings;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;
};

} //  namespace chart

// CHART2_REGRESSIONEQUATION_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
