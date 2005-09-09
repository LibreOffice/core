/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MasterScriptProviderFactory.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:29:57 $
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
