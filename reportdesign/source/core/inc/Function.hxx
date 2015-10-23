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

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFunction.hpp>
#include <cppuhelper/basemutex.hxx>
#include "ReportControlModel.hxx"
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
        css::uno::Reference< css::uno::XComponentContext >  m_xContext;
        css::uno::WeakReference< css::report::XFunctions >  m_xParent;
        OUString m_sName;
        OUString m_sFormula;
        bool     m_bPreEvaluated;
        bool     m_bDeepTraversing;
    private:
        OFunction(const OFunction&) = delete;
        OFunction& operator=(const OFunction&) = delete;

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
        void set(  const OUString& _sProperty
                  ,bool _Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFunction();
    public:
        explicit OFunction(css::uno::Reference< css::uno::XComponentContext > const & _xContext);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // css::report::XFunction:
        virtual sal_Bool SAL_CALL getPreEvaluated() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPreEvaluated(sal_Bool the_value) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getDeepTraversing() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDeepTraversing(sal_Bool the_value) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setName(const OUString & the_value) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getFormula() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFormula(const OUString & the_value) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::beans::Optional< OUString> SAL_CALL getInitialFormula() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInitialFormula(const css::beans::Optional< OUString> & the_value) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FUNCTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
