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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_PROPSHEETS_LISTVIEWBUILDER_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_PROPSHEETS_LISTVIEWBUILDER_HXX

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include <memory>
#include "document_statistic.hxx"


class list_view_builder;
typedef std::unique_ptr<list_view_builder> list_view_builder_ptr;

// factory method for list_view_builder
list_view_builder_ptr create_list_view_builder(
    HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);


class list_view_builder
{
public:
    virtual ~list_view_builder();

    void build(statistic_group_list_t& gl);

protected:
    list_view_builder(
        HWND hwnd_list_view,
        const std::wstring& column1_title,
        const std::wstring& column2_title);

    virtual void setup_list_view();
    virtual void insert_group(const std::wstring& title);
    virtual void insert_item(const std::wstring& title, const std::wstring& value, bool is_editable);

    HWND get_list_view() const;
private:
    int  row_index_;
    HWND hwnd_list_view_;
    std::wstring column1_title_;
    std::wstring column2_title_;
    int  group_count_;
    int  row_count_;

    friend list_view_builder_ptr create_list_view_builder(HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
