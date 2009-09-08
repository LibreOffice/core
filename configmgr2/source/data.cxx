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

#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

bool isPrefix(rtl::OUString const & prefix, rtl::OUString const & path) {
    return prefix.getLength() < path.getLength() && path.match(prefix) &&
        path[prefix.getLength()] == '/';
}

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

rtl::OUString Data::parseLastSegment(
    rtl::OUString const & path, rtl::OUString * name)
{
    OSL_ASSERT(name != 0);
    sal_Int32 i = path.getLength();
    OSL_ASSERT(i > 0 && path[i - 1] != '/');
    if (path[i - 1] == ']') {
        OSL_ASSERT(i > 2 && (path[i - 2] == '\'' || (path[i - 2] == '"')));
        sal_Int32 j = path.lastIndexOf(path[i - 2], i - 2);
        OSL_ASSERT(j > 0);
        decode(path, j + 1, i - 2, name);
        i = path.lastIndexOf('/', j);
    } else {
        i = path.lastIndexOf('/');
        *name = path.copy(i + 1);
    }
    OSL_ASSERT(i != -1);
    return path.copy(0, i);
}

rtl::OUString Data::fullTemplateName(
    rtl::OUString const & component, rtl::OUString const & name)
{
    OSL_ASSERT(component.indexOf(':') == -1);
    rtl::OUStringBuffer buf(component);
    buf.append(sal_Unicode(':'));
    buf.append(name);
    return buf.makeStringAndClear();
}

rtl::Reference< Node > Data::findNode(
    int layer, NodeMap const & map, rtl::OUString const & name)
{
    NodeMap::const_iterator i(map.find(name));
    return
        (i == map.end() || i->second->getLayer() > layer ||
         i->second->isRemoved())
        ? rtl::Reference< Node >() : i->second;
}

rtl::Reference< Node > Data::resolvePath(
    rtl::OUString const & path, rtl::OUString * firstSegment,
    rtl::OUString * lastSegment, rtl::OUString * canonicalPath,
    rtl::Reference< Node > * parent, int * finalizedLayer) const
{
    if (path.getLength() == 0 || path[0] != '/') {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path, 0);
    }
    rtl::OUString seg;
    bool setElement;
    sal_Int32 n = parseSegment(path, 1, &seg, &setElement, 0);
    if (n == -1 || setElement)
    {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path, 0);
    }
    if (firstSegment != 0) {
        *firstSegment = seg;
    }
    NodeMap::const_iterator i(components.find(seg));
    rtl::OUStringBuffer canonic;
    rtl::Reference< Node > par;
    int finalized = NO_LAYER;
    for (rtl::Reference< Node > p(i == components.end() ? 0 : i->second);;) {
        if (!p.is()) {
            return p;
        }
        if (canonicalPath != 0) {
            canonic.append(sal_Unicode('/'));
            canonic.append(createSegment(p->getTemplateName(), seg));
        }
        finalized = std::min(finalized, p->getFinalized());
        if (n != path.getLength() && path[n++] != '/') {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                css::uno::Reference< css::uno::XInterface >());
        }
        // for backwards compatibility, ignore a final slash
        if (n == path.getLength()) {
            if (lastSegment != 0) {
                *lastSegment = seg;
            }
            if (canonicalPath != 0) {
                *canonicalPath = canonic.makeStringAndClear();
            }
            if (parent != 0) {
                *parent = par;
            }
            if (finalizedLayer != 0) {
                *finalizedLayer = finalized;
            }
            return p;
        }
        par = p;
        rtl::OUString templateName;
        n = parseSegment(path, n, &seg, &setElement, &templateName);
        if (n == -1) {
            throw css::uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
                css::uno::Reference< css::uno::XInterface >());
        }
        if (setElement) {
            if (p->kind() != Node::KIND_SET ||
                (templateName.getLength() != 0 &&
                 !dynamic_cast< SetNode * >(p.get())->isValidTemplate(
                     templateName)))
            {
                throw css::uno::RuntimeException(
                    (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                     path),
                    css::uno::Reference< css::uno::XInterface >());
            }
            p = p->getMember(seg);
            if (templateName.getLength() != 0 && p != 0) {
                rtl::OUString name(p->getTemplateName());
                OSL_ASSERT(name.getLength() != 0);
                if (templateName != name) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("bad path ")) +
                         path),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        } else {
            // For backwards compatibility, allow set members to be accessed
            // with simple path segments, like group members:
            p = p->getMember(seg);
        }
    }
}

rtl::Reference< Node > Data::getTemplate(
    int layer, rtl::OUString const & fullName) const
{
    return findNode(layer, templates, fullName);
}

void Data::addModification(rtl::OUString const & path) {
    //TODO
    for (Modifications::iterator i(modifications.begin());
         i != modifications.end();)
    {
        if (path == *i || isPrefix(*i, path)) {
            return;
        }
        if (isPrefix(path, *i)) {
            modifications.erase(i++);
        } else {
            ++i;
        }
    }
    modifications.push_back(path);
}

}
