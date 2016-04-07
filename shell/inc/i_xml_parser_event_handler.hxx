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

#ifndef INCLUDED_SHELL_INC_INTERNAL_I_XML_PARSER_EVENT_HANDLER_HXX
#define INCLUDED_SHELL_INC_INTERNAL_I_XML_PARSER_EVENT_HANDLER_HXX

#include <string>
#include <map>

#ifdef _WIN32
    typedef std::wstring string_t;
    typedef wchar_t char_t;
#else
    typedef std::string string_t;
    typedef char char_t;
#endif

// name-value container
typedef std::map<string_t, string_t> xml_tag_attribute_container_t;


class i_xml_parser_event_handler
{
public:
    virtual ~i_xml_parser_event_handler() {};

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

    virtual void comment(const string_t& comment) = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
