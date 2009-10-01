/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wininetbackend.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _FIXEDVALUEBACKEND_HXX_
#define _FIXEDVALUEBACKEND_HXX_

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/string.hxx>

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;

class WinInetBackend : public ::cppu::WeakImplHelper2 <
        css::container::XNameAccess,
        lang::XServiceInfo > {

    public :

        static WinInetBackend* createInstance();

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw (uno::RuntimeException) ;

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& aServiceName )
                throw (uno::RuntimeException) ;

        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(  )
                throw (uno::RuntimeException) ;

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getBackendName(void) ;
        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void) ;

        //XNameAccess
        virtual uno::Type SAL_CALL
        getElementType()
            throw (uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        hasElements()
            throw (uno::RuntimeException);

        virtual uno::Any SAL_CALL
        getByName( const rtl::OUString& aName )
            throw (css::container::NoSuchElementException,
                   lang::WrappedTargetException, uno::RuntimeException);

        virtual uno::Sequence<rtl::OUString> SAL_CALL
        getElementNames()
            throw (uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        hasByName( const rtl::OUString& aName )
            throw (uno::RuntimeException);

    protected:
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        WinInetBackend();

        /** Destructor */
        ~WinInetBackend(void) ;

    private:
        sal_Int32 valueProxyType_;
        rtl::OUString valueNoProxy_;
        rtl::OUString valueHttpProxyName_;
        sal_Int32 valueHttpProxyPort_;
        rtl::OUString valueHttpsProxyName_;
        sal_Int32 valueHttpsProxyPort_;
        rtl::OUString valueFtpProxyName_;
        sal_Int32 valueFtpProxyPort_;
        bool hasProxyType_;
        bool hasNoProxy_;
        bool hasHttpProxyName_;
        bool hasHttpProxyPort_;
        bool hasHttpsProxyName_;
        bool hasHttpsProxyPort_;
        bool hasFtpProxyName_;
        bool hasFtpProxyPort_;
} ;


#endif // _FIXEDVALUEBACKEND_HXX_
