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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_CONTEXTCLASSLOADER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_CONTEXTCLASSLOADER_HXX

#include "java/GlobalRef.hxx"

namespace comphelper
{
    class ResourceBasedEventLogger;
}


namespace connectivity { namespace jdbc
{
    class ContextClassLoaderScope
    {
    public:
        /** creates the instance. If isActive returns <FALSE/> afterwards, then an exception
            happened in the JVM, which should be raised as UNO exception by the caller

            @param  environment
                the current JNI environment
            @param  newClassLoader
                the new class loader to set at the current thread
            @param  _rLoggerForErrors
                the logger which should be passed to java_lang_object::ThrowLoggedSQLException in case
                an error occurs
            @param  _rxErrorContext
                the context which should be passed to java_lang_object::ThrowLoggedSQLException in case
                an error occurs

        */
        ContextClassLoaderScope(
            JNIEnv& environment,
            const GlobalRef< jobject >& newClassLoader,
            const ::comphelper::ResourceBasedEventLogger& _rLoggerForErrors,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxErrorContext
        );

        ~ContextClassLoaderScope() { pop(true); }

        bool isActive() const
        {
            return  ( m_currentThread.is() )
                &&  ( m_setContextClassLoaderMethod != nullptr );
        }

    private:
        ContextClassLoaderScope(ContextClassLoaderScope &) = delete;
        void operator =(ContextClassLoaderScope &) = delete;

        void pop( bool clearExceptions );

        JNIEnv&                             m_environment;
        LocalRef< jobject >                 m_currentThread;
        LocalRef< jobject >                 m_oldContextClassLoader;
        jmethodID                           m_setContextClassLoaderMethod;
    };



} } // namespace connectivity::jdbc


#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_JAVA_CONTEXTCLASSLOADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
