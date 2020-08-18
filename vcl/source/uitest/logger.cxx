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
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <svdata.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <memory>

namespace
{
bool isDialogWindow(vcl::Window const* pWindow)
{
    WindowType nType = pWindow->GetType();
    if (nType == WindowType::DIALOG || nType == WindowType::MODELESSDIALOG)
        return true;

    // MESSBOX, INFOBOX, WARNINGBOX, ERRORBOX, QUERYBOX
    if (nType >= WindowType::MESSBOX && nType <= WindowType::QUERYBOX)
        return true;

    if (nType == WindowType::TABDIALOG)
        return true;

    return false;
}

bool isTopWindow(vcl::Window const* pWindow)
{
    WindowType eType = pWindow->GetType();
    if (eType == WindowType::FLOATINGWINDOW)
    {
        return pWindow->GetStyle() & WB_SYSTEMFLOATWIN;
    }
    return false;
}

vcl::Window* get_top_parent(vcl::Window* pWindow)
{
    if (isDialogWindow(pWindow) || isTopWindow(pWindow))
        return pWindow;

    vcl::Window* pParent = pWindow->GetParent();
    if (!pParent)
        return pWindow;

    return get_top_parent(pParent);
}
}
UITestLogger::UITestLogger()
    : maStream()
    , mbValid(false)
{
    static const char* pFile = std::getenv("LO_COLLECT_UIINFO");
    if (pFile)
    {
        OUString aDirPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                          "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/uitest/");
        rtl::Bootstrap::expandMacros(aDirPath);
        osl::Directory::createPath(aDirPath);
        OUString aFilePath = aDirPath + OUString::fromUtf8(pFile);

        maStream.Open(aFilePath, StreamMode::READWRITE | StreamMode::TRUNC);
        mbValid = true;
    }
}

void UITestLogger::logCommand(const OUString& rAction,
                              const css::uno::Sequence<css::beans::PropertyValue>& rArgs)
{
    if (!mbValid)
        return;

    OUStringBuffer aBuffer(rAction);

    if (rArgs.hasElements())
    {
        aBuffer.append(" {");
        for (const css::beans::PropertyValue& rProp : rArgs)
        {
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

namespace
{
// most likely this should be recursive
bool child_windows_have_focus(VclPtr<vcl::Window> const& xUIElement)
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

void UITestLogger::logAction(VclPtr<Control> const& xUIElement, VclEventId nEvent)
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

void UITestLogger::logKeyInput(VclPtr<vcl::Window> const& xUIElement, const KeyEvent& rEvent)
{
    if (!mbValid)
        return;

    //We need to check for Parent's ID in case the UI Element is SubEdit of Combobox/SpinField
    const OUString& rID
        = xUIElement->get_id().isEmpty() ? xUIElement->GetParent()->get_id() : xUIElement->get_id();
    if (rID.isEmpty())
        return;

    sal_Unicode nChar = rEvent.GetCharCode();
    sal_uInt16 nKeyCode = rEvent.GetKeyCode().GetCode();
    bool bShift = rEvent.GetKeyCode().IsShift();
    bool bMod1 = rEvent.GetKeyCode().IsMod1();
    bool bMod2 = rEvent.GetKeyCode().IsMod2();
    bool bMod3 = rEvent.GetKeyCode().IsMod3();

    std::map<OUString, sal_uInt16> aKeyMap
        = { { "ESC", KEY_ESCAPE },    { "TAB", KEY_TAB },          { "DOWN", KEY_DOWN },
            { "UP", KEY_UP },         { "LEFT", KEY_LEFT },        { "RIGHT", KEY_RIGHT },
            { "DELETE", KEY_DELETE }, { "INSERT", KEY_INSERT },    { "BACKSPACE", KEY_BACKSPACE },
            { "RETURN", KEY_RETURN }, { "HOME", KEY_HOME },        { "END", KEY_END },
            { "PAGEUP", KEY_PAGEUP }, { "PAGEDOWN", KEY_PAGEDOWN } };

    OUString aFound;
    for (const auto& itr : aKeyMap)
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
            aKeyCode += OUStringChar(nChar) + "\"}";
        else
            aKeyCode += aFound + "\"}";
    }
    else
    {
        aKeyCode = "{\"TEXT\": \"" + OUStringChar(nChar) + "\"}";
    }

    std::unique_ptr<UIObject> pUIObject = xUIElement->GetUITestFactory()(xUIElement.get());

    VclPtr<vcl::Window> pParent = xUIElement->GetParent();

    while (!pParent->IsTopWindow())
    {
        pParent = pParent->GetParent();
    }

    OUString aParentID = pParent->get_id();

    OUString aContent;

    if (pUIObject->get_type() == "EditUIObject")
    {
        if (aParentID.isEmpty())
        {
            VclPtr<vcl::Window> pParent_top = get_top_parent(xUIElement);
            aParentID = pParent_top->get_id();
        }
        if (aParentID.isEmpty())
        {
            aContent += "Type on '" + rID + "' " + aKeyCode;
        }
        else
        {
            aContent += "Type on '" + rID + "' " + aKeyCode + " from " + aParentID;
        }
    }
    else if (pUIObject->get_type() == "SwEditWinUIObject" && rID == "writer_edit")
    {
        aContent = "Type on writer " + aKeyCode;
    }
    else if (pUIObject->get_type() == "ScGridWinUIObject" && rID == "grid_window")
    {
        aContent = "Type on current cell " + aKeyCode;
    }
    else if (pUIObject->get_type() == "ImpressWindowUIObject" && rID == "impress_win")
    {
        aContent = "Type on impress " + aKeyCode;
    }
    else if (pUIObject->get_type() == "WindowUIObject" && rID == "math_edit")
    {
        aContent = "Type on math " + aKeyCode;
    }
    else if (rID == "draw_win")
    {
        aContent = "Type on draw " + aKeyCode;
    }
    else
    {
        if (aParentID.isEmpty())
        {
            VclPtr<vcl::Window> pParent_top = get_top_parent(xUIElement);
            aParentID = pParent_top->get_id();
        }
        if (aParentID.isEmpty())
        {
            aContent = "Type on '" + rID + "' " + aKeyCode;
        }
        else
        {
            aContent = "Type on '" + rID + "' " + aKeyCode + " from " + aParentID;
        }
    }
    maStream.WriteLine(OUStringToOString(aContent, RTL_TEXTENCODING_UTF8));
}

namespace
{
OUString StringMapToOUString(const std::map<OUString, OUString>& rParameters)
{
    if (rParameters.empty())
        return "";

    OUStringBuffer aParameterString(static_cast<int>(rParameters.size()*32));
    aParameterString.append(" {");

    for (std::map<OUString, OUString>::const_iterator itr = rParameters.begin();
         itr != rParameters.end(); ++itr)
    {
        if (itr != rParameters.begin())
            aParameterString.append(", ");
        aParameterString.append("\"")
            .append(itr->first)
            .append("\": \"")
            .append(itr->second)
            .append("\"");
    }

    aParameterString.append("}");

    return aParameterString.makeStringAndClear();
}

OUString GetValueInMapWithIndex(const std::map<OUString, OUString>& rParameters, sal_Int32 index)
{
    sal_Int32 j = 0;

    std::map<OUString, OUString>::const_iterator itr = rParameters.begin();

    for (; itr != rParameters.end() && j < index; ++itr, ++j)
        ;

    assert(itr != rParameters.end());

    return itr->second;
}

OUString GetKeyInMapWithIndex(const std::map<OUString, OUString>& rParameters, sal_Int32 index)
{
    sal_Int32 j = 0;

    std::map<OUString, OUString>::const_iterator itr = rParameters.begin();

    for (; itr != rParameters.end() && j < index; ++itr, ++j)
        ;

    assert(itr != rParameters.end());

    return itr->first;
}
}

void UITestLogger::logEvent(const EventDescription& rDescription)
{
    OUString aParameterString = StringMapToOUString(rDescription.aParameters);

    //here we will customize our statements depending on the caller of this function
    OUString aLogLine;
    //first check on general commands
    if (rDescription.aAction == "SET")
    {
        aLogLine = "Set Zoom to " + GetValueInMapWithIndex(rDescription.aParameters, 0);
    }
    else if (rDescription.aAction == "SIDEBAR")
    {
        aLogLine = "From SIDEBAR Choose " + aParameterString;
    }
    else if (rDescription.aAction == "SELECT" && rDescription.aID.isEmpty())
    {
        aLogLine = "Select " + aParameterString;
    }
    else if (rDescription.aID == "writer_edit")
    {
        if (rDescription.aAction == "GOTO")
        {
            aLogLine = "GOTO page number " + GetValueInMapWithIndex(rDescription.aParameters, 0);
        }
        else if (rDescription.aAction == "SELECT")
        {
            OUString to = GetValueInMapWithIndex(rDescription.aParameters, 0);
            OUString from = GetValueInMapWithIndex(rDescription.aParameters, 1);
            aLogLine = "Select from Pos " + from + " to Pos " + to;
        }
        else if (rDescription.aAction == "CREATE_TABLE")
        {
            OUString size = GetValueInMapWithIndex(rDescription.aParameters, 0);
            aLogLine = "Create Table with " + size;
            ;
        }
        else if (rDescription.aAction == "COPY")
        {
            aLogLine = "Copy the Selected Text";
        }
        else if (rDescription.aAction == "CUT")
        {
            aLogLine = "Cut the Selected Text";
        }
        else if (rDescription.aAction == "PASTE")
        {
            aLogLine = "Paste in the Current Cursor Location";
        }
        else if (rDescription.aAction == "BREAK_PAGE")
        {
            aLogLine = "Insert Break Page";
        }
    }
    else if (rDescription.aID == "grid_window")
    {
        if (rDescription.aAction == "SELECT")
        {
            OUString type = GetKeyInMapWithIndex(rDescription.aParameters, 0);
            if (type == "CELL" || type == "RANGE")
            {
                aLogLine = "Select from calc" + aParameterString;
            }
            else if (type == "TABLE")
            {
                aLogLine = "Switch to sheet number "
                           + GetValueInMapWithIndex(rDescription.aParameters, 0);
            }
        }
        else if (rDescription.aAction == "LAUNCH")
        {
            aLogLine = "Launch"
                       + GetKeyInMapWithIndex(rDescription.aParameters, 2)
                       + " from Col "
                       + GetValueInMapWithIndex(rDescription.aParameters, 2)
                       + " and Row "
                       + GetValueInMapWithIndex(rDescription.aParameters, 1);
        }
        else if (rDescription.aAction == "DELETE_CONTENT")
        {
            aLogLine = "Remove Content from This " + aParameterString;
        }
        else if (rDescription.aAction == "DELETE_CELLS")
        {
            aLogLine = "Delete The Cells in" + aParameterString;
        }
        else if (rDescription.aAction == "INSERT_CELLS")
        {
            aLogLine = "Insert Cell around the " + aParameterString;
        }
        else if (rDescription.aAction == "CUT")
        {
            aLogLine = "CUT the selected " + aParameterString;
        }
        else if (rDescription.aAction == "COPY")
        {
            aLogLine = "COPY the selected " + aParameterString;
        }
        else if (rDescription.aAction == "PASTE")
        {
            aLogLine = "Paste in the " + aParameterString;
        }
        else if (rDescription.aAction == "MERGE_CELLS")
        {
            aLogLine = "Merge " + aParameterString;
        }
        else if (rDescription.aAction == "UNMERGE_CELL")
        {
            aLogLine = "Delete the merged " + aParameterString;
        }
        else if (rDescription.aAction == "Rename_Sheet")
        {
            aLogLine = "Rename The Selected Tab to \""
                       + GetValueInMapWithIndex(rDescription.aParameters, 0) + "\"";
        }
        else if (rDescription.aAction == "InsertTab")
        {
            aLogLine = "Insert New Tab ";
        }
    }
    else if (rDescription.aID == "impress_win_or_draw_win")
    {
        if (rDescription.aAction == "Insert_New_Page_or_Slide")
        {
            if (UITestLogger::getInstance().getAppName() == "impress")
            {
                aLogLine = "Insert New Slide at Position "
                           + GetValueInMapWithIndex(rDescription.aParameters, 0);
            }
            else if (UITestLogger::getInstance().getAppName() == "draw")
            {
                aLogLine = "Insert New Page at Position "
                           + GetValueInMapWithIndex(rDescription.aParameters, 0);
            }
        }
        else if (rDescription.aAction == "Delete_Slide_or_Page")
        {
            if (UITestLogger::getInstance().getAppName() == "impress")
            {
                aLogLine
                    = "Delete Slide number " + GetValueInMapWithIndex(rDescription.aParameters, 0);
            }
            else if (UITestLogger::getInstance().getAppName() == "draw")
            {
                aLogLine
                    = "Delete Page number " + GetValueInMapWithIndex(rDescription.aParameters, 0);
            }
        }
        else if (rDescription.aAction == "Duplicate")
        {
            aLogLine = "Duplicate The Selected Slide ";
        }
        else if (rDescription.aAction == "RENAME")
        {
            if (UITestLogger::getInstance().getAppName() == "impress")
            {
                aLogLine = "Rename The Selected Slide from \""
                           + GetValueInMapWithIndex(rDescription.aParameters, 1) + "\" to \""
                           + GetValueInMapWithIndex(rDescription.aParameters, 0) + "\"";
            }
            else if (UITestLogger::getInstance().getAppName() == "draw")
            {
                aLogLine = "Rename The Selected Page from \""
                           + GetValueInMapWithIndex(rDescription.aParameters, 1) + "\" to \""
                           + GetValueInMapWithIndex(rDescription.aParameters, 0) + "\"";
            }
        }
    }
    else if (rDescription.aParent == "element_selector")
    {
        aLogLine = "Select element no " + rDescription.aID + " From " + rDescription.aParent;
    }
    else
    {
        aLogLine = rDescription.aKeyWord + " Action:" + rDescription.aAction + " Id:"
                   + rDescription.aID + " Parent:" + rDescription.aParent + aParameterString;
    }
    log(aLogLine);
}

UITestLogger& UITestLogger::getInstance()
{
    ImplSVData* const pSVData = ImplGetSVData();
    assert(pSVData);

    if (!pSVData->maFrameData.m_pUITestLogger)
    {
        pSVData->maFrameData.m_pUITestLogger.reset(new UITestLogger);
    }

    return *pSVData->maFrameData.m_pUITestLogger;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
