/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: requeststringresolver.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:09:58 $
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

#ifndef UUI_REQUESTSTRINGRESOLVER_HXX
#define UUI_REQUESTSTRINGRESOLVER_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include "com/sun/star/lang/XServiceInfo.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUESTSTRINGRESOLVER_HPP_
#include "com/sun/star/task/XInteractionRequestStringResolver.hpp"
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include "cppuhelper/implbase2.hxx"
#endif

class UUIInteractionHelper;

class UUIInteractionRequestStringResolver:
    public cppu::WeakImplHelper2<
        com::sun::star::lang::XServiceInfo,
        com::sun::star::task::XInteractionRequestStringResolver >
{
public:
    static char const m_aImplementationName[];

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL
    createInstance(
        com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > const &
    rServiceFactory)
        SAL_THROW((com::sun::star::uno::Exception));

private:
    com::sun::star::uno::Reference<
    com::sun::star::lang::XMultiServiceFactory > m_xServiceFactory;
    UUIInteractionHelper * m_pImpl;

    UUIInteractionRequestStringResolver(UUIInteractionRequestStringResolver &); // not implemented
    void operator =(UUIInteractionRequestStringResolver); // not implemented

    UUIInteractionRequestStringResolver(
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >
                const & rServiceFactory)
        SAL_THROW(());

    virtual ~UUIInteractionRequestStringResolver() SAL_THROW(());

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                          rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::beans::Optional< rtl::OUString > SAL_CALL
    getStringFromInformationalRequest(
        const com::sun::star::uno::Reference<
            com::sun::star::task::XInteractionRequest >& Request )
        throw (com::sun::star::uno::RuntimeException);
};

#endif // UUI_REQUESTSTRINGRESOLVER_HXX
