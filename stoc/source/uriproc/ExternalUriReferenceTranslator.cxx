/*************************************************************************
 *
 *  $RCSfile: ExternalUriReferenceTranslator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:43:27 $
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

#include "ExternalUriReferenceTranslator.hxx"

#include "supportsService.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/XExternalUriReferenceTranslator.hpp"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/thread.h"
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

class Translator: public cppu::WeakImplHelper2<
    css::lang::XServiceInfo, css::uri::XExternalUriReferenceTranslator >
{
public:
    explicit Translator(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
    translateToInternal(rtl::OUString const & externalUriReference)
        throw (css::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL
    translateToExternal(rtl::OUString const & internalUriReference)
        throw (css::uno::RuntimeException);

private:
    Translator(Translator &); // not implemented
    void operator =(Translator); // not implemented

    virtual ~Translator() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

rtl::OUString Translator::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return
        stoc::uriproc::ExternalUriReferenceTranslator::getImplementationName();
}

sal_Bool Translator::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::supportsService(
        getSupportedServiceNames(), serviceName);
}

css::uno::Sequence< rtl::OUString > Translator::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return stoc::uriproc::ExternalUriReferenceTranslator::
        getSupportedServiceNames();
}

rtl::OUString Translator::translateToInternal(
    rtl::OUString const & externalUriReference)
    throw (css::uno::RuntimeException)
{
    if (!externalUriReference.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("file:/")))
    {
        return externalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file:");
    rtl::OUStringBuffer buf;
    buf.append(externalUriReference.getStr(), i);
    // Some environments (e.g., Java) produce illegal file URLs without an
    // authority part; treat them as having an empty authority part:
    if (!externalUriReference.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("//"), i))
    {
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
    }
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    for (bool path = true;;) {
        sal_Int32 j = i;
        while (j != externalUriReference.getLength()
               && externalUriReference[j] != '#'
               && (!path || externalUriReference[j] != '/'))
        {
            ++j;
        }
        if (j != i) {
            rtl::OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        externalUriReference.copy(i, j - i),
                        rtl_UriDecodeStrict, encoding),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrict,
                    RTL_TEXTENCODING_UTF8));
            if (seg.getLength() == 0) {
                return rtl::OUString();
            }
            buf.append(seg);
        }
        if (j == externalUriReference.getLength()) {
            break;
        }
        buf.append(externalUriReference[j]);
        path = externalUriReference[j] == '/';
        i = j + 1;
    }
    return buf.makeStringAndClear();
}

rtl::OUString Translator::translateToExternal(
    rtl::OUString const & internalUriReference)
    throw (css::uno::RuntimeException)
{
    if (!internalUriReference.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("file://")))
    {
        return internalUriReference;
    }
    sal_Int32 i = RTL_CONSTASCII_LENGTH("file://");
    rtl::OUStringBuffer buf;
    buf.append(internalUriReference.getStr(), i);
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    for (bool path = true;;) {
        sal_Int32 j = i;
        while (j != internalUriReference.getLength()
               && internalUriReference[j] != '#'
               && (!path || internalUriReference[j] != '/'))
        {
            ++j;
        }
        if (j != i) {
            rtl::OUString seg(
                rtl::Uri::encode(
                    rtl::Uri::decode(
                        internalUriReference.copy(i, j - i),
                        rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8),
                    rtl_UriCharClassPchar, rtl_UriEncodeStrict, encoding));
            if (seg.getLength() == 0) {
                return rtl::OUString();
            }
            buf.append(seg);
        }
        if (j == internalUriReference.getLength()) {
            break;
        }
        buf.append(internalUriReference[j]);
        path = internalUriReference[j] == '/';
        i = j + 1;
    }
    return buf.makeStringAndClear();
}

}

namespace stoc { namespace uriproc { namespace ExternalUriReferenceTranslator {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Translator(context));
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString::createFromAscii("std::bad_alloc"), 0);
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString::createFromAscii(
        "com.sun.star.comp.uri.ExternalUriReferenceTranslator");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    css::uno::Sequence< rtl::OUString > s(1);
    s[0] = rtl::OUString::createFromAscii(
        "com.sun.star.uri.ExternalUriReferenceTranslator");
    return s;
}

} } }
