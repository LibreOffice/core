/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
