/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confsvccomponent.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#define CONFIGMGR_API_SVCCOMPONENT_HXX_

#include "serviceinfohelper.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/typeprovider.hxx>

#ifndef _OSL_MUTEX_HXX_
#include <osl/Mutex.hxx>
#endif
#include <rtl/ustring.hxx>

namespace configmgr
{

//----------------------------------------------------------------------------
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace lang  = css::lang;

//----------------------------------------------------------------------------
    class ServiceComponentImpl
        : public ::cppu::WeakComponentImplHelper1< lang::XServiceInfo >
    {
    protected:
        ServiceImplementationInfo const*const     m_info;
    public:
        ServiceComponentImpl(ServiceImplementationInfo const* aInfo);

        // XTypeProvider
        virtual uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(uno::RuntimeException);
        //virtual uno::Sequence<uno::Type> SAL_CALL getTypes(  ) throw(uno::RuntimeException) = 0;

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName(  ) throw(uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException);
        virtual uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(uno::RuntimeException);

        // Component Helper - force override
        virtual void SAL_CALL disposing() = 0;
        // Component Helper - check object state
        virtual void checkAlive() throw (uno::RuntimeException);
        void checkAlive(char const* message) throw (uno::RuntimeException)
            { checkAlive( rtl::OUString::createFromAscii(message) ); }
        void checkAlive(rtl::OUString const& message) throw (uno::RuntimeException);

        // Extra helpers
        static uno::Sequence<sal_Int8> getStaticImplementationId(ServiceImplementationInfo const* pServiceInfo) throw(uno::RuntimeException);

    private: // no implementation
        ServiceComponentImpl(ServiceComponentImpl&);
        void operator=(ServiceComponentImpl&);
    };
//----------------------------------------------------------------------------

} // namespace configmgr

#endif // CONFIGMGR_API_SVCCOMPONENT_HXX_


