/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:36:26 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "osl/diagnose.h"
#include "osl/mutex.hxx"

#include "rtl/alloc.h"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"

#include "uno/mapping.hxx"
#include "uno/environment.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/component_context.hxx"

#include "com/sun/star/lang/XSingleComponentFactory.hpp"

#include "jni.h"
#include "jvmaccess/virtualmachine.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OString;
using ::rtl::OUString;

namespace javaunohelper
{
struct MutexHolder
{
    ::osl::Mutex m_mutex;
};
typedef ::cppu::WeakComponentImplHelper1< lang::XSingleComponentFactory > t_impl;

//==================================================================================================
class SingletonFactory : public MutexHolder, public t_impl
{
    ::rtl::Reference< ::jvmaccess::VirtualMachine > m_vm_access;

protected:
    virtual void SAL_CALL disposing();

public:
    inline SingletonFactory( ::rtl::Reference< ::jvmaccess::VirtualMachine > const & vm_access )
        : t_impl( m_mutex ),
          m_vm_access( vm_access )
        {}

    // XSingleComponentFactory impl
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & args, Reference< XComponentContext > const & xContext )
        throw (Exception);
};
//__________________________________________________________________________________________________
void SingletonFactory::disposing()
{
    m_vm_access.clear();
}
//__________________________________________________________________________________________________
Reference< XInterface > SingletonFactory::createInstanceWithContext(
    Reference< XComponentContext > const & xContext )
    throw (Exception)
{
    sal_Int64 handle = reinterpret_cast< sal_Int64 >( m_vm_access.get() );
    Any arg( makeAny( handle ) );
    return xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        OUSTR("com.sun.star.java.JavaVirtualMachine"), Sequence< Any >( &arg, 1 ), xContext );
}
//__________________________________________________________________________________________________
Reference< XInterface > SingletonFactory::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & args, Reference< XComponentContext > const & xContext )
    throw (Exception)
{
    return xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        OUSTR("com.sun.star.java.JavaVirtualMachine"), args, xContext );
}

//==================================================================================================
Reference< XComponentContext > install_vm_singleton(
    Reference< XComponentContext > const & xContext,
    ::rtl::Reference< ::jvmaccess::VirtualMachine > const & vm_access )
{
    Reference< lang::XSingleComponentFactory > xFac( new SingletonFactory( vm_access ) );
    ::cppu::ContextEntry_Init entry(
        OUSTR("/singletons/com.sun.star.java.theJavaVirtualMachine"), makeAny( xFac ), true );
    return ::cppu::createComponentContext( &entry, 1, xContext );
}
//==================================================================================================
::rtl::Reference< ::jvmaccess::VirtualMachine > create_vm_access( JNIEnv * jni_env )
{
    JavaVM * vm;
    jni_env->GetJavaVM( &vm );
    return new ::jvmaccess::VirtualMachine( vm, JNI_VERSION_1_2, false, jni_env );
}

//==================================================================================================
inline ::rtl::OUString jstring_to_oustring( jstring jstr, JNIEnv * jni_env )
{
    OSL_ASSERT( sizeof (sal_Unicode) == sizeof (jchar) );
    jsize len = jni_env->GetStringLength( jstr );
    rtl_uString * ustr =
        (rtl_uString *)rtl_allocateMemory( sizeof (rtl_uString) + (len * sizeof (sal_Unicode)) );
    jni_env->GetStringRegion( jstr, 0, len, ustr->buffer );
    OSL_ASSERT( JNI_FALSE == jni_env->ExceptionCheck() );
    ustr->refCount = 1;
    ustr->length = len;
    ustr->buffer[ len ] = '\0';
    return ::rtl::OUString( ustr, SAL_NO_ACQUIRE );
}

}

//==================================================================================================
extern "C" JNIEXPORT jobject JNICALL Java_com_sun_star_comp_helper_Bootstrap_cppuhelper_1bootstrap(
    JNIEnv * jni_env, jclass jClass, jstring juno_rc, jobjectArray jpairs )
{
    try
    {
        if (0 != jpairs)
        {
            jsize nPos = 0, len = jni_env->GetArrayLength( jpairs );
            while (nPos < len)
            {
                // name
                jstring jstr = (jstring)jni_env->GetObjectArrayElement( jpairs, nPos );
                if (JNI_FALSE != jni_env->ExceptionCheck())
                {
                    jni_env->ExceptionClear();
                    throw RuntimeException(
                        OUSTR("index out of bounds?!"), Reference< XInterface >() );
                }
                if (0 != jstr)
                {
                    OUString name( ::javaunohelper::jstring_to_oustring( jstr, jni_env ) );
                    // value
                    jstr = (jstring)jni_env->GetObjectArrayElement( jpairs, nPos +1 );
                    if (JNI_FALSE != jni_env->ExceptionCheck())
                    {
                        jni_env->ExceptionClear();
                        throw RuntimeException(
                            OUSTR("index out of bounds?!"), Reference< XInterface >() );
                    }
                    if (0 != jstr)
                    {
                        OUString value( ::javaunohelper::jstring_to_oustring( jstr, jni_env ) );

                        // set bootstrap parameter
                        ::rtl::Bootstrap::set( name, value );
                    }
                }
                nPos += 2;
            }
        }

        // bootstrap uno
        Reference< XComponentContext > xContext;
        if (0 == juno_rc)
        {
            xContext = ::cppu::defaultBootstrap_InitialComponentContext();
        }
        else
        {
            OUString uno_rc( ::javaunohelper::jstring_to_oustring( juno_rc, jni_env ) );
            xContext = ::cppu::defaultBootstrap_InitialComponentContext( uno_rc );
        }

        // create vm access
        ::rtl::Reference< ::jvmaccess::VirtualMachine > vm_access(
            ::javaunohelper::create_vm_access( jni_env ) );
        // wrap vm singleton entry
        xContext = ::javaunohelper::install_vm_singleton( xContext, vm_access );

        // get uno envs
        OUString cpp_env_name = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        OUString java_env_name = OUSTR(UNO_LB_JAVA);
        Environment java_env, cpp_env;
        uno_getEnvironment((uno_Environment **)&cpp_env, cpp_env_name.pData, NULL);
        uno_getEnvironment( (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

        // map to java
        Mapping mapping( cpp_env.get(), java_env.get() );
        if (! mapping.is())
        {
            Reference< lang::XComponent > xComp( xContext, UNO_QUERY );
            if (xComp.is())
                xComp->dispose();
            throw RuntimeException(
                OUSTR("cannot get mapping C++ <-> Java!"),
                Reference< XInterface >() );
        }

        jobject jret = (jobject)mapping.mapInterface( xContext.get(), ::getCppuType( &xContext ) );
        jobject jlocal = jni_env->NewLocalRef( jret );
        jni_env->DeleteGlobalRef( jret );

        return jlocal;
    }
    catch (RuntimeException & exc)
    {
        jclass c = jni_env->FindClass( "com/sun/star/uno/RuntimeException" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding RuntimeException: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }
    catch (Exception & exc)
    {
        jclass c = jni_env->FindClass( "com/sun/star/uno/Exception" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }

    return 0;
}

