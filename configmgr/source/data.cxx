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

#include <algorithm>
#include <cassert>
#include <cstddef>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/ref.hxx>
#include <rtl/string.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <o3tl/string_view.hxx>

#include "additions.hxx"
#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "rootnode.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

bool decode(
    std::u16string_view encoded, std::size_t begin, std::size_t end,
    OUString * decoded)
{
    assert(
        begin <= end && end <= encoded.size() &&
        decoded != nullptr);
    OUStringBuffer buf(end - begin);
    while (begin != end) {
        sal_Unicode c = encoded[begin++];
        if (c == '&') {
            if (o3tl::starts_with(encoded.substr(begin), u"amp;")) {
                buf.append('&');
                begin += RTL_CONSTASCII_LENGTH("amp;");
            } else if (o3tl::starts_with(encoded.substr(begin), u"quot;")) {
                buf.append('"');
                begin += RTL_CONSTASCII_LENGTH("quot;");
            } else if (o3tl::starts_with(encoded.substr(begin), u"apos;")) {
                buf.append('\'');
                begin += RTL_CONSTASCII_LENGTH("apos;");
            } else {
                return false;
            }
            assert(begin <= end);
        } else {
            buf.append(c);
        }
    }
    *decoded = buf.makeStringAndClear();
    return true;
}

}

OUString Data::createSegment(
    std::u16string_view templateName, OUString const & name)
{
    if (templateName.empty()) {
        return name;
    }
    OUStringBuffer buf(128);
    //TODO: verify template name contains no bad chars?
    buf.append(OUString::Concat(templateName) + "['");
    for (sal_Int32 i = 0; i < name.getLength(); ++i) {
        sal_Unicode c = name[i];
        switch (c) {
        case '&':
            buf.append("&amp;");
            break;
        case '"':
            buf.append("&quot;");
            break;
        case '\'':
            buf.append("&apos;");
            break;
        default:
            buf.append(c);
            break;
        }
    }
    buf.append("']");
    return buf.makeStringAndClear();
}

sal_Int32 Data::parseSegment(
    OUString const & path, sal_Int32 index, OUString * name,
    bool * setElement, OUString * templateName)
{
    assert(
        index >= 0 && index <= path.getLength() && name != nullptr &&
        setElement != nullptr);
    sal_Int32 i = index;
    while (i < path.getLength() && path[i] != '/' && path[i] != '[') {
        ++i;
    }
    if (i == path.getLength() || path[i] == '/') {
        *name = path.copy(index, i - index);
        *setElement = false;
        return i;
    }
    if (templateName != nullptr) {
        if (i - index == 1 && path[index] == '*') {
            templateName->clear();
        } else {
            *templateName = path.copy(index, i - index);
        }
    }
    if (++i == path.getLength()) {
        return -1;
    }
    sal_Unicode del = path[i++];
    if (del != '\'' && del != '"') {
        return -1;
    }
    sal_Int32 j = path.indexOf(del, i);
    if (j == -1 || j + 1 == path.getLength() || path[j + 1] != ']' ||
        !decode(path, i, j, name))
    {
        return -1;
    }
    *setElement = true;
    return j + 2;
}

OUString Data::fullTemplateName(
    std::u16string_view component, std::u16string_view name)
{
    if (component.find(':') != std::u16string_view::npos || name.find(':') != std::u16string_view::npos) {
        throw css::uno::RuntimeException(
            OUString::Concat("bad component/name pair containing colon ") + component + "/" +
            name);
    }
    return OUString::Concat(component) + ":" + name;
}

bool Data::equalTemplateNames(
    OUString const & shortName, OUString const & longName)
{
    if (shortName.indexOf(':') == -1) {
        sal_Int32 i = longName.indexOf(':') + 1;
        assert(i > 0);
        return
            rtl_ustr_compare_WithLength(
                shortName.getStr(), shortName.getLength(),
                longName.getStr() + i, longName.getLength() - i) ==
            0;
    } else {
        return shortName == longName;
    }
}

Data::Data(): root_(new RootNode) {}

rtl::Reference< Node > Data::resolvePathRepresentation(
    OUString const & pathRepresentation,
    OUString * canonicRepresentation, std::vector<OUString> * path, int * finalizedLayer)
    const
{
    if (pathRepresentation.isEmpty() || pathRepresentation[0] != '/') {
        throw css::uno::RuntimeException(
            "bad path " + pathRepresentation);
    }
    if (path != nullptr) {
        path->clear();
    }
    if (pathRepresentation == "/") {
        if (canonicRepresentation != nullptr) {
            *canonicRepresentation = pathRepresentation;
        }
        if (finalizedLayer != nullptr) {
            *finalizedLayer = NO_LAYER;
        }
        return root_;
    }
    OUString seg;
    bool setElement;
    OUString templateName;
    sal_Int32 n = parseSegment(pathRepresentation, 1, &seg, &setElement, nullptr);
    if (n == -1 || setElement)
    {
        throw css::uno::RuntimeException(
            "bad path " + pathRepresentation);
    }
    NodeMap const & components = getComponents();
    NodeMap::const_iterator i(components.find(seg));
    OUStringBuffer canonic(128);
    rtl::Reference< Node > parent;
    int finalized = NO_LAYER;
    for (rtl::Reference< Node > p(i == components.end() ? nullptr : i->second);;) {
        if (!p.is()) {
            return p;
        }
        if (canonicRepresentation != nullptr) {
            canonic.append('/');
            canonic.append(createSegment(templateName, seg));
        }
        if (path != nullptr) {
            path->push_back(seg);
        }
        finalized = std::min(finalized, p->getFinalized());
        if (n != pathRepresentation.getLength() &&
            pathRepresentation[n++] != '/')
        {
            throw css::uno::RuntimeException(
                "bad path " + pathRepresentation);
        }
        // for backwards compatibility, ignore a final slash
        if (n == pathRepresentation.getLength()) {
            if (canonicRepresentation != nullptr) {
                *canonicRepresentation = canonic.makeStringAndClear();
            }
            if (finalizedLayer != nullptr) {
                *finalizedLayer = finalized;
            }
            return p;
        }
        parent = p;
        templateName.clear();
        n = parseSegment(
            pathRepresentation, n, &seg, &setElement, &templateName);
        if (n == -1) {
            throw css::uno::RuntimeException(
                "bad path " + pathRepresentation);
        }
        // For backwards compatibility, allow set members to be accessed with
        // simple path segments, like group members:
        p = p->getMember(seg);
        if (setElement) {
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                if (!templateName.isEmpty()) {
                    throw css::uno::RuntimeException(
                        "bad path " + pathRepresentation);
                }
                break;
            case Node::KIND_SET:
                if (!templateName.isEmpty() &&
                    !static_cast< SetNode * >(parent.get())->isValidTemplate(
                        templateName))
                {
                    throw css::uno::RuntimeException(
                        "bad path " + pathRepresentation);
                }
                break;
            default:
                throw css::uno::RuntimeException(
                    "bad path " + pathRepresentation);
            }
            if (!templateName.isEmpty() && p != nullptr) {
                assert(!p->getTemplateName().isEmpty());
                if (!equalTemplateNames(templateName, p->getTemplateName())) {
                    throw css::uno::RuntimeException(
                        "bad path " + pathRepresentation);
                }
            }
        }
    }
}

rtl::Reference< Node > Data::getTemplate(
    int layer, OUString const & fullName) const
{
    return templates.findNode(layer, fullName);
}

NodeMap & Data::getComponents() const {
    return root_->getMembers();
}

Additions * Data::addExtensionXcuAdditions(
    OUString const & url, int layer)
{
    rtl::Reference item(new ExtensionXcu);
    ExtensionXcuAdditions::iterator i(
        extensionXcuAdditions_.emplace(
                url, rtl::Reference< ExtensionXcu >()).first);
    if (i->second.is()) {
        throw css::uno::RuntimeException(
            "already added extension xcu " + url);
    }
    i->second = item;
    item->layer = layer;
    return &item->additions;
}

rtl::Reference< Data::ExtensionXcu > Data::removeExtensionXcuAdditions(
    OUString const & url)
{
    ExtensionXcuAdditions::iterator i(extensionXcuAdditions_.find(url));
    if (i == extensionXcuAdditions_.end()) {
        // This can happen, as migration of pre OOo 3.3 UserInstallation
        // extensions in dp_registry::backend::configuration::BackendImpl::
        // PackageImpl::processPackage_ can cause just-in-time creation of
        // extension xcu files that are never added via addExtensionXcuAdditions
        // (also, there might be url spelling differences between calls to
        // addExtensionXcuAdditions and removeExtensionXcuAdditions?):
        SAL_INFO(
            "configmgr",
            "unknown Data::removeExtensionXcuAdditions(" << url << ")");
        return rtl::Reference< ExtensionXcu >();
    }
    rtl::Reference< ExtensionXcu > item(i->second);
    extensionXcuAdditions_.erase(i);
    return item;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
