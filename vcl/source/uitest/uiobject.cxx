/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>

#include <vcl/event.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/spin.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>

#include <comphelper/string.hxx>

#include <rtl/ustrbuf.hxx>

#include <iostream>
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
    return OUString("Generic UIObject");
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

namespace {

bool isDialogWindow(vcl::Window* pWindow)
{
    WindowType nType = pWindow->GetType();
    // DIALOG to FONTDIALOG
    if (nType >= 0x13a && nType <= 0x143)
        return true;

    // MESSBOX, INFOBOX, QUERYBOX, WARNINGBOX, ERRORBOX
    if (nType >= 0x130 && nType <= 0x134)
        return true;

    if (nType == WINDOW_TABDIALOG)
        return true;

    return false;
}

vcl::Window* get_dialog_parent(vcl::Window* pWindow)
{
    if (isDialogWindow(pWindow))
        return pWindow;

    vcl::Window* pParent = pWindow->GetParent();
    if (!pParent)
        return pWindow;

    return get_dialog_parent(pParent);
}

std::vector<KeyEvent> generate_key_events_from_text(const OUString& rStr)
{
    std::vector<KeyEvent> aEvents;
    vcl::KeyCode aCode;
    for (sal_Int32 i = 0, n = rStr.getLength();
            i != n; ++i)
    {
        aEvents.push_back(KeyEvent(rStr[i], aCode));
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
    for (auto itr = aTokens.begin(), itrEnd = aTokens.end(); itr != itrEnd; ++itr)
    {
        OUString aToken = itr->trim();
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
        aEvents.push_back(KeyEvent(0, aCode));
    }
    else if (aKeyMap.find(aRemainingText) != aKeyMap.end())
    {
        sal_uInt16 nKey = aKeyMap[aRemainingText];
        vcl::KeyCode aCode(nKey, bShift, bMod1, bMod2, false);
        aEvents.push_back(KeyEvent( 'a', aCode));
    }
    else
    {
        for (sal_Int32 i = 0; i < aRemainingText.getLength(); ++i)
        {
            bool bShiftThroughKey = false;
            sal_uInt16 nKey = get_key(aRemainingText[i], bShiftThroughKey);
            vcl::KeyCode aCode(nKey, bShift || bShiftThroughKey, bMod1, bMod2, false);
            aEvents.push_back(KeyEvent(aRemainingText[i], aCode));
        }
    }

    return aEvents;
}

OUString to_string(const Point& rPos)
{
    OUStringBuffer aBuffer;
    aBuffer.append(OUString::number(rPos.X()));
    aBuffer.append("x");
    aBuffer.append(OUString::number(rPos.Y()));

    return aBuffer.makeStringAndClear();
}

OUString to_string(const Size& rSize)
{
    OUStringBuffer aBuffer;
    aBuffer.append(rSize.Width());
    aBuffer.append("x");
    aBuffer.append(rSize.Height());

    return aBuffer.makeStringAndClear();
}

}

WindowUIObject::WindowUIObject(VclPtr<vcl::Window> xWindow):
    mxWindow(xWindow)
{
}

StringMap WindowUIObject::get_state()
{
    StringMap aMap;
    aMap["Visible"] = OUString::boolean(mxWindow->IsVisible());
    aMap["ReallyVisible"] = OUString::boolean(mxWindow->IsReallyVisible());
    aMap["Enabled"] = OUString::boolean(mxWindow->IsEnabled());
    aMap["WindowType"] = OUString::number(mxWindow->GetType(), 16);

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
    bool bHandled = true;
    if (rAction == "SET")
    {
        for (auto itr = rParameters.begin(); itr != rParameters.end(); ++itr)
        {
            std::cout << itr->first;
        }
    }
    else if (rAction == "TYPE")
    {
        auto it = rParameters.find("TEXT");
        if (it != rParameters.end())
        {
            const OUString& rText = it->second;
            auto aKeyEvents = generate_key_events_from_text(rText);
            for (auto itr = aKeyEvents.begin(), itrEnd = aKeyEvents.end();
                    itr != itrEnd; ++itr)
            {
                mxWindow->KeyInput(*itr);
            }
        }
        else if (rParameters.find("KEYCODE") != rParameters.end())
        {
            auto itr = rParameters.find("KEYCODE");
            const OUString rText = itr->second;
            auto aKeyEvents = generate_key_events_from_keycode(rText);
            for (auto itrKey = aKeyEvents.begin(), itrKeyEnd = aKeyEvents.end();
                    itrKey != itrKeyEnd; ++itrKey)
            {
                mxWindow->KeyInput(*itrKey);
            }
        }
        else
        {
            SAL_WARN("vcl.uitest", "missing parameter TEXT to action TYPE");
            return;
        }
    }
    else
    {
        bHandled = false;
    }

    if (!bHandled)
    {
        SAL_WARN("vcl.uitest", "unknown action or parameter for " << get_name() << ". Action: " << rAction);
    }
}

OUString WindowUIObject::get_type() const
{
    return get_name();
}

namespace {

vcl::Window* findChild(vcl::Window* pParent, const OUString& rID)
{
    if (!pParent)
        return nullptr;

    size_t nCount = pParent->GetChildCount();
    for (size_t i = 0; i < nCount; ++i)
    {
        vcl::Window* pChild = pParent->GetChild(i);
        if (pChild && pChild->get_id() == rID)
            return pChild;

        vcl::Window* pResult = findChild(pChild, rID);
        if (pResult)
            return pResult;
    }

    return nullptr;
}

void addChildren(vcl::Window* pParent, std::set<OUString>& rChildren)
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
                SAL_WARN_IF(!ret.second, "vcl.uitest", "duplicate ids for ui elements. violates locally unique requirement");
            }

            addChildren(pChild, rChildren);
        }
    }
}

}

std::unique_ptr<UIObject> WindowUIObject::get_child(const OUString& rID)
{
    vcl::Window* pDialogParent = get_dialog_parent(mxWindow.get());
    vcl::Window* pWindow = findChild(pDialogParent, rID);

    if (!pWindow)
        throw css::uno::RuntimeException("Could not find child with id: " + rID);

    FactoryFunction aFunction = pWindow->GetUITestFactory();
    return aFunction(pWindow);
}

std::set<OUString> WindowUIObject::get_children() const
{
    vcl::Window* pDialogParent = get_dialog_parent(mxWindow.get());
    std::set<OUString> aChildren;
    aChildren.insert(pDialogParent->get_id());
    addChildren(pDialogParent, aChildren);
    return aChildren;
}

OUString WindowUIObject::get_name() const
{
    return OUString("WindowUIObject");
}

OUString WindowUIObject::dumpState() const
{
    OUStringBuffer aStateString = "{\"name\":\"" + mxWindow->get_id() + "\"";
    aStateString.append(", \"ImplementationName\":\"").appendAscii(typeid(*mxWindow.get()).name()).append("\"");
    StringMap aState = const_cast<WindowUIObject*>(this)->get_state();
    for (auto itr = aState.begin(), itrEnd = aState.end(); itr != itrEnd; ++itr)
    {
        OUString property = ",\"" + itr->first + "\":\"" + itr->second + "\"";
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
    vcl::Window* pDialogParent = get_dialog_parent(mxWindow.get());
    std::unique_ptr<UIObject> pParentWrapper =
        pDialogParent->GetUITestFactory()(pDialogParent);
    return pParentWrapper->dumpState();
}

std::unique_ptr<UIObject> WindowUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new WindowUIObject(pWindow));
}

ButtonUIObject::ButtonUIObject(VclPtr<Button> xButton):
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
    // Move that to a Contrl base class
    aMap["Label"] = mxButton->GetDisplayText();

    return aMap;
}

void ButtonUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CLICK")
        mxButton->Click();
    else
        WindowUIObject::execute(rAction, rParameters);
}

OUString ButtonUIObject::get_name() const
{
    return OUString("ButtonUIObject");
}

std::unique_ptr<UIObject> ButtonUIObject::create(vcl::Window* pWindow)
{
    Button* pButton = dynamic_cast<Button*>(pWindow);
    assert(pButton);
    return std::unique_ptr<UIObject>(new ButtonUIObject(pButton));
}

DialogUIObject::DialogUIObject(VclPtr<Dialog> xDialog):
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
    return OUString("DialogUIObject");
}

std::unique_ptr<UIObject> DialogUIObject::create(vcl::Window* pWindow)
{
    Dialog* pDialog = dynamic_cast<Dialog*>(pWindow);
    assert(pDialog);
    return std::unique_ptr<UIObject>(new DialogUIObject(pDialog));
}

EditUIObject::EditUIObject(VclPtr<Edit> xEdit):
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
    if (rAction == "SET")
    {
        if (rParameters.find("TEXT") != rParameters.end())
        {
            auto it = rParameters.find("TEXT");
            if (it == rParameters.end())
            {
                SAL_WARN("vcl.uitest", "missing parameter TEXT to action SET");
                return;
            }

            const OUString& rText = it->second;
            auto aKeyEvents = generate_key_events_from_text(rText);
            for (auto itr = aKeyEvents.begin(), itrEnd = aKeyEvents.end();
                    itr != itrEnd; ++itr)
            {
                mxEdit->KeyInput(*itr);
            }
        }
        else if (rParameters.find("SELECTION") != rParameters.end())
        {
            // TODO: moggi: add code
        }
        else
        {
            bHandled = false;
        }
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
    aMap["SelectedText"] = mxEdit->GetSelected();
    aMap["Text"] = mxEdit->GetText();

    return aMap;
}

OUString EditUIObject::get_name() const
{
    return OUString("EditUIObject");
}

std::unique_ptr<UIObject> EditUIObject::create(vcl::Window* pWindow)
{
    Edit* pEdit = dynamic_cast<Edit*>(pWindow);
    assert(pEdit);
    return std::unique_ptr<UIObject>(new EditUIObject(pEdit));
}

CheckBoxUIObject::CheckBoxUIObject(VclPtr<CheckBox> xCheckbox):
    WindowUIObject(xCheckbox),
    mxCheckBox(xCheckbox)
{
}

CheckBoxUIObject::~CheckBoxUIObject()
{
}

void CheckBoxUIObject::execute(const OUString& rAction,
        const StringMap& /*rParameters*/)
{
    if (rAction == "CLICK")
    {
        // don't use toggle directly, it does not set the value
        mxCheckBox->ImplCheck();
    }
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
    return OUString("CheckBoxUIObject");
}

std::unique_ptr<UIObject> CheckBoxUIObject::create(vcl::Window* pWindow)
{
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pWindow);
    assert(pCheckBox);
    return std::unique_ptr<UIObject>(new CheckBoxUIObject(pCheckBox));
}

RadioButtonUIObject::RadioButtonUIObject(VclPtr<RadioButton> xRadioButton):
    WindowUIObject(xRadioButton),
    mxRadioButton(xRadioButton)
{
}

RadioButtonUIObject::~RadioButtonUIObject()
{
}

void RadioButtonUIObject::execute(const OUString& rAction,
        const StringMap& /*rParameters*/)
{
    if (rAction == "CLICK")
    {
        mxRadioButton->ImplCallClick();
    }
}

StringMap RadioButtonUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

OUString RadioButtonUIObject::get_name() const
{
    return OUString("RadioButtonUIObject");
}

std::unique_ptr<UIObject> RadioButtonUIObject::create(vcl::Window* pWindow)
{
    RadioButton* pRadioButton = dynamic_cast<RadioButton*>(pWindow);
    assert(pRadioButton);
    return std::unique_ptr<UIObject>(new RadioButtonUIObject(pRadioButton));
}

TabPageUIObject::TabPageUIObject(VclPtr<TabPage> xTabPage):
    WindowUIObject(xTabPage),
    mxTabPage(xTabPage)
{
}

TabPageUIObject::~TabPageUIObject()
{
}

void TabPageUIObject::execute(const OUString& rAction,
        const StringMap& /*rParameters*/)
{
    if (rAction == "SELECT")
    {

    }
}

StringMap TabPageUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

OUString TabPageUIObject::get_name() const
{
    return OUString("TabPageUIObject");
}

ListBoxUIObject::ListBoxUIObject(VclPtr<ListBox> xListBox):
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
    if (!mxListBox->IsEnabled() || !mxListBox->IsReallyVisible())
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
    aMap["SelectEntryCount"] = OUString::number(mxListBox->GetSelectEntryCount());
    aMap["SelectEntryPos"] = OUString::number(mxListBox->GetSelectEntryPos());
    aMap["SelectEntryText"] = mxListBox->GetSelectEntry();

    return aMap;
}

OUString ListBoxUIObject::get_name() const
{
    return OUString("ListBoxUIObject");
}

std::unique_ptr<UIObject> ListBoxUIObject::create(vcl::Window* pWindow)
{
    ListBox* pListBox = dynamic_cast<ListBox*>(pWindow);
    assert(pListBox);
    return std::unique_ptr<UIObject>(new ListBoxUIObject(pListBox));
}

ComboBoxUIObject::ComboBoxUIObject(VclPtr<ComboBox> xComboBox):
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
        mxComboBox->Select();
    }
    else if (rAction == "TYPE")
    {
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
    return OUString("ComboBoxUIObject");
}

std::unique_ptr<UIObject> ComboBoxUIObject::create(vcl::Window* pWindow)
{
    ComboBox* pComboBox = dynamic_cast<ComboBox*>(pWindow);
    assert(pComboBox);
    return std::unique_ptr<UIObject>(new ComboBoxUIObject(pComboBox));
}

SpinUIObject::SpinUIObject(VclPtr<SpinButton> xSpinButton):
    WindowUIObject(xSpinButton),
    mxSpinButton(xSpinButton)
{
}

SpinUIObject::~SpinUIObject()
{
}

void SpinUIObject::execute(const OUString& rAction,
        const StringMap& /*rParameters*/)
{
    if (rAction == "UP")
    {
        /* code */
    }
    else if (rAction == "DOWN")
    {
    }
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

OUString SpinUIObject::get_name() const
{
    return OUString("SpinUIObject");
}

SpinFieldUIObject::SpinFieldUIObject(VclPtr<SpinField> xSpinField):
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

OUString SpinFieldUIObject::get_name() const
{
    return OUString("SpinFieldUIObject");
}

std::unique_ptr<UIObject> SpinFieldUIObject::create(vcl::Window* pWindow)
{
    SpinField* pSpinField = dynamic_cast<SpinField*>(pWindow);
    assert(pSpinField);
    return std::unique_ptr<UIObject>(new SpinFieldUIObject(pSpinField));
}

TabControlUIObject::TabControlUIObject(VclPtr<TabControl> xTabControl):
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

    return aMap;
}

OUString TabControlUIObject::get_name() const
{
    return OUString("TabControlUIObject");
}

std::unique_ptr<UIObject> TabControlUIObject::create(vcl::Window* pWindow)
{
    TabControl* pTabControl = dynamic_cast<TabControl*>(pWindow);
    assert(pTabControl);
    return std::unique_ptr<UIObject>(new TabControlUIObject(pTabControl));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
