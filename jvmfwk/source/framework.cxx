/*************************************************************************
 *
 *  $RCSfile: framework.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2004-04-26 11:20:33 $
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
#include "sal/config.h"
#include "jvmfwk/framework.h"
#include "jvmfwk/vendorplugin.h"
#include "osl/mutex.hxx"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathinternals.h"
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
}

javaFrameworkError SAL_CALL jfw_findAllJREs(JavaInfo ***pparInfo, sal_Int32 *pSize)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    if (pparInfo == NULL || pSize == NULL)
        return JFW_E_INVALID_ARG;

    //Prepare the xml document and context
    rtl::OString sSettingsPath = jfw::getVendorSettingsPath();
     jfw::CXmlDocPtr doc = xmlParseFile(sSettingsPath.getStr());
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context = xmlXPathNewContext(doc);
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;

    //Get a list of plugins which provide Java information
    std::vector<jfw::PluginLibrary> vecPlugins;
    errcode = jfw::getVendorPluginURLs(doc, context, & vecPlugins);
    if (errcode != JFW_E_NONE)
        return errcode;
    //Add the JavaInfos found by getAllJavaInfos to the vector
    //Make sure that the contents are destroyed if this
    //function returns with an error
    std::vector<JavaInfo*> vecInfo;
    //Add the JavaInfos found by getJavaInfoByPath to this vector
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
            std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          jfw_freeJavaInfo);
            return JFW_E_CONFIG_READWRITE;
        }
        osl::Module pluginLib(library.sPath);
        if (pluginLib.is() == sal_False)
        {
            //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          jfw_freeJavaInfo);
            return JFW_E_NO_PLUGIN;
        }
        getAllJavaInfos_ptr getAllJavaFunc =
            (getAllJavaInfos_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getAllJavaInfos")));

        OSL_ASSERT(getAllJavaFunc);
        if (getAllJavaFunc == NULL)
        {
            //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          jfw_freeJavaInfo);
            return JFW_E_ERROR;
        }
        //get all installations of one vendor according to minVersion,
        //maxVersion and excludeVersions
        sal_Int32 cInfos = 0;
        JavaInfo** arInfos = NULL;
        javaPluginError plerr  = (*getAllJavaFunc)(
            versionInfo.sMinVersion.pData,
            versionInfo.sMaxVersion.pData,
            versionInfo.getExcludeVersions(),
            versionInfo.getExcludeVersionSize(),
            & arInfos,
            & cInfos);

        if (plerr != JFW_PLUGIN_E_NONE)
        {   //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          jfw_freeJavaInfo);
            return JFW_E_ERROR;
        }
        for (int i = 0; i < cInfos; i++)
            vecInfo.push_back(arInfos[i]);
        rtl_freeMemory(arInfos);

        //Check if the current plugin can detect JREs at the location
        // of the paths added by jfw_setJRELocations or jfw_addJRELocation
        //get the function from the plugin
        getJavaInfoByPath_ptr getJavaInfoByPathFunc =
            (getJavaInfoByPath_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getJavaInfoByPath")));

        OSL_ASSERT(getJavaInfoByPathFunc);
        if (getJavaInfoByPathFunc == NULL)
        {   //delete JavaInfo objects
            std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
            std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                          jfw_freeJavaInfo);
            return JFW_E_ERROR;
        }
        typedef std::vector<rtl::OString>::const_iterator citLoc;
        for (citLoc i = vecJRELocations.begin();
             i != vecJRELocations.end(); i++)
        {
            rtl::OUString sLocation =
                rtl::OStringToOUString(*i, RTL_TEXTENCODING_UTF8);
            JavaInfo* pInfo = NULL;
            plerr = (*getJavaInfoByPathFunc)(
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
                std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
                std::for_each(vecInfoManual.begin(), vecInfoManual.end(),
                              jfw_freeJavaInfo);
                return JFW_E_ERROR;
            }
            if (pInfo)
                vecInfoManual.push_back(pInfo);
        }
    }
    //Check which JavaInfo from vector vecInfoManual is already
    //contained in vecInfo. If it already exists then remove it from
    //vecInfoManual
    for (it_info i = vecInfo.begin(); i != vecInfo.end(); i++)
    {
        it_info it_duplicate =
            std::find_if(vecInfoManual.begin(), vecInfoManual.end(),
                         std::bind2nd(std::ptr_fun(jfw_areEqualJavaInfo), *i));
        if (it_duplicate != vecInfoManual.end())
            vecInfoManual.erase(it_duplicate);
    }
    //create an fill the array of JavaInfo*
    sal_Int32 nSize = vecInfo.size() + vecInfoManual.size();
    *pparInfo = (JavaInfo**) rtl_allocateMemory(
        nSize * sizeof(JavaInfo*));
    if (*pparInfo == NULL)
    {   //delete JavaInfo objects
        std::for_each(vecInfo.begin(), vecInfo.end(), jfw_freeJavaInfo);
        std::for_each(vecInfoManual.begin(), vecInfoManual.end(), jfw_freeJavaInfo);
        return JFW_E_ERROR;
    }
    typedef std::vector<JavaInfo*>::iterator it;
    int index = 0;
    //Add the automatically detected JREs
    for (it i = vecInfo.begin(); i != vecInfo.end(); i++)
        (*pparInfo)[index++] = *i;
    //Add the manually detected JREs
    //ToDo compare if the javainfo is already contained
    //new function jfw_isEqualJavaInfo
    for (it i = vecInfoManual.begin();i != vecInfoManual.end(); i++)
        (*pparInfo)[index++] = *i;

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
    if ((errcode = javaSettings.loadFromSettings()) != JFW_E_NONE)
        return errcode;

    //get the current java setting (javaInfo)
    jfw::CJavaInfo aInfo = javaSettings.getJavaInfo();
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

    //Check if the selected Java was set in this process. If so it
    //must not have the requirments flag JFW_REQUIRE_NEEDRESTART
    if ((aInfo->nRequirements & JFW_REQUIRE_NEEDRESTART)
        &&
        (jfw::wasJavaSelectedInSameProcess() == true))
        return JFW_E_NEED_RESTART;

    //get the function startJavaVirtualMachine
    rtl::OUString sLibPath;
    if ((errcode = jfw::getPluginLibrary(sLibPath)) != JFW_E_NONE)
        return errcode;
    osl::Module modulePlugin(sLibPath);
    if ( ! modulePlugin)
        return JFW_E_NO_PLUGIN;

    rtl::OUString sFunctionName(
        RTL_CONSTASCII_USTRINGPARAM("startJavaVirtualMachine"));
    startJavaVirtualMachine_ptr pFunc =
        (startJavaVirtualMachine_ptr)
        osl_getSymbol(modulePlugin, sFunctionName.pData);
    if (pFunc == NULL)
        return JFW_E_ERROR;

    //Compose the class path
    rtl::OUStringBuffer sBufCP(4096);
    //build the class path from the classes directory
    rtl::OUString sClassPath;
    errcode = jfw::buildClassPathFromDirectory(
        javaSettings.m_sClassesDirectory, sClassPath);
    if (errcode != JFW_E_NONE)
        return JFW_E_ERROR;
    sBufCP.append(sClassPath);
    // append all user selected jars to the classpath
    if (javaSettings.getUserClassPath().getLength() != 0)
    {
        char szSep[] = {SAL_PATHSEPARATOR,0};
        sBufCP.appendAscii(szSep);
        sBufCP.append(javaSettings.getUserClassPath());
    }
    //add the path of the UNO components
    rtl::OUString sComponents =
        jfw::retrieveClassPath(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                              "${$PKG_SharedUnoFile:UNO_JAVA_CLASSPATH}")));
    sBufCP.append(sComponents);
    sComponents = jfw::retrieveClassPath(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "${$PKG_UserUnoFile:UNO_JAVA_CLASSPATH}")));

    sBufCP.append(sComponents);
    rtl::OString sOptionClassPath("-Djava.class.path=");
    sOptionClassPath += rtl::OUStringToOString(
        sBufCP.makeStringAndClear(), osl_getThreadTextEncoding());


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
    for (int i = 0; i < cOptions; i++)
    {
        arOpt[index].optionString = arOptions[i].optionString;
        arOpt[index].extraInfo = arOptions[i].extraInfo;
        index++;
    }
    //start Java
    JavaVM *pVm = NULL;
    javaPluginError plerr = (*pFunc)(aInfo, arOpt, index, & pVm, ppEnv);
    if (plerr != JFW_PLUGIN_E_NONE)
    {
        errcode = JFW_E_ERROR;
    }
    else
    {
        g_pJavaVM = pVm;
        *ppVM = pVm;
    }
    return JFW_E_NONE;
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
     jfw::CXmlDocPtr doc = xmlParseFile(sSettingsPath.getStr());
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context = xmlXPathNewContext(doc);
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

        getAllJavaInfos_ptr getAllJavaFunc =
            (getAllJavaInfos_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getAllJavaInfos")));

        OSL_ASSERT(getAllJavaFunc);
        if (getAllJavaFunc == NULL)
            continue;

        //get all installations of one vendor according to minVersion,
        //maxVersion and excludeVersions
        sal_Int32 cInfos = 0;
        JavaInfo** arInfos = NULL;
        javaPluginError plerr  = (*getAllJavaFunc)(
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
        //The array returned by getAllJavaInfos must be freed as well as
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
            getJavaInfoByPath_ptr getJavaInfoByPathFunc =
                (getJavaInfoByPath_ptr) pluginLib.getSymbol(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getJavaInfoByPath")));

            OSL_ASSERT(getJavaInfoByPathFunc);
            if (getJavaInfoByPathFunc == NULL)
                return JFW_E_ERROR;

            typedef std::vector<rtl::OString>::const_iterator citLoc;
            for (citLoc i = vecJRELocations.begin();
                 i != vecJRELocations.end(); i++)
            {
                rtl::OUString sLocation =
                    rtl::OStringToOUString(*i, RTL_TEXTENCODING_UTF8);
                JavaInfo* pInfo = NULL;
                javaPluginError err = (*getJavaInfoByPathFunc)(
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
        javaNode.setJavaInfo(aCurrentInfo);
        errcode = javaNode.writeSettings();

        if (errcode == JFW_E_NONE && pInfo !=NULL)
        {
            //copy to out param
            *pInfo = aCurrentInfo.cloneJavaInfo();
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
    OSL_ASSERT(pInfoA != NULL && pInfoB != NULL);
    if (pInfoA == pInfoB)
        return sal_True;

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
        jfw::CJavaInfo aInfo = aSettings.getJavaInfo();
        if (aInfo == NULL)
            return JFW_E_NO_SELECT;
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
     jfw::CXmlDocPtr doc = xmlParseFile(sSettingsPath.getStr());
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_ERROR;
    }
    jfw::CXPathContextPtr context = xmlXPathNewContext(doc);
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;

    //Get a list of plugins which provide Java information
    std::vector<jfw::PluginLibrary> vecPlugins;
    errcode = jfw::getVendorPluginURLs(doc, context, & vecPlugins);
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
            return JFW_E_NO_PLUGIN;

        getJavaInfoByPath_ptr getJavaInfoByPathFunc =
            (getJavaInfoByPath_ptr) pluginLib.getSymbol(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getJavaInfoByPath")));

        OSL_ASSERT(getJavaInfoByPathFunc);
        if (getJavaInfoByPathFunc == NULL)
            continue;

        //ask the plugin if this is a JRE.
        //If so check if it meets the version requirements.
        //Only if it does return a JavaInfo
        JavaInfo* pInfo = NULL;
        javaPluginError plerr = (*getJavaInfoByPathFunc)(
            pPath,
            versionInfo.sMinVersion.pData,
            versionInfo.sMaxVersion.pData,
            versionInfo.getExcludeVersions(),
            versionInfo.getExcludeVersionSize(),
            & pInfo);

        if (plerr == JFW_PLUGIN_E_NONE)
        {
            *ppInfo = pInfo;
            break;
        }
        else if(plerr == JFW_PLUGIN_E_FAILED_VERSION)
        {//found JRE but it has the wrong version
            *ppInfo = NULL;
            errcode = JFW_E_FAILED_VERSION;
            break;
        }
        else if (plerr = JFW_PLUGIN_E_NO_JRE)
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
    jfw::CNodeJava node;
    node.setJavaInfo(pInfo);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
    return errcode;
}
javaFrameworkError SAL_CALL jfw_setEnabled(sal_Bool bEnabled)
{
    osl::MutexGuard guard(jfw::getFwkMutex());
    javaFrameworkError errcode = JFW_E_NONE;
    jfw::CNodeJava node;
    node.setEnabled(bEnabled);
    errcode = node.writeSettings();
    if (errcode != JFW_E_NONE)
        return errcode;
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
          (JavaInfo*) rtl_allocateMemory(sizeof JavaInfo);
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
