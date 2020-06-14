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
#include <svx/charmap.hxx>
#include <vcl/layout.hxx>

#include <memory>
#include <svx/numvset.hxx>
#include <vcl/layout.hxx>

SvxShowCharSetUIObject::SvxShowCharSetUIObject(const VclPtr<vcl::Window>& xCharSetWin, SvxShowCharSet* pCharSet):
    WindowUIObject(xCharSetWin),
    mpCharSet(pCharSet)
{
}

void SvxShowCharSetUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find("INDEX") != rParameters.end())
        {
            OUString aIndexStr = rParameters.find("INDEX")->second;

            sal_Int32 nIndex = aIndexStr.toInt32();
            mpCharSet->OutputIndex(nIndex);
        }
        else if (rParameters.find("COLUMN") != rParameters.end() &&
                rParameters.find("ROW") != rParameters.end())
        {
            OUString aColStr = rParameters.find("COLUMN")->second;
            OUString aRowStr = rParameters.find("ROW")->second;

            sal_Int32 nColumn = aColStr.toInt32();
            sal_Int32 nRow = aRowStr.toInt32();

            sal_Int32 nIndex = nColumn * COLUMN_COUNT + nRow;
            mpCharSet->OutputIndex(nIndex);
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SvxShowCharSetUIObject::create(vcl::Window* pWindow)
{
    VclDrawingArea* pCharSetWin = dynamic_cast<VclDrawingArea*>(pWindow);
    assert(pCharSetWin);
    return std::unique_ptr<UIObject>(new SvxShowCharSetUIObject(pCharSetWin, static_cast<SvxShowCharSet*>(pCharSetWin->GetUserData())));
}

OUString SvxShowCharSetUIObject::get_name() const
{
    return "SvxShowCharSetUIObject";
}


SvxNumValueSetUIObject::SvxNumValueSetUIObject(vcl::Window*  xNumValueSetWin , SvxNumValueSet* pNumValueSet):
    WindowUIObject(xNumValueSetWin),
    mpNumValueSet(pNumValueSet)
{
}

void SvxNumValueSetUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find("POS") != rParameters.end())
        {
            OUString aIndexStr = rParameters.find("POS")->second;
            sal_Int32 nIndex = aIndexStr.toInt32();
            mpNumValueSet->SelectItem(nIndex);
            mpNumValueSet->Select();
        }
    }
    else
       WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SvxNumValueSetUIObject::create(vcl::Window* pWindow)
{
    VclDrawingArea* pNumValueSetWin = dynamic_cast<VclDrawingArea*>(pWindow);
    assert(pNumValueSetWin);
    return std::unique_ptr<UIObject>(new SvxNumValueSetUIObject(pNumValueSetWin, static_cast<SvxNumValueSet*>(pNumValueSetWin->GetUserData())));
}

OUString SvxNumValueSetUIObject::get_name() const
{
    return "SvxNumValueSetUIObject";
}

StringMap SvxNumValueSetUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap["SelectedItemId"] = OUString::number( mpNumValueSet->GetSelectedItemId() );
    aMap["SelectedItemPos"] = OUString::number( mpNumValueSet->GetSelectItemPos() );
    aMap["ItemsCount"] = OUString::number(mpNumValueSet->GetItemCount());
    aMap["ItemText"] = mpNumValueSet->GetItemText(mpNumValueSet->GetSelectedItemId());
    return aMap;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
