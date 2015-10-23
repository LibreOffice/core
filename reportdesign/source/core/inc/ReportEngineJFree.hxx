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

        /** returns the file url for a new model
        *
        * \return The new file url.
        */
        OUString getNewOutputName();

    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!

        virtual ~OReportEngineJFree();
    public:
        typedef rtl::Reference<OReportEngineJFree> TReportEngine;

        OReportEngineJFree(const css::uno::Reference< css::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);
    private:
        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XReportEngine
            // Attributes
        virtual css::uno::Reference< css::report::XReportDefinition > SAL_CALL getReportDefinition() throw (css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL setReportDefinition( const css::uno::Reference< css::report::XReportDefinition >& _reportdefinition ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveConnection( const css::uno::Reference< css::sdbc::XConnection >& _activeconnection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL getStatusIndicator() throw (css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL setStatusIndicator( const css::uno::Reference< css::task::XStatusIndicator >& _statusindicator ) throw (css::uno::RuntimeException, std::exception) override ;
        virtual ::sal_Int32 SAL_CALL getMaxRows() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMaxRows( ::sal_Int32 _MaxRows ) throw (css::uno::RuntimeException, std::exception) override;
            // Methods
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL createDocumentModel(  ) throw (css::lang::DisposedException, css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException, std::exception) override ;
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL createDocumentAlive( const css::uno::Reference< css::frame::XFrame >& _frame ) throw (css::lang::DisposedException, css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException, std::exception) override ;
        css::uno::Reference< css::frame::XModel > SAL_CALL createDocumentAlive( const css::uno::Reference< css::frame::XFrame >& _frame ,bool _bHidden) throw (css::lang::DisposedException, css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException, std::exception) ;
        virtual css::util::URL SAL_CALL createDocument(  ) throw (css::lang::DisposedException, css::lang::IllegalArgumentException, css::uno::Exception, css::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL interrupt(  ) throw (css::lang::DisposedException, css::uno::Exception, css::uno::RuntimeException, std::exception) override ;

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
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTENGINEJFREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
