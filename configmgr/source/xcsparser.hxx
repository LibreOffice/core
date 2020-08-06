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

#pragma once

#include <sal/config.h>

#include <set>
#include <stack>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmlreader/xmlreader.hxx>

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
    virtual ~XcsParser() override;

    virtual xmlreader::XmlReader::Text getTextMode() override;

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
        std::set< OUString > const * existingDependencies) override;

    virtual void endElement(xmlreader::XmlReader const & reader) override;

    virtual void characters(xmlreader::Span const & text) override;

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
        OUString name;

        Element(
            rtl::Reference< Node > const & theNode,
            OUString const & theName):
            node(theNode), name(theName) {}
    };

    typedef std::stack< Element > ElementStack;

    ValueParser valueParser_;
    Data & data_;
    OUString componentName_;
    State state_;
    long ignoring_;
    ElementStack elements_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
