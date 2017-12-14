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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTENGINEJFREE_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTENGINEJFREE_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/report/XReportEngine.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ref.hxx>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper< css::report::XReportEngine
                                             ,css::lang::XServiceInfo> ReportEngineBase;
    typedef ::cppu::PropertySetMixin<css::report::XReportEngine> ReportEnginePropertySet;

    class OReportEngineJFree : public comphelper::OMutexAndBroadcastHelper,
                    public ReportEngineBase,
                    public ReportEnginePropertySet
    {
        typedef ::std::multimap< OUString, css::uno::Any , ::comphelper::UStringMixLess>            TComponentMap;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        css::uno::Reference< css::report::XReportDefinition >     m_xReport;
        css::uno::Reference< css::task::XStatusIndicator>         m_StatusIndicator;
        css::uno::Reference< css::sdbc::XConnection >             m_xActiveConnection;
        ::sal_Int32                                               m_nMaxRows;
    private:
        OReportEngineJFree(const OReportEngineJFree&) = delete;
        OReportEngineJFree& operator=(const OReportEngineJFree&) = delete;
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

        /** returns the file url for a new model
        *
        * \return The new file url.
        */
        OUString getNewOutputName();

    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OReportEngineJFree() override;
    public:
        typedef rtl::Reference<OReportEngineJFree> TReportEngine;

        OReportEngineJFree(const css::uno::Reference< css::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static();
        static css::uno::Reference< css::uno::XInterface >
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
    private:
        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XReportEngine
            // Attributes
        virtual css::uno::Reference< css::report::XReportDefinition > SAL_CALL getReportDefinition() override ;
        virtual void SAL_CALL setReportDefinition( const css::uno::Reference< css::report::XReportDefinition >& _reportdefinition ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() override;
        virtual void SAL_CALL setActiveConnection( const css::uno::Reference< css::sdbc::XConnection >& _activeconnection ) override;
        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL getStatusIndicator() override ;
        virtual void SAL_CALL setStatusIndicator( const css::uno::Reference< css::task::XStatusIndicator >& _statusindicator ) override ;
        virtual ::sal_Int32 SAL_CALL getMaxRows() override;
        virtual void SAL_CALL setMaxRows( ::sal_Int32 MaxRows ) override;
            // Methods
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL createDocumentModel(  ) override ;
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL createDocumentAlive( const css::uno::Reference< css::frame::XFrame >& _frame ) override ;
        /// @throws css::lang::DisposedException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::uno::Exception
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::frame::XModel > createDocumentAlive( const css::uno::Reference< css::frame::XFrame >& _frame ,bool _bHidden) ;
        virtual css::util::URL SAL_CALL createDocument(  ) override ;
        virtual void SAL_CALL interrupt(  ) override ;

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
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTENGINEJFREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
