/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_xml_parser_event_handler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:36:49 $
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

#ifndef I_XML_PARSER_EVENT_HANDLER_HXX_INCLUDED
#define I_XML_PARSER_EVENT_HANDLER_HXX_INCLUDED

#include <string>
#include <map>

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

