/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: classpath.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-12 10:42:47 $
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

#include "sal/config.h"

#include "jvmaccess/classpath.hxx"

#include <vector>

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/ExternalUriReferenceTranslator.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/XVndSunStarExpandUrlReference.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#if defined SOLAR_JAVA
#include "jni.h"
#endif

namespace {

namespace css = ::com::sun::star;

}

void * ::jvmaccess::ClassPath::doTranslateToUrls(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    ::JNIEnv * environment, ::rtl::OUString const & classPath)
{
    OSL_ASSERT(context.is() && environment != 0);
#if defined SOLAR_JAVA
    jclass classUrl(environment->FindClass("java/net/URL"));
    if (classUrl == 0) {
        return 0;
    }
    jmethodID ctorUrl(
        environment->GetMethodID(
            classUrl, "<init>", "(Ljava/lang/String;)V"));
    if (ctorUrl == 0) {
        return 0;
    }
    ::std::vector< jobject > urls;
    for (::sal_Int32 i = 0; i != -1;) {
        ::rtl::OUString url(classPath.getToken(0, ' ', i));
        if (url.getLength() != 0) {
            css::uno::Reference< css::uri::XVndSunStarExpandUrlReference >
                expUrl(
                    css::uri::UriReferenceFactory::create(context)->parse(url),
                    css::uno::UNO_QUERY);
            if (expUrl.is()) {
                css::uno::Reference< css::util::XMacroExpander > expander(
                    context->getValueByName(
                        ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "/singletons/"
                                "com.sun.star.util.theMacroExpander"))),
                    css::uno::UNO_QUERY_THROW);
                try {
                    url = expUrl->expand(expander);
                } catch (css::lang::IllegalArgumentException & e) {
                    throw css::uno::RuntimeException(
                        (::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.lang.IllegalArgumentException: "))
                         + e.Message),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            if (url.getLength() != 0) {
                url = css::uri::ExternalUriReferenceTranslator::create(
                    context)->translateToExternal(url);
                if (url.getLength() == 0) {
                    throw css::uno::RuntimeException(
                        ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.uri."
                                "ExternalUriReferenceTranslator."
                                "translateToExternal failed")),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
            jvalue arg;
            arg.l = environment->NewString(
                static_cast< jchar const * >(url.getStr()),
                static_cast< jsize >(url.getLength()));
            if (arg.l == 0) {
                return 0;
            }
            jobject o(environment->NewObjectA(classUrl, ctorUrl, &arg));
            if (o == 0) {
                return 0;
            }
            urls.push_back(o);
        }
    }
    jobjectArray result = environment->NewObjectArray(
        static_cast< jsize >(urls.size()), classUrl, 0);
        // static_cast is ok, as each element of urls occupied at least one
        // character of the ::rtl::OUString classPath
    if (result == 0) {
        return 0;
    }
    jsize idx = 0;
    for (std::vector< jobject >::iterator i(urls.begin()); i != urls.end(); ++i)
    {
        environment->SetObjectArrayElement(result, idx++, *i);
    }
    return result;
#else
    return 0;
#endif
}

void * ::jvmaccess::ClassPath::doLoadClass(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    ::JNIEnv * environment, ::rtl::OUString const & classPath,
    ::rtl::OUString const & name)
{
    OSL_ASSERT(context.is() && environment != 0);
#if defined SOLAR_JAVA
    jclass classLoader(environment->FindClass("java/net/URLClassLoader"));
    if (classLoader == 0) {
        return 0;
    }
    jmethodID ctorLoader(
        environment->GetMethodID(classLoader, "<init>", "([Ljava/net/URL;)V"));
    if (ctorLoader == 0) {
        return 0;
    }
    jvalue arg;
    arg.l = translateToUrls(context, environment, classPath);
    if (arg.l == 0) {
        return 0;
    }
    jobject cl = environment->NewObjectA(classLoader, ctorLoader, &arg);
    if (cl == 0) {
        return 0;
    }
    jmethodID methLoadClass(
        environment->GetMethodID(
            classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"));
    if (methLoadClass == 0) {
        return 0;
    }
    arg.l = environment->NewString(
        static_cast< jchar const * >(name.getStr()),
        static_cast< jsize >(name.getLength()));
    if (arg.l == 0) {
        return 0;
    }
    return environment->CallObjectMethodA(cl, methLoadClass, &arg);
#else
    return 0;
#endif
}
