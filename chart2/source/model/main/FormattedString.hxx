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
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper5<
    ::com::sun::star::chart2::XFormattedString,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::util::XCloneable,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::util::XModifyListener >
    FormattedString_Base;
}

class FormattedString :
    public MutexContainer,
    public impl::FormattedString_Base,
    public ::property::OPropertySet
{
public:
    FormattedString( ::com::sun::star::uno::Reference<
           ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~FormattedString();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( FormattedString )

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    explicit FormattedString( const FormattedString & rOther );

    // ____ XFormattedString ____
    virtual OUString SAL_CALL getString()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const OUString& String )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    // ____ OPropertySet ____
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

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();
    using OPropertySet::disposing;

    void fireModifyEvent();

private:
    OUString m_aString;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// CHART_FORMATTEDSTRING_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
