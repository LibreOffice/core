/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <uitest/uiobject.hxx>

#include <vcl/treelistbox.hxx>
#include <svtools/simptabl.hxx>

namespace {

}

SimpleTableUIObject::SimpleTableUIObject(const VclPtr<SvSimpleTable>& xTable):
    TreeListUIObject(xTable),
    mxTable(xTable)
{
}

StringMap SimpleTableUIObject::get_state()
{
    StringMap aMap = TreeListUIObject::get_state();

    aMap["ColumnCount"] = OUString::number(mxTable->GetColumnCount());
    aMap["RowCount"] = OUString::number(mxTable->GetRowCount());
    aMap["SelectedRowCount"] = OUString::number(mxTable->GetSelectedRowCount());
    aMap["SelectedColumnCount"] = OUString::number(mxTable->GetSelectedColumnCount());
    aMap["EntryCount"] = OUString::number(mxTable->GetEntryCount());

    return aMap;
}

OUString SimpleTableUIObject::get_type() const
{
    return "SimpleTable";
}

std::unique_ptr<UIObject> SimpleTableUIObject::createFromContainer(vcl::Window* pWindow)
{
    SvSimpleTableContainer* pTableContainer = dynamic_cast<SvSimpleTableContainer*>(pWindow);
    assert(pTableContainer);
    return std::unique_ptr<UIObject>(new SimpleTableUIObject(pTableContainer->GetTable()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
