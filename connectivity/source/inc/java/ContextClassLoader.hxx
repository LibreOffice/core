/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextClassLoader.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:06:42 $
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
            happend in the JVM, which should be raised as UNO exception by the caller

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
