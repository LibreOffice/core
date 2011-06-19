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

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "additions.hxx"
#include "data.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

bool decode(
    rtl::OUString const & encoded, sal_Int32 begin, sal_Int32 end,
    rtl::OUString * decoded)
{
    OSL_ASSERT(
        begin >= 0 && begin <= end && end <= encoded.getLength() &&
        decoded != 0);
    rtl::OUStringBuffer buf;
    while (begin != end) {
        sal_Unicode c = encoded[begin++];
        if (c == '&') {
            if (encoded.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("amp;"), begin))
            {
                buf.append(sal_Unicode('&'));
                begin += RTL_CONSTASCII_LENGTH("amp;");
            } else if (encoded.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("quot;"), begin))
            {
                buf.append(sal_Unicode('"'));
                begin += RTL_CONSTASCII_LENGTH("quot;");
            } else if (encoded.matchAsciiL(
                           RTL_CONSTASCII_STRINGPARAM("apos;"), begin))
            {
                buf.append(sal_Unicode('\''));
                begin += RTL_CONSTASCII_LENGTH("apos;");
            } else {
                return false;
            }
            OSL_ASSERT(begin <= end);
        } else {
            buf.append(c);
        }
    }
    *decoded = buf.makeStringAndClear();
    return true;
}

}

rtl::OUString Data::createSegment(
    rtl::OUString const & templateName, rtl::OUString const & name)
{
    if (templateName.getLength() == 0) {
        return name;
    }
    rtl::OUStringBuffer buf(templateName);
        //TODO: verify template name contains no bad chars?
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("['"));
    for (sal_Int32 i = 0; i < name.getLength(); ++i) {
        sal_Unicode c = name[i];
        switch (c) {
        case '&':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&amp;"));
            break;
        case '"':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&quot;"));
            break;
        case '\'':
            buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("&apos;"));
            break;
        default:
            buf.append(c);
            break;
        }
    }
    buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("']"));
    return buf.makeStringAndClear();
}

sal_Int32 Data::parseSegment(
    rtl::OUString const & path, sal_Int32 index, rtl::OUString * name,
    bool * setElement, rtl::OUString * templateName)
{
    OSL_ASSERT(
        index >= 0 && index <= path.getLength() && name != 0 &&
        setElement != 0);
    sal_Int32 i = index;
    while (i < path.getLength() && path[i] != '/' && path[i] != '[') {
        ++i;
    }
    if (i == path.getLength() || path[i] == '/') {
        *name = path.copy(index, i - index);
        *setElement = false;
        return i;
    }
    if (templateName != 0) {
        if (i - index == 1 && path[index] == '*') {
            *templateName = rtl::OUString();
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

rtl::OUString Data::fullTemplateName(
    rtl::OUString const & component, rtl::OUString const & name)
{
    if (component.indexOf(':') != -1 || name.indexOf(':') != -1) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "bad component/name pair containing colon ")) +
             component + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
             name),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUStringBuffer buf(component);
    buf.append(sal_Unicode(':'));
    buf.append(name);
    return buf.makeStringAndClear();
}

bool Data::equalTemplateNames(
    rtl::OUString const & shortName, rtl::OUString const & longName)
{
    if (shortName.indexOf(':') == -1) {
        sal_Int32 i = longName.indexOf(':') + 1;
        OSL_ASSERT(i > 0);
        return
            rtl_ustr_compare_WithLength(
                shortName.getStr(), shortName.getLength(),
                longName.getStr() + i, longName.getLength() - i) ==
            0;
    } else {
        return shortName == longName;
    }
}

rtl::Reference< Node > Data::findNode(
    int layer, NodeMap const & map, rtl::OUString const & name)
{
    NodeMap::const_iterator i(map.find(name));
    return i == map.end() || i->second->getLayer() > layer
        ? rtl::Reference< Node >() : i->second;
}

rtl::Reference< Node > Data::resolvePathRepresentation(
    rtl::OUString const & pathRepresentation,
    rtl::OUString * canonicRepresentation, Path * path, int * finalizedLayer)
    const
{
    if (pathRepresentation.getLength() == 0 || pathRepresentation[0] != '/') {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
             pathRepresentation),
            css::uno::Reference< css::uno::XInterface >());
    }
    rtl::OUString seg;
    bool setElement;
    rtl::OUString templateName;
    sal_Int32 n = parseSegment(pathRepresentation, 1, &seg, &setElement, 0);
    if (n == -1 || setElement)
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
             pathRepresentation),
            css::uno::Reference< css::uno::XInterface >());
    }
    NodeMap::const_iterator i(components.find(seg));
    rtl::OUStringBuffer canonic;
    if (path != 0) {
        path->clear();
    }
    rtl::Reference< Node > parent;
    int finalized = NO_LAYER;
    for (rtl::Reference< Node > p(i == components.end() ? 0 : i->second);;) {
        if (!p.is()) {
            return p;
        }
        if (canonicRepresentation != 0) {
            canonic.append(sal_Unicode('/'));
            canonic.append(createSegment(templateName, seg));
        }
        if (path != 0) {
            path->push_back(seg);
        }
        finalized = std::min(finalized, p->getFinalized());
        if (n != pathRepresentation.getLength() &&
            pathRepresentation[n++] != '/')
        {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                 pathRepresentation),
                css::uno::Reference< css::uno::XInterface >());
        }
        // for backwards compatibility, ignore a final slash
        if (n == pathRepresentation.getLength()) {
            if (canonicRepresentation != 0) {
                *canonicRepresentation = canonic.makeStringAndClear();
            }
            if (finalizedLayer != 0) {
                *finalizedLayer = finalized;
            }
            return p;
        }
        parent = p;
        templateName = rtl::OUString();
        n = parseSegment(
            pathRepresentation, n, &seg, &setElement, &templateName);
        if (n == -1) {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                 pathRepresentation),
                css::uno::Reference< css::uno::XInterface >());
        }
        // For backwards compatibility, allow set members to be accessed with
        // simple path segments, like group members:
        p = p->getMember(seg);
        if (setElement) {
            switch (parent->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                if (templateName.getLength() != 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                         pathRepresentation),
                    css::uno::Reference< css::uno::XInterface >());
                }
                break;
            case Node::KIND_SET:
                if (templateName.getLength() != 0 &&
                    !dynamic_cast< SetNode * >(parent.get())->isValidTemplate(
                        templateName))
                {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                         pathRepresentation),
                    css::uno::Reference< css::uno::XInterface >());
                }
                break;
            default:
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                     pathRepresentation),
                    css::uno::Reference< css::uno::XInterface >());
            }
            if (templateName.getLength() != 0 && p != 0) {
                OSL_ASSERT(p->getTemplateName().getLength() != 0);
                if (!equalTemplateNames(templateName, p->getTemplateName())) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                         pathRepresentation),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        }
    }
}

rtl::Reference< Node > Data::getTemplate(
    int layer, rtl::OUString const & fullName) const
{
    return findNode(layer, templates, fullName);
}

Additions * Data::addExtensionXcuAdditions(
    rtl::OUString const & url, int layer)
{
    rtl::Reference< ExtensionXcu > item(new ExtensionXcu);
    ExtensionXcuAdditions::iterator i(
        extensionXcuAdditions_.insert(
            ExtensionXcuAdditions::value_type(
                url, rtl::Reference< ExtensionXcu >())).first);
    if (i->second.is()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "already added extension xcu ")) +
             url),
            css::uno::Reference< css::uno::XInterface >());
    }
    i->second = item;
    item->layer = layer;
    return &item->additions;
}

rtl::Reference< Data::ExtensionXcu > Data::removeExtensionXcuAdditions(
    rtl::OUString const & url)
{
    ExtensionXcuAdditions::iterator i(extensionXcuAdditions_.find(url));
    if (i == extensionXcuAdditions_.end()) {
        // This can happen, as migration of pre OOo 3.3 UserInstallation
        // extensions in dp_registry::backend::configuration::BackendImpl::
        // PackageImpl::processPackage_ can cause just-in-time creation of
        // extension xcu files that are never added via addExtensionXcuAdditions
        // (also, there might be url spelling differences between calls to
        // addExtensionXcuAdditions and removeExtensionXcuAdditions?):
        OSL_TRACE(
            "unknown configmgr::Data::removeExtensionXcuAdditions(%s)",
            rtl::OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr());
        return rtl::Reference< ExtensionXcu >();
    }
    rtl::Reference< ExtensionXcu > item(i->second);
    extensionXcuAdditions_.erase(i);
    return item;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
