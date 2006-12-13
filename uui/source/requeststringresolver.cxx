/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: requeststringresolver.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:09:46 $
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

#include "requeststringresolver.hxx"
#include "iahndl.hxx"

using namespace com::sun;

UUIInteractionRequestStringResolver::UUIInteractionRequestStringResolver(
    star::uno::Reference< star::lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW(())
        : m_xServiceFactory(rServiceFactory),
          m_pImpl(new UUIInteractionHelper(rServiceFactory))
{
}

UUIInteractionRequestStringResolver::~UUIInteractionRequestStringResolver()
{
    delete m_pImpl;
}

rtl::OUString SAL_CALL
UUIInteractionRequestStringResolver::getImplementationName()
    throw (star::uno::RuntimeException)
{
    return rtl::OUString::createFromAscii(m_aImplementationName);
}

sal_Bool SAL_CALL
UUIInteractionRequestStringResolver::supportsService(
        rtl::OUString const & rServiceName)
    throw (star::uno::RuntimeException)
{
    star::uno::Sequence< rtl::OUString >
        aNames(getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i < aNames.getLength(); ++i)
        if (aNames[i] == rServiceName)
            return true;
    return false;
}

star::uno::Sequence< rtl::OUString > SAL_CALL
UUIInteractionRequestStringResolver::getSupportedServiceNames()
    throw (star::uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

star::beans::Optional< rtl::OUString > SAL_CALL
UUIInteractionRequestStringResolver::getStringFromInformationalRequest(
    const star::uno::Reference<
        star::task::XInteractionRequest >& Request )
    throw (star::uno::RuntimeException)
{
    try
    {
        return m_pImpl->getStringFromRequest(Request);
    }
    catch (star::uno::RuntimeException const & ex)
    {
        throw star::uno::RuntimeException(ex.Message, *this);
    }
}

char const UUIInteractionRequestStringResolver::m_aImplementationName[]
    = "com.sun.star.comp.uui.UUIInteractionRequestStringResolver";

star::uno::Sequence< rtl::OUString >
UUIInteractionRequestStringResolver::getSupportedServiceNames_static()
{
    star::uno::Sequence< rtl::OUString > aNames(1);
    aNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.task.InteractionRequestStringResolver"));
    return aNames;
}

star::uno::Reference< star::uno::XInterface > SAL_CALL
UUIInteractionRequestStringResolver::createInstance(
    star::uno::Reference< star::lang::XMultiServiceFactory > const &
        rServiceFactory)
    SAL_THROW((star::uno::Exception))
{
    try
    {
        return *new UUIInteractionRequestStringResolver(rServiceFactory);
    }
    catch (std::bad_alloc const &)
    {
        throw star::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
        0);
    }
}
