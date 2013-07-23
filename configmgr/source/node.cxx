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

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

NodeMap & Node::getMembers() {
    assert(false);
    throw css::uno::RuntimeException(
        "this cannot happen", css::uno::Reference< css::uno::XInterface >());
}

OUString Node::getTemplateName() const {
    return OUString();
}

void Node::setMandatory(int layer) {
    (void) layer; // avoid warnings
    assert(layer == Data::NO_LAYER);
}

int Node::getMandatory() const {
    return Data::NO_LAYER;
}

void Node::setLayer(int layer) {
    assert(layer >= layer_);
    layer_ = layer;
}

int Node::getLayer() const {
    return layer_;
}

void Node::setFinalized(int layer) {
    finalized_ = layer;
}

int Node::getFinalized() const {
    return finalized_;
}

rtl::Reference< Node > Node::getMember(OUString const & name) {
    NodeMap const & members = getMembers();
    NodeMap::const_iterator i(members.find(name));
    return i == members.end() ? rtl::Reference< Node >() : i->second;
}

Node::Node(int layer): layer_(layer), finalized_(Data::NO_LAYER) {}

Node::Node(const Node & other):
    SimpleReferenceObject(), layer_(other.layer_), finalized_(other.finalized_)
{}

Node::~Node() {}

void Node::clear() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
