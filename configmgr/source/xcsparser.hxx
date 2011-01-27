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

#include "node.hxx"
#include "parser.hxx"
#include "valueparser.hxx"
#include "xmlreader.hxx"

namespace configmgr {

class SetNode;
struct Data;
struct Span;

class XcsParser: public Parser {
public:
    XcsParser(int layer, Data & data);

private:
    virtual ~XcsParser();

    virtual XmlReader::Text getTextMode();

    virtual bool startElement(
        XmlReader & reader, XmlReader::Namespace ns, Span const & name);

    virtual void endElement(XmlReader const & reader);

    virtual void characters(Span const & text);

    void handleComponentSchema(XmlReader & reader);

    void handleNodeRef(XmlReader & reader);

    void handleProp(XmlReader & reader);

    void handlePropValue(
        XmlReader & reader, rtl::Reference< Node > const & property);

    void handleGroup(XmlReader & reader, bool isTemplate);

    void handleSet(XmlReader & reader, bool isTemplate);

    void handleSetItem(XmlReader & reader, SetNode * set);

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
