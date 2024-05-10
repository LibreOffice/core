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
#include <svx/numvset.hxx>
#include <vcl/window.hxx>

SvxShowCharSetUIObject::SvxShowCharSetUIObject(const VclPtr<vcl::Window>& rCharSetWin)
    : DrawingAreaUIObject(rCharSetWin)
    , mpCharSet(static_cast<SvxShowCharSet*>(mpController))
{
}

void SvxShowCharSetUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "SELECT")
    {
        if (rParameters.find(u"INDEX"_ustr) != rParameters.end())
        {
            OUString aIndexStr = rParameters.find(u"INDEX"_ustr)->second;

            sal_Int32 nIndex = aIndexStr.toInt32();
            mpCharSet->OutputIndex(nIndex);
        }
        else if (rParameters.find(u"COLUMN"_ustr) != rParameters.end() &&
                rParameters.find(u"ROW"_ustr) != rParameters.end())
        {
            OUString aColStr = rParameters.find(u"COLUMN"_ustr)->second;
            OUString aRowStr = rParameters.find(u"ROW"_ustr)->second;

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
    return std::unique_ptr<UIObject>(new SvxShowCharSetUIObject(pWindow));
}

OUString SvxShowCharSetUIObject::get_name() const
{
    return u"SvxShowCharSetUIObject"_ustr;
}


SvxNumValueSetUIObject::SvxNumValueSetUIObject(vcl::Window* pNumValueSetWin)
    : DrawingAreaUIObject(pNumValueSetWin)
    , mpNumValueSet(static_cast<SvxNumValueSet*>(mpController))
{
}

void SvxNumValueSetUIObject::execute(const OUString& rAction,
        const StringMap& rParameters)
{
    if (rAction == "CHOOSE")
    {
        if (rParameters.find(u"POS"_ustr) != rParameters.end())
        {
            OUString aIndexStr = rParameters.find(u"POS"_ustr)->second;
            sal_Int32 nIndex = aIndexStr.toInt32();
            mpNumValueSet->SelectItem(nIndex);
            mpNumValueSet->Select();
        }
    }
    else
       DrawingAreaUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> SvxNumValueSetUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new SvxNumValueSetUIObject(pWindow));
}

OUString SvxNumValueSetUIObject::get_name() const
{
    return u"SvxNumValueSetUIObject"_ustr;
}

StringMap SvxNumValueSetUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap[u"SelectedItemId"_ustr] = OUString::number( mpNumValueSet->GetSelectedItemId() );
    aMap[u"SelectedItemPos"_ustr] = OUString::number( mpNumValueSet->GetSelectItemPos() );
    aMap[u"ItemsCount"_ustr] = OUString::number(mpNumValueSet->GetItemCount());
    aMap[u"ItemText"_ustr] = mpNumValueSet->GetItemText(mpNumValueSet->GetSelectedItemId());
    return aMap;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
