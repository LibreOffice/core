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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <set>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <xmlreader/span.hxx>
#include <xmlreader/xmlreader.hxx>

#include "data.hxx"
#include "localizedpropertynode.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "parsemanager.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "xcsparser.hxx"
#include "xmldata.hxx"

namespace configmgr {

namespace {

// Conservatively merge a template or component (and its recursive parts) into
// an existing instance:
void merge(
    rtl::Reference< Node > const & original,
    rtl::Reference< Node > const & update)
{
    assert(
        original.is() && update.is() && original->kind() == update->kind() &&
        update->getFinalized() == Data::NO_LAYER);
    if (update->getLayer() >= original->getLayer() &&
        update->getLayer() <= original->getFinalized())
    {
        switch (original->kind()) {
        case Node::KIND_PROPERTY:
        case Node::KIND_LOCALIZED_PROPERTY:
        case Node::KIND_LOCALIZED_VALUE:
            break; //TODO: merge certain parts?
        case Node::KIND_GROUP:
            for (NodeMap::const_iterator i2(update->getMembers().begin());
                 i2 != update->getMembers().end(); ++i2)
            {
                NodeMap & members = original->getMembers();
                NodeMap::iterator i1(members.find(i2->first));
                if (i1 == members.end()) {
                    if (i2->second->kind() == Node::KIND_PROPERTY &&
                        static_cast< GroupNode * >(
                            original.get())->isExtensible())
                    {
                        members.insert(*i2);
                    }
                } else if (i2->second->kind() == i1->second->kind()) {
                    merge(i1->second, i2->second);
                }
            }
            break;
        case Node::KIND_SET:
            for (NodeMap::const_iterator i2(update->getMembers().begin());
                 i2 != update->getMembers().end(); ++i2)
            {
                NodeMap & members = original->getMembers();
                NodeMap::iterator i1(members.find(i2->first));
                if (i1 == members.end()) {
                    if (static_cast< SetNode * >(original.get())->
                        isValidTemplate(i2->second->getTemplateName()))
                    {
                        members.insert(*i2);
                    }
                } else if (i2->second->kind() == i1->second->kind() &&
                           (i2->second->getTemplateName() ==
                            i1->second->getTemplateName()))
                {
                    merge(i1->second, i2->second);
                }
            }
            break;
        case Node::KIND_ROOT:
            assert(false); // this cannot happen
            break;
        }
    }
}

}

XcsParser::XcsParser(int layer, Data & data):
    valueParser_(layer), data_(data), state_(STATE_START), ignoring_()
{}

XcsParser::~XcsParser() {}

xmlreader::XmlReader::Text XcsParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcsParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
    std::set< OUString > const * existingDependencies)
{
    if (valueParser_.startElement(reader, nsId, name, existingDependencies)) {
        return true;
    }
    if (state_ == STATE_START) {
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals("component-schema"))
        {
            handleComponentSchema(reader);
            state_ = STATE_COMPONENT_SCHEMA;
            ignoring_ = 0;
            return true;
        }
    } else {
        //TODO: ignoring component-schema import, component-schema uses, and
        // prop constraints; accepting all four at illegal places (and with
        // illegal content):
        if (ignoring_ > 0 ||
            (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
             (name.equals("info") || name.equals("import") ||
              name.equals("uses") || name.equals("constraints"))))
        {
            assert(ignoring_ < LONG_MAX);
            ++ignoring_;
            return true;
        }
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("templates"))
            {
                state_ = STATE_TEMPLATES;
                return true;
            }
            SAL_FALLTHROUGH;
        case STATE_TEMPLATES_DONE:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals("component"))
            {
                state_ = STATE_COMPONENT;
                assert(elements_.empty());
                elements_.push(
                    Element(
                        new GroupNode(valueParser_.getLayer(), false, ""),
                        componentName_));
                return true;
            }
            break;
        case STATE_TEMPLATES:
            if (elements_.empty()) {
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("group"))
                {
                    handleGroup(reader, true);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("set"))
                {
                    handleSet(reader, true);
                    return true;
                }
                break;
            }
            SAL_FALLTHROUGH;
        case STATE_COMPONENT:
            assert(!elements_.empty());
            switch (elements_.top().node->kind()) {
            case Node::KIND_PROPERTY:
            case Node::KIND_LOCALIZED_PROPERTY:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("value"))
                {
                    handlePropValue(reader, elements_.top().node);
                    return true;
                }
                break;
            case Node::KIND_GROUP:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("prop"))
                {
                    handleProp(reader);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("node-ref"))
                {
                    handleNodeRef(reader);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("group"))
                {
                    handleGroup(reader, false);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("set"))
                {
                    handleSet(reader, false);
                    return true;
                }
                break;
            case Node::KIND_SET:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name.equals("item"))
                {
                    handleSetItem(
                        reader,
                        static_cast< SetNode * >(elements_.top().node.get()));
                    return true;
                }
                break;
            default: // Node::KIND_LOCALIZED_VALUE
                assert(false); // this cannot happen
                break;
            }
            break;
        case STATE_COMPONENT_DONE:
            break;
        default: // STATE_START
            assert(false); // this cannot happen
            break;
        }
    }
    throw css::uno::RuntimeException(
        "bad member <" + name.convertFromUtf8() + "> in " + reader.getUrl());
}

void XcsParser::endElement(xmlreader::XmlReader const & reader) {
    if (valueParser_.endElement()) {
        return;
    }
    if (ignoring_ > 0) {
        --ignoring_;
    } else if (!elements_.empty()) {
        Element top(elements_.top());
        elements_.pop();
        if (top.node.is()) {
            if (elements_.empty()) {
                switch (state_) {
                case STATE_TEMPLATES:
                    {
                        NodeMap::iterator i(data_.templates.find(top.name));
                        if (i == data_.templates.end()) {
                            data_.templates.insert(
                                NodeMap::value_type(top.name, top.node));
                        } else {
                            merge(i->second, top.node);
                        }
                    }
                    break;
                case STATE_COMPONENT:
                    {
                        NodeMap & components = data_.getComponents();
                        NodeMap::iterator i(components.find(top.name));
                        if (i == components.end()) {
                            components.insert(
                                NodeMap::value_type(top.name, top.node));
                        } else {
                            merge(i->second, top.node);
                        }
                        state_ = STATE_COMPONENT_DONE;
                    }
                    break;
                default:
                    assert(false);
                    throw css::uno::RuntimeException(
                        "this cannot happen");
                }
            } else {
                if (!elements_.top().node->getMembers().insert(
                        NodeMap::value_type(top.name, top.node)).second)
                {
                    throw css::uno::RuntimeException(
                        "duplicate " + top.name + " in " + reader.getUrl());
                }
            }
        }
    } else {
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            // To support old, broken extensions with .xcs files that contain
            // empty <component-schema> elements:
            state_ = STATE_COMPONENT_DONE;
            break;
        case STATE_TEMPLATES:
            state_ = STATE_TEMPLATES_DONE;
            break;
        case STATE_TEMPLATES_DONE:
            throw css::uno::RuntimeException(
                "no component element in " + reader.getUrl());
        case STATE_COMPONENT_DONE:
            break;
        default:
            assert(false); // this cannot happen
        }
    }
}

void XcsParser::characters(xmlreader::Span const & text) {
    valueParser_.characters(text);
}

void XcsParser::handleComponentSchema(xmlreader::XmlReader & reader) {
    //TODO: oor:version, xml:lang attributes
    OStringBuffer buf;
    buf.append('.');
    bool hasPackage = false;
    bool hasName = false;
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
                    "multiple component-schema package attributes in " +
                    reader.getUrl());
            }
            hasPackage = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.insert(0, s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("name"))
        {
            if (hasName) {
                throw css::uno::RuntimeException(
                    "multiple component-schema name attributes in " +
                    reader.getUrl());
            }
            hasName = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.append(s.begin, s.length);
        }
    }
    if (!hasPackage) {
        throw css::uno::RuntimeException(
            "no component-schema package attribute in " + reader.getUrl());
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no component-schema name attribute in " + reader.getUrl());
    }
    componentName_ = xmlreader::Span(buf.getStr(), buf.getLength()).
        convertFromUtf8();
}

void XcsParser::handleNodeRef(xmlreader::XmlReader & reader) {
    bool hasName = false;
    OUString name;
    OUString component(componentName_);
    bool hasNodeType = false;
    OUString nodeType;
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
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no node-ref name attribute in " + reader.getUrl());
    }
    rtl::Reference< Node > tmpl(
        data_.getTemplate(
            valueParser_.getLayer(),
            xmldata::parseTemplateReference(
                component, hasNodeType, nodeType, nullptr)));
    if (!tmpl.is()) {
        //TODO: this can erroneously happen as long as import/uses attributes
        // are not correctly processed
        throw css::uno::RuntimeException(
            "unknown node-ref " + name + " in " + reader.getUrl());
    }
    rtl::Reference< Node > node(tmpl->clone(false));
    node->setLayer(valueParser_.getLayer());
    elements_.push(Element(node, name));
}

void XcsParser::handleProp(xmlreader::XmlReader & reader) {
    bool hasName = false;
    OUString name;
    valueParser_.type_ = TYPE_ERROR;
    bool localized = false;
    bool nillable = true;
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
            valueParser_.type_ = xmldata::parseType(
                reader, reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("localized"))
        {
            localized = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("nillable"))
        {
            nillable = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no prop name attribute in " + reader.getUrl());
    }
    if (valueParser_.type_ == TYPE_ERROR) {
        throw css::uno::RuntimeException(
            "no prop type attribute in " + reader.getUrl());
    }
    elements_.push(
        Element(
            (localized
             ? rtl::Reference< Node >(
                 new LocalizedPropertyNode(
                     valueParser_.getLayer(), valueParser_.type_, nillable))
             : rtl::Reference< Node >(
                 new PropertyNode(
                     valueParser_.getLayer(), valueParser_.type_, nillable,
                     css::uno::Any(), false))),
            name));
}

void XcsParser::handlePropValue(
    xmlreader::XmlReader & reader, rtl::Reference< Node > const & property)
{
    xmlreader::Span attrSeparator;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals("separator"))
        {
            attrSeparator = reader.getAttributeValue(false);
            if (attrSeparator.length == 0) {
                throw css::uno::RuntimeException(
                    "bad oor:separator attribute in " + reader.getUrl());
            }
        }
    }
    valueParser_.separator_ = OString(
        attrSeparator.begin, attrSeparator.length);
    valueParser_.start(property);
}

void XcsParser::handleGroup(xmlreader::XmlReader & reader, bool isTemplate) {
    bool hasName = false;
    OUString name;
    bool extensible = false;
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
                   attrLn.equals("extensible"))
        {
            extensible = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no group name attribute in " + reader.getUrl());
    }
    if (isTemplate) {
        name = Data::fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new GroupNode(
                valueParser_.getLayer(), extensible,
                isTemplate ? name : OUString()),
            name));
}

void XcsParser::handleSet(xmlreader::XmlReader & reader, bool isTemplate) {
    bool hasName = false;
    OUString name;
    OUString component(componentName_);
    bool hasNodeType = false;
    OUString nodeType;
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
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            "no set name attribute in " + reader.getUrl());
    }
    if (isTemplate) {
        name = Data::fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new SetNode(
                valueParser_.getLayer(),
                xmldata::parseTemplateReference(
                    component, hasNodeType, nodeType, nullptr),
                isTemplate ? name : OUString()),
            name));
}

void XcsParser::handleSetItem(xmlreader::XmlReader & reader, SetNode * set) {
    OUString component(componentName_);
    bool hasNodeType = false;
    OUString nodeType;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals("component"))
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals("node-type"))
        {
            hasNodeType = true;
            nodeType = reader.getAttributeValue(false).convertFromUtf8();
        }
    }
    set->getAdditionalTemplateNames().push_back(
        xmldata::parseTemplateReference(component, hasNodeType, nodeType, nullptr));
    elements_.push(Element(rtl::Reference< Node >(), ""));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
