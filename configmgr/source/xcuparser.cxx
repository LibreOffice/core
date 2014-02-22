/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <set>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

#include "data.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "groupnode.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "partial.hxx"
#include "path.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "xcuparser.hxx"
#include "xmldata.hxx"

namespace configmgr {

XcuParser::XcuParser(
    int layer, Data & data, Partial const * partial,
    Modifications * broadcastModifications, Additions * additions):
    valueParser_(layer), data_(data),
    partial_(partial), broadcastModifications_(broadcastModifications),
    additions_(additions), recordModifications_(layer == Data::NO_LAYER),
    trackPath_(
        partial_ != 0 || broadcastModifications_ != 0 || additions_ != 0 ||
        recordModifications_)
{}

XcuParser::~XcuParser() {}

xmlreader::XmlReader::Text XcuParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcuParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
    std::set< OUString > const * existingDependencies)
{
    if (valueParser_.startElement(reader, nsId, name, existingDependencies)) {
        return true;
    }
    if (state_.empty()) {
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals("component-data"))
        {
            handleComponentData(reader);
        } else if (nsId == ParseManager::NAMESPACE_OOR && name.equals("items"))
        {
            state_.push(State::Modify(rtl::Reference< Node >()));
        } else {
            throw css::uno::RuntimeException(
                ("bad root element <" + name.convertFromUtf8() + "> in " +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (state_.top().ignore) {
        state_.push(State::Ignore(false));
    } else if (!state_.top().node.is()) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE && name.equals("item"))
        {
            handleItem(reader);
        } else {
            throw css::uno::RuntimeException(
                ("bad items node member <" + name.convertFromUtf8() + "> in " +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        switch (state_.top().node->kind()) {
        case Node::KIND_PROPERTY:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("value"))
            {
                handlePropValue(
                    reader,
                    static_cast< PropertyNode * >(state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    ("bad property node member <" + name.convertFromUtf8() +
                     "> in " + reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("value"))
            {
                handleLocpropValue(
                    reader,
                    static_cast< LocalizedPropertyNode * >(
                        state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    ("bad localized property node member <" +
                     name.convertFromUtf8() + "> in " + reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            throw css::uno::RuntimeException(
                ("bad member <" + name.convertFromUtf8() + "> in " +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case Node::KIND_GROUP:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("prop"))
            {
                handleGroupProp(
                    reader,
                    static_cast< GroupNode * >(state_.top().node.get()));
            } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                       name.equals("node"))
            {
                handleGroupNode(reader, state_.top().node);
            } else {
                throw css::uno::RuntimeException(
                    ("bad group node member <" + name.convertFromUtf8() +
                     "> in " + reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_SET:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("node"))
            {
                handleSetNode(
                    reader, static_cast< SetNode * >(state_.top().node.get()));
            } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                       name.equals("prop"))
            {
                SAL_WARN(
                    "configmgr",
                    "bad set node <prop> member in \"" << reader.getUrl()
                        << '"');
                state_.push(State::Ignore(true));
            } else {
                throw css::uno::RuntimeException(
                    ("bad set node member <" + name.convertFromUtf8() +
                     "> in " + reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_ROOT:
            assert(false); 
            break;
        }
    }
    return true;
}

void XcuParser::endElement(xmlreader::XmlReader const &) {
    if (valueParser_.endElement()) {
        return;
    }
    assert(!state_.empty());
    bool pop = state_.top().pop;
    rtl::Reference< Node > insert;
    OUString name;
    if (state_.top().insert) {
        insert = state_.top().node;
        assert(insert.is());
        name = state_.top().name;
    }
    state_.pop();
    if (insert.is()) {
        assert(!state_.empty() && state_.top().node.is());
        state_.top().node->getMembers()[name] = insert;
    }
    if (pop && !path_.empty()) {
        path_.pop_back();
            
            
    }
}

void XcuParser::characters(xmlreader::Span const & text) {
    valueParser_.characters(text);
}

XcuParser::Operation XcuParser::parseOperation(xmlreader::Span const & text) {
    assert(text.is());
    if (text.equals("modify")) {
        return OPERATION_MODIFY;
    }
    if (text.equals("replace")) {
        return OPERATION_REPLACE;
    }
    if (text.equals("fuse")) {
        return OPERATION_FUSE;
    }
    if (text.equals("remove")) {
        return OPERATION_REMOVE;
    }
    throw css::uno::RuntimeException(
        "invalid op " + text.convertFromUtf8(),
        css::uno::Reference< css::uno::XInterface >());
}

void XcuParser::handleComponentData(xmlreader::XmlReader & reader) {
    OStringBuffer buf;
    buf.append('.');
    bool hasPackage = false;
    bool hasName = false;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn.equals("package"))
        {
            if (hasPackage) {
                throw css::uno::RuntimeException(
                    ("multiple component-update package attributes in " +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            hasPackage = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.insert(0, s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("name"))
        {
            if (hasName) {
                throw css::uno::RuntimeException(
                    ("multiple component-update name attributes in " +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            hasName = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.append(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("op"))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("finalized"))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasPackage) {
        throw css::uno::RuntimeException(
            "no component-data package attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no component-data name attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    componentName_ = xmlreader::Span(buf.getStr(), buf.getLength()).
        convertFromUtf8();
    if (trackPath_) {
        assert(path_.empty());
        path_.push_back(componentName_);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State::Ignore(true));
            return;
        }
    }
    rtl::Reference< Node > node(
        Data::findNode(
            valueParser_.getLayer(), data_.getComponents(), componentName_));
    if (!node.is()) {
        SAL_WARN(
            "configmgr",
            "unknown component \"" << componentName_ << "\" in \""
                << reader.getUrl() << '"');
        state_.push(State::Ignore(true));
        return;
    }
    switch (op) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        break;
    default:
        throw css::uno::RuntimeException(
            "invalid operation on root node in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        node->getFinalized());
    node->setFinalized(finalizedLayer);
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    state_.push(State::Modify(node));
}

void XcuParser::handleItem(xmlreader::XmlReader & reader) {
    xmlreader::Span attrPath;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn.equals("path")) {
            attrPath = reader.getAttributeValue(false);
        }
    }
    if (!attrPath.is()) {
        throw css::uno::RuntimeException(
            "missing path attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    OUString path(attrPath.convertFromUtf8());
    int finalizedLayer;
    rtl::Reference< Node > node(
        data_.resolvePathRepresentation(
            path, 0, &path_, &finalizedLayer));
    if (!node.is()) {
        SAL_WARN(
            "configmgr",
            "unknown item \"" << path << "\" in \"" << reader.getUrl() << '"');
        state_.push(State::Ignore(true));
        return;
    }
    assert(!path_.empty());
    componentName_ = path_.front();
    if (trackPath_) {
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State::Ignore(true));
            return;
        }
    } else {
        path_.clear();
    }
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
    case Node::KIND_LOCALIZED_VALUE:
        SAL_WARN(
            "configmgr",
            "item of bad type \"" << path << "\" in \"" << reader.getUrl()
                << '"');
        state_.push(State::Ignore(true));
        return;
    case Node::KIND_LOCALIZED_PROPERTY:
        valueParser_.type_ = static_cast< LocalizedPropertyNode * >(
            node.get())->getStaticType();
        break;
    default:
        break;
    }
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    state_.push(State::Modify(node));
}

void XcuParser::handlePropValue(
    xmlreader::XmlReader & reader, PropertyNode * prop)
 {
    bool nil = false;
    OString separator;
    OUString external;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_XSI && attrLn.equals("nil")) {
            nil = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("type"))
        {
            Type type = xmldata::parseType(
                reader, reader.getAttributeValue(true));
            if (valueParser_.type_ != TYPE_ANY && type != valueParser_.type_) {
                throw css::uno::RuntimeException(
                    "invalid value type in " + reader.getUrl(),
                    css::uno::Reference< css::uno::XInterface >());
            }
            valueParser_.type_ = type;
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("separator"))
        {
            xmlreader::Span s(reader.getAttributeValue(false));
            if (s.length == 0) {
                throw css::uno::RuntimeException(
                    "bad oor:separator attribute in " + reader.getUrl(),
                    css::uno::Reference< css::uno::XInterface >());
            }
            separator = OString(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("external"))
        {
            external = reader.getAttributeValue(true).convertFromUtf8();
            if (external.isEmpty()) {
                throw css::uno::RuntimeException(
                    "bad oor:external attribute value in " + reader.getUrl(),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    }
    if (nil) {
        if (!prop->isNillable()) {
            throw css::uno::RuntimeException(
                "xsi:nil attribute for non-nillable prop in " + reader.getUrl(),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (!external.isEmpty()) {
            throw css::uno::RuntimeException(
                ("xsi:nil and oor:external attributes for prop in " +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        prop->setValue(valueParser_.getLayer(), css::uno::Any());
        state_.push(State::Ignore(false));
    } else if (external.isEmpty()) {
        valueParser_.separator_ = separator;
        valueParser_.start(prop);
    } else {
        prop->setExternal(valueParser_.getLayer(), external);
        state_.push(State::Ignore(false));
    }
}

void XcuParser::handleLocpropValue(
    xmlreader::XmlReader & reader, LocalizedPropertyNode * locprop)
{
    OUString name;
    bool nil = false;
    OString separator;
    Operation op = OPERATION_FUSE;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == xmlreader::XmlReader::NAMESPACE_XML &&
            attrLn.equals("lang"))
        {
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_XSI &&
                   attrLn.equals("nil"))
        {
            nil = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("type"))
        {
            Type type = xmldata::parseType(
                reader, reader.getAttributeValue(true));
            if (valueParser_.type_ != TYPE_ANY && type != valueParser_.type_) {
                throw css::uno::RuntimeException(
                    "invalid value type in " + reader.getUrl(),
                    css::uno::Reference< css::uno::XInterface >());
            }
            valueParser_.type_ = type;
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("separator"))
        {
            xmlreader::Span s(reader.getAttributeValue(false));
            if (s.length == 0) {
                throw css::uno::RuntimeException(
                    "bad oor:separator attribute in " + reader.getUrl(),
                    css::uno::Reference< css::uno::XInterface >());
            }
            separator = OString(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("op"))
        {
            op = parseOperation(reader.getAttributeValue(true));
        }
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 &&
            partial_->contains(path_) != Partial::CONTAINS_NODE)
        {
            state_.push(State::Ignore(true));
            return;
        }
    }
    NodeMap & members = locprop->getMembers();
    NodeMap::iterator i(members.find(name));
    if (i != members.end() && i->second->getLayer() > valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    if (nil && !locprop->isNillable()) {
        throw css::uno::RuntimeException(
            "xsi:nil attribute for non-nillable prop in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (op) {
    case OPERATION_FUSE:
        {
            bool pop = false;
            if (nil) {
                if (i == members.end()) {
                    members[name] = new LocalizedValueNode(
                        valueParser_.getLayer(), css::uno::Any());
                } else {
                    static_cast< LocalizedValueNode * >(
                        i->second.get())->setValue(
                            valueParser_.getLayer(), css::uno::Any());
                }
                state_.push(State::Ignore(true));
            } else {
                valueParser_.separator_ = separator;
                valueParser_.start(locprop, name);
                pop = true;
            }
            if (trackPath_) {
                recordModification(false);
                if (pop) {
                    path_.pop_back();
                }
            }
        }
        break;
    case OPERATION_REMOVE:
        
        
        if (i != members.end()) {
            members.erase(i);
        }
        state_.push(State::Ignore(true));
        recordModification(false);
        break;
    default:
        throw css::uno::RuntimeException(
            "bad op attribute for value element in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupProp(
    xmlreader::XmlReader & reader, GroupNode * group)
{
    bool hasName = false;
    OUString name;
    Type type = TYPE_ERROR;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn.equals("name")) {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("type"))
        {
            type = xmldata::parseType(reader, reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("op"))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("finalized"))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no prop name attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        
        
        if (partial_ != 0 &&
            partial_->contains(path_) != Partial::CONTAINS_NODE)
        {
            state_.push(State::Ignore(true));
            return;
        }
    }
    NodeMap & members = group->getMembers();
    NodeMap::iterator i(members.find(name));
    if (i == members.end()) {
        handleUnknownGroupProp(reader, group, name, type, op, finalized);
    } else {
        switch (i->second->kind()) {
        case Node::KIND_PROPERTY:
            handlePlainGroupProp(reader, group, i, name, type, op, finalized);
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            handleLocalizedGroupProp(
                reader,
                static_cast< LocalizedPropertyNode * >(i->second.get()), name,
                type, op, finalized);
            break;
        default:
            throw css::uno::RuntimeException(
                "inappropriate prop " + name + " in " + reader.getUrl(),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void XcuParser::handleUnknownGroupProp(
    xmlreader::XmlReader const & reader, GroupNode * group,
    OUString const & name, Type type, Operation operation, bool finalized)
{
    switch (operation) {
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        if (group->isExtensible()) {
            if (type == TYPE_ERROR) {
                throw css::uno::RuntimeException(
                    ("missing type attribute for prop " + name + " in " +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            valueParser_.type_ = type;
            rtl::Reference< Node > prop(
                new PropertyNode(
                    valueParser_.getLayer(), TYPE_ANY, true, css::uno::Any(),
                    true));
            if (finalized) {
                prop->setFinalized(valueParser_.getLayer());
            }
            state_.push(State::Insert(prop, name));
            recordModification(false);
            break;
        }
        
    default:
        SAL_WARN(
            "configmgr",
            "unknown property \"" << name << "\" in \"" << reader.getUrl()
                << '"');
        state_.push(State::Ignore(true));
        break;
    }
}

void XcuParser::handlePlainGroupProp(
    xmlreader::XmlReader const & reader, GroupNode * group,
    NodeMap::iterator const & propertyIndex, OUString const & name,
    Type type, Operation operation, bool finalized)
{
    PropertyNode * property = static_cast< PropertyNode * >(
        propertyIndex->second.get());
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    if (type != TYPE_ERROR && property->getStaticType() != TYPE_ANY &&
        type != property->getStaticType())
    {
        throw css::uno::RuntimeException(
            "invalid type for prop " + name + " in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getStaticType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        state_.push(State::Modify(property));
        recordModification(false);
        break;
    case OPERATION_REMOVE:
        if (!property->isExtension()) {
            throw css::uno::RuntimeException(
                ("invalid remove of non-extension prop " + name + " in " +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        group->getMembers().erase(propertyIndex);
        state_.push(State::Ignore(true));
        recordModification(false);
        break;
    }
}

void XcuParser::handleLocalizedGroupProp(
    xmlreader::XmlReader const & reader, LocalizedPropertyNode * property,
    OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    if (type != TYPE_ERROR && property->getStaticType() != TYPE_ANY &&
        type != property->getStaticType())
    {
        throw css::uno::RuntimeException(
            "invalid type for prop " + name + " in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getStaticType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        state_.push(State::Modify(property));
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > replacement(
                new LocalizedPropertyNode(
                    valueParser_.getLayer(), property->getStaticType(),
                    property->isNillable()));
            replacement->setFinalized(property->getFinalized());
            state_.push(State::Insert(replacement, name));
            recordModification(false);
        }
        break;
    case OPERATION_REMOVE:
        throw css::uno::RuntimeException(
            ("invalid remove of non-extension prop " + name + " in " +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupNode(
    xmlreader::XmlReader & reader, rtl::Reference< Node > const & group)
{
    bool hasName = false;
    OUString name;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn.equals("name")) {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("op"))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("finalized"))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no node name attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State::Ignore(true));
            return;
        }
    }
    rtl::Reference< Node > child(
        Data::findNode(valueParser_.getLayer(), group->getMembers(), name));
    if (!child.is()) {
        SAL_WARN(
            "configmgr",
            "unknown node \"" << name << "\" in \"" << reader.getUrl() << '"');
        state_.push(State::Ignore(true));
        return;
    }
    Node::Kind kind = child->kind();
    if (kind != Node::KIND_GROUP && kind != Node::KIND_SET) {
        throw css::uno::RuntimeException(
            ("bad <node> \"" + name + "\" of non group/set kind in " +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (op != OPERATION_MODIFY && op != OPERATION_FUSE) {
        throw css::uno::RuntimeException(
            "invalid operation on group node in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        child->getFinalized());
    child->setFinalized(finalizedLayer);
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    state_.push(State::Modify(child));
}

void XcuParser::handleSetNode(xmlreader::XmlReader & reader, SetNode * set) {
    bool hasName = false;
    OUString name;
    OUString component(componentName_);
    bool hasNodeType = false;
    OUString nodeType;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    bool mandatory = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn.equals("name")) {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("component"))
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("node-type"))
        {
            hasNodeType = true;
            nodeType = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("op"))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("finalized"))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("mandatory"))
        {
            mandatory = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no node name attribute in " + reader.getUrl(),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State::Ignore(true));
            return;
        }
    }
    OUString templateName(
        xmldata::parseTemplateReference(
            component, hasNodeType, nodeType, &set->getDefaultTemplateName()));
    if (!set->isValidTemplate(templateName)) {
        throw css::uno::RuntimeException(
            ("set member node " + name + " references invalid template " +
             templateName + " in " + reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > tmpl(
        data_.getTemplate(valueParser_.getLayer(), templateName));
    if (!tmpl.is()) {
        throw css::uno::RuntimeException(
            ("set member node " + name + " references undefined template " +
             templateName + " in " + reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = finalized ? valueParser_.getLayer() : Data::NO_LAYER;
    int mandatoryLayer = mandatory ? valueParser_.getLayer() : Data::NO_LAYER;
    NodeMap & members = set->getMembers();
    NodeMap::iterator i(members.find(name));
    if (i != members.end()) {
        finalizedLayer = std::min(finalizedLayer, i->second->getFinalized());
        i->second->setFinalized(finalizedLayer);
        mandatoryLayer = std::min(mandatoryLayer, i->second->getMandatory());
        i->second->setMandatory(mandatoryLayer);
        if (i->second->getLayer() > valueParser_.getLayer()) {
            state_.push(State::Ignore(true));
            return;
        }
    }
    if (finalizedLayer < valueParser_.getLayer()) {
        state_.push(State::Ignore(true));
        return;
    }
    switch (op) {
    case OPERATION_MODIFY:
        if (i == members.end()) {
            SAL_WARN(
                "configmgr",
                "ignoring modify of unknown set member node \"" << name
                    << "\" in \"" << reader.getUrl() << '"');
            state_.push(State::Ignore(true));
        } else {
            state_.push(State::Modify(i->second));
        }
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > member(tmpl->clone(true));
            member->setLayer(valueParser_.getLayer());
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State::Insert(member, name));
            recordModification(i == members.end());
        }
        break;
    case OPERATION_FUSE:
        if (i == members.end()) {
            rtl::Reference< Node > member(tmpl->clone(true));
            member->setLayer(valueParser_.getLayer());
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State::Insert(member, name));
            recordModification(true);
        } else {
            state_.push(State::Modify(i->second));
        }
        break;
    case OPERATION_REMOVE:
        {
            
            
            
            
            bool known = i != members.end();
            if (known &&
                (mandatoryLayer == Data::NO_LAYER ||
                 mandatoryLayer > valueParser_.getLayer()))
            {
                members.erase(i);
            }
            state_.push(State::Ignore(true));
            if (known) {
                recordModification(false);
            }
            break;
        }
    }
}

void XcuParser::recordModification(bool addition) {
    if (broadcastModifications_ != 0) {
        broadcastModifications_->add(path_);
    }
    if (addition && additions_ != 0) {
        additions_->push_back(path_);
    }
    if (recordModifications_) {
        data_.modifications.add(path_);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
