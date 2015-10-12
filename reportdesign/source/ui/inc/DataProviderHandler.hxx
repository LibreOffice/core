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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATAPROVIDERHANDLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATAPROVIDERHANDLER_HXX

#include <sal/config.h>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDatabaseDataProvider.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <memory>


namespace rptui
{


    class OPropertyInfoService;

    typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::inspection::XPropertyHandler
                                            ,   ::com::sun::star::lang::XServiceInfo> DataProviderHandler_Base;

    class DataProviderHandler:
        private ::cppu::BaseMutex,
        public DataProviderHandler_Base
    {
    public:
        // XServiceInfo - static versions
        static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

        explicit DataProviderHandler(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);
    private:

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::lang::XComponent:
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)   throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::inspection::XPropertyHandler:
        virtual void SAL_CALL inspect(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Component) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setPropertyValue(const OUString & PropertyName, const ::com::sun::star::uno::Any & Value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(const OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::LineDescriptor SAL_CALL describePropertyLine(const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& ControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL convertToPropertyValue(const OUString & PropertyName, const ::com::sun::star::uno::Any & ControlValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL convertToControlValue(const OUString & PropertyName, const ::com::sun::star::uno::Any & PropertyValue, const ::com::sun::star::uno::Type & ControlValueType) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addPropertyChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & Listener) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removePropertyChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & _rxListener) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupersededProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getActuatingProperties() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isComposable(const OUString & PropertyName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, ::com::sun::star::uno::Any & out_Data, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI > & InspectorUI) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL actuatingPropertyChanged(const OUString & ActuatingPropertyName, const ::com::sun::star::uno::Any & NewValue, const ::com::sun::star::uno::Any & OldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::NullPointerException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL suspend(sal_Bool Suspend) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual ~DataProviderHandler() {}
    private:
        DataProviderHandler(DataProviderHandler &) = delete;
        void operator =(DataProviderHandler &) = delete;


        bool impl_dialogLinkedFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const;
        bool impl_dialogChartType_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const;
        void impl_updateChartTitle_throw(const ::com::sun::star::uno::Any& _aValue);

        // override WeakComponentImplHelperBase::disposing()
        // This function is called upon disposing the component,
        // if your component needs special work when it becomes
        // disposed, do it here.
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >                m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >          m_xFormComponentHandler; /// delegatee
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDatabaseDataProvider>    m_xDataProvider; /// inspectee
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >                       m_xFormComponent; /// inspectee
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >              m_xReportComponent; /// inspectee
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument>                 m_xChartModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >        m_xMasterDetails;
        /// type converter, needed on various occasions
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >                m_xTypeConverter;
        ::std::unique_ptr< OPropertyInfoService >                                                   m_pInfoService;
    };

} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATAPROVIDERHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
