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

#include <cassert>
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
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "jni.h"

jobjectArray jvmaccess::ClassPath::translateToUrls(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    JNIEnv * environment, OUString const & classPath)
{
    assert(context.is());
    assert(environment != 0);
    jclass classUrl(environment->FindClass("java/net/URL"));
    if (classUrl == 0) {
        return 0;
    }
    jmethodID ctorUrl(
        environment->GetMethodID(classUrl, "<init>", "(Ljava/lang/String;)V"));
    if (ctorUrl == 0) {
        return 0;
    }
    ::std::vector< jobject > urls;
    for (::sal_Int32 i = 0; i != -1;) {
        OUString url(classPath.getToken(0, ' ', i));
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
                        "com.sun.star.lang.IllegalArgumentException: "
                         + e.Message);
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
        // character of the OUString classPath
    if (result == 0) {
        return 0;
    }
    jsize idx = 0;
    for (std::vector< jobject >::iterator i(urls.begin()); i != urls.end(); ++i)
    {
        environment->SetObjectArrayElement(result, idx++, *i);
    }
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
