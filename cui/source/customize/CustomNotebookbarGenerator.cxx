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

static OUString getApplicationName()
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

void CustomNotebookbarGenerator::createCustomizedUIFile()
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
    OUString sAppName(lcl_getAppName(eApp));

    const Any aValue = aRoot.getNodeValue(sActiveAppName);
    OUString sNotebookbarUIFileName;
    aValue >>= sNotebookbarUIFileName;

    OUStringBuffer aOriginalUIPathBuffer = VclBuilderContainer::getUIRootDir();
    OUStringBuffer aCustomizedUIPathBuffer;

    OUStringBuffer aDirPath("$BRAND_BASE_DIR/user/config/soffice.cfg/");
    OUString sDirPath = aDirPath.makeStringAndClear();
    rtl::Bootstrap::expandMacros(sDirPath);
    aCustomizedUIPathBuffer.append(sDirPath);
    OUString sUIDirPath = OUString("modules/s") + sAppName.toAsciiLowerCase() + OUString("/ui/");
    OUString sUIFilePath = OUString("modules/s") + sAppName.toAsciiLowerCase() + OUString("/ui/")
                           + sNotebookbarUIFileName;

    aOriginalUIPathBuffer = aOriginalUIPathBuffer.append(sUIFilePath);
    aCustomizedUIPathBuffer = aCustomizedUIPathBuffer.append(sUIFilePath);

    OUString sOriginalUIPath = aOriginalUIPathBuffer.makeStringAndClear();
    OUString sCustomizedUIPath = aCustomizedUIPathBuffer.makeStringAndClear();

    OUStringBuffer aUserUIDir;
    aUserUIDir.append(sDirPath + sUIDirPath);
    OUString sUserUIDir = aUserUIDir.makeStringAndClear();

    sal_uInt32 nflag = osl_File_OpenFlag_Read | osl_File_OpenFlag_Write;

    osl::Directory aDirectory(sUserUIDir);
    if (aDirectory.open() != osl::FileBase::E_None)
        aDirectory.create(sUserUIDir, nflag);

    osl::File aFile(sCustomizedUIPath);
    if (aFile.open(nflag) != osl::FileBase::E_None)
    {
        aFile.copy(sOriginalUIPath, sCustomizedUIPath);
    }
}

Sequence<OUString> CustomNotebookbarGenerator::getCustomizedUIItem(OUString sNotebookbarConfigType)
{
    OUStringBuffer aPath = getApplicationName();
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
    OUStringBuffer aPath = getApplicationName();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(),
                                               aPath.makeStringAndClear(), true);
    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(sNotebookbarConfigType));

    css::uno::Any aUIItemProperties(makeAny(sUIItemProperties));
    aModeNode.setNodeValue("UIItemProperties", aUIItemProperties);
    aAppNode.commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */