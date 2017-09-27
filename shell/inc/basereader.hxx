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

#ifndef INCLUDED_SHELL_INC_INTERNAL_BASEREADER_HXX
#define INCLUDED_SHELL_INC_INTERNAL_BASEREADER_HXX

#include "global.hxx"
#include "types.hxx"
#include "config.hxx"
#include "utilities.hxx"
#include "i_xml_parser_event_handler.hxx"

#include "xml_parser.hxx"
#include "zipfile.hxx"
#include <filepath.hxx>

class CBaseReader : public i_xml_parser_event_handler
{
public:
    virtual ~CBaseReader() override;

protected: // protected because its only an implementation relevant class
    CBaseReader( const Filepath_t& DocumentName );

    CBaseReader( StreamInterface *stream );

    virtual void start_document();

    virtual void end_document();

    virtual void start_element(
        const std::wstring& raw_name,
        const std::wstring& local_name,
        const XmlTagAttributes_t& attributes) override = 0;

    virtual void end_element(
        const std::wstring& raw_name, const std::wstring& local_name) override = 0;

    virtual void characters(const std::wstring& character) override = 0;

    virtual void ignore_whitespace(const std::wstring& /*whitespaces*/) override {};

    virtual void processing_instruction(
        const std::wstring& /*target*/, const std::wstring& /*data*/){};

    virtual void comment(const std::wstring& /*comment*/) override {};

    void Initialize( const std::string& /*ContentName*/);

private:
    ZipFile        m_ZipFile;
    ZipFile::ZipContentBuffer_t m_ZipContent;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
