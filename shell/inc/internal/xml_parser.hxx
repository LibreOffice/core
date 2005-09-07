/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xml_parser.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:40:12 $
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

#ifndef _XML_PARSER_HXX_
#define _XML_PARSER_HXX_

#ifdef SYSTEM_EXPAT
#include <expat.h>
#else
#include <external/expat/xmlparse.h>
#endif
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
    xml_parser(const XML_Char* EncodingName, XML_Char NamespaceSeparator);

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

    //############################################
    void set_encoding(const XML_Char* Encoding);

private:

    void init();

    //####################################
    // callback functions
    static void xml_start_element_handler(void* UserData, const XML_Char* name, const XML_Char** atts);
    static void xml_end_element_handler(void* UserData, const XML_Char* name);
    static void xml_character_data_handler(void* UserData, const XML_Char* s, int len);
    static void xml_comment_handler(void* UserData, const XML_Char* Data);

private:
    i_xml_parser_event_handler* document_handler_;
    XML_Parser xml_parser_;

// prevent copy and assignment
private:
    xml_parser(const xml_parser&);
    xml_parser& operator=(const xml_parser&);
};

#endif

