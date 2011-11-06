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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "java/ContextClassLoader.hxx"
#include "java/lang/Object.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace connectivity { namespace jdbc
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    /** === end UNO using === **/

    using ::connectivity::java_lang_Object;

    //====================================================================
    //= ContextClassLoaderScope
    //====================================================================
    //--------------------------------------------------------------------
    ContextClassLoaderScope::ContextClassLoaderScope( JNIEnv& environment, const GlobalRef< jobject >& newClassLoader,
        const ::comphelper::ResourceBasedEventLogger& _rLoggerForErrors, const Reference< XInterface >& _rxErrorContext )
        :m_environment( environment )
        ,m_currentThread( environment )
        ,m_oldContextClassLoader( environment )
    {
        if ( !newClassLoader.is() )
            return;

        do  // artificial loop for easier flow control
        {

        LocalRef< jclass > threadClass( m_environment );
        threadClass.set( m_environment.FindClass( "java/lang/Thread" ) );
        if ( !threadClass.is() )
            break;

        jmethodID currentThreadMethod( m_environment.GetStaticMethodID(
            threadClass.get(), "currentThread", "()Ljava/lang/Thread;" ) );
        if ( currentThreadMethod == NULL )
            break;

        m_currentThread.set( m_environment.CallStaticObjectMethod( threadClass.get(), currentThreadMethod ) );
        if ( !m_currentThread.is() )
            break;

        jmethodID getContextClassLoaderMethod( m_environment.GetMethodID(
            threadClass.get(), "getContextClassLoader", "()Ljava/lang/ClassLoader;" ) );
        if ( getContextClassLoaderMethod == NULL )
            break;
        m_oldContextClassLoader.set( m_environment.CallObjectMethod( m_currentThread.get(), getContextClassLoaderMethod ) );
        LocalRef< jthrowable > throwable( m_environment, m_environment.ExceptionOccurred() );
        if ( throwable.is() )
            break;

        m_setContextClassLoaderMethod = m_environment.GetMethodID(
            threadClass.get(), "setContextClassLoader", "(Ljava/lang/ClassLoader;)V" );
        if ( m_setContextClassLoaderMethod == NULL )
            break;

        }
        while ( false );

        if ( !isActive() )
        {
            java_lang_Object::ThrowLoggedSQLException( _rLoggerForErrors, &environment, _rxErrorContext );
            return;
        }

        // set the new class loader
        m_environment.CallObjectMethod( m_currentThread.get(), m_setContextClassLoaderMethod, newClassLoader.get() );
        LocalRef< jthrowable > throwable( m_environment, m_environment.ExceptionOccurred() );
        if ( throwable.is() )
        {
            m_currentThread.reset();
            m_setContextClassLoaderMethod = NULL;
            java_lang_Object::ThrowLoggedSQLException( _rLoggerForErrors, &environment, _rxErrorContext );
        }
    }

    //--------------------------------------------------------------------
    void ContextClassLoaderScope::pop( bool clearExceptions )
    {
        if ( isActive() )
        {
            LocalRef< jobject > currentThread( m_currentThread.env(), m_currentThread.release() );
            jmethodID setContextClassLoaderMethod( m_setContextClassLoaderMethod );
            m_setContextClassLoaderMethod = NULL;

            m_environment.CallObjectMethod( currentThread.get(), setContextClassLoaderMethod, m_oldContextClassLoader.get() );
            if ( clearExceptions )
            {
                m_environment.ExceptionClear();
            }
        }
    }
//........................................................................
} } // namespace connectivity::jdbc
//........................................................................
