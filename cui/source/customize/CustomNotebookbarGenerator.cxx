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

#define UIItemIDLength 255

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

static OUString getOriginalUIPath()
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
    for (int i = 0; i < sString.getLength(); i++)
        *(cString + i) = char(sString[i]);
    *(cString + sString.getLength()) = '\0';
    return cString;
}

static void changeNodeValue(xmlNode* nodePtr, char* property, char* value)
{
    nodePtr = nodePtr->xmlChildrenNode;
    while (nodePtr)
    {
        if (!(xmlStrcmp(nodePtr->name, reinterpret_cast<const xmlChar*>("property"))))
        {
            xmlChar* UriValue = xmlGetProp(nodePtr, reinterpret_cast<const xmlChar*>("name"));
            if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>(property))))
                xmlNodeSetContent(nodePtr, reinterpret_cast<const xmlChar*>(value));
            xmlFree(UriValue);
            break;
        }
        nodePtr = nodePtr->next;
    }
}

static void searchNodeAndAttribute(xmlNode* nodePtr, char* UIItemID, char* property, char* value)
{
    nodePtr = nodePtr->xmlChildrenNode;
    while (nodePtr)
    {
        if (nodePtr->type == XML_ELEMENT_NODE)
        {
            if (!(xmlStrcmp(nodePtr->name, reinterpret_cast<const xmlChar*>("object"))))
            {
                xmlChar* UriValue = xmlGetProp(nodePtr, reinterpret_cast<const xmlChar*>("id"));
                if (!(xmlStrcmp(UriValue, reinterpret_cast<const xmlChar*>(UIItemID))))
                    changeNodeValue(nodePtr, property, value);
                xmlFree(UriValue);
            }
            searchNodeAndAttribute(nodePtr, UIItemID, property, value);
        }
        nodePtr = nodePtr->next;
    }
}

static xmlDocPtr notebookbarXMLParser(char* docName, char* UIItemID, char* property, char* value)
{
    xmlDocPtr docPtr;
    xmlNodePtr nodePtr;

    docPtr = xmlParseFile(docName);
    nodePtr = xmlDocGetRootElement(docPtr);
    searchNodeAndAttribute(nodePtr, UIItemID, property, value);
    return docPtr;
}

void CustomNotebookbarGenerator::modifyCustomizedUIFile(Sequence<OUString> sUIItemProperties)
{
    OUString sCustomizedUIPath = getCustomizedUIPath();
    char* cCustomizedUIPath = convertToCharPointer(sCustomizedUIPath);

    for (auto const& aValue : sUIItemProperties)
    {
        char** aProperties = new char*[3];
        aProperties[0] = new char[UIItemIDLength];
        aProperties[1] = new char[UIItemIDLength];
        aProperties[2] = new char[UIItemIDLength];
        int idx = 0;
        int nextIdx = 0;
        for (int i = 0; i < aValue.getLength(); i++)
        {
            if (aValue[i] == ',')
            {
                if (aProperties[idx])
                    aProperties[idx][nextIdx] = '\0';
                idx++;
                nextIdx = 0;
            }
            else if (aValue[i] != ' ')
            {
                aProperties[idx][nextIdx] = char(aValue[i]);
                nextIdx++;
            }
        }
        aProperties[idx][nextIdx] = '\0';
        xmlDocPtr doc;
        doc = notebookbarXMLParser(cCustomizedUIPath, aProperties[0], aProperties[1],
                                   aProperties[2]);
        if (doc != nullptr)
        {
            xmlSaveFormatFile(cCustomizedUIPath, doc, 1);
            xmlFreeDoc(doc);
        }
        delete aProperties;
    }
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
        SAL_WARN("cui.CustomNotebookbar",
                 "Cannot create the directory or directory was present :" << sUserUIDir);

    osl::File aFile(sCustomizedUIPath);
    if (aFile.open(nflag) != osl::FileBase::E_None)
        osl::File::copy(sOriginalUIPath, sCustomizedUIPath);
    else
        SAL_WARN("cui.CustomNotebookbar",
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */