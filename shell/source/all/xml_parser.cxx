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


#include "internal/xml_parser.hxx"
#include "internal/i_xml_parser_event_handler.hxx"

#include <assert.h>

namespace /* private */
{

    /*  Extracts the local part of tag without
        namespace decoration e.g. meta:creator -> creator */
    const XML_Char COLON = (XML_Char)':';

    const XML_Char* get_local_name(const XML_Char* rawname)
    {
        const XML_Char* p = rawname;

        // go to the end
        while (*p) p++;

        // go back until the first ':'
        while (*p != COLON && p > rawname)
            p--;

        // if we are on a colon one step forward
        if (*p == COLON)
            p++;

        return p;
    }

    inline xml_parser* get_parser_instance(void* data)
    {
        return reinterpret_cast<xml_parser*>(XML_GetUserData(
            reinterpret_cast<XML_Parser>(data)));
    }

    bool has_only_whitespaces(const XML_Char* s, int len)
    {
        const XML_Char* p = s;
        for (int i = 0; i < len; i++)
            if (*p++ != ' ') return false;
        return true;
    }
}

xml_parser::xml_parser(const XML_Char* EncodingName) :
    document_handler_(0),
    xml_parser_(XML_ParserCreate(EncodingName))
{
    init();
}

xml_parser::~xml_parser()
{
    XML_ParserFree(xml_parser_);
}

/* Callback functions will be called by the parser on
   different events */

extern "C"
{

static void xml_start_element_handler(void* UserData, const XML_Char* name, const XML_Char** atts)
{
    assert(UserData != NULL);

    xml_parser* pImpl  = get_parser_instance(UserData);

    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
    {
        xml_tag_attribute_container_t attributes;

        int i = 0;

        while(atts[i])
        {
            attributes[reinterpret_cast<const char_t*>(get_local_name(atts[i]))] = reinterpret_cast<const char_t*>(atts[i+1]);
            i += 2; // skip to next pair
        }

        pDocHdl->start_element(
            reinterpret_cast<const char_t*>(name), reinterpret_cast<const char_t*>(get_local_name(name)), attributes);
    }
}

static void xml_end_element_handler(void* UserData, const XML_Char* name)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
        pDocHdl->end_element(reinterpret_cast<const char_t*>(name), reinterpret_cast<const char_t*>(get_local_name(name)));
}

static void xml_character_data_handler(void* UserData, const XML_Char* s, int len)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
    {
        if (has_only_whitespaces(s,len))
            pDocHdl->ignore_whitespace(string_t(reinterpret_cast<const char_t*>(s), len));
        else
            pDocHdl->characters(string_t(reinterpret_cast<const char_t*>(s), len));
    }
}

static void xml_comment_handler(void* UserData, const XML_Char* Data)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
        pDocHdl->comment(reinterpret_cast<const char_t*>(Data));
}

} // extern "C"

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

void xml_parser::parse(const char* XmlData, size_t Length, bool IsFinal)
{
    if (XML_STATUS_ERROR ==
            XML_Parse(xml_parser_, XmlData, static_cast<int>(Length), IsFinal))
    {
        throw xml_parser_exception(
            (char*)XML_ErrorString(XML_GetErrorCode(xml_parser_)),
            (int)XML_GetErrorCode(xml_parser_),
            XML_GetCurrentLineNumber(xml_parser_),
            XML_GetCurrentColumnNumber(xml_parser_),
            XML_GetCurrentByteIndex(xml_parser_));
    }
}

void xml_parser::set_document_handler(
    i_xml_parser_event_handler* event_handler)
{
    document_handler_ = event_handler;
}

i_xml_parser_event_handler* xml_parser::get_document_handler() const
{
    return document_handler_;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
