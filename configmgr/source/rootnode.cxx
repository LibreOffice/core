/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "rtl/ref.hxx"

#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "rootnode.hxx"

namespace configmgr {

RootNode::RootNode(): Node(Data::NO_LAYER) {}

RootNode::~RootNode() {}

Node::Kind RootNode::kind() const {
    return KIND_ROOT;
}

rtl::Reference< Node > RootNode::clone(bool) const {
    assert(false); // this cannot happen
    return rtl::Reference< Node >();
}

NodeMap & RootNode::getMembers() {
    return members_;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
