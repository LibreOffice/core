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

#ifndef INCLUDED_CONFIGMGR_SOURCE_XCSPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCSPARSER_HXX

#include "sal/config.h"

#include <stack>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "xmlreader/xmlreader.hxx"

#include "node.hxx"
#include "parser.hxx"
#include "valueparser.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

class SetNode;
struct Data;

class XcsParser: public Parser {
public:
    XcsParser(int layer, Data & data);

private:
    virtual ~XcsParser();

    virtual xmlreader::XmlReader::Text getTextMode();

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name);

    virtual void endElement(xmlreader::XmlReader const & reader);

    virtual void characters(xmlreader::Span const & text);

    void handleComponentSchema(xmlreader::XmlReader & reader);

    void handleNodeRef(xmlreader::XmlReader & reader);

    void handleProp(xmlreader::XmlReader & reader);

    void handlePropValue(
        xmlreader::XmlReader & reader, rtl::Reference< Node > const & property);

    void handleGroup(xmlreader::XmlReader & reader, bool isTemplate);

    void handleSet(xmlreader::XmlReader & reader, bool isTemplate);

    void handleSetItem(xmlreader::XmlReader & reader, SetNode * set);

    enum State {
        STATE_START, STATE_COMPONENT_SCHEMA, STATE_TEMPLATES,
        STATE_TEMPLATES_DONE, STATE_COMPONENT, STATE_COMPONENT_DONE };

    struct Element {
        rtl::Reference< Node > node;
        rtl::OUString name;

        inline Element(
            rtl::Reference< Node > const & theNode,
            rtl::OUString const & theName):
            node(theNode), name(theName) {}
    };

    typedef std::stack< Element > ElementStack;

    ValueParser valueParser_;
    Data & data_;
    rtl::OUString componentName_;
    State state_;
    long ignoring_;
    ElementStack elements_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
