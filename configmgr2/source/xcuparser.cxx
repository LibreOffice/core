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

#include <algorithm>

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "groupnode.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "path.hxx"
#include "propertynode.hxx"
#include "setnode.hxx"
#include "span.hxx"
#include "xcuparser.hxx"
#include "xmldata.hxx"
#include "xmlreader.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

XcuParser::XcuParser(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    int layer, Data * data):
    context_(context), valueParser_(layer), data_(data)
{
    OSL_ASSERT(context.is());
}

XcuParser::~XcuParser() {}

XmlReader::Text XcuParser::getTextMode() {
    return valueParser_.getTextMode();
}

bool XcuParser::startElement(
    XmlReader & reader, XmlReader::Namespace ns, Span const & name)
{
    if (valueParser_.startElement(reader, ns, name)) {
        return true;
    }
    if (state_.empty()) {
        if (ns == XmlReader::NAMESPACE_OOR &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("component-data")))
        {
            handleComponentData(reader);
        } else if (ns == XmlReader::NAMESPACE_OOR &&
                   name.equals(RTL_CONSTASCII_STRINGPARAM("items")))
        {
            state_.push(State(rtl::Reference< Node >(), false, false));
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad root element <")) +
                 xmldata::convertFromUtf8(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else if (state_.top().ignore) {
        state_.push(state_.top());
    } else if (!state_.top().node.is()) {
        if (ns == XmlReader::NAMESPACE_NONE &&
            name.equals(RTL_CONSTASCII_STRINGPARAM("item")))
        {
            handleItem(reader);
        } else {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad items node member <")) +
                 xmldata::convertFromUtf8(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
    } else {
        switch (state_.top().node->kind()) {
        case Node::KIND_PROPERTY:
            if (ns == XmlReader::NAMESPACE_NONE &&
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
                     xmldata::convertFromUtf8(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_PROPERTY:
            if (ns == XmlReader::NAMESPACE_NONE &&
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
                     xmldata::convertFromUtf8(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad member <")) +
                 xmldata::convertFromUtf8(name) +
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        case Node::KIND_GROUP:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("prop")))
            {
                handleGroupProp(
                    reader,
                    dynamic_cast< GroupNode * >(state_.top().node.get()));
            } else if (ns == XmlReader::NAMESPACE_NONE &&
                       name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleGroupNode(reader, state_.top().node);
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad group node member <")) +
                     xmldata::convertFromUtf8(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case Node::KIND_SET:
            if (ns == XmlReader::NAMESPACE_NONE &&
                name.equals(RTL_CONSTASCII_STRINGPARAM("node")))
            {
                handleSetNode(
                    reader, dynamic_cast< SetNode * >(state_.top().node.get()));
            } else {
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM("bad set node member <")) +
                     xmldata::convertFromUtf8(name) +
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("> in ")) +
                     reader.getUrl()),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        }
    }
    return true;
}

void XcuParser::endElement(XmlReader const & reader) {
    if (valueParser_.endElement(reader)) {
        return;
    }
    OSL_ASSERT(!state_.empty());
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
}

void XcuParser::characters(Span const & text) {
    valueParser_.characters(text);
}

XcuParser::Operation XcuParser::parseOperation(Span const & text) {
    if (!text.is() || text.equals(RTL_CONSTASCII_STRINGPARAM("modify"))) {
        return OPERATION_MODIFY;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("replace"))) {
        return OPERATION_REPLACE;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("fuse"))) {
        return OPERATION_FUSE;
    } else if (text.equals(RTL_CONSTASCII_STRINGPARAM("remove"))) {
        return OPERATION_REMOVE;
    } else {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("invalid op ")) +
             xmldata::convertFromUtf8(text)),
            css::uno::Reference< css::uno::XInterface >());
    }
}

void XcuParser::handleComponentData(XmlReader & reader) {
    Span attrPackage;
    Span attrName;
    Span attrOp;
    Span attrFinalized;
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
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader.getAttributeValue(true);
        }
    }
    if (!attrPackage.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data package attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "no component-data name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OStringBuffer buf;
    buf.append(attrPackage.begin, attrPackage.length);
    buf.append('.');
    buf.append(attrName.begin, attrName.length);
    componentName_ = xmldata::convertFromUtf8(
        Span(buf.getStr(), buf.getLength()));
    rtl::Reference< Node > node(
        Data::findNode(
            valueParser_.getLayer(), data_->components, componentName_));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown component ")) +
             componentName_ +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (parseOperation(attrOp)) {
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
        (xmldata::parseBoolean(attrFinalized, false)
         ? valueParser_.getLayer() : Data::NO_LAYER),
        node->getFinalized());
    node->setFinalized(finalizedLayer);
    state_.push(State(node, finalizedLayer < valueParser_.getLayer(), false));
}

void XcuParser::handleItem(XmlReader & reader) {
    Span attrPath;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_OOR &&
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
    int finalizedLayer;
    rtl::Reference< Node > node(
        data_->resolvePathRepresentation(
            xmldata::convertFromUtf8(attrPath), &pathPrefix_, &finalizedLayer));
    if (!node.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("nonexisting path attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    OSL_ASSERT(!pathPrefix_.empty());
    componentName_ = pathPrefix_.front();
    state_.push(
        State(
            node, finalizedLayer < valueParser_.getLayer(),
            valueParser_.getLayer() == Data::NO_LAYER));
}

void XcuParser::handlePropValue(XmlReader & reader, PropertyNode * prop) {
    OSL_ASSERT(!state_.top().record);
    Span attrNil;
    Span attrSeparator;
    Span attrExternal;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            attrNil = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("external")))
        {
            attrExternal = reader.getAttributeValue(true);
        }
    }
    if (xmldata::parseBoolean(attrNil, false)) {
        if (!prop->isNillable()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil attribute for non-nillable prop in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        if (attrExternal.is()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "xsi:nil and oor:external attributes for prop in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        prop->setValue(valueParser_.getLayer(), css::uno::Any());
        state_.push(State());
    } else if (attrExternal.is()) {
        rtl::OUString external(xmldata::convertFromUtf8(attrExternal));
        sal_Int32 i = external.indexOf(' ');
        if (i <= 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "bad oor:external attribute value in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        css::uno::Reference< css::container::XNameAccess > service;
        try {
            service = css::uno::Reference< css::container::XNameAccess >(
                (css::uno::Reference< css::lang::XMultiComponentFactory >(
                    context_->getServiceManager(), css::uno::UNO_SET_THROW)->
                 createInstanceWithContext(external.copy(0, i), context_)),
                css::uno::UNO_QUERY_THROW);
        } catch (css::uno::RuntimeException &) {
            throw;
        } catch (css::uno::Exception & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "cannot instantiate oor:external service: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >());
        }
        css::uno::Any value;
        try {
            value = service->getByName(external.copy(i + 1));
        } catch (css::container::NoSuchElementException & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("unknwon oor:external ID: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >());
        } catch (css::lang::WrappedTargetException & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "cannot obtain oor:external value: ")) +
                 e.Message),
                css::uno::Reference< css::uno::XInterface >());
        }
        css::uno::Type t;
        if (!(value >>= t) || t != cppu::UnoType< cppu::UnoVoidType >::get()) {
            //TODO: check value type
            prop->setValue(valueParser_.getLayer(), value);
        }
        state_.push(State());
    } else {
        valueParser_.separator_ = attrSeparator;
        valueParser_.start(prop);
    }
}

void XcuParser::handleLocpropValue(
    XmlReader & reader, LocalizedPropertyNode * locprop)
{
    OSL_ASSERT(!state_.top().record);
    Span attrLang;
    Span attrNil;
    Span attrSeparator;
    Span attrOp;
    for (;;) {
        XmlReader::Namespace attrNs;
        Span attrLn;
        if (!reader.nextAttribute(&attrNs, &attrLn)) {
            break;
        }
        if (attrNs == XmlReader::NAMESPACE_XML &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("lang")))
        {
            attrLang = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_XSI &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("nil")))
        {
            attrNil = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("separator")))
        {
            attrSeparator = reader.getAttributeValue(false);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
            attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader.getAttributeValue(true);
        }
    }
    rtl::OUString name;
    if (attrLang.is()) {
        name = xmldata::convertFromUtf8(attrLang);
    }
    NodeMap::iterator i(locprop->getMembers().find(name));
    if (i != locprop->getMembers().end() &&
        i->second->getLayer() > valueParser_.getLayer())
    {
        state_.push(State()); // ignored
        return;
    }
    bool nil = xmldata::parseBoolean(attrNil, false);
    if (nil && !locprop->isNillable()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "xsi:nil attribute for non-nillable prop in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (parseOperation(attrOp)) {
    case OPERATION_MODIFY:
        if (nil) {
            if (i == locprop->getMembers().end()) {
                locprop->getMembers()[name] = new LocalizedValueNode(
                    valueParser_.getLayer(), css::uno::Any());
            } else {
                dynamic_cast< LocalizedValueNode * >(i->second.get())->setValue(
                    valueParser_.getLayer(), css::uno::Any());
            }
            state_.push(State());
        } else {
            valueParser_.separator_ = attrSeparator;
            valueParser_.start(locprop, name);
        }
        break;
    case OPERATION_REMOVE:
        //TODO: only allow if parent.op == OPERATION_FUSE
        //TODO: disallow removing when e.g. lang=""?
        if (i != locprop->getMembers().end()) {
            locprop->getMembers().erase(i);
        }
        state_.push(State());
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

void XcuParser::handleGroupProp(XmlReader & reader, GroupNode * group) {
    Span attrName;
    Span attrType;
    Span attrOp;
    Span attrFinalized;
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
            attrType = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader.getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no prop name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(xmldata::convertFromUtf8(attrName));
    if (state_.top().record) {
        Path path(pathPrefix_);
        path.push_back(name);
        data_->modifications.add(path);
    }
    Type type = xmldata::parseType(reader, attrType);
    if (type == TYPE_ANY) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid prop type attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    Operation op = parseOperation(attrOp);
    bool finalized = xmldata::parseBoolean(attrFinalized, false);
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
    XmlReader const & reader, GroupNode * group, rtl::OUString const & name,
    Type type, Operation operation, bool finalized)
{
    if (!group->isExtensible()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    switch (operation) {
    case OPERATION_MODIFY:
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid modify of extension prop ")) +
             name +
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        {
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
            state_.push(State(prop, name, state_.top().locked, false));
        }
        break;
    case OPERATION_REMOVE:
        // ignore unknown (presumably extension) properties
        state_.push(State());
        break;
    }
}

void XcuParser::handlePlainGroupProp(
    XmlReader const & reader, GroupNode * group,
    NodeMap::iterator const & propertyIndex, rtl::OUString const & name,
    Type type, Operation operation, bool finalized)
{
    PropertyNode * property = dynamic_cast< PropertyNode * >(
        propertyIndex->second.get());
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getType() != TYPE_ANY &&
        type != property->getType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_REPLACE:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
                false));
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
        state_.push(State()); // ignore children
        break;
    }
}

void XcuParser::handleLocalizedGroupProp(
    XmlReader const & reader, LocalizedPropertyNode * property,
    rtl::OUString const & name, Type type, Operation operation, bool finalized)
{
    if (property->getLayer() > valueParser_.getLayer()) {
        state_.push(State()); // ignored
        return;
    }
    int finalizedLayer = std::min(
        finalized ? valueParser_.getLayer() : Data::NO_LAYER,
        property->getFinalized());
    property->setFinalized(finalizedLayer);
    if (type != TYPE_ERROR && property->getType() != TYPE_ANY &&
        type != property->getType())
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("invalid type for prop ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    valueParser_.type_ = type == TYPE_ERROR ? property->getType() : type;
    switch (operation) {
    case OPERATION_MODIFY:
    case OPERATION_FUSE:
        state_.push(
            State(
                property,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
                false));
        break;
    case OPERATION_REPLACE:
        {
            rtl::Reference< Node > replacement(
                new LocalizedPropertyNode(
                    valueParser_.getLayer(), property->getType(),
                    property->isNillable()));
            replacement->setFinalized(property->getFinalized());
            state_.push(
                State(
                    replacement, name,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer()),
                    false));
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
    XmlReader & reader, rtl::Reference< Node > const & group)
{
    OSL_ASSERT(!state_.top().record);
    Span attrName;
    Span attrOp;
    Span attrFinalized;
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
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader.getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(xmldata::convertFromUtf8(attrName));
    rtl::Reference< Node > subgroup(
        Data::findNode(valueParser_.getLayer(), group->getMembers(), name));
    if (!subgroup.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown node ")) +
             name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    Operation op(parseOperation(attrOp));
    if ((op != OPERATION_MODIFY && op != OPERATION_FUSE) || state_.top().record)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "invalid operation on group node in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    int finalizedLayer = std::min(
        (xmldata::parseBoolean(attrFinalized, false)
         ? valueParser_.getLayer() : Data::NO_LAYER),
        subgroup->getFinalized());
    subgroup->setFinalized(finalizedLayer);
    state_.push(
        State(
            subgroup,
            state_.top().locked || finalizedLayer < valueParser_.getLayer(),
            false));
}

void XcuParser::handleSetNode(XmlReader & reader, SetNode * set) {
    Span attrName;
    Span attrComponent;
    Span attrNodeType;
    Span attrOp;
    Span attrFinalized;
    Span attrMandatory;
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
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("op")))
        {
            attrOp = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("finalized")))
        {
            attrFinalized = reader.getAttributeValue(true);
        } else if (attrNs == XmlReader::NAMESPACE_OOR &&
                   attrLn.equals(RTL_CONSTASCII_STRINGPARAM("mandatory")))
        {
            attrMandatory = reader.getAttributeValue(true);
        }
    }
    if (!attrName.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no node name attribute in ")) +
             reader.getUrl()),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString name(xmldata::convertFromUtf8(attrName));
    rtl::OUString templateName(
        xmldata::parseTemplateReference(
            attrComponent, attrNodeType, componentName_,
            &set->getDefaultTemplateName()));
    if (state_.top().record) {
        Path path(pathPrefix_);
        path.push_back(name);
        data_->modifications.add(path);
    }
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
        data_->getTemplate(valueParser_.getLayer(), templateName));
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
    Operation op(parseOperation(attrOp));
    int finalizedLayer = xmldata::parseBoolean(attrFinalized, false)
        ? valueParser_.getLayer() : Data::NO_LAYER;
    int mandatoryLayer = xmldata::parseBoolean(attrMandatory, false)
        ? valueParser_.getLayer() : Data::NO_LAYER;
    NodeMap::iterator i(set->getMembers().find(name));
    if (i != set->getMembers().end()) {
        finalizedLayer = std::min(finalizedLayer, i->second->getFinalized());
        i->second->setFinalized(finalizedLayer);
        mandatoryLayer = std::min(mandatoryLayer, i->second->getMandatory());
        i->second->setMandatory(mandatoryLayer);
        if (i->second->getLayer() > valueParser_.getLayer()) {
            state_.push(State()); // ignored
            return;
        }
    }
    switch (op) {
    case OPERATION_MODIFY:
        if (i == set->getMembers().end()) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "invalid modify of unknown set member node ")) +
                 name + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" in ")) +
                 reader.getUrl()),
                css::uno::Reference< css::uno::XInterface >());
        }
        state_.push(
            State(
                i->second,
                state_.top().locked || finalizedLayer < valueParser_.getLayer(),
                false));
        break;
    case OPERATION_REPLACE:
        if (state_.top().locked || finalizedLayer < valueParser_.getLayer()) {
            state_.push(State()); // ignored
        } else {
            rtl::Reference< Node > member(tmpl->clone());
            member->setLayer(valueParser_.getLayer());
            member->setFinalized(finalizedLayer);
            member->setMandatory(mandatoryLayer);
            state_.push(State(member, name, false, false));
        }
        break;
    case OPERATION_FUSE:
        if (i == set->getMembers().end()) {
            if (state_.top().locked || finalizedLayer < valueParser_.getLayer())
            {
                state_.push(State()); // ignored
            } else {
                rtl::Reference< Node > member(tmpl->clone());
                member->setLayer(valueParser_.getLayer());
                member->setFinalized(finalizedLayer);
                member->setMandatory(mandatoryLayer);
                state_.push(State(member, name, false, false));
            }
        } else {
            state_.push(
                State(
                    i->second,
                    (state_.top().locked ||
                     finalizedLayer < valueParser_.getLayer()),
                    false));
        }
        break;
    case OPERATION_REMOVE:
        // Ignore removal of unknown members, members finalized in a lower
        // layer, and members made mandatory in this or a lower layer:
        if (i != set->getMembers().end() && !state_.top().locked &&
            finalizedLayer >= valueParser_.getLayer() &&
            mandatoryLayer > valueParser_.getLayer())
        {
            set->getMembers().erase(i);
        }
        state_.push(State());
        break;
    }
}

}
