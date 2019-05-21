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
#include <customuigenerator.hxx>
#include <osl/file.hxx>
#include <vcl/dialog.hxx>
#include <sfx2/viewfrm.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>

using namespace css;

CustomUIGenerator::CustomUIGenerator() {  }

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

static OUStringBuffer getApplicationName()
{
    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;
    const Reference<frame::XFrame>& xFrame
        = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager
        = frame::ModuleManager::create(::comphelper::getProcessComponentContext());
    eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));

    OUString appName(lcl_getAppName(eApp));
    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    aPath.append(appName);
    return aPath;
}

void CustomUIGenerator::createCustomizedUIFile()
{
    utl::OConfigurationTreeRoot aRoot(::comphelper::getProcessComponentContext(),
                                      "org.openoffice.Office.UI.ToolbarMode/", false);
    const Reference<frame::XFrame>& xFrame
        = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
    const Reference<frame::XModuleManager> xModuleManager
        = frame::ModuleManager::create(::comphelper::getProcessComponentContext());

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;
    eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
    OUString activeAppName(lcl_activeAppName(eApp));
    OUString appName(lcl_getAppName(eApp));

    const Any aValue = aRoot.getNodeValue(activeAppName);
    OUString notebookBarType;
    aValue >>= notebookBarType;

    OUStringBuffer sShareLayer = VclBuilderContainer::getUIRootDir();
    OUStringBuffer sUserLayer;

    OUStringBuffer sdirPath("$BRAND_BASE_DIR/user/config/soffice.cfg/");
    OUString adirPath = sdirPath.makeStringAndClear();
    rtl::Bootstrap::expandMacros( adirPath  );
    sUserLayer.append( adirPath );

    OUString UIFilePath = OUString("modules/s") + appName.toAsciiLowerCase()
                          + OUString("/ui/") + notebookBarType;

    sShareLayer = sShareLayer.append(UIFilePath);
    sUserLayer = sUserLayer.append(UIFilePath);

    OUString sShareUIFilePath = sShareLayer.makeStringAndClear();
    OUString sUserUIFilePath = sUserLayer.makeStringAndClear();

    //Folder must exist to create a UIFile.
    osl::File file(sUserUIFilePath);
    sal_uInt32 flag = 0;
    if (file.open(flag) != osl::FileBase::E_None)
    {
        file.copy(sShareUIFilePath, sUserUIFilePath);
    }
}

Sequence<OUString> CustomUIGenerator::getCustomizedUIItem(OUString notebookbarType)
{
    OUStringBuffer aPath = getApplicationName();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(),
                                               aPath.makeStringAndClear(), false);

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(notebookbarType));
    const Any aValue = aModeNode.getNodeValue("UIItemProperties");
    Sequence<OUString> aValues;
    aValue >>= aValues;
    return aValues;
}

void CustomUIGenerator::setCustomizedUIItem(Sequence<OUString> sUIItemProperties,
                                            OUString NotebookBarConfig)
{
    OUStringBuffer aPath = getApplicationName();
    const utl::OConfigurationTreeRoot aAppNode(::comphelper::getProcessComponentContext(),
                                               aPath.makeStringAndClear(), true);
    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const utl::OConfigurationNode aModeNode(aModesNode.openNode(NotebookBarConfig));

    css::uno::Any aUIItemProperties(makeAny(sUIItemProperties));
    aModeNode.setNodeValue("UIItemProperties", aUIItemProperties);
    aAppNode.commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */