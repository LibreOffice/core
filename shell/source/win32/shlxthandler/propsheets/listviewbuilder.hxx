/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef LISTVIEWBUILDER_HXX_INCLUDED
#define LISTVIEWBUILDER_HXX_INCLUDED

//------------------------------------
// include
//------------------------------------

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include <memory>
#include "document_statistic.hxx"

//------------------------------------
//
//------------------------------------

class list_view_builder;
typedef std::auto_ptr<list_view_builder> list_view_builder_ptr;

// factory method for list_view_builder
list_view_builder_ptr create_list_view_builder(
    HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);

//------------------------------------
//
//------------------------------------

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
    int get_current_row() const;

    int  row_index_;

private:
    HWND hwnd_list_view_;
    std::wstring column1_title_;
    std::wstring column2_title_;

    friend list_view_builder_ptr create_list_view_builder(HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);
};

//------------------------------------
//
//------------------------------------

class winxp_list_view_builder : public list_view_builder
{
protected:
    winxp_list_view_builder(
        HWND hwnd_list_view,
        const std::wstring& column1_title,
        const std::wstring& column2_title);

    virtual void setup_list_view();
    virtual void insert_group(const std::wstring& name);
    virtual void insert_item(const std::wstring& title, const std::wstring& value, bool is_editable);

private:
    int  group_count_;
    int  row_count_;

    friend list_view_builder_ptr create_list_view_builder(HWND hwnd_lv, const std::wstring& col1, const std::wstring& col2);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
