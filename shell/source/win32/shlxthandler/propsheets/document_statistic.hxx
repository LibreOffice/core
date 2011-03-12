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

#ifndef DOCUMENT_STATISTIC_HXX_INCLUDED
#define DOCUMENT_STATISTIC_HXX_INCLUDED

#include <utility>
#include <string>
#include <vector>
#include "internal/metainforeader.hxx"


//------------------------------------
//
//------------------------------------

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

//------------------------------------
//
//------------------------------------

typedef std::vector<statistic_item>                     statistic_item_list_t;
typedef std::pair<std::wstring, statistic_item_list_t>  statistic_group_t;
typedef std::vector<statistic_group_t>                  statistic_group_list_t;

//------------------------------------
//
//------------------------------------

class document_statistic_reader;
typedef std::auto_ptr<document_statistic_reader> document_statistic_reader_ptr;

document_statistic_reader_ptr create_document_statistic_reader(const std::string& document_name, CMetaInfoReader* meta_info_accessor);

//------------------------------------
//
//------------------------------------

class document_statistic_reader
{
public:
    virtual ~document_statistic_reader();

    void read(statistic_group_list_t* group_list);

    std::string get_document_name() const;

protected:
    document_statistic_reader(const std::string& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list) = 0;

    virtual void fill_origin_section( CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list);

private:
    std::string document_name_;
    CMetaInfoReader* meta_info_accessor_;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, CMetaInfoReader* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class writer_document_statistic_reader : public document_statistic_reader
{
protected:
    writer_document_statistic_reader(const std::string& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor, statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, CMetaInfoReader* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class calc_document_statistic_reader : public document_statistic_reader
{
protected:
    calc_document_statistic_reader(const std::string& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section( CMetaInfoReader *meta_info_accessor,statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, CMetaInfoReader* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class draw_impress_math_document_statistic_reader : public document_statistic_reader
{
protected:
    draw_impress_math_document_statistic_reader(const std::string& document_name, CMetaInfoReader* meta_info_accessor);

    virtual void fill_description_section(CMetaInfoReader *meta_info_accessor, statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, CMetaInfoReader* meta_info_accessor);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
