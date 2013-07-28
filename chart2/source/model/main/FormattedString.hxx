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
#ifndef CHART_FORMATTEDSTRING_HXX
#define CHART_FORMATTEDSTRING_HXX

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include <cppuhelper/implbase5.hxx>
#include <comphelper/uno3.hxx>
#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XFormattedString2.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper5<
    ::css::chart2::XFormattedString2,
    ::css::lang::XServiceInfo,
    ::css::util::XCloneable,
    ::css::util::XModifyBroadcaster,
    ::css::util::XModifyListener >
    FormattedString_Base;
}

class FormattedString :
    public MutexContainer,
    public impl::FormattedString_Base,
    public ::property::OPropertySet
{
public:
    FormattedString( ::css::uno::Reference<
           ::css::uno::XComponentContext > const & xContext );
    virtual ~FormattedString();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( FormattedString )

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { ::property::OPropertySet::setPropertyValue(p1, p2); }
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p1) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { return ::property::OPropertySet::getPropertyValue(p1); }
    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { ::property::OPropertySet::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { ::property::OPropertySet::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { ::property::OPropertySet::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException)
        { ::property::OPropertySet::removeVetoableChangeListener(p1, p2); }

protected:
    explicit FormattedString( const FormattedString & rOther );

    // ____ XFormattedString ____
    virtual OUString SAL_CALL getString()
        throw (::css::uno::RuntimeException);
    virtual void SAL_CALL setString( const OUString& String )
        throw (::css::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual ::css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::css::beans::UnknownPropertyException);

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::css::uno::Reference< ::css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::css::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::css::uno::Reference< ::css::util::XCloneable > SAL_CALL createClone()
        throw (::css::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::css::uno::Reference< ::css::util::XModifyListener >& aListener )
        throw (::css::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::css::uno::Reference< ::css::util::XModifyListener >& aListener )
        throw (::css::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::css::lang::EventObject& aEvent )
        throw (::css::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::css::lang::EventObject& Source )
        throw (::css::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
    OUString m_aString;

    ::css::uno::Reference< ::css::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// CHART_FORMATTEDSTRING_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
