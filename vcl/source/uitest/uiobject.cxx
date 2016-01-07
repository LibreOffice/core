/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/uitest/uiobject.hxx>

#include <iostream>

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

UIObjectType UIObject::getType() const
{
    return UIObjectType::UNKNOWN;
}

std::unique_ptr<UIObject> UIObject::get_child(const OUString&)
{
    return std::unique_ptr<UIObject>();
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
}

UIObjectType WindowUIObject::getType() const
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
    vcl::Window* pWindow = findChild(mxWindow.get(), rID);

    if (pWindow)
        return std::unique_ptr<UIObject>(new WindowUIObject(pWindow));

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
