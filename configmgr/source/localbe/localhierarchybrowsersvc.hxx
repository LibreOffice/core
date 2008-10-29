/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localhierarchybrowsersvc.hxx,v $
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

#ifndef CONFIGMGR_LOCALBE_BROWSERSVC_HXX
#define CONFIGMGR_LOCALBE_BROWSERSVC_HXX

#include "serviceinfohelper.hxx"
#include <cppuhelper/implbase2.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localbe
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace task  = ::com::sun::star::task;
        namespace beans = ::com::sun::star::beans;
// -----------------------------------------------------------------------------

        class LocalHierarchyBrowserService : public ::cppu::WeakImplHelper2<
                                            task::XJob,
                                            lang::XServiceInfo
                                        >
        {
        public:
            explicit
            LocalHierarchyBrowserService(uno::Reference< uno::XComponentContext > const & _xContext);
            ~LocalHierarchyBrowserService();

            // XServiceInfo
            virtual rtl::OUString SAL_CALL
                getImplementationName(  )
                    throw (uno::RuntimeException);

            virtual sal_Bool SAL_CALL
                supportsService( const rtl::OUString& ServiceName )
                    throw (uno::RuntimeException);

            virtual uno::Sequence< rtl::OUString > SAL_CALL
                getSupportedServiceNames(  )
                    throw (uno::RuntimeException);

            // XJob
            virtual uno::Any SAL_CALL
                execute( const uno::Sequence< beans::NamedValue >& Arguments )
                    throw (lang::IllegalArgumentException, uno::Exception, uno::RuntimeException);

        private:
            uno::Reference< lang::XMultiServiceFactory > getServiceFactory() const
            { return m_xServiceFactory; }

            uno::Sequence< rtl::OUString > findLocalComponentNames( rtl::OUString const & _aBaseDirectory, rtl::OUString const & _aComponentFileExtension, uno::Sequence< rtl::OUString > const & aExcludeList);
            uno::Sequence< rtl::OUString > findLocalComponentUrls( rtl::OUString const & _aBaseDirectory, rtl::OUString const & _aComponentFileExtension, uno::Sequence< rtl::OUString > const & aExcludeList);
        private:
            uno::Reference< lang::XMultiServiceFactory >  m_xServiceFactory;

            static ServiceInfoHelper getServiceInfo();
        };
// -----------------------------------------------------------------------------
    } // namespace localbe
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




