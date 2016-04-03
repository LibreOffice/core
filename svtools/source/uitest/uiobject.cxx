/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uitest/uiobject.hxx"

#include <svtools/treelistbox.hxx>

TreeListUIObject::TreeListUIObject(VclPtr<SvTreeListBox> xTreeList):
    WindowUIObject(xTreeList)
{
}

StringMap TreeListUIObject::get_state()
{
    return WindowUIObject::get_state();
}

UIObjectType TreeListUIObject::get_type() const
{
    return UIObjectType::WINDOW;
}

OUString TreeListUIObject::get_name() const
{
    return OUString("TreeListUIObject");
}

std::unique_ptr<UIObject> TreeListUIObject::create(vcl::Window* pWindow)
{
    SvTreeListBox* pTreeList = dynamic_cast<SvTreeListBox*>(pWindow);
    assert(pTreeList);
    return std::unique_ptr<UIObject>(new TreeListUIObject(pTreeList));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
