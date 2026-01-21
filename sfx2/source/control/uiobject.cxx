/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/uiobject.hxx>
#include <templateviewitem.hxx>

TemplateDlgLocalViewUIObject::TemplateDlgLocalViewUIObject(vcl::Window* rTemplateDlgLocalView)
    : DrawingAreaUIObject(rTemplateDlgLocalView)
    , mpTemplateDlgLocalView(static_cast<TemplateDlgLocalView*>(mpController))
{
}

StringMap TemplateDlgLocalViewUIObject::get_state()
{
    StringMap aMap = WindowUIObject::get_state();
    aMap[u"SelectedIndex"_ustr] = OUString::number(mpTemplateDlgLocalView->get_selected_index());
    if (TemplateViewItem* pSelectedItem = mpTemplateDlgLocalView->mpSelectedItem)
    {
        aMap[u"SelectedItemTitle"_ustr] = pSelectedItem->getTitle();
    }

    return aMap;
}

void TemplateDlgLocalViewUIObject::execute(const OUString& rAction, const StringMap& rParameters)
{
    if (rAction == "EDIT" || rAction == "RENAME" || rAction == "DELETE" || rAction == "DEFAULT"
        || rAction == "MOVE" || rAction == "EXPORT")
    {
        if (mpTemplateDlgLocalView->mpSelectedItem)
        {
            mpTemplateDlgLocalView->ContextMenuSelectHdl(rAction.toAsciiLowerCase());
        }
    }
    else if (rAction == "SELECT")
    {
        if (rParameters.find(u"POS"_ustr) != rParameters.end())
        {
            auto itr = rParameters.find(u"POS"_ustr);
            sal_uInt32 nPos = itr->second.toUInt32();
            mpTemplateDlgLocalView->set_cursor(nPos);
            mpTemplateDlgLocalView->updateSelection();
        }
    }
    else
        WindowUIObject::execute(rAction, rParameters);
}

std::unique_ptr<UIObject> TemplateDlgLocalViewUIObject::create(vcl::Window* pWindow)
{
    return std::unique_ptr<UIObject>(new TemplateDlgLocalViewUIObject(pWindow));
}

OUString TemplateDlgLocalViewUIObject::get_name() const
{
    return u"TemplateDlgLocalViewUIObject"_ustr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
