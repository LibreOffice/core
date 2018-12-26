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
#include <vcl/event.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <svdata.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
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

void UITestLogger::logCommand(const OUString& rAction, const css::uno::Sequence< css::beans::PropertyValue >& rArgs)
{
    if (!mbValid)
        return;

    OUStringBuffer aBuffer(rAction);
    sal_Int32 nCount = rArgs.getLength();

    if (nCount > 0)
    {
        aBuffer.append(" {");
        for (sal_Int32 n = 0; n < nCount; n++)
        {
            const css::beans::PropertyValue& rProp = rArgs[n];

            OUString aTypeName = rProp.Value.getValueTypeName();

            if (aTypeName == "long" || aTypeName == "short")
            {
                sal_Int32 nValue = 0;
                rProp.Value >>= nValue;
                aBuffer.append("\"").append(rProp.Name).append("\": ");
                aBuffer.append(OUString::number(nValue)).append(", ");
            }
            else if (aTypeName == "unsigned long")
            {
                sal_uInt32 nValue = 0;
                rProp.Value >>= nValue;
                aBuffer.append("\"").append(rProp.Name).append("\": ");
                aBuffer.append(OUString::number(nValue)).append(", ");
            }
            else if (aTypeName == "boolean")
            {
                bool bValue = false;
                rProp.Value >>= bValue;
                aBuffer.append("\"").append(rProp.Name).append("\": ");
                if (bValue)
                    aBuffer.append("True, ");
                else
                    aBuffer.append("False, ");
            }
        }
        aBuffer.append("}");
    }

    OUString aCommand(aBuffer.makeStringAndClear());
    maStream.WriteLine(OUStringToOString(aCommand, RTL_TEXTENCODING_UTF8));
}

namespace {

// most likely this should be recursive
bool child_windows_have_focus(VclPtr<vcl::Window> const & xUIElement)
{
    sal_Int32 nCount = xUIElement->GetChildCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        vcl::Window* pChild = xUIElement->GetChild(i);
        if (pChild->HasFocus())
        {
            return true;
        }
        if (child_windows_have_focus(VclPtr<vcl::Window>(pChild)))
            return true;
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

    //We need to check for Parent's ID in case the UI Element is SubEdit of Combobox/SpinField
    const OUString& rID = xUIElement->get_id().isEmpty() ?
        xUIElement->GetParent()->get_id() : xUIElement->get_id();
    if (rID.isEmpty())
        return;

    sal_Unicode nChar = rEvent.GetCharCode();
    sal_uInt16 nKeyCode = rEvent.GetKeyCode().GetCode();
    bool bShift = rEvent.GetKeyCode().IsShift();
    bool bMod1 = rEvent.GetKeyCode().IsMod1();
    bool bMod2 = rEvent.GetKeyCode().IsMod2();
    bool bMod3 = rEvent.GetKeyCode().IsMod3();

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

    std::unique_ptr<UIObject> pUIObject = xUIElement->GetUITestFactory()(xUIElement.get());

    VclPtr <vcl::Window> pParent = xUIElement->GetParent();

    while (!pParent->IsTopWindow())
    {
        pParent = pParent->GetParent();
    }

    OUString aParentID = pParent->get_id();

    OUString aContent = pUIObject->get_type() + " Action:TYPE Id:" +
            rID + " Parent:"+ aParentID +" " + aKeyCode;
    maStream.WriteLine(OUStringToOString(aContent, RTL_TEXTENCODING_UTF8));
}

namespace {

OUString StringMapToOUString(const std::map<OUString, OUString>& rParameters)
{
    if (rParameters.empty())
        return OUString("");

    OUStringBuffer aParameterString = " {";

    for (std::map<OUString, OUString>::const_iterator itr = rParameters.begin();
        itr != rParameters.end(); ++itr)
    {
        if (itr != rParameters.begin())
            aParameterString.append(", ");
        aParameterString.append("\"").append(itr->first).append("\": \"").append(itr->second).append("\"");
    }

    aParameterString.append("}");

    return aParameterString.makeStringAndClear();
}

}

void UITestLogger::logEvent(const EventDescription& rDescription)
{
    OUString aParameterString = StringMapToOUString(rDescription.aParameters);

    OUString aLogLine = rDescription.aKeyWord + " Action:" +
        rDescription.aAction + " Id:" + rDescription.aID +
        " Parent:" + rDescription.aParent + aParameterString;

    log(aLogLine);
}

UITestLogger& UITestLogger::getInstance()
{
    ImplSVData *const pSVData = ImplGetSVData();
    assert(pSVData);

    if (!pSVData->maWinData.m_pUITestLogger)
    {
        pSVData->maWinData.m_pUITestLogger.reset(new UITestLogger);
    }

    return *pSVData->maWinData.m_pUITestLogger;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
