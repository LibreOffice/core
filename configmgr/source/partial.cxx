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

#include "sal/config.h"

#include <cassert>
#include <map>
#include <set>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "data.hxx"
#include "partial.hxx"

namespace configmgr {

namespace {

bool parseSegment(
    OUString const & path, sal_Int32 * index, OUString * segment)
{
    assert(
        index != 0 && *index >= 0 && *index <= path.getLength() &&
        segment != 0);
    if (path[(*index)++] == '/') {
        OUString name;
        bool setElement;
        OUString templateName;
        *index = Data::parseSegment(
            path, *index, &name, &setElement, &templateName);
        if (*index != -1) {
            *segment = Data::createSegment(templateName, name);
            return *index == path.getLength();
        }
    }
    throw css::uno::RuntimeException(
        "bad path " + path, css::uno::Reference< css::uno::XInterface >());
}

}

Partial::Partial(
    std::set< OUString > const & includedPaths,
    std::set< OUString > const & excludedPaths)
{
    // The Partial::Node tree built up here encodes the following information:
    // * Inner node, startInclude: an include starts here that contains excluded
    //   sub-trees
    // * Inner node, !startInclude: contains in-/excluded sub-trees
    // * Leaf node, startInclude: an include starts here
    // * Leaf node, !startInclude: an exclude starts here
    for (std::set< OUString >::const_iterator i(includedPaths.begin());
         i != includedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            OUString seg;
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
    for (std::set< OUString >::const_iterator i(excludedPaths.begin());
         i != excludedPaths.end(); ++i)
    {
        sal_Int32 n = 0;
        for (Node * p = &root_;;) {
            OUString seg;
            bool end = parseSegment(*i, &n, &seg);
            if (end) {
                p->children[seg].clear();
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

    // * If path ends at a leaf node or goes past a leaf node:
    // ** If that leaf node is startInclude: => CONTAINS_NODE
    // ** If that leaf node is !startInclude: => CONTAINS_NOT
    // * If path ends at inner node:
    // ** If there is some startInclude along its trace: => CONTAINS_NODE
    // ** If there is no startInclude along its trace: => CONTAINS_SUBNODES
    Node const * p = &root_;
    bool includes = false;
    for (Path::const_iterator i(path.begin()); i != path.end(); ++i) {
        Node::Children::const_iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            return p->startInclude ? CONTAINS_NODE : CONTAINS_NOT;
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
