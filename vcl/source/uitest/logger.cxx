/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

#include <vcl/uitest/logger.hxx>

#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <vcl/uitest/uiobject.hxx>

#include <memory>

UITestLogger::UITestLogger():
    maStream(),
    mbValid(false)
{
    static const char* pFile = std::getenv("LO_COLLECT_UIINFO");
    if (pFile)
    {
        OUString aDirPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/uitest/");
        rtl::Bootstrap::expandMacros(aDirPath);
        osl::Directory::createPath(aDirPath);
        OUString aFilePath = aDirPath + OUString::fromUtf8(pFile);

        maStream.Open(aFilePath, StreamMode::READWRITE);
        mbValid = true;
    }
}

void UITestLogger::logCommand(const OUString& rAction)
{
    if (!mbValid)
        return;

    maStream.WriteLine(OUStringToOString(rAction, RTL_TEXTENCODING_UTF8));
}

void UITestLogger::logAction(VclPtr<Control>& xUIElement, VclEventId nEvent)
{
    if (!mbValid)
        return;

    if (xUIElement->get_id().isEmpty())
        return;

    std::unique_ptr<UIObject> pUIObject = xUIElement->GetUITestFactory()(xUIElement.get());
    OUString aAction = pUIObject->get_action(nEvent);
    if (!aAction.isEmpty())
        maStream.WriteLine(OUStringToOString(aAction, RTL_TEXTENCODING_UTF8));
}

void UITestLogger::log(const OUString& rString)
{
    if (!mbValid)
        return;

    if (rString.isEmpty())
        return;

    maStream.WriteLine(OUStringToOString(rString, RTL_TEXTENCODING_UTF8));
}

UITestLogger& UITestLogger::getInstance()
{
    static UITestLogger aInstance;
    return aInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
