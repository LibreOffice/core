/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uitest/uiobject_impl.hxx"
#include "uitest/factory.hxx"

#include <vcl/event.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/spin.hxx>

#include <rtl/ustrbuf.hxx>

#include <iostream>
#include <vector>

#define DUMP_UITEST(x) SAL_INFO("vcl.uitest", x)

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

UIObjectType UIObject::get_type() const
{
    return UIObjectType::UNKNOWN;
}

std::unique_ptr<UIObject> UIObject::get_child(const OUString&)
{
    return std::unique_ptr<UIObject>();
}

void UIObject::dumpState() const
{
}

void UIObject::dumpHierarchy() const
{
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
        auto itr = rParameters.find("TEXT");
        if (itr == rParameters.end())
        {
            SAL_WARN("vcl.uitest", "missing parameter TEXT to action TYPE");
            return;
        }

        const OUString& rText = itr->second;
        auto aKeyEvents = generate_key_events_from_text(rText);
        for (auto itr = aKeyEvents.begin(), itrEnd = aKeyEvents.end();
                itr != itrEnd; ++itr)
        {
            mxWindow->KeyInput(*itr);
        }
    }
    else
    {
        bHandled = false;
    }

    if (!bHandled)
    {
        SAL_WARN("vcl.uitest", "unkown action or parameter for " << get_name() << ". Action: " << rAction);
    }
}

UIObjectType WindowUIObject::get_type() const
{
    return UIObjectType::WINDOW;
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

}

std::unique_ptr<UIObject> WindowUIObject::get_child(const OUString& rID)
{
    vcl::Window* pDialogParent = get_dialog_parent(mxWindow.get());
    vcl::Window* pWindow = findChild(pDialogParent, rID);

    return UITestWrapperFactory::createObject(pWindow);
}

OUString WindowUIObject::get_name() const
{
    return OUString("WindowUIObject");
}

void WindowUIObject::dumpState() const
{
    DUMP_UITEST(get_name() << " " << mxWindow->get_id());
    StringMap aState = const_cast<WindowUIObject*>(this)->get_state();
    for (auto itr = aState.begin(), itrEnd = aState.end(); itr != itrEnd; ++itr)
    {
        DUMP_UITEST("Property: " << itr->first << " with value: " << itr->second);
    }
    size_t nCount = mxWindow->GetChildCount();
    if (nCount)
        DUMP_UITEST("With " << nCount << " Children:");

    for (size_t i = 0; i < nCount; ++i)
    {
        vcl::Window* pChild = mxWindow->GetChild(i);
        // TODO: moggi: we need to go through a factory for the new objects
        std::unique_ptr<UIObject> pChildWrapper =
            UITestWrapperFactory::createObject(pChild);
        pChildWrapper->dumpState();
    }
}

void WindowUIObject::dumpHierarchy() const
{
    vcl::Window* pDialogParent = get_dialog_parent(mxWindow.get());
    std::unique_ptr<UIObject> pParentWrapper =
        UITestWrapperFactory::createObject(pDialogParent);
    pParentWrapper->dumpState();
}

ButtonUIObject::ButtonUIObject(VclPtr<Button> xButton):
    WindowUIObject(xButton),
    mxButton(xButton)
{
}

StringMap ButtonUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    // Move that to a Contrl base class
    aMap["Label"] = mxButton->GetDisplayText();

    return aMap;
}

UIObjectType ButtonUIObject::get_type() const
{
    return UIObjectType::BUTTON;
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

DialogUIObject::DialogUIObject(VclPtr<Dialog> xDialog):
    WindowUIObject(xDialog),
    mxDialog(xDialog)
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

UIObjectType DialogUIObject::get_type() const
{
    return UIObjectType::DIALOG;
}

EditUIObject::EditUIObject(VclPtr<Edit> xEdit):
    WindowUIObject(xEdit),
    mxEdit(xEdit)
{
}

void EditUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    bool bHandled = true;;
    if (rAction == "SET")
    {
        if (rParameters.find("TEXT") != rParameters.end())
        {
            auto itr = rParameters.find("TEXT");
            if (itr == rParameters.end())
            {
                SAL_WARN("vcl.uitest", "missing parameter TEXT to action SET");
                return;
            }

            const OUString& rText = rParameters.find("TEXT")->second;
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

UIObjectType EditUIObject::get_type() const
{
    return UIObjectType::EDIT;
}

OUString EditUIObject::get_name() const
{
    return OUString("EditUIObject");
}

CheckBoxUIObject::CheckBoxUIObject(VclPtr<CheckBox> xCheckbox):
    WindowUIObject(xCheckbox),
    mxCheckBox(xCheckbox)
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

UIObjectType CheckBoxUIObject::get_type() const
{
    return UIObjectType::CHECKBOX;
}

OUString CheckBoxUIObject::get_name() const
{
    return OUString("CheckBoxUIObject");
}

TabPageUIObject::TabPageUIObject(VclPtr<TabPage> xTabPage):
    WindowUIObject(xTabPage),
    mxTabPage(xTabPage)
{
}

void TabPageUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{

}

StringMap TabPageUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

UIObjectType TabPageUIObject::get_type() const
{
    return UIObjectType::TABPAGE;
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
}

StringMap ListBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["ReadOnly"] = OUString::boolean(mxListBox->IsReadOnly());
    aMap["MultiSelect"] = OUString::boolean(mxListBox->IsMultiSelectionEnabled());
    aMap["EntryCount"] = OUString::number(mxListBox->GetEntryCount());
    aMap["SelectEntryCount"] = OUString::number(mxListBox->GetSelectEntryCount());

    return aMap;
}

UIObjectType ListBoxUIObject::get_type() const
{
    return UIObjectType::LISTBOX;
}

OUString ListBoxUIObject::get_name() const
{
    return OUString("ListBoxUIObject");
}

ComboBoxUIObject::ComboBoxUIObject(VclPtr<ComboBox> xComboBox):
    WindowUIObject(xComboBox),
    mxComboBox(xComboBox)
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
}

StringMap ComboBoxUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();

    return aMap;
}

UIObjectType ComboBoxUIObject::get_type() const
{
    return UIObjectType::COMBOBOX;
}

OUString ComboBoxUIObject::get_name() const
{
    return OUString("ComboBoxUIObject");
}

SpinUIObject::SpinUIObject(VclPtr<SpinButton> xSpinButton):
    WindowUIObject(xSpinButton),
    mxSpinButton(xSpinButton)
{
}

void SpinUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
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

UIObjectType SpinUIObject::get_type() const
{
    return UIObjectType::SPINBUTTON;
}

OUString SpinUIObject::get_name() const
{
    return OUString("SpinUIObject");
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
