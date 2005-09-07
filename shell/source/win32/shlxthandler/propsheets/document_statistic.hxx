/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: document_statistic.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:02:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DOCUMENT_STATISTIC_HXX_INCLUDED
#define DOCUMENT_STATISTIC_HXX_INCLUDED

#include <utility>
#include <string>
#include <vector>

#ifndef METAINFOREADER_HXX_INCLUDED
#include "internal/metainforeader.hxx"
#endif


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
