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

#ifndef BASEREADER_HXX_INCLUDED
#define BASEREADER_HXX_INCLUDED

#include "internal/global.hxx"
#include "internal/types.hxx"
#include "internal/config.hxx"
#include "internal/utilities.hxx"
#include "internal/i_xml_parser_event_handler.hxx"

#include "internal/xml_parser.hxx"
#include "internal/zipfile.hxx"

class CBaseReader : public i_xml_parser_event_handler
{
public:
    virtual ~CBaseReader();

protected: // protected because its only an implementation relevant class
    CBaseReader( const std::string& DocumentName );

    CBaseReader( StreamInterface *stream );

    virtual void start_document();

    virtual void end_document();

    virtual void start_element(
        const std::wstring& raw_name,
        const std::wstring& local_name,
        const XmlTagAttributes_t& attributes) = 0;

    virtual void end_element(
        const std::wstring& raw_name, const std::wstring& local_name) = 0;

    virtual void characters(const std::wstring& character) = 0;

    virtual void ignore_whitespace(const std::wstring& /*whitespaces*/){};

    virtual void processing_instruction(
        const std::wstring& /*target*/, const std::wstring& /*data*/){};

    virtual void comment(const std::wstring& /*comment*/){};

    void Initialize( const std::string& /*ContentName*/);

private:
    ZipFile        m_ZipFile;
    ZipFile::ZipContentBuffer_t m_ZipContent;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
