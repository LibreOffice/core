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

#include <sal/config.h>

#include <cassert>
#include <memory>
#include <utility>

#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "jvmfwk/framework.hxx"
#include "vendorplugin.hxx"
#include <vector>
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

javaFrameworkError jfw_findAllJREs(std::vector<std::unique_ptr<JavaInfo>> *pparInfo)
{
    assert(pparInfo != nullptr);
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        jfw::VendorSettings aVendorSettings;
        std::vector<OUString> vecVendors =
            aVendorSettings.getSupportedVendors();
        //Add the JavaInfos found by jfw_plugin_getAllJavaInfos to the vector
        std::vector<std::unique_ptr<JavaInfo>> vecInfo;
        //get the list of paths to jre locations which have been
        //added manually
        const jfw::MergedSettings settings;
        const std::vector<OUString>& vecJRELocations =
            settings.getJRELocations();
        //Use every plug-in library to get Java installations.
        for (auto const & vendor: vecVendors)
        {
            jfw::VersionInfo versionInfo =
                aVendorSettings.getVersionInformation(vendor);

            //get all installations of one vendor according to minVersion,
            //maxVersion and excludeVersions
            std::vector<std::unique_ptr<JavaInfo>> arInfos;
            std::vector<rtl::Reference<jfw_plugin::VendorBase>> infos;
            javaPluginError plerr = jfw_plugin_getAllJavaInfos(
                true,
                vendor,
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.vecExcludeVersions,
                & arInfos,
                infos);

            if (plerr != javaPluginError::NONE)
                return JFW_E_ERROR;

            for (auto & j: arInfos)
                vecInfo.push_back(std::move(j));

            //Check if the current plugin can detect JREs at the location
            // of the paths added by jfw_addJRELocation
            //Check every manually added location
            for (auto const & ii: vecJRELocations)
            {
                std::unique_ptr<JavaInfo> aInfo;
                plerr = jfw_plugin_getJavaInfoByPath(
                    ii,
                    vendor,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.vecExcludeVersions,
                    &aInfo);
                if (plerr == javaPluginError::NoJre)
                    continue;
                if (plerr == javaPluginError::FailedVersion)
                    continue;
                else if (plerr != javaPluginError::NONE)
                    return JFW_E_ERROR;

                // Was this JRE already added?  Different plugins could detect
                // the same JRE.  Also make sure vecInfo contains only JavaInfos
                // for the vendors for which there is a javaSelection/plugins/
                // library entry in the javavendors.xml; jfw_getJavaInfoByPath
                // can return a JavaInfo of any vendor:
                if ((std::find_if(
                         vecInfo.begin(), vecInfo.end(),
                         [&aInfo](std::unique_ptr<JavaInfo> const & info) {
                             return areEqualJavaInfo(
                                 info.get(), aInfo.get());
                         })
                     == vecInfo.end())
                    && (std::find(
                            vecVendors.begin(), vecVendors.end(),
                            aInfo->sVendor)
                        != vecVendors.end()))
                {
                    vecInfo.push_back(std::move(aInfo));
                }
            }
        }

        *pparInfo = std::move(vecInfo);

        return JFW_E_NONE;
    }
    catch (const jfw::FrameworkException& e)
    {
        SAL_WARN( "jfw", e.message);
        return e.errorCode;
    }
}

javaFrameworkError jfw_startVM(
    JavaInfo const * pInfo, std::vector<OUString> const & arOptions,
    JavaVM ** ppVM, JNIEnv ** ppEnv)
{
    assert(ppVM != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;

    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        //We keep this pointer so we can determine if a VM has already
        //been created.
        if (g_pJavaVM != nullptr)
            return JFW_E_RUNNING_JVM;

        std::vector<OString> vmParams;
        OString sUserClassPath;
        std::unique_ptr<JavaInfo> aInfo;
        if (pInfo == nullptr)
        {
            jfw::JFW_MODE mode = jfw::getMode();
            if (mode == jfw::JFW_MODE_APPLICATION)
            {
                const jfw::MergedSettings settings;
                if (!settings.getEnabled())
                    return JFW_E_JAVA_DISABLED;
                aInfo = settings.createJavaInfo();
                //check if a Java has ever been selected
                if (!aInfo)
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
                    if (aInfo &&
                        (aInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE) == 0)
                    {
                        //has the user manually selected a JRE?
                        if (settings.getJavaInfoAttrAutoSelect())
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
                //must not have the requirements flag JFW_REQUIRE_NEEDRESTART
                if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
                    && jfw::wasJavaSelectedInSameProcess())
                    return JFW_E_NEED_RESTART;

                vmParams = settings.getVmParametersUtf8();
                sUserClassPath = jfw::makeClassPathOption(settings.getUserClassPath());
            } // end mode FWK_MODE_OFFICE
            else if (mode == jfw::JFW_MODE_DIRECT)
            {
                errcode = jfw_getSelectedJRE(&aInfo);
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
            pInfo = aInfo.get();
        }
        assert(pInfo != nullptr);

        // create JavaVMOptions array that is passed to the plugin
        // it contains the classpath and all options set in the
        //options dialog
        std::unique_ptr<JavaVMOption[]> sarJOptions(
            new JavaVMOption[arOptions.size() + 2 + vmParams.size()]);
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
        std::vector<OString> convertedOptions;
        for (auto const & ii: arOptions)
        {
            OString conv = OUStringToOString(ii, osl_getThreadTextEncoding());
            convertedOptions.push_back(conv);
            arOpt[index].optionString = const_cast<char *>(conv.getStr());
            arOpt[index].extraInfo = nullptr;
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
        SAL_WARN( "jfw", e.message);
    }

    return errcode;
}

/** We do not use here jfw_findAllJREs and then check if a JavaInfo
    meets the requirements, because that means using all plug-ins, which
    may take quite a while. The implementation first inspects JAVA_HOME and
    PATH environment variables. If no suitable JavaInfo is found there, it
    inspects all JavaInfos found by the jfw_plugin_get* functions.
 */
javaFrameworkError jfw_findAndSelectJRE(std::unique_ptr<JavaInfo> *pInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        sal_uInt64 nFeatureFlags = 0;
        std::unique_ptr<JavaInfo> aCurrentInfo;
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
        if (jfw_plugin_getJavaInfoFromJavaHome(
                versionInfos, &aCurrentInfo, infos)
            == javaPluginError::NONE)
        {
            // compare features
            // if the user does not require any features (nFeatureFlags = 0)
            // or the Java installation provides all features, then this installation is used
            if ((aCurrentInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
            {
                bInfoFound = true;
            }
        }

        // if no Java installation providing all features was detected by using JAVA_HOME,
        // query PATH for Java installations
        if (!bInfoFound)
        {
            std::vector<std::unique_ptr<JavaInfo>> vecJavaInfosFromPath;
            if (jfw_plugin_getJavaInfosFromPath(
                    versionInfos, vecJavaInfosFromPath, infos)
                == javaPluginError::NONE)
            {
                for (auto & pJInfo: vecJavaInfosFromPath)
                {
                    // if the current Java installation implements all required features: use it
                    if ((pJInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                    {
                        aCurrentInfo = std::move(pJInfo);
                        bInfoFound = true;
                        break;
                    }
                    else if (!aCurrentInfo)
                    {
                        // current Java installation does not provide all features
                        // but no Java installation has been detected before
                        // -> remember the current one until one is found
                        // that provides all features
                        aCurrentInfo = std::move(pJInfo);
                    }
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
                std::vector<std::unique_ptr<JavaInfo>> arInfos;
                javaPluginError plerr = jfw_plugin_getAllJavaInfos(
                    false,
                    vendor,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.vecExcludeVersions,
                    & arInfos,
                    infos);

                if (plerr != javaPluginError::NONE)
                    continue;
                //iterate over all installations to find the best which has
                //all features
                for (auto & pJInfo: arInfos)
                {
                    // compare features
                    // If the user does not require any features (nFeatureFlags = 0)
                    // then the first installation is used
                    if ((pJInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                    {
                        //the just found Java implements all required features
                        //currently there is only accessibility!!!
                        aCurrentInfo = std::move(pJInfo);
                        bInfoFound = true;
                        break;
                    }
                    else if (!aCurrentInfo)
                    {
                        // We remember the first installation in aCurrentInfo if
                        // no JavaInfo has been found before:
                        aCurrentInfo = std::move(pJInfo);
                    }
                }

                if (bInfoFound)
                    break;
                //All Java installations found by the current plug-in lib
                //do not provide the required features. Try the next plug-in
            }
            if (!aCurrentInfo)
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
                        std::unique_ptr<JavaInfo> aInfo;
                        javaPluginError err = jfw_plugin_getJavaInfoByPath(
                            *it,
                            vendor,
                            versionInfo.sMinVersion,
                            versionInfo.sMaxVersion,
                            versionInfo.vecExcludeVersions,
                            &aInfo);
                        if (err == javaPluginError::NoJre)
                            continue;
                        if (err == javaPluginError::FailedVersion)
                            continue;
                        else if (err !=javaPluginError::NONE)
                            return JFW_E_ERROR;

                        if (aInfo)
                        {
                            // compare features
                            // If the user does not require any features (nFeatureFlags = 0)
                            // then the first installation is used
                            if ((aInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                            {
                                //the just found Java implements all required features
                                //currently there is only accessibility!!!
                                aCurrentInfo = std::move(aInfo);
                                bInfoFound = true;
                                break;
                            }
                            else if (!aCurrentInfo)
                            {
                                // We remember the very first installation in
                                // aCurrentInfo:
                                aCurrentInfo = std::move(aInfo);
                            }
                        }
                    }//end iterate over paths
                    if (bInfoFound)
                        break;
                }// end iterate plug-ins
            }
        }
        if (aCurrentInfo)
        {
            jfw::NodeJava javaNode(jfw::NodeJava::USER);
            javaNode.setJavaInfo(aCurrentInfo.get(),true);
            javaNode.write();
            //remember that this JRE was selected in this process
            jfw::setJavaSelected();

            if (pInfo !=nullptr)
            {
                *pInfo = std::move(aCurrentInfo);
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
        SAL_WARN( "jfw", e.message );
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

javaFrameworkError jfw_getSelectedJRE(std::unique_ptr<JavaInfo> *ppInfo)
{
    assert(ppInfo != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
        {
            if ((errcode = jfw_getJavaInfoByPath(
                     jfw::BootParams::getJREHome(), ppInfo))
                != JFW_E_NONE)
                throw jfw::FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] The JRE specified by the bootstrap "
                    "variable UNO_JAVA_JFW_JREHOME  or  UNO_JAVA_JFW_ENV_JREHOME "
                    " could not be recognized. Check the values and make sure that you "
                    "use a plug-in library that can recognize that JRE.");

            return JFW_E_NONE;
        }

        const jfw::MergedSettings settings;
        *ppInfo = settings.createJavaInfo();
        if (!*ppInfo)
        {
            return JFW_E_NONE;
        }
        //If the javavendors.xml has changed, then the current selected
        //Java is not valid anymore
        // /java/javaInfo/@vendorUpdate != javaSelection/updated (javavendors.xml)
        OString sUpdated = jfw::getElementUpdated();

        if (sUpdated != settings.getJavaInfoAttrVendorUpdate())
        {
            ppInfo->reset();
            return JFW_E_INVALID_SETTINGS;
        }
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}

bool jfw_isVMRunning()
{
    osl::MutexGuard guard(jfw::FwkMutex::get());
    return g_pJavaVM != nullptr;
}

javaFrameworkError jfw_getJavaInfoByPath(OUString const & pPath, std::unique_ptr<JavaInfo> *ppInfo)
{
    assert(ppInfo != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());

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
            javaPluginError plerr = jfw_plugin_getJavaInfoByPath(
                pPath,
                vendor,
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.vecExcludeVersions,
                ppInfo);

            if (plerr == javaPluginError::NONE)
            {
                break;
            }
            else if(plerr == javaPluginError::FailedVersion)
            {//found JRE but it has the wrong version
                ppInfo->reset();
                errcode = JFW_E_FAILED_VERSION;
                break;
            }
            else if (plerr == javaPluginError::NoJre)
            {// plugin does not recognize this path as belonging to JRE
                continue;
            }
            OSL_ASSERT(false);
        }
        if (!*ppInfo && errcode != JFW_E_FAILED_VERSION)
            errcode = JFW_E_NOT_RECOGNIZED;
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
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
        std::unique_ptr<JavaInfo> currentInfo;
        errcode = jfw_getSelectedJRE( & currentInfo);
        if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
            return errcode;

        if (!jfw_areEqualJavaInfo(currentInfo.get(), pInfo))
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
        SAL_WARN( "jfw", e.message );
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
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}

javaFrameworkError jfw_getEnabled(bool *pbEnabled)
{
    assert(pbEnabled != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        osl::MutexGuard guard(jfw::FwkMutex::get());
        jfw::MergedSettings settings;
        *pbEnabled = settings.getEnabled();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}


javaFrameworkError jfw_setVMParameters(std::vector<OUString> const & arOptions)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        node.setVmParameters(arOptions);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }

    return errcode;
}

javaFrameworkError jfw_getVMParameters(std::vector<OUString> * parOptions)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;

        const jfw::MergedSettings settings;
        settings.getVmParametersArray(parOptions);
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}

javaFrameworkError jfw_setUserClassPath(OUString const & pCp)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        node.setUserClassPath(pCp);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}

javaFrameworkError jfw_getUserClassPath(OUString * ppCP)
{
    assert(ppCP != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        const jfw::MergedSettings settings;
        *ppCP = settings.getUserClassPath();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }
    return errcode;
}

javaFrameworkError jfw_addJRELocation(OUString const & sLocation)
{
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex::get());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        jfw::NodeJava node(jfw::NodeJava::USER);
        node.load();
        node.addJRELocation(sLocation);
        node.write();
    }
    catch (const jfw::FrameworkException& e)
    {
        errcode = e.errorCode;
        SAL_WARN( "jfw", e.message );
    }

    return errcode;

}

javaFrameworkError jfw_existJRE(const JavaInfo *pInfo, bool *exist)
{
    javaPluginError plerr = jfw_plugin_existJRE(pInfo, exist);

    javaFrameworkError ret = JFW_E_NONE;
    switch (plerr)
    {
    case javaPluginError::NONE:
        ret = JFW_E_NONE;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
