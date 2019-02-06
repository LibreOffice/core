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


#include <xml_parser.hxx>
#include <i_xml_parser_event_handler.hxx>

#ifdef _WIN32
#include <utilities.hxx>
#else
#define UTF8ToWString(s) s
#endif

#include <assert.h>

namespace /* private */
{

    /*  Extracts the local part of tag without
        namespace decoration e.g. meta:creator -> creator */
    const XML_Char COLON = ':';

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

    xml_parser* get_parser_instance(void* data)
    {
        return static_cast<xml_parser*>(XML_GetUserData(
            static_cast<XML_Parser>(data)));
    }

    bool has_only_whitespaces(const XML_Char* s, int len)
    {
        const XML_Char* p = s;
        for (int i = 0; i < len; i++)
            if (*p++ != ' ') return false;
        return true;
    }
}

xml_parser::xml_parser() :
    document_handler_(nullptr),
    xml_parser_(XML_ParserCreate(nullptr))
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
    assert(UserData != nullptr);

    xml_parser* pImpl  = get_parser_instance(UserData);

    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (!pDocHdl)
        return;

    xml_tag_attribute_container_t attributes;

    int i = 0;

    while(atts[i])
    {
        attributes[UTF8ToWString(reinterpret_cast<const char*>(get_local_name(atts[i])))] = UTF8ToWString(reinterpret_cast<const char*>(atts[i+1]));
        i += 2; // skip to next pair
    }

    pDocHdl->start_element(
        UTF8ToWString(reinterpret_cast<const char*>(name)), UTF8ToWString(reinterpret_cast<const char*>(get_local_name(name))), attributes);
}

static void xml_end_element_handler(void* UserData, const XML_Char* name)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
        pDocHdl->end_element(UTF8ToWString(reinterpret_cast<const char*>(name)), UTF8ToWString(reinterpret_cast<const char*>(get_local_name(name))));
}

static void xml_character_data_handler(void* UserData, const XML_Char* s, int len)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
    {
        if (has_only_whitespaces(s,len))
            pDocHdl->ignore_whitespace(UTF8ToWString(std::string(reinterpret_cast<const char*>(s), len)));
        else
            pDocHdl->characters(UTF8ToWString(std::string(reinterpret_cast<const char*>(s), len)));
    }
}

static void xml_comment_handler(void* UserData, const XML_Char* Data)
{
    assert(UserData);

    xml_parser* pImpl  = get_parser_instance(UserData);
    i_xml_parser_event_handler* pDocHdl = pImpl->get_document_handler();
    if (pDocHdl)
        pDocHdl->comment(UTF8ToWString(reinterpret_cast<const char*>(Data)));
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
            XML_ErrorString(XML_GetErrorCode(xml_parser_)));
    }
}

void xml_parser::set_document_handler(
    i_xml_parser_event_handler* event_handler)
{
    document_handler_ = event_handler;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
