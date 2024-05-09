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
#include <sal/log.hxx>
#include <libxml/xpathInternals.h>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <o3tl/string_view.hxx>
#include "framework.hxx"
#include <fwkutil.hxx>
#include <elements.hxx>
#include <fwkbase.hxx>

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

#if defined _WIN32
    // The paths are used in libxml. On Windows, it takes UTF-8 paths.
constexpr rtl_TextEncoding PathEncoding() { return RTL_TEXTENCODING_UTF8; }
#else
rtl_TextEncoding PathEncoding() { return osl_getThreadTextEncoding(); }
#endif

OString getVendorSettingsPath(OUString const & sURL)
{
    if (sURL.isEmpty())
        return OString();
    OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData,
        & sSystemPathSettings.pData) != osl_File_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getVendorSettingsPath (fwkbase.cxx) "_ostr);
    OString osSystemPathSettings = OUStringToOString(sSystemPathSettings, PathEncoding());
    return osSystemPathSettings;
}

OUString getParam(OUString const & name)
{
    OUString retVal;
    bool b = Bootstrap()->getFrom(name, retVal);
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


VendorSettings::VendorSettings()
{
    OUString xmlDocVendorSettingsFileUrl(BootParams::getVendorSettings());
    //Prepare the xml document and context
    OString sSettingsPath = getVendorSettingsPath(xmlDocVendorSettingsFileUrl);
    if (sSettingsPath.isEmpty())
    {
        OString sMsg("[Java framework] A vendor settings file was not specified."
               "Check the bootstrap parameter " UNO_JAVA_JFW_VENDOR_SETTINGS "."_ostr);
        SAL_WARN( "jfw", sMsg );
        throw FrameworkException(JFW_E_CONFIGURATION, sMsg);
    }
    if (sSettingsPath.isEmpty())
        return;

    m_xmlDocVendorSettings = xmlParseFile(sSettingsPath.getStr());
    if (m_xmlDocVendorSettings == nullptr)
        throw FrameworkException(
            JFW_E_ERROR,
            OString::Concat("[Java framework] Error while parsing file: ")
            + sSettingsPath + ".");

    m_xmlPathContextVendorSettings = xmlXPathNewContext(m_xmlDocVendorSettings);
    int res = xmlXPathRegisterNs(
        m_xmlPathContextVendorSettings, reinterpret_cast<xmlChar const *>("jf"),
        reinterpret_cast<xmlChar const *>(NS_JAVA_FRAMEWORK));
    if (res == -1)
        throw FrameworkException(JFW_E_ERROR,
                "[Java framework] Error in constructor VendorSettings::VendorSettings() (fwkbase.cxx)"_ostr);
}

VersionInfo VendorSettings::getVersionInformation(std::u16string_view sVendor) const
{
    OSL_ASSERT(!sVendor.empty());
    OString osVendor = OUStringToOString(sVendor, RTL_TEXTENCODING_UTF8);
    CXPathObjectPtr pathObject = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>(
            OString(
                "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"" + osVendor
                + "\"]/jf:minVersion").getStr()),
        m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(pathObject->nodesetval))
    {
        return {
            {},
#if defined MACOSX && defined __aarch64__
            "17",
#else
            u"1.8.0"_ustr,
#endif
            u""_ustr};
    }

    VersionInfo aVersionInfo;
    //Get minVersion
    OString sExpression =
        "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"" +
        osVendor + "\"]/jf:minVersion";

    CXPathObjectPtr xPathObjectMin =
        xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                               m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMin->nodesetval))
    {
        aVersionInfo.sMinVersion.clear();
    }
    else
    {
        CXmlCharPtr sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMin->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        aVersionInfo.sMinVersion = sVersion;
    }

    //Get maxVersion
    sExpression = "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"" +
        osVendor + "\"]/jf:maxVersion";
    CXPathObjectPtr xPathObjectMax = xmlXPathEvalExpression(
        reinterpret_cast<xmlChar const *>(sExpression.getStr()),
        m_xmlPathContextVendorSettings);
    if (xmlXPathNodeSetIsEmpty(xPathObjectMax->nodesetval))
    {
        aVersionInfo.sMaxVersion.clear();
    }
    else
    {
        CXmlCharPtr sVersion = xmlNodeListGetString(
            m_xmlDocVendorSettings,
            xPathObjectMax->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        aVersionInfo.sMaxVersion = sVersion;
    }

    //Get excludeVersions
    sExpression = "/jf:javaSelection/jf:vendorInfos/jf:vendor[@name=\"" +
        osVendor + "\"]/jf:excludeVersions/jf:version";
    CXPathObjectPtr xPathObjectVersions =
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
                    CXmlCharPtr sVersion = xmlNodeListGetString(
                        m_xmlDocVendorSettings, cur->xmlChildrenNode, 1);
                    OUString usVersion = sVersion;
                    aVersionInfo.vecExcludeVersions.push_back(usVersion);
                }
            }
            cur = cur->next;
        }
    }
    return aVersionInfo;
}

::std::vector<OString> BootParams::getVMParameters()
{
    ::std::vector<OString> vecParams;

    for (sal_Int32 i = 1; ; i++)
    {
        OUString sName = UNO_JAVA_JFW_PARAMETER + OUString::number(i);
        OUString sValue;
        if (Bootstrap()->getFrom(sName, sValue))
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
    return getParamFirstUrl(u"UNO_JAVA_JFW_USER_DATA"_ustr);
}

OUString BootParams::getSharedData()
{
    return getParamFirstUrl(u"UNO_JAVA_JFW_SHARED_DATA"_ustr);
}

OString BootParams::getClasspath()
{
    OString sClassPath;
    OUString sCP;
    if (Bootstrap()->getFrom( u"" UNO_JAVA_JFW_CLASSPATH ""_ustr, sCP ))
    {
        sClassPath = OUStringToOString(sCP, PathEncoding());
        SAL_INFO(
            "jfw.level2",
            "Using bootstrap parameter " UNO_JAVA_JFW_CLASSPATH " = "
                << sClassPath);
    }

    OUString sEnvCP;
    if (Bootstrap()->getFrom( u"" UNO_JAVA_JFW_ENV_CLASSPATH ""_ustr, sEnvCP ))
    {
        char * pCp = getenv("CLASSPATH");
        if (pCp)
        {
            sClassPath += OStringChar(SAL_PATHSEPARATOR) + pCp;
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
    if (Bootstrap()->getFrom(u"" UNO_JAVA_JFW_VENDOR_SETTINGS ""_ustr, sVendor))
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
                             UNO_JAVA_JFW_VENDOR_SETTINGS ""_ostr);
            sVendor = sAbsoluteUrl;
            s = checkFileURL(sVendor);
            if (s == jfw::FILE_INVALID || s == jfw::FILE_DOES_NOT_EXIST)
            {
                throw FrameworkException(
                    JFW_E_CONFIGURATION,
                    "[Java framework] Invalid value for bootstrap variable: "
                                 UNO_JAVA_JFW_VENDOR_SETTINGS ""_ostr);
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
    bool bJRE = Bootstrap()->getFrom(u"" UNO_JAVA_JFW_JREHOME ""_ustr, sJRE);
    bool bEnvJRE = Bootstrap()->getFrom(u"" UNO_JAVA_JFW_ENV_JREHOME ""_ustr, sEnvJRE);

    if (bJRE && bEnvJRE)
    {
        throw FrameworkException(
            JFW_E_CONFIGURATION,
            "[Java framework] Both bootstrap parameter "
            UNO_JAVA_JFW_JREHOME" and "
            UNO_JAVA_JFW_ENV_JREHOME" are set. However only one of them can be set."
            "Check bootstrap parameters: environment variables, command line "
            "arguments, rc/ini files for executable and java framework library."_ostr);
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
                "JAVA_HOME is not set."_ostr);
        }
        std::string_view osJRE(pJRE);
        OUString usJRE = OStringToOUString(osJRE, osl_getThreadTextEncoding());
        if (File::getFileURLFromSystemPath(usJRE, sJRE) != File::E_None)
            throw FrameworkException(
                JFW_E_ERROR,
                "[Java framework] Error in function BootParams::getJREHome() "
                "(fwkbase.cxx)."_ostr);
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
            " must be set in direct mode."_ostr);
    }

    SAL_INFO_IF(
        bJRE, "jfw.level2",
        "Using bootstrap parameter " UNO_JAVA_JFW_JREHOME " = " << sJRE);
    return sJRE;
}

OUString BootParams::getClasspathUrls()
{
    OUString sParams;
    Bootstrap()->getFrom( u"" UNO_JAVA_JFW_CLASSPATH_URLS ""_ustr, sParams);
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
        const rtl::Bootstrap * aBoot = Bootstrap();
        if (!aBoot->getFrom(u"" UNO_JAVA_JFW_JREHOME ""_ustr, sValue))
        {
            if (!aBoot->getFrom(u"" UNO_JAVA_JFW_ENV_JREHOME ""_ustr, sValue))
            {
                if (!aBoot->getFrom(u"" UNO_JAVA_JFW_CLASSPATH ""_ustr, sValue))
                {
                    if (!aBoot->getFrom(u"" UNO_JAVA_JFW_ENV_CLASSPATH ""_ustr, sValue))
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
    do
    {
        OUString token( o3tl::trim(o3tl::getToken(sParams, 0, ' ', index )) );
        if (!token.isEmpty())
        {
            OUString systemPathElement;
            oslFileError rc = osl_getSystemPathFromFileURL(
                token.pData, &systemPathElement.pData );
            OSL_ASSERT( rc == osl_File_E_None );
            if (rc == osl_File_E_None && !systemPathElement.isEmpty())
            {
                if (buf.getLength() > 0)
                    buf.append( SAL_PATHSEPARATOR );
                buf.append( systemPathElement );
            }
        }
    }
    while (index >= 0);
    return buf.makeStringAndClear();
}

OString makeClassPathOption(std::u16string_view sUserClassPath)
{
    //Compose the class path
    OString sPaths;
    OUStringBuffer sBufCP(4096);

    // append all user selected jars to the class path
    if (!sUserClassPath.empty())
        sBufCP.append(sUserClassPath);

    //append all jar libraries and components to the class path
    OUString sAppCP = getApplicationClassPath();
    if (!sAppCP.isEmpty())
    {
        if (!sUserClassPath.empty())
        {
            sBufCP.append(SAL_PATHSEPARATOR);
        }
        sBufCP.append(sAppCP);
    }

    sPaths = OUStringToOString(sBufCP, PathEncoding());
    if (sPaths.isEmpty()) {
        return ""_ostr;
    }

    OString sOptionClassPath = "-Djava.class.path=" + sPaths;
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
            "[Java framework] Error in function ::getSettingsPath (fwkbase.cxx)."_ostr);
    return OUStringToOString(sPath, PathEncoding());
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
