/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
        const OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL acceptsURL( const OUString& url )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo(
        const OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public: // XServiceInfo
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public: // XDataDefinitionSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByConnection(
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& connection )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByURL(
        const OUString& url,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

};


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
