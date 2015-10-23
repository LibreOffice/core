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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATCONDITION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATCONDITION_HXX

#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/report/XFormatCondition.hpp>
#include "ReportControlModel.hxx"
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ReportHelperDefines.hxx"

namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<        css::report::XFormatCondition   > FormatConditionPropertySet;
    typedef ::cppu::WeakComponentImplHelper<    css::report::XFormatCondition
                                                ,css::lang::XServiceInfo > FormatConditionBase;

    /** \class OFormatCondition Defines the implementation of a \interface com:::sun::star::report::XFormatCondition
     * \ingroup reportdesign_api
     *
     */
    class OFormatCondition :    public comphelper::OBaseMutex,
                            public FormatConditionBase,
                            public FormatConditionPropertySet
    {
        OFormatProperties   m_aFormatProperties;
        OUString            m_sFormula;
        bool                m_bEnabled;
    private:
        OFormatCondition(const OFormatCondition&) = delete;
        OFormatCondition& operator=(const OFormatCondition&) = delete;

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
        virtual ~OFormatCondition();
    public:
        explicit OFormatCondition(css::uno::Reference< css::uno::XComponentContext > const & _xContext
            );

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

        // XFormatCondition
        virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getFormula() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFormula( const OUString& _formula ) throw (css::uno::RuntimeException, std::exception) override;

        // XReportControlFormat
        REPORTCONTROLFORMAT_HEADER()
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
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_FORMATCONDITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
