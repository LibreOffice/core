/*************************************************************************
 *
 *  $RCSfile: javaloader.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:34 $
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

#include <string.h>

#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <rtl/ustring>
#include <rtl/process.h>

#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/mapping.h>
#include <uno/mapping.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/uno/Reference.hxx>

#ifdef LINUX
#undef minor
#undef major
#endif
#include <com/sun/star/corba/giop/MessageHeader_1_1.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/bridge/XInstanceProvider.hpp>

#include <com/sun/star/connection/XConnection.hpp>

#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <bridges/remote/context.h>
#include <bridges/remote/connection.h>
#include <bridges/remote/remote.h>

#include "jni.h"

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include "jthreadpool.hxx"

using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::java;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::loader;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::io;

using namespace ::cppu;
using namespace ::rtl;

namespace loader {
    class JVMThreadAttach {
        JavaVM * _pJavaVM;
        Reference<XJavaThreadRegister_11> _xJavaThreadRegister_11;

    public:
        JNIEnv * _pJNIEnv;

        JVMThreadAttach(JavaVM * pJavaVM, XJavaThreadRegister_11 * pXJavaThreadRegister_11) throw (RuntimeException);
        ~JVMThreadAttach() throw (RuntimeException);
    };

    JVMThreadAttach::JVMThreadAttach(JavaVM * pJavaVM, XJavaThreadRegister_11 * pXJavaThreadRegister_11) throw (RuntimeException)
        : _pJNIEnv(NULL),
          _pJavaVM(pJavaVM),
          _xJavaThreadRegister_11(pXJavaThreadRegister_11)
    {
        pXJavaThreadRegister_11->registerThread();
        pJavaVM->AttachCurrentThread(&_pJNIEnv, NULL);
    }

    JVMThreadAttach::~JVMThreadAttach() throw (RuntimeException)
    {
        _xJavaThreadRegister_11->revokeThread();
        if(!_xJavaThreadRegister_11->isThreadAttached())
            _pJavaVM->DetachCurrentThread();
    }



    struct ConnectionWrapper : public remote_Connection {
        sal_Int32                _refCount;

        JavaVM *                 _pJavaVM;
        XJavaThreadRegister_11 * _pXJavaThreadRegister_11;

        jclass                   _jcByteArray;
        jmethodID                _jmConnection_read;
        jmethodID                _jmConnection_write;
        jmethodID                _jmConnection_flush;
        jmethodID                _jmConnection_close;
        jobject                  _joConnection;

        ConnectionWrapper(JavaVM * pJavaVM, XJavaThreadRegister_11 * pXJavaThreadRegister_11, jobject joConnection) throw(RuntimeException);
        ~ConnectionWrapper() throw(RuntimeException);

    };

    void SAL_CALL ConnectionWrapper_acquire(remote_Connection * blb) {
        ++ ((ConnectionWrapper *)blb)->_refCount;
    }

    void SAL_CALL ConnectionWrapper_release (remote_Connection * blb) {
        if(!-- ((ConnectionWrapper *)blb)->_refCount)
            delete (ConnectionWrapper *)blb;
    }

    sal_Int32 SAL_CALL ConnectionWrapper_read(remote_Connection * blb, sal_Int8 *pDest, sal_Int32 nSize) {
        ConnectionWrapper * pC = (ConnectionWrapper *)blb;
        jint jiRes = -1;

        try {
            JVMThreadAttach jvm(pC->_pJavaVM, pC->_pXJavaThreadRegister_11);

            try {
                jobjectArray joDest = jvm._pJNIEnv->NewObjectArray(1, pC->_jcByteArray, 0);                                       if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 1"), Reference<XInterface>());
                jvm._pJNIEnv->SetObjectArrayElement(joDest, 0, jvm._pJNIEnv->NewByteArray(10));                                   if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 2"), Reference<XInterface>());

                jiRes = jvm._pJNIEnv->CallIntMethod(pC->_joConnection, pC->_jmConnection_read, joDest, (jint)nSize);              if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 3"), Reference<XInterface>());

                jbyteArray jaDest = reinterpret_cast<jbyteArray>(jvm._pJNIEnv->GetObjectArrayElement(joDest, 0));                 if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 4"), Reference<XInterface>());

                jbyte * pBytes = jvm._pJNIEnv->GetByteArrayElements(jaDest, NULL);                                                if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 5"), Reference<XInterface>());
                memcpy(pDest, pBytes, jiRes * sizeof(sal_Int8));
                jvm._pJNIEnv->ReleaseByteArrayElements(jaDest, pBytes, 0);                                                        if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 6"), Reference<XInterface>());
            }
            catch(RuntimeException & runtimeException) {
                if(jvm._pJNIEnv->ExceptionOccurred()) {
                    jvm._pJNIEnv->ExceptionDescribe();
                    jvm._pJNIEnv->ExceptionClear();
                }

                throw runtimeException;
            }
        }
        catch(RuntimeException &) {
            osl_trace("ConnectionWrapper_read - runtimeException occurred\n");
            jiRes = -1;
        }

        return (sal_Int32)jiRes;
    }

    sal_Int32 SAL_CALL ConnectionWrapper_write(remote_Connection * blb, const sal_Int8 *pSource, sal_Int32 nSize) {
        ConnectionWrapper * pC = (ConnectionWrapper *)blb;

        try {
            JVMThreadAttach jvm(pC->_pJavaVM, pC->_pXJavaThreadRegister_11);

            try {
                jbyteArray jaSource = jvm._pJNIEnv->NewByteArray(nSize);                                                       if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 7"), Reference<XInterface>());
                jbyte * pBytes = jvm._pJNIEnv->GetByteArrayElements(jaSource, NULL);                                           if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 8"), Reference<XInterface>());
                memcpy(pBytes, pSource, nSize * sizeof(jbyte));
                jvm._pJNIEnv->ReleaseByteArrayElements(jaSource, pBytes, 0);                                                   if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 9"), Reference<XInterface>());

                jvm._pJNIEnv->CallVoidMethod(pC->_joConnection, pC->_jmConnection_write, jaSource, (jint)nSize);               if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 10"), Reference<XInterface>());
            }
            catch(RuntimeException & runtimeException) {
                if(jvm._pJNIEnv->ExceptionOccurred()) {
                    jvm._pJNIEnv->ExceptionDescribe();
                    jvm._pJNIEnv->ExceptionClear();
                }

                throw runtimeException;
            }
        }
        catch(RuntimeException &) {
            osl_trace("ConnectionWrapper_read - runtimeException occurred\n");
            nSize = -1;
        }

        return nSize;
    }

    void SAL_CALL ConnectionWrapper_flush(remote_Connection * blb) {
        ConnectionWrapper * pC = (ConnectionWrapper *)blb;

        JVMThreadAttach jvm(pC->_pJavaVM, pC->_pXJavaThreadRegister_11);
        jvm._pJNIEnv->CallVoidMethod(pC->_joConnection, pC->_jmConnection_flush);
        if(jvm._pJNIEnv->ExceptionOccurred()) {
            jvm._pJNIEnv->ExceptionDescribe();
            jvm._pJNIEnv->ExceptionClear();
        }
    }

    void SAL_CALL ConnectionWrapper_close(remote_Connection * blb) {
        ConnectionWrapper * pC = (ConnectionWrapper *)blb;

        JVMThreadAttach jvm(pC->_pJavaVM, pC->_pXJavaThreadRegister_11);
        jvm._pJNIEnv->CallVoidMethod(pC->_joConnection, pC->_jmConnection_close);
        if(jvm._pJNIEnv->ExceptionOccurred()) {
            jvm._pJNIEnv->ExceptionDescribe();
            jvm._pJNIEnv->ExceptionClear();
        }
    }

    ConnectionWrapper::ConnectionWrapper(JavaVM * pJavaVM, XJavaThreadRegister_11 * pXJavaThreadRegister_11, jobject joConnection) throw(RuntimeException)
        : _refCount(0),
          _pJavaVM(pJavaVM),
          _pXJavaThreadRegister_11(pXJavaThreadRegister_11)
    {
        if(!pJavaVM || !pXJavaThreadRegister_11 || !joConnection)
            throw RuntimeException(OUString::createFromAscii("hier 11"), Reference<XInterface>());

        acquire = ConnectionWrapper_acquire;
        release = ConnectionWrapper_release;
        read = ConnectionWrapper_read;
        write = ConnectionWrapper_write;
        flush = ConnectionWrapper_flush;
        close = ConnectionWrapper_close;

        JVMThreadAttach jvm(pJavaVM, pXJavaThreadRegister_11);

        _jcByteArray = jvm._pJNIEnv->FindClass("[B");                                                                          if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 12"), Reference<XInterface>());
        _jcByteArray = reinterpret_cast<jclass>(jvm._pJNIEnv->NewGlobalRef(reinterpret_cast<jobject>(_jcByteArray)));          if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 13"), Reference<XInterface>());

        jclass jcConnection = jvm._pJNIEnv->FindClass("com/sun/star/connection/XConnection");                                  if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 14"), Reference<XInterface>());

        _jmConnection_read  = jvm._pJNIEnv->GetMethodID(jcConnection, "read", "([[BI)I");                                      if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 15"), Reference<XInterface>());
//          _jmConnection_read  = reinterpret_cast<jmethodID>(jvm._pJNIEnv->NewGlobalRef(reinterpret_cast<jobject>(_jmConnection_read)));
        _jmConnection_write = jvm._pJNIEnv->GetMethodID(jcConnection, "write", "([B)V");                                       if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 16"), Reference<XInterface>());
//          _jmConnection_write = reinterpret_cast<jmethodID>(jvm._pJNIEnv->NewGlobalRef(reinterpret_cast<jobject>(_jmConnection_write)));
        _jmConnection_flush = jvm._pJNIEnv->GetMethodID(jcConnection, "flush", "()V");                                         if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 17"), Reference<XInterface>());
//          _jmConnection_flush = reinterpret_cast<jmethodID>(jvm._pJNIEnv->NewGlobalRef(reinterpret_cast<jobject>(_jmConnection_flush)));
        _jmConnection_close = jvm._pJNIEnv->GetMethodID(jcConnection, "close", "()V");                                         if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 18"), Reference<XInterface>());
//          _jmConnection_close = reinterpret_cast<jmethodID>(jvm._pJNIEnv->NewGlobalRef(reinterpret_cast<jobject>(_jmConnection_close)));

        _joConnection = jvm._pJNIEnv->NewGlobalRef(joConnection);                                                              if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 19"), Reference<XInterface>());
    }

    ConnectionWrapper::~ConnectionWrapper() throw(RuntimeException) {
        JVMThreadAttach jvm(_pJavaVM, _pXJavaThreadRegister_11);

        jvm._pJNIEnv->DeleteGlobalRef(_joConnection);                                                                          if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 20"), Reference<XInterface>());

        jvm._pJNIEnv->DeleteGlobalRef(reinterpret_cast<jobject>(_jmConnection_read));                                          if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 21"), Reference<XInterface>());
        jvm._pJNIEnv->DeleteGlobalRef(reinterpret_cast<jobject>(_jmConnection_write));                                         if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 22"), Reference<XInterface>());
        jvm._pJNIEnv->DeleteGlobalRef(reinterpret_cast<jobject>(_jmConnection_flush));                                         if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 23"), Reference<XInterface>());
        jvm._pJNIEnv->DeleteGlobalRef(reinterpret_cast<jobject>(_jmConnection_close));                                         if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 24"), Reference<XInterface>());

        jvm._pJNIEnv->DeleteGlobalRef(_jcByteArray);                                                                           if(jvm._pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 25"), Reference<XInterface>());
    }




    class JavaComponentLoader : public WeakImplHelper2<XImplementationLoader, XServiceInfo> {
        Reference<XMultiServiceFactory>  _xSMgr;
        Reference<XImplementationLoader> _javaLoader;

        uno_Environment                * _pRemote_Environment;
    protected:
        JavaComponentLoader(const Reference<XMultiServiceFactory> & rXSMgr) throw(RuntimeException);
        ~JavaComponentLoader();

    public:
        static const OUString implname;
        static const OUString servname;
        static Reference<XInterface> CreateInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception);
        static Sequence<OUString> SAL_CALL getSupportedServiceNames_Static();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(::com::sun::star::uno::RuntimeException);
        virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XImplementationLoader
        virtual Reference<XInterface> SAL_CALL activate(const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey) throw(CannotActivateFactoryException, RuntimeException);
        virtual sal_Bool SAL_CALL writeRegistryInfo(const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl) throw(CannotRegisterImplementationException, RuntimeException);
    };

    const OUString JavaComponentLoader::implname = L"com.sun.star.comp.stoc.JavaComponentLoader";
    const OUString JavaComponentLoader::servname = L"com.sun.star.loader.Java2";

    Sequence<OUString> SAL_CALL JavaComponentLoader::getSupportedServiceNames_Static() {
        return Sequence<OUString>(&servname, 1);
    }


    Reference<XInterface> SAL_CALL JavaComponentLoader::CreateInstance(const Reference<XMultiServiceFactory> & rSMgr) throw(Exception)
    {
        Reference<XInterface> xRet;

        try {
            XImplementationLoader *pXLoader = (XImplementationLoader *)new JavaComponentLoader(rSMgr);

            xRet = Reference<XInterface>::query(pXLoader);
        }
        catch(RuntimeException & runtimeException) {
            osl_trace("Could not init javaloader\n");
            OString message = OUStringToOString(runtimeException.Message, RTL_TEXTENCODING_ASCII_US);
            fprintf(stderr, "exception: %s\n", (const char *)message);
        }

        return xRet;
    }

    JavaComponentLoader::JavaComponentLoader(const Reference<XMultiServiceFactory> & rSMgr) throw(RuntimeException)
        : _pRemote_Environment(NULL)
    {
        Reference<XJavaVM> xJavaVM(rSMgr->createInstance(OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine")), UNO_QUERY);

        Reference<XJavaThreadRegister_11> xJavaThreadRegister_11(xJavaVM, UNO_QUERY);

        JavaVM * pJavaVM;


        Sequence<sal_Int8> processID(16);
        rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8 *>(processID.getArray()));

        pJavaVM = *(JavaVM **)(xJavaVM->getJavaVM(processID).getValue());
//              pJavaVM = *reinterpret_cast<JavaVM **>(xJavaVM->getJavaVM(processID).getValue());

        if(!pJavaVM)
          throw RuntimeException(OUString::createFromAscii("hier -1"), Reference<XInterface>());

        JNIEnv * pJNIEnv;

          pJavaVM->AttachCurrentThread(&pJNIEnv, (void *)NULL);
          xJavaThreadRegister_11->registerThread();

        if( ! javaloader_initNativeThreadPool( pJNIEnv, xJavaThreadRegister_11 ) )
          {
              throw RuntimeException(
                  OUString::createFromAscii( "native threadpool couldn't be initialzed" ),
                  Reference< XInterface > () );
          }

        remote_Connection * pRemote_Connection = NULL;

        try {
          jclass jcServiceManager = pJNIEnv->FindClass("com/sun/star/comp/servicemanager/ServiceManager");                   if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 26"), Reference<XInterface>());
          jmethodID jmServiceManager_init = pJNIEnv->GetMethodID(jcServiceManager, "<init>", "()V");                         if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 27"), Reference<XInterface>());
          jobject joServiceManager = pJNIEnv->NewObject(jcServiceManager, jmServiceManager_init);                            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 28"), Reference<XInterface>());

          // we need to register the need services
          static const char * neededServices[] = {
              "com.sun.star.comp.servicemanager.ServiceManager",
              "com.sun.star.comp.loader.JavaLoader",
              "com.sun.star.lib.uno.bridges.java_remote.java_remote_bridge",
              "com.sun.star.comp.connections.PipedConnection",
              "com.sun.star.comp.connections.ConstantInstanceProvider"
          };


          jclass jcString = pJNIEnv->FindClass("java/lang/String");                                                          if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 29"), Reference<XInterface>());
          jobjectArray jaNeededServices = pJNIEnv->NewObjectArray(sizeof(neededServices) / sizeof(char *), jcString, NULL);  if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 30"), Reference<XInterface>());
          for(int i = 0; i < sizeof(neededServices) / sizeof(char *); ++ i) {
              pJNIEnv->SetObjectArrayElement(jaNeededServices, i, pJNIEnv->NewStringUTF(neededServices[i]));                 if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 31"), Reference<XInterface>());
          }

          jmethodID jmServiceManager_addFactories = pJNIEnv->GetMethodID(jcServiceManager,
                                                                         "addFactories",
                                                                         "([Ljava/lang/String;)V");                          if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 32"), Reference<XInterface>());

          pJNIEnv->CallVoidMethod(joServiceManager, jmServiceManager_addFactories, jaNeededServices);                        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 33"), Reference<XInterface>());

        //
          jmethodID jmServiceManager_createInstanceWithArguments = pJNIEnv->GetMethodID(jcServiceManager,
                                                                                        "createInstanceWithArguments",
                                                                                        "(Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;"); if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 34"), Reference<XInterface>());

          // create an instance of our special inprocess connection
          jmethodID jmServiceManager_createInstance = pJNIEnv->GetMethodID(jcServiceManager,
                                                                           "createInstance",
                                                                           "(Ljava/lang/String;)Ljava/lang/Object;");        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 35"), Reference<XInterface>());

          jstring jsConnectionName = pJNIEnv->NewStringUTF("com.sun.star.connection.PipedConnection");                     if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 36"), Reference<XInterface>());
          jobject joConnection_nativeSide = pJNIEnv->CallObjectMethod(joServiceManager,
                                                                      jmServiceManager_createInstance,
                                                                      jsConnectionName);                                     if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 37"), Reference<XInterface>());

          jclass jcObject = pJNIEnv->FindClass("java/lang/Object");                                                        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 38"), Reference<XInterface>());
          jobjectArray jaArgs = pJNIEnv->NewObjectArray(1, jcObject, joConnection_nativeSide);
          jobject joConnection_javaSide = pJNIEnv->CallObjectMethod(joServiceManager,
                                                                    jmServiceManager_createInstanceWithArguments,
                                                                    jsConnectionName, jaArgs);                             if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 39"), Reference<XInterface>());

          // create an constant service provider with java servicemanager
          jstring jsInstanceProvider = pJNIEnv->NewStringUTF("com.sun.star.comp.connection.InstanceProvider");             if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 40"), Reference<XInterface>());
          jobject joInstanceProvider = pJNIEnv->CallObjectMethod(joServiceManager,
                                                                 jmServiceManager_createInstance,
                                                                 jsInstanceProvider);                                          if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 41"), Reference<XInterface>());

          // create the bridge factory && the bridge
          jaArgs = pJNIEnv->NewObjectArray(3, jcObject, NULL);                                                             if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 42"), Reference<XInterface>());

          pJNIEnv->SetObjectArrayElement(jaArgs, 0, pJNIEnv->NewStringUTF("iiop"));                                        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 43"), Reference<XInterface>());
          pJNIEnv->SetObjectArrayElement(jaArgs, 1, joConnection_javaSide);                                                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 44"), Reference<XInterface>());
          pJNIEnv->SetObjectArrayElement(jaArgs, 2, joInstanceProvider);                                                       if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 45"), Reference<XInterface>());

          jstring jsBridgeComponent = pJNIEnv->NewStringUTF("com.sun.star.bridge.Bridge");                                 if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 46"), Reference<XInterface>());
          jobject joBridge = pJNIEnv->CallObjectMethod(joServiceManager,
                                                       jmServiceManager_createInstanceWithArguments,
                                                       jsBridgeComponent,
                                                       jaArgs);                                                            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 47"), Reference<XInterface>());

          pRemote_Connection = new ConnectionWrapper(pJavaVM, xJavaThreadRegister_11.get(), joConnection_nativeSide);
        }
        catch(RuntimeException & runtimeException) {
          if(pJNIEnv->ExceptionOccurred()) {
            pJNIEnv->ExceptionDescribe();
            pJNIEnv->ExceptionClear();
          }

          throw runtimeException;
        }

        xJavaThreadRegister_11->revokeThread();
        if(!xJavaThreadRegister_11->isThreadAttached())
            pJavaVM->DetachCurrentThread();


        //
        // everything is prepared to map an initial object from remote (java) to here
        //
        OString idStr("the id string");
        OString dcpStr("the description string");
        remote_Context * pRemote_Context = remote_createContext(pRemote_Connection, idStr.pData, dcpStr.pData, NULL);      if(!pRemote_Connection) throw RuntimeException(OUString::createFromAscii("hier 48"), Reference<XInterface>());

        uno_getEnvironment(&_pRemote_Environment, "remote", pRemote_Context);                                              if(!_pRemote_Environment) throw RuntimeException(OUString::createFromAscii("hier 49"), Reference<XInterface>());

        uno_Environment * pUNO_Environment = NULL;
        uno_getEnvironment(&pUNO_Environment, "uno", NULL);                                                                if(!pUNO_Environment) throw RuntimeException(OUString::createFromAscii("hier 50"), Reference<XInterface>());

        Mapping mapping(_pRemote_Environment, pUNO_Environment, OString());



        OString remoteO("SERVICEMANAGER");

        typelib_InterfaceTypeDescription *pType = 0;
        getCppuType((Reference <XInterface> *) 0).getDescription((typelib_TypeDescription **) & pType) ;                   if(!pType) throw RuntimeException(OUString::createFromAscii("hier 51"), Reference<XInterface>());

        remote_Interface *pRemoteI = 0;

        getCppuType( (::com::sun::star::corba::giop::MessageHeader_1_1*)0 );


        pRemote_Context->getRemoteInstance(_pRemote_Environment,
                                           &pRemoteI,
                                           remoteO.pData,
                                           pType);                                                                         if(!pRemoteI) throw RuntimeException(OUString::createFromAscii("hier 52"), Reference<XInterface>());

        // got an interface !
        uno_Environment *pEnvCpp =0;
        uno_getEnvironment(&pEnvCpp , CPPU_CURRENT_LANGUAGE_BINDING_NAME , 0);                                             if(!pEnvCpp) throw RuntimeException(OUString::createFromAscii("hier 53"), Reference<XInterface>());

        Mapping map(_pRemote_Environment , pEnvCpp);

        XInterface * pCppI = (XInterface *) map.mapInterface(pRemoteI, pType);                                             if(!pCppI) throw RuntimeException(OUString::createFromAscii("hier 54"), Reference<XInterface>());

        Reference<XInterface> rReturn = Reference<XInterface>(pCppI);

        Reference<XMultiServiceFactory> serviceManager(rReturn, UNO_QUERY);                                                if(!serviceManager.is()) throw RuntimeException(OUString::createFromAscii("hier 55"), Reference<XInterface>());

        Reference<XInterface> javaLoader_xInterface =
            serviceManager->createInstance(OUString::createFromAscii("com.sun.star.loader.Java"));                         if(!javaLoader_xInterface.is()) throw RuntimeException(OUString::createFromAscii("hier 56"), Reference<XInterface>());
        _javaLoader = Reference<XImplementationLoader>(javaLoader_xInterface, UNO_QUERY);                                  if(!_javaLoader.is()) throw RuntimeException(OUString::createFromAscii("hier 57"), Reference<XInterface>());
        _javaLoader->acquire();

        Reference<XInitialization> javaLoader_xInit(javaLoader_xInterface, UNO_QUERY);

        Any any_smgr;
        any_smgr <<= rSMgr;
        javaLoader_xInit->initialize(Sequence<Any>(&any_smgr, 1));


        // important: release the context and the environments

        pCppI->release();
        pEnvCpp->release(pEnvCpp);
        pRemoteI->release(pRemoteI);

          pUNO_Environment->release(pUNO_Environment);
          pRemote_Context->aBase.release(reinterpret_cast<uno_Context *>(pRemote_Context));
    }


    JavaComponentLoader::~JavaComponentLoader() {
        if(_pRemote_Environment)
        {
            _pRemote_Environment->dispose(_pRemote_Environment);
            _pRemote_Environment->release(_pRemote_Environment);
        }

//          JVMThreadAttach jvm(pC->_pJavaVM, pC->_pXJavaThreadRegister_11);

//          try {
//              jclass jcThreadPool = jvm._pJNIEnv->FindClass("com/sun/star/lib/uno/environments/remote/ThreadPool");             if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 1"), Reference<XInterface>());
//              jmethodID jmThreadPool_reset = jvm._pJNIEnv->GetMethodID(jcThreadPool, "reset", "()V");                           if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString::createFromAscii("hier 1"), Reference<XInterface>());

//              jvm._pJNIEnv->CallStaticVoidMethod(jcThreadPool, jmThreadPool_reset);
//          }
//          catch(
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
        sal_Bool bSuccess = sal_False;

        bSuccess = _javaLoader->writeRegistryInfo(xKey, blabla, rLibName);

        return bSuccess;
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

