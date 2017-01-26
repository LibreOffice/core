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

    typedef ::cppu::WeakComponentImplHelper<   css::inspection::XPropertyHandler
                                            ,   css::lang::XServiceInfo> DataProviderHandler_Base;

    class DataProviderHandler:
        private ::cppu::BaseMutex,
        public DataProviderHandler_Base
    {
    public:
        // XServiceInfo - static versions
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static(  );
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_static(  );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
                        create(const css::uno::Reference< css::uno::XComponentContext >&);

        explicit DataProviderHandler(css::uno::Reference< css::uno::XComponentContext > const & context);
    private:

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // css::lang::XComponent:
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & xListener) override;
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;

        // css::inspection::XPropertyHandler:
        virtual void SAL_CALL inspect(const css::uno::Reference< css::uno::XInterface > & Component) override;
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) override;
        virtual void SAL_CALL setPropertyValue(const OUString & PropertyName, const css::uno::Any & Value) override;
        virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString & PropertyName) override;
        virtual css::inspection::LineDescriptor SAL_CALL describePropertyLine(const OUString& PropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& ControlFactory ) override;
        virtual css::uno::Any SAL_CALL convertToPropertyValue(const OUString & PropertyName, const css::uno::Any & ControlValue) override;
        virtual css::uno::Any SAL_CALL convertToControlValue(const OUString & PropertyName, const css::uno::Any & PropertyValue, const css::uno::Type & ControlValueType) override;
        virtual void SAL_CALL addPropertyChangeListener(const css::uno::Reference< css::beans::XPropertyChangeListener > & Listener) override;
        virtual void SAL_CALL removePropertyChangeListener(const css::uno::Reference< css::beans::XPropertyChangeListener > & _rxListener) override;
        virtual css::uno::Sequence< css::beans::Property > SAL_CALL getSupportedProperties() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupersededProperties() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getActuatingProperties() override;
        virtual sal_Bool SAL_CALL isComposable(const OUString & PropertyName) override;
        virtual css::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, css::uno::Any & out_Data, const css::uno::Reference< css::inspection::XObjectInspectorUI > & InspectorUI) override;
        virtual void SAL_CALL actuatingPropertyChanged(const OUString & ActuatingPropertyName, const css::uno::Any & NewValue, const css::uno::Any & OldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit) override;
        virtual sal_Bool SAL_CALL suspend(sal_Bool Suspend) override;

    protected:
        virtual ~DataProviderHandler() override {}
    private:
        DataProviderHandler(DataProviderHandler &) = delete;
        void operator =(DataProviderHandler &) = delete;


        bool impl_dialogLinkedFields_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const;
        bool impl_dialogChartType_nothrow( ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const;
        void impl_updateChartTitle_throw(const css::uno::Any& _aValue);

        // override WeakComponentImplHelperBase::disposing()
        // This function is called upon disposing the component,
        // if your component needs special work when it becomes
        // disposed, do it here.
        virtual void SAL_CALL disposing() override;

        css::uno::Reference< css::uno::XComponentContext >                m_xContext;
        css::uno::Reference< css::inspection::XPropertyHandler >          m_xFormComponentHandler; /// delegatee
        css::uno::Reference< css::chart2::data::XDatabaseDataProvider>    m_xDataProvider; /// inspectee
        css::uno::Reference< css::uno::XInterface >                       m_xFormComponent; /// inspectee
        css::uno::Reference< css::report::XReportComponent >              m_xReportComponent; /// inspectee
        css::uno::Reference< css::chart2::XChartDocument>                 m_xChartModel;
        css::uno::Reference< css::beans::XPropertyChangeListener >        m_xMasterDetails;
        /// type converter, needed on various occasions
        css::uno::Reference< css::script::XTypeConverter >                m_xTypeConverter;
    };

} // namespace rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATAPROVIDERHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
