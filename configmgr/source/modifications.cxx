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

#include <rtl/ustring.hxx>

#include "modifications.hxx"

namespace configmgr {

Modifications::Modifications() {}

Modifications::~Modifications() {}

void Modifications::add(std::vector<OUString> const & path) {
    Node * p = &root_;
    bool wasPresent = false;
    for (auto const& pathItem : path) {
        Node::Children::iterator j(p->children.find(pathItem));
        if (j == p->children.end()) {
            if (wasPresent && p->children.empty()) {
                return;
            }
            j = p->children.emplace(pathItem, Node()).first;
            wasPresent = false;
        } else {
            wasPresent = true;
        }
        p = &j->second;
    }
    p->children.clear();
}

void Modifications::remove(std::vector<OUString> const & path) {
    assert(!path.empty());
    Node * p = &root_;
    for (auto i(path.begin());;) {
        Node::Children::iterator j(p->children.find(*i));
        if (j == p->children.end()) {
            break;
        }
        if (++i == path.end()) {
            p->children.erase(j);
            if (p->children.empty()) {
                std::vector<OUString> parent(path);
                parent.pop_back();
                remove(parent);
            }
            break;
        }
        p = &j->second;
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
