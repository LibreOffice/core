/*************************************************************************
 *
 *  $RCSfile: javaunohelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 14:36:35 $
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
#include <osl/module.h>

#include <uno/environment.hxx>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/component_context.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "jni.h"
#include "jvmaccess/virtualmachine.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OString;
using ::rtl::OUString;

namespace javaunohelper
{

//==================================================================================================
Reference< XComponentContext > install_vm_singleton(
    Reference< XComponentContext > const & xContext,
    ::rtl::Reference< ::jvmaccess::VirtualMachine > const & vm_access );
//==================================================================================================
::rtl::Reference< ::jvmaccess::VirtualMachine > create_vm_access( JNIEnv * jni_env );

}


/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_writeInfo
 * Signature: (Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jobject jSMgr, jobject jRegKey )
{
    sal_Bool bRet = sal_False;

    const jchar* pJLibName = pJEnv->GetStringChars( jLibName, NULL );
    OUString aLibName( pJLibName );
    pJEnv->ReleaseStringChars( jLibName, pJLibName);

    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (lib)
    {
        void * pSym;

        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        if (pSym = osl_getSymbol( lib, aGetEnvName.pData ))
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))(
                &pEnvTypeName, (uno_Environment **)&loader_env );
            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( (uno_Environment **)&loader_env, aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::VirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv ) );
            OUString java_env_name = OUSTR(UNO_LB_JAVA);
            uno_getEnvironment(
                (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

            OUString aWriteInfoName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_WRITEINFO) );
            if (pSym = osl_getSymbol( lib, aWriteInfoName.pData ))
            {
                if (loader_env.is() && java_env.is())
                {
                    Mapping java2dest(java_env.get(), loader_env.get());

                    if ( java2dest.is() )
                    {
                        void * pSMgr =
                            java2dest.mapInterface(
                                jSMgr, getCppuType((Reference< lang::XMultiServiceFactory > *) 0) );
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, getCppuType((Reference< registry::XRegistryKey > *) 0) );

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;
                        if (pKey)
                        {
                            bRet = (*((component_writeInfoFunc)pSym))( pSMgr, pKey );

                            if (env)
                                (*env->releaseInterface)( env, pKey );
                        }

                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );
                    }
                }
            }
        }
    }

    return bRet == sal_False? JNI_FALSE : JNI_TRUE;
}

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_getFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Ljava/lang/Object;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jLibName, jstring jImplName,
    jobject jSMgr, jobject jRegKey )
{
    const jchar* pJLibName = pJEnv->GetStringChars(jLibName, NULL);
    OUString aLibName( pJLibName );
    pJEnv->ReleaseStringChars( jLibName, pJLibName);

    aLibName += OUString( RTL_CONSTASCII_USTRINGPARAM(SAL_DLLEXTENSION) );

    jobject joSLL_cpp = 0;

    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );
    if (lib)
    {
        void * pSym;

        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        if (pSym = osl_getSymbol( lib, aGetEnvName.pData ))
        {
            Environment java_env, loader_env;

            const sal_Char * pEnvTypeName = 0;
            (*((component_getImplementationEnvironmentFunc)pSym))(
                &pEnvTypeName, (uno_Environment **)&loader_env );

            if (! loader_env.is())
            {
                OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );
                uno_getEnvironment( (uno_Environment **)&loader_env, aEnvTypeName.pData, 0 );
            }

            // create vm access
            ::rtl::Reference< ::jvmaccess::VirtualMachine > vm_access(
                ::javaunohelper::create_vm_access( pJEnv ) );
            OUString java_env_name = OUSTR(UNO_LB_JAVA);
            uno_getEnvironment(
                (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY) );
            if (pSym = osl_getSymbol( lib, aGetFactoryName.pData ))
            {
                if (loader_env.is() && java_env.is())
                {
                    Mapping java2dest( java_env.get(), loader_env.get() );
                    Mapping dest2java( loader_env.get(), java_env.get() );

                    if (dest2java.is() && java2dest.is())
                    {
                        void * pSMgr =
                            java2dest.mapInterface(
                                jSMgr, ::getCppuType((Reference< lang::XMultiServiceFactory > *) 0) );
                        void * pKey =
                            java2dest.mapInterface(
                                jRegKey, ::getCppuType((Reference< registry::XRegistryKey > *) 0) );

                        const char* pImplName = pJEnv->GetStringUTFChars( jImplName, NULL );

                        void * pSSF = (*((component_getFactoryFunc)pSym))(
                            pImplName, pSMgr, pKey );

                        pJEnv->ReleaseStringUTFChars( jImplName, pImplName );

                        uno_ExtEnvironment * env = loader_env.get()->pExtEnv;

                        if (pKey && env)
                            (*env->releaseInterface)( env, pKey );
                        if (pSMgr && env)
                            (*env->releaseInterface)( env, pSMgr );

                        if (pSSF)
                        {
                            jobject jglobal = (jobject) dest2java.mapInterface(
                                pSSF, getCppuType((Reference< XInterface > *) 0) );
                            joSLL_cpp = pJEnv->NewLocalRef( jglobal );
                            pJEnv->DeleteGlobalRef( jglobal );
                            if (env)
                                (*env->releaseInterface)( env, pSSF );
                        }
                    }
                }
            }
        }
    }

    return joSLL_cpp;
}

/*
 * Class:     com_sun_star_comp_helper_RegistryServiceFactory
 * Method:    createRegistryServiceFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/Object;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory(
    JNIEnv * pJEnv, jclass jClass, jstring jWriteRegFile,
    jstring jReadRegFile, jboolean jbReadOnly )
{
    jobject joRegServiceFac = 0;

    try
    {
        OUString aWriteRegFile;
        OUString aReadRegFile;

        sal_Bool bReadOnly = jbReadOnly == JNI_FALSE ? sal_False : sal_True;

        if (jReadRegFile) {
            const jchar* pjReadRegFile = pJEnv->GetStringChars(jReadRegFile, NULL);
            aReadRegFile = OUString(pjReadRegFile);
            pJEnv->ReleaseStringChars(jReadRegFile, pjReadRegFile);
        }

        if (jWriteRegFile) {
            const jchar * pjWriteRegFile = pJEnv->GetStringChars(jWriteRegFile, NULL);
            aWriteRegFile = OUString(pjWriteRegFile);
            pJEnv->ReleaseStringChars(jWriteRegFile, pjWriteRegFile);
        }

        // bootstrap
        Reference< lang::XMultiServiceFactory > rMSFac;
        if (aReadRegFile.getLength() == 0)
            rMSFac = ::cppu::createRegistryServiceFactory( aWriteRegFile, bReadOnly);
        else
            rMSFac = ::cppu::createRegistryServiceFactory(aWriteRegFile, aReadRegFile, bReadOnly);

        Reference< beans::XPropertySet > xProps(
            rMSFac, UNO_QUERY_THROW );
        Reference< XComponentContext > xContext(
            xProps->getPropertyValue( OUSTR("DefaultContext") ), UNO_QUERY_THROW );

        // create vm access
        ::rtl::Reference< ::jvmaccess::VirtualMachine > vm_access(
            ::javaunohelper::create_vm_access( pJEnv ) );
        // wrap vm singleton entry
        xContext = ::javaunohelper::install_vm_singleton( xContext, vm_access );
        rMSFac.set( xContext->getServiceManager(), UNO_QUERY_THROW );

        // get uno envs
        OUString aCurrentEnv(RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME));
        OUString java_env_name = OUSTR(UNO_LB_JAVA);
        Environment java_env, curr_env;
        uno_getEnvironment((uno_Environment **)&curr_env, aCurrentEnv.pData, NULL);
        uno_getEnvironment( (uno_Environment **)&java_env, java_env_name.pData, vm_access.get() );

        Mapping curr_java(curr_env.get(), java_env.get());
        if (! curr_java.is())
        {
            throw RuntimeException(
                OUSTR("no C++ <-> Java mapping available!"), Reference< XInterface >() );
        }

        jobject joGlobalRegServiceFac =
            (jobject)curr_java.mapInterface(
                rMSFac.get(),
                getCppuType((Reference< lang::XMultiServiceFactory > *)0) );
        joRegServiceFac = pJEnv->NewLocalRef( joGlobalRegServiceFac );
          pJEnv->DeleteGlobalRef(joGlobalRegServiceFac);
    }
    catch (Exception & exc)
    {
        jclass c = pJEnv->FindClass( "com/sun/star/uno/RuntimeException" );
        if (0 != c)
        {
            OString cstr( ::rtl::OUStringToOString(
                              exc.Message, RTL_TEXTENCODING_JAVA_UTF8 ) );
            OSL_TRACE( __FILE__": forwarding Exception: %s", cstr.getStr() );
            pJEnv->ThrowNew( c, cstr.getStr() );
        }
        return 0;
    }

    OSL_TRACE("javaunohelper.cxx: object %i", joRegServiceFac);

    return joRegServiceFac;
}
