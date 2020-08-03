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

#pragma once

#include <osl/mutex.hxx>
#include <sal/macros.h>
#include <cppuhelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>

namespace pq_sdbc_driver
{

#define PQ_SDBC_DRIVER_VERSION SAL_STRINGIFY(PQ_SDBC_MAJOR) "."  \
                               SAL_STRINGIFY(PQ_SDBC_MINOR) "." \
                               SAL_STRINGIFY(PQ_SDBC_MICRO)

struct MutexHolder { osl::Mutex m_mutex; };
// use this to switch off sdbc support !
// typedef cppu::WeakComponentImplHelper<
//     css::sdbc::XDriver,
//     css::lang::XServiceInfo
//     > DriverBase ;
typedef cppu::WeakComponentImplHelper<
    css::sdbc::XDriver,
    css::lang::XServiceInfo,
    css::sdbcx::XDataDefinitionSupplier > DriverBase ;
class Driver : public MutexHolder, public DriverBase
{
    css::uno::Reference< css::uno::XComponentContext > m_ctx;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_smgr;

public:
    explicit Driver ( const css::uno::Reference < css::uno::XComponentContext > & ctx )
        : DriverBase( m_mutex ),
          m_ctx( ctx ),
          m_smgr( ctx->getServiceManager() )
    {}

public: // XDriver
    virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect(
        const OUString& url,
        const css::uno::Sequence< css::beans::PropertyValue >& info ) override;

    virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override;

    virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo(
        const OUString& url,
        const css::uno::Sequence< css::beans::PropertyValue >& info ) override;

    virtual sal_Int32 SAL_CALL getMajorVersion(  ) override;
    virtual sal_Int32 SAL_CALL getMinorVersion(  ) override;

public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XDataDefinitionSupplier
    virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByConnection(
        const css::uno::Reference< css::sdbc::XConnection >& connection ) override;
    virtual css::uno::Reference< css::sdbcx::XTablesSupplier > SAL_CALL
    getDataDefinitionByURL(
        const OUString& url,
        const css::uno::Sequence< css::beans::PropertyValue >& info ) override;

    // XComponent
    virtual void SAL_CALL disposing() override;

};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
