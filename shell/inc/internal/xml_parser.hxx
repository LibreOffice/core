/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    //########################################################
    xml_parser(const XML_Char* EncodingName = 0);

    //########################################################
    ~xml_parser();

    //########################################################
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

    //########################################################
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

    //########################################################
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
