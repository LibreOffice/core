/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerwrapper.hxx,v $
 * $Revision: 1.4 $
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
#ifndef CONFIGMGR_API_PROVIDERWRAPPER_HXX_
#define CONFIGMGR_API_PROVIDERWRAPPER_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>

namespace configmgr
{
    //==========================================================================
    namespace uno = com::sun::star::uno;
    namespace lang = com::sun::star::lang;
    //==========================================================================
    //= ProviderWrapper
    //==========================================================================
    struct PWMutexHolder { osl::Mutex mutex; }; // ad hoc ...

    class ProviderWrapper : private PWMutexHolder, public cppu::WeakComponentImplHelper2< lang::XMultiServiceFactory, lang::XServiceInfo >
    {
    private:
        uno::Reference< lang::XMultiServiceFactory >    m_xDelegate;
        uno::Sequence< uno::Any >   m_aDefaults;
    private:
        ProviderWrapper(uno::Reference< lang::XMultiServiceFactory > const & xDelegate, uno::Sequence< com::sun::star::beans::NamedValue > const & aPresets);

    public:
        static uno::Reference< uno::XInterface > create( uno::Reference< uno::XInterface > xDelegate, uno::Sequence< com::sun::star::beans::NamedValue > const & aPresets);
        ~ProviderWrapper();

        /// XMultiServiceFactory
        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstance( const rtl::OUString& aServiceSpecifier )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getAvailableServiceNames(  )
                throw(uno::RuntimeException);

        /// XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw(uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsService( const ::rtl::OUString& ServiceName )
                throw(uno::RuntimeException);

        virtual uno::Sequence< rtl::OUString > SAL_CALL
            getSupportedServiceNames(  )
                throw(uno::RuntimeException);

    protected:
        virtual void SAL_CALL disposing();
    private:
        uno::Reference< lang::XMultiServiceFactory > getDelegate();
        uno::Reference<lang::XServiceInfo> getDelegateInfo();
        uno::Sequence< uno::Any > patchArguments(uno::Sequence< uno::Any > const & aArgs) const;
    };


} // namespace configmgr

#endif // CONFIGMGR_API_CONFPROVIDER2_HXX_


