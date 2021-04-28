/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/metricfielduiobject.hxx>
#include <vcl/uitest/formattedfielduiobject.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/toolkit/spin.hxx>
#include <vcl/toolkit/fmtfield.hxx>
#include <vcl/toolkit/spinfld.hxx>
#include <vcl/toolkit/ivctrl.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <vcl/toolkit/menubtn.hxx>
#include <vcl/toolkit/vclmedit.hxx>
#include <vcl/uitest/logger.hxx>
#include <uiobject-internal.hxx>
#include <verticaltabctrl.hxx>
#include <vcl/toolbox.hxx>

#include <comphelper/string.hxx>
#include <comphelper/lok.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <iostream>
#include <memory>
#include <vector>

UIObject::~UIObject()
{
}

StringMap UIObject::get_state()
{
    StringMap aMap;
    aMap["NotImplemented"] = "NotImplemented";
    return aMap;
}

void UIObject::execute(const OUString& /*rAction*/,
        const StringMap& /*rParameters*/)
{
    // should never be called
    throw std::exception();
}

OUString UIObject::get_type() const
{
    return "Generic UIObject";
}

std::unique_ptr<UIObject> UIObject::get_child(const OUString&)
{
    return std::unique_ptr<UIObject>();
}

std::set<OUString> UIObject::get_children() const
{
    return std::set<OUString>();
}

OUString UIObject::dumpState() const
{
    return OUString();
}

OUString UIObject::dumpHierarchy() const
{
    return OUString();
}

OUString UIObject::get_action(VclEventId /*nEvent*/) const
{
    return OUString();
}

namespace {

bool isDialogWindow(vcl::Window const * pWindow)
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

bool isTopWindow(vcl::Window const * pWindow)
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

std::vector<KeyEvent> generate_key_events_from_text(const OUString& rStr)
{
    std::vector<KeyEvent> aEvents;
    vcl::KeyCode aCode;
    for (sal_Int32 i = 0, n = rStr.getLength();
            i != n; ++i)
    {
        aEvents.emplace_back(rStr[i], aCode);
    }
    return aEvents;
}

sal_uInt16 get_key(sal_Unicode cChar, bool& bShift)
{
    bShift = false;
    if (cChar >= 'a' && cChar <= 'z')
        return KEY_A + (cChar - 'a');
    else if (cChar >= 'A' && cChar <= 'Z')
    {
        bShift = true;
        return KEY_A + (cChar - 'A');
    }
    else if (cChar >= '0' && cChar <= '9')
        return KEY_0 + (cChar - 'A');

    return cChar;
}

bool isFunctionKey(const OUString& rStr, sal_uInt16& rKeyCode)
{
    std::map<OUString, sal_uInt16> aFunctionKeyMap = {
        {"F1", KEY_F1},
        {"F2", KEY_F2},
        {"F3", KEY_F3},
        {"F4", KEY_F4},
        {"F5", KEY_F5},
        {"F6", KEY_F6},
        {"F7", KEY_F7},
        {"F8", KEY_F8},
        {"F9", KEY_F9},
        {"F10", KEY_F10},
        {"F11", KEY_F11},
        {"F12", KEY_F12}
    };

    rKeyCode = 0;
    auto itr = aFunctionKeyMap.find(rStr);
    if (itr == aFunctionKeyMap.end())
        return false;

    rKeyCode = itr->second;
    return true;
}

std::vector<KeyEvent> generate_key_events_from_keycode(const OUString& rStr)
{
    std::vector<KeyEvent> aEvents;

    std::map<OUString, sal_uInt16> aKeyMap = {
        {"ESC", KEY_ESCAPE},
        {"TAB", KEY_TAB},
        {"DOWN", KEY_DOWN},
        {"UP", KEY_UP},
        {"LEFT", KEY_LEFT},
        {"RIGHT", KEY_RIGHT},
        {"DELETE", KEY_DELETE},
        {"INSERT", KEY_INSERT},
        {"SPACE", KEY_SPACE},
        {"BACKSPACE", KEY_BACKSPACE},
        {"RETURN", KEY_RETURN},
        {"HOME", KEY_HOME},
        {"END", KEY_END},
        {"PAGEUP", KEY_PAGEUP},
        {"PAGEDOWN", KEY_PAGEDOWN}
    };

    // split string along '+'
    // then translate to keycodes
    bool bShift = false;
    bool bMod1 = false;
    bool bMod2 = false;
    OUString aRemainingText;

    std::vector<OUString> aTokens = comphelper::string::split(rStr, '+');
    for (auto const& token : aTokens)
    {
        OUString aToken = token.trim();
        if (aToken == "CTRL")
        {
            bMod1 = true;
        }
        else if (aToken == "SHIFT")
        {
            bShift = true;
        }
        else if (aToken == "ALT")
        {
            bMod2 = true;
        }
        else
            aRemainingText = aToken;
    }

    sal_uInt16 nFunctionKey = 0;
    if (isFunctionKey(aRemainingText, nFunctionKey))
    {
        vcl::KeyCode aCode(nFunctionKey, bShift, bMod1, bMod2, false);
        aEvents.emplace_back(0, aCode);
    }
    else if (aKeyMap.find(aRemainingText) != aKeyMap.end())
    {
        sal_uInt16 nKey = aKeyMap[aRemainingText];
        vcl::KeyCode aCode(nKey, bShift, bMod1, bMod2, false);
        aEvents.emplace_back( 'a', aCode);
    }
    else
    {
        for (sal_Int32 i = 0; i < aRemainingText.getLength(); ++i)
        {
            bool bShiftThroughKey = false;
            sal_uInt16 nKey = get_key(aRemainingText[i], bShiftThroughKey);
            vcl::KeyCode aCode(nKey, bShift || bShiftThroughKey, bMod1, bMod2, false);
            aEvents.emplace_back(aRemainingText[i], aCode);
        }
    }

    return aEvents;
}

OUString to_string(const Point& rPos)
{
    OUString sStr = OUString::number(rPos.X())
                  + "x"
                  + OUString::number(rPos.Y());

    return sStr;
}

OUString to_string(const Size& rSize)
{
    OUString sStr = OUString::number(rSize.Width())
                  + "x"
                  + OUString::number(rSize.Height());

    return sStr;
}

}

WindowUIObject::WindowUIObject(const VclPtr<vcl::Window>& xWindow):
    mxWindow(xWindow)
{
}

StringMap WindowUIObject::get_state()
{
    // Double-buffering is not interesting for uitesting, but can result in direct paint for a
    // double-buffered widget, which is incorrect.
    if (mxWindow->SupportsDoubleBuffering())
        mxWindow->RequestDoubleBuffering(false);

    StringMap aMap;
    aMap["Visible"] = OUString::boolean(mxWindow->IsVisible());
    aMap["ReallyVisible"] = OUString::boolean(mxWindow->IsReallyVisible());
    aMap["Enabled"] = OUString::boolean(mxWindow->IsEnabled());
    aMap["WindowType"] = OUString::number(static_cast<sal_uInt16>(mxWindow->GetType()), 16);

    Point aPos = mxWindow->GetPosPixel();
    aMap["RelPosition"] = to_string(aPos);
    aMap["Size"] = to_string(mxWindow->GetSizePixel());
    aMap["ID"] = mxWindow->get_id();
    vcl::Window* pParent = mxWindow->GetParent();
    if (pParent)
        aMap["Parent"] = mxWindow->GetParent()->get_id();

    bool bIgnoreAllExceptTop = isDialogWindow(mxWindow.get());
    while(pParent)
    {
        Point aParentPos = pParent->GetPosPixel();
        if (!bIgnoreAllExceptTop)
            aPos += aParentPos;

        if (isDialogWindow(pParent))
        {
            bIgnoreAllExceptTop = true;
        }

        pParent = pParent->GetParent();

        if (!pParent && bIgnoreAllExceptTop)
            aPos += aParentPos;
    }
    aMap["AbsPosition"] = to_string(aPos);
    aMap["Text"] = mxWindow->GetText();
    aMap["DisplayText"] = mxWindow->GetDisplayText();

    return aMap;
}

void WindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        for (auto const& parameter : rParameters)
        {
            std::cout << parameter.first;
        }
    }
    else if (rAction == "TYPE")
    {
        auto it = rParameters.find("TEXT");
        if (it != rParameters.end())
        {
            const OUString& rText = it->second;
            auto aKeyEvents = generate_key_events_from_text(rText);
            for (auto const& keyEvent : aKeyEvents)
            {
                mxWindow->KeyInput(keyEvent);
            }
        }
        else if (rParameters.find("KEYCODE") != rParameters.end())
        {
            auto itr = rParameters.find("KEYCODE");
            const OUString rText = itr->second;
            auto aKeyEvents = generate_key_events_from_keycode(rText);
            for (auto const& keyEvent : aKeyEvents)
            {
                mxWindow->KeyInput(keyEvent);
            }
        }
        else
        {
            OStringBuffer buf;
            for (auto const & rPair : rParameters)
                buf.append("," + rPair.first.toUtf8() + "=" + rPair.second.toUtf8());
            SAL_WARN("vcl.uitest", "missing parameter TEXT to action TYPE "
                << buf.makeStringAndClear());
            throw std::logic_error("missing parameter TEXT to action TYPE");
        }
    }
    else if (rAction == "FOCUS")
    {
        mxWindow->GrabFocus();
    }
    else
    {
        OStringBuffer buf;
        for (auto const & rPair : rParameters)
            buf.append("," + rPair.first.toUtf8() + "=" + rPair.second.toUtf8());
        SAL_WARN("vcl.uitest", "unknown action for " << get_name()
            << ". Action: " << rAction << buf.makeStringAndClear());
        throw std::logic_error("unknown action");
    }
}

OUString WindowUIObject::get_type() const
{
    return get_name();
}

namespace {

vcl::Window* findChild(vcl::Window* pParent, const OUString& rID, bool bRequireVisible = false)
{
    if (!pParent || pParent->IsDisposed())
        return nullptr;

    if (pParent->get_id() == rID)
        return pParent;

    size_t nCount = pParent->GetChildCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        vcl::Window* pChild = pParent->GetChild(i);
        if (pChild && pChild->get_id() == rID
            && (!bRequireVisible || pChild->IsVisible()))
            return pChild;

        if (!bRequireVisible || pChild->IsVisible())
        {
            vcl::Window* pResult = findChild(pChild, rID);
            if (pResult)
                return pResult;
        }
    }

    return nullptr;
}

void addChildren(vcl::Window const * pParent, std::set<OUString>& rChildren)
{
    if (!pParent)
        return;

    size_t nCount = pParent->GetChildCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        vcl::Window* pChild = pParent->GetChild(i);
        if (pChild)
        {
            OUString aId = pChild->get_id();
            if (!aId.isEmpty())
            {
                auto ret = rChildren.insert(aId);
                SAL_WARN_IF(!ret.second, "vcl.uitest", "duplicate ids '" << aId << "' for ui elements. violates locally unique requirement");
            }

            addChildren(pChild, rChildren);
        }
    }
}

}

std::unique_ptr<UIObject> WindowUIObject::get_child(const OUString& rID)
{
    // in a first step try the real children before moving to the top level parent
    // This makes it easier to handle cases with the same ID as there is a way
    // to resolve conflicts
    vcl::Window* pWindow = findChild(mxWindow.get(), rID);
    if (!pWindow)
    {
        vcl::Window* pDialogParent = get_top_parent(mxWindow.get());
        pWindow = findChild(pDialogParent, rID);
    }

    if (!pWindow)
        throw css::uno::RuntimeException("Could not find child with id: " + rID);

    FactoryFunction aFunction = pWindow->GetUITestFactory();
    return aFunction(pWindow);
}

std::unique_ptr<UIObject> WindowUIObject::get_visible_child(const OUString& rID)
{
    // in a first step try the real children before moving to the top level parent
    // This makes it easier to handle cases with the same ID as there is a way
    // to resolve conflicts
    vcl::Window* pWindow = findChild(mxWindow.get(), rID, true);
    if (!pWindow)
    {
        vcl::Window* pDialogParent = get_top_parent(mxWindow.get());
        pWindow = findChild(pDialogParent, rID, true);
    }

    if (!pWindow)
        throw css::uno::RuntimeException("Could not find child with id: " + rID);

    FactoryFunction aFunction = pWindow->GetUITestFactory();
    return aFunction(pWindow);
}

std::set<OUString> WindowUIObject::get_children() const
{
    std::set<OUString> aChildren;
    vcl::Window* pDialogParent = get_top_parent(mxWindow.get());
    if (!pDialogParent->isDisposed())
    {
        aChildren.insert(pDialogParent->get_id());
        addChildren(pDialogParent, aChildren);
    }
    return aChildren;
}

OUString WindowUIObject::get_name() const
{
    return "WindowUIObject";
}

namespace {

OUString escape(const OUString& rStr)
{
    return rStr.replaceAll("\"", "\\\"");
}

}

OUString WindowUIObject::dumpState() const
{
    OUStringBuffer aStateString = "{\"name\":\"" + mxWindow->get_id() + "\"";
    aStateString.append(", \"ImplementationName\":\"").appendAscii(typeid(*mxWindow).name()).append("\"");
    StringMap aState = const_cast<WindowUIObject*>(this)->get_state();
    for (auto const& elem : aState)
    {
        OUString property = ",\"" + elem.first + "\":\"" + escape(elem.second) + "\"";
        aStateString.append(property);
    }

    size_t nCount = mxWindow->GetChildCount();

    if (nCount)
        aStateString.append(",\"children\":[");

    for (size_t i = 0; i < nCount; ++i)
    {
        if (i != 0)
        {
            aStateString.append(",");
        }
        vcl::Window* pChild = mxWindow->GetChild(i);
        std::unique_ptr<UIObject> pChildWrapper =
            pChild->GetUITestFactory()(pChild);
        OUString children = pChildWrapper->dumpState();
        aStateString.append(children);
    }

    if (nCount)
        aStateString.append("]");

    aStateString.append("}");

    OUString aString = aStateString.makeStringAndClear();
    return aString.replaceAll("\n", "\\n");
}

OUString WindowUIObject::dumpHierarchy() const
{
    vcl::Window* pDialogParent = get_top_parent(mxWindow.get());
    std::unique_ptr<UIObject> pParentWrapper =
        pDialogParent->GetUITestFactory()(pDialogParent);
    return pParentWrapper->dumpState();
}

OUString WindowUIObject::get_action(VclEventId nEvent) const
{

    OUString aActionName;
    switch (nEvent)
    {
        case VclEventId::ControlGetFocus:
        case VclEventId::ControlLoseFocus:
            return OUString();

        case VclEventId::ButtonClick:
        case VclEventId::CheckboxToggle:
            aActionName = "CLICK";
        break;

        case VclEventId::EditModify:
            aActionName = "TYPE";
        break;
        default:
            aActionName = OUString::number(static_cast<int>(nEvent));
    }
    return "";
    //return "Action on element: " + mxWindow->get_id() + " with action : " + aActionName;
}

std::unique_ptr<UIObject> WindowUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new WindowUIObject(pWindow));
}

ButtonUIObject::ButtonUIObject(const VclPtr<Button>& xButton):
    WindowUIObject(xButton),
    mxButton(xButton)
{
}

ButtonUIObject::~ButtonUIObject()
{
}

StringMap ButtonUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    // Move that to a Control base class
    aMap["Label"] = mxButton->GetDisplayText();

    return aMap;
}

void ButtonUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        //Click doesn't call toggle when it's a pushbutton tweaked to be a toggle-button
        if (PushButton *pPushButton = (mxButton->GetStyle() & WB_TOGGLE) ? dynamic_cast<PushButton*>(mxButton.get()) : nullptr)
        {
            pPushButton->Check(!pPushButton->IsChecked());
            pPushButton->Toggle();
            return;
        }
        mxButton->Click();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString ButtonUIObject::get_name() const
{
    return "ButtonUIObject";
}

OUString ButtonUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::ButtonClick)
    {
        if(mxButton->get_id()=="writer_all")
        {
            UITestLogger::getInstance().setAppName("writer");
            return "Start writer" ;
        }
        else if(mxButton->get_id()=="calc_all")
        {
            UITestLogger::getInstance().setAppName("calc");
            return "Start calc" ;
        }
        else if(mxButton->get_id()=="impress_all")
        {
            UITestLogger::getInstance().setAppName("impress");
            return "Start impress" ;
        }
        else if(mxButton->get_id()=="draw_all")
        {
            UITestLogger::getInstance().setAppName("draw");
            return "Start draw" ;
        }
        else if(mxButton->get_id()=="math_all")
        {
            UITestLogger::getInstance().setAppName("math");
            return "Start math" ;
        }
        else if(mxButton->get_id()=="database_all")
        {
            UITestLogger::getInstance().setAppName("database");
            return "Start database" ;
        }
        else{
            if (get_top_parent(mxButton)->get_id().isEmpty()){
                //This part because if we don't have parent
                return "Click on '" + mxButton->get_id() ;
            }
            return "Click on '" + mxButton->get_id() + "' from "+
                get_top_parent(mxButton)->get_id();
        }
    }
    else
        return WindowUIObject::get_action(nEvent);
}

std::unique_ptr<UIObject> ButtonUIObject::create(vcl::Window* pWindow)
{
    Button* pButton = dynamic_cast<Button*>(pWindow);
    assert(pButton);
    return std::unique_ptr<UIObject>(new ButtonUIObject(pButton));
}

DialogUIObject::DialogUIObject(const VclPtr<Dialog>& xDialog):
    WindowUIObject(xDialog),
    mxDialog(xDialog)
{
}

DialogUIObject::~DialogUIObject()
{
}

StringMap DialogUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["Modal"] = OUString::boolean(mxDialog->IsModalInputMode());

    return aMap;
}

OUString DialogUIObject::get_name() const
{
    return "DialogUIObject";
}

std::unique_ptr<UIObject> DialogUIObject::create(vcl::Window* pWindow)
{
    Dialog* pDialog = dynamic_cast<Dialog*>(pWindow);
    assert(pDialog);
    return std::unique_ptr<UIObject>(new DialogUIObject(pDialog));
}

EditUIObject::EditUIObject(const VclPtr<Edit>& xEdit):
    WindowUIObject(xEdit),
    mxEdit(xEdit)
{
}

EditUIObject::~EditUIObject()
{
}

void EditUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    bool bHandled = true;
    if (rAction == "TYPE")
    {
        auto it = rParameters.find("TEXT");
        if (it != rParameters.end())
        {
            const OUString& rText = it->second;
            auto aKeyEvents = generate_key_events_from_text(rText);
            for (auto const& keyEvent : aKeyEvents)
            {
                mxEdit->KeyInput(keyEvent);
            }
        }
        else
        {
            bHandled = false;
        }
    }
    else if (rAction == "SET")
    {
        auto it = rParameters.find("TEXT");
        if (it != rParameters.end())
        {
            mxEdit->SetText(it->second);
            mxEdit->Modify();
        }
        else
            bHandled = false;
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find("FROM") != rParameters.end() &&
                rParameters.find("TO") != rParameters.end())
        {
            tools::Long nMin = rParameters.find("FROM")->second.toInt32();
            tools::Long nMax = rParameters.find("TO")->second.toInt32();
            Selection aSelection(nMin, nMax);
            mxEdit->SetSelection(aSelection);
        }
    }
    else if (rAction == "CLEAR")
    {
        mxEdit->SetText("");
        mxEdit->Modify();
        bHandled = true;
    }
    else
    {
        bHandled = false;
    }

    if (!bHandled)
        WindowUIObject::execute(rAction, rParameters);
}

StringMap EditUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["MaxTextLength"] = OUString::number(mxEdit->GetMaxTextLen());
    aMap["QuickHelpText"] = mxEdit->GetQuickHelpText();
    aMap["SelectedText"] = mxEdit->GetSelected();
    aMap["Text"] = mxEdit->GetText();

    return aMap;
}

OUString EditUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::EditSelectionChanged)
    {
        const Selection& rSelection  = mxEdit->GetSelection();
        tools::Long nMin = rSelection.Min();
        tools::Long nMax = rSelection.Max();
        if(get_top_parent(mxEdit)->get_id().isEmpty()){
            //This part because if we don't have parent
            return  "Select in '" +
                mxEdit->get_id() +
                "' {\"FROM\": \"" + OUString::number(nMin) + "\", \"TO\": \"" +
                OUString::number(nMax) + "\"}"
                ;
        }
        return  "Select in '" +
                mxEdit->get_id() +
                "' {\"FROM\": \"" + OUString::number(nMin) + "\", \"TO\": \"" +
                OUString::number(nMax) + "\"} from "
                + get_top_parent(mxEdit)->get_id()
                ;
    }
    else
        return WindowUIObject::get_action(nEvent);
}

OUString EditUIObject::get_name() const
{
    return "EditUIObject";
}

std::unique_ptr<UIObject> EditUIObject::create(vcl::Window* pWindow)
{
    Edit* pEdit = dynamic_cast<Edit*>(pWindow);
    assert(pEdit);
    return std::unique_ptr<UIObject>(new EditUIObject(pEdit));
}

MultiLineEditUIObject::MultiLineEditUIObject(const VclPtr<VclMultiLineEdit>& xEdit):
    WindowUIObject(xEdit),
    mxEdit(xEdit)
{
}

MultiLineEditUIObject::~MultiLineEditUIObject()
{
}

void MultiLineEditUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    bool bHandled = true;
    if (rAction == "TYPE")
    {
        WindowUIObject aChildObj(mxEdit->GetTextWindow());
        aChildObj.execute(rAction, rParameters);
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find("FROM") != rParameters.end() &&
                rParameters.find("TO") != rParameters.end())
        {
            tools::Long nMin = rParameters.find("FROM")->second.toInt32();
            tools::Long nMax = rParameters.find("TO")->second.toInt32();
            Selection aSelection(nMin, nMax);
            mxEdit->SetSelection(aSelection);
        }
    }
    else
    {
        bHandled = false;
    }

    if (!bHandled)
        WindowUIObject::execute(rAction, rParameters);
}

StringMap MultiLineEditUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["MaxTextLength"] = OUString::number(mxEdit->GetMaxTextLen());
    aMap["SelectedText"] = mxEdit->GetSelected();
    aMap["Text"] = mxEdit->GetText();

    return aMap;
}

OUString MultiLineEditUIObject::get_name() const
{
    return "MultiLineEditUIObject";
}

std::unique_ptr<UIObject> MultiLineEditUIObject::create(vcl::Window* pWindow)
{
    VclMultiLineEdit* pEdit = dynamic_cast<VclMultiLineEdit*>(pWindow);
    assert(pEdit);
    return std::unique_ptr<UIObject>(new MultiLineEditUIObject(pEdit));
}

CheckBoxUIObject::CheckBoxUIObject(const VclPtr<CheckBox>& xCheckbox):
    WindowUIObject(xCheckbox),
    mxCheckBox(xCheckbox)
{
}

CheckBoxUIObject::~CheckBoxUIObject()
{
}

void CheckBoxUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        // don't use toggle directly, it does not set the value
        mxCheckBox->ImplCheck();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap CheckBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["Selected"] = OUString::boolean(mxCheckBox->IsChecked());
    aMap["TriStateEnabled"] = OUString::boolean(mxCheckBox->IsTriStateEnabled());
    return aMap;
}

OUString CheckBoxUIObject::get_name() const
{
    return "CheckBoxUIObject";
}

OUString CheckBoxUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::CheckboxToggle)
    {
        if(get_top_parent(mxCheckBox)->get_id().isEmpty()){
            //This part because if we don't have parent
            return "Toggle '" + mxCheckBox->get_id() + "' CheckBox";
        }
        return "Toggle '" + mxCheckBox->get_id() + "' CheckBox from " +
            get_top_parent(mxCheckBox)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

std::unique_ptr<UIObject> CheckBoxUIObject::create(vcl::Window* pWindow)
{
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pWindow);
    assert(pCheckBox);
    return std::unique_ptr<UIObject>(new CheckBoxUIObject(pCheckBox));
}

RadioButtonUIObject::RadioButtonUIObject(const VclPtr<RadioButton>& xRadioButton):
    WindowUIObject(xRadioButton),
    mxRadioButton(xRadioButton)
{
}

RadioButtonUIObject::~RadioButtonUIObject()
{
}

void RadioButtonUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        mxRadioButton->ImplCallClick();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap RadioButtonUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["Checked"] = OUString::boolean(mxRadioButton->IsChecked());

    return aMap;
}

OUString RadioButtonUIObject::get_name() const
{
    return "RadioButtonUIObject";
}

OUString RadioButtonUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::RadiobuttonToggle)
    {
        if(get_top_parent(mxRadioButton)->get_id().isEmpty()){
            //This part because if we don't have parent
            return "Select '" + mxRadioButton->get_id() + "' RadioButton";
        }
        return "Select '" + mxRadioButton->get_id() + "' RadioButton from " +
            get_top_parent(mxRadioButton)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

std::unique_ptr<UIObject> RadioButtonUIObject::create(vcl::Window* pWindow)
{
    RadioButton* pRadioButton = dynamic_cast<RadioButton*>(pWindow);
    assert(pRadioButton);
    return std::unique_ptr<UIObject>(new RadioButtonUIObject(pRadioButton));
}

TabPageUIObject::TabPageUIObject(const VclPtr<TabPage>& xTabPage):
    WindowUIObject(xTabPage),
    mxTabPage(xTabPage)
{
}

TabPageUIObject::~TabPageUIObject()
{
}

void TabPageUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    WindowUIObject::execute(rAction, rParameters);
}

StringMap TabPageUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

OUString TabPageUIObject::get_name() const
{
    return "TabPageUIObject";
}

ListBoxUIObject::ListBoxUIObject(const VclPtr<ListBox>& xListBox):
    WindowUIObject(xListBox),
    mxListBox(xListBox)
{
}

ListBoxUIObject::~ListBoxUIObject()
{
}

void ListBoxUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (!mxListBox->IsEnabled())
        return;

    bool isTiledRendering = comphelper::LibreOfficeKit::isActive();
    if (!isTiledRendering && !mxListBox->IsReallyVisible())
        return;

    if (rAction == "SELECT")
    {
        bool bSelect = true;
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            OUString aVal = itr->second;
            sal_Int32 nPos = aVal.toInt32();
            mxListBox->SelectEntryPos(nPos, bSelect);
        }
        else if (rParameters.find("TEXT") != rParameters.end())
        {
            auto itr = rParameters.find("TEXT");
            OUString aText = itr->second;
            mxListBox->SelectEntry(aText, bSelect);
        }
        mxListBox->Select();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap ListBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["ReadOnly"] = OUString::boolean(mxListBox->IsReadOnly());
    aMap["MultiSelect"] = OUString::boolean(mxListBox->IsMultiSelectionEnabled());
    aMap["EntryCount"] = OUString::number(mxListBox->GetEntryCount());
    aMap["SelectEntryCount"] = OUString::number(mxListBox->GetSelectedEntryCount());
    aMap["SelectEntryPos"] = OUString::number(mxListBox->GetSelectedEntryPos());
    aMap["SelectEntryText"] = mxListBox->GetSelectedEntry();

    return aMap;
}

OUString ListBoxUIObject::get_name() const
{
    return "ListBoxUIObject";
}

OUString ListBoxUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::ListboxSelect)
    {
        sal_Int32 nPos = mxListBox->GetSelectedEntryPos();
        if(get_top_parent(mxListBox)->get_id().isEmpty()){
            //This part because if we don't have parent
            return "Select element with position " + OUString::number(nPos) +
                 " in '" + mxListBox->get_id();
        }
        return "Select element with position " + OUString::number(nPos) +
                 " in '" + mxListBox->get_id() +"' from" + get_top_parent(mxListBox)->get_id() ;
    }
    else if (nEvent == VclEventId::ListboxFocus)
    {
        if(get_top_parent(mxListBox)->get_id().isEmpty())
        {
            //This part because if we don't have parent
            return this->get_type() + " Action:FOCUS Id:" + mxListBox->get_id();
        }
        return this->get_type() + " Action:FOCUS Id:" + mxListBox->get_id() +
            " Parent:" + get_top_parent(mxListBox)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

std::unique_ptr<UIObject> ListBoxUIObject::create(vcl::Window* pWindow)
{
    ListBox* pListBox = dynamic_cast<ListBox*>(pWindow);
    assert(pListBox);
    return std::unique_ptr<UIObject>(new ListBoxUIObject(pListBox));
}

ComboBoxUIObject::ComboBoxUIObject(const VclPtr<ComboBox>& xComboBox):
    WindowUIObject(xComboBox),
    mxComboBox(xComboBox)
{
}

ComboBoxUIObject::~ComboBoxUIObject()
{
}

void ComboBoxUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            OUString aVal = itr->second;
            sal_Int32 nPos = aVal.toInt32();
            mxComboBox->SelectEntryPos(nPos);
        }
        else if(rParameters.find("TEXT") != rParameters.end()){
            auto itr = rParameters.find("TEXT");
            OUString aVal = itr->second;
            sal_Int32 nPos = mxComboBox->GetEntryPos(aVal);
            mxComboBox->SelectEntryPos(nPos);
        }
        mxComboBox->Select();
    }
    else if ( rAction == "TYPE" || rAction == "SET" || rAction == "CLEAR" ){
        if (mxComboBox->GetSubEdit())
        {
            Edit* pEdit = mxComboBox->GetSubEdit();
            std::unique_ptr<UIObject> pObj = EditUIObject::create(pEdit);
            pObj->execute(rAction, rParameters);
        }
        else
            WindowUIObject::execute(rAction, rParameters);
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap ComboBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    return aMap;
}

OUString ComboBoxUIObject::get_name() const
{
    return "ComboBoxUIObject";
}

OUString ComboBoxUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::ComboboxSelect)
    {
        sal_Int32 nPos = mxComboBox->GetSelectedEntryPos();
        if (get_top_parent(mxComboBox)->get_id().isEmpty()){
            //This part because if we don't have parent
            return "Select in '" + mxComboBox->get_id() +
                "' ComboBox item number " + OUString::number(nPos);
        }
        return "Select in '" + mxComboBox->get_id() +
                "' ComboBox item number " + OUString::number(nPos) +
                " from " + get_top_parent(mxComboBox)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

std::unique_ptr<UIObject> ComboBoxUIObject::create(vcl::Window* pWindow)
{
    ComboBox* pComboBox = dynamic_cast<ComboBox*>(pWindow);
    assert(pComboBox);
    return std::unique_ptr<UIObject>(new ComboBoxUIObject(pComboBox));
}

SpinUIObject::SpinUIObject(const VclPtr<SpinButton>& xSpinButton):
    WindowUIObject(xSpinButton),
    mxSpinButton(xSpinButton)
{
}

SpinUIObject::~SpinUIObject()
{
}

void SpinUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "UP")
    {
        mxSpinButton->Up();
    }
    else if (rAction == "DOWN")
    {
        mxSpinButton->Down();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap SpinUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["Min"] = OUString::number(mxSpinButton->GetRangeMin());
    aMap["Max"] = OUString::number(mxSpinButton->GetRangeMax());
    aMap["Step"] = OUString::number(mxSpinButton->GetValueStep());
    aMap["Value"] = OUString::number(mxSpinButton->GetValue());

    return aMap;
}

OUString SpinUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::SpinbuttonUp)
    {
        return this->get_type() + " Action:UP Id:" + mxSpinButton->get_id() +
            " Parent:" + get_top_parent(mxSpinButton)->get_id();
    }
    else if (nEvent == VclEventId::SpinbuttonDown)
    {
        return this->get_type() + " Action:DOWN Id:" + mxSpinButton->get_id() +
            " Parent:" + get_top_parent(mxSpinButton)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

OUString SpinUIObject::get_name() const
{
    return "SpinUIObject";
}

SpinFieldUIObject::SpinFieldUIObject(const VclPtr<SpinField>& xSpinField):
    EditUIObject(xSpinField),
    mxSpinField(xSpinField)
{
}

SpinFieldUIObject::~SpinFieldUIObject()
{
}

void SpinFieldUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "UP")
    {
        mxSpinField->Up();
    }
    else if (rAction == "DOWN")
    {
        mxSpinField->Down();
    }
    else if (rAction == "TYPE")
    {
        if (mxSpinField->GetSubEdit())
        {
            Edit* pSubEdit = mxSpinField->GetSubEdit();
            EditUIObject aSubObject(pSubEdit);
            aSubObject.execute(rAction, rParameters);
        }
    }
    else
        EditUIObject::execute(rAction, rParameters);
}

StringMap SpinFieldUIObject::get_state()
{
    StringMap aMap = EditUIObject::get_state();

    return aMap;
}

OUString SpinFieldUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::SpinfieldUp)
    {
        if(get_top_parent(mxSpinField)->get_id().isEmpty())
        {
            //This part because if we don't have parent
            return "Increase '" + mxSpinField->get_id();
        }
        return "Increase '" + mxSpinField->get_id() +
            "' from " + get_top_parent(mxSpinField)->get_id();
    }
    else if (nEvent == VclEventId::SpinfieldDown)
    {
        if(get_top_parent(mxSpinField)->get_id().isEmpty())
        {
            //This part because if we don't have parent
            return "Decrease '" + mxSpinField->get_id();
        }
        return "Decrease '" + mxSpinField->get_id() +
            "' from " + get_top_parent(mxSpinField)->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

OUString SpinFieldUIObject::get_name() const
{
    return "SpinFieldUIObject";
}

std::unique_ptr<UIObject> SpinFieldUIObject::create(vcl::Window* pWindow)
{
    SpinField* pSpinField = dynamic_cast<SpinField*>(pWindow);
    assert(pSpinField);
    return std::unique_ptr<UIObject>(new SpinFieldUIObject(pSpinField));
}


MetricFieldUIObject::MetricFieldUIObject(const VclPtr<MetricField>& xMetricField):
    SpinFieldUIObject(xMetricField),
    mxMetricField(xMetricField)
{
}

MetricFieldUIObject::~MetricFieldUIObject()
{
}

void MetricFieldUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "VALUE")
    {
        auto itPos = rParameters.find("VALUE");
        if (itPos != rParameters.end())
        {
            mxMetricField->SetValueFromString(itPos->second);
        }
    }
    else
        SpinFieldUIObject::execute(rAction, rParameters);
}

StringMap MetricFieldUIObject::get_state()
{
    StringMap aMap = EditUIObject::get_state();
    aMap["Value"] = mxMetricField->GetValueString();

    return aMap;
}

OUString MetricFieldUIObject::get_name() const
{
    return "MetricFieldUIObject";
}

std::unique_ptr<UIObject> MetricFieldUIObject::create(vcl::Window* pWindow)
{
    MetricField* pMetricField = dynamic_cast<MetricField*>(pWindow);
    assert(pMetricField);
    return std::unique_ptr<UIObject>(new MetricFieldUIObject(pMetricField));
}

FormattedFieldUIObject::FormattedFieldUIObject(const VclPtr<FormattedField>& xFormattedField):
    SpinFieldUIObject(xFormattedField),
    mxFormattedField(xFormattedField)
{
}

FormattedFieldUIObject::~FormattedFieldUIObject()
{
}

void FormattedFieldUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "VALUE")
    {
        auto itPos = rParameters.find("VALUE");
        if (itPos != rParameters.end())
        {
            mxFormattedField->SetValueFromString(itPos->second);
        }
    }
    else
        SpinFieldUIObject::execute(rAction, rParameters);
}

StringMap FormattedFieldUIObject::get_state()
{
    StringMap aMap = EditUIObject::get_state();
    aMap["Value"] = OUString::number(mxFormattedField->GetFormatter().GetValue());

    return aMap;
}

OUString FormattedFieldUIObject::get_name() const
{
    return "FormattedFieldUIObject";
}

std::unique_ptr<UIObject> FormattedFieldUIObject::create(vcl::Window* pWindow)
{
    FormattedField* pFormattedField = dynamic_cast<FormattedField*>(pWindow);
    assert(pFormattedField);
    return std::unique_ptr<UIObject>(new FormattedFieldUIObject(pFormattedField));
}

TabControlUIObject::TabControlUIObject(const VclPtr<TabControl>& xTabControl):
    WindowUIObject(xTabControl),
    mxTabControl(xTabControl)
{
}

TabControlUIObject::~TabControlUIObject()
{
}

void TabControlUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            sal_uInt32 nPos = itr->second.toUInt32();
            std::vector<sal_uInt16> aIds = mxTabControl->GetPageIDs();
            mxTabControl->SelectTabPage(aIds[nPos]);
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap TabControlUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["PageCount"] = OUString::number(mxTabControl->GetPageCount());

    sal_uInt16 nPageId = mxTabControl->GetCurPageId();
    aMap["CurrPageId"] = OUString::number(nPageId);
    aMap["CurrPagePos"] = OUString::number(mxTabControl->GetPagePos(nPageId));

    return aMap;
}

OUString TabControlUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::TabpageActivate)
    {
        sal_Int32 nPageId = mxTabControl->GetCurPageId();

        if(get_top_parent(mxTabControl)->get_id().isEmpty()){
            //This part because if we don't have parent
            return "Choose Tab number " + OUString::number(mxTabControl->GetPagePos(nPageId)) +
                " in '" + mxTabControl->get_id();
        }
        return "Choose Tab number " + OUString::number(mxTabControl->GetPagePos(nPageId)) +
                " in '" + mxTabControl->get_id()+
                "' from " + get_top_parent(mxTabControl)->get_id() ;
    }
    else
        return WindowUIObject::get_action(nEvent);
}

OUString TabControlUIObject::get_name() const
{
    return "TabControlUIObject";
}

std::unique_ptr<UIObject> TabControlUIObject::create(vcl::Window* pWindow)
{
    TabControl* pTabControl = dynamic_cast<TabControl*>(pWindow);
    assert(pTabControl);
    return std::unique_ptr<UIObject>(new TabControlUIObject(pTabControl));
}

RoadmapWizardUIObject::RoadmapWizardUIObject(const VclPtr<vcl::RoadmapWizard>& xRoadmapWizard):
    WindowUIObject(xRoadmapWizard),
    mxRoadmapWizard(xRoadmapWizard)
{
}

RoadmapWizardUIObject::~RoadmapWizardUIObject()
{
}

void RoadmapWizardUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            sal_uInt32 nPos = itr->second.toUInt32();
            mxRoadmapWizard->SelectRoadmapItemByID(nPos);
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap RoadmapWizardUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    aMap["CurrentStep"] = OUString::number(mxRoadmapWizard->GetCurrentRoadmapItemID());

    return aMap;
}

OUString RoadmapWizardUIObject::get_name() const
{
    return "RoadmapWizardUIObject";
}

std::unique_ptr<UIObject> RoadmapWizardUIObject::create(vcl::Window* pWindow)
{
    vcl::RoadmapWizard* pRoadmapWizard = dynamic_cast<vcl::RoadmapWizard*>(pWindow);
    assert(pRoadmapWizard);
    return std::unique_ptr<UIObject>(new RoadmapWizardUIObject(pRoadmapWizard));
}

VerticalTabControlUIObject::VerticalTabControlUIObject(const VclPtr<VerticalTabControl>& xTabControl):
    WindowUIObject(xTabControl),
    mxTabControl(xTabControl)
{
}

VerticalTabControlUIObject::~VerticalTabControlUIObject()
{
}

void VerticalTabControlUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            sal_uInt32 nPos = itr->second.toUInt32();
            OString xid = mxTabControl->GetPageId(nPos);
            mxTabControl->SetCurPageId(xid);
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

StringMap VerticalTabControlUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["PageCount"] = OUString::number(mxTabControl->GetPageCount());

    OString nPageId = mxTabControl->GetCurPageId();
    aMap["CurrPageTitel"] = mxTabControl->GetPageText(nPageId);
    aMap["CurrPagePos"] = OUString::number(mxTabControl->GetPagePos(nPageId));

    return aMap;
}

OUString VerticalTabControlUIObject::get_name() const
{
    return "VerticalTabControlUIObject";
}

std::unique_ptr<UIObject> VerticalTabControlUIObject::create(vcl::Window* pWindow)
{
    VerticalTabControl* pTabControl = dynamic_cast<VerticalTabControl*>(pWindow);
    assert(pTabControl);
    return std::unique_ptr<UIObject>(new VerticalTabControlUIObject(pTabControl));
}


ToolBoxUIObject::ToolBoxUIObject(const VclPtr<ToolBox>& xToolBox):
    WindowUIObject(xToolBox),
    mxToolBox(xToolBox)
{
}

ToolBoxUIObject::~ToolBoxUIObject()
{
}

void ToolBoxUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            auto itr = rParameters.find("POS");
            sal_uInt16 nPos = itr->second.toUInt32();
            mxToolBox->SetCurItemId(mxToolBox->GetItemId(nPos));
            mxToolBox->Click();
            mxToolBox->Select();
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString ToolBoxUIObject::get_action(VclEventId nEvent) const
{
    if (nEvent == VclEventId::ToolboxClick)
    {
        return "Click on item number " + OUString::number(sal_uInt16(mxToolBox->GetCurItemId())) +
                " in " + mxToolBox->get_id();
    }
    else
        return WindowUIObject::get_action(nEvent);
}

StringMap ToolBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["CurrSelectedItemID"] = OUString::number(sal_uInt16(mxToolBox->GetCurItemId()));
    aMap["CurrSelectedItemText"] = mxToolBox->GetItemText(mxToolBox->GetCurItemId());
    aMap["CurrSelectedItemCommand"] = mxToolBox->GetItemCommand(mxToolBox->GetCurItemId());
    aMap["ItemCount"] = OUString::number(mxToolBox->GetItemCount());
    return aMap;
}

OUString ToolBoxUIObject::get_name() const
{
    return "ToolBoxUIObject";
}

std::unique_ptr<UIObject> ToolBoxUIObject::create(vcl::Window* pWindow)
{
    ToolBox* pToolBox = dynamic_cast<ToolBox*>(pWindow);
    assert(pToolBox);
    return std::unique_ptr<UIObject>(new ToolBoxUIObject(pToolBox));
}

MenuButtonUIObject::MenuButtonUIObject(const VclPtr<MenuButton>& xMenuButton):
    WindowUIObject(xMenuButton),
    mxMenuButton(xMenuButton)
{
}

MenuButtonUIObject::~MenuButtonUIObject()
{
}

StringMap MenuButtonUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["Label"] = mxMenuButton->GetDisplayText();
    aMap["CurrentItem"] = OUString::createFromAscii(mxMenuButton->GetCurItemIdent());
    return aMap;
}

void MenuButtonUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        mxMenuButton->Check(!mxMenuButton->IsChecked());
        mxMenuButton->Toggle();
    }
    else if (rAction == "OPENLIST")
    {
        mxMenuButton->ExecuteMenu();
    }
    else if (rAction == "OPENFROMLIST")
    {
        auto itr = rParameters.find("POS");
        sal_uInt32 nPos = itr->second.toUInt32();

        sal_uInt32 nId = mxMenuButton->GetPopupMenu()->GetItemId(nPos);
        mxMenuButton->GetPopupMenu()->SetSelectedEntry(nId);
        mxMenuButton->SetCurItemId();
        mxMenuButton->Select();
    }
    else if (rAction == "CLOSELIST")
    {
        mxMenuButton->GetPopupMenu()->EndExecute();
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString MenuButtonUIObject::get_name() const
{
    return "MenuButtonUIObject";
}

std::unique_ptr<UIObject> MenuButtonUIObject::create(vcl::Window* pWindow)
{
    MenuButton* pMenuButton = dynamic_cast<MenuButton*>(pWindow);
    assert(pMenuButton);
    return std::unique_ptr<UIObject>(new MenuButtonUIObject(pMenuButton));
}

DrawingAreaUIObject::DrawingAreaUIObject(const VclPtr<vcl::Window>& rDrawingArea)
    : WindowUIObject(rDrawingArea)
    , mxDrawingArea(dynamic_cast<VclDrawingArea*>(rDrawingArea.get()))
{
    assert(mxDrawingArea);
    mpController = static_cast<weld::CustomWidgetController*>(mxDrawingArea->GetUserData());
}

DrawingAreaUIObject::~DrawingAreaUIObject()
{
}

void DrawingAreaUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "CLICK")
    {
        // POSX and POSY are percentage of width/height dimensions
        if (rParameters.find("POSX") != rParameters.end() &&
            rParameters.find("POSY") != rParameters.end())
        {
            auto aPosX = rParameters.find("POSX");
            auto aPosY = rParameters.find("POSY");

            OString sPosX2 = OUStringToOString(aPosX->second, RTL_TEXTENCODING_ASCII_US);
            OString sPoxY2 = OUStringToOString(aPosY->second, RTL_TEXTENCODING_ASCII_US);

            if (!sPosX2.isEmpty() && !sPoxY2.isEmpty())
            {
                double fPosX = std::atof(sPosX2.getStr());
                double fPosY = std::atof(sPoxY2.getStr());

                fPosX = fPosX * mxDrawingArea->GetOutputWidthPixel();
                fPosY = fPosY * mxDrawingArea->GetOutputHeightPixel();

                MouseEvent aEvent(Point(fPosX, fPosY), 1, MouseEventModifiers::NONE, MOUSE_LEFT, 0);
                mxDrawingArea->MouseButtonDown(aEvent);
                mxDrawingArea->MouseButtonUp(aEvent);
            }
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> DrawingAreaUIObject::create(vcl::Window* pWindow)
{
    VclDrawingArea* pVclDrawingArea = dynamic_cast<VclDrawingArea*>(pWindow);
    assert(pVclDrawingArea);
    return std::unique_ptr<UIObject>(new DrawingAreaUIObject(pVclDrawingArea));
}

IconViewUIObject::IconViewUIObject(const VclPtr<SvTreeListBox>& xIconView):
    TreeListUIObject(xIconView)
{
}

StringMap IconViewUIObject::get_state()
{
    StringMap aMap = TreeListUIObject::get_state();

    SvTreeListEntry* pEntry = mxTreeList->FirstSelected();

    OUString* pId = static_cast<OUString*>(pEntry->GetUserData());
    if (pId)
        aMap["SelectedItemId"] = *pId;

    SvTreeList* pModel = mxTreeList->GetModel();
    if (pModel)
        aMap["SelectedItemPos"] = OUString::number(pModel->GetAbsPos(pEntry));

    return aMap;
}

OUString IconViewUIObject::get_name() const
{
    return "IconViewUIObject";
}

std::unique_ptr<UIObject> IconViewUIObject::create(vcl::Window* pWindow)
{
    SvTreeListBox* pTreeList = dynamic_cast<SvTreeListBox*>(pWindow);
    assert(pTreeList);
    return std::unique_ptr<UIObject>(new IconViewUIObject(pTreeList));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
