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
    return OUString("SvxShowCharSetUIObject");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
