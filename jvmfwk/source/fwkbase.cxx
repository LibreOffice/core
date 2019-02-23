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

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <osl/thread.hxx>
#include <osl/process.h>
#include <libxml/xpathInternals.h>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include "framework.hxx"
#include <fwkutil.hxx>
#include <elements.hxx>
#include "fwkbase.hxx"

using namespace osl;


#define UNO_JAVA_JFW_PARAMETER "UNO_JAVA_JFW_PARAMETER_"
#define UNO_JAVA_JFW_JREHOME "UNO_JAVA_JFW_JREHOME"
#define UNO_JAVA_JFW_ENV_JREHOME "UNO_JAVA_JFW_ENV_JREHOME"
#define UNO_JAVA_JFW_CLASSPATH "UNO_JAVA_JFW_CLASSPATH"
#define UNO_JAVA_JFW_ENV_CLASSPATH "UNO_JAVA_JFW_ENV_CLASSPATH"
#define UNO_JAVA_JFW_CLASSPATH_URLS "UNO_JAVA_JFW_CLASSPATH_URLS"
#define UNO_JAVA_JFW_VENDOR_SETTINGS "UNO_JAVA_JFW_VENDOR_SETTINGS"

namespace jfw
{
static bool  g_bJavaSet = false;

namespace {

OString getVendorSettingsPath(OUString const & sURL)
{
    if (sURL.isEmpty())
        return OString();
    OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getVendorSettingsPath (fwkbase.cxx) ");
    OString osSystemPathSettings =
        OUStringToOString(sSystemPathSettings,osl_getThreadTextEncoding());
    return osSystemPathSettings;
}

OUString getParam(OUString const & name)
{
    OUString retVal;
    bool b = Bootstrap::get()->getFrom(name, retVal);
    SAL_INFO(
        "jfw",
        "Using bootstrap parameter " << name << " = \"" << retVal << "\""
            << (b ? "" : " (undefined)"));
    return retVal;
}

OUString getParamFirstUrl(OUString const & name)
{
    // Some parameters can consist of multiple URLs (separated by space
    // characters, although trim() harmlessly also removes other white-space),
    // of which only the first is used:
    return getParam(name).trim().getToken(0, ' ');
}

}//blind namespace


VendorSettings::VendorSettings():
    m_xmlDocVendorSettingsFileUrl(BootParams::getVendorSettings())
{
    //Prepare the xml document and context
    OString sSettingsPath = getVendorSettingsPath(m_xmlDocVendorSettingsFileUrl);
    if (sSettingsPath.isEmpty())
    {
        OString sMsg("[Java framework] A vendor settings file was not specified."
               "Check the bootstrap parameter " UNO_JAVA_JFW_VENDOR_SETTINGS ".");
        SAL_WARN( "jfw", sMsg );
        throw FrameworkException(JFW_E_CONFIGURATION, sMsg);
    }
    if (!sSettingsPath.isEmpty())
    {
        m_xmlDocVendorSettings = xmlParseFile(sSettingsPath.getStr());
        if (m_xmlDocVendorSettings == nullptr)
            throw FrameworkException(
                JFW_E_ERROR,
                OString("[Java framework] Error while parsing file: ")
                + sSettingsPath + ".");

        m_xmlPathContextVendorSettings = xmlXPathNewContext(m_xmlDocVendorSettings);
        int res = xmlXPathRegisterNs(
            m_xmlPathContextVendorSettings, reinterpret_cast<xmlChar const *>("jf"),
            reinterpret_cast<xmlChar const *>(NS_JAVA_FRAMEWORK));
        if (res == -1)
            throw FrameworkException(JFW_E_ERROR,
                    "[Java framework] Error in constructor VendorSettings::VendorSettings() (fwkbase.cxx)");
    }
}

VersionInfo VendorSettings::getVersionInformation(const OUString & sVendor) const
{
    OSL_ASSERT(!sVendor.isEmpty());
    VersionInfo aVersionInfo;
    OString osVendor = OUStringToOString(sVendor, RTL_TEXTENCODING_UTF8);
    //Get minVersion
    OString sExpression = OString(
        "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:minVersion");

    CXPathObjectPtr xPathObjectMin;
    xPathObjectMin =
        xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                               m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMin->nodesetval))
    {
        aVersionInfo.sMinVersion.clear();
    }
    else
    {
        CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMin->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        OString osVersion(sVersion);
        aVersionInfo.sMinVersion = OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get maxVersion
    sExpression = OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:maxVersion");
    CXPathObjectPtr xPathObjectMax;
    xPathObjectMax = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>(sExpression.getStr()),
        m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMax->nodesetval))
    {
        aVersionInfo.sMaxVersion.clear();
    }
    else
    {
        CXmlCharPtr sVersion;
        sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMax->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        OString osVersion(sVersion);
        aVersionInfo.sMaxVersion = OStringToOUString(
            osVersion, RTL_TEXTENCODING_UTF8);
    }

    //Get excludeVersions
    sExpression = OString("/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"") +
        osVendor + OString("\"]/jf:excludeVersions/jf:version");
    CXPathObjectPtr xPathObjectVersions;
    xPathObjectVersions =
        xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                               m_xmlPathContextVendorSettings);
    if (!xmlXPathNodeSetIsEmpty(xPathObjectVersions->nodesetval))
    {
        xmlNode* cur = xPathObjectVersions->nodesetval->nodeTab[0];
        while (cur != nullptr)
        {
            if (cur->type == XML_ELEMENT_NODE )
            {
                if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("version")) == 0)
                {
                    CXmlCharPtr sVersion;
                    sVersion = xmlNodeListGetString(
                        m_xmlDocVendorSettings, cur->xmlChildrenNode, 1);
                    OString osVersion(sVersion);
                    OUString usVersion = OStringToOUString(
                        osVersion, RTL_TEXTENCODING_UTF8);
                    aVersionInfo.vecExcludeVersions.push_back(usVersion);
                }
            }
            cur = cur->next;
        }
    }
    return aVersionInfo;
}

std::vector<OUString> VendorSettings::getSupportedVendors() const
{
    std::vector<OUString> vecVendors;
    //get the nodeset for the vendor elements
    jfw::CXPathObjectPtr result;
    result = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>("/jf:javaSelection/jf:vendorInfos/jf:vendor"),
        m_xmlPathContextVendorSettings);
    if (!xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        //get the values of the vendor elements + name attribute
        xmlNode* cur = result->nodesetval->nodeTab[0];
        while (cur != nullptr)
        {
            //between vendor elements are also text elements
            if (cur->type == XML_ELEMENT_NODE)
            {
                jfw::CXmlCharPtr sAttrVendor(xmlGetProp(cur, reinterpret_cast<xmlChar const *>("name")));
                vecVendors.push_back(sAttrVendor);
            }
            cur = cur->next;
        }
    }
    return vecVendors;
}

::std::vector<OString> BootParams::getVMParameters()
{
    ::std::vector<OString> vecParams;

    for (sal_Int32 i = 1; ; i++)
    {
        OUString sName = UNO_JAVA_JFW_PARAMETER + OUString::number(i);
        OUString sValue;
        if (Bootstrap::get()->getFrom(sName, sValue))
        {
            OString sParam =
                OUStringToOString(sValue, osl_getThreadTextEncoding());
            vecParams.push_back(sParam);
            SAL_INFO(
                "jfw.level2",
                "Using bootstrap parameter " << sName << " = " << sParam);
        }
        else
            break;
    }
    return vecParams;
}

OUString BootParams::getUserData()
{
    return getParamFirstUrl("UNO_JAVA_JFW_USER_DATA");
}

OUString BootParams::getSharedData()
{
    return getParamFirstUrl("UNO_JAVA_JFW_SHARED_DATA");
}

OString BootParams::getClasspath()
{
    OString sClassPath;
    OUString sCP;
    if (Bootstrap::get()->getFrom( UNO_JAVA_JFW_CLASSPATH, sCP ))
    {
        sClassPath = OUStringToOString(sCP, osl_getThreadTextEncoding());
        SAL_INFO(
            "jfw.level2",
            "Using bootstrap parameter " UNO_JAVA_JFW_CLASSPATH " = "
                << sClassPath);
    }

    OUString sEnvCP;
    if (Bootstrap::get()->getFrom( UNO_JAVA_JFW_ENV_CLASSPATH, sEnvCP ))
    {
        char * pCp = getenv("CLASSPATH");
        if (pCp)
        {
            char szSep[] = {SAL_PATHSEPARATOR,0};
            sClassPath += OString(szSep) + OString(pCp);
        }
        SAL_INFO(
            "jfw.level2",
            "Using bootstrap parameter " UNO_JAVA_JFW_ENV_CLASSPATH
                " and class path is: " << (pCp ? pCp : ""));
    }

    return sClassPath;
}

OUString BootParams::getVendorSettings()
{
    OUString sVendor;
    if (Bootstrap::get()->getFrom(UNO_JAVA_JFW_VENDOR_SETTINGS, sVendor))
    {
        //check the value of the bootstrap variable
        jfw::FileStatus s = checkFileURL(sVendor);
        if (s != FILE_OK)
        {
            //This bootstrap parameter can contain a relative URL
            OUString sAbsoluteUrl;
            OUString sBaseDir = getLibraryLocation();
            if (File::getAbsoluteFileURL(sBaseDir, sVendor, sAbsoluteUrl)
                != File::E_None)
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] Invalid value for bootstrap variable: "
                             UNO_JAVA_JFW_VENDOR_SETTINGS);
            sVendor = sAbsoluteUrl;
            s = checkFileURL(sVendor);
            if (s == jfw::FILE_INVALID || s == jfw::FILE_DOES_NOT_EXIST)
            {
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] Invalid value for bootstrap variable: "
                                 UNO_JAVA_JFW_VENDOR_SETTINGS);
            }
        }
        SAL_INFO(
        "jfw.level2",
        "Using bootstrap parameter " UNO_JAVA_JFW_VENDOR_SETTINGS " = "
            << sVendor);
    }
    return sVendor;
}

OUString BootParams::getJREHome()
{
    OUString sJRE;
    OUString sEnvJRE;
    bool bJRE = Bootstrap::get()->getFrom(UNO_JAVA_JFW_JREHOME, sJRE);
    bool bEnvJRE = Bootstrap::get()->getFrom(UNO_JAVA_JFW_ENV_JREHOME, sEnvJRE);

    if (bJRE && bEnvJRE)
    {
        throw FrameworkException(
            JFW_E_CONFIGURATION,
            "[Java framework] Both bootstrap parameter "
            UNO_JAVA_JFW_JREHOME" and "
            UNO_JAVA_JFW_ENV_JREHOME" are set. However only one of them can be set."
            "Check bootstrap parameters: environment variables, command line "
            "arguments, rc/ini files for executable and java framework library.");
    }
    else if (bEnvJRE)
    {
        const char * pJRE = getenv("JAVA_HOME");
        if (pJRE == nullptr)
        {
            throw FrameworkException(
                JFW_E_CONFIGURATION,
                "[Java framework] Both bootstrap parameter "
                UNO_JAVA_JFW_ENV_JREHOME" is set, but the environment variable "
                "JAVA_HOME is not set.");
        }
        OString osJRE(pJRE);
        OUString usJRE = OStringToOUString(osJRE, osl_getThreadTextEncoding());
        if (File::getFileURLFromSystemPath(usJRE, sJRE) != File::E_None)
            throw FrameworkException(
                JFW_E_ERROR,
                "[Java framework] Error in function BootParams::getJREHome() "
                "(fwkbase.cxx).");
        SAL_INFO(
            "jfw.level2",
            "Using bootstrap parameter " UNO_JAVA_JFW_ENV_JREHOME
                " with JAVA_HOME = " << pJRE);
    }
    else if (getMode() == JFW_MODE_DIRECT && !bJRE)
    {
        throw FrameworkException(
            JFW_E_CONFIGURATION,
            "[Java framework] The bootstrap parameter "
            UNO_JAVA_JFW_ENV_JREHOME" or " UNO_JAVA_JFW_JREHOME
            " must be set in direct mode.");
    }

    SAL_INFO_IF(
        bJRE, "jfw.level2",
        "Using bootstrap parameter " UNO_JAVA_JFW_JREHOME " = " << sJRE);
    return sJRE;
}

OUString BootParams::getClasspathUrls()
{
    OUString sParams;
    Bootstrap::get()->getFrom( UNO_JAVA_JFW_CLASSPATH_URLS, sParams);
    SAL_INFO(
        "jfw.level2",
        "Using bootstrap parameter " UNO_JAVA_JFW_CLASSPATH_URLS " = "
            << sParams);
    return sParams;
}

JFW_MODE getMode()
{
    static bool g_bMode = false;
    static JFW_MODE g_mode = JFW_MODE_APPLICATION;

    if (!g_bMode)
    {
        //check if either of the "direct mode" bootstrap variables is set
        bool bDirectMode = true;
        OUString sValue;
        const rtl::Bootstrap * aBoot = Bootstrap::get();
        if (!aBoot->getFrom(UNO_JAVA_JFW_JREHOME, sValue))
        {
            if (!aBoot->getFrom(UNO_JAVA_JFW_ENV_JREHOME, sValue))
            {
                if (!aBoot->getFrom(UNO_JAVA_JFW_CLASSPATH, sValue))
                {
                    if (!aBoot->getFrom(UNO_JAVA_JFW_ENV_CLASSPATH, sValue))
                    {
                        OUString sParams = UNO_JAVA_JFW_PARAMETER +
                            OUString::number(1);
                        if (!aBoot->getFrom(sParams, sValue))
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

OUString getApplicationClassPath()
{
    OSL_ASSERT(getMode() == JFW_MODE_APPLICATION);
    OUString sParams = BootParams::getClasspathUrls();
    if (sParams.isEmpty())
        return OUString();

    OUStringBuffer buf;
    sal_Int32 index = 0;
    const char szClassPathSep[] = {SAL_PATHSEPARATOR,0};
    do
    {
        OUString token( sParams.getToken( 0, ' ', index ).trim() );
        if (!token.isEmpty())
        {
            OUString systemPathElement;
            oslFileError rc = osl_getSystemPathFromFileURL(
                token.pData, &systemPathElement.pData );
            OSL_ASSERT( rc == osl_File_E_None );
            if (rc == osl_File_E_None && !systemPathElement.isEmpty())
            {
                if (buf.getLength() > 0)
                    buf.append( szClassPathSep );
                buf.append( systemPathElement );
            }
        }
    }
    while (index >= 0);
    return buf.makeStringAndClear();
}

OString makeClassPathOption(OUString const & sUserClassPath)
{
    //Compose the class path
    OString sPaths;
    OUStringBuffer sBufCP(4096);

    // append all user selected jars to the class path
    if (!sUserClassPath.isEmpty())
        sBufCP.append(sUserClassPath);

    //append all jar libraries and components to the class path
    OUString sAppCP = getApplicationClassPath();
    if (!sAppCP.isEmpty())
    {
        if (!sUserClassPath.isEmpty())
        {
            sBufCP.append(SAL_PATHSEPARATOR);
        }
        sBufCP.append(sAppCP);
    }

    sPaths = OUStringToOString(
        sBufCP.makeStringAndClear(), osl_getThreadTextEncoding());

    OString sOptionClassPath("-Djava.class.path=");
    sOptionClassPath += sPaths;
    return sOptionClassPath;
}

OString getUserSettingsPath()
{
    return getSettingsPath(BootParams::getUserData());
}

OString getSharedSettingsPath()
{
    return getSettingsPath(BootParams::getSharedData());
}

OString getSettingsPath( const OUString & sURL)
{
    if (sURL.isEmpty())
        return OString();
    OUString sPath;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sPath.pData) != osl_File_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function ::getSettingsPath (fwkbase.cxx).");
    return OUStringToOString(sPath,osl_getThreadTextEncoding());
}

OString getVendorSettingsPath()
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
    return g_bJavaSet;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
