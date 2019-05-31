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

#include <rtl/bootstrap.hxx>
#include <CustomNotebookbarGenerator.hxx>
#include <osl/file.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/viewfrm.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define aUIItemIDLength 255
#define aUIPropertiesCount 3

using namespace css;

CustomNotebookbarGenerator::CustomNotebookbarGenerator() {}

static OUString lcl_activeAppName(vcl::EnumContext::Application eApp)
{
    switch (eApp)
    {
        case vcl::EnumContext::Application::Writer:
            return OUString("ActiveWriter");
            break;
        case vcl::EnumContext::Application::Calc:
            return OUString("ActiveCalc");
            break;
        case vcl::EnumContext::Application::Impress:
            return OUString("ActiveImpress");
            break;
        case vcl::EnumContext::Application::Draw:
            return OUString("ActiveDraw");
            break;
        default:
            return OUString();
            break;
    }
}

static OUString lcl_getAppName(vcl::EnumContext::Application eApp)
{
    switch (eApp)
    {
        case vcl::EnumContext::Application::Writer:
            return OUString("Writer");
            break;
        case vcl::EnumContext::Application::Calc:
            return OUString("Calc");
            break;
        case vcl::EnumContext::Application::Impress:
            return OUString("Impress");
            break;
        case vcl::EnumContext::Application::Draw:
            return OUString("Draw");
            break;
        default:
            return OUString();
            break;
    }
}

static OUString getAppNameRegistryPath()
{
    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;
    const Reference<frame::XFrame>& xFrame
        = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager
        = frame::ModuleManager::create(::comphelper::getProcessComponentContext());
    eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));

    OUString sAppName(lcl_getAppName(eApp));
    OUStringBuffer sPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    sPath.append(sAppName);
    return sPath.makeStringAndClear();
}

static OUString customizedUIPathBuffer()
{
    OUString sDirPath("$BRAND_BASE_DIR/user/config/soffice.cfg/");
    rtl::Bootstrap::expandMacros(sDirPath);
    return sDirPath;
}

OUString CustomNotebookbarGenerator::getCustomizedUIPath()
{
    OUStringBuffer aCustomizedUIPathBuffer;
    aCustomizedUIPathBuffer.append(customizedUIPathBuffer());
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    OUString sUIFilePath
        = "modules/s" + sAppName.toAsciiLowerCase() + "/ui/" + sNotebookbarUIFileName;
    aCustomizedUIPathBuffer.append(sUIFilePath);
    OUString sCustomizedUIPath = aCustomizedUIPathBuffer.makeStringAndClear();
    return sCustomizedUIPath;
}

OUString CustomNotebookbarGenerator::getOriginalUIPath()
{
    OUStringBuffer aOriginalUIPathBuffer = VclBuilderContainer::getUIRootDir();
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    OUString sUIFilePath
        = "modules/s" + sAppName.toAsciiLowerCase() + "/ui/" + sNotebookbarUIFileName;
    aOriginalUIPathBuffer.append(sUIFilePath);
    OUString sOriginalUIPath = aOriginalUIPathBuffer.makeStringAndClear();
    return sOriginalUIPath;
}

static OUString getUIDirPath()
{
    OUString sAppName, sNotebookbarUIFileName;
    CustomNotebookbarGenerator::getFileNameAndAppName(sAppName, sNotebookbarUIFileName);
    OUString sUIDirPath
        = customizedUIPathBuffer() + "modules/s" + sAppName.toAsciiLowerCase() + "/ui/";
    return sUIDirPath;
}

char* CustomNotebookbarGenerator::convertToCharPointer(const OUString& sString)
{
    char* cString = nullptr;
    cString = new char[sString.getLength() + 1];
    for (int nIdx = 0; nIdx < sString.getLength(); nIdx++)
        *(cString + nIdx) = char(sString[nIdx]);
    *(cString + sString.getLength()) = '\0';
    return cString;
}

static void changeNodeValue(xmlNode* pNodePtr, char* pProperty, char* pValue)
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

static void searchNodeAndAttribute(xmlNode* pNodePtr, char* pUIItemID, char* pProperty,
                                   char* pValue)
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

static xmlDocPtr notebookbarXMLParser(char* pDocName, char* pUIItemID, char* pProperty,
                                      char* pValue)
{
    xmlDocPtr pDocPtr;
    xmlNodePtr pNodePtr;

    pDocPtr = xmlParseFile(pDocName);
    pNodePtr = xmlDocGetRootElement(pDocPtr);
    searchNodeAndAttribute(pNodePtr, pUIItemID, pProperty, pValue);
    return pDocPtr;
}

void CustomNotebookbarGenerator::modifyCustomizedUIFile(Sequence<OUString> sUIItemProperties)
{
    OUString sCustomizedUIPath = getCustomizedUIPath();
    char* cCustomizedUIPath = convertToCharPointer(sCustomizedUIPath);
    for (auto const& aValue : sUIItemProperties)
    {
        char** pProperties = new char*[aUIPropertiesCount];
        for (sal_Int32 aIndex = 0; aIndex < aUIPropertiesCount; aIndex++)
        {
            int nIdx = int(aIndex);
            sal_Int32 rPos = aIndex;
            pProperties[nIdx] = new char[aUIItemIDLength];
            pProperties[nIdx] = convertToCharPointer(aValue.getToken(rPos, ',', rPos));
        }
        xmlDocPtr doc;
        doc = notebookbarXMLParser(cCustomizedUIPath, pProperties[0], pProperties[1],
                                   pProperties[2]);

        for (int nIdx = 0; nIdx < aUIPropertiesCount; nIdx++)
        {
            delete[] pProperties[nIdx];
        }
        delete[] pProperties;

        if (doc != nullptr)
        {
            xmlSaveFormatFile(cCustomizedUIPath, doc, 1);
            xmlFreeDoc(doc);
        }
    }
    delete[] cCustomizedUIPath;
}

void CustomNotebookbarGenerator::getFileNameAndAppName(OUString& sAppName,
                                                       OUString& sNotebookbarUIFileName)
{
    utl::OConfigurationTreeRoot aRoot(::comphelper::getProcessComponentContext(),
                                      "org.openoffice.Office.UI.ToolbarMode/", false);
    const Reference<frame::XFrame>& xFrame
        = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager
        = frame::ModuleManager::create(::comphelper::getProcessComponentContext());

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;
    eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
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
    OUStringBuffer aPath = getAppNameRegistryPath();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(),
                                               aPath.makeStringAndClear(), false);

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(sNotebookbarConfigType));
    const Any aValue = aModeNode.getNodeValue("UIItemProperties");
    Sequence<OUString> aValues;
    aValue >>= aValues;
    return aValues;
}

void CustomNotebookbarGenerator::setCustomizedUIItem(Sequence<OUString> sUIItemProperties,
                                                     OUString sNotebookbarConfigType)
{
    OUStringBuffer aPath = getAppNameRegistryPath();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(),
                                               aPath.makeStringAndClear(), true);
    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(sNotebookbarConfigType));

    css::uno::Any aUIItemProperties(makeAny(sUIItemProperties));
    aModeNode.setNodeValue("UIItemProperties", aUIItemProperties);
    aAppNode.commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */