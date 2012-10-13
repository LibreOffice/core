/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifdef _MSC_VER
#pragma warning (disable : 4786 4503)
#endif

#include "listviewbuilder.hxx"
#include "document_statistic.hxx"
#include "internal/utilities.hxx"
#include "internal/config.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <commctrl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <commctrl.h>
#include <tchar.h>
#include "internal/resource.h"

//------------------------------------

list_view_builder_ptr create_list_view_builder(
    HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2)
{
    if (is_windows_xp_or_above())
        return list_view_builder_ptr(new winxp_list_view_builder(hwnd_lv, col1, col2));
    else
        return list_view_builder_ptr(new list_view_builder(hwnd_lv, col1, col2));
}

//------------------------------------

list_view_builder::list_view_builder(
    HWND hwnd_list_view,
    const std::wstring& column1_title,
    const std::wstring& column2_title) :
    row_index_(-1),
    hwnd_list_view_(hwnd_list_view),
    column1_title_(column1_title),
    column2_title_(column2_title)
{
}

//------------------------------------

list_view_builder::~list_view_builder()
{
}

//------------------------------------

void list_view_builder::build(statistic_group_list_t& gl)
{
    setup_list_view();

    statistic_group_list_t::iterator group_iter     = gl.begin();
    statistic_group_list_t::iterator group_iter_end = gl.end();

    for (/**/; group_iter != group_iter_end; ++group_iter)
    {
        statistic_item_list_t::iterator item_iter     = group_iter->second.begin();
        statistic_item_list_t::iterator item_iter_end = group_iter->second.end();

        if (item_iter != item_iter_end)
            insert_group(group_iter->first);

        for (/**/; item_iter != item_iter_end; ++item_iter)
            insert_item(item_iter->title_, item_iter->value_, item_iter->editable_);
    }
}

//------------------------------------

void list_view_builder::setup_list_view()
{
    HIMAGELIST h_ils = ImageList_Create(16,15,ILC_MASK, 7, 0);
    HBITMAP    h_bmp = LoadBitmap(GetModuleHandle(MODULE_NAME), MAKEINTRESOURCE(IDB_PROPERTY_IMAGES));
    ImageList_AddMasked(h_ils, h_bmp, RGB(255, 0, 255));

    (void) ListView_SetImageList(hwnd_list_view_, h_ils, LVSIL_SMALL);

    std::wstring header = GetResString(IDS_PROPERTY);

    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT |
               LVCF_WIDTH |
                LVCF_TEXT |
               LVCF_SUBITEM;

    lvc.iSubItem = 0;
    lvc.pszText  = const_cast<wchar_t*>(header.c_str());
    lvc.cx       = 120;
    lvc.fmt      = LVCFMT_LEFT;

    ListView_InsertColumn(hwnd_list_view_, 0, &lvc);
    lvc.iSubItem = 1;
    header = GetResString(IDS_PROPERTY_VALUE);
    lvc.pszText = const_cast<wchar_t*>(header.c_str());
    ListView_InsertColumn(hwnd_list_view_, 1, &lvc);
}

//------------------------------------

void list_view_builder::insert_group(const std::wstring& /*title*/)
{
    insert_item(L"", L"", false);
}

//------------------------------------

void list_view_builder::insert_item(const std::wstring& title, const std::wstring& value, bool is_editable)
{
    LVITEM lvi;

    lvi.iItem      = ++row_index_;
    lvi.iSubItem   = 0;
    lvi.mask       = LVIF_TEXT;
    lvi.state      = 0;
    lvi.cchTextMax = static_cast<int>(title.size() + 1);
    lvi.stateMask  = 0;
    lvi.pszText    = const_cast<wchar_t*>(title.c_str());

    if (title.length() > 0)
    {
        lvi.mask |= LVIF_IMAGE;

        if (is_editable)
            lvi.iImage = 4;
        else
            lvi.iImage = 3;
    }

    ListView_InsertItem(hwnd_list_view_, &lvi);

    lvi.mask     = LVIF_TEXT;
    lvi.iSubItem = 1;
    lvi.pszText  = const_cast<wchar_t*>(value.c_str());

    ListView_SetItem(hwnd_list_view_, &lvi);
}

//------------------------------------

HWND list_view_builder::get_list_view() const
{
    return hwnd_list_view_;
}

//------------------------------------

winxp_list_view_builder::winxp_list_view_builder(
    HWND hwnd_list_view,
    const std::wstring& column1_title,
    const std::wstring& column2_title) :
    list_view_builder(hwnd_list_view, column1_title, column2_title),
    group_count_(-1),
    row_count_(0)
{
}

//------------------------------------

void winxp_list_view_builder::setup_list_view()
{
    list_view_builder::setup_list_view();

    ListView_EnableGroupView(get_list_view(), TRUE);
}

//------------------------------------

void winxp_list_view_builder::insert_group(const std::wstring& name)
{
    LVGROUP lvg;

    ZeroMemory(&lvg, sizeof(lvg));

    lvg.cbSize    = sizeof(lvg);
    lvg.mask      = LVGF_HEADER | LVGF_STATE | LVGF_GROUPID;
    lvg.pszHeader = const_cast<wchar_t*>(name.c_str());
    lvg.cchHeader = static_cast<int>(name.size() + 1);
    lvg.iGroupId  = ++group_count_;
    lvg.state     = LVGS_NORMAL;
    lvg.uAlign    = LVGA_HEADER_CENTER;

    ListView_InsertGroup(get_list_view(), row_count_++, &lvg);
}

//------------------------------------

void winxp_list_view_builder::insert_item(
    const std::wstring& title, const std::wstring& value, bool is_editable)
{
    LVITEM lvi;

    lvi.iItem      = ++row_index_;
    lvi.iSubItem   = 0;
    lvi.mask       = LVIF_TEXT | LVIF_GROUPID;
    lvi.state      = 0;
    lvi.stateMask  = 0;
    lvi.pszText    = const_cast<wchar_t*>(title.c_str());
    lvi.iGroupId   = group_count_;

    if (title.length() > 0)
    {
        lvi.mask |= LVIF_IMAGE;

        if (is_editable)
            lvi.iImage = 4;
        else
            lvi.iImage = 3;
    }

    ListView_InsertItem(get_list_view(), &lvi);

    lvi.mask     = LVIF_TEXT;
    lvi.iSubItem = 1;
    lvi.pszText  = const_cast<wchar_t*>(value.c_str());

    ListView_SetItem(get_list_view(), &lvi);

    row_count_++;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
