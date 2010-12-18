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

#ifndef INCLUDED_CONFIGMGR_SOURCE_XCUPARSER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XCUPARSER_HXX

#include "sal/config.h"

#include <stack>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "additions.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parser.hxx"
#include "path.hxx"
#include "type.hxx"
#include "valueparser.hxx"
#include "xmldata.hxx"
#include "xmlreader.hxx"

namespace configmgr {

class GroupNode;
class LocalizedPropertyNode;
class Modifications;
class Partial;
class PropertyNode;
class SetNode;
struct Data;
struct Span;

class XcuParser: public Parser {
public:
    XcuParser(
        int layer, Data & data, Partial const * partial,
        Modifications * broadcastModifications, Additions * additions);

private:
    virtual ~XcuParser();

    virtual XmlReader::Text getTextMode();

    virtual bool startElement(
        XmlReader & reader, XmlReader::Namespace ns, Span const & name);

    virtual void endElement(XmlReader const & reader);

    virtual void characters(Span const & span);

    enum Operation {
        OPERATION_MODIFY, OPERATION_REPLACE, OPERATION_FUSE, OPERATION_REMOVE };

    static Operation parseOperation(Span const & text);

    void handleComponentData(XmlReader & reader);

    void handleItem(XmlReader & reader);

    void handlePropValue(XmlReader & reader, PropertyNode * prop);

    void handleLocpropValue(
        XmlReader & reader, LocalizedPropertyNode * locprop);

    void handleGroupProp(XmlReader & reader, GroupNode * group);

    void handleUnknownGroupProp(
        XmlReader const & reader, GroupNode * group, rtl::OUString const & name,
        Type type, Operation operation, bool finalized);

    void handlePlainGroupProp(
        XmlReader const & reader, GroupNode * group,
        NodeMap::iterator const & propertyIndex, rtl::OUString const & name,
        Type type, Operation operation, bool finalized);

    void handleLocalizedGroupProp(
        XmlReader const & reader, LocalizedPropertyNode * property,
        rtl::OUString const & name, Type type, Operation operation,
        bool finalized);

    void handleGroupNode(
        XmlReader & reader, rtl::Reference< Node > const & group);

    void handleSetNode(XmlReader & reader, SetNode * set);

    void recordModification(bool addition);

    struct State {
        rtl::Reference< Node > node; // empty iff ignore or <items>
        rtl::OUString name; // empty and ignored if !insert
        bool ignore;
        bool insert;
        bool locked;
        bool pop;

        inline State(bool thePop):
            ignore(true), insert(false), locked(false), pop(thePop)
        {}

        inline State(rtl::Reference< Node > const & theNode, bool theLocked):
            node(theNode), ignore(false), insert(false), locked(theLocked),
            pop(true)
        {}

        inline State(
            rtl::Reference< Node > const & theNode,
            rtl::OUString const & theName, bool theLocked):
            node(theNode), name(theName), ignore(false), insert(true),
            locked(theLocked), pop(true)
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
    rtl::OUString componentName_;
    StateStack state_;
    Path path_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
