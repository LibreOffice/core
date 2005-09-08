/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZDriverWrapper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:17:25 $
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

#ifndef _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_
#define _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    class OConnectionPool;
    //====================================================================
    //= ODriverWrapper
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::sdbc::XDriver
                                    >   ODriverWrapper_BASE;

    class ODriverWrapper : public ODriverWrapper_BASE
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                            m_xDriverAggregate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >
                            m_xDriver;
        OConnectionPool*    m_pConnectionPool;

    public:
        /** creates a new wrapper for a driver
            @param _rxAggregateDriver
                the driver to aggregate. The object will be reset to <NULL/> when returning from the ctor.
        */
        ODriverWrapper(
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >& _rxAggregateDriver,
            OConnectionPool* _pPool
            );



        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        /// dtor
        virtual ~ODriverWrapper();
        // XDriver
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_


