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

#ifndef INCLUDED_CONFIGMGR_SOURCE_XCUPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCUPARSER_HXX

#include <sal/config.h>

#include <set>
#include <stack>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmlreader/xmlreader.hxx>

#include "additions.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parser.hxx"
#include "path.hxx"
#include "type.hxx"
#include "valueparser.hxx"
#include "xmldata.hxx"

namespace xmlreader { struct Span; }

namespace configmgr {

class GroupNode;
class LocalizedPropertyNode;
class Modifications;
class Partial;
class PropertyNode;
class SetNode;
struct Data;

class XcuParser: public Parser {
public:
    XcuParser(
        int layer, Data & data, Partial const * partial,
        Modifications * broadcastModifications, Additions * additions);

private:
    virtual ~XcuParser();

    virtual xmlreader::XmlReader::Text getTextMode() override;

    virtual bool startElement(
        xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
        std::set< OUString > const * existingDependencies) override;

    virtual void endElement(xmlreader::XmlReader const & reader) override;

    virtual void characters(xmlreader::Span const & span) override;

    enum Operation {
        OPERATION_MODIFY, OPERATION_REPLACE, OPERATION_FUSE, OPERATION_REMOVE };

    static Operation parseOperation(xmlreader::Span const & text);

    void handleComponentData(xmlreader::XmlReader & reader);

    void handleItem(xmlreader::XmlReader & reader);

    void handlePropValue(xmlreader::XmlReader & reader, PropertyNode * prop);

    void handleLocpropValue(
        xmlreader::XmlReader & reader, LocalizedPropertyNode * locprop);

    void handleGroupProp(xmlreader::XmlReader & reader, GroupNode * group);

    void handleUnknownGroupProp(
        xmlreader::XmlReader const & reader, GroupNode * group,
        OUString const & name, Type type, Operation operation,
        bool finalized);

    void handlePlainGroupProp(
        xmlreader::XmlReader const & reader, GroupNode * group,
        NodeMap::iterator const & propertyIndex, OUString const & name,
        Type type, Operation operation, bool finalized);

    void handleLocalizedGroupProp(
        xmlreader::XmlReader const & reader, LocalizedPropertyNode * property,
        OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleGroupNode(
        xmlreader::XmlReader & reader, rtl::Reference< Node > const & group);

    void handleSetNode(xmlreader::XmlReader & reader, SetNode * set);

    void recordModification(bool addition);

    struct State {
        rtl::Reference< Node > node; // empty if ignore or <items>
        OUString name; // empty and ignored if !insert
        bool ignore;
        bool insert;
        bool pop;

        static State Ignore(bool thePop) { return State(thePop); }

        static State Modify(rtl::Reference< Node > const & theNode)
        { return State(theNode); }

        static State Insert(
            rtl::Reference< Node > const & theNode, OUString const & theName)
        { return State(theNode, theName); }

    private:
        explicit State(bool thePop): ignore(true), insert(false), pop(thePop) {}

        explicit State(rtl::Reference< Node > const & theNode):
            node(theNode), ignore(false), insert(false), pop(true)
        {}

        State(
            rtl::Reference< Node > const & theNode, OUString const & theName):
            node(theNode), name(theName), ignore(false), insert(true), pop(true)
        {}
    };

    typedef std::stack< State > StateStack;

    ValueParser valueParser_;
    Data & data_;
    Partial const * partial_;
    Modifications * broadcastModifications_;
    Additions * additions_;
    bool recordModifications_;
    bool trackPath_;
    OUString componentName_;
    StateStack state_;
    Path path_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
