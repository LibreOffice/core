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
#include <set>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
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
    if (update->getLayer() < original->getLayer() ||
        update->getLayer() > original->getFinalized())
        return;

    switch (original->kind()) {
    case Node::KIND_PROPERTY:
    case Node::KIND_LOCALIZED_PROPERTY:
    case Node::KIND_LOCALIZED_VALUE:
        break; //TODO: merge certain parts?
    case Node::KIND_GROUP:
        for (auto const& updateMember : update->getMembers())
        {
            NodeMap & members = original->getMembers();
            NodeMap::iterator i1(members.find(updateMember.first));
            if (i1 == members.end()) {
                if (updateMember.second->kind() == Node::KIND_PROPERTY &&
                    static_cast< GroupNode * >(
                        original.get())->isExtensible())
                {
                    members.insert(updateMember);
                }
            } else if (updateMember.second->kind() == i1->second->kind()) {
                merge(i1->second, updateMember.second);
            }
        }
        break;
    case Node::KIND_SET:
        for (auto const& updateMember : update->getMembers())
        {
            NodeMap & members = original->getMembers();
            NodeMap::iterator i1(members.find(updateMember.first));
            if (i1 == members.end()) {
                if (static_cast< SetNode * >(original.get())->
                    isValidTemplate(updateMember.second->getTemplateName()))
                {
                    members.insert(updateMember);
                }
            } else if (updateMember.second->kind() == i1->second->kind() &&
                       (updateMember.second->getTemplateName() ==
                        i1->second->getTemplateName()))
            {
                merge(i1->second, updateMember.second);
            }
        }
        break;
    case Node::KIND_ROOT:
        assert(false); // this cannot happen
        break;
    }
}

}

XcsParser::XcsParser(int layer, Data & data):
    valueParser_(layer), data_(data), state_(STATE_START), ignoring_(), bIsParsingInfo_(false)
{}

XcsParser::~XcsParser() {}

xmlreader::XmlReader::Text XcsParser::getTextMode() {
    if (bIsParsingInfo_)
        return xmlreader::XmlReader::Text::Raw;
    return valueParser_.getTextMode();
}

bool XcsParser::startElement(
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name,
    std::set< OUString > const * /*existingDependencies*/)
{
    //TODO: ignoring component-schema import, component-schema uses, and
    // prop constraints; accepting all four at illegal places (and with
    // illegal content):
    if (ignoring_ > 0
        || (nsId == xmlreader::XmlReader::NAMESPACE_NONE
            && (name == "import" || name == "uses" || name == "constraints" || name == "desc"
                // the following are unused by LO but valid
                || name == "deprecated" || name == "author" || name == "label")))
    {
        assert(ignoring_ < LONG_MAX);
        ++ignoring_;
        return true;
    }

    if (bIsParsingInfo_)
        return true;
    if (valueParser_.startElement(reader, nsId, name)) {
        return true;
    }
    if (state_ == STATE_START) {
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name == "component-schema")
        {
            handleComponentSchema(reader);
            state_ = STATE_COMPONENT_SCHEMA;
            ignoring_ = 0;
            return true;
        }
    } else {
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name == "templates")
            {
                state_ = STATE_TEMPLATES;
                return true;
            }
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name == "info")
            {
                bIsParsingInfo_ = true;
                return true;
            }
            [[fallthrough]];
        case STATE_TEMPLATES_DONE:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name == "component")
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
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "info")
            {
                bIsParsingInfo_ = true;
                return true;
            }
            if (elements_.empty()) {
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "group")
                {
                    handleGroup(reader, true);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "set")
                {
                    handleSet(reader, true);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "info")
                {
                    bIsParsingInfo_ = true;
                    return true;
                }
                break;
            }
            [[fallthrough]];
        case STATE_COMPONENT:
            assert(!elements_.empty());
            switch (elements_.top().node->kind()) {
            case Node::KIND_PROPERTY:
            case Node::KIND_LOCALIZED_PROPERTY:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "value")
                {
                    handlePropValue(reader, elements_.top().node);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "info")
                {
                    bIsParsingInfo_ = true;
                    return true;
                }
                break;
            case Node::KIND_GROUP:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "prop")
                {
                    handleProp(reader);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "node-ref")
                {
                    handleNodeRef(reader);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "group")
                {
                    handleGroup(reader, false);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "set")
                {
                    handleSet(reader, false);
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "info")
                {
                    bIsParsingInfo_ = true;
                    return true;
                }
                break;
            case Node::KIND_SET:
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "item")
                {
                    handleSetItem(
                        reader,
                        static_cast< SetNode * >(elements_.top().node.get()));
                    return true;
                }
                if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                    name == "info")
                {
                    bIsParsingInfo_ = true;
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
    if (ignoring_ > 0) {
        --ignoring_;
        return;
    }
    if (bIsParsingInfo_)
    {
        bIsParsingInfo_ = false;
        return;
    }
    if (valueParser_.endElement()) {
        return;
    }
    if (!elements_.empty()) {
        Element top(std::move(elements_.top()));
        elements_.pop();
        if (top.node.is()) {
            if (top.node->kind() == Node::KIND_PROPERTY
                || top.node->kind() == Node::KIND_LOCALIZED_PROPERTY)
            {
                // Remove whitespace from description_ resulting from line breaks/indentation in xml files
                OUString desc(description_.makeStringAndClear());
                desc = desc.trim();
                while (desc.indexOf("  ") != -1)
                    desc = desc.replaceAll("  ", " ");
                top.node->setDescription(desc);
            }
            if (elements_.empty()) {
                switch (state_) {
                case STATE_TEMPLATES:
                    {
                        auto itPair = data_.templates.insert({top.name, top.node});
                        if (!itPair.second) {
                            merge(itPair.first->second, top.node);
                        }
                    }
                    break;
                case STATE_COMPONENT:
                    {
                        NodeMap & components = data_.getComponents();
                        auto itPair = components.insert({top.name, top.node});
                        if (!itPair.second) {
                            merge(itPair.first->second, top.node);
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
    if (bIsParsingInfo_)
    {
        description_.append(text.convertFromUtf8());
        return;
    }
    valueParser_.characters(text);
}

void XcsParser::handleComponentSchema(xmlreader::XmlReader & reader) {
    //TODO: oor:version, xml:lang attributes
    OStringBuffer buf(256);
    buf.append('.');
    bool hasPackage = false;
    bool hasName = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn == "package")
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
                   attrLn == "name")
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
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn == "name") {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "component")
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "node-type")
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
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn == "name") {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "type")
        {
            valueParser_.type_ = xmldata::parseType(
                reader, reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "localized")
        {
            localized = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "nillable")
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
            attrLn == "separator")
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
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn == "name") {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "extensible")
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
        if (attrNsId == ParseManager::NAMESPACE_OOR && attrLn == "name") {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "component")
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "node-type")
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
            attrLn == "component")
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn == "node-type")
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
