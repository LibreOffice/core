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

        maStream.Open(aFilePath, StreamMode::READWRITE | StreamMode::TRUNC);
        mbValid = true;
    }
}

void UITestLogger::logCommand(const OUString& rAction)
{
    if (!mbValid)
        return;

    maStream.WriteLine(OUStringToOString(rAction, RTL_TEXTENCODING_UTF8));
}

namespace {

// most likely this should be recursive
bool child_windows_have_focus(VclPtr<Control> const & xUIElement)
{
    sal_Int32 nCount = xUIElement->GetChildCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        if (xUIElement->GetChild(i)->HasFocus())
        {
            return true;
        }
    }
    return false;
}

}

void UITestLogger::logAction(VclPtr<Control> const & xUIElement, VclEventId nEvent)
{
    if (!mbValid)
        return;

    if (xUIElement->get_id().isEmpty())
        return;

    std::unique_ptr<UIObject> pUIObject = xUIElement->GetUITestFactory()(xUIElement.get());
    OUString aAction = pUIObject->get_action(nEvent);
    if (!xUIElement->HasFocus() && !child_windows_have_focus(xUIElement))
    {
        return;
    }

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

void UITestLogger::logKeyInput(VclPtr<vcl::Window> const & xUIElement, const KeyEvent& rEvent)
{
    if (!mbValid)
        return;

    const OUString& rID = xUIElement->get_id();
    if (rID.isEmpty())
        return;

    sal_Unicode nChar = rEvent.GetCharCode();
    sal_uInt16 nKeyCode = rEvent.GetKeyCode().GetCode();
    bool bShift = rEvent.GetKeyCode().IsShift();
    bool bMod1 = rEvent.GetKeyCode().IsMod1();
    bool bMod2 = rEvent.GetKeyCode().IsMod1();
    bool bMod3 = rEvent.GetKeyCode().IsMod1();

    std::map<OUString, sal_uInt16> aKeyMap = {
        {"ESC", KEY_ESCAPE},
        {"TAB", KEY_TAB},
        {"DOWN", KEY_DOWN},
        {"UP", KEY_UP},
        {"LEFT", KEY_LEFT},
        {"RIGHT", KEY_RIGHT},
        {"DELETE", KEY_DELETE},
        {"INSERT", KEY_INSERT},
        {"BACKSPACE", KEY_BACKSPACE},
        {"RETURN", KEY_RETURN},
        {"HOME", KEY_HOME},
        {"END", KEY_END},
        {"PAGEUP", KEY_PAGEUP},
        {"PAGEDOWN", KEY_PAGEDOWN}
    };

    OUString aFound;
    for (auto& itr : aKeyMap)
    {
        if (itr.second == nKeyCode)
        {
            aFound = itr.first;
            break;
        }
    }

    OUString aKeyCode;
    if (!aFound.isEmpty() || bShift || bMod1 || bMod2 || bMod3)
    {
        aKeyCode = "{\"KEYCODE\": \"";
        if (bShift)
            aKeyCode += "SHIFT+";

        if (bMod1)
            aKeyCode += "CTRL+";

        if (bMod2)
            aKeyCode += "ALT+";

        if (aFound.isEmpty())
            aKeyCode += OUStringLiteral1(nChar) + "\"}";
        else
            aKeyCode += aFound + "\"}";
    }
    else
    {
        aKeyCode = "{\"TEXT\": \"" + OUStringLiteral1(nChar) + "\"}";
    }

    OUString aContent = "Action on element: " + rID + " with action: TYPE and content: " + aKeyCode;
    maStream.WriteLine(OUStringToOString(aContent, RTL_TEXTENCODING_UTF8));
}

UITestLogger& UITestLogger::getInstance()
{
    static UITestLogger aInstance;
    return aInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
