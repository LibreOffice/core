/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uiobject.hxx>
#include <svtools/valueset.hxx>

ValueSetUIObject::ValueSetUIObject(const VclPtr<vcl::Window>& rSetWin)
    : DrawingAreaUIObject(rSetWin)
    , mpSet(static_cast<ValueSet*>(mpController))
{
}

void ValueSetUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            OUString aIndexStr = rParameters.find("POS")->second;

            sal_Int32 nIndex = aIndexStr.toInt32();
            mpSet->SelectItem(nIndex);
            mpSet->Select();
        }
    }
    else
        DrawingAreaUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> ValueSetUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new ValueSetUIObject(pWindow));
}

OUString ValueSetUIObject::get_name() const { return "ValueSetUIObject"; }

StringMap ValueSetUIObject::get_state()
{
    StringMap aMap = DrawingAreaUIObject::get_state();
    aMap["SelectedItemId"] = OUString::number(mpSet->GetSelectedItemId());
    aMap["SelectedItemPos"] = OUString::number(mpSet->GetSelectItemPos());
    aMap["ItemsCount"] = OUString::number(mpSet->GetItemCount());
    return aMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
