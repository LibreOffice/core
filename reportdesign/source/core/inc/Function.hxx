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
#pragma once
#if 1

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFunction.hpp>
#include <cppuhelper/basemutex.hxx>
#include "ReportControlModel.hxx"
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        com::sun::star::report::XFunction  > FunctionPropertySet;
    typedef ::cppu::WeakComponentImplHelper2<    com::sun::star::report::XFunction
                                                ,com::sun::star::lang::XServiceInfo > FunctionBase;

    /** \class OFunction Defines the implementation of a \interface com:::sun::star::report::XFunction
     * \ingroup reportdesign_api
     *
     */
    class OFunction :   public cppu::BaseMutex,
                            public FunctionBase,
                            public FunctionPropertySet
    {
        com::sun::star::beans::Optional< ::rtl::OUString> m_sInitialFormula;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XFunctions >  m_xParent;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sFormula;
        ::sal_Bool      m_bPreEvaluated;
        ::sal_Bool      m_bDeepTraversing;
    private:
        OFunction(const OFunction&);
        OFunction& operator=(const OFunction&);

        template <typename T> void set(  const ::rtl::OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFunction();
    public:
        explicit OFunction(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::report::XFunction:
        virtual ::sal_Bool SAL_CALL getPreEvaluated() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPreEvaluated(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getDeepTraversing() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDeepTraversing(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFormula() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFormula(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual com::sun::star::beans::Optional< ::rtl::OUString> SAL_CALL getInitialFormula() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInitialFormula(const com::sun::star::beans::Optional< ::rtl::OUString> & the_value) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif //RPT_FUNCTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
