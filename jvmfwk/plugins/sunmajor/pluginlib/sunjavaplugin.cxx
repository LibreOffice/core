/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifdef WNT
# include <stdio.h>
# include <sys/stat.h>
# include <windows.h>
#endif

#ifdef ANDROID
# include <dlfcn.h>
#endif

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif
#include <string.h>

#include "boost/scoped_array.hpp"
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
#include "vendorlist.hxx"
#include "diagnostics.h"

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#else
#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/valgrind.h>
#else
#define RUNNING_ON_VALGRIND 0
#endif
#endif

#define SUN_MICRO "Sun Microsystems Inc."

using namespace osl;
using namespace std;
using namespace jfw_plugin;


namespace {

struct PluginMutex: public ::rtl::Static<osl::Mutex, PluginMutex> {};

#if defined UNX
OString getPluginJarPath(
    const OUString & sVendor,
    const OUString& sLocation,
    const OUString& sVersion)
{
    OString ret;
    OUString sName1("javaplugin.jar");
    OUString sName2("plugin.jar");
    OUString sPath;
    if ( sVendor == SUN_MICRO )
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
        if (!sName.isEmpty())
        {
            sName = sLocation + "/lib/" + sName;
            OSL_VERIFY(
                osl_getSystemPathFromFileURL(sName.pData, & sPath.pData)
                == osl_File_E_None);
        }
    }
    else
    {
        char sep[] =  {SAL_PATHSEPARATOR, 0};
        OUString sName(sLocation + "/lib/" + sName1);
        OUString sPath1;
        OUString sPath2;
        if (osl_getSystemPathFromFileURL(sName.pData, & sPath1.pData)
            == osl_File_E_None)
        {
            sName = sLocation + "/lib/" + sName2;
            if (osl_getSystemPathFromFileURL(sName.pData, & sPath2.pData)
                == osl_File_E_None)
            {
                sPath = sPath1 + OUString::createFromAscii(sep) + sPath2;
            }
        }
        OSL_ASSERT(!sPath.isEmpty());
    }
    ret = OUStringToOString(sPath, osl_getThreadTextEncoding());

    return ret;
}
#endif // UNX


JavaInfo* createJavaInfo(const rtl::Reference<VendorBase> & info)
{
    JavaInfo* pInfo = (JavaInfo*) rtl_allocateMemory(sizeof(JavaInfo));
    if (pInfo == NULL)
        return NULL;
    OUString sVendor = info->getVendor();
    pInfo->sVendor = sVendor.pData;
    rtl_uString_acquire(sVendor.pData);
    OUString sHome = info->getHome();
    pInfo->sLocation = sHome.pData;
    rtl_uString_acquire(pInfo->sLocation);
    OUString sVersion = info->getVersion();
    pInfo->sVersion = sVersion.pData;
    rtl_uString_acquire(pInfo->sVersion);
    pInfo->nFeatures = info->supportsAccessibility() ? 1 : 0;
    pInfo->nRequirements = info->needsRestart() ? JFW_REQUIRE_NEEDRESTART : 0;
    OUStringBuffer buf(1024);
    buf.append(info->getRuntimeLibrary());
    if (!info->getLibraryPaths().isEmpty())
    {
        buf.appendAscii("\n");
        buf.append(info->getLibraryPaths());
        buf.appendAscii("\n");
    }

    OUString sVendorData = buf.makeStringAndClear();
    rtl::ByteSequence byteSeq( (sal_Int8*) sVendorData.pData->buffer,
                               sVendorData.getLength() * sizeof(sal_Unicode));
    pInfo->arVendorData = byteSeq.get();
    rtl_byte_sequence_acquire(pInfo->arVendorData);

    return pInfo;
}

OUString getRuntimeLib(const rtl::ByteSequence & data)
{
    const sal_Unicode* chars = (sal_Unicode*) data.getConstArray();
    sal_Int32 len = data.getLength();
    OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    OUString aToken = sData.getToken( 0, '\n', index);

    return aToken;
}

jmp_buf jmp_jvm_abort;
sig_atomic_t g_bInGetJavaVM = 0;

extern "C" void JNICALL abort_handler()
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
    OSL_ASSERT(sVendor);
    OSL_ASSERT(sMinVersion);
    OSL_ASSERT(sMaxVersion);
    OSL_ASSERT(parJavaInfo);
    OSL_ASSERT(parJavaInfo);
    OSL_ASSERT(nLenInfoList);
    if (!sVendor || !sMinVersion || !sMaxVersion || !parJavaInfo || !nLenInfoList)
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of element in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    OUString ouVendor(sVendor);
    OUString ouMinVer(sMinVersion);
    OUString ouMaxVer(sMaxVersion);

    OSL_ASSERT(!ouVendor.isEmpty());
    if (ouVendor.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    JavaInfo** arInfo = NULL;

    //Find all JREs
    vector<rtl::Reference<VendorBase> > vecInfos =
        getAllJREInfos();
    vector<rtl::Reference<VendorBase> > vecVerifiedInfos;

    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfos.begin(); i != vecInfos.end(); ++i)
    {
        const rtl::Reference<VendorBase>& cur = *i;

        if (ouVendor.equals(cur->getVendor()) == sal_False)
            continue;

        if (!ouMinVer.isEmpty())
        {
            try
            {
                if (cur->compareVersions(sMinVersion) == -1)
                    continue;
            }
            catch (MalformedVersionException&)
            {
                //The minVersion was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,"[Java framework]sunjavaplugin does not know version: "
                    + ouMinVer + " for vendor: " + cur->getVendor()
                    + " .Check minimum Version." );
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
        }

        if (!ouMaxVer.isEmpty())
        {
            try
            {
                if (cur->compareVersions(sMaxVersion) == 1)
                    continue;
            }
            catch (MalformedVersionException&)
            {
                //The maxVersion was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,"[Java framework]sunjavaplugin does not know version: "
                    + ouMaxVer + " for vendor: " + cur->getVendor()
                    + " .Check maximum Version." );
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
        }

        bool bExclude = false;
        for (int j = 0; j < nLenList; j++)
        {
            OUString sExVer(arExcludeList[j]);
            try
            {
                if (cur->compareVersions(sExVer) == 0)
                {
                    bExclude = true;
                    break;
                }
            }
            catch (MalformedVersionException&)
            {
                //The excluded version was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,"[Java framework]sunjavaplugin does not know version: "
                    + sExVer + " for vendor: " + cur->getVendor()
                    + " .Check excluded versions." );
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
        }
        if (bExclude == true)
            continue;

        vecVerifiedInfos.push_back(*i);
    }
    //Now vecVerifiedInfos contains all those JREs which meet the version requirements
    //Transfer them into the array that is passed out.
    arInfo = (JavaInfo**) rtl_allocateMemory(vecVerifiedInfos.size() * sizeof (JavaInfo*));
    int j = 0;
    typedef vector<rtl::Reference<VendorBase> >::const_iterator cit;
    for (cit ii = vecVerifiedInfos.begin(); ii != vecVerifiedInfos.end(); ++ii, ++j)
    {
        arInfo[j] = createJavaInfo(*ii);
    }
    *nLenInfoList = vecVerifiedInfos.size();


    *parJavaInfo = arInfo;
    return JFW_PLUGIN_E_NONE;
}

extern "C"
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  *  *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo)
{
    javaPluginError errorcode = JFW_PLUGIN_E_NONE;

    OSL_ASSERT(path);
    OSL_ASSERT(sVendor);
    OSL_ASSERT(sMinVersion);
    OSL_ASSERT(sMaxVersion);
    if (!path || !sVendor || !sMinVersion || !sMaxVersion || !ppInfo)
        return JFW_PLUGIN_E_INVALID_ARG;
    OUString ouPath(path);
    OSL_ASSERT(!ouPath.isEmpty());
    if (ouPath.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of element in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    OUString ouVendor(sVendor);
    OUString ouMinVer(sMinVersion);
    OUString ouMaxVer(sMaxVersion);

    OSL_ASSERT(!ouVendor.isEmpty());
    if (ouVendor.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath(ouPath);
    if (aVendorInfo.is() == sal_False)
        return JFW_PLUGIN_E_NO_JRE;

    //Check if the detected JRE matches the version requirements
    if (ouVendor.equals(aVendorInfo->getVendor()) == sal_False)
        return JFW_PLUGIN_E_NO_JRE;

    if (!ouMinVer.isEmpty())
    {
        int nRes = 0;
        try
        {
            nRes = aVendorInfo->compareVersions(ouMinVer);
        }
        catch (MalformedVersionException&)
        {
            //The minVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                0,"[Java framework]sunjavaplugin does not know version: "
                + ouMinVer + " for vendor: " + aVendorInfo->getVendor()
                + " .Check minimum Version." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
        if (nRes < 0)
            return JFW_PLUGIN_E_FAILED_VERSION;
    }

    if (!ouMaxVer.isEmpty())
    {
        int nRes = 0;
        try
        {
            nRes = aVendorInfo->compareVersions(ouMaxVer);
        }
        catch (MalformedVersionException&)
        {
            //The maxVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                0,"[Java framework]sunjavaplugin does not know version: "
                + ouMaxVer + " for vendor: " + aVendorInfo->getVendor()
                + " .Check maximum Version." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
        if (nRes > 0)
            return JFW_PLUGIN_E_FAILED_VERSION;
    }

    for (int i = 0; i < nLenList; i++)
    {
        OUString sExVer(arExcludeList[i]);
        int nRes = 0;
        try
        {
            nRes = aVendorInfo->compareVersions(sExVer);
        }
        catch (MalformedVersionException&)
        {
            //The excluded version was not recognized as valid for this vendor.
            JFW_ENSURE(
                0,"[Java framework]sunjavaplugin does not know version: "
                + sExVer + " for vendor: " + aVendorInfo->getVendor()
                + " .Check excluded versions." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
        if (nRes == 0)
            return JFW_PLUGIN_E_FAILED_VERSION;
    }
    *ppInfo = createJavaInfo(aVendorInfo);

    return errorcode;
}

#if defined(WNT)

// Load msvcr71.dll using an explicit full path from where it is
// present as bundled with the JRE. In case it is not found where we
// think it should be, do nothing, and just let the implicit loading
// that happens when loading the JVM take care of it.

static void load_msvcr71(LPCWSTR jvm_dll)
{
    wchar_t msvcr71_dll[MAX_PATH];
    wchar_t *slash;

    if (wcslen(jvm_dll) > MAX_PATH - 15)
        return;

    wcscpy(msvcr71_dll, jvm_dll);

    // First check if msvcr71.dll is in the same folder as jvm.dll. It
    // normally isn't, at least up to 1.6.0_22, but who knows if it
    // might be in the future.
    slash = wcsrchr(msvcr71_dll, L'\\');

    if (!slash)
    {
        // Huh, weird path to jvm.dll. Oh well.
        return;
    }

    wcscpy(slash+1, L"msvcr71.dll");
    if (LoadLibraryW(msvcr71_dll))
        return;

    // Then check if msvcr71.dll is in the parent folder of where
    // jvm.dll is. That is currently (1.6.0_22) as far as I know the
    // normal case.
    *slash = 0;
    slash = wcsrchr(msvcr71_dll, L'\\');

    if (!slash)
        return;

    wcscpy(slash+1, L"msvcr71.dll");
    LoadLibraryW(msvcr71_dll);
}

// Check if the jvm DLL imports msvcr71.dll, and in that case try
// loading it explicitly. In case something goes wrong, do nothing,
// and just let the implicit loading try to take care of it.
static void do_msvcr71_magic(rtl_uString *jvm_dll)
{
    FILE *f;
    rtl_uString* Module;
    oslFileError nError;
    struct stat st;
    PIMAGE_DOS_HEADER dos_hdr;
    IMAGE_NT_HEADERS *nt_hdr;
    IMAGE_IMPORT_DESCRIPTOR *imports;

    nError = osl_getSystemPathFromFileURL(jvm_dll, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, jvm_dll);

    f = _wfopen(reinterpret_cast<LPCWSTR>(Module->buffer), L"rb");

    if (fstat(fileno(f), &st) == -1)
    {
        fclose(f);
        return;
    }

    dos_hdr = (PIMAGE_DOS_HEADER) malloc(st.st_size);

    if (fread(dos_hdr, st.st_size, 1, f) != 1 ||
        memcmp(dos_hdr, "MZ", 2) != 0 ||
        dos_hdr->e_lfanew < 0 ||
        dos_hdr->e_lfanew > (LONG) (st.st_size - sizeof(IMAGE_NT_HEADERS)))
    {
        free(dos_hdr);
        fclose(f);
        return;
    }

    fclose(f);

    nt_hdr = (IMAGE_NT_HEADERS *) ((char *)dos_hdr + dos_hdr->e_lfanew);

    imports = (IMAGE_IMPORT_DESCRIPTOR *) ((char *) dos_hdr + nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (imports <= (IMAGE_IMPORT_DESCRIPTOR *) ((char *) dos_hdr + st.st_size - sizeof (IMAGE_IMPORT_DESCRIPTOR)) &&
           imports->Name != 0 &&
           imports->Name < (DWORD) st.st_size)
    {
        // Intentional use of sizeof("msvcr71.dll") here to include the terminating zero byte
        if (strnicmp((char *) dos_hdr + imports->Name, "msvcr71.dll", sizeof("msvcr71.dll")) == 0)
        {
            load_msvcr71(reinterpret_cast<LPCWSTR>(Module->buffer));
            break;
        }
        imports++;
    }

    free(dos_hdr);
}

#endif

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
    // unless guard is volatile the following warning occurs on gcc:
    // warning: variable 't' might be clobbered by `longjmp' or `vfork'
    volatile osl::MutexGuard guard(PluginMutex::get());
    // unless errorcode is volatile the following warning occurs on gcc:
    // warning: variable 'errorcode' might be clobbered by `longjmp' or `vfork'
    volatile javaPluginError errorcode = JFW_PLUGIN_E_NONE;
    if ( pInfo == NULL || ppVm == NULL || ppEnv == NULL)
        return JFW_PLUGIN_E_INVALID_ARG;
    //Check if the Vendor (pInfo->sVendor) is supported by this plugin
    if ( ! isVendorSupported(pInfo->sVendor))
        return JFW_PLUGIN_E_WRONG_VENDOR;
    OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
    JFW_TRACE2("[Java framework] Using Java runtime library: "
              + sRuntimeLib + ".\n");

#ifndef ANDROID
    // On linux we load jvm with RTLD_GLOBAL. This is necessary for debugging, because
    // libjdwp.so need a symbol (fork1) from libjvm which it only gets if the jvm is loaded
    // witd RTLD_GLOBAL. On Solaris libjdwp.so is correctly linked with libjvm.so
    oslModule moduleRt = 0;
#if defined(LINUX)
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData,
                                   SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_NOW)) == 0 )
#else
#if defined(WNT)
    do_msvcr71_magic(sRuntimeLib.pData);
#endif
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData, SAL_LOADMODULE_DEFAULT)) == 0)
#endif
     {
         JFW_ENSURE(0, "[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                       " could not load Java runtime library: \n"
                    + sRuntimeLib + "\n");
         JFW_TRACE0("[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                    " could not load Java runtime library: \n"
                    + sRuntimeLib +  "\n");
         return JFW_PLUGIN_E_VM_CREATION_FAILED;
     }

#ifdef UNX
    //Setting the JAVA_HOME is needed for awt
    OUString javaHome("JAVA_HOME=");
    OUString sPathLocation;
    osl_getSystemPathFromFileURL(pInfo->sLocation, & sPathLocation.pData);
    javaHome += sPathLocation;
    OString osJavaHome = OUStringToOString(
        javaHome, osl_getThreadTextEncoding());
    putenv(strdup(osJavaHome.getStr()));
#endif

    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);
    OUString sSymbolCreateJava("JNI_CreateJavaVM");

    JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *) osl_getFunctionSymbol(
        moduleRt, sSymbolCreateJava.pData);
    if (!pCreateJavaVM)
    {
        OSL_ASSERT(0);
        OString sLib = OUStringToOString(
            sRuntimeLib, osl_getThreadTextEncoding());
        OString sSymbol = OUStringToOString(
            sSymbolCreateJava, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                "Java runtime library: %s does not export symbol %s !\n",
                sLib.getStr(), sSymbol.getStr());
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
    }

    // Valgrind typically emits many false errors when executing JIT'ed JVM
    // code, so force the JVM into interpreted mode:
    bool forceInterpreted = RUNNING_ON_VALGRIND > 0;

    // Some testing with Java 1.4 showed that JavaVMOption.optionString has to
    // be encoded with the system encoding (i.e., osl_getThreadTextEncoding):
    JavaVMInitArgs vm_args;

    sal_Int32 nOptions = 1 + cOptions + (forceInterpreted ? 1 : 0);
        //TODO: check for overflow
    boost::scoped_array<JavaVMOption> sarOptions(new JavaVMOption[nOptions]);
    JavaVMOption * options = sarOptions.get();

    // We set an abort handler which is called when the VM calls _exit during
    // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
    // all some directories of the Java installation. This is necessary for
    // all versions below 1.5.1
    int n = 0;
    options[n].optionString= (char *) "abort";
    options[n].extraInfo= (void* )(sal_IntPtr)abort_handler;
    ++n;
    OString sClassPathProp("-Djava.class.path=");
    OString sClassPathOption;
    for (int i = 0; i < cOptions; i++)
    {
#ifdef UNX
    // Until java 1.5 we need to put a plugin.jar or javaplugin.jar (<1.4.2)
    // in the class path in order to have applet support.
        OString sClassPath = arOptions[i].optionString;
        if (sClassPath.match(sClassPathProp, 0) == sal_True)
        {
            char sep[] =  {SAL_PATHSEPARATOR, 0};
            OString sAddPath = getPluginJarPath(pInfo->sVendor, pInfo->sLocation,pInfo->sVersion);
            if (!sAddPath.isEmpty())
                sClassPathOption = sClassPath + OString(sep) + sAddPath;
            else
                sClassPathOption = sClassPath;
            options[n].optionString = (char *) sClassPathOption.getStr();
            options[n].extraInfo = arOptions[i].extraInfo;
        }
        else
        {
#endif
            options[n].optionString = arOptions[i].optionString;
            options[n].extraInfo = arOptions[i].extraInfo;
#ifdef UNX
        }
#endif
#if OSL_DEBUG_LEVEL >= 2
        JFW_TRACE2("VM option: " << options[n].optionString << "\n");
#endif
        ++n;
    }
    if (forceInterpreted) {
        options[n].optionString = const_cast<char *>("-Xint");
        options[n].extraInfo = 0;
        ++n;
    }

#ifdef MACOSX
    vm_args.version= JNI_VERSION_1_4; // issue 88987
#else
    vm_args.version= JNI_VERSION_1_2;
#endif
    vm_args.options= options;
    vm_args.nOptions= nOptions;
    vm_args.ignoreUnrecognized= JNI_TRUE;

    /* We set a global flag which is used by the abort handler in order to
       determine whether it is  should use longjmp to get back into this function.
       That is, the abort handler determines if it is on the same stack as this function
       and then jumps back into this function.
    */
    g_bInGetJavaVM = 1;
    jint err;
    JavaVM * pJavaVM = 0;
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
        if( err < 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin" SAL_DLLEXTENSION
                    "Can not create Java Virtual Machine\n");
            errorcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
        else if( err > 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin" SAL_DLLEXTENSION
                    "Can not create JavaVirtualMachine, abort handler was called.\n");
            errorcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
    }
    else
    {
        *ppVm = pJavaVM;
        JFW_TRACE2("[Java framework] sunjavaplugin" SAL_DLLEXTENSION " has created a VM.\n");
    }
#else
    (void) arOptions;
    (void) cOptions;
    // On Android we always have a Java VM as we only expect this code
    // to be run in an Android app anyway.
    *ppVm = lo_get_javavm();
    fprintf(stderr, "lo_get_javavm returns %p", *ppVm);
#endif

   return errorcode;
}

extern "C"
javaPluginError jfw_plugin_existJRE(const JavaInfo *pInfo, sal_Bool *exist)
{
    javaPluginError ret = JFW_PLUGIN_E_NONE;
    if (!pInfo || !exist)
        return JFW_PLUGIN_E_INVALID_ARG;
    OUString sLocation(pInfo->sLocation);

    if (sLocation.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;
    ::osl::DirectoryItem item;
    ::osl::File::RC rc_item = ::osl::DirectoryItem::get(sLocation, item);
    if (::osl::File::E_None == rc_item)
    {
        *exist = sal_True;
    }
    else if (::osl::File::E_NOENT == rc_item)
    {
        *exist = sal_False;
    }
    else
    {
        ret = JFW_PLUGIN_E_ERROR;
    }
#ifdef MACOSX
    //We can have the situation that the JavaVM runtime library is not
    //contained within JAVA_HOME. Then the check for JAVA_HOME would return
    //true although the runtime library may not be loadable.
    if (ret == JFW_PLUGIN_E_NONE && *exist == sal_True)
    {
        OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
        JFW_TRACE2("[Java framework] Checking existence of Java runtime library.\n");

        ::osl::DirectoryItem itemRt;
        ::osl::File::RC rc_itemRt = ::osl::DirectoryItem::get(sRuntimeLib, itemRt);
        if (::osl::File::E_None == rc_itemRt)
        {
            *exist = sal_True;
            JFW_TRACE2("[Java framework] Java runtime library exist: "
              + sRuntimeLib + "\n");

        }
        else if (::osl::File::E_NOENT == rc_itemRt)
        {
            *exist = sal_False;
            JFW_TRACE2("[Java framework] Java runtime library does not exist: "
                       + sRuntimeLib + "\n");
        }
        else
        {
            ret = JFW_PLUGIN_E_ERROR;
            JFW_TRACE2("[Java framework] Error while looking for Java runtime library: "
                       + sRuntimeLib + " \n");
        }
    }
#endif
    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
