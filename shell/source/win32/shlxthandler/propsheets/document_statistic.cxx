/*************************************************************************
 *
 *  $RCSfile: document_statistic.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 11:14:05 $
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

#pragma warning (disable : 4786 4503)

#ifndef DOCUMENT_STATISTIC_HXX_INCLUDED
#include "document_statistic.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef METAINFO_HXX_INCLUDED
#include "internal/metainfo.hxx"
#endif

#ifndef RESOURCE_H_INCLUDED
#include "internal/resource.h"
#endif

#ifndef FILEEXTENSIONS_HXX_INCLUDED
#include "internal/fileextensions.hxx"
#endif

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef ISO8601_CONVERTER_HXX_INCLUDED
#include "internal/iso8601_converter.hxx"
#endif

//#####################################
const bool READONLY  = false;
const bool WRITEABLE = true;

//#####################################
document_statistic_reader_ptr create_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor)
{
    File_Type_t file_type = get_file_type(document_name);

    if (WRITER == file_type)
        return document_statistic_reader_ptr(new writer_document_statistic_reader(document_name, meta_info_accessor));
    else if (CALC == file_type)
        return document_statistic_reader_ptr(new calc_document_statistic_reader(document_name, meta_info_accessor));
    else
        return document_statistic_reader_ptr(new draw_impress_math_document_statistic_reader(document_name, meta_info_accessor));
}


//#####################################
document_statistic_reader::document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor) :
    document_name_(document_name),
    meta_info_accessor_(meta_info_accessor)
{}

//#####################################
document_statistic_reader::~document_statistic_reader()
{}

//#####################################
void document_statistic_reader::read(statistic_group_list_t* group_list)
{
    group_list->clear();
    fill_description_section(meta_info_accessor_, group_list);
    fill_origin_section(meta_info_accessor_, group_list);
}

//#####################################
std::string document_statistic_reader::get_document_name() const
{
    return document_name_;
}

//#####################################
void document_statistic_reader::fill_origin_section(COpenOfficeMetaInformation *meta_info_accessor, statistic_group_list_t* group_list)
{
    statistic_item_list_t il;

    il.push_back(statistic_item(GetResString(IDS_AUTHOR), meta_info_accessor->getTagData( META_INFO_AUTHOR ), READONLY));

    il.push_back(statistic_item(GetResString(IDS_MODIFIED),
        iso8601_date_to_local_date(meta_info_accessor->getTagData(META_INFO_MODIFIED )), READONLY));

    il.push_back(statistic_item(GetResString(IDS_DOCUMENT_NUMBER), meta_info_accessor->getTagData( META_INFO_DOCUMENT_NUMBER ), READONLY));

    il.push_back(statistic_item(GetResString(IDS_EDITING_TIME),
        iso8601_duration_to_local_duration(meta_info_accessor->getTagData( META_INFO_EDITING_TIME )), READONLY));

    group_list->push_back(statistic_group_t(GetResString(IDS_ORIGIN), il));
}

//#####################################
writer_document_statistic_reader::writer_document_statistic_reader(const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor) :
    document_statistic_reader(document_name, meta_info_accessor)
{}

//#####################################
void writer_document_statistic_reader::fill_description_section(COpenOfficeMetaInformation *meta_info_accessor, statistic_group_list_t* group_list)
{
    statistic_item_list_t il;

    il.push_back(statistic_item(GetResString(IDS_TITLE),    meta_info_accessor->getTagData( META_INFO_TITLE ),       READONLY));
    il.push_back(statistic_item(GetResString(IDS_COMMENTS), meta_info_accessor->getTagData( META_INFO_DESCRIPTION ), READONLY));
    il.push_back(statistic_item(GetResString(IDS_SUBJECT),  meta_info_accessor->getTagData( META_INFO_SUBJECT ),     READONLY));
    il.push_back(statistic_item(GetResString(IDS_KEYWORDS), meta_info_accessor->getTagData(META_INFO_KEYWORDS ),    READONLY));
    il.push_back(statistic_item(GetResString(IDS_PAGES),    meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_PAGES) , READONLY));
    il.push_back(statistic_item(GetResString(IDS_TABLES), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_TABLES) , READONLY));
    il.push_back(statistic_item(GetResString(IDS_GRAPHICS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_DRAWS) , READONLY));
    il.push_back(statistic_item(GetResString(IDS_OLE_OBJECTS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_OBJECTS) ,    READONLY));
    il.push_back(statistic_item(GetResString(IDS_PARAGRAPHS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_PARAGRAPHS) , READONLY));
    il.push_back(statistic_item(GetResString(IDS_WORDS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_WORDS) , READONLY));
    il.push_back(statistic_item(GetResString(IDS_CHARACTERS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_CHARACTERS) , READONLY));

    group_list->push_back(statistic_group_t(GetResString(IDS_DESCRIPTION), il));
}

//#######################################
calc_document_statistic_reader::calc_document_statistic_reader(
    const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor) :
    document_statistic_reader(document_name, meta_info_accessor)
{}

//#######################################
void calc_document_statistic_reader::fill_description_section(
    COpenOfficeMetaInformation *meta_info_accessor,statistic_group_list_t* group_list)
{
    statistic_item_list_t il;

    il.push_back(statistic_item(GetResString(IDS_TITLE),       meta_info_accessor->getTagData( META_INFO_TITLE ),       READONLY));
    il.push_back(statistic_item(GetResString(IDS_COMMENTS),    meta_info_accessor->getTagData( META_INFO_DESCRIPTION ), READONLY));
    il.push_back(statistic_item(GetResString(IDS_SUBJECT),     meta_info_accessor->getTagData( META_INFO_SUBJECT ),     READONLY));
    il.push_back(statistic_item(GetResString(IDS_KEYWORDS),    meta_info_accessor->getTagData(META_INFO_KEYWORDS ),    READONLY));
    il.push_back(statistic_item(GetResString(IDS_TABLES),      meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_TABLES) ,  READONLY));
    il.push_back(statistic_item(GetResString(IDS_CELLS),       meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_CELLS) ,   READONLY));
    il.push_back(statistic_item(GetResString(IDS_OLE_OBJECTS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_OBJECTS) , READONLY));

    group_list->push_back(statistic_group_t(GetResString(IDS_DESCRIPTION), il));
}

//#######################################
draw_impress_math_document_statistic_reader::draw_impress_math_document_statistic_reader(
    const std::string& document_name, COpenOfficeMetaInformation* meta_info_accessor) :
    document_statistic_reader(document_name, meta_info_accessor)
{}

//#######################################
void draw_impress_math_document_statistic_reader::fill_description_section(
    COpenOfficeMetaInformation *meta_info_accessor, statistic_group_list_t* group_list)
{
    statistic_item_list_t il;

    il.push_back(statistic_item(GetResString(IDS_TITLE),       meta_info_accessor->getTagData( META_INFO_TITLE ),       READONLY));
    il.push_back(statistic_item(GetResString(IDS_COMMENTS),    meta_info_accessor->getTagData( META_INFO_DESCRIPTION ), READONLY));
    il.push_back(statistic_item(GetResString(IDS_SUBJECT),     meta_info_accessor->getTagData( META_INFO_SUBJECT ),     READONLY));
    il.push_back(statistic_item(GetResString(IDS_KEYWORDS),    meta_info_accessor->getTagData(META_INFO_KEYWORDS ),    READONLY));
    il.push_back(statistic_item(GetResString(IDS_PAGES),       meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_PAGES) ,   READONLY));
    il.push_back(statistic_item(GetResString(IDS_OLE_OBJECTS), meta_info_accessor->getTagAttribute( META_INFO_DOCUMENT_STATISTIC,META_INFO_OBJECTS) , READONLY));

    group_list->push_back(statistic_group_t(GetResString(IDS_DESCRIPTION), il));
}
