/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2002-11-14 16:31:02 $
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

#include <osl/diagnose.h>
#include <rtl/alloc.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>

#include <uno/mapping.hxx>
#include <cppuhelper/bootstrap.hxx>

#include "jvm_uno_helper.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::jvm_uno_helper;

namespace jvm_uno_helper
{
//==================================================================================================
void get_java_env( Environment * java_env, JNIEnv * jni_env ) SAL_THROW( () )
{
    OUString java_env_name = OUSTR(UNO_LB_JAVA);

    uno_Environment ** java_envs;
    sal_Int32 nSize;
    uno_getRegisteredEnvironments(
        &java_envs, &nSize, (uno_memAlloc)rtl_allocateMemory, java_env_name.pData );
    if (0 < nSize)
    {
        java_env->operator = ( java_envs[ 0 ] );
        while (nSize--)
        {
            (*java_envs[ nSize ]->release)( java_envs[ nSize ] );
        }
        rtl_freeMemory( java_envs );
    }
    else
    {
        JavaVM * vm;
        jni_env->GetJavaVM( &vm );
        ::JavaVMContext * jvm_context = new JavaVMContext( vm );
        JVM_registration_guard jvm_guard( jvm_context );
        uno_getEnvironment( (uno_Environment **)java_env, java_env_name.pData, jvm_context );
    }
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
                    OUString name( jstring_to_oustring( jstr, jni_env ) );
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
                        OUString value( jstring_to_oustring( jstr, jni_env ) );

                        // set bootstrap parameter
                        Bootstrap::set( name, value );
                    }
                }
                nPos += 2;
            }
        }

        // env
        OUString cpp_env_name = OUSTR(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        Environment cpp_env, java_env;
        uno_getEnvironment( (uno_Environment **)&cpp_env, cpp_env_name.pData, 0 );
        if (! cpp_env.is())
            throw RuntimeException( OUSTR("cannot get cpp env!"), Reference< XInterface >() );
        get_java_env( &java_env, jni_env );
        if (! java_env.is())
            throw RuntimeException( OUSTR("cannot get java env!"), Reference< XInterface >() );
        // register before doing any complex uno that may call java (beware of detaching!)
        JVM_registration_guard jvm_guard( java_env.get() );

        // bootstrap uno
        Reference< XComponentContext > xContext;
        if (0 == juno_rc)
        {
            xContext = ::cppu::defaultBootstrap_InitialComponentContext();
        }
        else
        {
            OUString uno_rc( jstring_to_oustring( juno_rc, jni_env ) );
            xContext = ::cppu::defaultBootstrap_InitialComponentContext( uno_rc );
        }

        // map to java
        Mapping mapping( cpp_env.get(), java_env.get() );
        if (! mapping.is())
        {
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
            OString cstr( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( __FILE__": forwarding RuntimeException: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }
    catch (Exception & exc)
    {
        jclass c = jni_env->FindClass( "com/sun/star/uno/Exception" );
        if (0 != c)
        {
            OString cstr( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            jni_env->ThrowNew( c, cstr.getStr() );
        }
    }

    return 0;
}

