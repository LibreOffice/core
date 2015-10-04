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

#include <string.h>

#include <memory>
#include "config_options.h"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "rtl/instance.hxx"
#include "osl/getglobalmutex.hxx"
#include <setjmp.h>
#include <signal.h>
#include <stack>

#include "jni.h"
#include "rtl/byteseq.hxx"
#include "vendorplugin.hxx"
#include "util.hxx"
#include "sunversion.hxx"
#include "vendorlist.hxx"
#include "diagnostics.h"

#ifdef MACOSX
#include "util_cocoa.hxx"
#endif

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#else
#if !ENABLE_RUNTIME_OPTIMIZATIONS
#define FORCE_INTERPRETED 1
#elif defined HAVE_VALGRIND_HEADERS
#include <valgrind/valgrind.h>
#define FORCE_INTERPRETED RUNNING_ON_VALGRIND
#else
#define FORCE_INTERPRETED 0
#endif
#endif

#if defined LINUX && (defined X86 || defined X86_64)
#include <sys/resource.h>
#endif

using namespace osl;
using namespace std;
using namespace jfw_plugin;


namespace {

struct PluginMutex: public ::rtl::Static<osl::Mutex, PluginMutex> {};

#if defined(UNX) && !defined(ANDROID)
OString getPluginJarPath(
    const OUString & sVendor,
    const OUString& sLocation,
    const OUString& sVersion)
{
    OString ret;
    OUString sName1("javaplugin.jar");
    OUString sName2("plugin.jar");
    OUString sPath;
    if ( sVendor == "Sun Microsystems Inc." )
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
                char sep[] = {SAL_PATHSEPARATOR, 0};
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
    JavaInfo* pInfo = static_cast<JavaInfo*>(rtl_allocateMemory(sizeof(JavaInfo)));
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
    if (!info->getLibraryPath().isEmpty())
    {
        buf.append("\n");
        buf.append(info->getLibraryPath());
        buf.append("\n");
    }

    OUString sVendorData = buf.makeStringAndClear();
    rtl::ByteSequence byteSeq( reinterpret_cast<sal_Int8*>(sVendorData.pData->buffer),
                               sVendorData.getLength() * sizeof(sal_Unicode));
    pInfo->arVendorData = byteSeq.get();
    rtl_byte_sequence_acquire(pInfo->arVendorData);

    return pInfo;
}

OUString getRuntimeLib(const rtl::ByteSequence & data)
{
    const sal_Unicode* chars = reinterpret_cast<sal_Unicode const *>(data.getConstArray());
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
        fprintf(stderr, "JavaVM: JNI_CreateJavaVM called os::abort(), caught by abort_handler in javavm.cxx\n");
        longjmp( jmp_jvm_abort, 0);
    }
}

/** helper function to check Java version requirements

    This function checks if the Java version of the given VendorBase
    meets the given Java version requirements.

    @param aVendorInfo
        [in]  the object to be inspected whether it meets the version requirements
    @param sMinVersion
        [in] represents the minimum version of a JRE. The string can be empty.
    @param sMaxVersion
        [in] represents the maximum version of a JRE. The string can be empty.
    @param arExcludeList
        [in] contains a list of &quot;bad&quot; versions. JREs which have one of these
        versions must not be returned by this function. It can be NULL.
    @param nLenList
        [in] the number of version strings contained in <code>arExcludeList</code>.

   @return
    JFW_PLUGIN_E_NONE the function ran successfully and the version requirements are met
    JFW_PLUGIN_E_FAILED_VERSION at least one of the version requirements (minVersion,
    maxVersion, excludeVersions) was violated
    JFW_PLUGIN_E_WRONG_VERSION_FORMAT the version strings in
    <code>sMinVersion,sMaxVersion,arExcludeList</code> are not recognized as valid
    version strings.

     */
javaPluginError checkJavaVersionRequirements(
    rtl::Reference<VendorBase> const & aVendorInfo,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    rtl_uString * * arExcludeList,
    sal_Int32  nLenList)
{
    if (!sMinVersion.isEmpty())
    {
        try
        {
            if (aVendorInfo->compareVersions(sMinVersion) < 0)
                return JFW_PLUGIN_E_FAILED_VERSION;
        }
        catch (MalformedVersionException&)
        {
            //The minVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sMinVersion + " for vendor: " + aVendorInfo->getVendor()
                + " .Check minimum Version." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
    }

    if (!sMaxVersion.isEmpty())
    {
        try
        {
            if (aVendorInfo->compareVersions(sMaxVersion) > 0)
                return JFW_PLUGIN_E_FAILED_VERSION;
        }
        catch (MalformedVersionException&)
        {
            //The maxVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sMaxVersion + " for vendor: " + aVendorInfo->getVendor()
                + " .Check maximum Version." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
    }

    for (int i = 0; i < nLenList; i++)
    {
        OUString sExVer(arExcludeList[i]);
        try
        {
            if (aVendorInfo->compareVersions(sExVer) == 0)
                return JFW_PLUGIN_E_FAILED_VERSION;
        }
        catch (MalformedVersionException&)
        {
            //The excluded version was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sExVer + " for vendor: " + aVendorInfo->getVendor()
                + " .Check excluded versions." );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
    }

    return JFW_PLUGIN_E_NONE;
}

}

javaPluginError jfw_plugin_getAllJavaInfos(
    bool checkJavaHomeAndPath,
    OUString const& sVendor,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    rtl_uString  * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos)
{
    OSL_ASSERT(parJavaInfo);
    OSL_ASSERT(nLenInfoList);
    if (!parJavaInfo || !nLenInfoList)
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of elements in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    OSL_ASSERT(!sVendor.isEmpty());
    if (sVendor.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    JavaInfo** arInfo = NULL;

    //Find all JREs
    vector<rtl::Reference<VendorBase> > vecInfos =
        addAllJREInfos(checkJavaHomeAndPath, infos);
    vector<rtl::Reference<VendorBase> > vecVerifiedInfos;

    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfos.begin(); i != vecInfos.end(); ++i)
    {
        const rtl::Reference<VendorBase>& cur = *i;

        if (!sVendor.equals(cur->getVendor()))
            continue;

        javaPluginError err = checkJavaVersionRequirements(
            cur, sMinVersion, sMaxVersion, arExcludeList, nLenList);

        if (err == JFW_PLUGIN_E_FAILED_VERSION)
            continue;
        else if (err == JFW_PLUGIN_E_WRONG_VERSION_FORMAT)
            return err;

        vecVerifiedInfos.push_back(*i);
    }
    //Now vecVerifiedInfos contains all those JREs which meet the version requirements
    //Transfer them into the array that is passed out.
    arInfo = static_cast<JavaInfo**>(rtl_allocateMemory(vecVerifiedInfos.size() * sizeof (JavaInfo*)));
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

javaPluginError jfw_plugin_getJavaInfoByPath(
    OUString const& sPath,
    OUString const& sVendor,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    rtl_uString  *  *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo)
{
    if (!ppInfo)
        return JFW_PLUGIN_E_INVALID_ARG;
    OSL_ASSERT(!sPath.isEmpty());
    if (sPath.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of elements in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    OSL_ASSERT(!sVendor.isEmpty());
    if (sVendor.isEmpty())
        return JFW_PLUGIN_E_INVALID_ARG;

    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath(sPath);
    if (!aVendorInfo.is())
        return JFW_PLUGIN_E_NO_JRE;

    //Check if the detected JRE matches the version requirements
    if (!sVendor.equals(aVendorInfo->getVendor()))
        return JFW_PLUGIN_E_NO_JRE;
    javaPluginError errorcode = checkJavaVersionRequirements(
            aVendorInfo, sMinVersion, sMaxVersion, arExcludeList, nLenList);

    if (errorcode == JFW_PLUGIN_E_NONE)
        *ppInfo = createJavaInfo(aVendorInfo);

    return errorcode;
}

javaPluginError jfw_plugin_getJavaInfoFromJavaHome(
    std::vector<pair<OUString, jfw::VersionInfo>> const& vecVendorInfos,
    JavaInfo ** ppInfo, std::vector<rtl::Reference<VendorBase>> & infos)
{
    if (!ppInfo)
        return JFW_PLUGIN_E_INVALID_ARG;

    std::vector<rtl::Reference<VendorBase>> infoJavaHome;
    addJavaInfoFromJavaHome(infos, infoJavaHome);

    if (infoJavaHome.empty())
        return JFW_PLUGIN_E_NO_JRE;
    assert(infoJavaHome.size() == 1);

    //Check if the detected JRE matches the version requirements
    typedef std::vector<pair<OUString, jfw::VersionInfo>>::const_iterator ci_pl;
    for (ci_pl vendorInfo = vecVendorInfos.begin(); vendorInfo != vecVendorInfos.end(); ++vendorInfo)
    {
        const OUString& vendor = vendorInfo->first;
        jfw::VersionInfo versionInfo = vendorInfo->second;

        if (vendor.equals(infoJavaHome[0]->getVendor()))
        {
            javaPluginError errorcode = checkJavaVersionRequirements(
                infoJavaHome[0],
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize());

            if (errorcode == JFW_PLUGIN_E_NONE)
            {
                *ppInfo = createJavaInfo(infoJavaHome[0]);
                return JFW_PLUGIN_E_NONE;
            }
        }
    }

    return JFW_PLUGIN_E_NO_JRE;
}

javaPluginError jfw_plugin_getJavaInfosFromPath(
    std::vector<std::pair<OUString, jfw::VersionInfo>> const& vecVendorInfos,
    std::vector<JavaInfo*> & javaInfosFromPath,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos)
{
    // find JREs from PATH
    vector<rtl::Reference<VendorBase>> vecInfosFromPath;
    addJavaInfosFromPath(infos, vecInfosFromPath);

    vector<JavaInfo*> vecVerifiedInfos;

    // copy infos of JREs that meet version requirements to vecVerifiedInfos
    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfosFromPath.begin(); i != vecInfosFromPath.end(); ++i)
    {
        const rtl::Reference<VendorBase>& currentInfo = *i;

        typedef std::vector<pair<OUString, jfw::VersionInfo>>::const_iterator ci_pl;
        for (ci_pl vendorInfo = vecVendorInfos.begin(); vendorInfo != vecVendorInfos.end(); ++vendorInfo)
        {
            const OUString& vendor = vendorInfo->first;
            jfw::VersionInfo versionInfo = vendorInfo->second;

            if (vendor.equals(currentInfo->getVendor()))
            {
                javaPluginError errorcode = checkJavaVersionRequirements(
                    currentInfo,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.getExcludeVersions(),
                    versionInfo.getExcludeVersionSize());

                if (errorcode == JFW_PLUGIN_E_NONE)
                {
                    vecVerifiedInfos.push_back(createJavaInfo(currentInfo));
                }
            }
        }
    }

    if (vecVerifiedInfos.empty())
        return JFW_PLUGIN_E_NO_JRE;

    javaInfosFromPath = vecVerifiedInfos;

    return JFW_PLUGIN_E_NONE;
}




#if defined(WNT)

// Load msvcr71.dll using an explicit full path from where it is
// present as bundled with the JRE. In case it is not found where we
// think it should be, do nothing, and just let the implicit loading
// that happens when loading the JVM take care of it.

static void load_msvcr(LPCWSTR jvm_dll, wchar_t const* msvcr)
{
    wchar_t msvcr_dll[MAX_PATH];
    wchar_t *slash;

    if (wcslen(jvm_dll) > MAX_PATH - 15)
        return;

    wcscpy(msvcr_dll, jvm_dll);

    // First check if msvcr71.dll is in the same folder as jvm.dll. It
    // normally isn't, at least up to 1.6.0_22, but who knows if it
    // might be in the future.
    slash = wcsrchr(msvcr_dll, L'\\');

    if (!slash)
    {
        // Huh, weird path to jvm.dll. Oh well.
        return;
    }

    wcscpy(slash+1, msvcr);
    if (LoadLibraryW(msvcr_dll))
        return;

    // Then check if msvcr71.dll is in the parent folder of where
    // jvm.dll is. That is currently (1.6.0_22) as far as I know the
    // normal case.
    *slash = 0;
    slash = wcsrchr(msvcr_dll, L'\\');

    if (!slash)
        return;

    wcscpy(slash+1, msvcr);
    LoadLibraryW(msvcr_dll);
}

// Check if the jvm DLL imports msvcr71.dll, and in that case try
// loading it explicitly. In case something goes wrong, do nothing,
// and just let the implicit loading try to take care of it.
static void do_msvcr_magic(rtl_uString *jvm_dll)
{
    rtl_uString* Module(0);
    struct stat st;

    oslFileError nError = osl_getSystemPathFromFileURL(jvm_dll, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, jvm_dll);

    FILE *f = _wfopen(reinterpret_cast<LPCWSTR>(Module->buffer), L"rb");

    if (fstat(fileno(f), &st) == -1)
    {
        fclose(f);
        return;
    }

    PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER) malloc(st.st_size);

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

    IMAGE_NT_HEADERS *nt_hdr = (IMAGE_NT_HEADERS *) ((char *)dos_hdr + dos_hdr->e_lfanew);

    DWORD importsVA = nt_hdr->OptionalHeader
            .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    // first determine Virtual-to-File-address mapping for the section
    // that contains the import directory
    IMAGE_SECTION_HEADER *sections = IMAGE_FIRST_SECTION(nt_hdr);
    ptrdiff_t VAtoPhys = -1;
    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; ++i)
    {
        if (sections->VirtualAddress <= importsVA &&
            importsVA < sections->VirtualAddress + sections->SizeOfRawData)
        {
            VAtoPhys = static_cast<size_t>(sections->PointerToRawData) - static_cast<size_t>(sections->VirtualAddress);
            break;
        }
        ++sections;
    }
    if (-1 == VAtoPhys) // not found?
    {
        free(dos_hdr);
        return;
    }
    IMAGE_IMPORT_DESCRIPTOR *imports =
        (IMAGE_IMPORT_DESCRIPTOR *) ((char *) dos_hdr + importsVA + VAtoPhys);

    while (imports <= (IMAGE_IMPORT_DESCRIPTOR *) ((char *) dos_hdr + st.st_size - sizeof (IMAGE_IMPORT_DESCRIPTOR)) &&
           imports->Name != 0 &&
           imports->Name + VAtoPhys < (DWORD) st.st_size)
    {
        static struct { char const * name; wchar_t const * wname; } msvcrts[] =
        {
            { "msvcr71.dll" , L"msvcr71.dll"  },
            { "msvcr100.dll", L"msvcr100.dll" },
        };
        char const* importName = (char *) dos_hdr + imports->Name + VAtoPhys;
        for (size_t i = 0; i < SAL_N_ELEMENTS(msvcrts); ++i)
        {
            if (0 == strnicmp(importName,
                // Intentional strlen() + 1 here to include terminating zero
                        msvcrts[i].name, strlen(msvcrts[i].name) + 1))
            {
                load_msvcr(reinterpret_cast<LPCWSTR>(Module->buffer),
                        msvcrts[i].wname);
                free(dos_hdr);
                return;
            }
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
#ifdef MACOSX
    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath( OUString( pInfo->sLocation ) );
    if ( !aVendorInfo.is() || aVendorInfo->compareVersions( OUString( pInfo->sVersion ) ) < 0 )
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
#endif
    OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
#ifdef MACOSX
    if ( !JvmfwkUtil_isLoadableJVM( sRuntimeLib ) )
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
#endif
    JFW_TRACE2("Using Java runtime library: " << sRuntimeLib);

#ifndef ANDROID
    // On linux we load jvm with RTLD_GLOBAL. This is necessary for debugging, because
    // libjdwp.so need a symbol (fork1) from libjvm which it only gets if the jvm is loaded
    // with RTLD_GLOBAL. On Solaris libjdwp.so is correctly linked with libjvm.so
    osl::Module moduleRt;
#if defined(LINUX)
    if (!moduleRt.load(sRuntimeLib, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_NOW))
#else
#if defined(WNT)
    do_msvcr_magic(sRuntimeLib.pData);
#endif
    if (!moduleRt.load(sRuntimeLib))
#endif
    {
        JFW_ENSURE(false,
                   "[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                      " could not load Java runtime library: \n"
                   + sRuntimeLib + "\n");
        JFW_TRACE0("Could not load Java runtime library: " << sRuntimeLib);
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
    }

#if defined UNX && !defined MACOSX
    //Setting the JAVA_HOME is needed for awt
    OUString sPathLocation;
    osl_getSystemPathFromFileURL(pInfo->sLocation, & sPathLocation.pData);
    osl_setEnvironment(OUString("JAVA_HOME").pData, sPathLocation.pData);
#endif

    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);
    OUString sSymbolCreateJava("JNI_CreateJavaVM");

    JNI_CreateVM_Type * pCreateJavaVM =
        reinterpret_cast<JNI_CreateVM_Type *>(moduleRt.getFunctionSymbol(sSymbolCreateJava));
    if (!pCreateJavaVM)
    {
        OSL_ASSERT(false);
        OString sLib = OUStringToOString(
            sRuntimeLib, osl_getThreadTextEncoding());
        OString sSymbol = OUStringToOString(
            sSymbolCreateJava, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                "Java runtime library: %s does not export symbol %s !\n",
                sLib.getStr(), sSymbol.getStr());
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
    }
    moduleRt.release();

    // Valgrind typically emits many false errors when executing JIT'ed JVM
    // code, so force the JVM into interpreted mode:
    bool addForceInterpreted = FORCE_INTERPRETED > 0;

    // Some testing with Java 1.4 showed that JavaVMOption.optionString has to
    // be encoded with the system encoding (i.e., osl_getThreadTextEncoding):
    JavaVMInitArgs vm_args;

    struct Option {
        Option(OString const & theOptionString, void * theExtraInfo):
            optionString(theOptionString), extraInfo(theExtraInfo)
        {}

        OString optionString;
        void * extraInfo;
    };
    std::vector<Option> options;

    // We set an abort handler which is called when the VM calls _exit during
    // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
    // all some directories of the Java installation. This is necessary for
    // all versions below 1.5.1
    options.push_back(Option("abort", reinterpret_cast<void*>(abort_handler)));
    bool hasStackSize = false;
    for (int i = 0; i < cOptions; i++)
    {
        OString opt(arOptions[i].optionString);
#ifdef UNX
        // Until java 1.5 we need to put a plugin.jar or javaplugin.jar (<1.4.2)
        // in the class path in order to have applet support:
        if (opt.startsWith("-Djava.class.path="))
        {
            OString sAddPath = getPluginJarPath(pInfo->sVendor, pInfo->sLocation,pInfo->sVersion);
            if (!sAddPath.isEmpty())
                opt += OString(SAL_PATHSEPARATOR) + sAddPath;
        }
#endif
        if (opt == "-Xint") {
            addForceInterpreted = false;
        }
        if (opt.startsWith("-Xss")) {
            hasStackSize = true;
        }
        options.push_back(Option(opt, arOptions[i].extraInfo));
    }
    if (addForceInterpreted) {
        options.push_back(Option("-Xint", nullptr));
    }
    if (!hasStackSize) {
#if defined LINUX && (defined X86 || defined X86_64)
        // At least OpenJDK 1.8.0's os::workaround_expand_exec_shield_cs_limit
        // (hotspot/src/os_cpu/linux_x86/vm/os_linux_x86.cpp) can mmap an rwx
        // page into the area that the main stack can grow down to according to
        // "ulimit -s", as os::init_2's (hotspot/src/os/linux/vm/os_linux.cpp)
        // call to
        //
        //   Linux::capture_initial_stack(JavaThread::stack_size_at_create());
        //
        // caps _initial_thread_stack_size at threadStackSizeInBytes ,i.e.,
        // -Xss, which appears to default to only 327680, whereas "ulimit -s"
        // defaults to 8192 * 1024 at least on Fedora 20; so attempt to pass in
        // a useful -Xss argument:
        rlimit l;
        if (getrlimit(RLIMIT_STACK, &l) == 0) {
            if (l.rlim_cur == RLIM_INFINITY) {
                SAL_INFO("jfw", "RLIMIT_STACK RLIM_INFINITY -> 8192K");
                l.rlim_cur = 8192 * 1024;
            } else if (l.rlim_cur > 512 * 1024 * 1024) {
                SAL_INFO(
                    "jfw", "huge RLIMIT_STACK " << l.rlim_cur << " -> 8192K");
                l.rlim_cur = 8192 * 1024;
            }
            options.push_back(
                Option("-Xss" + OString::number(l.rlim_cur), nullptr));
        } else {
            int e = errno;
            SAL_WARN("jfw", "getrlimit(RLIMIT_STACK) failed with errno " << e);
        }
#endif
    }

    std::unique_ptr<JavaVMOption[]> sarOptions(new JavaVMOption[options.size()]);
    for (std::vector<Option>::size_type i = 0; i != options.size(); ++i) {
        SAL_INFO(
            "jfw",
            "VM option \"" << options[i].optionString << "\" "
                << options[i].extraInfo);
        sarOptions[i].optionString = const_cast<char *>(
            options[i].optionString.getStr());
        sarOptions[i].extraInfo = options[i].extraInfo;
    }

#ifdef MACOSX
    vm_args.version= JNI_VERSION_1_4; // issue 88987
#else
    vm_args.version= JNI_VERSION_1_2;
#endif
    vm_args.options= sarOptions.get();
    vm_args.nOptions= options.size(); //TODO overflow
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
        JFW_TRACE2("JVM created");
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
    //We can have the situation that the JavaVM runtime library is not
    //contained within JAVA_HOME. Then the check for JAVA_HOME would return
    //true although the runtime library may not be loadable.
    //Or the JAVA_HOME directory of a deinstalled JRE left behind.
    if (ret == JFW_PLUGIN_E_NONE && *exist)
    {
        OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
        JFW_TRACE2("Checking existence of Java runtime library");

        ::osl::DirectoryItem itemRt;
        ::osl::File::RC rc_itemRt = ::osl::DirectoryItem::get(sRuntimeLib, itemRt);
        if (::osl::File::E_None == rc_itemRt)
        {
            *exist = sal_True;
            JFW_TRACE2("Java runtime library exist: " << sRuntimeLib);

        }
        else if (::osl::File::E_NOENT == rc_itemRt)
        {
            *exist = sal_False;
            JFW_TRACE2("Java runtime library does not exist: " << sRuntimeLib);
        }
        else
        {
            ret = JFW_PLUGIN_E_ERROR;
            JFW_TRACE2("Error while looking for Java runtime library: " << sRuntimeLib);
        }
    }
    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
