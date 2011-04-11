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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <algorithm>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
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

namespace {

namespace css = com::sun::star;

}

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
    xmlreader::XmlReader & reader, int nsId, xmlreader::Span const & name)
{
    if (valueParser_.startElement(reader, nsId, name)) {
        return true;
    }
    if (state_.empty()) {
        if (nsId == ParseManager::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-data")))
        {
            handleComponentData(reader);
        } else if (nsId == ParseManager::NAMESPACE_OOR &&
                   name.equals(RTL_CONSTASCII_STRINGPARAM("items")))
        {
            state_.push(State(rtl::Reference< Node >(), false));
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad root element <")) +
                 name.convertFromUtf8() +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (state_.top().ignore) {
        state_.push(State(false));
    } else if (!state_.top().node.is()) {
        if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("item")))
        {
            handleItem(reader);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad items node member <")) +
                 name.convertFromUtf8() +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        switch (state_.top().node->kind()) {
        case Node::KIND_PROPERTY:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
            {
                handlePropValue(
                    reader,
                    dynamic_cast< PropertyNode * >(state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad property node member <")) +
                     name.convertFromUtf8() +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
            {
                handleLocpropValue(
                    reader,
                    dynamic_cast< LocalizedPropertyNode * >(
                        state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad localized property node member <")) +
                     name.convertFromUtf8() +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
                 name.convertFromUtf8() +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case Node::KIND_GROUP:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
            {
                handleGroupProp(
                    reader,
                    dynamic_cast< GroupNode * >(state_.top().node.get()));
            } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                       name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleGroupNode(reader, state_.top().node);
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad group node member <")) +
                     name.convertFromUtf8() +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_SET:
            if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleSetNode(
                    reader, dynamic_cast< SetNode * >(state_.top().node.get()));
            } else if (nsId == xmlreader::XmlReader::NAMESPACE_NONE &&
                       name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
            {
                OSL_TRACE(
                    "configmgr bad set node <prop> member in %s",
                    rtl::OUStringToOString(
                        reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
                state_.push(State(true)); // ignored
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("bad set node member <")) +
                     name.convertFromUtf8() +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        }
    }
    return true;
}

void XcuParser::endElement(xmlreader::XmlReader const &) {
    if (valueParser_.endElement()) {
        return;
    }
    OSL_ASSERT(!state_.empty());
    bool pop = state_.top().pop;
    rtl::Reference< Node > insert;
    rtl::OUString name;
    if (state_.top().insert) {
        insert = state_.top().node;
        OSL_ASSERT(insert.is());
        name = state_.top().name;
    }
    state_.pop();
    if (insert.is()) {
        OSL_ASSERT(!state_.empty() && state_.top().node.is());
        state_.top().node->getMembers()[name] = insert;
    }
    if (pop && !path_.empty()) {
        path_.pop_back();
            // </item> will pop less than <item> pushed, but that is harmless,
            // as the next <item> will reset path_
    }
}

void XcuParser::characters(xmlreader::Span const & text) {
    valueParser_.characters(text);
}

XcuParser::Operation XcuParser::parseOperation(xmlreader::Span const & text) {
    OSL_ASSERT(text.is());
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("modify"))) {
        return OPERATION_MODIFY;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("replace"))) {
        return OPERATION_REPLACE;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("fuse"))) {
        return OPERATION_FUSE;
    }
    if (text.equals(RTL_CONSTASCII_STRINGPARAM("remove"))) {
        return OPERATION_REMOVE;
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid op ")) +
         text.convertFromUtf8()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcuParser::handleComponentData(xmlreader::XmlReader & reader) {
    rtl::OStringBuffer buf;
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
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("package")))
        {
            if (hasPackage) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "multiple component-update package attributes"
                            " in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            hasPackage = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.insert(0, s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            if (hasName) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "multiple component-update name attributes in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            hasName = true;
            xmlreader::Span s(reader.getAttributeValue(false));
            buf.append(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasPackage) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data package attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    componentName_ = xmlreader::Span(buf.getStr(), buf.getLength()).
        convertFromUtf8();
    if (trackPath_) {
        OSL_ASSERT(path_.empty());
        path_.push_back(componentName_);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State(true)); // ignored
            return;
        }
    }
    rtl::Reference< Node > node(
        Data::findNode(
            valueParser_.getLayer(), data_.components, componentName_));
    if (!node.is()) {
        OSL_TRACE(
            "configmgr unknown component %s in %s",
            rtl::OUStringToOString(
                componentName_, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(
                reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
        state_.push(State(true)); // ignored
        return;
    }
    switch (op) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        break;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on root node in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        node->getFinalized());
    node->setFinalized(finalizedLayer);
    state_.push(State(node, finalizedLayer < valueParser_.getLayer()));
}

void XcuParser::handleItem(xmlreader::XmlReader & reader) {
    xmlreader::Span attrPath;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("path")))
        {
            attrPath = reader.getAttributeValue(false);
        }
    }
    if (!attrPath.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("missing path attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString path(attrPath.convertFromUtf8());
    int finalizedLayer;
    rtl::Reference< Node > node(
        data_.resolvePathRepresentation(
            path, 0, &path_, &finalizedLayer));
    if (!node.is()) {
        OSL_TRACE(
            "configmgr unknown item %s in %s",
            rtl::OUStringToOString(path, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(
                reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
        state_.push(State(true)); // ignored
        return;
    }
    OSL_ASSERT(!path_.empty());
    componentName_ = path_.front();
    if (trackPath_) {
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State(true)); // ignored
            return;
        }
    } else {
        path_.clear();
    }
    switch (node->kind()) {
    case Node::KIND_PROPERTY:
    case Node::KIND_LOCALIZED_VALUE:
        OSL_TRACE(
            "configmgr item of bad type %s in %s",
            rtl::OUStringToOString(path, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(
                reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
        state_.push(State(true)); // ignored
        return;
    case Node::KIND_LOCALIZED_PROPERTY:
        valueParser_.type_ = dynamic_cast< LocalizedPropertyNode * >(
            node.get())->getStaticType();
        break;
    default:
        break;
    }
    state_.push(State(node, finalizedLayer < valueParser_.getLayer()));
}

void XcuParser::handlePropValue(
    xmlreader::XmlReader & reader, PropertyNode * prop)
 {
    bool nil = false;
    rtl::OString separator;
    rtl::OUString external;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            nil = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            Type type = xmldata::parseType(
                reader, reader.getAttributeValue(true));
            if (valueParser_.type_ != TYPE_ANY && type != valueParser_.type_) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("invalid value type in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            valueParser_.type_ = type;
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            xmlreader::Span s(reader.getAttributeValue(false));
            if (s.length == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad oor:separator attribute in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            separator = rtl::OString(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("external")))
        {
            external = reader.getAttributeValue(true).convertFromUtf8();
            if (external.getLength() == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad oor:external attribute value in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    }
    if (nil) {
        if (!prop->isNillable()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil attribute for non-nillable prop in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (external.getLength() != 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil and oor:external attributes for prop in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        prop->setValue(valueParser_.getLayer(), css::uno::Any());
        state_.push(State(false));
    } else if (external.getLength() == 0) {
        valueParser_.separator_ = separator;
        valueParser_.start(prop);
    } else {
        prop->setExternal(valueParser_.getLayer(), external);
        state_.push(State(false));
    }
}

void XcuParser::handleLocpropValue(
    xmlreader::XmlReader & reader, LocalizedPropertyNode * locprop)
{
    rtl::OUString name;
    bool nil = false;
    rtl::OString separator;
    Operation op = OPERATION_FUSE;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == xmlreader::XmlReader::NAMESPACE_XML &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("lang")))
        {
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            nil = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            Type type = xmldata::parseType(
                reader, reader.getAttributeValue(true));
            if (valueParser_.type_ != TYPE_ANY && type != valueParser_.type_) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("invalid value type in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            valueParser_.type_ = type;
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            xmlreader::Span s(reader.getAttributeValue(false));
            if (s.length == 0) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad oor:separator attribute in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            separator = rtl::OString(s.begin, s.length);
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            op = parseOperation(reader.getAttributeValue(true));
        }
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 &&
            partial_->contains(path_) != Partial::CONTAINS_NODE)
        {
            state_.push(State(true)); // ignored
            return;
        }
    }
    NodeMap::iterator i(locprop->getMembers().find(name));
    if (i != locprop->getMembers().end() &&
        i->second->getLayer() > valueParser_.getLayer())
    {
        state_.push(State(true)); // ignored
        return;
    }
    if (nil && !locprop->isNillable()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xsi:nil attribute for non-nillable prop in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (op) {
    case OPERATION_FUSE:
        {
            bool pop = false;
            if (nil) {
                if (i == locprop->getMembers().end()) {
                    locprop->getMembers()[name] = new LocalizedValueNode(
                        valueParser_.getLayer(), css::uno::Any());
                } else {
                    dynamic_cast< LocalizedValueNode * >(
                        i->second.get())->setValue(
                            valueParser_.getLayer(), css::uno::Any());
                }
                state_.push(State(true));
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
        //TODO: only allow if parent.op == OPERATION_FUSE
        //TODO: disallow removing when e.g. lang=""?
        if (i != locprop->getMembers().end()) {
            locprop->getMembers().erase(i);
        }
        state_.push(State(true));
        recordModification(false);
        break;
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "bad op attribute for value element in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupProp(
    xmlreader::XmlReader & reader, GroupNode * group)
{
    bool hasName = false;
    rtl::OUString name;
    Type type = TYPE_ERROR;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            type = xmldata::parseType(reader, reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        //TODO: This ignores locprop values for which specific include paths
        // exist (i.e., for which contains(locprop path) = CONTAINS_SUBNODES):
        if (partial_ != 0 &&
            partial_->contains(path_) != Partial::CONTAINS_NODE)
        {
            state_.push(State(true)); // ignored
            return;
        }
    }
    NodeMap::iterator i(group->getMembers().find(name));
    if (i == group->getMembers().end()) {
        handleUnknownGroupProp(reader, group, name, type, op, finalized);
    } else {
        switch (i->second->kind()) {
        case Node::KIND_PROPERTY:
            handlePlainGroupProp(reader, group, i, name, type, op, finalized);
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            handleLocalizedGroupProp(
                reader,
                dynamic_cast< LocalizedPropertyNode * >(i->second.get()), name,
                type, op, finalized);
            break;
        default:
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("inappropriate prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    }
}

void XcuParser::handleUnknownGroupProp(
    xmlreader::XmlReader const & reader, GroupNode * group,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    switch (operation) {
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        if (group->isExtensible()) {
            if (type == TYPE_ERROR) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "missing type attribute for prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
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
            state_.push(State(prop, name, state_.top().locked));
            recordModification(false);
            break;
        }
        // fall through
    default:
        OSL_TRACE(
            "configmgr unknown property %s in %s",
            rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(
                reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
        state_.push(State(true)); // ignored
        break;
    }
}

void XcuParser::handlePlainGroupProp(
    xmlreader::XmlReader const & reader, GroupNode * group,
    NodeMap::iterator const & propertyIndex, rtl::OUString const & name,
    Type type, Operation operation, bool finalized)
{
    PropertyNode * property = dynamic_cast< PropertyNode * >(
        propertyIndex->second.get());
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State(true)); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getStaticType() != TYPE_ANY &&
        type != property->getStaticType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getStaticType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                (state_.top().locked ||
                 finalizedLayer < valueParser_.getLayer())));
        recordModification(false);
        break;
    case OPERATION_REMOVE:
        if (!property->isExtension()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "invalid remove of non-extension prop ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        group->getMembers().erase(propertyIndex);
        state_.push(State(true)); // ignore children
        recordModification(false);
        break;
    }
}

void XcuParser::handleLocalizedGroupProp(
    xmlreader::XmlReader const & reader, LocalizedPropertyNode * property,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State(true)); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getStaticType() != TYPE_ANY &&
        type != property->getStaticType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getStaticType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                (state_.top().locked ||
                 finalizedLayer < valueParser_.getLayer())));
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > replacement(
                new LocalizedPropertyNode(
                    valueParser_.getLayer(), property->getStaticType(),
                    property->isNillable()));
            replacement->setFinalized(property->getFinalized());
            state_.push(
                State(
                    replacement, name,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer())));
            recordModification(false);
        }
        break;
    case OPERATION_REMOVE:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid remove of non-extension prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleGroupNode(
    xmlreader::XmlReader & reader, rtl::Reference< Node > const & group)
{
    bool hasName = false;
    rtl::OUString name;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State(true)); // ignored
            return;
        }
    }
    rtl::Reference< Node > child(
        Data::findNode(valueParser_.getLayer(), group->getMembers(), name));
    if (!child.is()) {
        OSL_TRACE(
            "configmgr unknown node %s in %s",
            rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr(),
            rtl::OUStringToOString(
                reader.getUrl(), RTL_TEXTENCODING_UTF8).getStr());
        state_.push(State(true)); // ignored
        return;
    }
    if (op != OPERATION_MODIFY && op != OPERATION_FUSE) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on group node in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        child->getFinalized());
    child->setFinalized(finalizedLayer);
    state_.push(
        State(
            child,
            state_.top().locked || finalizedLayer < valueParser_.getLayer()));
}

void XcuParser::handleSetNode(xmlreader::XmlReader & reader, SetNode * set) {
    bool hasName = false;
    rtl::OUString name;
    rtl::OUString component(componentName_);
    bool hasNodeType = false;
    rtl::OUString nodeType;
    Operation op = OPERATION_MODIFY;
    bool finalized = false;
    bool mandatory = false;
    for (;;) {
        int attrNsId;
        xmlreader::Span attrLn;
        if (!reader.nextAttribute(&attrNsId, &attrLn)) {
            break;
        }
        if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            hasName = true;
            name = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            component = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            hasNodeType = true;
            nodeType = reader.getAttributeValue(false).convertFromUtf8();
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            op = parseOperation(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            finalized = xmldata::parseBoolean(reader.getAttributeValue(true));
        } else if (attrNsId == ParseManager::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("mandatory")))
        {
            mandatory = xmldata::parseBoolean(reader.getAttributeValue(true));
        }
    }
    if (!hasName) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (trackPath_) {
        path_.push_back(name);
        if (partial_ != 0 && partial_->contains(path_) == Partial::CONTAINS_NOT)
        {
            state_.push(State(true)); // ignored
            return;
        }
    }
    rtl::OUString templateName(
        xmldata::parseTemplateReference(
            component, hasNodeType, nodeType, &set->getDefaultTemplateName()));
    if (!set->isValidTemplate(templateName)) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("set member node ")) +
             name +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(" references invalid template ")) +
             templateName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > tmpl(
        data_.getTemplate(valueParser_.getLayer(), templateName));
    if (!tmpl.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("set member node ")) +
             name +
             rtl::OUString(
                 RTL_CONSTASCII_USTRINGPARAM(
                     " references undefined template ")) +
             templateName + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = finalized ? valueParser_.getLayer() : Data::NO_LAYER;
    int mandatoryLayer = mandatory ? valueParser_.getLayer() : Data::NO_LAYER;
    NodeMap::iterator i(set->getMembers().find(name));
    if (i != set->getMembers().end()) {
        finalizedLayer = std::min(finalizedLayer, i->second->getFinalized());
        i->second->setFinalized(finalizedLayer);
        mandatoryLayer = std::min(mandatoryLayer, i->second->getMandatory());
        i->second->setMandatory(mandatoryLayer);
        if (i->second->getLayer() > valueParser_.getLayer()) {
            state_.push(State(true)); // ignored
            return;
        }
    }
    switch (op) {
    case OPERATION_MODIFY:
        if (i == set->getMembers().end()) {
            OSL_TRACE("ignoring modify of unknown set member node");
            state_.push(State(true)); // ignored
        } else {
            state_.push(
                State(
                    i->second,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer())));
        }
        break;
    case OPERATION_REPLACE:
        if (state_.top().locked || finalizedLayer < valueParser_.getLayer()) {
            state_.push(State(true)); // ignored
        } else {
            rtl::Reference< Node > member(tmpl->clone(true));
            member->setLayer(valueParser_.getLayer());
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State(member, name, false));
            recordModification(i == set->getMembers().end());
        }
        break;
    case OPERATION_FUSE:
        if (i == set->getMembers().end()) {
            if (state_.top().locked || finalizedLayer < valueParser_.getLayer())
            {
                state_.push(State(true)); // ignored
            } else {
                rtl::Reference< Node > member(tmpl->clone(true));
                member->setLayer(valueParser_.getLayer());
                member->setFinalized(finalizedLayer);
                member->setMandatory(mandatoryLayer);
                state_.push(State(member, name, false));
                recordModification(true);
            }
        } else {
            state_.push(
                State(
                    i->second,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer())));
        }
        break;
    case OPERATION_REMOVE:
        {
            // Ignore removal of unknown members, members finalized in a lower
            // layer, and members made mandatory in this or a lower layer;
            // forget about user-layer removals that no longer remove anything
            // (so that paired additions/removals in the user layer do not grow
            // registrymodifications.xcu unbounded):
            bool known = i != set->getMembers().end();
            if (known && !state_.top().locked &&
                finalizedLayer >= valueParser_.getLayer() &&
                mandatoryLayer > valueParser_.getLayer())
            {
                set->getMembers().erase(i);
            }
            state_.push(State(true));
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
