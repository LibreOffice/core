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
    if (nType >= 0x13a && nType <= 0x13c)
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

}

WindowUIObject::WindowUIObject(VclPtr<vcl::Window> xWindow):
    mxWindow(xWindow)
{
}

StringMap WindowUIObject::get_state()
{
    StringMap aMap;
    aMap["Visible"] = OUString::boolean(mxWindow->IsVisible());
    aMap["Enabled"] = OUString::boolean(mxWindow->IsEnabled());
    aMap["WindowType"] = OUString::number(mxWindow->GetType(), 16);
    if (mxWindow->GetParent())
        aMap["Parent"] = mxWindow->GetParent()->get_id();

    return aMap;
}

void WindowUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SET")
    {
        for (auto itr = rParameters.begin(); itr != rParameters.end(); ++itr)
        {
            std::cout << itr->first;
        }
    }
    else if (rAction == "TYPE")
    {
        assert(rParameters.find("TEXT") != rParameters.end());
        const OUString& rText = rParameters.find("TEXT")->second;
        auto aKeyEvents = generate_key_events_from_text(rText);
        for (auto itr = aKeyEvents.begin(), itrEnd = aKeyEvents.end();
                itr != itrEnd; ++itr)
        {
            mxWindow->KeyInput(*itr);
        }
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
    if (rAction == "SET")
    {
        if (rParameters.find("TEXT") != rParameters.end())
        {
            assert(rParameters.size() == 1); // only the text
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
            SAL_WARN("vcl.uitest", "unkown set parameters for EditUIObject");
    }
    else
    {
        SAL_WARN("vcl.uitest", "unknown action for EditUIObject: " << rAction);
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
