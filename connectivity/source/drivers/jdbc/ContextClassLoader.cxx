/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextClassLoader.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:03:01 $
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
