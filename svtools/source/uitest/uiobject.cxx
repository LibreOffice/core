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
#include <vcl/window.hxx>

ValueSetUIObject::ValueSetUIObject(const VclPtr<vcl::Window>& rSetWin)
    : DrawingAreaUIObject(rSetWin)
    , mpSet(static_cast<ValueSet*>(mpController))
{
}

void ValueSetUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find(u"POS"_ustr) != rParameters.end())
        {
            OUString aIndexStr = rParameters.find(u"POS"_ustr)->second;

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

OUString ValueSetUIObject::get_name() const { return u"ValueSetUIObject"_ustr; }

StringMap ValueSetUIObject::get_state()
{
    StringMap aMap = DrawingAreaUIObject::get_state();
    aMap[u"SelectedItemId"_ustr] = OUString::number(mpSet->GetSelectedItemId());
    aMap[u"SelectedItemPos"_ustr] = OUString::number(mpSet->GetSelectItemPos());
    aMap[u"ItemsCount"_ustr] = OUString::number(mpSet->GetItemCount());
    return aMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
