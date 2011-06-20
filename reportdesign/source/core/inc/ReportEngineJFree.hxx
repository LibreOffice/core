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
#ifndef REPORTDESIGN_API_REPORTENGINEJFREE_HXX
#define REPORTDESIGN_API_REPORTENGINEJFREE_HXX

#include <com/sun/star/report/XReportEngine.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <comphelper/stl_types.hxx>
#include <comphelper/implementationreference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace reportdesign
{
    typedef ::cppu::WeakComponentImplHelper2< com::sun::star::report::XReportEngine
                                             ,com::sun::star::lang::XServiceInfo> ReportEngineBase;
    typedef ::cppu::PropertySetMixin<com::sun::star::report::XReportEngine> ReportEnginePropertySet;

    class OReportEngineJFree : public comphelper::OMutexAndBroadcastHelper,
                    public ReportEngineBase,
                    public ReportEnginePropertySet
    {
        typedef ::std::multimap< ::rtl::OUString, ::com::sun::star::uno::Any , ::comphelper::UStringMixLess>            TComponentMap;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReport;
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator>         m_StatusIndicator;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >             m_xActiveConnection;
        ::sal_Int32                                                                         m_nMaxRows;
    private:
        OReportEngineJFree(const OReportEngineJFree&);
        OReportEngineJFree& operator=(const OReportEngineJFree&);
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

        /** returns the file url for a new model
        *
        * \return The new file url.
        */
        ::rtl::OUString getNewOutputName();

    protected:
        // TODO: VirtualFunctionFinder: This is virtual function!
        //
        virtual ~OReportEngineJFree();
    public:
        typedef ::comphelper::ImplementationReference< OReportEngineJFree   ,::com::sun::star::report::XReportEngine,::com::sun::star::uno::XWeak > TReportEngine;

        OReportEngineJFree(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& context);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
    private:
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XReportEngine
            // Attributes
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > SAL_CALL getReportDefinition() throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL setReportDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _reportdefinition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _activeconnection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL getStatusIndicator() throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL setStatusIndicator( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >& _statusindicator ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::sal_Int32 SAL_CALL getMaxRows() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMaxRows( ::sal_Int32 _MaxRows ) throw (::com::sun::star::uno::RuntimeException);
            // Methods
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentModel(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentAlive( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _frame ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL createDocumentAlive( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _frame ,bool _bHidden) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::util::URL SAL_CALL createDocument(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL interrupt(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException) ;

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
    };
}
#endif //REPORTDESIGN_API_REPORTENGINEJFREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
