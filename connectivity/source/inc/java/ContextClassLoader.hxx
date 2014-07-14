/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef CONNECTIVITY_CONTEXTCLASSLOADER_HXX
#define CONNECTIVITY_CONTEXTCLASSLOADER_HXX

#include "java/GlobalRef.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

namespace comphelper
{
    class ResourceBasedEventLogger;
}

//........................................................................
namespace connectivity { namespace jdbc
{
//........................................................................

    //====================================================================
    //= ContextClassLoaderScope
    //====================================================================
    /**
    */
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

        void pop() { pop(false); }

        bool isActive() const
        {
            return  ( m_currentThread.is() )
                &&  ( m_setContextClassLoaderMethod != NULL );
        }

    private:
        ContextClassLoaderScope(ContextClassLoaderScope &); // not defined
        void operator =(ContextClassLoaderScope &); // not defined

        void pop( bool clearExceptions );

        JNIEnv&                             m_environment;
        LocalRef< jobject >                 m_currentThread;
        LocalRef< jobject >                 m_oldContextClassLoader;
        jmethodID                           m_setContextClassLoaderMethod;
    };


//........................................................................
} } // namespace connectivity::jdbc
//........................................................................

#endif // CONNECTIVITY_CONTEXTCLASSLOADER_HXX
