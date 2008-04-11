/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterScriptProviderFactory.hxx,v $
 * $Revision: 1.7 $
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
#include "rtl/ustring.hxx"
#include "rtl/ref.hxx"
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include "ActiveMSPList.hxx"

namespace func_provider
{
// for simplification
#define css ::com::sun::star

class MasterScriptProviderFactory :
    public ::cppu::WeakImplHelper2 <
        css::script::provider::XScriptProviderFactory,
        css::lang::XServiceInfo >
{
private:

    mutable rtl::Reference< ActiveMSPList > m_MSPList;

    const css::uno::Reference< css::uno::XComponentContext > m_xComponentContext;

    const rtl::Reference< ActiveMSPList > & getActiveMSPList() const;

protected:
    virtual ~MasterScriptProviderFactory();

public:
    MasterScriptProviderFactory(
        css::uno::Reference< css::uno::XComponentContext > const & xComponentContext );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
        supportsService( ::rtl::OUString const & serviceName )
            throw ( css::uno::RuntimeException );

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames()
            throw ( css::uno::RuntimeException );

    // XScriptProviderFactory
    virtual css::uno::Reference< css::script::provider::XScriptProvider >
        SAL_CALL createScriptProvider( const css::uno::Any& context )
            throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException);
};


} // namespace func_provider
