/*************************************************************************
 *
 *  $RCSfile: xml_parser.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:54:27 $
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

#ifndef _XML_PARSER_HXX_
#include "internal/xml_parser.hxx"
#endif

#ifndef _I_XML_PARSER_EVENT_HANDLER_HXX_
#include "internal/i_xml_parser_event_handler.hxx"
#endif

#include <assert.h>

namespace /* private */
{

    //######################################################
    /*  Extracts the local part of tag without
        namespace decoration e.g. meta:creator -> creator */
    const XML_Char COLON = (XML_Char)':';

    const XML_Char* get_local_name(const XML_Char* RawName)
    {
        const XML_Char* p = RawName;

        // go to the end
        while (*p) p++;

        // go back until the first ':'
        while (*p != COLON && p > RawName)
            p--;

        // if we are on a colon one step forward
        if (*p == COLON)
            p++;

        return p;
    }

    //################################################
    inline xml_parser* get_parser_instance(void* data)
    {
        return reinterpret_cast<xml_parser*>(XML_GetUserData(
            reinterpret_cast<XML_Parser>(data)));
    }

    //################################################
    bool has_only_whitespaces(const XML_Char* s, int len)
    {
        const XML_Char* p = s;
        for (int i = 0; i < len; i++)
            if (*p++ != ' ') return false;
        return true;
    }
}

//###################################################
xml_parser::xml_parser(const XML_Char* EncodingName) :
    document_handler_(0),
    xml_parser_(XML_ParserCreate(EncodingName))
{
    init();
}

//###################################################
xml_parser::xml_parser(const XML_Char* EncodingName, XML_Char NamespaceSeparator) :
    document_handler_(0),
    xml_parser_(XML_ParserCreate(EncodingName))
{
    init();
}

//###################################################
xml_parser::~xml_parser()
{
    XML_ParserFree(xml_parser_);
}

//###################################################
/* Callback functions will be called by the parser on
   different events */

//###################################################
extern "C" void xml_parser::xml_start_element_handler(
    void* UserData, const XML_Char* name, const XML_Char** atts)
{
    assert(UserData != NULL);

    xml_parser* pImpl  = get_parser_instance(UserData);

    if (pImpl->document_handler_)
    {
        xml_tag_attribute_container_t attributes;

        int i = 0;

        while(atts[i])
        {
            attributes[get_local_name(atts[i])] = atts[i+1];
            i += 2; // skip to next pair
        }

        pImpl->document_handler_->start_element(
            name, get_local_name(name), attributes);
    }
}

//###################################################
extern "C" void xml_parser::xml_end_element_handler(
    void* UserData, const XML_Char* name)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    if (pImpl->document_handler_)
        pImpl->document_handler_->end_element(name, get_local_name(name));
}

//###################################################
extern "C" void xml_parser::xml_character_data_handler(
    void* UserData, const XML_Char* s, int len)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    if (pImpl->document_handler_)
    {
        if (has_only_whitespaces(s,len))
            pImpl->document_handler_->ignore_whitespace(string_t(s, len));
        else
            pImpl->document_handler_->characters(string_t(s, len));
    }
}

//###################################################
extern "C" void xml_parser::xml_comment_handler(void* UserData, const XML_Char* Data)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    if (pImpl->document_handler_)
        pImpl->document_handler_->comment(Data);
}

//###################################################
void xml_parser::init()
{
    XML_SetUserData(xml_parser_, this);

    // we use the parser as handler argument,
    // so we could use it if necessary, the
    // UserData are usable anyway using
    // XML_GetUserData(...)
    XML_UseParserAsHandlerArg(xml_parser_);

    XML_SetElementHandler(
        xml_parser_,
        xml_start_element_handler,
        xml_end_element_handler);

    XML_SetCharacterDataHandler(
        xml_parser_,
        xml_character_data_handler);

    XML_SetCommentHandler(
        xml_parser_,
        xml_comment_handler);
}

//###################################################
void xml_parser::parse(const char* XmlData, size_t Length, bool IsFinal)
{
    if (0 == XML_Parse(xml_parser_, XmlData, Length, IsFinal))
        throw xml_parser_exception(
            XML_ErrorString(XML_GetErrorCode(xml_parser_)),
            XML_GetErrorCode(xml_parser_),
            XML_GetCurrentLineNumber(xml_parser_),
            XML_GetCurrentColumnNumber(xml_parser_),
            XML_GetCurrentByteIndex(xml_parser_));
}

//###################################################
void xml_parser::set_document_handler(
    i_xml_parser_event_handler* event_handler)
{
    document_handler_ = event_handler;
}

//###################################################
i_xml_parser_event_handler* xml_parser::get_document_handler() const
{
    return document_handler_;
}

//###################################################
void xml_parser::set_encoding(const XML_Char* Encoding)
{
    XML_SetEncoding(xml_parser_, Encoding);
}
