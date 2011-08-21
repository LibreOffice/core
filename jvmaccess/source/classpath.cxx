/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "sal/config.h"

#include "jvmaccess/classpath.hxx"

#include <vector>

#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
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
    void * environment, ::rtl::OUString const & classPath)
{
    OSL_ASSERT(context.is() && environment != 0);
#if defined SOLAR_JAVA
    ::JNIEnv * const env = static_cast< ::JNIEnv * >(environment);
    jclass classUrl(env->FindClass("java/net/URL"));
    if (classUrl == 0) {
        return 0;
    }
    jmethodID ctorUrl(
        env->GetMethodID(classUrl, "<init>", "(Ljava/lang/String;)V"));
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
            jvalue arg;
            arg.l = env->NewString(
                static_cast< jchar const * >(url.getStr()),
                static_cast< jsize >(url.getLength()));
            if (arg.l == 0) {
                return 0;
            }
            jobject o(env->NewObjectA(classUrl, ctorUrl, &arg));
            if (o == 0) {
                return 0;
            }
            urls.push_back(o);
        }
    }
    jobjectArray result = env->NewObjectArray(
        static_cast< jsize >(urls.size()), classUrl, 0);
        // static_cast is ok, as each element of urls occupied at least one
        // character of the ::rtl::OUString classPath
    if (result == 0) {
        return 0;
    }
    jsize idx = 0;
    for (std::vector< jobject >::iterator i(urls.begin()); i != urls.end(); ++i)
    {
        env->SetObjectArrayElement(result, idx++, *i);
    }
    return result;
#else
    return 0;
#endif
}

void * ::jvmaccess::ClassPath::doLoadClass(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    void * environment, ::rtl::OUString const & classPath,
    ::rtl::OUString const & name)
{
    OSL_ASSERT(context.is() && environment != 0);
#if defined SOLAR_JAVA
    ::JNIEnv * const env = static_cast< ::JNIEnv * >(environment);
    jclass classLoader(env->FindClass("java/net/URLClassLoader"));
    if (classLoader == 0) {
        return 0;
    }
    jmethodID ctorLoader(
        env->GetMethodID(classLoader, "<init>", "([Ljava/net/URL;)V"));
    if (ctorLoader == 0) {
        return 0;
    }
    jvalue arg;
    arg.l = translateToUrls(context, env, classPath);
    if (arg.l == 0) {
        return 0;
    }
    jobject cl = env->NewObjectA(classLoader, ctorLoader, &arg);
    if (cl == 0) {
        return 0;
    }
    jmethodID methLoadClass(
        env->GetMethodID(
            classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"));
    if (methLoadClass == 0) {
        return 0;
    }
    arg.l = env->NewString(
        static_cast< jchar const * >(name.getStr()),
        static_cast< jsize >(name.getLength()));
    if (arg.l == 0) {
        return 0;
    }
    return env->CallObjectMethodA(cl, methLoadClass, &arg);
#else
    return 0;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
