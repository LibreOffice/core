/*************************************************************************
 *
 *  $RCSfile: pq_driver.hxx,v $
 *
 *  $Revision: 1.1.2.2 $
 *
 *  last change: $Author: jbu $ $Date: 2004/05/09 19:47:14 $
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
 *  The Initial Developer of the Original Code is: Joerg Budischewski
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Joerg Budischewski
 *
 *
 ************************************************************************/
#ifndef _PG_DRIVER_HXX_
#define _PG_DRIVER_HXX_

#include <osl/mutex.hxx>

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>

namespace pq_sdbc_driver
{

#define MY_STRINGIFY( x ) #x

#define PQ_SDBC_DRIVER_VERSION MY_STRINGIFY(PQ_SDBC_MAJOR) "."  \
                               MY_STRINGIFY(PQ_SDBC_MINOR) "." \
                               MY_STRINGIFY(PQ_SDBC_MICRO)

#define POSTGRES_MAJOR 7
#define POSTGRES_MINOR 3
#define POSTGRES_MICRO 2
#define POSTGRESQL_VERSION MY_STRINGIFY(POSTGRESQL_MAJOR) "." \
                           MY_STRINGIFY(POSTGRESQL_MINOR) "." \
                           MY_STRINGIFY(POSTGRESQL_MICRO)


struct MutexHolder { osl::Mutex m_mutex; };
// use this to switch off sdbc support !
// typedef cppu::WeakComponentImplHelper2<
//     com::sun::star::sdbc::XDriver,
//     com::sun::star::lang::XServiceInfo
//     > DriverBase ;
typedef cppu::WeakComponentImplHelper3<
    com::sun::star::sdbc::XDriver,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::sdbcx::XDataDefinitionSupplier > DriverBase ;
class Driver : public MutexHolder, public DriverBase
{
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_ctx;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiComponentFactory > m_smgr;

public:
    Driver ( const com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > & ctx )
        : DriverBase( this->m_mutex ),
          m_ctx( ctx ),
          m_smgr( ctx->getServiceManager() )
    {}

public: // XDriver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect(
        const ::rtl::OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo(
        const ::rtl::OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException);

public: // XServiceInfo
    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);

public: // XDataDefinitionSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByConnection(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByURL(
        const ::rtl::OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL disposing();

};


}

#endif
