/*************************************************************************
 *
 *  $RCSfile: javavm.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: jl $ $Date: 2002-08-08 10:39:53 $
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

#ifdef UNX
#include <signal.h>
#endif
#include <setjmp.h>
#include <string.h>
#include <time.h>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>
#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <bridges/java/jvmcontext.hxx>
#ifndef _COM_SUN_STAR_TASK_XINTERACTION_ABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVADISABLEDEXCEPTION_HPP_
#include <com/sun/star/java/JavaDisabledException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_MISSINGJAVARUNTIMEEXCEPTION_HPP_
#include <com/sun/star/java/MissingJavaRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVAVMCREATIONFAILUREEXCEPTION_HPP_
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#endif
#ifndef _COM_SUN_STAR_JAVA_JAVANOTCONFIGUREDEXCEPTION_HPP_
#include <com/sun/star/java/JavaNotConfiguredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
#endif
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif
#include "jvmargs.hxx"
#include "javavm.hxx"
#include "interact.hxx"

// Properties of the javavm can be put
// as a komma separated list in this
// environment variable
#define PROPERTIES_ENV "OO_JAVA_PROPERTIES"
#ifdef UNIX
#define INI_FILE "javarc"
#ifdef MACOSX
#define DEF_JAVALIB "JavaVM.framework"
#else
#define DEF_JAVALIB "libjvm.so"
#endif
#define TIMEZONE "MEZ"
#else
#define INI_FILE "java.ini"
#define DEF_JAVALIB "jvm.dll"
#define TIMEZONE "MET"
#endif

#define OUSTR(x)    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(x))

#define INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::java;
using namespace com::sun::star::registry;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::task;
using namespace rtl;
using namespace cppu;
using namespace osl;

static void  abort_handler();
jmp_buf jmp_jvm_abort;
sal_Bool g_bInGetJavaVM;

namespace stoc_javavm {

static void getINetPropsFromConfig(JVM * pjvm,
                                   const Reference<XMultiComponentFactory> & xSMgr,
                                   const Reference<XComponentContext> &xCtx ) throw (Exception);


Mutex* javavm_getMutex()
{
    static Mutex* pMutex= NULL;

    if( ! pMutex)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pMutex)
        {
            static Mutex aMutex;
            pMutex= &aMutex;
        }
    }
    return pMutex;
}


static Sequence< OUString > javavm_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( javavm_getMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUSTR(
                 "com.sun.star.java.JavaVirtualMachine");
            pNames = &seqNames;
        }
    }
    return *pNames;
}

static OUString javavm_getImplementationName()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        MutexGuard guard( javavm_getMutex() );
        if( ! pImplName )
        {
            static OUString implName(RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.comp.stoc.JavaVirtualMachine") );
            pImplName = &implName;
        }
    }
    return *pImplName;
}

static jint JNICALL vm_vfprintf( FILE *fp, const char *format, va_list args ) {
#ifdef DEBUG
        char buff[1024];

        vsprintf( buff, format, args );

        OSL_TRACE("%s", buff);
        return strlen(buff);
#else
        return 0;
#endif
}

static void JNICALL vm_exit(jint code) {
    OSL_TRACE("vm_exit: %d\n", code);
}

static void JNICALL vm_abort() {
    OSL_TRACE("vm_abort\n");
    abort();
}

typedef ::std::hash_map<sal_uInt32, sal_uInt32> UINT32_UINT32_HashMap;

class JavaVirtualMachine_Impl;

class OCreatorThread : public Thread {
    JavaVirtualMachine_Impl * _pJavaVirtualMachine_Impl;
    JavaVM                  * _pJVM;

    Condition _start_Condition;
    Condition _wait_Condition;

    JVM _jvm;
    RuntimeException _runtimeException;
    WrappedTargetRuntimeException wrappedException;

protected:
    virtual void SAL_CALL run() throw();

public:
    OCreatorThread(JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl) throw();
    JavaVM * createJavaVM(const JVM & jvm) throw (RuntimeException);
    void disposeJavaVM() throw();

};


OCreatorThread::OCreatorThread(JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl) throw()
    : _pJVM(NULL)
{
    _pJavaVirtualMachine_Impl = pJavaVirtualMachine_Impl;
}

void OCreatorThread::run() throw()
{
    _start_Condition.wait();
    _start_Condition.reset();
    try {
        _pJVM = _pJavaVirtualMachine_Impl->createJavaVM(_jvm);
    }
    catch(WrappedTargetRuntimeException& wrappedExc)
    {
        wrappedException= wrappedExc;
    }
    catch(RuntimeException & runtimeException)
    {
        _runtimeException = runtimeException;
    }

    _wait_Condition.set();

    if (_pJVM) {
#if defined(WNT) || defined(OS2)
        suspend();

#else
        _start_Condition.wait();
        _start_Condition.reset();

        _pJavaVirtualMachine_Impl->disposeJavaVM();

        _wait_Condition.set();
#endif
    }
}

JavaVM * OCreatorThread::createJavaVM(const JVM & jvm ) throw(RuntimeException) {
    _jvm = jvm;

    if (!_pJVM )
    {
        create();

        _start_Condition.set();

        _wait_Condition.wait();
        _wait_Condition.reset();

        if(!_pJVM)
        {
            if( wrappedException.TargetException.getValue() != 0)
                throw wrappedException;
            else
                throw _runtimeException;
        }
    }

    return _pJVM;
}

void OCreatorThread::disposeJavaVM() throw() {
    _start_Condition.set(); // start disposing vm

#ifdef UNX
    _wait_Condition.wait(); // wait until disposed
    _wait_Condition.reset();
#endif
}



// XServiceInfo
OUString SAL_CALL JavaVirtualMachine_Impl::getImplementationName() throw(RuntimeException)
{
    return javavm_getImplementationName();
}

// XServiceInfo
sal_Bool SAL_CALL JavaVirtualMachine_Impl::supportsService(const OUString& ServiceName) throw(RuntimeException) {
    Sequence<OUString> aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for (sal_Int32 i = 0; i < aSNL.getLength(); ++ i)
        if (pArray[i] == ServiceName)
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence<OUString> SAL_CALL JavaVirtualMachine_Impl::getSupportedServiceNames() throw(RuntimeException)
{
    return javavm_getSupportedServiceNames();
}
// XContainerListener
void SAL_CALL JavaVirtualMachine_Impl::elementInserted( const ContainerEvent& Event )
    throw (RuntimeException)
{
}

// XContainerListener
void SAL_CALL JavaVirtualMachine_Impl::elementRemoved( const ContainerEvent& Event )
    throw (RuntimeException)
{
}

// XContainerListener
// If a user changes the setting, for example for proxy settings, then this function
// will be called from the configuration manager. Even if the .xml file does not contain
// an entry yet and that entry has to be inserted, this function will be called.
// We call java.lang.System.setProperty for the new values
void SAL_CALL JavaVirtualMachine_Impl::elementReplaced( const ContainerEvent& Event )
    throw (RuntimeException)
{
    MutexGuard aGuard( javavm_getMutex());
    OUString sAccessor;
    Event.Accessor >>= sAccessor;
    OUString sPropertyName;
    OUString sPropertyValue;
    OUString sPropertyName2;
    sal_Bool bSecurityChanged= sal_False;
    sal_Bool bDone= sal_False;

    if (sAccessor == OUSTR("ooInetFTPProxyName"))
    {
        sPropertyName= OUSTR("ftp.proxyHost");
        Event.Element >>= sPropertyValue;
    }
    else if (sAccessor == OUSTR("ooInetFTPProxyPort"))
    {
        sPropertyName= OUSTR("ftp.proxyPort");
        sPropertyValue= OUString::valueOf( *(sal_Int32*)Event.Element.getValue());
    }
    else if (sAccessor == OUSTR("ooInetHTTPProxyName"))
    {
        sPropertyName= OUSTR("http.proxyHost");
        Event.Element >>= sPropertyValue;
    }
    else if (sAccessor == OUSTR("ooInetHTTPProxyPort"))
    {
        sPropertyName= OUSTR("http.proxyPort");
        sPropertyValue= OUString::valueOf( *(sal_Int32*)Event.Element.getValue());
    }
    else if (sAccessor == OUSTR("ooInetNoProxy"))
    {
        sPropertyName= OUSTR("ftp.nonProxyHosts");
        sPropertyName2= OUSTR("http.nonProxyHosts");
        Event.Element >>= sPropertyValue;
        sPropertyValue= sPropertyValue.replace((sal_Unicode)';', (sal_Unicode)'|');
    }
/*  else if (sAccessor == OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetSOCKSProxyName")))
    {
        sPropertyName= OUString(RTL_CONSTASCII_USTRINGPARAM("socksProxyHost"));
        Event.Element >>= sPropertyValue;
    }
    else if (sAccessor == OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetSOCKSProxyPort")))
    {
        sPropertyName= OUString(RTL_CONSTASCII_USTRINGPARAM("socksProxyPort"));
        sPropertyValue= OUString::valueOf( *(sal_Int32*)Event.Element.getValue());
    }
*/  else if (sAccessor == OUSTR("ooInetProxyType"))
    {
        //Proxy none, manually
        sal_Int32 value;
        Event.Element >>= value;
        setINetSettingsInVM( (sal_Bool) value);
        bDone= sal_True;
    }
    else if (sAccessor == OUSTR("NetAccess"))
    {
        bSecurityChanged= sal_True;
        sPropertyName= OUSTR("appletviewer.security.mode");
        sal_Int32 val;
        if( Event.Element >>= val)
        {
            switch( val)
            {
            case 0: sPropertyValue= OUSTR("unrestricted");
                break;
            case 1: sPropertyValue= OUSTR("none");
                break;
            case 2: sPropertyValue= OUSTR("host");
                break;
            }
        }
        else
            return;
    }
    else if (sAccessor == OUSTR("Security"))
    {
        bSecurityChanged= sal_True;
        sPropertyName= OUSTR("stardiv.security.disableSecurity");
        sal_Bool val;
        if( Event.Element >>= val)
        {
            if( val)
                sPropertyValue= OUSTR("false");
            else
                sPropertyValue= OUSTR("true");
        }
        else
            return;
    }
    else
        return;

    if ( ! bDone && _pVMContext && _pVMContext->_pJavaVM)
    {
        JNIEnv* pJNIEnv= NULL;
        sal_Bool bThreadAttached= sal_False;
        jint ret= _pVMContext->_pJavaVM->AttachCurrentThread((void **)&pJNIEnv, (void*)NULL);
        OSL_ENSURE( !ret,"JavaVM could not attach current thread to VM");
        if ( ! _pVMContext->isThreadAttached())
        {
            bThreadAttached= sal_True;
        }

        // call java.lang.System.setProperty
        // String setProperty( String key, String value)
        jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass java/lang/System"), Reference<XInterface>());
        jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetStaticMethodID java.lang.System.setProperty"), Reference<XInterface>());

        jstring jsPropName= pJNIEnv->NewString( sPropertyName.getStr(), sPropertyName.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());

        // remove the property if it does not have a value ( user left the dialog field empty)
        // or if the port is set to 0
        sPropertyValue= sPropertyValue.trim();
        if( sPropertyValue.getLength() == 0 ||
            (sPropertyName.equals( OUSTR("ftp.proxyPort")) ||
             sPropertyName.equals( OUSTR("http.proxyPort")) /*||
              sPropertyName.equals( OUString( RTL_CONSTASCII_USTRINGPARAM("socksProxyPort")))*/) &&
            sPropertyValue.equals(OUSTR("0")))
        {
            // call java.lang.System.getProperties
            jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetStaticMethodID java.lang.System.getProperties"), Reference<XInterface>());
            jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.getProperties"), Reference<XInterface>());
            // call java.util.Properties.remove
            jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass java/util/Properties"), Reference<XInterface>());
            jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetMethodID java.util.Properties.remove"), Reference<XInterface>());
            jobject joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName);

            // special calse for ftp.nonProxyHosts and http.nonProxyHosts. The office only
            // has a value for two java properties
            if (sPropertyName2.getLength() > 0)
            {
                jstring jsPropName2= pJNIEnv->NewString( sPropertyName2.getStr(), sPropertyName2.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                jobject joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsPropName2);
            }
        }
        else
        {
            // Change the Value of the property
            jstring jsPropValue= pJNIEnv->NewString( sPropertyValue.getStr(), sPropertyValue.getLength());
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
            jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName, jsPropValue);
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());

            // special calse for ftp.nonProxyHosts and http.nonProxyHosts. The office only
            // has a value for two java properties
            if (sPropertyName2.getLength() > 0)
            {
                jstring jsPropName= pJNIEnv->NewString( sPropertyName2.getStr(), sPropertyName2.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                jstring jsPropValue= pJNIEnv->NewString( sPropertyValue.getStr(), sPropertyValue.getLength());
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsPropName, jsPropValue);
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
            }
        }

        // If the settings for Security and NetAccess changed then we have to notify the SandboxSecurity
        // SecurityManager
        // call System.getSecurityManager()
        if (bSecurityChanged)
        {
            jmethodID jmGetSecur= pJNIEnv->GetStaticMethodID( jcSystem,"getSecurityManager","()Ljava/lang/SecurityManager;");
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetStaticMethodID java.lang.System.getSecurityManager"), Reference<XInterface>());
            jobject joSecur= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetSecur);
            if (joSecur != 0)
            {
                // Make sure the SecurityManager is our SandboxSecurity
                // FindClass("com.sun.star.lib.sandbox.SandboxSecurityManager" only worked at the first time
                // this code was executed. Maybe it is a security feature. However, all attempts to debug the
                // SandboxSecurity class (maybe the VM invokes checkPackageAccess)  failed.
//                  jclass jcSandboxSec= pJNIEnv->FindClass("com.sun.star.lib.sandbox.SandboxSecurity");
//                  if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass com.sun.star.lib.sandbox.SandboxSecurity"), Reference<XInterface>());
//                  jboolean bIsSand= pJNIEnv->IsInstanceOf( joSecur, jcSandboxSec);
                // The SecurityManagers class Name must be com.sun.star.lib.sandbox.SandboxSecurity
                jclass jcSec= pJNIEnv->GetObjectClass( joSecur);
                jclass jcClass= pJNIEnv->FindClass("java/lang/Class");
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass java.lang.Class"), Reference<XInterface>());
                jmethodID jmName= pJNIEnv->GetMethodID( jcClass,"getName","()Ljava/lang/String;");
                if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetMethodID java.lang.Class.getName"), Reference<XInterface>());
                jstring jsClass= (jstring) pJNIEnv->CallObjectMethod( jcSec, jmName);
                const jchar* jcharName= pJNIEnv->GetStringChars( jsClass, NULL);
                OUString sName( jcharName);
                jboolean bIsSandbox;
                if (sName == OUSTR("com.sun.star.lib.sandbox.SandboxSecurity"))
                    bIsSandbox= JNI_TRUE;
                else
                    bIsSandbox= JNI_FALSE;
                pJNIEnv->ReleaseStringChars( jsClass, jcharName);

                if (bIsSandbox == JNI_TRUE)
                {
                    // call SandboxSecurity.reset
                    jmethodID jmReset= pJNIEnv->GetMethodID( jcSec,"reset","()V");
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetMethodID com.sun.star.lib.sandbox.SandboxSecurity.reset"), Reference<XInterface>());
                    pJNIEnv->CallVoidMethod( joSecur, jmReset);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallVoidMethod com.sun.star.lib.sandbox.SandboxSecurity.reset"), Reference<XInterface>());
                }
            }
        }
        if (bThreadAttached)
        {
            jint ret= _pVMContext->_pJavaVM->DetachCurrentThread();
            OSL_ENSURE( !ret,"JavaVM could not detach current thread to VM");
        }
    }
}

// param true: all Inet setting are set as Java Properties on a live VM.
// false: the Java net properties are set to empty value.
void JavaVirtualMachine_Impl::setINetSettingsInVM( sal_Bool set_reset)
{
    MutexGuard aGuard( javavm_getMutex());
    try
    {
    if (_pVMContext && _pVMContext->_pJavaVM)
    {
        JNIEnv* pJNIEnv= NULL;
        sal_Bool bThreadAttached= sal_False;
        jint ret= _pVMContext->_pJavaVM->AttachCurrentThread((void **)&pJNIEnv, (void*)NULL);
        OSL_ENSURE( !ret,"JavaVM could not attach current thread to VM");
        if ( ! _pVMContext->isThreadAttached())
        {
            bThreadAttached= sal_True;
        }

        // The Java Properties
        OUString sFtpProxyHost(RTL_CONSTASCII_USTRINGPARAM("ftp.proxyHost") );
        OUString sFtpProxyPort(RTL_CONSTASCII_USTRINGPARAM("ftp.proxyPort") );
        OUString sFtpNonProxyHosts (RTL_CONSTASCII_USTRINGPARAM("ftp.nonProxyHosts"));
        OUString sHttpProxyHost(RTL_CONSTASCII_USTRINGPARAM("http.proxyHost") );
        OUString sHttpProxyPort(RTL_CONSTASCII_USTRINGPARAM("http.proxyPort") );
        OUString sHttpNonProxyHosts(RTL_CONSTASCII_USTRINGPARAM("http.nonProxyHosts"));
//      OUString sSocksProxyHost(RTL_CONSTASCII_USTRINGPARAM("socksProxyHost"));
//      OUString sSocksProxyPort(RTL_CONSTASCII_USTRINGPARAM("socksProxyPort"));
        // creat Java Properties as JNI strings
        jstring jsFtpProxyHost= pJNIEnv->NewString( sFtpProxyHost.getStr(), sFtpProxyHost.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
        jstring jsFtpProxyPort= pJNIEnv->NewString( sFtpProxyPort.getStr(), sFtpProxyPort.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
        jstring jsFtpNonProxyHosts= pJNIEnv->NewString( sFtpNonProxyHosts.getStr(), sFtpNonProxyHosts.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
        jstring jsHttpProxyHost= pJNIEnv->NewString( sHttpProxyHost.getStr(), sHttpProxyHost.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
        jstring jsHttpProxyPort= pJNIEnv->NewString( sHttpProxyPort.getStr(), sHttpProxyPort.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
        jstring jsHttpNonProxyHosts= pJNIEnv->NewString( sHttpNonProxyHosts.getStr(), sHttpNonProxyHosts.getLength());
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
//      jstring jsSocksProxyHost= pJNIEnv->NewString( sSocksProxyHost.getStr(), sSocksProxyHost.getLength());
//      if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR(""), Reference<XInterface>());
//      jstring jsSocksProxyPort= pJNIEnv->NewString( sSocksProxyPort.getStr(), sSocksProxyPort.getLength());
//      if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR(""), Reference<XInterface>());

        // prepare java.lang.System.setProperty
        jclass jcSystem= pJNIEnv->FindClass("java/lang/System");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass java/lang/System"), Reference<XInterface>());
        jmethodID jmSetProps= pJNIEnv->GetStaticMethodID( jcSystem, "setProperty","(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetStaticMethodID java.lang.System.setProperty"), Reference<XInterface>());

        // call java.lang.System.getProperties
        jmethodID jmGetProps= pJNIEnv->GetStaticMethodID( jcSystem, "getProperties","()Ljava/util/Properties;");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetStaticMethodID java.lang.System.getProperties"), Reference<XInterface>());
        jobject joProperties= pJNIEnv->CallStaticObjectMethod( jcSystem, jmGetProps);
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.getProperties"), Reference<XInterface>());
        // prepare java.util.Properties.remove
        jclass jcProperties= pJNIEnv->FindClass("java/util/Properties");
        if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:FindClass java/util/Properties"), Reference<XInterface>());

        if (set_reset)
        {
            // Set all network properties with the VM
            JVM jvm;
            getINetPropsFromConfig( &jvm, _xSMgr, _xCtx);
            const ::std::vector< OUString> & Props = jvm.getProperties();
            typedef ::std::vector< OUString >::const_iterator C_IT;

            for( C_IT i= Props.begin(); i != Props.end(); i++)
            {
                OUString prop= *i;
                sal_Int32 index= prop.indexOf( (sal_Unicode)'=');
                OUString propName= prop.copy( 0, index);
                OUString propValue= prop.copy( index + 1);

                if( propName.equals( sFtpProxyHost))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyHost, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
                else if( propName.equals( sFtpProxyPort))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpProxyPort, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
                else if( propName.equals( sFtpNonProxyHosts))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsFtpNonProxyHosts, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
                else if( propName.equals( sHttpProxyHost))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyHost, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
                else if( propName.equals( sHttpProxyPort))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpProxyPort, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
                else if( propName.equals( sHttpNonProxyHosts))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:NewString"), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsHttpNonProxyHosts, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:CallStaticObjectMethod java.lang.System.setProperty"), Reference<XInterface>());
                }
/*              else if( propName.equals( sSocksProxyHost))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("")), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsSocksProxyHost, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("")), Reference<XInterface>());
                }
                else if( propName.equals( sSocksProxyPort))
                {
                    jstring jsVal= pJNIEnv->NewString( propValue.getStr(), propValue.getLength());
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("")), Reference<XInterface>());
                    jstring jsPrevValue= (jstring)pJNIEnv->CallStaticObjectMethod( jcSystem, jmSetProps, jsSocksProxyPort, jsVal);
                    if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("")), Reference<XInterface>());
                }
*/          }
        }
        else
        {
            // call java.util.Properties.remove
            jmethodID jmRemove= pJNIEnv->GetMethodID( jcProperties, "remove", "(Ljava/lang/Object;)Ljava/lang/Object;");
            if(pJNIEnv->ExceptionOccurred()) throw RuntimeException(OUSTR("JNI:GetMethodID java.util.Property.remove"), Reference<XInterface>());
            jobject joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpProxyHost);
            joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpProxyPort);
            joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsFtpNonProxyHosts);
            joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpProxyHost);
            joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpProxyPort);
            joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsHttpNonProxyHosts);
//          joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsSocksProxyHost);
//          joPrev= pJNIEnv->CallObjectMethod( joProperties, jmRemove, jsSocksProxyPort);
        }

        if (bThreadAttached)
        {
            jint ret= _pVMContext->_pJavaVM->DetachCurrentThread();
            OSL_ENSURE( !ret,"JavaVM could not detach current thread to VM");
        }
    }
    }
    catch( RuntimeException& e)
    {e;
    }

}
// XEventListenerListener
void SAL_CALL JavaVirtualMachine_Impl::disposing( const EventObject& Source )
    throw (RuntimeException)
{
    // In case the configuration manager wants to shut down
    if (_xConfigurationAccess.is() &&  Source.Source == _xConfigurationAccess)
    {
        Reference< XContainer > xContainer(_xConfigurationAccess, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener( static_cast< XContainerListener* >(this));
        _xConfigurationAccess= 0;
    }
    // In case the configuration manager wants to shut down
    if (_xConfigurationAccess2.is() &&  Source.Source == _xConfigurationAccess2)
    {
        Reference< XContainer > xContainer(_xConfigurationAccess2, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener( static_cast< XContainerListener* >(this));
        _xConfigurationAccess2= 0;
    }

    // If the service manager calls us then we are about to be shut down, therefore
    // unregister everywhere. Currently this service is only registered with the
    // configuration manager
    Reference< XInterface > xIntMgr( _xSMgr, UNO_QUERY);
    if (Source.Source == xIntMgr)
    {
        Reference< XContainer > xContainer(_xConfigurationAccess, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener( static_cast< XContainerListener* >(this));

        Reference< XContainer > xContainer2(_xConfigurationAccess2, UNO_QUERY);
        if (xContainer2.is())
            xContainer2->removeContainerListener( static_cast< XContainerListener* >(this));
    }
}

static void getDefaultLocaleFromConfig(JVM * pjvm,
                                       const Reference<XMultiComponentFactory> & xSMgr,
                                       const Reference<XComponentContext> &xCtx ) throw(Exception)
{
    Reference<XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
        OUSTR("com.sun.star.configuration.ConfigurationRegistry"),
        xCtx );
    if(!xConfRegistry.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
    if(!xConfRegistry_simple.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    xConfRegistry_simple->open(OUSTR("org.openoffice.Setup"), sal_True, sal_False);
    Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

    // read locale
    Reference<XRegistryKey> locale = xRegistryRootKey->openKey(OUSTR("L10N/ooLocale"));
    if(locale.is() && locale->getStringValue().getLength()) {
        OUString language;
        OUString country;

        sal_Int32 index = locale->getStringValue().indexOf((sal_Unicode) '-');

        if(index >= 0) {
            language = locale->getStringValue().copy(0, index);
            country = locale->getStringValue().copy(index + 1);

            if(language.getLength()) {
                OUString prop(RTL_CONSTASCII_USTRINGPARAM("user.language="));
                prop += language;

                pjvm->pushProp(prop);
            }

            if(country.getLength()) {
                OUString prop(RTL_CONSTASCII_USTRINGPARAM("user.region="));
                prop += country;

                pjvm->pushProp(prop);
            }
        }
    }

    xConfRegistry_simple->close();
}

static void setTimeZone(JVM * pjvm) throw() {
    /* A Bug in the Java function
    ** struct Hjava_util_Properties * java_lang_System_initProperties(
    ** struct Hjava_lang_System *this,
    ** struct Hjava_util_Properties *props);
    ** This function doesn't detect MEZ, MET or "W. Europe Standard Time"
    */
    struct tm *tmData;
    time_t clock = time(NULL);
    tzset();
    tmData = localtime(&clock);
#ifdef MACOSX
    char * p = tmData->tm_zone;
#else
    char * p = tzname[0];
#endif

    if (!strcmp(TIMEZONE, p))
        pjvm->pushProp(OUString::createFromAscii("user.timezone=ECT"));
}

// Only gets the properties if the "Proxy Server" entry in the option dialog is
// set to manual (i.e. not to none)
static void getINetPropsFromConfig(JVM * pjvm,
                                   const Reference<XMultiComponentFactory> & xSMgr,
                                   const Reference<XComponentContext> &xCtx ) throw (Exception)
{
    Reference<XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
            OUSTR("com.sun.star.configuration.ConfigurationRegistry"),
            xCtx );
    if(!xConfRegistry.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
    if(!xConfRegistry_simple.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    xConfRegistry_simple->open(OUSTR("org.openoffice.Inet"), sal_True, sal_False);
    Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

//  if ooInetProxyType is not 0 then read the settings
    Reference<XRegistryKey> proxyEnable= xRegistryRootKey->openKey(OUSTR("Settings/ooInetProxyType"));
    if( proxyEnable.is() && 0 != proxyEnable->getLongValue())
    {
        // read ftp proxy name
        Reference<XRegistryKey> ftpProxy_name = xRegistryRootKey->openKey(OUSTR("Settings/ooInetFTPProxyName"));
        if(ftpProxy_name.is() && ftpProxy_name->getStringValue().getLength()) {
            OUString ftpHost = OUSTR("ftp.proxyHost=");
            ftpHost += ftpProxy_name->getStringValue();

            // read ftp proxy port
            Reference<XRegistryKey> ftpProxy_port = xRegistryRootKey->openKey(OUSTR("Settings/ooInetFTPProxyPort"));
            if(ftpProxy_port.is() && ftpProxy_port->getLongValue()) {
                OUString ftpPort = OUSTR("ftp.proxyPort=");
                ftpPort += OUString::valueOf(ftpProxy_port->getLongValue());

                pjvm->pushProp(ftpHost);
                pjvm->pushProp(ftpPort);
            }
        }

        // read http proxy name
        Reference<XRegistryKey> httpProxy_name = xRegistryRootKey->openKey(OUSTR("Settings/ooInetHTTPProxyName"));
        if(httpProxy_name.is() && httpProxy_name->getStringValue().getLength()) {
            OUString httpHost = OUSTR("http.proxyHost=");
            httpHost += httpProxy_name->getStringValue();

            // read http proxy port
            Reference<XRegistryKey> httpProxy_port = xRegistryRootKey->openKey(OUSTR("Settings/ooInetHTTPProxyPort"));
            if(httpProxy_port.is() && httpProxy_port->getLongValue()) {
                OUString httpPort = OUSTR("http.proxyPort=");
                httpPort += OUString::valueOf(httpProxy_port->getLongValue());

                pjvm->pushProp(httpHost);
                pjvm->pushProp(httpPort);
            }
        }

        // read  nonProxyHosts
        Reference<XRegistryKey> nonProxies_name = xRegistryRootKey->openKey(OUSTR("Settings/ooInetNoProxy"));
        if(nonProxies_name.is() && nonProxies_name->getStringValue().getLength()) {
            OUString httpNonProxyHosts = OUSTR("http.nonProxyHosts=");
            OUString ftpNonProxyHosts= OUSTR("ftp.nonProxyHosts=");
            OUString value= nonProxies_name->getStringValue();
            // replace the separator ";" by "|"
            value= value.replace((sal_Unicode)';', (sal_Unicode)'|');

            httpNonProxyHosts += value;
            ftpNonProxyHosts += value;

            pjvm->pushProp(httpNonProxyHosts);
            pjvm->pushProp(ftpNonProxyHosts);
        }

        // read socks settings
/*      Reference<XRegistryKey> socksProxy_name = xRegistryRootKey->openKey(OUSTR("Settings/ooInetSOCKSProxyName"));
        if (socksProxy_name.is() && httpProxy_name->getStringValue().getLength()) {
            OUString socksHost = OUSTR("socksProxyHost=");
            socksHost += socksProxy_name->getStringValue();

            // read http proxy port
            Reference<XRegistryKey> socksProxy_port = xRegistryRootKey->openKey(OUSTR("Settings/ooInetSOCKSProxyPort"));
            if (socksProxy_port.is() && socksProxy_port->getLongValue()) {
                OUString socksPort = OUSTR("socksProxyPort=");
                socksPort += OUString::valueOf(socksProxy_port->getLongValue());

                pjvm->pushProp(socksHost);
                pjvm->pushProp(socksPort);
            }
        }
*/  }
    xConfRegistry_simple->close();
}

static void getJavaPropsFromConfig(JVM * pjvm,
                                   const Reference<XMultiComponentFactory> & xSMgr,
                                   const Reference<XComponentContext> &xCtx) throw(Exception)
{
    Reference<XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
        OUSTR("com.sun.star.configuration.ConfigurationRegistry"),
        xCtx);
    if(!xConfRegistry.is())
        throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"),
                               Reference<XInterface>());
    Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
    if(!xConfRegistry_simple.is())
        throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"),
                               Reference<XInterface>());
    xConfRegistry_simple->open(OUSTR("org.openoffice.Setup"), sal_True, sal_False);
    Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();
    Reference<XRegistryKey> key_InstallPath = xRegistryRootKey->openKey(
        OUSTR("Office/ooSetupInstallPath"));
    if(!key_InstallPath.is())
        throw RuntimeException(OUSTR("javavm.cxx: can not find key: " \
                                     "Office/InstallPath in org.openoffice.UserProfile"),
                                                     Reference<XInterface>());
    OUString rcPath = key_InstallPath->getStringValue();
    Reference<XInterface> xIniManager(xSMgr->createInstanceWithContext(
        OUSTR("com.sun.star.config.INIManager"),xCtx));
    if(!xIniManager.is())
        throw RuntimeException(OUSTR("javavm.cxx: couldn't get: com.sun.star.config.INIManager"),
                               Reference<XInterface>());
    Reference<XSimpleRegistry> xIniManager_simple(xIniManager, UNO_QUERY);
    if(!xIniManager_simple.is())
        throw RuntimeException(OUSTR("javavm.cxx: couldn't get: com.sun.star.config.INIManager"),
                               Reference<XInterface>());
    // normalize the path
    OUString urlrcPath;
    if( osl_File_E_None != File::getFileURLFromSystemPath( rcPath, urlrcPath ) )
    {
        urlrcPath = rcPath;
    }
    urlrcPath += OUSTR("/config/" INI_FILE);


    Reference<XRegistryKey> xJavaSection;
    sal_Bool bNoUserJavarc= sal_False;
    try
    {   // open user/config/javarc
        xIniManager_simple->open(urlrcPath, sal_True, sal_False);
    }
    catch( Exception & e)
    {
        //now we try share/config/javarc
        bNoUserJavarc= sal_True;
    }

    // Network installation. A workstation installation does not need to have its own javarc.
    // Then we use the one from the network insallation ( <install-di>/share/config/javarc)
    if( bNoUserJavarc)
    {
        xConfRegistry_simple->open(OUSTR("org.openoffice.Office.Common"), sal_True, sal_False);
        Reference<XRegistryKey> xRegistryCommonRootKey = xConfRegistry_simple->getRootKey();

        Reference<XRegistryKey> key_NetInstallPath = xRegistryCommonRootKey->openKey(
            OUSTR("Path/Current/OfficeInstall"));
        if(!key_NetInstallPath.is()) throw RuntimeException(
            OUSTR("javavm.cxx: can not find key: Office/InstallPath in org.openoffice.UserProfile"),
            Reference<XInterface>());
        rcPath = key_NetInstallPath->getStringValue();
        // convert to file url
        if( osl_File_E_None != File::getFileURLFromSystemPath( rcPath, urlrcPath ) )
        {
            urlrcPath = rcPath;
        }
        urlrcPath += OUSTR("/share/config/" INI_FILE);

        try
        {
            // open share/config/javarc
            xIniManager_simple->open(urlrcPath, sal_True, sal_False);
        }
        catch( Exception & e)
        {
            throw JavaNotConfiguredException (
                OUSTR("javavm.cxx: can not find " INI_FILE ),
                Reference<XInterface>());
        }
    }
    try
    {
        xJavaSection = xIniManager_simple->getRootKey()->openKey(OUSTR("Java"));
    }
    catch( Exception & e)
    {
        throw JavaNotConfiguredException(OUSTR("javavm.cxx: cannot open Java section" \
                                               "in " INI_FILE), Reference<XInterface>());
    }
    if(!xJavaSection.is() || !xJavaSection->isValid())
        throw JavaNotConfiguredException(OUSTR("javavm.cxx: "  INI_FILE " has invalid" \
                                             "Java section"), Reference<XInterface>());

    Sequence<OUString> javaProperties = xJavaSection->getKeyNames();
    OUString * pSectionEntry = javaProperties.getArray();
    sal_Int32 nCount         = javaProperties.getLength();

    for(sal_Int32 i=0; i < nCount; ++ i)
    {
        //Reconstruct the whole lines of the java.ini
        Reference< XRegistryKey > key= xJavaSection->openKey(pSectionEntry[i]);
        if (key.is())
        {
            // there was a "=" in the line, hence key/value pair.
            OUString entryValue = key->getStringValue();

            if(entryValue.getLength()) {
                pSectionEntry[i] += OUSTR("=");
                pSectionEntry[i] += entryValue;
            }
        }

        pjvm->pushProp(pSectionEntry[i]);
    }

    xIniManager_simple->close();
}

static const Bootstrap & getBootstrapHandle()
{
    static rtl::Bootstrap *pBootstrap = 0;
    if( !pBootstrap )
    {
        OUString exe;
        osl_getExecutableFile( &(exe.pData) );

        sal_Int32 nIndex = exe.lastIndexOf( '/' );
        OUString ret;
        if( exe.getLength() && nIndex != -1 )
        {
            OUStringBuffer buf( exe.getLength() + 10 );
            buf.append( exe.getStr() , nIndex +1 ).appendAscii( SAL_CONFIGFILE("uno") );
            ret = buf.makeStringAndClear();
        }
#ifdef DEBUG
        OString o = OUStringToOString( ret , RTL_TEXTENCODING_ASCII_US );
        printf( "JavaVM: Used ininame %s\n" , o.getStr() );
#endif
        static Bootstrap  bootstrap( ret );
        pBootstrap = &bootstrap;
    }
    return *pBootstrap;
}


static OUString retrieveComponentClassPath( const sal_Char *pVariableName )
{
    OUString ret;
    OUStringBuffer buf( 128 );
    buf.appendAscii( "$" ).appendAscii( pVariableName );
    OUString path( buf.makeStringAndClear() );

    const Bootstrap & handle = getBootstrapHandle();
    rtl_bootstrap_expandMacros_from_handle( *(void**)&handle , &(path.pData) );
    if( path.getLength() )
    {
        buf.append( path ).appendAscii( "/java_classpath" );

        OUString fileName( buf.makeStringAndClear() );
        sal_Char * p = 0;

        DirectoryItem item;
        if( DirectoryItem::E_None == DirectoryItem::get( fileName , item ) )
        {
            FileStatus status ( osl_FileStatus_Mask_FileSize );
            if( FileBase::E_None == item.getFileStatus( status ) )
            {
                sal_Int64 nSize = status.getFileSize();
                if( nSize )
                {
                    sal_Char * p = (sal_Char * ) rtl_allocateMemory( (sal_uInt32)nSize +1 );
                    if( p )
                    {
                        File file( fileName );
                        if( File::E_None == file.open( OpenFlag_Read ) )
                        {
                            sal_uInt64 nRead;
                            if( File::E_None == file.read( p , (sal_uInt64)nSize , nRead )
                                && (sal_uInt64)nSize == nRead )
                            {
                                buf = OUStringBuffer( 1024 );

                                sal_Int32 nIndex = 0;
                                sal_Bool bPrepend = sal_False;
                                while( nIndex < nSize )
                                {
                                    while( nIndex < nSize && p[nIndex] == ' ' ) nIndex ++;
                                    sal_Int32 nStart = nIndex;
                                    while( nIndex < nSize && p[nIndex] != ' ' ) nIndex ++;
                                    OUString relativeUrl( &(p[nStart]), nIndex-nStart, RTL_TEXTENCODING_ASCII_US);
                                    OUString fileurlElement;
                                    OUString systemPathElement;
                                    OSL_VERIFY( osl_File_E_None ==
                                                osl_getAbsoluteFileURL( path.pData, relativeUrl.pData , &(fileurlElement.pData) ) );
                                    OSL_VERIFY( osl_File_E_None ==
                                                osl_getSystemPathFromFileURL( fileurlElement.pData, &(systemPathElement.pData) ) );
                                    if( systemPathElement.getLength() )
                                    {
                                        if( bPrepend )
                                            buf.appendAscii( CLASSPATH_DELIMETER );
                                        else
                                            bPrepend = sal_True;
                                        buf.append( systemPathElement );
                                    }
                                }
                                ret = buf.makeStringAndClear();
                            }
                        }
                        rtl_freeMemory( p );
                    }
                }
            }
        }
    }
#ifdef DEBUG
    fprintf( stderr, "JavaVM: classpath retrieved from $%s: %s\n", pVariableName,
             OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US).getStr());
#endif
    return ret;
}

static void getJavaPropsFromSafetySettings(JVM * pjvm,
                                           const Reference<XMultiComponentFactory> & xSMgr,
                                           const Reference<XComponentContext> &xCtx) throw(Exception)
{
    Reference<XInterface> xConfRegistry = xSMgr->createInstanceWithContext(
        OUSTR("com.sun.star.configuration.ConfigurationRegistry"),
        xCtx);
    if(!xConfRegistry.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
    if(!xConfRegistry_simple.is()) throw RuntimeException(OUSTR("javavm.cxx: couldn't get ConfigurationRegistry"), Reference<XInterface>());

    xConfRegistry_simple->open(OUSTR("org.openoffice.Office.Java"), sal_True, sal_False);
    Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

    if (xRegistryRootKey.is())
    {
        Reference<XRegistryKey> key_Enable = xRegistryRootKey->openKey(OUSTR("VirtualMachine/Enable"));
        if (key_Enable.is())
        {
            sal_Bool bEnableVal= (sal_Bool) key_Enable->getLongValue();
            pjvm->setEnabled( bEnableVal);
        }
        Reference<XRegistryKey> key_UserClasspath = xRegistryRootKey->openKey(OUSTR("VirtualMachine/UserClassPath"));
        if (key_UserClasspath.is())
        {
            OUString sClassPath= key_UserClasspath->getStringValue();
            pjvm->addUserClasspath( sClassPath);
        }
                Reference<XRegistryKey> key_NetAccess= xRegistryRootKey->openKey(OUSTR("VirtualMachine/NetAccess"));
        if (key_NetAccess.is())
        {
            sal_Int32 val= key_NetAccess->getLongValue();
            OUString sVal;
            switch( val)
            {
            case 0: sVal= OUSTR("unrestricted");
                break;
            case 1: sVal= OUSTR("none");
                break;
            case 2: sVal= OUSTR("host");
                break;
            }
            OUString sProperty( RTL_CONSTASCII_USTRINGPARAM("appletviewer.security.mode="));
            sProperty= sProperty + sVal;
            pjvm->pushProp(sProperty);
        }
        Reference<XRegistryKey> key_CheckSecurity= xRegistryRootKey->openKey(
            OUSTR("VirtualMachine/Security"));
        if( key_CheckSecurity.is())
        {
            sal_Bool val= (sal_Bool) key_CheckSecurity->getLongValue();
            OUString sProperty(RTL_CONSTASCII_USTRINGPARAM("stardiv.security.disableSecurity="));
            if( val)
                sProperty= sProperty + OUSTR("false");
            else
                sProperty= sProperty + OUSTR("true");
            pjvm->pushProp( sProperty);
        }
    }
    xConfRegistry_simple->close();
}


static void getJavaPropsFromEnvironment(JVM * pjvm) throw() {

    const char * pClassPath = getenv("CLASSPATH");
    if( pClassPath )
    {
        pjvm->addSystemClasspath( OUString::createFromAscii(pClassPath) );
    }
    pjvm->setRuntimeLib(OUString::createFromAscii(DEF_JAVALIB));
    pjvm->setEnabled(1);

    // See if properties have been set and parse them
    const char * pOOjavaProperties = getenv(PROPERTIES_ENV);
    if(pOOjavaProperties) {
        OUString properties(OUString::createFromAscii(pOOjavaProperties));

        sal_Int32 index;
        sal_Int32 lastIndex = 0;

        do {
            index = properties.indexOf((sal_Unicode)',', lastIndex);
            OUString token = (index == -1) ? properties.copy(lastIndex) : properties.copy(lastIndex, index - lastIndex);

            lastIndex = index + 1;

            pjvm->pushProp(token);
        }
        while(index > -1);
    }
}

static void initVMConfiguration(JVM * pjvm,
                                const Reference<XMultiComponentFactory> & xSMgr,
                                const Reference<XComponentContext > &xCtx) throw(Exception) {
    JVM jvm;
    try {
        getINetPropsFromConfig(&jvm, xSMgr, xCtx);
    }
    catch(Exception & exception) {
#ifdef DEBUG
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: can not get INetProps cause of >%s<", message.getStr());
#endif
    }

    try {
        getDefaultLocaleFromConfig(&jvm, xSMgr,xCtx);
    }
    catch(Exception & exception) {
#ifdef DEBUG
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: can not get locale cause of >%s<", message.getStr());
#endif
    }

    sal_Bool bPropsFail= sal_False;
    sal_Bool bPropsFail2= sal_False;
    JavaNotConfiguredException confexc;
    try
    {
        getJavaPropsFromConfig(&jvm, xSMgr,xCtx);
    }
    catch(JavaNotConfiguredException& e)
    {
        confexc= e;
        bPropsFail= sal_True;
    }
    catch(Exception & exception)
    {
#ifdef DEBUG
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: couldn't use configuration cause of >%s<", message.getStr());
#endif
        bPropsFail2= sal_True;
    }

    if( bPropsFail ||bPropsFail2)
    {
        getJavaPropsFromEnvironment(&jvm);
        // at this point we have to find out if there is a classpath and runtimelib. If not
        // we'll throw the exception, because Java is misconfigured and won't run.
        OUString usRuntimeLib= jvm.getRuntimeLib();
        OUString usUserClasspath= jvm.getUserClasspath();
        OUString usSystemClasspath= jvm.getSystemClasspath();
        if( usRuntimeLib.getLength() == 0
            || (usUserClasspath.getLength == 0 && usSystemClasspath.getLength() == 0))
        {
            if (bPropsFail)
                throw confexc;
            throw new JavaNotConfiguredException(OUSTR("There is neither a java.ini (or javarc) " \
                                                       "and there are no environment variables set which " \
                                                       "contain configuration data"), Reference<XInterface>());
        }

    }

    try {
        getJavaPropsFromSafetySettings(&jvm, xSMgr, xCtx);
    }
    catch(Exception & exception) {
#ifdef DEBUG
        OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: couldn't get safety settings because of >%s<", message.getStr());
#endif
    }
    jvm.addSystemClasspath( retrieveComponentClassPath( "UNO_SHARE_PACKAGES_CACHE" ) );
    jvm.addUserClasspath( retrieveComponentClassPath( "UNO_USER_PACKAGES_CACHE" ) );

    *pjvm= jvm;
    setTimeZone(pjvm);

//          pjvm->setPrint(vm_vfprintf);
//          pjvm->setExit(vm_exit);
//          pjvm->setAbort(vm_abort);
}

JavaVirtualMachine_Impl::JavaVirtualMachine_Impl(const Reference< XComponentContext > &xCtx) throw()
    : _pVMContext(NULL)
    ,  _pJava_environment(NULL)
    , _xSMgr( xCtx->getServiceManager() )
      , _xCtx( xCtx )
      ,m_bInteractionAbort(sal_False)
      ,m_bInteractionRetry(sal_False)

{
    pCreatorThread= new OCreatorThread(this);
}

JavaVirtualMachine_Impl::~JavaVirtualMachine_Impl() throw()
{
    if (_pVMContext)
    {
        if( pCreatorThread)
        {
            pCreatorThread->disposeJavaVM();
            delete pCreatorThread;
        }

    }
    if (_xConfigurationAccess.is())
    {
        Reference< XContainer > xContainer(_xConfigurationAccess, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener( static_cast< XContainerListener* >(this));
    }
    if (_xConfigurationAccess2.is())
    {
        Reference< XContainer > xContainer(_xConfigurationAccess2, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener( static_cast< XContainerListener* >(this));
    }

    if (_xSMgr.is())
    {
        Reference< XComponent > xComp(_xSMgr, UNO_QUERY);
        if (xComp.is())
            xComp->removeEventListener( static_cast< XEventListener* >(this));
    }
}


JavaVM * JavaVirtualMachine_Impl::createJavaVM(const JVM & jvm) throw(RuntimeException)
{
    JavaVM * pJavaVM;

    if(!_javaLib.load(jvm.getRuntimeLib()))
    {
        //Java installation was deleted or moved
        OUString libURL;
        if( File::getFileURLFromSystemPath( jvm.getRuntimeLib(), libURL) != File::E_None)
            libURL= OUSTR("");
        MissingJavaRuntimeException exc(
            OUSTR("JavaVirtualMachine_Impl::createJavaVM, Java runtime library cannot be found"),
            Reference<XInterface>(), libURL);
        WrappedTargetRuntimeException wt;
        wt.TargetException<<= exc;
        throw wt;
    }

#ifdef UNX
        OUString javaHome(RTL_CONSTASCII_USTRINGPARAM("JAVA_HOME="));
        javaHome += jvm.getJavaHome();
        const OUString & vmType  = jvm.getVMType();

        if(!vmType.equals(OUSTR("jre")))
        {
            javaHome += OUSTR("/jre");
        }

        OString osJavaHome = OUStringToOString(javaHome, osl_getThreadTextEncoding());
        putenv(strdup(osJavaHome.getStr()));
#endif

        JNI_InitArgs_Type * initArgs = (JNI_InitArgs_Type *)_javaLib.getSymbol(
            OUSTR("JNI_GetDefaultJavaVMInitArgs"));
        JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *)_javaLib.getSymbol(
            OUSTR("JNI_CreateJavaVM"));
        if (!initArgs || !pCreateJavaVM)
        {
            // The java installation is somehow corrupted
            JavaVMCreationFailureException exc(
                OUSTR("JavaVirtualMachine_Impl::createJavaVM, could not find symbol " \
                      "JNI_GetDefaultJavaVMInitArgs or JNI_CreateJavaVM"),
                Reference<XInterface>(), 1);

            WrappedTargetRuntimeException wt;
            wt.TargetException<<= exc;
            throw wt;
        }

        JNIEnv * pJNIEnv = NULL;

        // Try VM 1.1
        JDK1_1InitArgs vm_args;
        vm_args.version= 0x00010001;
        jint ret= initArgs(&vm_args);
        jvm.setArgs(&vm_args);

        jint err;
        err= pCreateJavaVM(&pJavaVM, &pJNIEnv, &vm_args);

        if( err != 0)
        {
            // Try VM 1.2

            // The office sets a signal handler at startup. That causes a crash
            // with java 1.3 under Solaris. To make it work, we set back the
            // handler
#ifdef UNX
            struct sigaction act;
            act.sa_handler=SIG_DFL;
            act.sa_flags= 0;
            sigaction( SIGSEGV, &act, NULL);
            sigaction( SIGPIPE, &act, NULL);
            sigaction( SIGBUS, &act, NULL);
            sigaction( SIGILL, &act, NULL);
            sigaction( SIGFPE, &act, NULL);

#endif
            sal_uInt16 cprops= jvm.getProperties().size();

            JavaVMInitArgs vm_args2;

            // we have "addOpt" additional properties to those kept in the JVM struct
            sal_Int32 addOpt=2;
            JavaVMOption * options= new JavaVMOption[cprops + addOpt];
            OString sClassPath= OString("-Djava.class.path=") + vm_args.classpath;
            options[0].optionString= (char*)sClassPath.getStr();
            options[0].extraInfo= NULL;

            // We set an abort handler which is called when the VM calls _exit during
            // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
            // all some directories of the Java installation. This is necessary for
            // linux j2re1.3, 1.4 and Solaris j2re1.3. With a j2re1.4 on Solaris the
            // LD_LIBRARY_PATH need not to be set anymore.
            options[1].optionString= "abort";
            options[1].extraInfo= (void* )abort_handler;

            OString * arProps= new OString[cprops];

            /*If there are entries in the Java section of the java.ini/javarc which are meant
              to be java system properties then they get a "-D" at the beginning of the string.
              Entries which start with "-" are regarded as java options as they are passed at
              the command-line. If those entries appear under the Java section then there are
              used as they are. For example, the entry  "-ea" would be uses as
              JavaVMOption.optionString.
            */
            OString sJavaOption("-");
            for( sal_uInt16 x= 0; x< cprops; x++)
            {
                OString sOption(vm_args.properties[x]);
                if (!sOption.matchIgnoreAsciiCase(sJavaOption, 0))
                    arProps[x]= OString("-D") + vm_args.properties[x];
                else
                    arProps[x]= vm_args.properties[x];
                options[x+addOpt].optionString= (char*)arProps[x].getStr();
                options[x+addOpt].extraInfo= NULL;
            }
            vm_args2.version= 0x00010002;
            vm_args2.options= options;
            vm_args2.nOptions= cprops + addOpt;
            vm_args2.ignoreUnrecognized= JNI_TRUE;

            /* We set a global flag which is used by the abort handler in order to
               determine whether it is  should use longjmp to get back into this function.
               That is, the abort handler determines if it is on the same stack as this function
               and then jumps back into this function.
            */
            g_bInGetJavaVM= sal_True;
            memset( jmp_jvm_abort, 0, sizeof(jmp_jvm_abort));
            int jmpval= setjmp( jmp_jvm_abort );
            /* If jmpval is not "0" then this point was reached by a longjmp in the
               abort_handler, which was called indirectly by JNI_CreateVM.
            */
            if( jmpval == 0)
            {
                //returns negative number on failure
                err= pCreateJavaVM(&pJavaVM, &pJNIEnv, &vm_args2);
                g_bInGetJavaVM= sal_False;
                // Necessary to make debugging work. This thread will be suspended when this function
                // returns.
                if( err == 0)
                    pJavaVM->DetachCurrentThread();
            }
            else
                // set err to a positive number, so as or recognize that an abort (longjmp)
                //occurred
                err= 1;

            delete [] options;
            delete [] arProps;
        }
        if(err != 0)
        {
            OUString message;
            if( err < 0)
            {
                message= OUSTR(
                    "JavaVirtualMachine_Impl::createJavaVM - can not create VM, cause of err:");
                message += OUString::valueOf((sal_Int32)err);
            }
            else if( err > 0)
                message= OUSTR(
                    "JavaVirtualMachine_Impl::createJavaVM - can not create VM, abort handler was called");
            JavaVMCreationFailureException exc(message,
                                                 Reference<XInterface>(), err);
            WrappedTargetRuntimeException wt;
            wt.TargetException<<= exc;
            throw wt;
        }
        return pJavaVM;
}

// XJavaVM
Any JavaVirtualMachine_Impl::getJavaVM(const Sequence<sal_Int8> & processId) throw (RuntimeException) {
    MutexGuard guarg(javavm_getMutex());
    Sequence<sal_Int8> localProcessID(16);
    rtl_getGlobalProcessId( (sal_uInt8*) localProcessID.getArray() );

    if (localProcessID == processId && !_pVMContext)
    {
        uno_Environment ** ppEnviroments = NULL;
        sal_Int32 size = 0;
        OUString java(OUString::createFromAscii("java"));

        uno_getRegisteredEnvironments(&ppEnviroments, &size, (uno_memAlloc)malloc, java.pData);

        if(size)
        { // do we found an existing java environment?
            OSL_TRACE("javavm.cxx: found an existing environment");

            _pJava_environment = ppEnviroments[0];
            _pJava_environment->acquire(_pJava_environment);
            _pVMContext = (JavaVMContext *)_pJava_environment->pContext;

            for(sal_Int32 i = 0; i  < size; ++ i)
                ppEnviroments[i]->release(ppEnviroments[i]);

            free(ppEnviroments);
        }
        else
        {
            JVM *pJvm= NULL;
            JavaVM * pJavaVM;
        retry:
            if( pJvm)
                delete pJvm;
            pJvm= new JVM;

            try
            {
                initVMConfiguration(pJvm, _xSMgr, _xCtx);
            }
            catch( JavaNotConfiguredException & e)
            {
                Any anyExc;
                anyExc <<= e;
                SelectedAction  action= doClientInteraction( anyExc);
                switch( action)
                {
                case action_abort:
                    if( pJvm)
                        delete pJvm;
                    throw e;
                case action_retry: goto retry;
                default: goto retry;
                }
            }

            if (pJvm->isEnabled())
            {
                // create the java vm
                try {
                    // If there is an exception in createJavaVM then the client
                    // is notified through XInteractionHandler. If they decide to
                    // retry the action then createJavaVM does a longjmp to this
                    // point
                    pJavaVM = pCreatorThread->createJavaVM( * pJvm);
                }
                catch(WrappedTargetRuntimeException& e)
                {
                    delete pCreatorThread;
                    pCreatorThread= new OCreatorThread(this);
                    if( pJvm)
                    {
                        delete pJvm;
                        pJvm= NULL;
                    }

                    SelectedAction  action= doClientInteraction( e.TargetException);
                    switch( action)
                    {
                    case action_abort:
                    {
                        const Type& valueType= e.TargetException.getValueType();
                        if( valueType == getCppuType((JavaNotConfiguredException*)0))
                        {
                            JavaNotConfiguredException exc;
                            e.TargetException >>= exc;
                            throw exc;
                        }
                        else if( valueType == getCppuType((MissingJavaRuntimeException*)0))
                        {
                            MissingJavaRuntimeException exc;
                            e.TargetException >>= exc;
                            throw exc;
                        }
                        else if( valueType == getCppuType((JavaVMCreationFailureException*)0))
                        {
                            JavaVMCreationFailureException exc;
                            e.TargetException >>= exc;
                            throw exc;
                        }
                    }
                    case action_retry: goto retry;
                    default: goto retry;
                    }
                }
                catch(RuntimeException & e)
                {
                    e;
                    // When the creation of the VM failed then we ensure that on the next
                    // getJavaVM call a new attempt is made to create it. Therefore we
                    // must get rid of the running thread and prepare a new one.
                    delete pCreatorThread;
                    pCreatorThread= new OCreatorThread(this);
                    if( pJvm)
                        delete pJvm;
                    throw;
                }
                // create a context
                _pVMContext = new JavaVMContext(pJavaVM);
                // register the java vm at the uno runtime
                uno_getEnvironment(&_pJava_environment, java.pData, _pVMContext);
                // listen for changes in the configuration, e.g. proxy settings.
                registerConfigChangesListener();
            }
            else
            {
                // Java is not enabled. Notify the user via the XInteractionHandler.
                // If the client selects retry then we jump back to the retry label,otherwise we
                // throw a JavaDisableException.
                JavaDisabledException exc(OUSTR("JavaVirtualMachine_Impl::getJavaVM " \
                     "failed because Java is deactivated in the configuration"),
                     Reference< XInterface >());
                Any anyExc;
                anyExc <<= exc;
                SelectedAction  action= doClientInteraction( anyExc);
                switch( action)
                {
                case action_abort:
                    if( pJvm)
                        delete pJvm;
                    throw exc;
                case action_retry: goto retry;
                default: goto retry;
                }
            }
            if( pJvm)
                delete pJvm;
        }
    }

    Any any;
    if(_pVMContext)
    {
        if(sizeof(_pVMContext->_pJavaVM) == sizeof(sal_Int32)) { // 32 bit system?
            sal_Int32 nP = (sal_Int32)_pVMContext->_pJavaVM;
            any <<= nP;
        }
        else if(sizeof(_pVMContext->_pJavaVM) == sizeof(sal_Int64)) { // 64 bit system?
            sal_Int64 nP = (sal_Int64)_pVMContext->_pJavaVM;
            any <<= nP;
        }
    }

    return any;
}

// XJavaVM
sal_Bool JavaVirtualMachine_Impl::isVMStarted(void) throw(RuntimeException) {
    return _pVMContext != NULL;
}

// XJavaVM
sal_Bool JavaVirtualMachine_Impl::isVMEnabled(void) throw(RuntimeException) {
    JVM jvm;

    initVMConfiguration(&jvm, _xSMgr, _xCtx);

    return jvm.isEnabled();
}

// XJavaThreadRegister_11
sal_Bool JavaVirtualMachine_Impl::isThreadAttached(void) throw (RuntimeException) {
    if(!_pVMContext)
        throw RuntimeException(OUSTR("JavaVirtualMachine_Impl::isThreadAttached - not vm context"), Reference<XInterface>());

    return _pVMContext->isThreadAttached();
}

// XJavaThreadRegister_11
void JavaVirtualMachine_Impl::registerThread(void) throw (RuntimeException) {
    if(!_pVMContext)
        throw RuntimeException(OUSTR("JavaVirtualMachine_Impl::registerThread - not vm context"), Reference<XInterface>());

    _pVMContext->registerThread();
}

// XJavaThreadRegister_11
void JavaVirtualMachine_Impl::revokeThread(void) throw (RuntimeException) {
    if(!_pVMContext)
        throw RuntimeException(OUSTR("JavaVirtualMachine_Impl::revokeThread - not vm context"), Reference<XInterface>());

    _pVMContext->revokeThread();
}


// JavaVirtualMachine_Impl_CreateInstance()
static Reference<XInterface> SAL_CALL JavaVirtualMachine_Impl_createInstance(const Reference<XComponentContext> & xCtx)
    throw (RuntimeException)
{
    Reference< XInterface > xRet;
    {
        MutexGuard guard( javavm_getMutex() );
        // The javavm is never destroyed !
        static Reference< XInterface > *pStaticRef = 0;
        if( pStaticRef )
        {
            xRet = *pStaticRef;
        }
        else
            {
                xRet = *new JavaVirtualMachine_Impl( xCtx);
                pStaticRef = new Reference< XInterface> ( xRet );
            }
    }

        return xRet;
}

/*We listen to changes in the configuration. For example, the user changes the proxy
  settings in the options dialog (menu tools). Then we are notified of this change and
  if the java vm is already running we change the properties (System.lang.System.setProperties)
  through JNI.
  To receive notifications this class implements XContainerListener.
*/
void JavaVirtualMachine_Impl::registerConfigChangesListener()
{
    try
    {
        Reference< XMultiServiceFactory > xConfigProvider(
            _xSMgr->createInstanceWithContext( OUSTR(
                "com.sun.star.configuration.ConfigurationProvider"), _xCtx), UNO_QUERY);

        if (xConfigProvider.is())
        {
            // We register this instance as listener to changes in org.openoffice.Inet/Settings
            // arguments for ConfigurationAccess
            Sequence< Any > aArguments(2);
            aArguments[0] <<= PropertyValue(
                OUSTR("nodepath"),
                0,
                makeAny(OUSTR("org.openoffice.Inet/Settings")),
                PropertyState_DIRECT_VALUE);
            // depth: -1 means unlimited
            aArguments[1] <<= PropertyValue(
                OUSTR("depth"),
                0,
                makeAny( (sal_Int32)-1),
                PropertyState_DIRECT_VALUE);

            _xConfigurationAccess= xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                aArguments);
            Reference< XContainer > xContainer(_xConfigurationAccess, UNO_QUERY);

            if (xContainer.is())
                xContainer->addContainerListener( static_cast< XContainerListener* >(this));

            // now register as listener to changes in org.openoffice.Java/VirtualMachine
            Sequence< Any > aArguments2(2);
            aArguments2[0] <<= PropertyValue(
                OUSTR("nodepath"),
                0,
                makeAny(OUSTR("org.openoffice.Office.Java/VirtualMachine")),
                PropertyState_DIRECT_VALUE);
            // depth: -1 means unlimited
            aArguments2[1] <<= PropertyValue(
                OUSTR("depth"),
                0,
                makeAny( (sal_Int32)-1),
                PropertyState_DIRECT_VALUE);

            _xConfigurationAccess2= xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                aArguments2);

            Reference< XContainer > xContainer2(_xConfigurationAccess2, UNO_QUERY);

            if (xContainer2.is())
                xContainer2->addContainerListener( static_cast< XContainerListener* >(this));

            // The JavaVM service is registered as listener with the configuration service. That
            // service therefore keeps a reference of JavaVM. We need to unregister JavaVM with the
            // configuration service, otherwise the ref count of JavaVM won't drop to zero.
            Reference< XComponent > xComp( _xSMgr, UNO_QUERY);
            if (xComp.is())
            {
                xComp->addEventListener( static_cast< XEventListener* >(this));
            }
        }
    }catch( Exception & e)
    {
#ifdef DEBUG
        OString message = OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE("javavm.cxx: could not set up listener for Configuration because of >%s<", message.getStr());
#endif
    }
}

JavaVirtualMachine_Impl::SelectedAction JavaVirtualMachine_Impl::doClientInteraction( Any& except)
{
    Reference<XCurrentContext> context= getCurrentContext();
    if( context.is())
    {
        Any val= context->getValueByName(OUSTR(INTERACTION_HANDLER_NAME));

        Reference<XInteractionHandler> handler;
        val >>= handler;
        if( handler.is())
        {
            Reference<XInterface> xIntJVM( static_cast<XWeak*>(this), UNO_QUERY);
            Reference<XInteractionRequest> request(
                new InteractionRequest(xIntJVM,
                                       static_cast<JavaVirtualMachine_Impl*>(this),except));

            m_bInteractionAbort= sal_False;
            m_bInteractionRetry= sal_False;
            handler->handle(request);

            if( m_bInteractionAbort)
                return action_abort;
            else if( m_bInteractionRetry)
                return action_retry;
        }
    }
    return action_abort;
}
void JavaVirtualMachine_Impl::selectAbort(  )
{
   m_bInteractionAbort= sal_True;
}

void JavaVirtualMachine_Impl::selectRetry(  )
{
    m_bInteractionRetry= sal_True;
}



void JavaVirtualMachine_Impl::disposeJavaVM() throw() {
    if (_pVMContext){
//          pJavaVM->DestroyJavaVM();
//              _pJavaVM = NULL;
    }
}
}

using namespace stoc_javavm;

static struct ImplementationEntry g_entries[] =
{
    {
        JavaVirtualMachine_Impl_createInstance, javavm_getImplementationName,
        javavm_getSupportedServiceNames, createSingleComponentFactory,
        0 , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
// NOTE: component_canUnload is not exported, as the library cannot be unloaded.

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}

void abort_handler()
{
    // If we are within JNI_CreateJavaVM then we jump back into getJavaVM
    if( g_bInGetJavaVM )
    {
        fprintf( stderr, "JavaVM: JNI_CreateJavaVM called _exit, caught by abort_handler in javavm.cxx\n");
        longjmp( jmp_jvm_abort, 0);
    }
}
