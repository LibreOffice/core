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
#include <sal/log.hxx>

#include <cassert>
#include <memory>

#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#ifdef _WIN32
#include <osl/process.h>
#endif
#include <osl/thread.hxx>
#include <o3tl/string_view.hxx>
#include <jvmfwk/framework.hxx>
#include <vendorbase.hxx>
#include <vendorplugin.hxx>
#include <vector>
#include <algorithm>
#include "framework.hxx"
#include <fwkutil.hxx>
#include <elements.hxx>
#include <fwkbase.hxx>

namespace {

bool g_bEnabledSwitchedOn = false;

JavaVM * g_pJavaVM = nullptr;

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
        osl::MutexGuard guard(jfw::FwkMutex());

        jfw::VendorSettings aVendorSettings;
        std::vector<std::unique_ptr<JavaInfo>> vecInfo;

        //Use all plug-in libraries to get Java installations.
        std::vector<std::unique_ptr<JavaInfo>> arInfos;
        std::vector<rtl::Reference<jfw_plugin::VendorBase>> infos;
        javaPluginError plerr = jfw_plugin_getAllJavaInfos(
            true,
            aVendorSettings,
            & arInfos,
            infos);

        if (plerr != javaPluginError::NONE)
            return JFW_E_ERROR;

        for (auto & j: arInfos)
            vecInfo.push_back(std::move(j));

        // direct mode disregards Java settings, so only retrieve
        // JREs from settings when application mode is used
        if (jfw::getMode() == jfw::JFW_MODE_APPLICATION)
        {
            //get the list of paths to jre locations which have been
            //added manually
            const jfw::MergedSettings settings;
            const std::vector<OUString> vecJRELocations =
                settings.getJRELocations();
            //Check if any plugin can detect JREs at the location
            // of the paths added by jfw_addJRELocation
            //Check every manually added location
            for (auto const & ii: vecJRELocations)
            {
                std::unique_ptr<JavaInfo> aInfo;
                plerr = jfw_plugin_getJavaInfoByPath(
                    ii,
                    aVendorSettings,
                    &aInfo);
                if (plerr == javaPluginError::NoJre)
                    continue;
                if (plerr == javaPluginError::FailedVersion)
                    continue;
                if (plerr == javaPluginError::WrongArch)
                    continue;
                else if (plerr != javaPluginError::NONE)
                    return JFW_E_ERROR;

                // Was this JRE already added?
                if (std::none_of(
                        vecInfo.begin(), vecInfo.end(),
                        [&aInfo](std::unique_ptr<JavaInfo> const & info) {
                            return areEqualJavaInfo(
                                info.get(), aInfo.get());
                        }))
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

std::vector<OUString> jfw_convertUserPathList(std::u16string_view sUserPath)
{
    std::vector<OUString> result;
    sal_Int32 nIdx = 0;
    do
    {
        size_t nextColon = sUserPath.find(SAL_PATHSEPARATOR, nIdx);
        std::u16string_view sToken;
        if (nextColon != 0 && nextColon != std::u16string_view::npos)
            sToken = sUserPath.substr(nIdx, nextColon - nIdx);
        else
            sToken = sUserPath.substr(nIdx, sUserPath.size() - nIdx);

        // Check if we are in bootstrap variable mode (class path starts with '$').
        // Then the class path must be in URL format.
        if (o3tl::starts_with(sToken, u"$"))
        {
            // Detect open bootstrap variables - they might contain colons - we need to skip those.
            size_t nBootstrapVarStart = sToken.find(u"${");
            if (nBootstrapVarStart != std::u16string_view::npos)
            {
                size_t nBootstrapVarEnd = sToken.find(u"}", nBootstrapVarStart);
                if (nBootstrapVarEnd == std::u16string_view::npos)
                {
                    // Current colon is part of bootstrap variable - skip it!
                    nextColon = sUserPath.find(SAL_PATHSEPARATOR, nextColon + 1);
                    if (nextColon != 0 && nextColon != std::u16string_view::npos)
                        sToken = sUserPath.substr(nIdx, nextColon - nIdx);
                    else
                        sToken = sUserPath.substr(nIdx, sUserPath.size() - nIdx);
                }
            }
        }
        result.emplace_back(sToken);
        nIdx = nextColon + 1;
    } while (nIdx > 0);
    return result;
}

javaFrameworkError jfw_startVM(
    JavaInfo const * pInfo, std::vector<OUString> const & arOptions,
    JavaVM ** ppVM, JNIEnv ** ppEnv)
{
    assert(ppVM != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;

    try
    {
        osl::MutexGuard guard(jfw::FwkMutex());

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
                // Expand user classpath (might contain bootstrap vars)
                OUString sUserPath(settings.getUserClassPath());
                std::vector paths = jfw_convertUserPathList(sUserPath);
                OUString sUserPathExpanded;
                for (auto& path : paths)
                {
                    if (!sUserPathExpanded.isEmpty())
                        sUserPathExpanded += OUStringChar(SAL_PATHSEPARATOR);
                    if (path.startsWith("$"))
                    {
                        OUString sURL = path;
                        rtl::Bootstrap::expandMacros(sURL);
                        osl::FileBase::getSystemPathFromFileURL(sURL, path);
                    }
                    sUserPathExpanded += path;
                }
                sUserClassPath = jfw::makeClassPathOption(sUserPathExpanded);
            } // end mode FWK_MODE_OFFICE
            else if (mode == jfw::JFW_MODE_DIRECT)
            {
                errcode = jfw_getSelectedJRE(&aInfo);
                if (errcode != JFW_E_NONE)
                    return errcode;
                //In direct mode the options are specified by bootstrap variables
                //of the form UNO_JAVA_JFW_PARAMETER_1 .. UNO_JAVA_JFW_PARAMETER_n
                vmParams = jfw::BootParams::getVMParameters();
                auto const cp = jfw::BootParams::getClasspath();
                if (!cp.isEmpty())
                {
                    sUserClassPath =
                        "-Djava.class.path=" + cp;
                }
            }
            else
                OSL_ASSERT(false);
            pInfo = aInfo.get();
        }
        assert(pInfo != nullptr);

#ifdef _WIN32
        // Alternative JREs (AdoptOpenJDK, Azul Zulu) are missing the bin/ folder in
        // java.library.path. Somehow setting java.library.path accordingly doesn't work,
        // but the PATH gets picked up, so add it there.
        // Without this hack, some features don't work in alternative JREs.
        OUString sPATH;
        osl_getEnvironment(OUString("PATH").pData, &sPATH.pData);
        OUString sJRELocation;
        osl::FileBase::getSystemPathFromFileURL(pInfo->sLocation + "/bin", sJRELocation);
        if (sPATH.isEmpty())
            sPATH = sJRELocation;
        else
            sPATH = sJRELocation + OUStringChar(SAL_PATHSEPARATOR) + sPATH;
        osl_setEnvironment(OUString("PATH").pData, sPATH.pData);
#endif // _WIN32

        // create JavaVMOptions array that is passed to the plugin
        // it contains the classpath and all options set in the
        //options dialog
        std::unique_ptr<JavaVMOption[]> sarJOptions(
            new JavaVMOption[
                arOptions.size() + (sUserClassPath.isEmpty() ? 2 : 3) + vmParams.size()]);
        JavaVMOption * arOpt = sarJOptions.get();
        if (! arOpt)
            return JFW_E_ERROR;

        //The first argument is the classpath
        int index = 0;
        if (!sUserClassPath.isEmpty()) {
            arOpt[index].optionString= const_cast<char*>(sUserClassPath.getStr());
            arOpt[index].extraInfo = nullptr;
            ++index;
        }
        // Set a flag that this JVM has been created via the JNI Invocation API
        // (used, for example, by UNO remote bridges to share a common thread pool
        // factory among Java and native bridge implementations):
        arOpt[index].optionString = const_cast<char *>("-Dorg.openoffice.native=");
        arOpt[index].extraInfo = nullptr;
        ++index;

        // Don't intercept SIGTERM
        arOpt[index].optionString = const_cast<char *>("-Xrs");
        arOpt[index].extraInfo = nullptr;
        ++index;

        //add the options set by options dialog
        for (auto const & vmParam : vmParams)
        {
            arOpt[index].optionString = const_cast<char*>(vmParam.getStr());
            arOpt[index].extraInfo = nullptr;
            index ++;
        }
        //add all options of the arOptions argument
        std::vector<OString> convertedOptions;
        for (auto const & ii: arOptions)
        {
            OString conv = OUStringToOString(ii, osl_getThreadTextEncoding());
            convertedOptions.push_back(conv);
                // keep conv.getStr() alive until after the call to
                // jfw_plugin_startJavaVirtualMachine below
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
        osl::MutexGuard guard(jfw::FwkMutex());
        if (jfw::getMode() == jfw::JFW_MODE_DIRECT)
            return JFW_E_DIRECT_MODE;
        std::unique_ptr<JavaInfo> aCurrentInfo;


        // 'bInfoFound' indicates whether a Java installation has been found
        bool bInfoFound = false;

        // get list of vendors for Java installations
        jfw::VendorSettings aVendorSettings;

        std::vector<rtl::Reference<jfw_plugin::VendorBase>> infos;

        // first inspect Java installation that the JAVA_HOME
        // environment variable points to (if it is set)
        if (jfw_plugin_getJavaInfoFromJavaHome(
                aVendorSettings, &aCurrentInfo, infos)
            == javaPluginError::NONE)
        {
            bInfoFound = true;
        }

        // if no Java installation was detected by using JAVA_HOME,
        // query PATH for Java installations
        if (!bInfoFound)
        {
            std::vector<std::unique_ptr<JavaInfo>> vecJavaInfosFromPath;
            if (jfw_plugin_getJavaInfosFromPath(
                    aVendorSettings, vecJavaInfosFromPath, infos)
                == javaPluginError::NONE)
            {
                assert(!vecJavaInfosFromPath.empty());
                aCurrentInfo = std::move(vecJavaInfosFromPath[0]);
                bInfoFound = true;
            }
        }


        // if no suitable Java installation has been found yet:
        // first use jfw_plugin_getAllJavaInfos to find a suitable Java installation,
        // then try paths that have been added manually
        if (!bInfoFound)
        {
            //get all installations
            std::vector<std::unique_ptr<JavaInfo>> arInfos;
            javaPluginError plerr = jfw_plugin_getAllJavaInfos(
                false,
                aVendorSettings,
                & arInfos,
                infos);

            if (plerr == javaPluginError::NONE && !arInfos.empty())
            {
                aCurrentInfo = std::move(arInfos[0]);
            }

            if (!aCurrentInfo)
            {//The plug-ins did not find a suitable Java. Now try the paths which have been
            //added manually.
                //get the list of paths to jre locations which have been added manually
                const jfw::MergedSettings settings;
                //node.loadFromSettings();
                const std::vector<OUString> & vecJRELocations =
                    settings.getJRELocations();
                //use all plug-ins to determine the JavaInfo objects
                for (auto const & JRELocation : vecJRELocations)
                {
                    std::unique_ptr<JavaInfo> aInfo;
                    javaPluginError err = jfw_plugin_getJavaInfoByPath(
                        JRELocation,
                        aVendorSettings,
                        &aInfo);
                    if (err == javaPluginError::NoJre)
                        continue;
                    if (err == javaPluginError::FailedVersion)
                        continue;
                    else if (err !=javaPluginError::NONE)
                        return JFW_E_ERROR;

                    if (aInfo)
                    {
                        aCurrentInfo = std::move(aInfo);
                        break;
                    }
                }//end iterate over paths
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
        osl::MutexGuard guard(jfw::FwkMutex());

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
                    "use a plug-in library that can recognize that JRE."_ostr);

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
    osl::MutexGuard guard(jfw::FwkMutex());
    return g_pJavaVM != nullptr;
}

javaFrameworkError jfw_getJavaInfoByPath(OUString const & pPath, std::unique_ptr<JavaInfo> *ppInfo)
{
    assert(ppInfo != nullptr);
    javaFrameworkError errcode = JFW_E_NONE;
    try
    {
        osl::MutexGuard guard(jfw::FwkMutex());

        jfw::VendorSettings aVendorSettings;

        //ask all plugins if this is a JRE.
        //If so check if it meets the version requirements.
        //Only if it does return a JavaInfo
        javaPluginError plerr = jfw_plugin_getJavaInfoByPath(
            pPath,
            aVendorSettings,
            ppInfo);

        if(plerr == javaPluginError::FailedVersion)
        {//found JRE but it has the wrong version
            ppInfo->reset();
            errcode = JFW_E_FAILED_VERSION;
        }
        OSL_ASSERT(plerr == javaPluginError::NONE || plerr == javaPluginError::NoJre);
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
        osl::MutexGuard guard(jfw::FwkMutex());
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
    jfw::FwkMutex().acquire();
}

void jfw_unlock()
{
    jfw::FwkMutex().release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
