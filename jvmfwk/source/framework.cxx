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

#include "boost/scoped_array.hpp"
#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"
#include <vector>
#include <functional>
#include <algorithm>
#include "framework.hxx"
#include "fwkutil.hxx"
#include "elements.hxx"
#include "fwkbase.hxx"

#ifdef WNT
/** The existence of the file useatjava.txt decides if a Java should be used
    that supports accessibility tools.
 */
#define USE_ACCESSIBILITY_FILE "useatjava.txt"
#endif

#define UNO_JAVA_JFW_JREHOME "UNO_JAVA_JFW_JREHOME"
namespace {
JavaVM * g_pJavaVM = NULL;

bool g_bEnabledSwitchedOn = false;

sal_Bool areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    return jfw_areEqualJavaInfo(pInfoA, pInfoB);
}
}

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
        //Get a list of plugins which provide Java information
        std::vector<jfw::PluginLibrary> vecPlugins =
            aVendorSettings.getPluginData();

        //Create a vector that holds the libraries, which will be later
        //dynamically loaded;
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();
        //Add the JavaInfos found by jfw_plugin_getAllJavaInfos to the vector
        //Make sure that the contents are destroyed if this
        //function returns with an error
        std::vector<jfw::CJavaInfo> vecInfo;
        //Add the JavaInfos found by jfw_plugin_getJavaInfoByPath to this vector
        //Make sure that the contents are destroyed if this
        //function returns with an error
        std::vector<jfw::CJavaInfo> vecInfoManual;
        typedef std::vector<jfw::CJavaInfo>::iterator it_info;
        //get the list of paths to jre locations which have been
        //added manually
        const jfw::MergedSettings settings;
        const std::vector<rtl::OUString>& vecJRELocations =
            settings.getJRELocations();
        //Use every plug-in library to get Java installations.
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
         for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i, ++cModule)
         {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);
            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];

            if (pluginLib.is() == sal_False)
            {
                rtl::OString msg = rtl::OUStringToOString(
                    library.sPath, osl_getThreadTextEncoding());
                fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                        "Modify the javavendors.xml accordingly!\n", msg.getStr());
                return JFW_E_NO_PLUGIN;
            }
            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getFunctionSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getAllJavaInfos")));

            OSL_ASSERT(getAllJavaFunc);
            if (getAllJavaFunc == NULL)
                return JFW_E_ERROR;

            //get all installations of one vendor according to minVersion,
            //maxVersion and excludeVersions
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

            //Check if the current plugin can detect JREs at the location
            // of the paths added by jfw_setJRELocations or jfw_addJRELocation
            //get the function from the plugin
            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getJavaInfoByPath")));

            OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
            if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                return JFW_E_ERROR;

            typedef std::vector<rtl::OUString>::const_iterator citLoc;
            //Check every manually added location
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
                    //Was this JRE already added?. Different plugins could detect
                    //the same JRE
                    it_info it_duplicate =
                        std::find_if(vecInfoManual.begin(), vecInfoManual.end(),
                                std::bind2nd(std::ptr_fun(areEqualJavaInfo), aInfo));
                    if (it_duplicate == vecInfoManual.end())
                        vecInfoManual.push_back(aInfo);
                }
            }
        }
        //Make sure vecInfoManual contains only JavaInfos for the vendors for which
        //there is a javaSelection/plugins/library entry in the javavendors.xml
        //To obtain the JavaInfos for the manually added JRE locations the function
        //jfw_getJavaInfoByPath is called which can return a JavaInfo of any vendor.
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
        //Check which JavaInfo from vector vecInfoManual2 is already
        //contained in vecInfo. If it already exists then remove it from
        //vecInfoManual2
        for (it_info j = vecInfo.begin(); j != vecInfo.end(); ++j)
        {
            it_info it_duplicate =
                std::find_if(vecInfoManual2.begin(), vecInfoManual2.end(),
                            std::bind2nd(std::ptr_fun(areEqualJavaInfo), *j));
            if (it_duplicate != vecInfoManual2.end())
                vecInfoManual2.erase(it_duplicate);
        }
        //create an fill the array of JavaInfo*
        sal_Int32 nSize = vecInfo.size() + vecInfoManual2.size();
        *pparInfo = (JavaInfo**) rtl_allocateMemory(
            nSize * sizeof(JavaInfo*));
        if (*pparInfo == NULL)
            return JFW_E_ERROR;

        typedef std::vector<jfw::CJavaInfo>::iterator it;
        int index = 0;
        //Add the automatically detected JREs
        for (it k = vecInfo.begin(); k != vecInfo.end(); ++k)
            (*pparInfo)[index++] = k->detach();
        //Add the manually detected JREs
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

javaFrameworkError SAL_CALL jfw_startVM(JavaVMOption *arOptions, sal_Int32 cOptions,
                                 JavaVM **ppVM, JNIEnv **ppEnv)
{
#ifndef SOLAR_JAVA
    (void) arOptions;
    (void) cOptions;
    (void) ppVM;
    (void) ppEnv;

    return JFW_E_ERROR;
#else
    javaFrameworkError errcode = JFW_E_NONE;
    if (cOptions > 0 && arOptions == NULL)
        return JFW_E_INVALID_ARG;

    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        //We keep this pointer so we can determine if a VM has already
        //been created.
        if (g_pJavaVM != NULL)
            return JFW_E_RUNNING_JVM;

        if (ppVM == NULL)
            return JFW_E_INVALID_ARG;

        std::vector<rtl::OString> vmParams;
        rtl::OString sUserClassPath;
        jfw::CJavaInfo aInfo;
        jfw::JFW_MODE mode = jfw::getMode();
        if (mode == jfw::JFW_MODE_APPLICATION)
        {
            const jfw::MergedSettings settings;
            if (sal_False == settings.getEnabled())
                return JFW_E_JAVA_DISABLED;
            aInfo.attach(settings.createJavaInfo());
            //check if a Java has ever been selected
            if (aInfo == NULL)
                return JFW_E_NO_SELECT;

#ifdef WNT
            //Because on Windows there is no system setting that we can use to determine
            //if Assistive Technology Tool support is needed, we ship a .reg file that the
            //user can use to create a registry setting. When the user forgets to set
            //the key before he starts the office then a JRE may be selected without access bridge.
            //When he later sets the key then we select a JRE with accessibility support but
            //only if the user has not manually changed the selected JRE in the options dialog.
            if (jfw::isAccessibilitySupportDesired())
            {
                // If no JRE has been selected then we do not select one. This function shall then
                //return JFW_E_NO_SELECT
                if (aInfo != NULL &&
                    (aInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE) == 0)
                {
                    //has the user manually selected a JRE?
                    if (settings.getJavaInfoAttrAutoSelect() == true)
                    {
                        // if not then the automatism has previously selected a JRE
                        //without accessibility support. We return JFW_E_NO_SELECT
                        //to cause that we search for another JRE. The search code will
                        //then prefer a JRE with accessibility support.
                        return JFW_E_NO_SELECT;
                    }
                }
            }
#endif
            //check if the javavendors.xml has changed after a Java was selected
            rtl::OString sVendorUpdate = jfw::getElementUpdated();

            if (sVendorUpdate != settings.getJavaInfoAttrVendorUpdate())
                return JFW_E_INVALID_SETTINGS;

            //check if JAVA is disabled
            //If Java is enabled, but it was disabled when this process was started
            // then no preparational work, such as setting the LD_LIBRARY_PATH, was
            //done. Therefore if a JRE needs it it must not be started.
            if (g_bEnabledSwitchedOn &&
                    (aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
                return JFW_E_NEED_RESTART;

            //Check if the selected Java was set in this process. If so it
            //must not have the requirments flag JFW_REQUIRE_NEEDRESTART
            if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
                &&
                (jfw::wasJavaSelectedInSameProcess() == true))
                return JFW_E_NEED_RESTART;

            vmParams = settings.getVmParametersUtf8();
            sUserClassPath = jfw::makeClassPathOption(settings.getUserClassPath());
        } // end mode FWK_MODE_OFFICE
        else if (mode == jfw::JFW_MODE_DIRECT)
        {
            errcode = jfw_getSelectedJRE(&aInfo.pInfo);
            if (errcode != JFW_E_NONE)
                return errcode;
            //In direct mode the options are specified by bootstrap variables
            //of the form UNO_JAVA_JFW_PARAMETER_1 .. UNO_JAVA_JFW_PARAMETER_n
            vmParams = jfw::BootParams::getVMParameters();
            sUserClassPath =
                "-Djava.class.path=" + jfw::BootParams::getClasspath();
        }
        else
            OSL_ASSERT(0);

        //get the function jfw_plugin_startJavaVirtualMachine
        jfw::VendorSettings aVendorSettings;
        rtl::OUString sLibPath = aVendorSettings.getPluginLibrary(aInfo.getVendor());

        osl::Module modulePlugin(sLibPath);
        if ( ! modulePlugin)
            return JFW_E_NO_PLUGIN;

        rtl::OUString sFunctionName(
            RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_startJavaVirtualMachine"));
        jfw_plugin_startJavaVirtualMachine_ptr pFunc =
            (jfw_plugin_startJavaVirtualMachine_ptr)
            osl_getFunctionSymbol(modulePlugin, sFunctionName.pData);
        if (pFunc == NULL)
            return JFW_E_ERROR;

        // create JavaVMOptions array that is passed to the plugin
        // it contains the classpath and all options set in the
        //options dialog
        boost::scoped_array<JavaVMOption> sarJOptions(
            new JavaVMOption[cOptions + 2 + vmParams.size()]);
        JavaVMOption * arOpt = sarJOptions.get();
        if (! arOpt)
            return JFW_E_ERROR;

        //The first argument is the classpath
        arOpt[0].optionString= (char*) sUserClassPath.getStr();
        arOpt[0].extraInfo = NULL;
        // Set a flag that this JVM has been created via the JNI Invocation API
        // (used, for example, by UNO remote bridges to share a common thread pool
        // factory among Java and native bridge implementations):
        arOpt[1].optionString = (char *) "-Dorg.openoffice.native=";
        arOpt[1].extraInfo = 0;

        //add the options set by options dialog
        int index = 2;
        typedef std::vector<rtl::OString>::const_iterator cit;
        for (cit i = vmParams.begin(); i != vmParams.end(); ++i)
        {
            arOpt[index].optionString = const_cast<sal_Char*>(i->getStr());
            arOpt[index].extraInfo = 0;
            index ++;
        }
        //add all options of the arOptions argument
        for (int ii = 0; ii < cOptions; ii++)
        {
            arOpt[index].optionString = arOptions[ii].optionString;
            arOpt[index].extraInfo = arOptions[ii].extraInfo;
            index++;
        }

        //start Java
        JavaVM *pVm = NULL;
        SAL_INFO("jvmfwk", "starting java");
        javaPluginError plerr = (*pFunc)(aInfo, arOpt, index, & pVm, ppEnv);
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
#endif
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
//Determine if accessibility support is needed
        bool bSupportAccessibility = jfw::isAccessibilitySupportDesired();
        nFeatureFlags = bSupportAccessibility ?
            JFW_FEATURE_ACCESSBRIDGE : 0L;

        //Get a list of services which provide Java information
        jfw::VendorSettings aVendorSettings;
        std::vector<jfw::PluginLibrary> vecPlugins =
             aVendorSettings.getPluginData();
        //Create a vector that holds the libraries, which will be later
        //dynamically loaded;
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();

        //Use every plug-in library to get Java installations. At the first usable
        //Java the loop will break
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
        for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i, ++cModule)
        {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);

            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];
            if (pluginLib.is() == sal_False)
                return JFW_E_NO_PLUGIN;

            jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
                (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getFunctionSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getAllJavaInfos")));

            OSL_ASSERT(getAllJavaFunc);
            if (getAllJavaFunc == NULL)
                continue;

            //get all installations of one vendor according to minVersion,
            //maxVersion and excludeVersions
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
            //iterate over all installations to find the best which has
            //all features
            if (cInfos == 0)
            {
                rtl_freeMemory(arInfos);
                continue;
            }
            bool bInfoFound = false;
            for (int ii = 0; ii < cInfos; ii++)
            {
                JavaInfo* pJInfo = arInfos[ii];

                //We remember the very first installation in aCurrentInfo
                if (aCurrentInfo.getLocation().isEmpty())
                        aCurrentInfo = pJInfo;
                // compare features
                // If the user does not require any features (nFeatureFlags = 0)
                // then the first installation is used
                if ((pJInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                {
                    //the just found Java implements all required features
                    //currently there is only accessibility!!!
                    aCurrentInfo = pJInfo;
                    bInfoFound = true;
                    break;
                }
            }
            //The array returned by jfw_plugin_getAllJavaInfos must be freed as well as
            //its contents
            for (int j = 0; j < cInfos; j++)
                jfw_freeJavaInfo(arInfos[j]);
            rtl_freeMemory(arInfos);

            if (bInfoFound == true)
                break;
            //All Java installations found by the current plug-in lib
            //do not provide the required features. Try the next plug-in
        }
        if ((JavaInfo*) aCurrentInfo == NULL)
        {//The plug-ins did not find a suitable Java. Now try the paths which have been
        //added manually.
            //get the list of paths to jre locations which have been added manually
            const jfw::MergedSettings settings;
            //node.loadFromSettings();
            const std::vector<rtl::OUString> & vecJRELocations =
                settings.getJRELocations();
            //use every plug-in to determine the JavaInfo objects
            bool bInfoFound = false;
            for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); ++i)
            {
                const jfw::PluginLibrary & library = *i;
                jfw::VersionInfo versionInfo =
                    aVendorSettings.getVersionInformation(library.sVendor);

                osl::Module pluginLib(library.sPath);
                if (pluginLib.is() == sal_False)
                    return JFW_E_NO_PLUGIN;
                //Check if the current plugin can detect JREs at the location
                // of the paths added by jfw_setJRELocations or jfw_addJRELocation
                //get the function from the plugin
                jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                    (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getJavaInfoByPath")));

                OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
                if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                    return JFW_E_ERROR;

                typedef std::vector<rtl::OUString>::const_iterator citLoc;
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
                        //We remember the very first installation in aCurrentInfo
                        if (aCurrentInfo.getLocation().isEmpty())
                            aCurrentInfo = aInfo;
                        // compare features
                        // If the user does not require any features (nFeatureFlags = 0)
                        // then the first installation is used
                        if ((aInfo.getFeatures() & nFeatureFlags) == nFeatureFlags)
                        {
                            //the just found Java implements all required features
                            //currently there is only accessibility!!!
                            aCurrentInfo = aInfo;
                            bInfoFound = true;
                            break;
                        }
                    }
                }//end iterate over paths
                if (bInfoFound == true)
                    break;
            }// end iterate plug-ins
        }
        if ((JavaInfo*) aCurrentInfo)
        {
            jfw::NodeJava javaNode(jfw::NodeJava::USER);
            javaNode.setJavaInfo(aCurrentInfo,true);
            javaNode.write();

            if (pInfo !=NULL)
            {
                //copy to out param
                *pInfo = aCurrentInfo.cloneJavaInfo();
                //remember that this JRE was selected in this process
                jfw::setJavaSelected();
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
    rtl::OUString sVendor(pInfoA->sVendor);
    rtl::OUString sLocation(pInfoA->sLocation);
    rtl::OUString sVersion(pInfoA->sVersion);
    rtl::ByteSequence sData(pInfoA->arVendorData);
    if (sVendor.equals(pInfoB->sVendor) == sal_True
        && sLocation.equals(pInfoB->sLocation) == sal_True
        && sVersion.equals(pInfoB->sVersion) == sal_True
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
            rtl::OUString sJRE = jfw::BootParams::getJREHome();

            jfw::CJavaInfo aInfo;
            if ((errcode = jfw_getJavaInfoByPath(sJRE.pData, & aInfo.pInfo))
                != JFW_E_NONE)
                throw jfw::FrameworkException(
                    JFW_E_CONFIGURATION,
                    rtl::OString(
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
        //If the javavendors.xml has changed, then the current selected
        //Java is not valid anymore
        // /java/javaInfo/@vendorUpdate != javaSelection/updated (javavendors.xml)
        rtl::OString sUpdated = jfw::getElementUpdated();

        if (sUpdated.equals(settings.getJavaInfoAttrVendorUpdate()) == sal_False)
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
        //Get a list of plugins which provide Java information
        std::vector<jfw::PluginLibrary> vecPlugins =
            aVendorSettings.getPluginData();
        //Create a vector that holds the libraries, which will be later
        //dynamically loaded;
        boost::scoped_array<osl::Module> sarModules;
        sarModules.reset(new osl::Module[vecPlugins.size()]);
        osl::Module * arModules = sarModules.get();

        typedef std::vector<rtl::OUString>::const_iterator CIT_VENDOR;
        std::vector<rtl::OUString> vecVendors =
            aVendorSettings.getSupportedVendors();

        //Use every plug-in library to determine if the path represents a
        //JRE. If a plugin recognized it then the loop will break
        typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
        int cModule = 0;
        for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end();
             ++i, ++cModule)
        {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(library.sVendor);
            arModules[cModule].load(library.sPath);
            osl::Module & pluginLib = arModules[cModule];
            if (pluginLib.is() == sal_False)
            {
                rtl::OString msg = rtl::OUStringToOString(
                    library.sPath, osl_getThreadTextEncoding());
                fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                        "Modify the javavendors.xml accordingly!\n", msg.getStr());
                return JFW_E_NO_PLUGIN;
            }

            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getFunctionSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getJavaInfoByPath")));

            OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
            if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                continue;

            //ask the plugin if this is a JRE.
            //If so check if it meets the version requirements.
            //Only if it does return a JavaInfo
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
                //check if the vendor of the found JRE is supported
                if (vecVendors.empty())
                {
                    //vendor does not matter
                    *ppInfo = pInfo;
                    break;
                }
                else
                {
                    rtl::OUString sVendor(pInfo->sVendor);
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
            {//found JRE but it has the wrong version
                *ppInfo = NULL;
                errcode = JFW_E_FAILED_VERSION;
                break;
            }
            else if (plerr == JFW_PLUGIN_E_NO_JRE)
            {// plugin does not recognize this path as belonging to JRE
                continue;
            }
            OSL_ASSERT(0);
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
        //check if pInfo is the selected JRE
        JavaInfo *currentInfo = NULL;
        errcode = jfw_getSelectedJRE( & currentInfo);
        if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
            return errcode;

        if (jfw_areEqualJavaInfo(currentInfo, pInfo) == sal_False)
        {
            jfw::NodeJava node(jfw::NodeJava::USER);
            node.setJavaInfo(pInfo, false);
            node.write();
            //remember that the JRE was selected in this process
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
            //When the process started then Enabled was false.
            //This is first time enabled is set to true.
            //That means, no preparational work has been done, such as setting the
            //LD_LIBRARY_PATH, etc.

            //check if Enabled is false;
            const jfw::MergedSettings settings;
            if (settings.getEnabled() == sal_False)
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
    //get the function jfw_plugin_existJRE
    jfw::VendorSettings aVendorSettings;
    jfw::CJavaInfo aInfo;
    aInfo = (const ::JavaInfo*) pInfo; //makes a copy of pInfo
    rtl::OUString sLibPath = aVendorSettings.getPluginLibrary(aInfo.getVendor());
    osl::Module modulePlugin(sLibPath);
    if ( ! modulePlugin)
        return JFW_E_NO_PLUGIN;
    rtl::OUString sFunctionName(
        RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_existJRE"));
    jfw_plugin_existJRE_ptr pFunc =
        (jfw_plugin_existJRE_ptr)
        osl_getFunctionSymbol(modulePlugin, sFunctionName.pData);
    if (pFunc == NULL)
        return JFW_E_ERROR;

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
        rtl_copyMemory(newInfo, pInfo, sizeof(JavaInfo));
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

rtl::OUString CJavaInfo::getVendor() const
{
    if (pInfo)
        return rtl::OUString(pInfo->sVendor);
    else
        return rtl::OUString();
}

rtl::OUString CJavaInfo::getLocation() const
{
    if (pInfo)
        return rtl::OUString(pInfo->sLocation);
    else
        return rtl::OUString();
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
