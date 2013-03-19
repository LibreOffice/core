/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include "com/sun/star/util/theMacroExpander.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#if defined SOLAR_JAVA
#include "jni.h"
#endif

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
        if (!url.isEmpty()) {
            css::uno::Reference< css::uri::XVndSunStarExpandUrlReference >
                expUrl(
                    css::uri::UriReferenceFactory::create(context)->parse(url),
                    css::uno::UNO_QUERY);
            if (expUrl.is()) {
                css::uno::Reference< css::util::XMacroExpander > expander =
                    css::util::theMacroExpander::get(context);
                try {
                    url = expUrl->expand( expander );
                } catch (const css::lang::IllegalArgumentException & e) {
                    throw css::uno::RuntimeException(
                        (::rtl::OUString(
                                "com.sun.star.lang.IllegalArgumentException: ")
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
    (void) context;
    (void) environment;
    (void) classPath;
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
    (void) context;
    (void) environment;
    (void) classPath;
    (void) name;
    return 0;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
