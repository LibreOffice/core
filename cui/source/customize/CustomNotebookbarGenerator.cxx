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

void CustomNotebookbarGenerator::getFileNameAndAppName(OUString& sAppName,
                                                       OUString& sNotebookbarUIFileName)
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (!pFrame)
        return;

    const auto& xContext = comphelper::getProcessComponentContext();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
