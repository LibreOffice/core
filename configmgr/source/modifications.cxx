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

#include "rtl/ustring.hxx"

#include "modifications.hxx"
#include "path.hxx"

namespace configmgr {

Modifications::Modifications() {}

Modifications::~Modifications() {}

void Modifications::add(Path const & path) {
    Node * p = &root_;
    bool wasPresent = false;
    for (Path::const_iterator i(path.begin()); i != path.end(); ++i) {
        Node::Children::iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            if (wasPresent && p->children.empty()) {
                return;
            }
            j = p->children.insert(Node::Children::value_type(*i, Node())).
                first;
            wasPresent = false;
        } else {
            wasPresent = true;
        }
        p = &j->second;
    }
    p->children.clear();
}

void Modifications::remove(Path const & path) {
    OSL_ASSERT(!path.empty());
    Node * p = &root_;
    for (Path::const_iterator i(path.begin());;) {
        Node::Children::iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            break;
        }
        if (++i == path.end()) {
            p->children.erase(j);
            if (p->children.empty()) {
                Path parent(path);
                parent.pop_back();
                remove(parent);
            }
            break;
        }
        p = &j->second;
    }
}

Modifications::Node const & Modifications::getRoot() const {
    return root_;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
