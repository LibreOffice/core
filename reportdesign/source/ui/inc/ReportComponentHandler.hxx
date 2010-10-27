/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef RPT_REPORTCOMPONENTHANDLER_HXX
#define RPT_REPORTCOMPONENTHANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/basemutex.hxx"
#include "com/sun/star/inspection/XPropertyHandler.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"

#include <memory>

//........................................................................
namespace rptui
{
//........................................................................

    class OPropertyInfoService;

    typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::inspection::XPropertyHandler
                                            ,   ::com::sun::star::lang::XServiceInfo> ReportComponentHandler_Base;

    class ReportComponentHandler:
        private ::cppu::BaseMutex,
        public ReportComponentHandler_Base
    {
    public:
        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    public:
        explicit ReportComponentHandler(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XComponent:
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)   throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::inspection::XPropertyHandler:
        virtual void SAL_CALL inspect(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Component) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const ::rtl::OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual void SAL_CALL setPropertyValue(const ::rtl::OUString & PropertyName, const ::com::sun::star::uno::Any & Value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(const ::rtl::OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual ::com::sun::star::inspection::LineDescriptor SAL_CALL describePropertyLine(const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& ControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToPropertyValue(const ::rtl::OUString & PropertyName, const ::com::sun::star::uno::Any & ControlValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToControlValue(const ::rtl::OUString & PropertyName, const ::com::sun::star::uno::Any & PropertyValue, const ::com::sun::star::uno::Type & ControlValueType) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual void SAL_CALL addPropertyChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & Listener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException);
        virtual void SAL_CALL removePropertyChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & _rxListener) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupersededProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getActuatingProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isComposable(const ::rtl::OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection(const ::rtl::OUString & PropertyName, ::sal_Bool Primary, ::com::sun::star::uno::Any & out_Data, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI > & InspectorUI) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException);
        virtual void SAL_CALL actuatingPropertyChanged(const ::rtl::OUString & ActuatingPropertyName, const ::com::sun::star::uno::Any & NewValue, const ::com::sun::star::uno::Any & OldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI > & InspectorUI, ::sal_Bool FirstTimeInit) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException);
        virtual ::sal_Bool SAL_CALL suspend(::sal_Bool Suspend) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~ReportComponentHandler() {}
    private:
        ReportComponentHandler(ReportComponentHandler &); // not defined
        void operator =(ReportComponentHandler &); // not defined



        // overload WeakComponentImplHelperBase::disposing()
        // This function is called upon disposing the component,
        // if your component needs special work when it becomes
        // disposed, do it here.
        virtual void SAL_CALL disposing();

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >  m_xFormComponentHandler; /// delegatee
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xReportComponent; /// inspectee
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xFormComponent; /// inspectee
        ::std::auto_ptr< OPropertyInfoService >                                             m_pInfoService;
    };
//........................................................................
} // namespace rptui
//........................................................................

#endif // RPT_REPORTCOMPONENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
