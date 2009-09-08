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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "data.hxx"
#include "localizedpropertynode.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "span.hxx"
#include "xcsparser.hxx"
#include "xmldata.hxx"
#include "xmlreader.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

XcsParser::XcsParser(int layer, Data * data):
    valueParser_(layer), data_(data), state_(STATE_START)
{}

XcsParser::~XcsParser() {}

XmlReader::Text XcsParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcsParser::startElement(
    XmlReader & reader, XmlReader::Namespace ns, Span const & name)
{
    if (valueParser_.startElement(reader, ns, name)) {
        return true;
    }
    if (state_ == STATE_START) {
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-schema"))) {
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
            (ns == XmlReader::NAMESPACE_NONE &&
             (name.equals(RTL_CONSTASCII_STRINGPARAM("info")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("import")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("uses")) ||
              name.equals(RTL_CONSTASCII_STRINGPARAM("constraints")))))
        {
            OSL_ASSERT(ignoring_ < LONG_MAX);
            ++ignoring_;
            return true;
        }
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("templates")))
            {
                state_ = STATE_TEMPLATES;
                return true;
            }
            // fall through
        case STATE_TEMPLATES_DONE:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("component")))
            {
                state_ = STATE_COMPONENT;
                OSL_ASSERT(elements_.empty());
                elements_.push(
                    Element(
                        new GroupNode(
                            valueParser_.getLayer(), false, rtl::OUString()),
                        componentName_));
                return true;
            }
            break;
        case STATE_TEMPLATES:
            if (elements_.empty()) {
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("group")))
                {
                    handleGroup(reader, true);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("set")))
                {
                    handleSet(reader, true);
                    return true;
                }
                break;
            }
            // fall through
        case STATE_COMPONENT:
            OSL_ASSERT(!elements_.empty());
            switch (elements_.top().node->kind()) {
            case Node::KIND_PROPERTY:
            case Node::KIND_LOCALIZED_PROPERTY:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("value")))
                {
                    handlePropValue(reader, elements_.top().node);
                    return true;
                }
                break;
            case Node::KIND_GROUP:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
                {
                    handleProp(reader);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("node-ref")))
                {
                    handleNodeRef(reader);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("group")))
                {
                    handleGroup(reader, false);
                    return true;
                }
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("set")))
                {
                    handleSet(reader, false);
                    return true;
                }
                break;
            case Node::KIND_SET:
                if (ns == XmlReader::NAMESPACE_NONE &&
                    name.equals(RTL_CONSTASCII_STRINGPARAM("item")))
                {
                    handleSetItem(
                        reader,
                        dynamic_cast< SetNode * >(elements_.top().node.get()));
                    return true;
                }
                break;
            default: // Node::KIND_LOCALIZED_VALUE
                OSL_ASSERT(false); // this cannot happen
                break;
            }
            break;
        case STATE_COMPONENT_DONE:
            break;
        default: // STATE_START
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
    throw css::uno::RuntimeException(
        (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
         xmldata::convertFromUtf8(name) +
         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) + reader.getUrl()),
        css::uno::Reference< css::uno::XInterface >());
}

void XcsParser::endElement(XmlReader const & reader) {
    if (valueParser_.endElement(reader)) {
        return;
    }
    if (ignoring_ > 0) {
        --ignoring_;
    } else if (!elements_.empty()) {
        Element top(elements_.top());
        elements_.pop();
        if (top.node.is()) {
            NodeMap * map;
            if (elements_.empty()) {
                switch (state_) {
                case STATE_TEMPLATES:
                    map = &data_->templates;
                    break;
                case STATE_COMPONENT:
                    map = &data_->components;
                    state_ = STATE_COMPONENT_DONE;
                    break;
                default:
                    OSL_ASSERT(false);
                    throw css::uno::RuntimeException(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                        css::uno::Reference< css::uno::XInterface >());
                }
            } else {
                map = &elements_.top().node->getMembers();
            }
            if (!map->insert(NodeMap::value_type(top.name, top.node)).second) {
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("duplicate ")) +
                     top.name +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
        }
    } else {
        switch (state_) {
        case STATE_COMPONENT_SCHEMA:
        case STATE_TEMPLATES_DONE:
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("no component element in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case STATE_TEMPLATES:
            state_ = STATE_TEMPLATES_DONE;
            break;
        case STATE_COMPONENT_DONE:
            break;
        default:
            OSL_ASSERT(false); // this cannot happen
        }
    }
}

void XcsParser::characters(Span const & text) {
    valueParser_.characters(text);
}

void XcsParser::handleComponentSchema(XmlReader & reader) {
    //TODO: oor:version, xml:lang attributes
    Span attrPackage;
    Span attrName;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("package")))
        {
            attrPackage = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader.getAttributeValue(false);
        }
    }
    if (!attrPackage.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema package attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-schema name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(attrPackage.begin, attrPackage.length);
    buf.append('.');
    buf.append(attrName.begin, attrName.length);
    componentName_ = xmldata::convertFromUtf8(
        Span(buf.getStr(), buf.getLength()));
}

void XcsParser::handleNodeRef(XmlReader & reader) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader.getAttributeValue(false);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node-ref name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > tmpl(
        data_->getTemplate(
            valueParser_.getLayer(),
            xmldata::parseTemplateReference(
                attrComponent, attrNodeType, componentName_, 0)));
    if (!tmpl.is()) {
        //TODO: this can erroneously happen as long as import/uses attributes
        // are not correctly processed
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("unknown node-ref ")) +
             xmldata::convertFromUtf8(attrName) +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::Reference< Node > node(tmpl->clone());
    node->setLayer(valueParser_.getLayer());
    elements_.push(Element(node, xmldata::convertFromUtf8(attrName)));
}

void XcsParser::handleProp(XmlReader & reader) {
    Span attrName;
    Span attrType;
    Span attrLocalized;
    Span attrNillable;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("type")))
        {
            attrType = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("localized")))
        {
            attrLocalized = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nillable")))
        {
            attrNillable = reader.getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = xmldata::parseType(reader, attrType);
    if (valueParser_.type_ == TYPE_ERROR) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop type attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    bool localized = xmldata::parseBoolean(attrLocalized, false);
    bool nillable = xmldata::parseBoolean(attrNillable, true);
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
            xmldata::convertFromUtf8(attrName)));
}

void XcsParser::handlePropValue(
    XmlReader & reader, rtl::Reference< Node > const & property)
{
    Span attrSeparator;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader.getAttributeValue(false);
        }
    }
    valueParser_.separator_ = attrSeparator;
    valueParser_.start(property);
}

void XcsParser::handleGroup(XmlReader & reader, bool isTemplate) {
    Span attrName;
    Span attrExtensible;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("extensible")))
        {
            attrExtensible = reader.getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no group name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(xmldata::convertFromUtf8(attrName));
    if (isTemplate) {
        name = Data::fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new GroupNode(
                valueParser_.getLayer(),
                xmldata::parseBoolean(attrExtensible, false),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSet(XmlReader & reader, bool isTemplate) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("name")))
        {
            attrName = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader.getAttributeValue(false);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no set name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(xmldata::convertFromUtf8(attrName));
    if (isTemplate) {
        name = Data::fullTemplateName(componentName_, name);
    }
    elements_.push(
        Element(
            new SetNode(
                valueParser_.getLayer(),
                xmldata::parseTemplateReference(
                    attrComponent, attrNodeType, componentName_, 0),
                isTemplate ? name : rtl::OUString()),
            name));
}

void XcsParser::handleSetItem(XmlReader & reader, SetNode * set) {
    Span attrComponent;
    Span attrNodeType;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("component")))
        {
            attrComponent = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("node-type")))
        {
            attrNodeType = reader.getAttributeValue(false);
        }
    }
    set->getAdditionalTemplateNames().push_back(
        xmldata::parseTemplateReference(
            attrComponent, attrNodeType, componentName_, 0));
    elements_.push(Element(rtl::Reference< Node >(), rtl::OUString()));
}

}
