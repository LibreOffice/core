/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/uiobject.hxx>
#include <svx/svdobj.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <memory>

SvxColorValueSetUIObject::SvxColorValueSetUIObject(vcl::Window* pColorSetWin)
    : DrawingAreaUIObject(pColorSetWin)
    , mpColorSet(static_cast<SvxColorValueSet*>(mpController))
{
}

void SvxColorValueSetUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find(u"POS"_ustr) != rParameters.end())
        {
            OUString aIndexStr = rParameters.find(u"POS"_ustr)->second;
            sal_Int32 nIndex = aIndexStr.toInt32();
            mpColorSet->SelectItem(nIndex);
            mpColorSet->Select();
        }
    }
    else
        DrawingAreaUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SvxColorValueSetUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new SvxColorValueSetUIObject(pWindow));
}

OUString SvxColorValueSetUIObject::get_name() const { return u"SvxColorValueSetUIObject"_ustr; }

StringMap SvxColorValueSetUIObject::get_state()
{
    StringMap aMap = DrawingAreaUIObject::get_state();
    aMap[u"CurrColorId"_ustr] = OUString::number(mpColorSet->GetSelectedItemId());
    aMap[u"CurrColorPos"_ustr] = OUString::number(mpColorSet->GetSelectItemPos());
    aMap[u"ColorsCount"_ustr] = OUString::number(mpColorSet->GetItemCount());
    aMap[u"ColCount"_ustr] = OUString::number(mpColorSet->GetColCount());
    aMap[u"ColorText"_ustr] = mpColorSet->GetItemText(mpColorSet->GetSelectedItemId());
    Color currColor = mpColorSet->GetItemColor(mpColorSet->GetSelectedItemId());
    aMap[u"R"_ustr] = OUString::number(currColor.GetRed());
    aMap[u"G"_ustr] = OUString::number(currColor.GetGreen());
    aMap[u"B"_ustr] = OUString::number(currColor.GetBlue());
    aMap[u"RGB"_ustr] = "(" + OUString::number(currColor.GetRed()) + ","
                        + OUString::number(currColor.GetGreen()) + ","
                        + OUString::number(currColor.GetBlue()) + ")";
    return aMap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
