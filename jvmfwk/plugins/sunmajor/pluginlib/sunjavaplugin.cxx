/*************************************************************************
 *
 *  $RCSfile: sunjavaplugin.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:51:38 $
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
#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

#include "external/boost/scoped_array.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "rtl/instance.hxx"
#include "osl/getglobalmutex.hxx"
#include <setjmp.h>
#include <signal.h>
#include <stack>

#include "jni.h"
#include "rtl/byteseq.hxx"
#include "jvmfwk/vendorplugin.h"
#include "util.hxx"
#include "sunversion.hxx"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SUN_MICRO "Sun Microsystems Inc."

using namespace osl;
using namespace rtl;
using namespace std;
using namespace jfw_plugin;

namespace {

struct Init
{
    osl::Mutex * operator()()
        {
            static osl::Mutex aInstance;
            return &aInstance;
        }
};
osl::Mutex * getPluginMutex()
{
    return rtl_Instance< osl::Mutex, Init, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            Init(), ::osl::GetGlobalMutex());
}

#if defined UNX
OString getPluginJarPath(
    const OUString & sVendor,
    const OUString& sLocation,
    const OUString& sVersion)
{
    OString ret;
    OUString sName1(RTL_CONSTASCII_USTRINGPARAM("javaplugin.jar"));
    OUString sName2(RTL_CONSTASCII_USTRINGPARAM("plugin.jar"));
    OUString sPath;
    if (sVendor.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SUN_MICRO))))
    {
        SunVersion ver142("1.4.2-ea");
        SunVersion ver150("1.5.0-ea");
        SunVersion ver(sVersion);
        OSL_ASSERT(ver142 && ver150 && ver);

        OUString sName;
        if (ver < ver142)
        {
            sName = sName1;
        }
        else if (ver < ver150)
        {//this will cause ea, beta etc. to have plugin.jar in path.
            //but this does not harm. 1.5.0-beta < 1.5.0
            sName = sName2;
        }
        if (sName.getLength())
        {
            sName = sLocation + OUSTR("/lib/") + sName;
            OSL_VERIFY(
                osl_getSystemPathFromFileURL(sName.pData, & sPath.pData)
                == osl_File_E_None);
        }
    }
    else
    {
        char sep[] =  {SAL_PATHSEPARATOR, 0};
        OUString sName(sLocation + OUSTR("/lib/") + sName1);
        OUString sPath1;
        OUString sPath2;
        bool bOk = false;
        if (osl_getSystemPathFromFileURL(sName.pData, & sPath1.pData)
            == osl_File_E_None)
        {
            sName = sLocation + OUSTR("/lib/") + sName2;
            if (osl_getSystemPathFromFileURL(sName.pData, & sPath2.pData)
                == osl_File_E_None)
            {
                sPath = sPath1 + OUString::createFromAscii(sep) + sPath2;
            }
        }
        OSL_ASSERT(sPath.getLength());
    }
    ret = rtl::OUStringToOString(sPath, osl_getThreadTextEncoding());

    return ret;
}
#endif // UNX


JavaInfo* createJavaInfo(const rtl::Reference<VendorBase> & info)
{
    JavaInfo* pInfo = (JavaInfo*) rtl_allocateMemory(sizeof(JavaInfo));
    if (pInfo == NULL)
        return NULL;
    rtl::OUString sVendor = info->getVendor();
    pInfo->sVendor = sVendor.pData;
    rtl_uString_acquire(sVendor.pData);
    rtl::OUString sHome = info->getHome();
    pInfo->sLocation = sHome.pData;
    rtl_uString_acquire(pInfo->sLocation);
    rtl::OUString sVersion = info->getVersion();
    pInfo->sVersion = sVersion.pData;
    rtl_uString_acquire(pInfo->sVersion);
    pInfo->nFeatures = info->supportsAccessibility() ? 1 : 0;
    pInfo->nRequirements = info->needsRestart() ? JFW_REQUIRE_NEEDRESTART : 0;
    rtl::OUStringBuffer buf(1024);
    buf.append(info->getRuntimeLibrary());
    if (info->getLibraryPaths().getLength() > 0)
    {
        buf.appendAscii("\n");
        buf.append(info->getLibraryPaths());
        buf.appendAscii("\n");
    }

    rtl::OUString sVendorData = buf.makeStringAndClear();
    rtl::ByteSequence byteSeq( (sal_Int8*) sVendorData.pData->buffer,
                               sVendorData.getLength() * sizeof(sal_Unicode));
    pInfo->arVendorData = byteSeq.get();
    rtl_byte_sequence_acquire(pInfo->arVendorData);

    return pInfo;
}

rtl::OUString getRuntimeLib(const rtl::ByteSequence & data)
{
    const sal_Unicode* chars = (sal_Unicode*) data.getConstArray();
    sal_Int32 len = data.getLength();
    rtl::OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    rtl::OUString aToken = sData.getToken( 0, '\n', index);

    return aToken;
}

jmp_buf jmp_jvm_abort;
sig_atomic_t g_bInGetJavaVM = 0;

void abort_handler()
{
    // If we are within JNI_CreateJavaVM then we jump back into getJavaVM
    if( g_bInGetJavaVM != 0 )
    {
        fprintf( stderr, "JavaVM: JNI_CreateJavaVM called _exit, caught by abort_handler in javavm.cxx\n");
        longjmp( jmp_jvm_abort, 0);
    }
}

}

extern "C"
javaPluginError jfw_plugin_getAllJavaInfos(
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList)
{
    if (parJavaInfo == NULL || nLenInfoList == NULL)
        return JFW_PLUGIN_E_INVALID_ARG;
    JavaInfo** arInfo = NULL;
    try
    {
        //check if we know all the required features
        vector<OUString> vecExclude;
        for (int i = 0; i != nLenList; i++)
        {
            vecExclude.push_back(arExcludeList[i]);
        }

        std::vector<rtl::Reference<VendorBase> > vec =
            getAllJREInfos(sVendor, sMinVersion, sMaxVersion, vecExclude);

        arInfo = (JavaInfo**) rtl_allocateMemory(vec.size() * sizeof (JavaInfo*));

        int j = 0;
        typedef vector<rtl::Reference<VendorBase> >::const_iterator cit;
        for (cit ii = vec.begin(); ii != vec.end(); ii++, j++)
        {
            arInfo[j] = createJavaInfo(*ii);
        }
        *nLenInfoList = vec.size();
    }
    catch(MalformedVersionException&)
    {
        return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
    }

    *parJavaInfo = arInfo;
    return JFW_PLUGIN_E_NONE;
}

extern "C"
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  *  *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo)
{
    javaPluginError errcode = JFW_PLUGIN_E_NONE;
    if (ppInfo == NULL)
        return JFW_PLUGIN_E_INVALID_ARG;
    try
    {
        rtl::OUString sPath((rtl_uString*)path);
        rtl::Reference<VendorBase> aVendorInfo =
            getJREInfoByPath(sPath);

        if (aVendorInfo.is() == sal_False)
            return JFW_PLUGIN_E_NO_JRE;
        //check if the version meets the requirements
        rtl::OUString sTheMinVersion((rtl_uString*) sMinVersion);

         if (sTheMinVersion.getLength() > 0)
         {
             int nRes = aVendorInfo->compareVersions(sTheMinVersion);
             if (nRes < 0)
                 return JFW_PLUGIN_E_FAILED_VERSION;
         }

         rtl::OUString sTheMaxVersion((rtl_uString*) sMaxVersion);
         if (sTheMaxVersion.getLength() > 0)
         {
             int nRes = aVendorInfo->compareVersions(sTheMaxVersion);
             if (nRes > 0)
                 return JFW_PLUGIN_E_FAILED_VERSION;
         }

         if (arExcludeList > 0)
         {
             for (int i = 0; i < nLenList; i++)
             {
                 rtl::OUString sExVer((rtl_uString*) arExcludeList[i]);
                 int nRes = aVendorInfo->compareVersions(sExVer);
                 if (nRes == 0)
                     return JFW_PLUGIN_E_FAILED_VERSION;
             }
         }

         *ppInfo = createJavaInfo(aVendorInfo);
    }
    catch(MalformedVersionException& )
    {
        errcode = JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
    }
    catch(...)
    {
        errcode = JFW_PLUGIN_E_ERROR;
    }
    return errcode;
}

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.
    </p>
 */
extern "C"
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo,
    const JavaVMOption* arOptions,
    sal_Int32 cOptions,
    JavaVM ** ppVm,
    JNIEnv ** ppEnv)
{
    osl::MutexGuard guard(getPluginMutex());
    javaPluginError errcode = JFW_PLUGIN_E_NONE;
    if ( pInfo == NULL || ppVm == NULL || ppEnv == NULL)
        return JFW_PLUGIN_E_INVALID_ARG;
    rtl::OUString sVendor(pInfo->sVendor);
    rtl::OUString sSunVendor(OUSTR(SUN_MICRO));
    if (sVendor.equals(sSunVendor) == sal_False)
        return JFW_PLUGIN_E_WRONG_VENDOR;
    // On linux we load jvm with RTLD_GLOBAL. This is necessary for debugging, because
    // libjdwp.so need a symbol (fork1) from libjvm which it only gets if the jvm is loaded
    // witd RTLD_GLOBAL. On Solaris libjdwp.so is correctly linked with libjvm.so
    rtl::OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
    oslModule moduleRt =
        osl_loadModule(sRuntimeLib.pData, SAL_LOADMODULE_DEFAULT);
#if defined(LINUX)
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData,
                                   SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_NOW)) == 0 )
#else
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData, SAL_LOADMODULE_DEFAULT)) == 0)
#endif
     {
         OSL_ASSERT(0);
         rtl::OString msg = rtl::OUStringToOString(
             sRuntimeLib, osl_getThreadTextEncoding());
         fprintf(stderr,"Could not load java runtime library : %s",
                 msg.getStr());
         return JFW_PLUGIN_E_VM_CREATION_FAILED;
     }

#ifdef UNX
    //Setting the JAVA_HOME is needed for awt
    rtl::OUString javaHome(RTL_CONSTASCII_USTRINGPARAM("JAVA_HOME="));
    rtl::OUString sPathLocation;
    osl_getSystemPathFromFileURL(pInfo->sLocation, & sPathLocation.pData);
    javaHome += sPathLocation;
    rtl::OString osJavaHome = rtl::OUStringToOString(
        javaHome, osl_getThreadTextEncoding());
    putenv(strdup(osJavaHome.getStr()));
#if OSL_DEBUG_LEVEL >=2
    fprintf(stderr, "Setting JAVA_HOME: %s\n", osJavaHome.getStr());
#endif
#endif

    typedef jint JNICALL JNI_InitArgs_Type(void *);
    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);
    rtl::OUString sSymbolCreateJava(
            RTL_CONSTASCII_USTRINGPARAM("JNI_CreateJavaVM"));

    JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *) osl_getSymbol(
        moduleRt, sSymbolCreateJava.pData);
    if (!pCreateJavaVM)
    {
        OSL_ASSERT(0);
        rtl::OString sLib = rtl::OUStringToOString(
            sRuntimeLib, osl_getThreadTextEncoding());
        rtl::OString sSymbol = rtl::OUStringToOString(
            sSymbolCreateJava, osl_getThreadTextEncoding());
        fprintf(stderr,"Java runtime library: %s does not export symbol %s !",
                sLib.getStr(), sSymbol.getStr());
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
    }

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

    // Some testing with Java 1.4 showed that JavaVMOption.optionString has to
    // be encoded with the system encoding (i.e., osl_getThreadTextEncoding):
    JavaVMInitArgs vm_args;


    boost::scoped_array<JavaVMOption> sarOptions(
        new JavaVMOption[cOptions + 1]);
    JavaVMOption * options = sarOptions.get();

    // We set an abort handler which is called when the VM calls _exit during
    // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
    // all some directories of the Java installation. This is necessary for
    // all versions below 1.5.1
    options[0].optionString= "abort";
    options[0].extraInfo= (void* )abort_handler;
    int index = 1;
    rtl::OString sClassPathProp("-Djava.class.path=");
    rtl::OString sClassPathOption;
    for (int i = 0; i < cOptions; i++)
    {
#ifdef UNX
    // Until java 1.5 we need to put a plugin.jar or javaplugin.jar (<1.4.2)
    // in the class path in order to have applet support.
        rtl::OString sClassPath = arOptions[i].optionString;
        if (sClassPath.match(sClassPathProp, 0) == sal_True)
        {
            char sep[] =  {SAL_PATHSEPARATOR, 0};
            sClassPathOption = sClassPath + rtl::OString(sep) +
                getPluginJarPath(sVendor, pInfo->sLocation,pInfo->sVersion);
            options[i+1].optionString = (char *) sClassPathOption.getStr();
            options[i+1].extraInfo = arOptions[i].extraInfo;
        }
        else
        {
#endif
            options[i+1].optionString = arOptions[i].optionString;
            options[i+1].extraInfo = arOptions[i].extraInfo;
#ifdef UNX
        }
#endif
#if OSL_DEBUG_LEVEL >= 2
        fprintf(stderr, "VM option: %s\n", options[i+1].optionString);
#endif

    }

    vm_args.version= JNI_VERSION_1_2;
    vm_args.options= options;
    vm_args.nOptions= cOptions + 1;
    vm_args.ignoreUnrecognized= JNI_TRUE;

    /* We set a global flag which is used by the abort handler in order to
       determine whether it is  should use longjmp to get back into this function.
       That is, the abort handler determines if it is on the same stack as this function
       and then jumps back into this function.
    */
    g_bInGetJavaVM = 1;
    jint err;
    JavaVM * pJavaVM;
    memset( jmp_jvm_abort, 0, sizeof(jmp_jvm_abort));
    int jmpval= setjmp( jmp_jvm_abort );
    /* If jmpval is not "0" then this point was reached by a longjmp in the
       abort_handler, which was called indirectly by JNI_CreateVM.
    */
    if( jmpval == 0)
    {
        //returns negative number on failure
        err= pCreateJavaVM(&pJavaVM, ppEnv, &vm_args);
        g_bInGetJavaVM = 0;
    }
    else
        // set err to a positive number, so as or recognize that an abort (longjmp)
        //occurred
        err= 1;

    if(err != 0)
    {
        rtl::OUString message;
        if( err < 0)
        {
            fprintf(stderr,"Can not create Java Virtual Machine");
            errcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
        else if( err > 0)
        {
            fprintf(stderr,"Can not create JavaVirtualMachine, abort handler was called");
            errcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
    }
        else
    {
        *ppVm = pJavaVM;
    }


   return errcode;
}



