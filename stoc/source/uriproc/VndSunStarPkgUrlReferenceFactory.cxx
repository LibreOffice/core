/*************************************************************************
 *
 *  $RCSfile: VndSunStarPkgUrlReferenceFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:43:51 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "VndSunStarPkgUrlReferenceFactory.hxx"

#include "supportsService.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "com/sun/star/uri/XUriReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarPkgUrlReferenceFactory.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/uri.h"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <new>

namespace css = com::sun::star;

namespace {

class Factory: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XVndSunStarPkgUrlReferenceFactory >
{
public:
    explicit Factory(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::uri::XUriReference > SAL_CALL
    createVndSunStarPkgUrlReference(
        css::uno::Reference< css::uri::XUriReference > const & authority)
        throw (css::uno::RuntimeException);

private:
    Factory(Factory &); // not implemented
    void operator =(Factory); // not implemented

    virtual ~Factory() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

rtl::OUString Factory::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return
        stoc::uriproc::VndSunStarPkgUrlReferenceFactory::
        getImplementationName();
}

sal_Bool Factory::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > Factory::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::VndSunStarPkgUrlReferenceFactory::
        getSupportedServiceNames();
}

css::uno::Reference< css::uri::XUriReference >
Factory::createVndSunStarPkgUrlReference(
    css::uno::Reference< css::uri::XUriReference > const & authority)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(authority.is());
    if (authority->isAbsolute() && !authority->hasFragment()) {
        rtl::OUStringBuffer buf;
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg://"));
        buf.append(
            rtl::Uri::encode(
                authority->getUriReference(), rtl_UriCharClassRegName,
                rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8));
        css::uno::Reference< css::uri::XUriReference > uriRef(
            css::uri::UriReferenceFactory::create(m_context)->parse(
                buf.makeStringAndClear()));
        OSL_ASSERT(uriRef.is());
        return uriRef;
    } else {
        return css::uno::Reference< css::uri::XUriReference >();
    }
}

}

namespace stoc { namespace uriproc { namespace VndSunStarPkgUrlReferenceFactory
{

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Factory(context));
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString::createFromAscii(
        "com.sun.star.comp.uri.VndSunStarPkgUrlReferenceFactory");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString::createFromAscii(
        "com.sun.star.uri.VndSunStarPkgUrlReferenceFactory");
    return s;
}

} } }
