/*************************************************************************
 *
 *  $RCSfile: javavm.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:35:12 $
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

#include <time.h>


#include <osl/diagnose.h>
#include <osl/file.hxx>

#include <rtl/process.h>


#include <vos/module.hxx>
#include <vos/thread.hxx>
#include <vos/conditn.hxx>
//#include <vos/profile.hxx>

#include <uno/environment.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>

#include <bridges/java/jvmcontext.hxx>

#include "jvmargs.hxx"

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
#define DEF_JAVALIB "javai.dll"

#define TIMEZONE "MET"


#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::java;
using namespace com::sun::star::registry;

using namespace vos;
using namespace rtl;
using namespace cppu;
using namespace osl;

namespace stoc_javavm {
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

    class OCreatorThread : public OThread {
        JavaVirtualMachine_Impl * _pJavaVirtualMachine_Impl;
        JavaVM                  * _pJVM;

        OCondition _start_Condition;
        OCondition _wait_Condition;

        JVM _jvm;
        RuntimeException _runtimeException;

    protected:
        virtual void SAL_CALL run() throw();

    public:
        OCreatorThread(JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl) throw();
        JavaVM * createJavaVM(const JVM & jvm) throw (RuntimeException);
        void disposeJavaVM() throw();

    };

    class JavaVirtualMachine_Impl : public WeakImplHelper3<XJavaVM, XJavaThreadRegister_11, XServiceInfo> {
        Mutex                           _Mutex;

        OCreatorThread                  _creatorThread;

        uno_Environment               * _pJava_environment;
        JavaVMContext                 * _pVMContext;

        Reference<XMultiServiceFactory> _xSMgr;

        OModule    _javaLib;

    public:
        OUString _error;

        JavaVirtualMachine_Impl(const Reference<XMultiServiceFactory> & rSMgr) throw();
        ~JavaVirtualMachine_Impl() throw();

        // XJavaVM
        virtual Any      SAL_CALL getJavaVM(const Sequence<sal_Int8> & processID)   throw(RuntimeException);
        virtual sal_Bool SAL_CALL isVMStarted(void)                                 throw( RuntimeException);
        virtual sal_Bool SAL_CALL isVMEnabled(void)                                 throw( RuntimeException);

        // XJavaThreadRegister_11
        virtual sal_Bool SAL_CALL isThreadAttached(void) throw(RuntimeException);
        virtual void     SAL_CALL registerThread(void)   throw(RuntimeException);
        virtual void     SAL_CALL revokeThread(void)     throw(RuntimeException);

        // XServiceInfo
        virtual OUString           SAL_CALL getImplementationName()                      throw(RuntimeException);
        virtual sal_Bool           SAL_CALL supportsService(const OUString& ServiceName) throw(RuntimeException);
        virtual Sequence<OUString> SAL_CALL getSupportedServiceNames(void)               throw(RuntimeException);


        static OUString SAL_CALL getImplementationName_Static() throw() {
            return OUString::createFromAscii("com.sun.star.comp.stoc.JavaVirtualMachine");
        }

        static Sequence<OUString> SAL_CALL getSupportedServiceNames_Static() throw() {
            Sequence<OUString> aSNS(1);
            aSNS.getArray()[0] = OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine");
            return aSNS;
        }

        JavaVM *                createJavaVM(const JVM & jvm) throw(RuntimeException);
        void                    disposeJavaVM() throw();
    };

    OCreatorThread::OCreatorThread(JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl) throw()
        : _pJVM(NULL)
    {
        _pJavaVirtualMachine_Impl = pJavaVirtualMachine_Impl;
    }

    void OCreatorThread::run() throw() {
        _start_Condition.wait();
        _start_Condition.reset();

        try {
            _pJVM = _pJavaVirtualMachine_Impl->createJavaVM(_jvm);
        }
        catch(RuntimeException & runtimeException) {
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

        if (!_pJVM) {
            create();

            _start_Condition.set();

            _wait_Condition.wait();
            _wait_Condition.reset();

            if(!_pJVM)
                throw _runtimeException;
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
    OUString SAL_CALL JavaVirtualMachine_Impl::getImplementationName() throw(RuntimeException) {
        return JavaVirtualMachine_Impl::getImplementationName_Static();
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
    Sequence<OUString> SAL_CALL JavaVirtualMachine_Impl::getSupportedServiceNames() throw(RuntimeException) {
        return getSupportedServiceNames_Static();
    }


    static void getDefaultLocaleFromConfig(JVM * pjvm, const Reference<XMultiServiceFactory> & xSMgr) throw(Exception) {
        Reference<XInterface> xConfRegistry = xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationRegistry")));
        if(!xConfRegistry.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

        Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
        if(!xConfRegistry_simple.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

          xConfRegistry_simple->open(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile")), sal_True, sal_False);
          Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

        // read ftp proxy name
        Reference<XRegistryKey> locale = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("International/Locale")));
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

    static void getINetPropsFromConfig(JVM * pjvm, const Reference<XMultiServiceFactory> & xSMgr) throw (Exception) {
        Reference<XInterface> xConfRegistry = xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationRegistry")));
        if(!xConfRegistry.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

        Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
        if(!xConfRegistry_simple.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

          xConfRegistry_simple->open(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Inet")), sal_True, sal_False);
          Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

        // read ftp proxy name
        Reference<XRegistryKey> ftpProxy_name = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Proxy/FTP/Name")));
        if(ftpProxy_name.is() && ftpProxy_name->getStringValue().getLength()) {
            OUString ftpHost = OUString(RTL_CONSTASCII_USTRINGPARAM("ftp.proxyHost="));
            ftpHost += ftpProxy_name->getStringValue();

            // read ftp proxy port
            Reference<XRegistryKey> ftpProxy_port = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Proxy/FTP/Port")));
            if(ftpProxy_port.is() && ftpProxy_port->getLongValue()) {
                OUString ftpPort = OUString(RTL_CONSTASCII_USTRINGPARAM("ftp.proxyPort="));
                ftpPort += OUString::valueOf(ftpProxy_port->getLongValue());

                pjvm->pushProp(ftpHost);
                pjvm->pushProp(ftpPort);
            }
        }

        // read http proxy name
        Reference<XRegistryKey> httpProxy_name = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Proxy/HTTP/Name")));
        if(httpProxy_name.is() && httpProxy_name->getStringValue().getLength()) {
            OUString httpHost = OUString(RTL_CONSTASCII_USTRINGPARAM("http.proxyHost="));
            httpHost += httpProxy_name->getStringValue();

            // read http proxy port
            Reference<XRegistryKey> httpProxy_port = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Proxy/HTTP/Port")));
            if(httpProxy_port.is() && httpProxy_port->getLongValue()) {
                OUString httpPort = OUString(RTL_CONSTASCII_USTRINGPARAM("http.proxyPort="));
                httpPort += OUString::valueOf(httpProxy_port->getLongValue());

                pjvm->pushProp(httpHost);
                pjvm->pushProp(httpPort);
            }
        }

        xConfRegistry_simple->close();
    }

    static void getJavaPropsFromConfig(JVM * pjvm, const Reference<XMultiServiceFactory> & xSMgr) throw(Exception) {
        Reference<XInterface> xConfRegistry = xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationRegistry")));
        if(!xConfRegistry.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

        Reference<XSimpleRegistry> xConfRegistry_simple(xConfRegistry, UNO_QUERY);
        if(!xConfRegistry_simple.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get ConfigurationRegistry")), Reference<XInterface>());

        xConfRegistry_simple->open(OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.UserProfile")), sal_True, sal_False);
        Reference<XRegistryKey> xRegistryRootKey = xConfRegistry_simple->getRootKey();

        Reference<XRegistryKey> key_InstallPath = xRegistryRootKey->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Office/InstallPath")));
        if(!key_InstallPath.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: can not find key: Office/InstallPath in org.openoffice.UserProfile")),
                                                         Reference<XInterface>());

        OUString rcPath = key_InstallPath->getStringValue();

        // normalize the path
        OUString norm_rcPath;
        File::normalizePath(rcPath, norm_rcPath);
        norm_rcPath += OUString(RTL_CONSTASCII_USTRINGPARAM("/config/" INI_FILE));

        Reference<XInterface> xIniManager(xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.config.INIManager"))));
        if(!xIniManager.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get: com.sun.star.config.INIManager")), Reference<XInterface>());

        Reference<XSimpleRegistry> xIniManager_simple(xIniManager, UNO_QUERY);
        if(!xIniManager_simple.is()) throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: couldn't get: com.sun.star.config.INIManager")), Reference<XInterface>());

        OUString tmpJavarc;
        File::getFileURLFromNormalizedPath(norm_rcPath, tmpJavarc);
        xIniManager_simple->open(tmpJavarc, sal_True, sal_False);

        Reference<XRegistryKey> xJavaSection = xIniManager_simple->getRootKey()->openKey(OUString(RTL_CONSTASCII_USTRINGPARAM("Java")));
        if(!xJavaSection.is() || !xJavaSection->isValid())
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.cxx: can not find java section in " INI_FILE)), Reference<XInterface>());

        Sequence<OUString> javaProperties = xJavaSection->getKeyNames();
        OUString * pSectionEntry = javaProperties.getArray();
        sal_Int32 nCount         = javaProperties.getLength();

        for(sal_Int32 i=0; i < nCount; ++ i) {
            OUString entryValue = (xJavaSection->openKey(pSectionEntry[i]))->getStringValue();

            if(entryValue.len()) {
                pSectionEntry[i] += OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                pSectionEntry[i] += entryValue;
            }

            pjvm->pushProp(pSectionEntry[i]);
        }

          xIniManager_simple->close();
    }

    static void getJavaPropsFromEnvironment(JVM * pjvm) throw() {
        // try some defaults for CLASSPATH and runtime lib
        const char * pClassPath = getenv("CLASSPATH");
        pjvm->setSystemClasspath(OUString::createFromAscii(pClassPath));

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

    static void initVMConfiguration(JVM * pjvm, const Reference<XMultiServiceFactory> & xSMgr) throw() {
        try {
            JVM jvm;

            getINetPropsFromConfig(&jvm, xSMgr);

            *pjvm = jvm;
        }
        catch(Exception & exception) {
#ifdef DEBUG
            OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("javavm.cxx: can not get INetProps cause of >%s<", message.getStr());
#endif
        }

        try {
            JVM jvm;

            getDefaultLocaleFromConfig(&jvm, xSMgr);
        }
        catch(Exception & exception) {
#ifdef DEBUG
            OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("javavm.cxx: can not get locale cause of >%s<", message.getStr());
#endif
        }

        try {

            JVM jvm;

            getJavaPropsFromConfig(&jvm, xSMgr);

            *pjvm = jvm;
        }
        catch(Exception & exception) {
#ifdef DEBUG
            OString message = OUStringToOString(exception.Message, RTL_TEXTENCODING_ASCII_US);
            OSL_TRACE("javavm.cxx: couldn't use configuration cause of >%s<", message.getStr());
#endif
            JVM jvm;

            getJavaPropsFromEnvironment(&jvm);
            *pjvm = jvm;
        }
        setTimeZone(pjvm);

        pjvm->setPrint(vm_vfprintf);
        pjvm->setExit(vm_exit);
        pjvm->setAbort(vm_abort);
    }

    JavaVirtualMachine_Impl::JavaVirtualMachine_Impl(const Reference<XMultiServiceFactory> & rSMgr) throw()
        : _pVMContext(NULL),
           _xSMgr(rSMgr),
          _creatorThread(this),
           _pJava_environment(NULL)
    {
    }

    JavaVirtualMachine_Impl::~JavaVirtualMachine_Impl() throw() {
        if (_pVMContext)
            _creatorThread.disposeJavaVM();
    }


    JavaVM * JavaVirtualMachine_Impl::createJavaVM(const JVM & jvm) throw(RuntimeException) {
        if(!_javaLib.load(jvm.getRuntimeLib())) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::createJavaVM - cannot find java runtime: "));
            message += jvm.getRuntimeLib();

            throw RuntimeException(message, Reference<XInterface>());
        }

        JNI_InitArgs_Type * initArgs = (JNI_InitArgs_Type *)_javaLib.getSymbol(OUString::createFromAscii("JNI_GetDefaultJavaVMInitArgs"));
        JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *)_javaLib.getSymbol(OUString::createFromAscii("JNI_CreateJavaVM"));
        if (!initArgs || !pCreateJavaVM) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::createJavaVM - cannot find symbols: JNI_GetDefaultJavaVMInitArgs or JNI_CreateJavaVM "));

            throw RuntimeException(message, Reference<XInterface>());
        }

        JDK1_1InitArgs vm_args;
        initArgs(&vm_args);

        jvm.setArgs(&vm_args);

        JNIEnv * pJNIEnv = NULL;
        JavaVM * pJavaVM;
        jint err = pCreateJavaVM(&pJavaVM, &pJNIEnv, &vm_args);

        if(err) {
            OUString message(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::createJavaVM - can not create vm, cause of err:"));
            message += OUString::valueOf((sal_Int32)err);

            throw RuntimeException(message, Reference<XInterface>());
        }

        return pJavaVM;
    }

    // XJavaVM
    Any JavaVirtualMachine_Impl::getJavaVM(const Sequence<sal_Int8> & processId) throw (RuntimeException) {
        MutexGuard guarg(_Mutex);

        Sequence<sal_Int8> localProcessID(16);
        rtl_getGlobalProcessId( (sal_uInt8*) localProcessID.getArray() );

        if (localProcessID == processId && !_pVMContext) {
            if(_error.getLength()) // do we have an error?
                throw RuntimeException(_error, Reference<XInterface>());

            uno_Environment ** ppEnviroments = NULL;
            sal_Int32 size = 0;
            OUString java(OUString::createFromAscii("java"));

            uno_getRegisteredEnvironments(&ppEnviroments, &size, (uno_memAlloc)malloc, java.pData);

            if(size) { // do we found an existing java environment?
                OSL_TRACE("javavm.cxx: found an existing environment");

                _pJava_environment = ppEnviroments[0];
                _pJava_environment->acquire(_pJava_environment);
                _pVMContext = (JavaVMContext *)_pJava_environment->pContext;

                for(sal_Int32 i = 0; i  < size; ++ i)
                    ppEnviroments[i]->release(ppEnviroments[i]);

                free(ppEnviroments);
            }
            else {
                JVM jvm;
                JavaVM * pJavaVM;

                initVMConfiguration(&jvm, _xSMgr);

                if (jvm.isEnabled()) {
                    // create the java vm
                    try {
                        pJavaVM = _creatorThread.createJavaVM(jvm);
                    }
                    catch(RuntimeException & runtimeException) {
                        // save the error message
                        _error = runtimeException.Message;

                        throw;
                    }

                    // create a context
                    _pVMContext = new JavaVMContext(pJavaVM);

                    // register the java vm at the uno runtime
                    uno_getEnvironment(&_pJava_environment, java.pData, _pVMContext);
                }
            }
        }

        Any any;
        if(_pVMContext) {
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

        initVMConfiguration(&jvm, _xSMgr);

        return jvm.isEnabled();
    }

    // XJavaThreadRegister_11
    sal_Bool JavaVirtualMachine_Impl::isThreadAttached(void) throw (RuntimeException) {
        if(!_pVMContext)
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::isThreadAttached - not vm context")), Reference<XInterface>());

        return _pVMContext->isThreadAttached();
    }

    // XJavaThreadRegister_11
    void JavaVirtualMachine_Impl::registerThread(void) throw (RuntimeException) {
        if(!_pVMContext)
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::registerThread - not vm context")), Reference<XInterface>());

        _pVMContext->registerThread();
    }

    // XJavaThreadRegister_11
    void JavaVirtualMachine_Impl::revokeThread(void) throw (RuntimeException) {
        if(!_pVMContext)
            throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("JavaVirtualMachine_Impl::revokeThread - not vm context")), Reference<XInterface>());

        _pVMContext->revokeThread();
    }


    // JavaVirtualMachine_Impl_CreateInstance()
    static Reference<XInterface> SAL_CALL JavaVirtualMachine_Impl_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
     {
        XJavaVM *pJVM= static_cast<XJavaVM *>(new JavaVirtualMachine_Impl(rSMgr));
        return Reference<XInterface>(pJVM);
    }



    void JavaVirtualMachine_Impl::disposeJavaVM() throw() {
        if (_pVMContext){
//          pJavaVM->DestroyJavaVM();
//              _pJavaVM = NULL;
        }
    }
}

extern "C" {
    void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv){
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey) throw() {
        if (pRegistryKey) {
            try     {
                Reference<XRegistryKey> xNewKey(reinterpret_cast<XRegistryKey *>(pRegistryKey)->createKey(
                    OUString::createFromAscii("/") +
                    stoc_javavm::JavaVirtualMachine_Impl::getImplementationName_Static() +
                    OUString::createFromAscii("/UNO/SERVICES")));

                const Sequence<OUString> & rSNL = stoc_javavm::JavaVirtualMachine_Impl::getSupportedServiceNames_Static();
                const OUString * pArray = rSNL.getConstArray();
                for(sal_Int32 nPos = rSNL.getLength(); nPos--;)
                    xNewKey->createKey(pArray[nPos]);

                return sal_True;
            }
            catch (InvalidRegistryException &) {
                OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey) throw() {
        void * pRet = 0;

        if (stoc_javavm::JavaVirtualMachine_Impl::getImplementationName_Static().equals(OUString::createFromAscii(pImplName))) {
            Reference<XSingleServiceFactory> xFactory( createOneInstanceFactory(
                reinterpret_cast<XMultiServiceFactory *>(pServiceManager),
                stoc_javavm::JavaVirtualMachine_Impl::getImplementationName_Static(),
                stoc_javavm::JavaVirtualMachine_Impl_createInstance,
                stoc_javavm::JavaVirtualMachine_Impl::getSupportedServiceNames_Static()));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }

        return pRet;
    }
}
