D/*************************************************************************
 *
 *  $RCSfile: javaloader.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2000-10-26 11:59:59 $
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


#include <cstdarg>
#include <osl/diagnose.h>

#include <rtl/ustring>
#include <rtl/process.h>

#include <uno/environment.h>
#include <uno/mapping.hxx>

#include <cppuhelper/servicefactory.hxx>

#ifdef LINUX
#undef minor
#undef major
#endif

#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "jni.h"

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>


using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::loader;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;

using namespace ::cppu;
using namespace ::rtl;

namespace loader {
    class JavaComponentLoader : public WeakImplHelper2<XImplementationLoader, XServiceInfo> {
        Reference<XMultiServiceFactory>  _xSMgr;
        Reference<XImplementationLoader> _javaLoader;

    protected:
        JavaComponentLoader(const Reference<XMultiServiceFactory> & rXSMgr) throw(RuntimeException);
        virtual ~JavaComponentLoader() throw();

    public:
        static const OUString implname;
        static const OUString servname;

        static Reference<XInterface> SAL_CALL CreateInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception);
        static Sequence<OUString>    SAL_CALL getSupportedServiceNames_Static() throw();

        // XServiceInfo
        virtual OUString           SAL_CALL getImplementationName()                      throw(RuntimeException);
        virtual sal_Bool           SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
        virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()                   throw(RuntimeException);

        // XImplementationLoader
        virtual Reference<XInterface> SAL_CALL activate(const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey) throw(CannotActivateFactoryException, RuntimeException);
        virtual sal_Bool SAL_CALL writeRegistryInfo(const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl) throw(CannotRegisterImplementationException, RuntimeException);
    };

    const OUString JavaComponentLoader::implname = OUString::createFromAscii("com.sun.star.comp.stoc.JavaComponentLoader");
    const OUString JavaComponentLoader::servname = OUString::createFromAscii("com.sun.star.loader.Java2");

    Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames_Static() throw() {
        return Sequence<OUString>(&servname, 1);
    }


    Reference<XInterface> SAL_CALL JavaComponentLoader::CreateInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception) {
        Reference<XInterface> xRet;

        try {
            XImplementationLoader *pXLoader = (XImplementationLoader *)new JavaComponentLoader(rSMgr);

            xRet = Reference<XInterface>::query(pXLoader);
        }
        catch(RuntimeException & runtimeException) {
            OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
            osl_trace("javaloader - could not init javaloader cause of %s", message.getStr());
        }

        return xRet;
    }

    JavaComponentLoader::JavaComponentLoader(const Reference<XMultiServiceFactory> & rSMgr) throw(RuntimeException)
        : _xSMgr(rSMgr)
    {
        sal_Int32 size = 0;
        uno_Environment ** ppJava_environments = NULL;
        uno_Environment * pUno_environment = NULL;
        typelib_InterfaceTypeDescription * pType_XImplementationLoader = 0;
        JNIEnv * pJNIEnv = NULL;

        Reference<XJavaThreadRegister_11> xJavaThreadRegister_11;
        Reference<XInterface> javaVM;
        JavaVM * pJavaVM = NULL;

        try {
            // get a java vm, where we can create a loader
            javaVM = rSMgr->createInstance(OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine"));
            if(!javaVM.is()) throw RuntimeException(OUString::createFromAscii("javaloader error - 10"), Reference<XInterface>());
            Reference<XJavaVM>    javaVM_xJavaVM(javaVM, UNO_QUERY);
            if(!javaVM_xJavaVM.is()) throw RuntimeException(OUString::createFromAscii("javaloader error - 11"), Reference<XInterface>());

            Sequence<sal_Int8> processID(16);
            rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8 *>(processID.getArray()));

            if(javaVM_xJavaVM->getJavaVM(processID).hasValue())
                pJavaVM = *(JavaVM **)(javaVM_xJavaVM->getJavaVM(processID).getValue());
            if(!pJavaVM) throw RuntimeException(OUString::createFromAscii("javaloader error - 12"), Reference<XInterface>());

            xJavaThreadRegister_11 = Reference<XJavaThreadRegister_11>(javaVM, UNO_QUERY);
            if(xJavaThreadRegister_11.is())
                xJavaThreadRegister_11->registerThread();

            pJavaVM->AttachCurrentThread((void **)&pJNIEnv, (void *)NULL);


            // instantiate the java JavaLoader
            jclass jcJavaLoader = pJNIEnv->FindClass("com/sun/star/comp/loader/JavaLoader");     if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("error - 26"), Reference<XInterface>());
            jmethodID jmJavaLoader_init = pJNIEnv->GetMethodID(jcJavaLoader, "<init>", "()V");   if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("error - 27"), Reference<XInterface>());
            jobject joJavaLoader = pJNIEnv->NewObject(jcJavaLoader, jmJavaLoader_init);          if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("error - 28"), Reference<XInterface>());


            // map the java JavaLoader to this environment
            OUString sJava(RTL_CONSTASCII_USTRINGPARAM("java"));

            uno_getRegisteredEnvironments(&ppJava_environments, &size, (uno_memAlloc)malloc, sJava.pData);
            if(!size) throw RuntimeException(OUString::createFromAscii("javaloader error - 1"), Reference<XInterface>());

            // why is there no convinient contructor?
            OUString sCppu_current_lb_name(RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME));
            uno_getEnvironment(&pUno_environment, sCppu_current_lb_name.pData, NULL);
            if(!pUno_environment) throw RuntimeException(OUString::createFromAscii("javaloader error - 2"), Reference<XInterface>());

            Mapping java_curr(ppJava_environments[0], pUno_environment);
            if(!java_curr.is()) throw RuntimeException(OUString::createFromAscii("javaloader error - 3"), Reference<XInterface>());

            // release all java environments
            for(sal_Int32 i = 0; i < size; ++ i)
                ppJava_environments[i]->release(ppJava_environments[i]);
            size = 0;
            free(ppJava_environments);

            // release uno environment
            pUno_environment->release(pUno_environment);
            pUno_environment = NULL;

            getCppuType((Reference<XImplementationLoader> *) 0).getDescription((typelib_TypeDescription **) & pType_XImplementationLoader);
            if(!pType_XImplementationLoader) throw RuntimeException(OUString::createFromAscii("javaloader error - 4"), Reference<XInterface>());

            _javaLoader = Reference<XImplementationLoader>(reinterpret_cast<XImplementationLoader *>(java_curr.mapInterface(joJavaLoader, pType_XImplementationLoader)));
            if(!_javaLoader.is()) throw RuntimeException(OUString::createFromAscii("javaloader error - 13"), Reference<XInterface>());

            typelib_typedescription_release(reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));
            pType_XImplementationLoader = NULL;


            // detach from java vm
            if(xJavaThreadRegister_11.is()) {
                xJavaThreadRegister_11->revokeThread();

                if(!xJavaThreadRegister_11->isThreadAttached())
                    pJavaVM->DetachCurrentThread();
            }
            else
                pJavaVM->DetachCurrentThread();

            pJNIEnv = NULL;

            // set the service manager at the javaloader
            Reference<XInitialization> javaLoader_XInitialization(_javaLoader, UNO_QUERY);
            if(!javaLoader_XInitialization.is()) throw RuntimeException(OUString::createFromAscii("javaloader error - 14"), Reference<XInterface>());

            Any any;
            any <<= _xSMgr;

            javaLoader_XInitialization->initialize(Sequence<Any>(&any, 1));
        }
        catch(RuntimeException & runtimeException) {
            // release all java environments
            for(sal_Int32 i = 0; i < size; ++ i)
                ppJava_environments[i]->release(ppJava_environments[i]);

            if(pUno_environment)
                pUno_environment->release(pUno_environment);

            if(pType_XImplementationLoader)
                typelib_typedescription_release(reinterpret_cast<typelib_TypeDescription *>(pType_XImplementationLoader));

            // detach from java vm
              if(pJNIEnv)
                if(xJavaThreadRegister_11.is()) {
                    xJavaThreadRegister_11->revokeThread();

                    if(!xJavaThreadRegister_11->isThreadAttached())
                        pJavaVM->DetachCurrentThread();
                }
                else
                    pJavaVM->DetachCurrentThread();

            throw;
        }
        OSL_TRACE("javaloader.cxx: mapped javaloader - 0x%x", _javaLoader.get());
    }


    JavaComponentLoader::~JavaComponentLoader() throw() {
    }

    // XServiceInfo
    OUString SAL_CALL JavaComponentLoader::getImplementationName() throw(::com::sun::star::uno::RuntimeException) {
        return implname;
    }

    sal_Bool SAL_CALL JavaComponentLoader::supportsService(const OUString & ServiceName) throw(::com::sun::star::uno::RuntimeException) {
        sal_Bool bSupport = sal_False;

        Sequence<OUString> aSNL = getSupportedServiceNames();
        const OUString * pArray = aSNL.getArray();
        for(sal_Int32 i = 0; i < aSNL.getLength() && !bSupport; ++ i)
            bSupport = pArray[i] == ServiceName;

        return bSupport;
    }

    Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException) {
        return getSupportedServiceNames_Static();
    }



    // XImplementationLoader
    sal_Bool SAL_CALL JavaComponentLoader::writeRegistryInfo(const Reference<XRegistryKey> & xKey, const OUString & blabla, const OUString & rLibName)
        throw(CannotRegisterImplementationException, RuntimeException)
    {
        OSL_TRACE("JavaComponentLoader::writeRegistryInfo");
        return _javaLoader->writeRegistryInfo(xKey, blabla, rLibName);
    }


    Reference<XInterface> SAL_CALL JavaComponentLoader::activate(const OUString & rImplName,
                                                                 const OUString & blabla,
                                                                 const OUString & rLibName,
                                                                 const Reference<XRegistryKey> & xKey)
        throw(CannotActivateFactoryException, RuntimeException)
    {
        return _javaLoader->activate(rImplName, blabla, rLibName, xKey);
    }
}


extern "C"
{
    SAL_DLLEXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)   {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    SAL_DLLEXPORT sal_Bool SAL_CALL component_writeInfo(XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey) {
        sal_Bool bRes = sal_False;

        if (pRegistryKey) {
            try {
                OUString x = OUString::createFromAscii("/");
                x += ::loader::JavaComponentLoader::implname;
                x += OUString::createFromAscii("/UNO/SERVICES");

                Reference<XRegistryKey> xNewKey(pRegistryKey->createKey(x));

                const Sequence<OUString> rSNL = ::loader::JavaComponentLoader::getSupportedServiceNames_Static();
                const OUString * pArray = rSNL.getConstArray();
                for (sal_Int32 nPos = rSNL.getLength(); nPos--;)
                    xNewKey->createKey(pArray[nPos]);

                bRes = sal_True;
            }
            catch (InvalidRegistryException &) {
                OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
            }
        }

        return bRes;
    }

    SAL_DLLEXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName, XMultiServiceFactory * pServiceManager, XRegistryKey * pRegistryKey) {
        void * pRet = 0;


        OString xx(::loader::JavaComponentLoader::implname.getStr(), ::loader::JavaComponentLoader::implname.getLength(), RTL_TEXTENCODING_DONTKNOW);
        if (pServiceManager && rtl_str_compare(pImplName, xx) == 0)
        {
            Reference<XSingleServiceFactory> xFactory(createOneInstanceFactory(pServiceManager,
                                                                               OUString::createFromAscii(pImplName),
                                                                               ::loader::JavaComponentLoader::CreateInstance,
                                                                               ::loader::JavaComponentLoader::getSupportedServiceNames_Static()));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }

        return pRet;
    }
}
