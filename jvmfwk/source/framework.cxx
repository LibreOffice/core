/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "boost/scoped_array.hpp"
#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"
#include <cassert>
#include <vector>
#include <functional>
#include <algorithm>
#include "framework.hxx"
#include "fwkutil.hxx"
#include "elements.hxx"
#include "fwkbase.hxx"

namespace {

static bool g_bEnabledSwitchedOn = false;

static JavaVM * g_pJavaVM = NULL;

bool areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    return jfw_areEqualJavaInfo(pInfoA, pInfoB);
}

}

#ifdef DISABLE_DYNLOADING

extern "C"
javaPluginError jfw_plugin_getAllJavaInfos(
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList);

extern "C"
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  *  *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo);

extern "C"
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo,
    const JavaVMOption* arOptions,
    sal_Int32 cOptions,
    JavaVM ** ppVm,
    JNIEnv ** ppEnv);

extern "C"
javaPluginError jfw_plugin_existJRE(const JavaInfo *pInfo, sal_Bool *exist);

#endif

javaFrameworkError SAL_CALL jfw_findAllJREs(JavaInfo ***pparInfo, sal_Int32 *pSize)
{
    javaFrameworkError retVal = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        javaFrameworkError errcode = JFW_E_NONE;
        if (pparInfo == NULL || pSize == NULL)
            return JFW_E_INVALID_ARG;

        jfw::VendorSettings aVendorSettings;
        
        std::vector<jfw::PluginLibrary> vecPlugins =
            aVendorSettings.getPluginData();
#ifndef DISABLE_DYNLOADING
        
        
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();
#endif
        
        
        
        std::vector<jfw::CJavaInfo> vecInfo;
        
        
        
        std::vector<jfw::CJavaInfo> vecInfoManual;
        typedef std::vector<jfw::CJavaInfo>::iterator it_info;
        
        
        const jfw::MergedSettings settings;
        const std::vector<OUString>& vecJRELocations =
            settings.getJRELocations();
        
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
         for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i, ++cModule)
         {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);
#ifndef DISABLE_DYNLOADING
            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];

            if (!pluginLib.is())
            {
                OString msg = OUStringToOString(
                    library.sPath, osl_getThreadTextEncoding());
                fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                        "Modify the javavendors.xml accordingly!\n", msg.getStr());
                return JFW_E_NO_PLUGIN;
            }
            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getFunctionSymbol(
                    OUString("jfw_plugin_getAllJavaInfos"));
#else
            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                jfw_plugin_getAllJavaInfos;
#endif
            OSL_ASSERT(getAllJavaFunc);
            if (getAllJavaFunc == NULL)
                return JFW_E_ERROR;

            
            
            sal_Int32 cInfos = 0;
            JavaInfo** arInfos = NULL;
            javaPluginError plerr  = (*getAllJavaFunc)(
                library.sVendor.pData,
                versionInfo.sMinVersion.pData,
                versionInfo.sMaxVersion.pData,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & arInfos,
                & cInfos);

            if (plerr != JFW_PLUGIN_E_NONE)
                return JFW_E_ERROR;

            for (int j = 0; j < cInfos; j++)
                vecInfo.push_back(jfw::CJavaInfo::createWrapper(arInfos[j]));

            rtl_freeMemory(arInfos);

            
            
            
#ifndef DISABLE_DYNLOADING
            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                    OUString("jfw_plugin_getJavaInfoByPath"));
            OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
            if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                return JFW_E_ERROR;
#else
            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                jfw_plugin_getJavaInfoByPath;
#endif

            typedef std::vector<OUString>::const_iterator citLoc;
            
            for (citLoc ii = vecJRELocations.begin();
                ii != vecJRELocations.end(); ++ii)
            {
                jfw::CJavaInfo aInfo;
                plerr = (*jfw_plugin_getJavaInfoByPathFunc)(
                    ii->pData,
                    library.sVendor.pData,
                    versionInfo.sMinVersion.pData,
                    versionInfo.sMaxVersion.pData,
                    versionInfo.getExcludeVersions(),
                    versionInfo.getExcludeVersionSize(),
                    & aInfo.pInfo);
                if (plerr == JFW_PLUGIN_E_NO_JRE)
                    continue;
                if (plerr == JFW_PLUGIN_E_FAILED_VERSION)
                    continue;
                else if (plerr !=JFW_PLUGIN_E_NONE)
                    return JFW_E_ERROR;

                if (aInfo)
                {
                    
                    
                    it_info it_duplicate =
                        std::find_if(vecInfoManual.begin(), vecInfoManual.end(),
                                std::bind2nd(std::ptr_fun(areEqualJavaInfo), aInfo));
                    if (it_duplicate == vecInfoManual.end())
                        vecInfoManual.push_back(aInfo);
                }
            }
        }
        
        
        
        
        std::vector<jfw::CJavaInfo> vecInfoManual2;
        for (it_info ivm = vecInfoManual.begin(); ivm != vecInfoManual.end(); ++ivm)
        {
            for (ci_pl ii = vecPlugins.begin(); ii != vecPlugins.end(); ++ii)
            {
                if ( ii->sVendor.equals((*ivm)->sVendor))
                {
                    vecInfoManual2.push_back(*ivm);
                    break;
                }
            }
        }
        
        
        
        for (it_info j = vecInfo.begin(); j != vecInfo.end(); ++j)
        {
            it_info it_duplicate =
                std::find_if(vecInfoManual2.begin(), vecInfoManual2.end(),
                            std::bind2nd(std::ptr_fun(areEqualJavaInfo), *j));
            if (it_duplicate != vecInfoManual2.end())
                vecInfoManual2.erase(it_duplicate);
        }
        
        sal_Int32 nSize = vecInfo.size() + vecInfoManual2.size();
        *pparInfo = (JavaInfo**) rtl_allocateMemory(
            nSize * sizeof(JavaInfo*));
        if (*pparInfo == NULL)
            return JFW_E_ERROR;

        typedef std::vector<jfw::CJavaInfo>::iterator it;
        int index = 0;
        
        for (it k = vecInfo.begin(); k != vecInfo.end(); ++k)
            (*pparInfo)[index++] = k->detach();
        
        for (it l = vecInfoManual2.begin(); l != vecInfoManual2.end(); ++l)
            (*pparInfo)[index++] = l->detach();

        *pSize = nSize;
        return errcode;
    }
    catch (const jfw::FrameworkException& e)
    {
        retVal = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return retVal;
}

javaFrameworkError SAL_CALL jfw_startVM(
    JavaInfo const * pInfo, JavaVMOption * arOptions, sal_Int32 cOptions,
    JavaVM ** ppVM, JNIEnv ** ppEnv)
{
    javaFrameworkError errcode = JFW_E_NONE;
    if (cOptions > 0 && arOptions == NULL)
        return JFW_E_INVALID_ARG;

    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        
        
        if (g_pJavaVM != NULL)
            return JFW_E_RUNNING_JVM;

        if (ppVM == NULL)
            return JFW_E_INVALID_ARG;

        std::vector<OString> vmParams;
        OString sUserClassPath;
        jfw::CJavaInfo aInfo;
        if (pInfo == NULL)
        {
            jfw::JFW_MODE mode = jfw::getMode();
            if (mode == jfw::JFW_MODE_APPLICATION)
            {
                const jfw::MergedSettings settings;
                if (!settings.getEnabled())
                    return JFW_E_JAVA_DISABLED;
                aInfo.attach(settings.createJavaInfo());
                
                if (aInfo == NULL)
                    return JFW_E_NO_SELECT;

#ifdef WNT
                
                
                
                
                
                
                if (jfw::isAccessibilitySupportDesired())
                {
                    
                    
                    if (aInfo != NULL &&
                        (aInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE) == 0)
                    {
                        
                        if (settings.getJavaInfoAttrAutoSelect() == true)
                        {
                            
                            
                            
                            
                            return JFW_E_NO_SELECT;
                        }
                    }
                }
#endif
                
                OString sVendorUpdate = jfw::getElementUpdated();

                if (sVendorUpdate != settings.getJavaInfoAttrVendorUpdate())
                    return JFW_E_INVALID_SETTINGS;

                
                
                
                
                if (g_bEnabledSwitchedOn &&
                    (aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
                    return JFW_E_NEED_RESTART;

                
                
                if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
                    &&
                    (jfw::wasJavaSelectedInSameProcess() == true))
                    return JFW_E_NEED_RESTART;

                vmParams = settings.getVmParametersUtf8();
                sUserClassPath = jfw::makeClassPathOption(settings.getUserClassPath());
            } 
            else if (mode == jfw::JFW_MODE_DIRECT)
            {
                errcode = jfw_getSelectedJRE(&aInfo.pInfo);
                if (errcode != JFW_E_NONE)
                    return errcode;
                
                
                vmParams = jfw::BootParams::getVMParameters();
                sUserClassPath =
                    "-Djava.class.path=" + jfw::BootParams::getClasspath();
            }
            else
                OSL_ASSERT(false);
            pInfo = aInfo.pInfo;
        }
        assert(pInfo != NULL);

        
        jfw::VendorSettings aVendorSettings;
        OUString sLibPath = aVendorSettings.getPluginLibrary(pInfo->sVendor);

#ifndef DISABLE_DYNLOADING
        osl::Module modulePlugin(sLibPath);
        if ( ! modulePlugin)
            return JFW_E_NO_PLUGIN;

        OUString sFunctionName("jfw_plugin_startJavaVirtualMachine");
        jfw_plugin_startJavaVirtualMachine_ptr pFunc =
            (jfw_plugin_startJavaVirtualMachine_ptr)
            osl_getFunctionSymbol(modulePlugin, sFunctionName.pData);
        if (pFunc == NULL)
            return JFW_E_ERROR;
#else
        jfw_plugin_startJavaVirtualMachine_ptr pFunc =
            jfw_plugin_startJavaVirtualMachine;
#endif

        
        
        
        boost::scoped_array<JavaVMOption> sarJOptions(
            new JavaVMOption[cOptions + 2 + vmParams.size()]);
        JavaVMOption * arOpt = sarJOptions.get();
        if (! arOpt)
            return JFW_E_ERROR;

        
        arOpt[0].optionString= (char*) sUserClassPath.getStr();
        arOpt[0].extraInfo = NULL;
        
        
        
        arOpt[1].optionString = (char *) "-Dorg.openoffice.native=";
        arOpt[1].extraInfo = 0;

        
        int index = 2;
        typedef std::vector<OString>::const_iterator cit;
        for (cit i = vmParams.begin(); i != vmParams.end(); ++i)
        {
            arOpt[index].optionString = const_cast<sal_Char*>(i->getStr());
            arOpt[index].extraInfo = 0;
            index ++;
        }
        
        for (int ii = 0; ii < cOptions; ii++)
        {
            arOpt[index].optionString = arOptions[ii].optionString;
            arOpt[index].extraInfo = arOptions[ii].extraInfo;
            index++;
        }

        
        JavaVM *pVm = NULL;
        SAL_INFO("jfw", "Starting Java");
        javaPluginError plerr = (*pFunc)(pInfo, arOpt, index, & pVm, ppEnv);
        if (plerr == JFW_PLUGIN_E_VM_CREATION_FAILED)
        {
            errcode = JFW_E_VM_CREATION_FAILED;
        }
        else if (plerr != JFW_PLUGIN_E_NONE )
        {
            errcode = JFW_E_ERROR;
        }
        else
        {
            g_pJavaVM = pVm;
            *ppVM = pVm;
        }
        OSL_ASSERT(plerr != JFW_PLUGIN_E_WRONG_VENDOR);
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;
}

/** We do not use here jfw_findAllJREs and then check if a JavaInfo
    meets the requirements, because that means using all plug-ins, which
    may take quite a while. The implementation uses one plug-in and if
    it already finds a suitable JRE then it is done and does not need to
    load another plug-in
 */
javaFrameworkError SAL_CALL jfw_findAndSelectJRE(JavaInfo **pInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        sal_uInt64 nFeatureFlags = 0;
        jfw::CJavaInfo aCurrentInfo;

        bool bSupportAccessibility = jfw::isAccessibilitySupportDesired();
        nFeatureFlags = bSupportAccessibility ?
            JFW_FEATURE_ACCESSBRIDGE : 0L;

        
        jfw::VendorSettings aVendorSettings;
        std::vector<jfw::PluginLibrary> vecPlugins =
             aVendorSettings.getPluginData();
#ifndef DISABLE_DYNLOADING
        
        
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();
#endif
        
        
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
        for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i, ++cModule)
        {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);
#ifndef DISABLE_DYNLOADING
            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];
            if (!pluginLib.is())
                return JFW_E_NO_PLUGIN;

            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getFunctionSymbol(
                    OUString("jfw_plugin_getAllJavaInfos"));
#else
            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                jfw_plugin_getAllJavaInfos;
#endif
            OSL_ASSERT(getAllJavaFunc);
            if (getAllJavaFunc == NULL)
                continue;

            
            
            sal_Int32 cInfos = 0;
            JavaInfo** arInfos = NULL;
            javaPluginError plerr  = (*getAllJavaFunc)(
                library.sVendor.pData,
                versionInfo.sMinVersion.pData,
                versionInfo.sMaxVersion.pData,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & arInfos,
                & cInfos);

            if (plerr != JFW_PLUGIN_E_NONE)
                continue;
            
            
            if (cInfos == 0)
            {
                rtl_freeMemory(arInfos);
                continue;
            }
            bool bInfoFound = false;
            for (int ii = 0; ii < cInfos; ii++)
            {
                JavaInfo* pJInfo = arInfos[ii];

                
                if (aCurrentInfo.getLocation().isEmpty())
                        aCurrentInfo = pJInfo;
                
                
                
                if ((pJInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                {
                    
                    
                    aCurrentInfo = pJInfo;
                    bInfoFound = true;
                    break;
                }
            }
            
            
            for (int j = 0; j < cInfos; j++)
                jfw_freeJavaInfo(arInfos[j]);
            rtl_freeMemory(arInfos);

            if (bInfoFound == true)
                break;
            
            
        }
        if ((JavaInfo*) aCurrentInfo == NULL)
        {
        
            
            const jfw::MergedSettings settings;
            
            const std::vector<OUString> & vecJRELocations =
                settings.getJRELocations();
            
            bool bInfoFound = false;
            for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i)
            {
                const jfw::PluginLibrary & library = *i;
                jfw::VersionInfo versionInfo =
                    aVendorSettings.getVersionInformation(library.sVendor);
#ifndef DISABLE_DYNLOADING
                osl::Module pluginLib(library.sPath);
                if (!pluginLib.is())
                    return JFW_E_NO_PLUGIN;
                
                
                
                jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                    (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                        OUString("jfw_plugin_getJavaInfoByPath"));
#else
                jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                    jfw_plugin_getJavaInfoByPath;
#endif
                OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
                if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                    return JFW_E_ERROR;

                typedef std::vector<OUString>::const_iterator citLoc;
                for (citLoc it = vecJRELocations.begin();
                    it != vecJRELocations.end(); ++it)
                {
                    jfw::CJavaInfo aInfo;
                    javaPluginError err = (*jfw_plugin_getJavaInfoByPathFunc)(
                        it->pData,
                        library.sVendor.pData,
                        versionInfo.sMinVersion.pData,
                        versionInfo.sMaxVersion.pData,
                        versionInfo.getExcludeVersions(),
                        versionInfo.getExcludeVersionSize(),
                        & aInfo.pInfo);
                    if (err == JFW_PLUGIN_E_NO_JRE)
                        continue;
                    if (err == JFW_PLUGIN_E_FAILED_VERSION)
                        continue;
                    else if (err !=JFW_PLUGIN_E_NONE)
                        return JFW_E_ERROR;

                    if (aInfo)
                    {
                        
                        if (aCurrentInfo.getLocation().isEmpty())
                            aCurrentInfo = aInfo;
                        
                        
                        
                        if ((aInfo.getFeatures() & nFeatureFlags) == nFeatureFlags)
                        {
                            
                            
                            aCurrentInfo = aInfo;
                            bInfoFound = true;
                            break;
                        }
                    }
                }
                if (bInfoFound == true)
                    break;
            }
        }
        if ((JavaInfo*) aCurrentInfo)
        {
            jfw::NodeJava javaNode(jfw::NodeJava::USER);
            javaNode.setJavaInfo(aCurrentInfo,true);
            javaNode.write();
            
            jfw::setJavaSelected();

            if (pInfo !=NULL)
            {
                
                *pInfo = aCurrentInfo.cloneJavaInfo();
            }
        }
        else
        {
            errcode = JFW_E_NO_JAVA_FOUND;
        }
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;
}

sal_Bool SAL_CALL jfw_areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    if (pInfoA == pInfoB)
        return sal_True;
    if (pInfoA == NULL || pInfoB == NULL)
        return sal_False;
    OUString sVendor(pInfoA->sVendor);
    OUString sLocation(pInfoA->sLocation);
    OUString sVersion(pInfoA->sVersion);
    rtl::ByteSequence sData(pInfoA->arVendorData);
    if (sVendor.equals(pInfoB->sVendor)
        && sLocation.equals(pInfoB->sLocation)
        && sVersion.equals(pInfoB->sVersion)
        && pInfoA->nFeatures == pInfoB->nFeatures
        && pInfoA->nRequirements == pInfoB->nRequirements
        && sData == pInfoB->arVendorData)
    {
        return sal_True;
    }
    return sal_False;
}


void SAL_CALL jfw_freeJavaInfo(JavaInfo *pInfo)
{
    if (pInfo == NULL)
        return;
    rtl_uString_release(pInfo->sVendor);
    rtl_uString_release(pInfo->sLocation);
    rtl_uString_release(pInfo->sVersion);
    rtl_byte_sequence_release(pInfo->arVendorData);
    rtl_freeMemory(pInfo);
}

javaFrameworkError SAL_CALL jfw_getSelectedJRE(JavaInfo **ppInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (ppInfo == NULL)
            return JFW_E_INVALID_ARG;

        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
        {
            OUString sJRE = jfw::BootParams::getJREHome();

            jfw::CJavaInfo aInfo;
            if ((errcode = jfw_getJavaInfoByPath(sJRE.pData, & aInfo.pInfo))
                != JFW_E_NONE)
                throw jfw::FrameworkException(
                    JFW_E_CONFIGURATION,
                    OString(
                        "[Java framework] The JRE specified by the bootstrap "
                        "variable UNO_JAVA_JFW_JREHOME  or  UNO_JAVA_JFW_ENV_JREHOME "
                        " could not be recognized. Check the values and make sure that you "
                        "use a plug-in library that can recognize that JRE."));

            *ppInfo = aInfo.detach();
            return JFW_E_NONE;
        }

        const jfw::MergedSettings settings;
        jfw::CJavaInfo aInfo;
        aInfo.attach(settings.createJavaInfo());
        if (! aInfo)
        {
            *ppInfo = NULL;
            return JFW_E_NONE;
        }
        
        
        
        OString sUpdated = jfw::getElementUpdated();

        if (!sUpdated.equals(settings.getJavaInfoAttrVendorUpdate()))
            return JFW_E_INVALID_SETTINGS;
        *ppInfo = aInfo.detach();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_isVMRunning(sal_Bool *bRunning)
{
    osl::MutexGuard guard(jfw::FwkMutex::get());
    if (bRunning == NULL)
        return JFW_E_INVALID_ARG;
    if (g_pJavaVM == NULL)
        *bRunning = sal_False;
    else
        *bRunning = sal_True;
    return JFW_E_NONE;
}

javaFrameworkError SAL_CALL jfw_getJavaInfoByPath(
    rtl_uString *pPath, JavaInfo **ppInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (pPath == NULL || ppInfo == NULL)
            return JFW_E_INVALID_ARG;

        jfw::VendorSettings aVendorSettings;
        
        std::vector<jfw::PluginLibrary> vecPlugins =
            aVendorSettings.getPluginData();
#ifndef DISABLE_DYNLOADING
        
        
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();
#endif
        typedef std::vector<OUString>::const_iterator CIT_VENDOR;
        std::vector<OUString> vecVendors =
            aVendorSettings.getSupportedVendors();

        
        
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
        for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end();
             ++i, ++cModule)
        {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);

#ifndef DISABLE_DYNLOADING
            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];
            if (!pluginLib.is())
            {
                OString msg = OUStringToOString(
                    library.sPath, osl_getThreadTextEncoding());
                fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                        "Modify the javavendors.xml accordingly!\n", msg.getStr());
                return JFW_E_NO_PLUGIN;
            }

            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                    OUString("jfw_plugin_getJavaInfoByPath"));
#else
            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                jfw_plugin_getJavaInfoByPath;
#endif
            OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
            if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                continue;

            
            
            
            JavaInfo* pInfo = NULL;
            javaPluginError plerr = (*jfw_plugin_getJavaInfoByPathFunc)(
                pPath,
                library.sVendor.pData,
                versionInfo.sMinVersion.pData,
                versionInfo.sMaxVersion.pData,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & pInfo);

            if (plerr == JFW_PLUGIN_E_NONE)
            {
                
                if (vecVendors.empty())
                {
                    
                    *ppInfo = pInfo;
                    break;
                }
                else
                {
                    OUString sVendor(pInfo->sVendor);
                    CIT_VENDOR ivendor = std::find(vecVendors.begin(), vecVendors.end(),
                                                   sVendor);
                    if (ivendor != vecVendors.end())
                    {
                        *ppInfo = pInfo;
                    }
                    else
                    {
                        *ppInfo = NULL;
                        errcode = JFW_E_NOT_RECOGNIZED;
                    }
                    break;
                }
            }
            else if(plerr == JFW_PLUGIN_E_FAILED_VERSION)
            {
                *ppInfo = NULL;
                errcode = JFW_E_FAILED_VERSION;
                break;
            }
            else if (plerr == JFW_PLUGIN_E_NO_JRE)
            {
                continue;
            }
            OSL_ASSERT(false);
        }
        if (*ppInfo == NULL && errcode != JFW_E_FAILED_VERSION)
            errcode = JFW_E_NOT_RECOGNIZED;
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;
}


javaFrameworkError SAL_CALL jfw_setSelectedJRE(JavaInfo const *pInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        
        JavaInfo *currentInfo = NULL;
        errcode = jfw_getSelectedJRE( & currentInfo);
        if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
            return errcode;

        if (jfw_areEqualJavaInfo(currentInfo, pInfo) == sal_False)
        {
            jfw::NodeJava node(jfw::NodeJava::USER);
            node.setJavaInfo(pInfo, false);
            node.write();
            
            jfw::setJavaSelected();
        }
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}
javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        if (g_bEnabledSwitchedOn == false && bEnabled == sal_True)
        {
            
            
            
            

            
            const jfw::MergedSettings settings;
            if (!settings.getEnabled())
                g_bEnabledSwitchedOn = true;
        }
        jfw::NodeJava node(jfw::NodeJava::USER);
        node.setEnabled(bEnabled);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_getEnabled(sal_Bool *pbEnabled)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (pbEnabled == NULL)
            return JFW_E_INVALID_ARG;
        jfw::MergedSettings settings;
        *pbEnabled = settings.getEnabled();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}


javaFrameworkError SAL_CALL jfw_setVMParameters(
    rtl_uString * * arOptions, sal_Int32 nLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (arOptions == NULL && nLen != 0)
            return JFW_E_INVALID_ARG;
        node.setVmParameters(arOptions, nLen);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;
}

javaFrameworkError SAL_CALL jfw_getVMParameters(
    rtl_uString *** parOptions, sal_Int32 * pLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        if (parOptions == NULL || pLen == NULL)
            return JFW_E_INVALID_ARG;
        const jfw::MergedSettings settings;
        settings.getVmParametersArray(parOptions, pLen);
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_setUserClassPath(rtl_uString * pCp)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (pCp == NULL)
            return JFW_E_INVALID_ARG;
        node.setUserClassPath(pCp);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_getUserClassPath(rtl_uString ** ppCP)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        if (ppCP == NULL)
            return JFW_E_INVALID_ARG;
        const jfw::MergedSettings settings;
        *ppCP = settings.getUserClassPath().pData;
        rtl_uString_acquire(*ppCP);
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_addJRELocation(rtl_uString * sLocation)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (sLocation == NULL)
            return JFW_E_INVALID_ARG;
        node.load();
        node.addJRELocation(sLocation);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;

}

javaFrameworkError SAL_CALL jfw_setJRELocations(
    rtl_uString ** arLocations, sal_Int32 nLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (arLocations == NULL && nLen != 0)
            return JFW_E_INVALID_ARG;
        node.setJRELocations(arLocations, nLen);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;

}

javaFrameworkError SAL_CALL jfw_getJRELocations(
    rtl_uString *** parLocations, sal_Int32 *pLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        if (parLocations == NULL || pLen == NULL)
            return JFW_E_INVALID_ARG;
        const jfw::MergedSettings settings;
        settings.getJRELocations(parLocations, pLen);
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }

    return errcode;
}


javaFrameworkError jfw_existJRE(const JavaInfo *pInfo, sal_Bool *exist)
{
    
    jfw::VendorSettings aVendorSettings;
    jfw::CJavaInfo aInfo;
    aInfo = (const ::JavaInfo*) pInfo; 
#ifndef DISABLE_DYNLOADING
    OUString sLibPath = aVendorSettings.getPluginLibrary(aInfo.getVendor());
    osl::Module modulePlugin(sLibPath);
    if ( ! modulePlugin)
        return JFW_E_NO_PLUGIN;
    OUString sFunctionName("jfw_plugin_existJRE");
    jfw_plugin_existJRE_ptr pFunc =
        (jfw_plugin_existJRE_ptr)
        osl_getFunctionSymbol(modulePlugin, sFunctionName.pData);
    if (pFunc == NULL)
        return JFW_E_ERROR;
#else
    jfw_plugin_existJRE_ptr pFunc =
        jfw_plugin_existJRE;
#endif
    javaPluginError plerr = (*pFunc)(pInfo, exist);

    javaFrameworkError ret = JFW_E_NONE;
    switch (plerr)
    {
    case JFW_PLUGIN_E_NONE:
        ret = JFW_E_NONE;
        break;
    case JFW_PLUGIN_E_INVALID_ARG:
        ret = JFW_E_INVALID_ARG;
        break;
    case JFW_PLUGIN_E_ERROR:
        ret = JFW_E_ERROR;
        break;
    default:
        ret = JFW_E_ERROR;
    }
    return ret;
}

void SAL_CALL jfw_lock()
{
    jfw::FwkMutex::get().acquire();
}

void SAL_CALL jfw_unlock()
{
    jfw::FwkMutex::get().release();
}


namespace jfw
{
CJavaInfo::CJavaInfo(): pInfo(0)
{
}

CJavaInfo::CJavaInfo(const CJavaInfo & info)
{
    pInfo = copyJavaInfo(info.pInfo);
}

CJavaInfo::CJavaInfo(::JavaInfo * info, _transfer_ownership)
{
    pInfo = info;
}
CJavaInfo CJavaInfo::createWrapper(::JavaInfo* info)
{
    return CJavaInfo(info, TRANSFER);
}
void CJavaInfo::attach(::JavaInfo * info)
{
    jfw_freeJavaInfo(pInfo);
    pInfo = info;
}
::JavaInfo * CJavaInfo::detach()
{
    JavaInfo * tmp = pInfo;
    pInfo = NULL;
    return tmp;
}

CJavaInfo::~CJavaInfo()
{
    jfw_freeJavaInfo(pInfo);
}

CJavaInfo::operator ::JavaInfo* ()
{
    return pInfo;
}

JavaInfo * CJavaInfo::copyJavaInfo(const JavaInfo * pInfo)
{
    if (pInfo == NULL)
        return NULL;
    JavaInfo* newInfo =
          (JavaInfo*) rtl_allocateMemory(sizeof(JavaInfo));
    if (newInfo)
    {
        memcpy(newInfo, pInfo, sizeof(JavaInfo));
        rtl_uString_acquire(pInfo->sVendor);
        rtl_uString_acquire(pInfo->sLocation);
        rtl_uString_acquire(pInfo->sVersion);
        rtl_byte_sequence_acquire(pInfo->arVendorData);
    }
    return newInfo;
}


JavaInfo* CJavaInfo::cloneJavaInfo() const
{
    if (pInfo == NULL)
        return NULL;
    return copyJavaInfo(pInfo);
}

CJavaInfo & CJavaInfo::operator = (const CJavaInfo& info)
{
    if (&info == this)
        return *this;

    jfw_freeJavaInfo(pInfo);
    pInfo = copyJavaInfo(info.pInfo);
    return *this;
}
CJavaInfo & CJavaInfo::operator = (const ::JavaInfo* info)
{
    if (info == pInfo)
        return *this;

    jfw_freeJavaInfo(pInfo);
    pInfo = copyJavaInfo(info);
    return *this;
}

const ::JavaInfo* CJavaInfo::operator ->() const
{
    return pInfo;
}

CJavaInfo::operator JavaInfo const * () const
{
    return pInfo;
}

OUString CJavaInfo::getVendor() const
{
    if (pInfo)
        return OUString(pInfo->sVendor);
    else
        return OUString();
}

OUString CJavaInfo::getLocation() const
{
    if (pInfo)
        return OUString(pInfo->sLocation);
    else
        return OUString();
}

sal_uInt64 CJavaInfo::getFeatures() const
{
    if (pInfo)
        return pInfo->nFeatures;
    else
        return 0l;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
