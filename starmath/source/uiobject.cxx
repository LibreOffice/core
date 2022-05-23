/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include "uiobject.hxx"
#include <ElementsDockingWindow.hxx>

ElementUIObject::ElementUIObject(SmElementsControl* pElementSelector,
        const OUString& rID):
    mpElementsSelector(pElementSelector),
    maID(rID)
{
}

StringMap ElementUIObject::get_state()
{
    StringMap aMap;
    aMap["ID"] = maID;

    SmElement* pElement = nullptr;
    if (pElement)
        aMap["Text"] = pElement->getText();

    return aMap;
}

void ElementUIObject::execute(const OUString& rAction,
        const StringMap& /*rParameters*/)
{
    if (rAction == "SELECT")
    {
        weld::IconView* pElement = nullptr;
        if (pElement)
            mpElementsSelector->maSelectHdlLink.Call(*pElement);
    }
}

ElementSelectorUIObject::ElementSelectorUIObject(vcl::Window* pElementSelectorWindow)
    : DrawingAreaUIObject(pElementSelectorWindow)
    , mpElementsSelector(static_cast<SmElementsControl*>(nullptr))
{
}

StringMap ElementSelectorUIObject::get_state()
{
    StringMap aMap = DrawingAreaUIObject::get_state();

    SmElement* pElement = nullptr;
    if (pElement)
        aMap["CurrentEntry"] = pElement->getText();

    aMap["CurrentSelection"] = OUString::fromUtf8(mpElementsSelector->msCurrentSetId.mpId);

    return aMap;
}

std::unique_ptr<UIObject> ElementSelectorUIObject::get_child(const OUString& rID)
{
    size_t nID = rID.toInt32();
    size_t n = 0;
    if (nID >= n)
        throw css::uno::RuntimeException("invalid id");

    return std::unique_ptr<UIObject>(new ElementUIObject(mpElementsSelector, rID));
}

std::set<OUString> ElementSelectorUIObject::get_children() const
{
    std::set<OUString> aChildren;

    size_t n = 0;
    for (size_t i = 0; i < n; ++i)
    {
        aChildren.insert(OUString::number(i));
    }

    return aChildren;
}

std::unique_ptr<UIObject> ElementSelectorUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new ElementSelectorUIObject(pWindow));
}

OUString ElementSelectorUIObject::get_name() const
{
    return "SmElementSelector";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
