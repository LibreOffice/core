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

#ifndef _XML_PARSER_HXX_
#define _XML_PARSER_HXX_

#include <expat.h>
#include <stdexcept>

//-----------------------------------------------------
class xml_parser_exception : public std::runtime_error
{
public:

    xml_parser_exception(
        const std::string& error_msg,
        int error_code,
        int line_number,
        int column_number,
        long byte_index) :
        std::runtime_error(error_msg),
        error_code_(error_code),
        line_number_(line_number),
        column_number_(column_number),
        byte_index_(byte_index)
    {}

    int  error_code_;
    int  line_number_;
    int  column_number_;
    long byte_index_;
};


//-----------------------------------------------------
//  Simple wrapper around expat, the xml parser library
//  created by James Clark
//-----------------------------------------------------
class i_xml_parser_event_handler;

class xml_parser
{
public:
    xml_parser(const XML_Char* EncodingName = 0);

    ~xml_parser();

    /** Parse a XML data stream

        @param      pXmlData
                    Pointer to a buffer containing the xml data

        @param      Length
                    Length of the buffer containing the xml data

        @param      IsFinal
                    Indicates whether these are the last xml data
                    of an xml document to parse. For very large
                    xml documents it may be usefull to read and
                    parse the document partially.

        @precond    XmlData must not be null

        @throws     SaxException
                    If the used Sax parser returns an error. The SaxException
                    contains detailed information about the error.  */
    void parse(const char* XmlData, size_t Length, bool IsFinal = true);

    /** Set a document handler

        @descr      A document handler implements the interface i_xml_parser_event_handler.
                    The document handler receive notifications of various events
                    from the sax parser for instance "start_document".

                    The client is responsible for the life time management of
                    the given document handler, that means the document handler
                    instance must exist until a new one was set or until the parser
                    no longer exist.

        @param      SaxDocumentHandler
                    The new document handler, may be null if not interessted in
                    sax parser events.

        @postcond   currently used document handler == pSaxDocumentHandler  */
    void set_document_handler(i_xml_parser_event_handler* event_handler);

    /** Returns the currently used document handler or null if
        no document handler was set before. */
    i_xml_parser_event_handler* get_document_handler() const;
private:

    void init();

private:
    i_xml_parser_event_handler* document_handler_;
    XML_Parser xml_parser_;

// prevent copy and assignment
private:
    xml_parser(const xml_parser&);
    xml_parser& operator=(const xml_parser&);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
