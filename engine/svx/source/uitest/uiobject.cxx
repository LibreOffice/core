/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
        auto itIndex = rParameters.find(u"INDEX"_ustr);
        if (itIndex != rParameters.end())
        {
            OUString aIndexStr = itIndex->second;

            sal_Int32 nIndex = aIndexStr.toInt32();
            mpCharSet->OutputIndex(nIndex);
        }
        else
        {
            auto itColumn = rParameters.find(u"COLUMN"_ustr);
            auto itRow = rParameters.find(u"ROW"_ustr);
            if (itColumn != rParameters.end() && itRow != rParameters.end())
            {
                OUString aColStr = itColumn->second;
                OUString aRowStr = itRow->second;

                sal_Int32 nColumn = aColStr.toInt32();
                sal_Int32 nRow = aRowStr.toInt32();

                sal_Int32 nIndex = nColumn * COLUMN_COUNT + nRow;
                mpCharSet->OutputIndex(nIndex);
            }
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
