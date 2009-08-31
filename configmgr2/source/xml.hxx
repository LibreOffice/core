/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_XML_HXX
#define INCLUDED_CONFIGMGR_XML_HXX

#include "sal/config.h"

#include <memory>
#include <set>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

#include "span.hxx"
#include "xmlreader.hxx"

namespace configmgr {

struct Data;

namespace xml {

class Parser;

class Reader: public salhelper::SimpleReferenceObject {
public:
    Reader(rtl::OUString const & url, rtl::Reference< Parser > const & parser);

    bool parse();

private:
    virtual ~Reader();

    rtl::OUString url_;
    rtl::Reference< Parser > parser_;
    std::auto_ptr< XmlReader > reader_;
    Span itemData_;
    XmlReader::Namespace itemNamespace_;
};

class Parser: public salhelper::SimpleReferenceObject {
public:
    virtual XmlReader::Text getTextMode() = 0;

    virtual bool startElement(
        XmlReader * reader, XmlReader::Namespace ns, Span const & name) = 0;

    virtual void endElement(XmlReader const * reader) = 0;

    virtual void characters(Span const & text) = 0;

protected:
    Parser();

    virtual ~Parser();
};

class XcdParser: public Parser {
public:
    typedef std::set< rtl::OUString > Dependencies;

    XcdParser(int layer, Dependencies const & dependencies, Data * data);

private:
    virtual ~XcdParser();

    virtual XmlReader::Text getTextMode();

    virtual bool startElement(
        XmlReader * reader, XmlReader::Namespace ns, Span const & name);

    virtual void endElement(XmlReader const * reader);

    virtual void characters(Span const & text);

    enum State {
        STATE_START, STATE_DEPENDENCIES, STATE_DEPENDENCY, STATE_COMPONENTS };

    int layer_;
    Dependencies const & dependencies_;
    Data * data_;
    State state_;
    rtl::OUString dependency_;
    rtl::Reference< Parser > nestedParser_;
    long nesting_;
};

bool decodeXml( //TODO
    rtl::OUString const & encoded, sal_Int32 begin, sal_Int32 end,
    rtl::OUString * decoded);

void parseXcsFile(rtl::OUString const & url, int layer, Data * data);

void parseXcuFile(rtl::OUString const & url, int layer, Data * data);

void parseModFile(rtl::OUString const & url, Data * data);

void writeModFile(rtl::OUString const & url, Data const & data);

}

}

#endif
