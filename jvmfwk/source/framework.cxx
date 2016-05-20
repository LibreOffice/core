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

#include <memory>
#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "jvmfwk/framework.hxx"
#include "vendorplugin.hxx"
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

static JavaVM * g_pJavaVM = nullptr;

bool areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    return jfw_areEqualJavaInfo(pInfoA, pInfoB);
}

}

javaFrameworkError jfw_findAllJREs(JavaInfo ***pparInfo, sal_Int32 *pSize)
{
    javaFrameworkError retVal = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        javaFrameworkError errcode = JFW_E_NONE;
        if (pparInfo == nullptr || pSize == nullptr)
            return JFW_E_INVALID_ARG;

        jfw::VendorSettings aVendorSettings;
        std::vector<OUString> vecVendors =
            aVendorSettings.getSupportedVendors();
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
        const std::vector<OUString>& vecJRELocations =
            settings.getJRELocations();
        //Use every plug-in library to get Java installations.
        typedef std::vector<OUString>::const_iterator ci_pl;
        for (ci_pl i = vecVendors.begin(); i != vecVendors.end(); ++i)
        {
            const OUString & vendor = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(vendor);

            //get all installations of one vendor according to minVersion,
            //maxVersion and excludeVersions
            sal_Int32 cInfos = 0;
            JavaInfo** arInfos = nullptr;
            std::vector<rtl::Reference<jfw_plugin::VendorBase>> infos;
            javaPluginError plerr = jfw_plugin_getAllJavaInfos(
                true,
                vendor,
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & arInfos,
                & cInfos,
                infos);

            if (plerr != javaPluginError::NONE)
                return JFW_E_ERROR;

            for (int j = 0; j < cInfos; j++)
                vecInfo.push_back(jfw::CJavaInfo::createWrapper(arInfos[j]));

            rtl_freeMemory(arInfos);

            //Check if the current plugin can detect JREs at the location
            // of the paths added by jfw_addJRELocation
            //get the function from the plugin
            typedef std::vector<OUString>::const_iterator citLoc;
            //Check every manually added location
            for (citLoc ii = vecJRELocations.begin();
                ii != vecJRELocations.end(); ++ii)
            {
                jfw::CJavaInfo aInfo;
                plerr = jfw_plugin_getJavaInfoByPath(
                    *ii,
                    vendor,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.getExcludeVersions(),
                    versionInfo.getExcludeVersionSize(),
                    & aInfo.pInfo);
                if (plerr == javaPluginError::NoJre)
                    continue;
                if (plerr == javaPluginError::FailedVersion)
                    continue;
                else if (plerr != javaPluginError::NONE)
                    return JFW_E_ERROR;

                if (aInfo)
                {
                    //Was this JRE already added?. Different plugins could detect
                    //the same JRE
                    it_info it_duplicate =
                        std::find_if(vecInfoManual.begin(), vecInfoManual.end(),
                                     std::bind(areEqualJavaInfo, std::placeholders::_1, aInfo));
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
            for (ci_pl ii = vecVendors.begin(); ii != vecVendors.end(); ++ii)
            {
                if ( ii->equals((*ivm)->sVendor))
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
                             std::bind(areEqualJavaInfo, std::placeholders::_1, *j));
            if (it_duplicate != vecInfoManual2.end())
                vecInfoManual2.erase(it_duplicate);
        }
        //create an fill the array of JavaInfo*
        sal_Int32 nSize = vecInfo.size() + vecInfoManual2.size();
        *pparInfo = static_cast<JavaInfo**>(rtl_allocateMemory(
            nSize * sizeof(JavaInfo*)));
        if (*pparInfo == nullptr)
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

javaFrameworkError jfw_startVM(
    JavaInfo const * pInfo, JavaVMOption * arOptions, sal_Int32 cOptions,
    JavaVM ** ppVM, JNIEnv ** ppEnv)
{
    javaFrameworkError errcode = JFW_E_NONE;
    if (cOptions > 0 && arOptions == nullptr)
        return JFW_E_INVALID_ARG;

    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        //We keep this pointer so we can determine if a VM has already
        //been created.
        if (g_pJavaVM != nullptr)
            return JFW_E_RUNNING_JVM;

        if (ppVM == nullptr)
            return JFW_E_INVALID_ARG;

        std::vector<OString> vmParams;
        OString sUserClassPath;
        jfw::CJavaInfo aInfo;
        if (pInfo == nullptr)
        {
            jfw::JFW_MODE mode = jfw::getMode();
            if (mode == jfw::JFW_MODE_APPLICATION)
            {
                const jfw::MergedSettings settings;
                if (!settings.getEnabled())
                    return JFW_E_JAVA_DISABLED;
                aInfo.attach(settings.createJavaInfo());
                //check if a Java has ever been selected
                if (aInfo == nullptr)
                    return JFW_E_NO_SELECT;

#ifdef _WIN32
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
                OString sVendorUpdate = jfw::getElementUpdated();

                if (sVendorUpdate != settings.getJavaInfoAttrVendorUpdate())
                    return JFW_E_INVALID_SETTINGS;

                //check if JAVA is disabled
                //If Java is enabled, but it was disabled when this process was started
                // then no preparational work, such as setting the LD_LIBRARY_PATH, was
                //done. Therefore if a JRE needs it, it must not be started.
                if (g_bEnabledSwitchedOn &&
                    (aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
                    return JFW_E_NEED_RESTART;

                //Check if the selected Java was set in this process. If so it
                //must not have the requirments flag JFW_REQUIRE_NEEDRESTART
                if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
                    && jfw::wasJavaSelectedInSameProcess())
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
                OSL_ASSERT(false);
            pInfo = aInfo.pInfo;
        }
        assert(pInfo != nullptr);

        //get the function jfw_plugin_startJavaVirtualMachine
        jfw::VendorSettings aVendorSettings;

        // create JavaVMOptions array that is passed to the plugin
        // it contains the classpath and all options set in the
        //options dialog
        std::unique_ptr<JavaVMOption[]> sarJOptions(
            new JavaVMOption[cOptions + 2 + vmParams.size()]);
        JavaVMOption * arOpt = sarJOptions.get();
        if (! arOpt)
            return JFW_E_ERROR;

        //The first argument is the classpath
        arOpt[0].optionString= const_cast<char*>(sUserClassPath.getStr());
        arOpt[0].extraInfo = nullptr;
        // Set a flag that this JVM has been created via the JNI Invocation API
        // (used, for example, by UNO remote bridges to share a common thread pool
        // factory among Java and native bridge implementations):
        arOpt[1].optionString = const_cast<char *>("-Dorg.openoffice.native=");
        arOpt[1].extraInfo = nullptr;

        //add the options set by options dialog
        int index = 2;
        typedef std::vector<OString>::const_iterator cit;
        for (cit i = vmParams.begin(); i != vmParams.end(); ++i)
        {
            arOpt[index].optionString = const_cast<sal_Char*>(i->getStr());
            arOpt[index].extraInfo = nullptr;
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
        JavaVM *pVm = nullptr;
        SAL_INFO("jfw", "Starting Java");
        javaPluginError plerr = jfw_plugin_startJavaVirtualMachine(pInfo, arOpt, index, & pVm, ppEnv);
        if (plerr == javaPluginError::VmCreationFailed)
        {
            errcode = JFW_E_VM_CREATION_FAILED;
        }
        else if (plerr != javaPluginError::NONE )
        {
            errcode = JFW_E_ERROR;
        }
        else
        {
            g_pJavaVM = pVm;
            *ppVM = pVm;
        }
        OSL_ASSERT(plerr != javaPluginError::WrongVendor);
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
    may take quite a while. The implementation first inspects JAVA_HOME and
    PATH environment variables. If no suitable JavaInfo is found there, it
    inspects all JavaInfos found by the jfw_plugin_get* functions.
 */
javaFrameworkError jfw_findAndSelectJRE(JavaInfo **pInfo)
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


        // 'bInfoFound' indicates whether a Java installation has been found
        // that supports all desired features
        bool bInfoFound = false;

        // get list of vendors for Java installations
        jfw::VendorSettings aVendorSettings;
        std::vector<OUString> vecVendors =
            aVendorSettings.getSupportedVendors();

        // save vendors and respective version requirements pair-wise in a vector
        std::vector<std::pair<OUString, jfw::VersionInfo>> versionInfos;
        typedef std::vector<OUString>::const_iterator ciVendor;
        for (ciVendor i = vecVendors.begin(); i != vecVendors.end(); ++i)
        {
            const OUString & vendor = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(vendor);

            versionInfos.push_back(
                std::pair<OUString, jfw::VersionInfo>(vendor, versionInfo));
        }

        std::vector<rtl::Reference<jfw_plugin::VendorBase>> infos;

        // first inspect Java installation that the JAVA_HOME
        // environment variable points to (if it is set)
        JavaInfo* pHomeInfo = nullptr;
        if (jfw_plugin_getJavaInfoFromJavaHome(versionInfos, &pHomeInfo, infos)
            == javaPluginError::NONE)
        {
            aCurrentInfo = pHomeInfo;

            // compare features
            // if the user does not require any features (nFeatureFlags = 0)
            // or the Java installation provides all features, then this installation is used
            if ((pHomeInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
            {
                bInfoFound = true;
            }
            delete pHomeInfo;
        }

        // if no Java installation providing all features was detected by using JAVA_HOME,
        // query PATH for Java installations
        if (!bInfoFound)
        {
            std::vector<JavaInfo*> vecJavaInfosFromPath;
            if (jfw_plugin_getJavaInfosFromPath(
                    versionInfos, vecJavaInfosFromPath, infos)
                == javaPluginError::NONE)
            {
                std::vector<JavaInfo*>::const_iterator it = vecJavaInfosFromPath.begin();
                while(it != vecJavaInfosFromPath.end() && !bInfoFound)
                {
                    JavaInfo* pJInfo = *it;
                    if (pJInfo != nullptr)
                    {
                        // if the current Java installation implements all required features: use it
                        if ((pJInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                        {
                            aCurrentInfo = pJInfo;
                            bInfoFound = true;
                        }
                        else if (static_cast<JavaInfo*>(aCurrentInfo) == nullptr)
                        {
                            // current Java installation does not provide all features
                            // but no Java installation has been detected before
                            // -> remember the current one until one is found
                            // that provides all features
                            aCurrentInfo = pJInfo;
                        }

                        delete pJInfo;
                    }
                    ++it;
                }
            }
        }


        // if no suitable Java installation has been found yet:
        // first iterate over all vendors to find a suitable Java installation,
        // then try paths that have been added manually
        if (!bInfoFound)
        {
            //Use every vendor to get Java installations. At the first usable
            //Java the loop will break
            typedef std::vector<OUString>::const_iterator ci_pl;
            for (ci_pl i = vecVendors.begin(); i != vecVendors.end(); ++i)
            {
                const OUString & vendor = *i;
                jfw::VersionInfo versionInfo =
                    aVendorSettings.getVersionInformation(vendor);

                //get all installations of one vendor according to minVersion,
                //maxVersion and excludeVersions
                sal_Int32 cInfos = 0;
                JavaInfo** arInfos = nullptr;
                javaPluginError plerr = jfw_plugin_getAllJavaInfos(
                    false,
                    vendor,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.getExcludeVersions(),
                    versionInfo.getExcludeVersionSize(),
                    & arInfos,
                    & cInfos,
                    infos);

                if (plerr != javaPluginError::NONE)
                    continue;
                //iterate over all installations to find the best which has
                //all features
                if (cInfos == 0)
                {
                    rtl_freeMemory(arInfos);
                    continue;
                }
                for (int ii = 0; ii < cInfos; ii++)
                {
                    JavaInfo* pJInfo = arInfos[ii];

                    //We remember the first installation in aCurrentInfo
                    // if no JavaInfo has been found before
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
                    delete arInfos[j];
                rtl_freeMemory(arInfos);

                if (bInfoFound)
                    break;
                //All Java installations found by the current plug-in lib
                //do not provide the required features. Try the next plug-in
            }
            if (static_cast<JavaInfo*>(aCurrentInfo) == nullptr)
            {//The plug-ins did not find a suitable Java. Now try the paths which have been
            //added manually.
                //get the list of paths to jre locations which have been added manually
                const jfw::MergedSettings settings;
                //node.loadFromSettings();
                const std::vector<OUString> & vecJRELocations =
                    settings.getJRELocations();
                //use every plug-in to determine the JavaInfo objects
                for (ci_pl i = vecVendors.begin(); i != vecVendors.end(); ++i)
                {
                    const OUString & vendor = *i;
                    jfw::VersionInfo versionInfo =
                        aVendorSettings.getVersionInformation(vendor);

                    typedef std::vector<OUString>::const_iterator citLoc;
                    for (citLoc it = vecJRELocations.begin();
                        it != vecJRELocations.end(); ++it)
                    {
                        jfw::CJavaInfo aInfo;
                        javaPluginError err = jfw_plugin_getJavaInfoByPath(
                            *it,
                            vendor,
                            versionInfo.sMinVersion,
                            versionInfo.sMaxVersion,
                            versionInfo.getExcludeVersions(),
                            versionInfo.getExcludeVersionSize(),
                            & aInfo.pInfo);
                        if (err == javaPluginError::NoJre)
                            continue;
                        if (err == javaPluginError::FailedVersion)
                            continue;
                        else if (err !=javaPluginError::NONE)
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
                    if (bInfoFound)
                        break;
                }// end iterate plug-ins
            }
        }
        if (static_cast<JavaInfo*>(aCurrentInfo))
        {
            jfw::NodeJava javaNode(jfw::NodeJava::USER);
            javaNode.setJavaInfo(aCurrentInfo,true);
            javaNode.write();
            //remember that this JRE was selected in this process
            jfw::setJavaSelected();

            if (pInfo !=nullptr)
            {
                //copy to out param
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

bool jfw_areEqualJavaInfo(JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    if (pInfoA == pInfoB)
        return true;
    if (pInfoA == nullptr || pInfoB == nullptr)
        return false;
    if (pInfoA->sVendor == pInfoB->sVendor
        && pInfoA->sLocation == pInfoB->sLocation
        && pInfoA->sVersion == pInfoB->sVersion
        && pInfoA->nFeatures == pInfoB->nFeatures
        && pInfoA->nRequirements == pInfoB->nRequirements
        && pInfoA->arVendorData == pInfoB->arVendorData)
    {
        return true;
    }
    return false;
}

javaFrameworkError jfw_getSelectedJRE(JavaInfo **ppInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (ppInfo == nullptr)
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
            *ppInfo = nullptr;
            return JFW_E_NONE;
        }
        //If the javavendors.xml has changed, then the current selected
        //Java is not valid anymore
        // /java/javaInfo/@vendorUpdate != javaSelection/updated (javavendors.xml)
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

javaFrameworkError jfw_isVMRunning(sal_Bool *bRunning)
{
    osl::MutexGuard guard(jfw::FwkMutex::get());
    if (bRunning == nullptr)
        return JFW_E_INVALID_ARG;
    if (g_pJavaVM == nullptr)
        *bRunning = false;
    else
        *bRunning = true;
    return JFW_E_NONE;
}

javaFrameworkError jfw_getJavaInfoByPath(rtl_uString *pPath, JavaInfo **ppInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (pPath == nullptr || ppInfo == nullptr)
            return JFW_E_INVALID_ARG;

        OUString ouPath(pPath);

        jfw::VendorSettings aVendorSettings;
        std::vector<OUString> vecVendors =
            aVendorSettings.getSupportedVendors();

        //Use every plug-in library to determine if the path represents a
        //JRE. If a plugin recognized it then the loop will break
        typedef std::vector<OUString>::const_iterator ci_pl;
        for (ci_pl i = vecVendors.begin(); i != vecVendors.end(); ++i)
        {
            const OUString & vendor = *i;
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(vendor);

            //ask the plugin if this is a JRE.
            //If so check if it meets the version requirements.
            //Only if it does return a JavaInfo
            JavaInfo* pInfo = nullptr;
            javaPluginError plerr = jfw_plugin_getJavaInfoByPath(
                ouPath,
                vendor,
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & pInfo);

            if (plerr == javaPluginError::NONE)
            {
                *ppInfo = pInfo;
                break;
            }
            else if(plerr == javaPluginError::FailedVersion)
            {//found JRE but it has the wrong version
                *ppInfo = nullptr;
                errcode = JFW_E_FAILED_VERSION;
                break;
            }
            else if (plerr == javaPluginError::NoJre)
            {// plugin does not recognize this path as belonging to JRE
                continue;
            }
            OSL_ASSERT(false);
        }
        if (*ppInfo == nullptr && errcode != JFW_E_FAILED_VERSION)
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


javaFrameworkError jfw_setSelectedJRE(JavaInfo const *pInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        //check if pInfo is the selected JRE
        JavaInfo *currentInfo = nullptr;
        errcode = jfw_getSelectedJRE( & currentInfo);
        if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
            return errcode;

        if (!jfw_areEqualJavaInfo(currentInfo, pInfo))
        {
            jfw::NodeJava node(jfw::NodeJava::USER);
            node.setJavaInfo(pInfo, false);
            node.write();
            //remember that the JRE was selected in this process
            jfw::setJavaSelected();
        }

        delete currentInfo;
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        fprintf(stderr, "%s\n", e.message.getStr());
        OSL_FAIL(e.message.getStr());
    }
    return errcode;
}
javaFrameworkError jfw_setEnabled(bool bEnabled)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        if (!g_bEnabledSwitchedOn && bEnabled)
        {
            //When the process started then Enabled was false.
            //This is first time enabled is set to true.
            //That means, no preparational work has been done, such as setting the
            //LD_LIBRARY_PATH, etc.

            //check if Enabled is false;
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

javaFrameworkError jfw_getEnabled(sal_Bool *pbEnabled)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (pbEnabled == nullptr)
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


javaFrameworkError jfw_setVMParameters(
    rtl_uString * * arOptions, sal_Int32 nLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (arOptions == nullptr && nLen != 0)
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

javaFrameworkError jfw_getVMParameters(
    rtl_uString *** parOptions, sal_Int32 * pLen)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        if (parOptions == nullptr || pLen == nullptr)
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

javaFrameworkError jfw_setUserClassPath(rtl_uString * pCp)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (pCp == nullptr)
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

javaFrameworkError jfw_getUserClassPath(rtl_uString ** ppCP)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        if (ppCP == nullptr)
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

javaFrameworkError jfw_addJRELocation(rtl_uString * sLocation)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        if (sLocation == nullptr)
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

javaFrameworkError jfw_existJRE(const JavaInfo *pInfo, sal_Bool *exist)
{
    //get the function jfw_plugin_existJRE
    jfw::VendorSettings aVendorSettings;
    jfw::CJavaInfo aInfo;
    aInfo = pInfo; //makes a copy of pInfo
    javaPluginError plerr = jfw_plugin_existJRE(pInfo, exist);

    javaFrameworkError ret = JFW_E_NONE;
    switch (plerr)
    {
    case javaPluginError::NONE:
        ret = JFW_E_NONE;
        break;
    case javaPluginError::InvalidArg:
        ret = JFW_E_INVALID_ARG;
        break;
    case javaPluginError::Error:
        ret = JFW_E_ERROR;
        break;
    default:
        ret = JFW_E_ERROR;
    }
    return ret;
}

void jfw_lock()
{
    jfw::FwkMutex::get().acquire();
}

void jfw_unlock()
{
    jfw::FwkMutex::get().release();
}


namespace jfw
{
CJavaInfo::CJavaInfo(): pInfo(nullptr)
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
    delete pInfo;
    pInfo = info;
}
::JavaInfo * CJavaInfo::detach()
{
    JavaInfo * tmp = pInfo;
    pInfo = nullptr;
    return tmp;
}

CJavaInfo::~CJavaInfo()
{
    delete pInfo;
}


JavaInfo * CJavaInfo::copyJavaInfo(const JavaInfo * pInfo)
{
    return pInfo == nullptr ? nullptr : new JavaInfo(*pInfo);
}


JavaInfo* CJavaInfo::cloneJavaInfo() const
{
    if (pInfo == nullptr)
        return nullptr;
    return copyJavaInfo(pInfo);
}

CJavaInfo & CJavaInfo::operator = (const CJavaInfo& info)
{
    if (&info == this)
        return *this;

    delete pInfo;
    pInfo = copyJavaInfo(info.pInfo);
    return *this;
}
CJavaInfo & CJavaInfo::operator = (const ::JavaInfo* info)
{
    if (info == pInfo)
        return *this;

    delete pInfo;
    pInfo = copyJavaInfo(info);
    return *this;
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
