/*************************************************************************
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

#ifndef SD_WORKBENCH_CTP_FACTORY_HXX
#define SD_WORKBENCH_CTP_FACTORY_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/basemutex.hxx>

//......................................................................................................................
namespace sd { namespace colortoolpanel
{
//......................................................................................................................

    class FactoryGuard;

    //==================================================================================================================
    //= ResourceFactory
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::drawing::framework::XResourceFactory
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::lang::XInitialization
                                    >   ResourceFactory_Base;
    class ResourceFactory   :public ::cppu::BaseMutex
                            ,public ResourceFactory_Base
    {
    public:
        ResourceFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext
        );
        ~ResourceFactory();

        // XResourceFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResource > SAL_CALL createResource( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& xResourceId ) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException);
        virtual void SAL_CALL releaseResource( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResource >& xResource ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static versions
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rContext ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    public:
        struct GuardAccess { friend class FactoryGuard; private: GuardAccess() { } };

        void            checkInitialized( GuardAccess ) const;
        void            checkDisposed( GuardAccess ) const;
        ::osl::Mutex&   getMutex( GuardAccess ) { return m_aMutex; }

    private:
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XControllerManager >
                m_xControllerManager;
    };

    class FactoryGuard
    {
    public:
        FactoryGuard( ResourceFactory& i_rFactory, const bool i_bNeedInit = true )
            :m_aGuard( i_rFactory.getMutex( ResourceFactory::GuardAccess() ) )
        {
            i_rFactory.checkDisposed( ResourceFactory::GuardAccess() );
            if ( i_bNeedInit )
                i_rFactory.checkInitialized( ResourceFactory::GuardAccess() );
        }

    protected:
        ::osl::MutexGuard   m_aGuard;
    };

//......................................................................................................................
} } // namespace sd::colortoolpanel
//......................................................................................................................

#endif // SD_WORKBENCH_CTP_FACTORY_HXX
