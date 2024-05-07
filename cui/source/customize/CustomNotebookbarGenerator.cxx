/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <CustomNotebookbarGenerator.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <vcl/EnumContext.hxx>
#include <vcl/settings.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>
#include <libxml/parser.h>
#include <o3tl/string_view.hxx>

#define aUIPropertiesCount 3

using namespace css;

CustomNotebookbarGenerator::CustomNotebookbarGenerator() {}

static OUString lcl_activeAppName(vcl::EnumContext::Application eApp)
{
    switch (eApp)
    {
        case vcl::EnumContext::Application::Writer:
            return u"ActiveWriter"_ustr;
        case vcl::EnumContext::Application::Calc:
            return u"ActiveCalc"_ustr;
        case vcl::EnumContext::Application::Impress:
            return u"ActiveImpress"_ustr;
        case vcl::EnumContext::Application::Draw:
            return u"ActiveDraw"_ustr;
        default:
            return OUString();
    }
}

static OUString lcl_getAppName(vcl::EnumContext::Application eApp)
{
    switch (eApp)
    {
        case vcl::EnumContext::Application::Writer:
            return u"Writer"_ustr;
        case vcl::EnumContext::Application::Calc:
            return u"Calc"_ustr;
        case vcl::EnumContext::Application::Impress:
            return u"Impress"_ustr;
        case vcl::EnumContext::Application::Draw:
            return u"Draw"_ustr;
        default:
            return OUString();
    }
}

static OUString getAppNameRegistryPath()
{
    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;

    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = pViewFrame->GetFrame().GetFrameInterface();
        const Reference<frame::XModuleManager> xModuleManager
            = frame::ModuleManager::create(::comphelper::getProcessComponentContext());
        eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
    }

    OUString sAppName(lcl_getAppName(eApp));
    return "org.openoffice.Office.UI.ToolbarMode/Applications/" + sAppName;
}

static OUString customizedUIPathBuffer()
{
    OUString sDirPath(u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE(
        "bootstrap") ":UserInstallation}/user/config/soffice.cfg/"_ustr);
    rtl::Bootstrap::expandMacros(sDirPath);
    return sDirPath;
}

OUString CustomNotebookbarGenerator::getCustomizedUIPath()
{
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    return customizedUIPathBuffer() + "modules/s" + sAppName.toAsciiLowerCase() + "/ui/"
           + sNotebookbarUIFileName;
}

OUString CustomNotebookbarGenerator::getOriginalUIPath()
{
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    return AllSettings::GetUIRootDir() + "modules/s" + sAppName.toAsciiLowerCase() + "/ui/"
           + sNotebookbarUIFileName;
}

static OUString getUIDirPath()
{
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    OUString sUIDirPath
        = customizedUIPathBuffer() + "modules/s" + sAppName.toAsciiLowerCase() + "/ui/";
    return sUIDirPath;
}

OString CustomNotebookbarGenerator::getSystemPath(OUString const& sURL)
{
    if (sURL.isEmpty())
        return OString();
    OUString sSystemPathSettings;
    if (osl_getSystemPathFromFileURL(sURL.pData, &sSystemPathSettings.pData) != osl_File_E_None)
    {
        SAL_WARN("cui.customnotebookbar", "Cannot get system path for :" << sURL);
        return OString();
    }
    OString osSystemPathSettings
        = OUStringToOString(sSystemPathSettings, osl_getThreadTextEncoding());
    return osSystemPathSettings;
}

static void changeNodeValue(xmlNode* pNodePtr, const char* pProperty, const char* pValue)
{
    pNodePtr = pNodePtr->xmlChildrenNode;
    while (pNodePtr)
    {
        if (!(xmlStrcmp(pNodePtr->name, reinterpret_cast<const xmlChar*>("property"))))
        {
            xmlChar* UriValue = xmlGetProp(pNodePtr, reinterpret_cast<const xmlChar*>("name"));
            if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>(pProperty))))
                xmlNodeSetContent(pNodePtr, reinterpret_cast<const xmlChar*>(pValue));
            xmlFree(UriValue);
            break;
        }
        pNodePtr = pNodePtr->next;
    }
}

static void searchNodeAndAttribute(xmlNode* pNodePtr, const char* pUIItemID, const char* pProperty,
                                   const char* pValue)
{
    pNodePtr = pNodePtr->xmlChildrenNode;
    while (pNodePtr)
    {
        if (pNodePtr->type == XML_ELEMENT_NODE)
        {
            if (!(xmlStrcmp(pNodePtr->name, reinterpret_cast<const xmlChar*>("object"))))
            {
                xmlChar* UriValue = xmlGetProp(pNodePtr, reinterpret_cast<const xmlChar*>("id"));
                if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>(pUIItemID))))
                    changeNodeValue(pNodePtr, pProperty, pValue);
                xmlFree(UriValue);
            }
            searchNodeAndAttribute(pNodePtr, pUIItemID, pProperty, pValue);
        }
        pNodePtr = pNodePtr->next;
    }
}

static xmlDocPtr notebookbarXMLParser(const OString& rDocName, const OString& rUIItemID,
                                      const OString& rProperty, const OString& rValue)
{
    xmlDocPtr pDocPtr = xmlParseFile(rDocName.getStr());
    xmlNodePtr pNodePtr = xmlDocGetRootElement(pDocPtr);
    searchNodeAndAttribute(pNodePtr, rUIItemID.getStr(), rProperty.getStr(), rValue.getStr());
    return pDocPtr;
}

void CustomNotebookbarGenerator::modifyCustomizedUIFile(const Sequence<OUString>& sUIItemProperties)
{
    const OUString sUIPath = getCustomizedUIPath();
    if (osl::File(sUIPath).open(osl_File_OpenFlag_Read) != osl::FileBase::E_None)
        createCustomizedUIFile();

    const OString sCustomizedUIPath = getSystemPath(sUIPath);
    for (auto const& aValue : sUIItemProperties)
    {
        std::vector<OString> aProperties(aUIPropertiesCount);
        for (sal_Int32 aIndex = 0; aIndex < aUIPropertiesCount; aIndex++)
        {
            sal_Int32 nPos = aIndex;
            std::u16string_view sToken = o3tl::getToken(aValue, nPos, ',', nPos);
            aProperties[aIndex] = OUStringToOString(sToken, RTL_TEXTENCODING_UTF8);
        }
        xmlDocPtr doc = notebookbarXMLParser(sCustomizedUIPath, aProperties[0], aProperties[1],
                                             aProperties[2]);

        if (doc != nullptr)
        {
            xmlSaveFormatFile(sCustomizedUIPath.getStr(), doc, 1);
            xmlFreeDoc(doc);
        }
    }
}

void CustomNotebookbarGenerator::getFileNameAndAppName(OUString& sAppName,
                                                       OUString& sNotebookbarUIFileName)
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (!pFrame)
        return;

    const auto xContext = comphelper::getProcessComponentContext();
    utl::OConfigurationTreeRoot aRoot(xContext, u"org.openoffice.Office.UI.ToolbarMode/"_ustr,
                                      false);
    const Reference<frame::XFrame>& xFrame = pFrame->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create(xContext);

    vcl::EnumContext::Application eApp
        = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
    OUString sActiveAppName(lcl_activeAppName(eApp));
    sAppName = lcl_getAppName(eApp);
    const Any aValue = aRoot.getNodeValue(sActiveAppName);
    aValue >>= sNotebookbarUIFileName;
}

void CustomNotebookbarGenerator::createCustomizedUIFile()
{
    OUString sUserUIDir = getUIDirPath();
    OUString sOriginalUIPath = getOriginalUIPath();
    OUString sCustomizedUIPath = getCustomizedUIPath();

    sal_uInt32 nflag = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;
    osl::Directory aDirectory(sUserUIDir);
    if (aDirectory.open() != osl::FileBase::E_None)
        osl::Directory::create(sUserUIDir, nflag);
    else
        SAL_WARN("cui.customnotebookbar",
                 "Cannot create the directory or directory was present :" << sUserUIDir);

    osl::File aFile(sCustomizedUIPath);
    if (aFile.open(nflag) != osl::FileBase::E_None)
        osl::File::copy(sOriginalUIPath, sCustomizedUIPath);
    else
        SAL_WARN("cui.customnotebookbar",
                 "Cannot copy the file or file was present :" << sCustomizedUIPath);
}

Sequence<OUString> CustomNotebookbarGenerator::getCustomizedUIItem(OUString sNotebookbarConfigType)
{
    OUString aPath = getAppNameRegistryPath();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(), aPath,
                                               false);

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(sNotebookbarConfigType));
    const Any aValue = aModeNode.getNodeValue("UIItemProperties");
    Sequence<OUString> aValues;
    aValue >>= aValues;
    return aValues;
}

void CustomNotebookbarGenerator::setCustomizedUIItem(const Sequence<OUString>& rUIItemProperties,
                                                     const OUString& rNotebookbarConfigType)
{
    OUString aPath = getAppNameRegistryPath();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(), aPath,
                                               true);
    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(rNotebookbarConfigType));

    css::uno::Any aUIItemProperties(rUIItemProperties);
    aModeNode.setNodeValue("UIItemProperties", aUIItemProperties);
    aAppNode.commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
