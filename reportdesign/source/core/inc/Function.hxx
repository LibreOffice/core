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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTION_HXX

#include <comphelper/uno3.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/report/XFunctions.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        css::report::XFunction  > FunctionPropertySet;
    typedef ::cppu::WeakComponentImplHelper<    css::report::XFunction
                                                ,css::lang::XServiceInfo > FunctionBase;

    /** \class OFunction Defines the implementation of a \interface com:::sun::star::report::XFunction
     * \ingroup reportdesign_api
     *
     */
    class OFunction :   public cppu::BaseMutex,
                            public FunctionBase,
                            public FunctionPropertySet
    {
        css::beans::Optional< OUString> m_sInitialFormula;
        css::uno::WeakReference< css::report::XFunctions >  m_xParent;
        OUString m_sName;
        OUString m_sFormula;
        bool     m_bPreEvaluated;
        bool     m_bDeepTraversing;
    private:
        OFunction(const OFunction&) = delete;
        OFunction& operator=(const OFunction&) = delete;

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(Value), &l);
                _member = Value;
            }
            l.notify();
        }
        void set(  const OUString& _sProperty
                  ,bool Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(Value), &l);
                _member = Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFunction() override;
    public:
        explicit OFunction(css::uno::Reference< css::uno::XComponentContext > const & _xContext);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // css::report::XFunction:
        virtual sal_Bool SAL_CALL getPreEvaluated() override;
        virtual void SAL_CALL setPreEvaluated(sal_Bool the_value) override;
        virtual sal_Bool SAL_CALL getDeepTraversing() override;
        virtual void SAL_CALL setDeepTraversing(sal_Bool the_value) override;
        virtual OUString SAL_CALL getName() override;
        virtual void SAL_CALL setName(const OUString & the_value) override;
        virtual OUString SAL_CALL getFormula() override;
        virtual void SAL_CALL setFormula(const OUString & the_value) override;
        virtual css::beans::Optional< OUString> SAL_CALL getInitialFormula() override;
        virtual void SAL_CALL setInitialFormula(const css::beans::Optional< OUString> & the_value) override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
