/*************************************************************************
 *
 *  $RCSfile: framework.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:54:39 $
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
#include "external/boost/scoped_array.hpp"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/thread.hxx"
#include "osl/module.hxx"
#include "osl/process.h"
#include "sal/config.h"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"
#include "osl/mutex.hxx"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include <vector>
#include <algorithm>
#include <functional>
#include "jni.h"

#include "framework.hxx"
#include "libxmlutil.hxx"
#include "fwkutil.hxx"
#include "elements.hxx"
//#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#ifdef WNT
/** The existence of the file useatjava.txt decides if a Java should be used
    that supports accessibility tools.
 */
#define USE_ACCESSIBILITY_FILE "useatjava.txt"
#endif

namespace {
JavaVM * g_pJavaVM = NULL;

bool g_bEnabledSwitchedOn = false;

sal_Bool areEqualJavaInfo(
    JavaInfo const * pInfoA,JavaInfo const * pInfoB)
{
    return jfw_areEqualJavaInfo(pInfoA, pInfoB);
}

struct EqualInfo
{
    const JavaInfo *pInfo;
    EqualInfo(const JavaInfo* info):pInfo(info){}

    bool operator () (const JavaInfo*  pInfo2)
    {
        return areEqualJavaInfo(pInfo, pInfo2) == sal_True ? true : false;
    }
};

void freeJavaInfo( JavaInfo * pInfo)
{
    jfw_freeJavaInfo(pInfo);
}

}

javaFrameworkError SAL_CALL jfw_findAllJREs(JavaInfo ***pparInfo, sal_Int32 *pSize)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (pparInfo == NULL || pSize == NULL)
        return JFW_E_INVALID_ARG;

    //Prepare the xml document and context
    rtl::OString sSettingsPath = jfw::getVendorSettingsPath();
     jfw::CXmlDocPtr doc(xmlParseFile(sSettingsPath.getStr()));
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context(xmlXPathNewContext(doc));
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;

    //Get a list of plugins which provide Java information
    std::vector<jfw::PluginLibrary> vecPlugins;
    errcode = jfw::getVendorPluginURLs(doc, context, & vecPlugins);
    if (errcode != JFW_E_NONE)
        return errcode;

    //Add the JavaInfos found by jfw_plugin_getAllJavaInfos to the vector
    //Make sure that the contents are destroyed if this
    //function returns with an error
    std::vector<JavaInfo*> vecInfo;
    //Add the JavaInfos found by jfw_plugin_getJavaInfoByPath to this vector
    //Make sure that the contents are destroyed if this
    //function returns with an error
    std::vector<JavaInfo*> vecInfoManual;
    typedef std::vector<JavaInfo*>::iterator it_info;
    //get the list of paths to jre locations which have been
    //added manually
    jfw::CNodeJava node;
    errcode = node.loadFromSettings();
    if (errcode != JFW_E_NONE)
        return errcode;

    const std::vector<rtl::OString>& vecJRELocations =
        node.getJRELocations();
    //Use every plug-in library to get Java installations.
    typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
    for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); i++)
    {
        const jfw::PluginLibrary & library = *i;
        jfw::VersionInfo versionInfo;
        errcode =  jfw::getVersionInformation(doc, context, library.sVendor,
                                         & versionInfo);
        if (errcode != JFW_E_NONE)
        {   //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          freeJavaInfo);
            return JFW_E_CONFIG_READWRITE;
        }
        osl::Module pluginLib(library.sPath);
        if (pluginLib.is() == sal_False)
        {
            //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          freeJavaInfo);

            rtl::OString msg = rtl::OUStringToOString(
                library.sPath, osl_getThreadTextEncoding());
            fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                    "Modify the javavendors.xml accordingly!\n", msg.getStr());
            return JFW_E_NO_PLUGIN;
        }
        jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
            (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getAllJavaInfos")));

        OSL_ASSERT(getAllJavaFunc);
        if (getAllJavaFunc == NULL)
        {
            //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          freeJavaInfo);
            return JFW_E_ERROR;
        }
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
        {   //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          freeJavaInfo);
            return JFW_E_ERROR;
        }
        for (int i = 0; i < cInfos; i++)
            vecInfo.push_back(arInfos[i]);
        rtl_freeMemory(arInfos);

        //Check if the current plugin can detect JREs at the location
        // of the paths added by jfw_setJRELocations or jfw_addJRELocation
        //get the function from the plugin
        jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
            (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getJavaInfoByPath")));

        OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
        if (jfw_plugin_getJavaInfoByPathFunc == NULL)
        {   //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          freeJavaInfo);
            return JFW_E_ERROR;
        }
        typedef std::vector<rtl::OString>::const_iterator citLoc;
        //Check every manually added location
        for (citLoc ii = vecJRELocations.begin();
             ii != vecJRELocations.end(); ii++)
        {
            rtl::OUString sLocation =
                rtl::OStringToOUString(*ii, RTL_TEXTENCODING_UTF8);
            JavaInfo* pInfo = NULL;
            plerr = (*jfw_plugin_getJavaInfoByPathFunc)(
                sLocation.pData,
                versionInfo.sMinVersion.pData,
                versionInfo.sMaxVersion.pData,
                versionInfo.getExcludeVersions(),
                versionInfo.getExcludeVersionSize(),
                & pInfo);
            if (plerr == JFW_PLUGIN_E_NO_JRE)
                continue;
            if (plerr == JFW_PLUGIN_E_FAILED_VERSION)
                continue;
            else if (plerr !=JFW_PLUGIN_E_NONE)
            {   //delete JavaInfo objects
                std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
                std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                              freeJavaInfo);
                return JFW_E_ERROR;
            }
            if (pInfo)
            {
                //Was this JRE already added?. Different plugins could detect
                //the same JRE
                it_info it_duplicate =
                    std::find_if(vecInfoManual.begin(), vecInfoManual.end(),
                              EqualInfo(pInfo));
                if (it_duplicate == vecInfoManual.end())
                    vecInfoManual.push_back(pInfo);
                else
                    jfw_freeJavaInfo(pInfo);
            }
        }
    }
    //Make sure vecInfoManual contains only JavaInfos for the vendors for which
    //there is a javaSelection/plugins/library entry in the javavendors.xml
    //To obtain the JavaInfos for the manually added JRE locations the function
    //jfw_getJavaInfoByPath is called which can return a JavaInfo of any vendor.
    std::vector<JavaInfo*> vecInfoManual2;
    for (it_info ivm = vecInfoManual.begin(); ivm != vecInfoManual.end(); ivm++)
    {
        bool bAllowedVendor = false;
        for (ci_pl ii = vecPlugins.begin(); ii != vecPlugins.end(); ii++)
        {
            if ( ii->sVendor.equals((*ivm)->sVendor))
            {
                vecInfoManual2.push_back(*ivm);
                bAllowedVendor = true;
                break;
            }
        }
        if (bAllowedVendor == false)
            jfw_freeJavaInfo(*ivm);
    }
    //Check which JavaInfo from vector vecInfoManual2 is already
    //contained in vecInfo. If it already exists then remove it from
    //vecInfoManual2
    for (it_info j = vecInfo.begin(); j != vecInfo.end(); j++)
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
    {   //delete JavaInfo objects
        std::for_each(vecInfo.begin(), vecInfo.end(), freeJavaInfo);
        std::for_each(vecInfoManual2.begin(), vecInfoManual2.end(), freeJavaInfo);
        return JFW_E_ERROR;
    }
    typedef std::vector<JavaInfo*>::iterator it;
    int index = 0;
    //Add the automatically detected JREs
    for (it k = vecInfo.begin(); k != vecInfo.end(); k++)
        (*pparInfo)[index++] = *k;
    //Add the manually detected JREs
    for (it l = vecInfoManual2.begin(); l != vecInfoManual2.end(); l++)
        (*pparInfo)[index++] = *l;

    *pSize = nSize;
    return errcode;
}

javaFrameworkError SAL_CALL jfw_startVM(JavaVMOption *arOptions, sal_Int32 cOptions,
                                 JavaVM **ppVM, JNIEnv **ppEnv)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    //We keep this pointer so we can determine if a VM has already
    //been created.
    if (g_pJavaVM != NULL)
        return JFW_E_RUNNING_JVM;
    if (ppVM == NULL)
        return JFW_E_INVALID_ARG;

    jfw::CNodeJava javaSettings;
    jfw::CJavaInfo aInfo;
    jfw::JFW_MODE mode = jfw::getMode();

    if (mode == jfw::JFW_MODE_OFFICE)
    {
#ifdef WNT
        //Because on Windows there is no system setting that we can use to determine
        //if Assistive Technology Tool support is needed, we ship a .reg file that the
        //user can use to create a registry setting. When the user forgets to set
        //the key before he starts the office then a JRE may be selected without access bridge.
        //When he later sets the key then we select a JRE with accessibility support but
        //only if the user has not manually changed the selected JRE in the options dialog.
        if (jfw::isAccessibilitySupportDesired())
        {
            jfw::CJavaInfo info = NULL;
            javaFrameworkError err = JFW_E_NONE;
            if ((err = jfw_getSelectedJRE( & info)) != JFW_E_NONE)
                return err;
            // If no JRE has been selected then we do no select one. This function shall then
            //return JFW_E_NO_SELECT
            if (info != NULL &&
                (info->nFeatures & JFW_FEATURE_ACCESSBRIDGE) == 0)
            {
                //has the user manually selected a JRE?
                jfw::CNodeJava settings;
                if ((errcode = settings.loadFromSettings()) != JFW_E_NONE)
                    return errcode;
                if (settings.getJavaInfoAttrAutoSelect() == true)
                {
                    //The currently selected JRE has no access bridge
                    if ((err = jfw_findAndSelectJRE(NULL)) != JFW_E_NONE)
                        return err;
                }
            }
        }
#endif
        if ((errcode = javaSettings.loadFromSettings()) != JFW_E_NONE)
            return errcode;

        //get the current java setting (javaInfo)
        aInfo = javaSettings.getJavaInfo();
        //check if a Java has ever been selected
        if (aInfo == NULL)
            return JFW_E_NO_SELECT;
        //check if the javavendors.xml has changed after a Java was selected
        rtl::OString sVendorUpdate;
        if ((errcode = jfw::getElementUpdated(sVendorUpdate))
            != JFW_E_NONE)
            return errcode;
        if (sVendorUpdate != javaSettings.getJavaInfoAttrVendorUpdate())
            return JFW_E_INVALID_SETTINGS;

        //check if JAVA is disabled
        //If Java is enabled, but it was disabled when this process was started
        // then no preparational work, such as setting the LD_LIBRARY_PATH, was
        //done. Therefore if a JRE needs it it must not be started.
        if (javaSettings.getEnabled() == sal_False)
            return JFW_E_JAVA_DISABLED;
        else if (g_bEnabledSwitchedOn &&
                 (aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART))
            return JFW_E_NEED_RESTART;

        //Check if the selected Java was set in this process. If so it
        //must not have the requirments flag JFW_REQUIRE_NEEDRESTART
        if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
            &&
            (jfw::wasJavaSelectedInSameProcess() == true))
            return JFW_E_NEED_RESTART;
    } // end mode FWK_MODE_OFFICE
    else if (mode == jfw::JFW_MODE_ENV_SIMPLE)
    {
        rtl::OUString sOO_USE_JRE(RTL_CONSTASCII_USTRINGPARAM(ENVIRONMENT_VAR_JRE_PATH));

        //get JAVA_HOME
        rtl_uString * psJAVA_HOME = 0;
        if (osl_getEnvironment(sOO_USE_JRE.pData, & psJAVA_HOME) != osl_Process_E_None)
            return JFW_E_ERROR;
        rtl::OUString sJAVA_HOME(psJAVA_HOME, SAL_NO_ACQUIRE);
        //convert to file URL
        rtl_uString * pJavaUrl = 0;
        if (osl_getFileURLFromSystemPath(sJAVA_HOME.pData, & pJavaUrl) != osl_File_E_None)
            return JFW_E_ERROR;
        rtl::OUString sJavaUrl(pJavaUrl, SAL_NO_ACQUIRE);
        if ((errcode = jfw_getJavaInfoByPath(sJavaUrl.pData, & aInfo))
            != JFW_E_NONE)
            return errcode;
    }
    //get the function jfw_plugin_startJavaVirtualMachine
    rtl::OUString sLibPath;
    if ((errcode = jfw::getPluginLibrary(aInfo.getVendor(), sLibPath)) != JFW_E_NONE)
        return errcode;

    osl::Module modulePlugin(sLibPath);
    if ( ! modulePlugin)
        return JFW_E_NO_PLUGIN;

    rtl::OUString sFunctionName(
        RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_startJavaVirtualMachine"));
    jfw_plugin_startJavaVirtualMachine_ptr pFunc =
        (jfw_plugin_startJavaVirtualMachine_ptr)
        osl_getSymbol(modulePlugin, sFunctionName.pData);
    if (pFunc == NULL)
        return JFW_E_ERROR;

    // create JavaVMOptions array that is passed to the plugin
    // it contains the classpath and all options set in the
    //options dialog
    if (cOptions > 0 && arOptions == NULL)
        return JFW_E_INVALID_ARG;
    boost::scoped_array<JavaVMOption> sarJOptions(
        new JavaVMOption[cOptions + 2 + javaSettings.getVmParameters().size()]);
    JavaVMOption * arOpt = sarJOptions.get();
    if (! arOpt)
        return JFW_E_ERROR;

    //The first argument is the classpath
    rtl::OString sOptionClassPath;
    if ((errcode = jfw::makeClassPathOption(
             mode, javaSettings, sOptionClassPath)) != JFW_E_NONE)
        return errcode;
    arOpt[0].optionString= (char*) sOptionClassPath.getStr();
    arOpt[0].extraInfo = NULL;
    // Set a flag that this JVM has been created via the JNI Invocation API
    // (used, for example, by UNO remote bridges to share a common thread pool
    // factory among Java and native bridge implementations):
    arOpt[1].optionString = "-Dorg.openoffice.native=";
    arOpt[1].extraInfo = 0;

    //add the options set by options dialog
    typedef std::vector<rtl::OString>::const_iterator cit;
    const std::vector<rtl::OString> & params = javaSettings.getVmParameters();
    int index = 2;
    for (cit i = params.begin(); i != params.end(); i ++)
    {
        arOpt[index].optionString= (char*) i->getStr();
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
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    sal_Int64 nFeatureFlags = 0L;
    jfw::CJavaInfo aCurrentInfo;
    //Prepare the xml document and context
    rtl::OString sSettingsPath = jfw::getVendorSettingsPath();
     jfw::CXmlDocPtr doc(xmlParseFile(sSettingsPath.getStr()));
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context(xmlXPathNewContext(doc));
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;
    //Determine if accessibility support is needed
    bool bSupportAccessibility = jfw::isAccessibilitySupportDesired();
    nFeatureFlags = bSupportAccessibility ?
        JFW_FEATURE_ACCESSBRIDGE : 0L;

    //Get a list of services which provide Java information
    std::vector<jfw::PluginLibrary> vecPlugins;
    errcode = jfw::getVendorPluginURLs(doc, context, & vecPlugins);
    if (errcode != JFW_E_NONE)
        return errcode;

    //Use every plug-in library to get Java installations. At the first usable
    //Java the loop will break
    typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
    for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); i++)
    {
        const jfw::PluginLibrary & library = *i;
        jfw::VersionInfo versionInfo;
        errcode =  jfw::getVersionInformation(doc, context, library.sVendor,
                                         & versionInfo);
        if (errcode != JFW_E_NONE)
            return JFW_E_CONFIG_READWRITE;
        osl::Module pluginLib(library.sPath);
        if (pluginLib.is() == sal_False)
            return JFW_E_NO_PLUGIN;

        jfw_plugin_getAllJavaInfos_ptr getAllJavaFunc =
            (jfw_plugin_getAllJavaInfos_ptr) pluginLib.getSymbol(
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
            JavaInfo* pInfo = arInfos[ii];

            //We remember the very first installation in aCurrentInfo
            if (aCurrentInfo.getLocation().getLength() == 0)
                     aCurrentInfo = pInfo;
            // compare features
            // If the user does not require any features (nFeatureFlags = 0)
            // then the first installation is used
            if ((pInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
            {
                //the just found Java implements all required features
                //currently there is only accessibility!!!
                aCurrentInfo = pInfo;
                bInfoFound = true;
                break;
            }
        }
        //The array returned by jfw_plugin_getAllJavaInfos must be freed as well as
        //its contents
        for (int i = 0; i < cInfos; i++)
            jfw_freeJavaInfo(arInfos[i]);
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
        jfw::CNodeJava node;
        errcode = node.loadFromSettings();
        if (errcode != JFW_E_NONE)
            return errcode;
        const std::vector<rtl::OString> & vecJRELocations =
            node.getJRELocations();
        //use every plug-in to determine the JavaInfo objects
        bool bInfoFound = false;
        for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); i++)
        {
            const jfw::PluginLibrary & library = *i;
            jfw::VersionInfo versionInfo;
            errcode =  jfw::getVersionInformation(doc, context, library.sVendor,
                                                  & versionInfo);
            if (errcode != JFW_E_NONE)
                return JFW_E_CONFIG_READWRITE;
            osl::Module pluginLib(library.sPath);
            if (pluginLib.is() == sal_False)
                return JFW_E_NO_PLUGIN;
            //Check if the current plugin can detect JREs at the location
            // of the paths added by jfw_setJRELocations or jfw_addJRELocation
            //get the function from the plugin
            jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
                (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jfw_plugin_getJavaInfoByPath")));

            OSL_ASSERT(jfw_plugin_getJavaInfoByPathFunc);
            if (jfw_plugin_getJavaInfoByPathFunc == NULL)
                return JFW_E_ERROR;

            typedef std::vector<rtl::OString>::const_iterator citLoc;
            for (citLoc i = vecJRELocations.begin();
                 i != vecJRELocations.end(); i++)
            {
                rtl::OUString sLocation =
                    rtl::OStringToOUString(*i, RTL_TEXTENCODING_UTF8);
                JavaInfo* pInfo = NULL;
                javaPluginError err = (*jfw_plugin_getJavaInfoByPathFunc)(
                    sLocation.pData,
                    versionInfo.sMinVersion.pData,
                    versionInfo.sMaxVersion.pData,
                    versionInfo.getExcludeVersions(),
                    versionInfo.getExcludeVersionSize(),
                    & pInfo);
                if (err == JFW_PLUGIN_E_NO_JRE)
                    continue;
                if (err == JFW_PLUGIN_E_FAILED_VERSION)
                    continue;
                else if (err !=JFW_PLUGIN_E_NONE)
                    return JFW_E_ERROR;

                if (pInfo)
                {
                    //We remember the very first installation in aCurrentInfo
                    if (aCurrentInfo.getLocation().getLength() == 0)
                        aCurrentInfo = pInfo;
                    // compare features
                    // If the user does not require any features (nFeatureFlags = 0)
                    // then the first installation is used
                    if ((pInfo->nFeatures & nFeatureFlags) == nFeatureFlags)
                    {
                        //the just found Java implements all required features
                        //currently there is only accessibility!!!
                        aCurrentInfo = pInfo;
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
        jfw::CNodeJava javaNode;
        javaNode.setJavaInfo(aCurrentInfo,true);
        errcode = javaNode.writeSettings();

        if (errcode == JFW_E_NONE && pInfo !=NULL)
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
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (ppInfo == NULL)
        return JFW_E_INVALID_ARG;
    jfw::CNodeJava aSettings;
    errcode = aSettings.loadFromSettings();
    if (errcode == JFW_E_NONE)
    {
        jfw::CJavaInfo aInfo(aSettings.getJavaInfo());
        if (aInfo == NULL)
        {
            *ppInfo = NULL;
            return JFW_E_NONE;
        }
        //If the javavendors.xml has changed, then the current selected
        //Java is not valid anymore
        // /java/javaInfo/@vendorUpdate != javaSelection/updated (javavendors.xml)
        rtl::OString sUpdated;
        if ((errcode = jfw::getElementUpdated(sUpdated)) != JFW_E_NONE)
            return errcode;
        if (sUpdated.equals(aSettings.getJavaInfoAttrVendorUpdate()) == sal_False)
            return JFW_E_INVALID_SETTINGS;
        *ppInfo = aSettings.getJavaInfo();
    }

    return errcode;
}

javaFrameworkError SAL_CALL jfw_isVMRunning(sal_Bool *bRunning)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
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
    osl::MutexGuard guard(jfw::getFwkMutex());
    if (pPath == NULL || ppInfo == NULL)
        return JFW_E_INVALID_ARG;
    javaFrameworkError errcode = JFW_E_NONE;
    //Prepare the xml document and context
    rtl::OString sSettingsPath = jfw::getVendorSettingsPath();
     jfw::CXmlDocPtr doc(xmlParseFile(sSettingsPath.getStr()));
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context(xmlXPathNewContext(doc));
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;

    //Get a list of plugins which provide Java information
    std::vector<jfw::PluginLibrary> vecPlugins;
    errcode = jfw::getVendorPluginURLs(doc, context, & vecPlugins);
    if (errcode != JFW_E_NONE)
        return errcode;
    std::vector<rtl::OUString> vecVendors;
    typedef std::vector<rtl::OUString>::const_iterator CIT_VENDOR;
    errcode = jfw::getSupportedVendors(doc, context, & vecVendors);
    if (errcode != JFW_E_NONE)
        return errcode;

    //Use every plug-in library to determine if the path represents a
    //JRE. If a plugin recognized it then the loop will break
    typedef std::vector<jfw::PluginLibrary>::const_iterator ci_pl;
    for (ci_pl i = vecPlugins.begin(); i != vecPlugins.end(); i++)
    {
        const jfw::PluginLibrary & library = *i;
        jfw::VersionInfo versionInfo;
        errcode =  jfw::getVersionInformation(doc, context, library.sVendor,
                                         & versionInfo);
        if (errcode != JFW_E_NONE)
            return JFW_E_CONFIG_READWRITE;
        osl::Module pluginLib(library.sPath);

        if (pluginLib.is() == sal_False)
        {
            rtl::OString msg = rtl::OUStringToOString(
                library.sPath, osl_getThreadTextEncoding());
            fprintf(stderr,"[jvmfwk] Could not load plugin %s\n" \
                    "Modify the javavendors.xml accordingly!\n", msg.getStr());
            return JFW_E_NO_PLUGIN;
        }

        jfw_plugin_getJavaInfoByPath_ptr jfw_plugin_getJavaInfoByPathFunc =
            (jfw_plugin_getJavaInfoByPath_ptr) pluginLib.getSymbol(
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
            versionInfo.sMinVersion.pData,
            versionInfo.sMaxVersion.pData,
            versionInfo.getExcludeVersions(),
            versionInfo.getExcludeVersionSize(),
            & pInfo);

        if (plerr == JFW_PLUGIN_E_NONE)
        {
            //check if the vendor of the found JRE is supported
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
    return errcode;
}


javaFrameworkError SAL_CALL jfw_setSelectedJRE(JavaInfo const *pInfo)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    //check if pInfo is the selected JRE
    JavaInfo *currentInfo = NULL;
    errcode = jfw_getSelectedJRE( & currentInfo);
    if (errcode != JFW_E_NONE && errcode != JFW_E_INVALID_SETTINGS)
        return errcode;

    if (jfw_areEqualJavaInfo(currentInfo, pInfo) == sal_False)
    {
        jfw::CNodeJava node;
        node.setJavaInfo(pInfo, false);
        errcode = node.writeSettings();
        if (errcode != JFW_E_NONE)
            return errcode;
        //remember that the JRE was selected in this process
        jfw::setJavaSelected();
    }
    return errcode;
}
javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;

    if (g_bEnabledSwitchedOn == false && bEnabled == sal_True)
    {
        //When the process started then Enabled was false.
        //This is first time enabled is set to true.
        //That means, no preparational work has been done, such as setting the
        //LD_LIBRARY_PATH, etc.

        //check if Enabled is false;
        errcode = node.loadFromSettings();
        if (errcode != JFW_E_NONE)
            return errcode;
        if (node.getEnabled() == sal_False)
            g_bEnabledSwitchedOn = true;
    }
    node.setEnabled(bEnabled);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    //remember if the enabled was false at the beginning and has
    //been changed to true.
    return errcode;
}

javaFrameworkError SAL_CALL jfw_getEnabled(sal_Bool *pbEnabled)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (pbEnabled == NULL)
        return JFW_E_INVALID_ARG;
    jfw::CNodeJava node;
    errcode = node.loadFromSettings();
    if (errcode == JFW_E_NONE)
    {
        *pbEnabled = node.getEnabled();
    }
    return errcode;
}


javaFrameworkError SAL_CALL jfw_setVMParameters(
    rtl_uString * * arOptions, sal_Int32 nLen)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;

    if (arOptions == NULL && nLen != 0)
        return JFW_E_INVALID_ARG;
    node.setVmParameters(arOptions, nLen);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    return errcode;
}

javaFrameworkError SAL_CALL jfw_getVMParameters(
    rtl_uString *** parOptions, sal_Int32 * pLen)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (parOptions == NULL || pLen == NULL)
        return JFW_E_INVALID_ARG;
    jfw::CNodeJava node;
    errcode = node.loadFromSettings();
    if (errcode == JFW_E_NONE)
    {
        node.getVmParametersArray(parOptions, pLen);
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_setUserClassPath(rtl_uString * pCp)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;

    if (pCp == NULL)
        return JFW_E_INVALID_ARG;
    node.setUserClassPath(pCp);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    return errcode;
}

javaFrameworkError SAL_CALL jfw_getUserClassPath(rtl_uString ** ppCP)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (ppCP == NULL)
        return JFW_E_INVALID_ARG;
    jfw::CNodeJava node;
    errcode = node.loadFromSettings();
    if (errcode == JFW_E_NONE)
    {
        *ppCP = node.getUserClassPath().pData;
        rtl_uString_acquire(*ppCP);
    }
    return errcode;
}

javaFrameworkError SAL_CALL jfw_addJRELocation(rtl_uString * sLocation)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;

    if (sLocation == NULL)
        return JFW_E_INVALID_ARG;
    errcode = node.loadFromSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    node.addJRELocation(sLocation);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    return errcode;

}

javaFrameworkError SAL_CALL jfw_setJRELocations(
    rtl_uString ** arLocations, sal_Int32 nLen)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;

    if (arLocations == NULL && nLen != 0)
        return JFW_E_INVALID_ARG;
    node.setJRELocations(arLocations, nLen);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    return errcode;

}

javaFrameworkError SAL_CALL jfw_getJRELocations(
    rtl_uString *** parLocations, sal_Int32 *pLen)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (parLocations == NULL || pLen == NULL)
        return JFW_E_INVALID_ARG;
    jfw::CNodeJava node;
    errcode = node.loadFromSettings();
    if (errcode == JFW_E_NONE)
    {
        node.getJRELocations(parLocations, pLen);
    }
    return errcode;
}

void SAL_CALL jfw_lock()
{
    osl::Mutex * mutex = jfw::getFwkMutex();
    mutex->acquire();
}

void SAL_CALL jfw_unlock()
{
    osl::Mutex * mutex = jfw::getFwkMutex();
    mutex->release();
}


namespace jfw
{
CJavaInfo::CJavaInfo(): pInfo(0)
{
}

CJavaInfo::CJavaInfo(const ::JavaInfo* info): pInfo(0)
{
    pInfo = copyJavaInfo(info);
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
CJavaInfo & CJavaInfo::operator = (const ::JavaInfo* info)
{
    if (info == pInfo)
        return *this;

    jfw_freeJavaInfo(pInfo);
    pInfo = NULL;
    if (info != NULL)
    {
        pInfo = copyJavaInfo(info);
    }
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
::JavaInfo** CJavaInfo::operator & ()
{
    return & pInfo;
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

rtl::OUString CJavaInfo::getVersion() const
{
    if (pInfo)
        return rtl::OUString(pInfo->sVersion);
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

sal_uInt64 CJavaInfo::getRequirements() const
{
    if (pInfo)
        return pInfo->nRequirements;
    else
        return 0l;
}

rtl::ByteSequence CJavaInfo::getVendorData() const
{
    if (pInfo)
        return rtl::ByteSequence(pInfo->arVendorData);
    else
        return rtl::ByteSequence();
}
}
