/*************************************************************************
 *
 *  $RCSfile: document_statistic.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 11:14:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DOCUMENT_STATISTIC_HXX_INCLUDED
#define DOCUMENT_STATISTIC_HXX_INCLUDED

#include <utility>
#include <string>
#include <vector>

#ifndef METAINFO_HXX_INCLUDED
#include "internal/metainfo.hxx"
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

document_statistic_reader_ptr create_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);

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
    document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);

    virtual void fill_description_section(COpenOfficeMetaInformation *meta_info_accessor,statistic_group_list_t* group_list) = 0;

    virtual void fill_origin_section( COpenOfficeMetaInformation *meta_info_accessor,statistic_group_list_t* group_list);

private:
    std::string document_name_;
    COpenOfficeMetaInformation* meta_info_accessor_;

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class writer_document_statistic_reader : public document_statistic_reader
{
protected:
    writer_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);

    virtual void fill_description_section(COpenOfficeMetaInformation *meta_info_accessor, statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class calc_document_statistic_reader : public document_statistic_reader
{
protected:
    calc_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);

    virtual void fill_description_section( COpenOfficeMetaInformation *meta_info_accessor,statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);
};

//------------------------------------
//
//------------------------------------

class draw_impress_math_document_statistic_reader : public document_statistic_reader
{
protected:
    draw_impress_math_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);

    virtual void fill_description_section(COpenOfficeMetaInformation *meta_info_accessor, statistic_group_list_t* group_list);

    friend document_statistic_reader_ptr create_document_statistic_reader(
        const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor);
};

#endif
