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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_PROPSHEETS_DOCUMENT_STATISTIC_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_PROPSHEETS_DOCUMENT_STATISTIC_HXX

#include <utility>
#include <string>
#include <memory>
#include <vector>
#include "metainforeader.hxx"


struct statistic_item
{
    statistic_item();

    statistic_item(
        const std::wstring& title,
        const std::wstring& value,
        bool editable) :
        title_(title),
        value_(value),
        editable_(editable)
    {}

    std::wstring title_;
    std::wstring value_;
    bool editable_;
};


typedef std::vector<statistic_item>                     statistic_item_list_t;
typedef std::pair<std::wstring, statistic_item_list_t>  statistic_group_t;
typedef std::vector<statistic_group_t>                  statistic_group_list_t;


class document_statistic_reader;
typedef std::unique_ptr<document_statistic_reader> document_statistic_reader_ptr;

document_statistic_reader_ptr create_document_statistic_reader(const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);


class document_statistic_reader
{
public:
    virtual ~document_statistic_reader();

    void read(statistic_group_list_t* group_list);

    std::wstring get_document_name() const;

protected:
    document_statistic_reader(const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list) = 0;

    virtual void fill_origin_section( CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list);

private:
    std::wstring document_name_;
    CMetaInfoReader* meta_info_accessor_;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);
};


class writer_document_statistic_reader : public document_statistic_reader
{
protected:
    writer_document_statistic_reader(const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor, statistic_group_list_t* group_list) override;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);
};


class calc_document_statistic_reader : public document_statistic_reader
{
protected:
    calc_document_statistic_reader(const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section( CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list) override;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);
};


class draw_impress_math_document_statistic_reader : public document_statistic_reader
{
protected:
    draw_impress_math_document_statistic_reader(const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor, statistic_group_list_t* group_list) override;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::wstring& document_name, CMetaInfoReader* meta_info_accessor);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
