/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confprovider2.hxx,v $
 * $Revision: 1.9 $
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
#ifndef CONFIGMGR_API_CONFPROVIDER2_HXX_
#define CONFIGMGR_API_CONFPROVIDER2_HXX_

#include "provider.hxx"
#include <comphelper/proparrhlp.hxx>


namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;
    using ::rtl::OUString;
    using ::vos::ORef;

    class OConfigurationProviderImpl;

    //==========================================================================
    //= OConfigurationProvider
    //==========================================================================
    /** Factory for receiving access for user configuration data. */
    class OConfigurationProvider : public OProvider
                                 , public  ::comphelper::OPropertyArrayUsageHelper<OConfigurationProvider>
    {
        OConfigurationProviderImpl*         m_pImpl;
        uno::Sequence< ::rtl::OUString >    m_aPrefetchNodes;
        sal_Bool                            m_bEnableAsync;

    public:
        OConfigurationProvider(CreationContext const & xContext, ServiceImplementationInfo const* pServices);
        void connect() throw (uno::Exception);

    public:
        ~OConfigurationProvider();

        /// XMultiServiceFactory
        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstance( const OUString& aServiceSpecifier )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Sequence< OUString > SAL_CALL
            getAvailableServiceNames(  )
                throw(uno::RuntimeException);

        /// XLocalizable
        virtual void SAL_CALL
            setLocale( const lang::Locale& eLocale )
                throw (uno::RuntimeException);

        virtual lang::Locale SAL_CALL
            getLocale(  )
                throw (uno::RuntimeException);


        //XRefreshable
         virtual void SAL_CALL
            refresh(  )
                throw (uno::RuntimeException);
        virtual void SAL_CALL
            addRefreshListener(
                const uno::Reference< util::XRefreshListener >& aListener )
                    throw (uno::RuntimeException);
        virtual void SAL_CALL
            removeRefreshListener(
                const uno::Reference< util::XRefreshListener >& aListener )
                    throw (uno::RuntimeException);
        //XFlushable
         virtual void SAL_CALL
            flush(  )
                throw (uno::RuntimeException);
        virtual void SAL_CALL
            addFlushListener(
                const uno::Reference< util::XFlushListener >& aListener )
                    throw (uno::RuntimeException);
        virtual void SAL_CALL
            removeFlushListener(
                const uno::Reference< util::XFlushListener >& aListener )
                    throw (uno::RuntimeException);

        // OPropertSetHelper
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception);

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType ) throw(uno::RuntimeException);
        //XTypeProvider
        virtual uno::Sequence< uno::Type > SAL_CALL getTypes(  ) throw(uno::RuntimeException);

    protected:
              using OProvider::disposing;
        virtual void SAL_CALL disposing();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
    };


} // namespace configmgr

#endif // CONFIGMGR_API_CONFPROVIDER2_HXX_


