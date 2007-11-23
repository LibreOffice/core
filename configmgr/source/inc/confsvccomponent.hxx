/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confsvccomponent.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:17:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#define CONFIGMGR_API_SVCCOMPONENT_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

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

namespace configmgr
{

//----------------------------------------------------------------------------
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace lang  = css::lang;
    using ::rtl::OUString;

//----------------------------------------------------------------------------
    typedef ::cppu::WeakComponentImplHelper1< lang::XServiceInfo > ServiceImplBase;

//----------------------------------------------------------------------------
    class ServiceComponentImpl
        : public ServiceImplBase
    {
    protected:
        ServiceImplementationInfo const*const     m_info;
    public:
        ServiceComponentImpl(ServiceImplementationInfo const* aInfo);

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
        static uno::Sequence<sal_Int8> getStaticImplementationId(ServiceImplementationInfo const* pServiceInfo) throw(uno::RuntimeException);

    private: // no implementation
        ServiceComponentImpl(ServiceComponentImpl&);
        void operator=(ServiceComponentImpl&);
    };
//----------------------------------------------------------------------------

} // namespace configmgr

#endif // CONFIGMGR_API_SVCCOMPONENT_HXX_


