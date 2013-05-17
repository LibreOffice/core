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
#ifndef CHART_TITLE_HXX
#define CHART_TITLE_HXX

#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <cppuhelper/implbase5.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XTitle2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper5<
    ::com::sun::star::chart2::XTitle2,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::util::XCloneable,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::util::XModifyListener >
    Title_Base;
}

class Title :
    public MutexContainer,
    public impl::Title_Base,
    public ::property::OPropertySet
{
public:
    Title( ::com::sun::star::uno::Reference<
           ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Title();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Title )

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()


    // ambiguous overloads
    virtual void SAL_CALL setPropertyValue(const rtl::OUString& p1, const com::sun::star::uno::Any& p2) throw(css::uno::RuntimeException)
        { ::property::OPropertySet::setPropertyValue(p1, p2); }
    virtual com::sun::star::uno::Any SAL_CALL getPropertyValue(const rtl::OUString& p1) throw(css::uno::RuntimeException)
        { return ::property::OPropertySet::getPropertyValue(p1); }
    virtual void SAL_CALL addPropertyChangeListener(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw(css::uno::RuntimeException)
        { ::property::OPropertySet::addPropertyChangeListener(p1, p2); }
    virtual void SAL_CALL removePropertyChangeListener(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>& p2) throw(css::uno::RuntimeException)
        { ::property::OPropertySet::removePropertyChangeListener(p1, p2); }
    virtual void SAL_CALL addVetoableChangeListener(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw(css::uno::RuntimeException)
        { ::property::OPropertySet::addVetoableChangeListener(p1, p2); }
    virtual void SAL_CALL removeVetoableChangeListener(const rtl::OUString& p1, const com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener>& p2) throw(css::uno::RuntimeException)
        { ::property::OPropertySet::removeVetoableChangeListener(p1, p2); }

    // Attributes
    virtual double SAL_CALL getTextRotation() throw(css::uno::RuntimeException)
        { return getPropertyDouble("TextRotation"); }
    virtual void SAL_CALL setTextRotation(double p1) throw(css::uno::RuntimeException)
        { setPropertyDouble("TextRotation", p1); }
    virtual sal_Bool SAL_CALL getStackCharacters() throw(css::uno::RuntimeException)
        { return getPropertyBool("StackCharacters"); }
    virtual void SAL_CALL setStackCharacters(sal_Bool p1) throw(css::uno::RuntimeException)
        { setPropertyBool("StackCharacters", p1); }
    virtual com::sun::star::chart2::RelativePosition SAL_CALL getRelativePosition() throw(css::uno::RuntimeException);
    virtual void SAL_CALL setRelativePosition(const com::sun::star::chart2::RelativePosition& p1) throw(css::uno::RuntimeException)
        { setPropertyValue("RelativePosition", css::uno::Any(p1)); }
    virtual com::sun::star::awt::Size SAL_CALL getReferencePageSize() throw(css::uno::RuntimeException);
    virtual void SAL_CALL setReferencePageSize(const com::sun::star::awt::Size& p1) throw(css::uno::RuntimeException)
        { setPropertyValue("ReferencePageSize", css::uno::Any(p1)); }


protected:
    explicit Title( const Title & rOther );

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
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

    sal_Bool getPropertyBool(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    double getPropertyDouble(const OUString& p1) throw(::com::sun::star::uno::RuntimeException);
    void setPropertyBool(const OUString& p1, sal_Bool p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }
    void setPropertyDouble(const OUString& p1, double p2) throw(::com::sun::star::uno::RuntimeException)
        { setPropertyValue( p1, css::uno::Any(p2) ); }

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XFormattedString > > m_aStrings;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// CHART_TITLE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
