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

#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/thread.hxx"
#include "osl/process.h"
#include "libxml/xpathInternals.h"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "framework.hxx"
#include "fwkutil.hxx"
#include "elements.hxx"
#include "fwkbase.hxx"

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;
#define JAVASETTINGS "javasettings"
#define JAVASETTINGS_XML "javasettings.xml"
#define VENDORSETTINGS "javavendors.xml"

#define UNO_JAVA_JFW_PARAMETER "UNO_JAVA_JFW_PARAMETER_"
#define UNO_JAVA_JFW_JREHOME "UNO_JAVA_JFW_JREHOME"
#define UNO_JAVA_JFW_ENV_JREHOME "UNO_JAVA_JFW_ENV_JREHOME"
#define UNO_JAVA_JFW_CLASSPATH "UNO_JAVA_JFW_CLASSPATH"
#define UNO_JAVA_JFW_ENV_CLASSPATH "UNO_JAVA_JFW_ENV_CLASSPATH"
#define UNO_JAVA_JFW_CLASSPATH_URLS "UNO_JAVA_JFW_CLASSPATH_URLS"
#define UNO_JAVA_JFW_PARAMETERS "UNO_JAVA_JFW_PARAMETERS"
#define UNO_JAVA_JFW_VENDOR_SETTINGS "UNO_JAVA_JFW_VENDOR_SETTINGS"
#define UNO_JAVA_JFW_USER_DATA "UNO_JAVA_JFW_USER_DATA"
#define UNO_JAVA_JFW_SHARED_DATA "UNO_JAVA_JFW_SHARED_DATA"

namespace jfw
{
bool  g_bJavaSet = false;

namespace {

rtl::OString getVendorSettingsPath(rtl::OUString const & sURL)
{
    if (sURL.isEmpty())
        return rtl::OString();
    rtl::OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            rtl::OString("[Java framework] Error in function "
                         "getVendorSettingsPath (fwkbase.cxx) "));
    rtl::OString osSystemPathSettings =
        rtl::OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}

rtl::OUString getParam(const char * name)
{
    rtl::OUString retVal;
    if (Bootstrap::get()->getFrom(rtl::OUString::createFromAscii(name), retVal))
    {
#if OSL_DEBUG_LEVEL >=2
        rtl::OString sValue = rtl::OUStringToOString(retVal, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework] Using bootstrap parameter %s = %s.\n",
                name, sValue.getStr());
#endif
    }
    return retVal;
}

rtl::OUString getParamFirstUrl(const char * name)
{
    // Some parameters can consist of multiple URLs (separated by space
    // characters, although trim() harmlessly also removes other white-space),
    // of which only the first is used:
    sal_Int32 i = 0;
    return getParam(name).trim().getToken(0, ' ', i);
}

}//blind namespace


VendorSettings::VendorSettings():
    m_xmlDocVendorSettingsFileUrl(BootParams::getVendorSettings())
{
    OString sMsgExc("[Java framework] Error in constructor "
                         "VendorSettings::VendorSettings() (fwkbase.cxx)");
    //Prepare the xml document and context
    OString sSettingsPath = getVendorSettingsPath(m_xmlDocVendorSettingsFileUrl);
    if (sSettingsPath.isEmpty())
    {
        OString sMsg("[Java framework] A vendor settings file was not specified."
               "Check the bootstrap parameter " UNO_JAVA_JFW_VENDOR_SETTINGS ".");
        OSL_FAIL(sMsg.getStr());
        throw FrameworkException(JFW_E_CONFIGURATION, sMsg);
    }
    if (!sSettingsPath.isEmpty())
    {
        m_xmlDocVendorSettings = xmlParseFile(sSettingsPath.getStr());
        if (m_xmlDocVendorSettings == NULL)
            throw FrameworkException(
                JFW_E_ERROR,
                OString("[Java framework] Error while parsing file: ")
                + sSettingsPath + OString("."));

        m_xmlPathContextVendorSettings = xmlXPathNewContext(m_xmlDocVendorSettings);
        int res = xmlXPathRegisterNs(
            m_xmlPathContextVendorSettings, (xmlChar*) "jf",
            (xmlChar*) NS_JAVA_FRAMEWORK);
        if (res == -1)
            throw FrameworkException(JFW_E_ERROR, sMsgExc);
    }
}

std::vector<PluginLibrary> VendorSettings::getPluginData()
{
    OString sExcMsg("[Java framework] Error in function VendorSettings::getVendorPluginURLs "
                         "(fwkbase.cxx).");
    std::vector<PluginLibrary> vecPlugins;
    CXPathObjectPtr result(xmlXPathEvalExpression(
        (xmlChar*)"/jf:javaSelection/jf:plugins/jf:library",
        m_xmlPathContextVendorSettings));
    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    //get the values of the library elements + vendor attribute
    xmlNode* cur = result->nodesetval->nodeTab[0];

    while (cur != NULL)
    {
        //between library elements are also text elements
        if (cur->type == XML_ELEMENT_NODE)
        {
            CXmlCharPtr sAttrVendor(xmlGetProp(cur, (xmlChar*) "vendor"));
            CXmlCharPtr sTextLibrary(
                xmlNodeListGetString(m_xmlDocVendorSettings,
                                     cur->xmlChildrenNode, 1));
            PluginLibrary plugin;
            OString osVendor((sal_Char*)(xmlChar*) sAttrVendor);
            plugin.sVendor = OStringToOUString(osVendor, RTL_TEXTENCODING_UTF8);

            //create the file URL to the library
            OUString sUrl = findPlugin(
                m_xmlDocVendorSettingsFileUrl, sTextLibrary);
            if (sUrl.isEmpty())
            {
                OString sPlugin = OUStringToOString(
                    sTextLibrary, osl_getThreadTextEncoding());
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] The file: " + sPlugin + " does not exist.");
            }
            plugin.sPath  = sUrl;

            vecPlugins.push_back(plugin);
        }
        cur = cur->next;
    }
    return vecPlugins;
}

VersionInfo VendorSettings::getVersionInformation(const rtl::OUString & sVendor)
{
    OSL_ASSERT(!sVendor.isEmpty());
    VersionInfo aVersionInfo;
    OString osVendor = OUStringToOString(sVendor, RTL_TEXTENCODING_UTF8);
    //Get minVersion
    OString sExpresion = OString(
        "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:minVersion");

    CXPathObjectPtr xPathObjectMin;
    xPathObjectMin =
        xmlXPathEvalExpression((xmlChar*) sExpresion.getStr(),
                               m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMin->nodesetval))
    {
        aVersionInfo.sMinVersion = OUString();
    }
    else
    {
        CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMin->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        OString osVersion((sal_Char*)(xmlChar*) sVersion);
        aVersionInfo.sMinVersion = OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get maxVersion
    sExpresion = OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:maxVersion");
    CXPathObjectPtr xPathObjectMax;
    xPathObjectMax = xmlXPathEvalExpression(
        (xmlChar*) sExpresion.getStr(),
        m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMax->nodesetval))
    {
        aVersionInfo.sMaxVersion = OUString();
    }
    else
    {
        CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMax->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        OString osVersion((sal_Char*) (xmlChar*) sVersion);
        aVersionInfo.sMaxVersion = OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get excludeVersions
    sExpresion = OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:excludeVersions/jf:version");
    CXPathObjectPtr xPathObjectVersions;
    xPathObjectVersions =
        xmlXPathEvalExpression((xmlChar*) sExpresion.getStr(),
                               m_xmlPathContextVendorSettings);
    if (!xmlXPathNodeSetIsEmpty(xPathObjectVersions->nodesetval))
    {
        xmlNode* cur = xPathObjectVersions->nodesetval->nodeTab[0];
        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE )
            {
                if (xmlStrcmp(cur->name, (xmlChar*) "version") == 0)
                {
                    CXmlCharPtr sVersion;
                    sVersion = xmlNodeListGetString(
                        m_xmlDocVendorSettings, cur->xmlChildrenNode, 1);
                    OString osVersion((sal_Char*)(xmlChar*) sVersion);
                    OUString usVersion = OStringToOUString(
                        osVersion, RTL_TEXTENCODING_UTF8);
                    aVersionInfo.addExcludeVersion(usVersion);
                }
            }
            cur = cur->next;
        }
    }
    return aVersionInfo;
}

std::vector<OUString> VendorSettings::getSupportedVendors()
{
    std::vector<rtl::OUString> vecVendors;
    //get the nodeset for the library elements
    jfw::CXPathObjectPtr result;
    result = xmlXPathEvalExpression(
        (xmlChar*)"/jf:javaSelection/jf:plugins/jf:library",
        m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(result->nodesetval))
        throw FrameworkException(
            JFW_E_ERROR,
            rtl::OString("[Java framework] Error in function getSupportedVendors (fwkbase.cxx)."));

    //get the values of the library elements + vendor attribute
    xmlNode* cur = result->nodesetval->nodeTab[0];
    while (cur != NULL)
    {
        //between library elements are also text elements
        if (cur->type == XML_ELEMENT_NODE)
        {
            jfw::CXmlCharPtr sAttrVendor(xmlGetProp(cur, (xmlChar*) "vendor"));
            vecVendors.push_back(sAttrVendor);
        }
        cur = cur->next;
    }
    return vecVendors;
}

OUString VendorSettings::getPluginLibrary(const OUString& sVendor)
{
    OSL_ASSERT(!sVendor.isEmpty());

    OString sExcMsg("[Java framework] Error in function getPluginLibrary (fwkbase.cxx).");
    OUStringBuffer usBuffer(256);
    usBuffer.appendAscii("/jf:javaSelection/jf:plugins/jf:library[@vendor=\"");
    usBuffer.append(sVendor);
    usBuffer.appendAscii("\"]/text()");
    OUString ouExpr = usBuffer.makeStringAndClear();
    OString sExpression =
        OUStringToOString(ouExpr, osl_getThreadTextEncoding());
    CXPathObjectPtr pathObjVendor;
    pathObjVendor = xmlXPathEvalExpression(
        (xmlChar*) sExpression.getStr(), m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(pathObjVendor->nodesetval))
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    CXmlCharPtr xmlCharPlugin;
    xmlCharPlugin =
        xmlNodeListGetString(
            m_xmlDocVendorSettings,pathObjVendor->nodesetval->nodeTab[0], 1);

    //make an absolute file url from the relative plugin URL
    OUString sUrl = findPlugin(m_xmlDocVendorSettingsFileUrl, xmlCharPlugin);
    if (sUrl.isEmpty())
    {
        OString sPlugin = OUStringToOString(
            xmlCharPlugin, osl_getThreadTextEncoding());
        throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] The file: " + sPlugin + " does not exist.");
    }
    return sUrl;
}

::std::vector<OString> BootParams::getVMParameters()
{
    ::std::vector<OString> vecParams;

    for (sal_Int32 i = 1; ; i++)
    {
        OUString sName =
            OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_PARAMETER)) +
            OUString::valueOf(i);
        OUString sValue;
        if (Bootstrap::get()->getFrom(sName, sValue) == sal_True)
        {
            OString sParam =
                OUStringToOString(sValue, osl_getThreadTextEncoding());
            vecParams.push_back(sParam);
#if OSL_DEBUG_LEVEL >=2
            rtl::OString sParamName = rtl::OUStringToOString(sName, osl_getThreadTextEncoding());
            fprintf(stderr,"[Java framework] Using bootstrap parameter %s"
                    " = %s.\n", sParamName.getStr(), sParam.getStr());
#endif
        }
        else
            break;
    }
    return vecParams;
}

rtl::OUString BootParams::getUserData()
{
    return getParamFirstUrl(UNO_JAVA_JFW_USER_DATA);
}

rtl::OUString BootParams::getSharedData()
{
    return getParamFirstUrl(UNO_JAVA_JFW_SHARED_DATA);
}

rtl::OString BootParams::getClasspath()
{
    rtl::OString sClassPath;
    rtl::OUString sCP;
    char szSep[] = {SAL_PATHSEPARATOR,0};
    if (Bootstrap::get()->getFrom(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_CLASSPATH)),
        sCP) == sal_True)
    {
        sClassPath = rtl::OUStringToOString(
            sCP, osl_getThreadTextEncoding());
#if OSL_DEBUG_LEVEL >=2
        fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_CLASSPATH " = %s.\n", sClassPath.getStr());
#endif
    }

    rtl::OUString sEnvCP;
    if (Bootstrap::get()->getFrom(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_ENV_CLASSPATH)),
        sEnvCP) == sal_True)
    {
        char * pCp = getenv("CLASSPATH");
        if (pCp)
        {
            sClassPath += rtl::OString(szSep) + rtl::OString(pCp);
        }
#if OSL_DEBUG_LEVEL >=2
        fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_ENV_CLASSPATH " and class path is:\n %s.\n", pCp ? pCp : "");
#endif
    }

    return sClassPath;
}

rtl::OUString BootParams::getVendorSettings()
{
    rtl::OUString sVendor;
    rtl::OUString sName(
        RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_VENDOR_SETTINGS));
    if (Bootstrap::get()->getFrom(sName ,sVendor) == sal_True)
    {
        //check the value of the bootstrap variable
        jfw::FileStatus s = checkFileURL(sVendor);
        if (s != FILE_OK)
        {
            //This bootstrap parameter can contain a relative URL
            rtl::OUString sAbsoluteUrl;
            rtl::OUString sBaseDir = getLibraryLocation();
            if (File::getAbsoluteFileURL(sBaseDir, sVendor, sAbsoluteUrl)
                != File::E_None)
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    rtl::OString("[Java framework] Invalid value for bootstrap variable: "
                             UNO_JAVA_JFW_VENDOR_SETTINGS));
            sVendor = sAbsoluteUrl;
            s = checkFileURL(sVendor);
            if (s == jfw::FILE_INVALID || s == jfw::FILE_DOES_NOT_EXIST)
            {
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    rtl::OString("[Java framework] Invalid value for bootstrap variable: "
                                 UNO_JAVA_JFW_VENDOR_SETTINGS));
            }
        }
#if OSL_DEBUG_LEVEL >=2
    rtl::OString sValue = rtl::OUStringToOString(sVendor, osl_getThreadTextEncoding());
    fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_VENDOR_SETTINGS" = %s.\n", sValue.getStr());
#endif
    }
    return sVendor;
}

rtl::OUString BootParams::getJREHome()
{
    rtl::OUString sJRE;
    rtl::OUString sEnvJRE;
    sal_Bool bJRE = Bootstrap::get()->getFrom(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_JREHOME)) ,sJRE);
    sal_Bool bEnvJRE = Bootstrap::get()->getFrom(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_ENV_JREHOME)) ,sEnvJRE);

    if (bJRE == sal_True && bEnvJRE == sal_True)
    {
        throw FrameworkException(
            JFW_E_CONFIGURATION,
            rtl::OString("[Java framework] Both bootstrap parameter "
                         UNO_JAVA_JFW_JREHOME" and "
                         UNO_JAVA_JFW_ENV_JREHOME" are set. However only one of them can be set."
                             "Check bootstrap parameters: environment variables, command line "
                             "arguments, rc/ini files for executable and java framework library."));
    }
    else if (bEnvJRE == sal_True)
    {
        const char * pJRE = getenv("JAVA_HOME");
        if (pJRE == NULL)
        {
            throw FrameworkException(
            JFW_E_CONFIGURATION,
            rtl::OString("[Java framework] Both bootstrap parameter "
                         UNO_JAVA_JFW_ENV_JREHOME" is set, but the environment variable "
                         "JAVA_HOME is not set."));
        }
        rtl::OString osJRE(pJRE);
        rtl::OUString usJRE = rtl::OStringToOUString(osJRE, osl_getThreadTextEncoding());
        if (File::getFileURLFromSystemPath(usJRE, sJRE) != File::E_None)
            throw FrameworkException(
                JFW_E_ERROR,
                rtl::OString("[Java framework] Error in function BootParams::getJREHome() "
                             "(fwkbase.cxx)."));
#if OSL_DEBUG_LEVEL >=2
    fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_ENV_JREHOME" with JAVA_HOME = %s.\n", pJRE);
#endif
    }
    else if (getMode() == JFW_MODE_DIRECT
        && bEnvJRE == sal_False
        && bJRE == sal_False)
    {
        throw FrameworkException(
            JFW_E_CONFIGURATION,
            rtl::OString("[Java framework] The bootstrap parameter "
                         UNO_JAVA_JFW_ENV_JREHOME" or " UNO_JAVA_JFW_JREHOME
                         " must be set in direct mode."));
    }

#if OSL_DEBUG_LEVEL >=2
    if (bJRE == sal_True)
    {
        rtl::OString sValue = rtl::OUStringToOString(sJRE, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_JREHOME" = %s.\n", sValue.getStr());
    }
#endif
    return sJRE;
}

rtl::OUString BootParams::getClasspathUrls()
{
    rtl::OUString sParams;
    Bootstrap::get()->getFrom(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_CLASSPATH_URLS)),
        sParams);
#if OSL_DEBUG_LEVEL >=2
    rtl::OString sValue = rtl::OUStringToOString(sParams, osl_getThreadTextEncoding());
    fprintf(stderr,"[Java framework] Using bootstrap parameter "
            UNO_JAVA_JFW_CLASSPATH_URLS " = %s.\n", sValue.getStr());
#endif
    return sParams;
}

JFW_MODE getMode()
{
    static bool g_bMode = false;
    static JFW_MODE g_mode = JFW_MODE_APPLICATION;

    if (g_bMode == false)
    {
        //check if either of the "direct mode" bootstrap variables is set
        bool bDirectMode = true;
        rtl::OUString sValue;
        const rtl::Bootstrap * aBoot = Bootstrap::get();
        rtl::OUString sJREHome(
            RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_JREHOME));
        if (aBoot->getFrom(sJREHome, sValue) == sal_False)
        {
            rtl::OUString sEnvJRE(
            RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_ENV_JREHOME));
            if (aBoot->getFrom(sEnvJRE, sValue) == sal_False)
            {
                rtl::OUString sClasspath(
                    RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_CLASSPATH));
                if (aBoot->getFrom(sClasspath, sValue) == sal_False)
                {
                    rtl::OUString sEnvClasspath(
                        RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_ENV_CLASSPATH));
                    if (aBoot->getFrom(sEnvClasspath, sValue) == sal_False)
                    {
                        rtl::OUString sParams = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(UNO_JAVA_JFW_PARAMETER)) +
                            rtl::OUString::valueOf((sal_Int32)1);
                        if (aBoot->getFrom(sParams, sValue) == sal_False)
                        {
                            bDirectMode = false;
                        }
                    }
                }
            }
        }

        if (bDirectMode)
            g_mode = JFW_MODE_DIRECT;
        else
            g_mode = JFW_MODE_APPLICATION;
        g_bMode = true;
    }

    return g_mode;
}

rtl::OUString getApplicationClassPath()
{
    OSL_ASSERT(getMode() == JFW_MODE_APPLICATION);
    rtl::OUString retVal;
    rtl::OUString sParams = BootParams::getClasspathUrls();
    if (sParams.isEmpty())
        return retVal;

    rtl::OUStringBuffer buf;
    sal_Int32 index = 0;
    char szClassPathSep[] = {SAL_PATHSEPARATOR,0};
    do
    {
        ::rtl::OUString token( sParams.getToken( 0, ' ', index ).trim() );
        if (!token.isEmpty())
        {
            ::rtl::OUString systemPathElement;
            oslFileError rc = osl_getSystemPathFromFileURL(
                token.pData, &systemPathElement.pData );
            OSL_ASSERT( rc == osl_File_E_None );
            if (rc == osl_File_E_None && !systemPathElement.isEmpty())
            {
                if (buf.getLength() > 0)
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                         szClassPathSep) );
                buf.append( systemPathElement );
            }
        }
    }
    while (index >= 0);
    return buf.makeStringAndClear();
}

rtl::OString makeClassPathOption(OUString const & sUserClassPath)
{
    //Compose the class path
    rtl::OString sPaths;
    rtl::OUStringBuffer sBufCP(4096);
    char szSep[] = {SAL_PATHSEPARATOR,0};

    // append all user selected jars to the class path
    if (!sUserClassPath.isEmpty())
        sBufCP.append(sUserClassPath);

    //append all jar libraries and components to the class path
    OUString sAppCP = getApplicationClassPath();
    if (!sAppCP.isEmpty())
    {
        if (!sUserClassPath.isEmpty())
            sBufCP.appendAscii(szSep);
        sBufCP.append(sAppCP);
    }

    sPaths = rtl::OUStringToOString(
        sBufCP.makeStringAndClear(), osl_getThreadTextEncoding());

    rtl::OString sOptionClassPath("-Djava.class.path=");
    sOptionClassPath += sPaths;
    return sOptionClassPath;
}

rtl::OString getUserSettingsPath()
{
    return getSettingsPath(BootParams::getUserData());
}

rtl::OString getSharedSettingsPath()
{
    return getSettingsPath(BootParams::getSharedData());
}

rtl::OString getSettingsPath( const rtl::OUString & sURL)
{
    if (sURL.isEmpty())
        return rtl::OString();
    rtl::OUString sPath;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sPath.pData) != osl_File_E_None)
        throw FrameworkException(
            JFW_E_ERROR, rtl::OString(
                "[Java framework] Error in function ::getSettingsPath (fwkbase.cxx)."));
    return rtl::OUStringToOString(sPath,osl_getThreadTextEncoding());
}

rtl::OString getVendorSettingsPath()
{
    return getVendorSettingsPath(BootParams::getVendorSettings());
}

void setJavaSelected()
{
    g_bJavaSet = true;
}

bool wasJavaSelectedInSameProcess()
{
    //g_setJavaProcId not set means no Java selected
    if (g_bJavaSet == true)
        return true;
    return false;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
