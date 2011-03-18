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

#include <map>
#include <set>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "partial.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

bool parseSegment(
    rtl::OUString const & path, sal_Int32 * index, rtl::OUString * segment)
{
    OSL_ASSERT(
        index != 0 && *index >= 0 && *index <= path.getLength() &&
        segment != 0);
    if (path[(*index)++] == '/') {
        rtl::OUString name;
        bool setElement;
        rtl::OUString templateName;
        *index = Data::parseSegment(
            path, *index, &name, &setElement, &templateName);
        if (*index != -1) {
            *segment = Data::createSegment(templateName, name);
            return *index == path.getLength();
        }
    }
    throw css::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad path ")) + path,
        css::uno::Reference< css::uno::XInterface >());
}

}

Partial::Partial(
    std::set< rtl::OUString > const & includedPaths,
    std::set< rtl::OUString > const & excludedPaths)
{
    for (std::set< rtl::OUString >::const_iterator i(includedPaths.begin());
         i != includedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            rtl::OUString seg;
            bool end = parseSegment(*i, &n, &seg);
            p = &p->children[seg];
            if (p->startInclude) {
                break;
            }
            if (end) {
                p->children.clear();
                p->startInclude = true;
                break;
            }
        }
    }
    for (std::set< rtl::OUString >::const_iterator i(excludedPaths.begin());
         i != excludedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            rtl::OUString seg;
            bool end = parseSegment(*i, &n, &seg);
            if (end) {
                p->children[seg] = Node();
                break;
            }
            Node::Children::iterator j(p->children.find(seg));
            if (j == p->children.end()) {
                break;
            }
            p = &j->second;
        }
    }
}

Partial::~Partial() {}

Partial::Containment Partial::contains(Path const & path) const {
    //TODO: For set elements, the segment names recorded in the node tree need
    // not match the corresponding path segments, so this function can fail.
    Node const * p = &root_;
    bool includes = false;
    for (Path::const_iterator i(path.begin()); i != path.end(); ++i) {
        Node::Children::const_iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            break;
        }
        p = &j->second;
        includes |= p->startInclude;
    }
    return p->children.empty() && !p->startInclude
        ? CONTAINS_NOT
        : includes ? CONTAINS_NODE : CONTAINS_SUBNODES;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
