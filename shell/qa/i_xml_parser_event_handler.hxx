/*************************************************************************
 *
 *  $RCSfile: i_xml_parser_event_handler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:53:53 $
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

