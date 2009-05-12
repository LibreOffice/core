/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: i_xml_parser_event_handler.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _I_XML_PARSER_EVENT_HANDLER_HXX_
#define _I_XML_PARSER_EVENT_HANDLER_HXX_

#include <string>
#include <map>
#include <utility>

#if defined(XML_UNICODE) || defined(XML_UNICODE_WCHAR_T)
    typedef std::wstring string_t;
#else
    typedef std::string string_t;
#endif

// name-value container
typedef std::map<string_t, string_t> xml_tag_attribute_container_t;


//#########################################
class i_xml_parser_event_handler
{
public:
    virtual ~i_xml_parser_event_handler() {};

    virtual void start_document() = 0;

    virtual void end_document() = 0;

    virtual void start_element(
        const string_t& raw_name,
        const string_t& local_name,
        const xml_tag_attribute_container_t& attributes) = 0;

    virtual void end_element(
        const string_t& raw_name,
        const string_t& local_name) = 0;

    virtual void characters(
        const string_t& character) = 0;

    virtual void ignore_whitespace(
        const string_t& whitespaces) = 0;

    virtual void processing_instruction(
        const string_t& target, const string_t& data) = 0;

    virtual void comment(const string_t& comment) = 0;
};

#endif

