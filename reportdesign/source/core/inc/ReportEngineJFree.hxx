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
    typedef ::cppu::WeakComponentImplHelper< com::sun::star::report::XReportEngine
                                             ,com::sun::star::lang::XServiceInfo> ReportEngineBase;
    typedef ::cppu::PropertySetMixin<com::sun::star::report::XReportEngine> ReportEnginePropertySet;

    class OReportEngineJFree : public comphelper::OMutexAndBroadcastHelper,
                    public ReportEngineBase,
                    public ReportEnginePropertySet
    {
        typedef ::std::multimap< OUString, ::com::sun::star::uno::Any , ::comphelper::UStringMixLess>            TComponentMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReport;
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator>         m_StatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xActiveConnection;
        ::sal_Int32                                                                         m_nMaxRows;
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
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
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

        OReportEngineJFree(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
    private:
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // XReportEngine
            // Attributes
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > SAL_CALL getReportDefinition() throw (::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL setReportDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _reportdefinition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _activeconnection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL getStatusIndicator() throw (::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL setStatusIndicator( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& _statusindicator ) throw (::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual ::sal_Int32 SAL_CALL getMaxRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMaxRows( ::sal_Int32 _MaxRows ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            // Methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentModel(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentAlive( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _frame ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentAlive( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _frame ,bool _bHidden) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) ;
        virtual ::com::sun::star::util::URL SAL_CALL createDocument(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override ;
        virtual void SAL_CALL interrupt(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override ;

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTENGINEJFREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
