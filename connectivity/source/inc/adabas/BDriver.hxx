/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BDriver.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:59:20 $
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

#ifndef _CONNECTIVITY_ADABAS_BDRIVER_HXX_
#define _CONNECTIVITY_ADABAS_BDRIVER_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#include "odbc/OFunctiondefs.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ODRIVER_HXX_
#include "odbc/ODriver.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef odbc::ODBCDriver ODriver_BASE;

        class ODriver : public ODriver_BASE,
                        public ::com::sun::star::sdbcx::XDataDefinitionSupplier
        {
        protected:
            virtual SQLHANDLE EnvironmentHandle(::rtl::OUString &_rPath);
            virtual ~ODriver();
        public:
            DECLARE_SERVICE_INFO();

            ODriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            virtual oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const;
            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XDriver
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDataDefinitionSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL getDataDefinitionByURL( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };
    }

}
#endif //_CONNECTIVITY_ADABAS_BDRIVER_HXX_


