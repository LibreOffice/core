/*************************************************************************
 *
 *  $RCSfile: javaunohelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2000-11-24 15:47:12 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#include <bridges/java/jvmcontext.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace rtl;

#define JAVA_ENV_NAME "java"

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_writeInfo
 * Signature: (Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1writeInfo
  (JNIEnv * pJEnv, jclass jClass, jstring jLibName, jobject jSMgr, jobject jRegKey)
{
    OUString sMessage;
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
            uno_Environment * pJavaEnv = 0;
            uno_Environment * pLoaderEnv = 0;
            const sal_Char * pEnvTypeName = 0;

            (*((component_getImplementationEnvironmentFunc)pSym))( &pEnvTypeName, &pLoaderEnv );
            OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

            if (! pLoaderEnv)
                uno_getEnvironment( &pLoaderEnv, aEnvTypeName.pData, 0 );

            JavaVM * pJVM;
            pJEnv->GetJavaVM( &pJVM );

            JavaVMContext * pVMContext = new JavaVMContext(pJVM);
            uno_getEnvironment(&pJavaEnv, OUString::createFromAscii("java").pData, pVMContext);

            OUString aWriteInfoName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_WRITEINFO) );
            if (pSym = osl_getSymbol( lib, aWriteInfoName.pData ))
            {
                if (pJavaEnv && pLoaderEnv)
                {
                    Mapping java2dest(pJavaEnv, pLoaderEnv);

                    if ( java2dest.is() )
                    {
                        typelib_InterfaceTypeDescription * pType = 0;

                        getCppuType((Reference< XMultiServiceFactory > *) 0).getDescription((typelib_TypeDescription **) & pType);
                        void * pSMgr  = java2dest.mapInterface(jSMgr, pType);

                        getCppuType((Reference< XRegistryKey > *) 0).getDescription((typelib_TypeDescription **) & pType);
                        void * pKey = java2dest.mapInterface(jRegKey, pType);

                        if (pKey)
                        {
                            bRet = (*((component_writeInfoFunc)pSym))( pSMgr, pKey );

                            if (pLoaderEnv->pExtEnv)
                            (*pLoaderEnv->pExtEnv->releaseInterface)( pLoaderEnv->pExtEnv, pKey );
                        }

                        if (pSMgr && pLoaderEnv->pExtEnv)
                            (*pLoaderEnv->pExtEnv->releaseInterface)( pLoaderEnv->pExtEnv, pSMgr );
                    }
                }
            } else
            {
                sMessage = OUString::createFromAscii("symbol \"");
                sMessage += aWriteInfoName;
                sMessage += OUString::createFromAscii("\" could not be found in \"");
                sMessage += aLibName;
                sMessage += OUString::createFromAscii("\"");
            }

            if (pLoaderEnv)
                (*pLoaderEnv->release)( pLoaderEnv );
            if (pJavaEnv)
                (*pJavaEnv->release)( pJavaEnv );
        }
    }

    return bRet == sal_False? JNI_FALSE : JNI_TRUE;
}

/*
 * Class:     com_sun_star_comp_helper_SharedLibraryLoader
 * Method:    component_getFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Lcom/sun/star/lang/XMultiServiceFactory;Lcom/sun/star/registry/XRegistryKey;)Ljava/lang/Object;
 */
extern "C" JNIEXPORT jobject JNICALL Java_com_sun_star_comp_helper_SharedLibraryLoader_component_1getFactory
  (JNIEnv * pJEnv, jclass jClass, jstring jLibName, jstring jImplName, jobject jSMgr, jobject jRegKey)
{   OUString sMessage;

    const jchar* pJLibName = pJEnv->GetStringChars(jLibName, NULL);
    OUString aLibName( pJLibName );
    pJEnv->ReleaseStringChars( jLibName, pJLibName);

    //OUString aLibName = OUString::createFromAscii("cpld");


    oslModule lib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_LAZY | SAL_LOADMODULE_GLOBAL );

    jobject joSLL_cpp;

    if (lib)
    {
        void * pSym;

        // ========================= LATEST VERSION =========================
        OUString aGetEnvName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETENV) );
        if (pSym = osl_getSymbol( lib, aGetEnvName.pData ))
        {
            uno_Environment * pJavaEnv = 0;
            uno_Environment * pLoaderEnv = 0;
            const sal_Char * pEnvTypeName = 0;

            (*((component_getImplementationEnvironmentFunc)pSym))( &pEnvTypeName, &pLoaderEnv );
            OUString aEnvTypeName( OUString::createFromAscii( pEnvTypeName ) );

            if (! pLoaderEnv)
                uno_getEnvironment( &pLoaderEnv, aEnvTypeName.pData, 0 );

            JavaVM * pJVM;

            pJEnv->GetJavaVM( &pJVM );

            JavaVMContext * pVMContext = new JavaVMContext(pJVM);
            uno_getEnvironment(&pJavaEnv, OUString::createFromAscii("java").pData, pVMContext);

            OUString aGetFactoryName( RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY) );

            if (pSym = osl_getSymbol( lib, aGetFactoryName.pData ))
            {
                if (pJavaEnv && pLoaderEnv)
                {
                    Mapping java2dest( pJavaEnv, pLoaderEnv );
                    Mapping dest2java( pLoaderEnv, pJavaEnv );

                    if (dest2java.is() && java2dest.is())
                    {
                        typelib_InterfaceTypeDescription * pType = 0;

                        getCppuType((Reference< XMultiServiceFactory > *) 0).getDescription((typelib_TypeDescription **) & pType);
                        void * pSMgr  = java2dest.mapInterface(jSMgr, pType);

                        getCppuType((Reference< XRegistryKey > *) 0).getDescription((typelib_TypeDescription **) & pType);
                        void * pKey = java2dest.mapInterface(jRegKey, pType);

                        const char* pImplName = pJEnv->GetStringUTFChars( jImplName, NULL );

                        //com.sun.star.comp.stoc.DLLComponentLoader
                        //void * pSSF = (*((component_getFactoryFunc)pSym))( "com.sun.star.comp.stoc.DLLComponentLoader", pSMgr, pKey );
                        void * pSSF = (*((component_getFactoryFunc)pSym))( pImplName, pSMgr, pKey );

                        pJEnv->ReleaseStringUTFChars( jImplName, pImplName );

                        if (pKey && pLoaderEnv->pExtEnv)
                            (*pLoaderEnv->pExtEnv->releaseInterface)( pLoaderEnv->pExtEnv, pKey );
                        if (pSMgr && pLoaderEnv->pExtEnv)
                            (*pLoaderEnv->pExtEnv->releaseInterface)( pLoaderEnv->pExtEnv, pSMgr );

                        if (pSSF)
                        {
                            typelib_InterfaceTypeDescription * pXSharedLibraryLoader_Type = 0;
                            getCppuType((Reference< XServiceInfo > *) 0).getDescription((typelib_TypeDescription **) & pXSharedLibraryLoader_Type);

                            pXSharedLibraryLoader_Type = 0;
                            getCppuType((Reference< XSingleServiceFactory > *) 0).getDescription((typelib_TypeDescription **) & pXSharedLibraryLoader_Type);

                            joSLL_cpp = (jobject) dest2java.mapInterface( pSSF, getCppuType((Reference< XSingleServiceFactory > *) 0));

                            /*
                            XSingleServiceFactory * pRet = (XSingleServiceFactory *)
                                aDest2Java.mapInterface(
                                    pSSF, ::getCppuType( (const Reference< XSingleServiceFactory > *)0 ) );
                            if (pRet)
                            {
                                xRet = pRet;
                                pRet->release();
                            }
                            */
                            if (pLoaderEnv->pExtEnv)
                            (*pLoaderEnv->pExtEnv->releaseInterface)( pLoaderEnv->pExtEnv, pSSF );
                        }
                        else
                        {
                            sMessage = OUString::createFromAscii("got no factory from component \"");
                            sMessage += aLibName;
                            sMessage += OUString::createFromAscii("\"!");
                        }
                    }
                }
            } else
            {
                sMessage = OUString::createFromAscii("symbol \"");
                sMessage += aGetFactoryName;
                sMessage += OUString::createFromAscii("\" could not be found in \"");
                sMessage += aLibName;
                sMessage += OUString::createFromAscii("\"");
            }

            if (pLoaderEnv)
                (*pLoaderEnv->release)( pLoaderEnv );
            if (pJavaEnv)
                (*pJavaEnv->release)( pJavaEnv );
        }
    }
    return joSLL_cpp;
}

/*
 * Class:     com_sun_star_comp_helper_RegistryServiceFactory
 * Method:    createRegistryServiceFactory
 * Signature: (Ljava/lang/String;Ljava/lang/String;Z)Ljava/lang/Object;
 */
extern "C" JNIEXPORT jobject JNICALL Java_com_sun_star_comp_helper_RegistryServiceFactory_createRegistryServiceFactory
    (JNIEnv * pJEnv, jclass jClass, jstring jWriteRegFile, jstring jReadRegFile, jboolean jbReadOnly )
{
    uno_Environment * pJavaEnv = 0;
    uno_Environment * pCurrEnv = 0;

    JavaVMContext * pVMContext = NULL;
    JavaVM * pJVM;
    pJEnv->GetJavaVM(&pJVM);
    jobject joGlobalRegServiceFac = 0;
    jobject joRegServiceFac = 0;

    try {
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

        Reference<XMultiServiceFactory> rMSFac;
        if (aReadRegFile.getLength() == 0)
            rMSFac = createRegistryServiceFactory( aWriteRegFile, bReadOnly);

        else
            rMSFac = createRegistryServiceFactory(aWriteRegFile, aReadRegFile, bReadOnly);


        JavaVMContext * pVMContext = NULL;

        // possible race ?
        { // get the java environment
            uno_Environment ** ppEnviroments = NULL;
            sal_Int32 size = 0;
            OUString java(RTL_CONSTASCII_USTRINGPARAM("java"));

            uno_getRegisteredEnvironments(&ppEnviroments, &size, (uno_memAlloc)malloc, java.pData);
            if(size) { // did we find an existing java environment?
                OSL_TRACE("javaunohelper.cxx: RegistryServiceFactory.createRegistryServiceFactory: found an existing java environment");

                pJavaEnv = ppEnviroments[0];
                pVMContext = (JavaVMContext *)pJavaEnv->pContext;

                for(sal_Int32 i = 1; i < size; ++ i)
                    ppEnviroments[i]->release(ppEnviroments[i]);

                free(ppEnviroments);
            }
            else { // no, create one
                pVMContext = new JavaVMContext(pJVM);

                uno_getEnvironment(&pJavaEnv, java.pData, pVMContext);
                if(!pJavaEnv) throw RuntimeException();
            }
        }



        pVMContext->registerThread();

        OUString aCurrentEnv(RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME));
        uno_getEnvironment(&pCurrEnv, aCurrentEnv.pData, NULL);
        if(!pCurrEnv) throw RuntimeException();

        Mapping curr_java(pCurrEnv, pJavaEnv);
        if (!curr_java.is()) throw RuntimeException();

        pJavaEnv->release(pJavaEnv);
        pJavaEnv = NULL;

        pCurrEnv->release(pCurrEnv);
        pCurrEnv = NULL;


        joGlobalRegServiceFac = (jobject)curr_java.mapInterface(rMSFac.get(), getCppuType((Reference<XMultiServiceFactory > *)0));
        if(!joGlobalRegServiceFac) throw RuntimeException();

        // we need a local reference have to delete the global reference to the mapped object
        jclass jcObject = pJEnv->FindClass("java/lang/Object");
        if(pJEnv->ExceptionOccurred()) throw RuntimeException();

        jobjectArray jaTmp = pJEnv->NewObjectArray(1, jcObject, joGlobalRegServiceFac);
        if(pJEnv->ExceptionOccurred()) throw RuntimeException();

        // delete the global reference
          pJEnv->DeleteGlobalRef(joGlobalRegServiceFac);
          joGlobalRegServiceFac = 0;

        // get the local reference
        joRegServiceFac = pJEnv->GetObjectArrayElement(jaTmp, 0);
        if(pJEnv->ExceptionOccurred()) throw RuntimeException();


        pVMContext->revokeThread();
        pVMContext = NULL;
    }
    catch (RuntimeException & runtimeException) {
        if(joGlobalRegServiceFac)
            pJEnv->DeleteGlobalRef(joGlobalRegServiceFac);

        if(pVMContext)
            pVMContext->revokeThread();

        if (pJavaEnv)
            (*pJavaEnv->release)(pJavaEnv);

        if (pCurrEnv)
            (*pCurrEnv->release)(pCurrEnv);
    }

    OSL_TRACE("javaunohelper.cxx: object %i", joRegServiceFac);

    return joRegServiceFac;
}
