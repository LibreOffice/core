/*************************************************************************
 *
 *  $RCSfile: confsvccomponent.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#define CONFIGMGR_API_SVCCOMPONENT_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/Mutex.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace configmgr {

    namespace css = ::com::sun::star;
    namespace uno = css::uno;
    namespace lang = css::lang;
    using ::rtl::OUString;

    typedef sal_Char const * AsciiServiceName;
    struct ServiceInfo
    {
        AsciiServiceName implementationName;
        AsciiServiceName const * serviceNames;
    };

    typedef ::cppu::WeakComponentImplHelper1< lang::XServiceInfo > ServiceImplBase;

    class ServiceComponentImpl
        : public ServiceImplBase
    {
    protected:
        ::osl::Mutex m_aMutex;
        ServiceInfo const* const m_info;
    public:
        ServiceComponentImpl(ServiceInfo const* aInfo);

        // XTypeProvider
        virtual uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(uno::RuntimeException);
        //virtual uno::Sequence<uno::Type> SAL_CALL getTypes(  ) throw(uno::RuntimeException) = 0;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(uno::RuntimeException);
        virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(uno::RuntimeException);

        // Component Helper - force override
        virtual void SAL_CALL disposing() = 0;
        // Component Helper - check object state
        virtual void checkAlive() throw (uno::RuntimeException);
        void checkAlive(char const* message) throw (uno::RuntimeException)
            { checkAlive( OUString::createFromAscii(message) ); }
        void checkAlive(OUString const& message) throw (uno::RuntimeException);

        // Extra helpers
        static sal_Int32 countServices(ServiceInfo const* aInfo);
        static uno::Sequence< OUString > getServiceNames(ServiceInfo const* aInfo) throw(uno::RuntimeException);
        static uno::Sequence<sal_Int8> getStaticImplementationId(ServiceInfo const* pServiceInfo) throw(uno::RuntimeException);

    private: // no implementation
        ServiceComponentImpl(ServiceComponentImpl&);
        void operator=(ServiceComponentImpl&);
    };

} // namespace configmgr

#endif // CONFIGMGR_API_SVCCOMPONENT_HXX_


