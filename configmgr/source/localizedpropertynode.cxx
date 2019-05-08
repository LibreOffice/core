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

#include <rtl/ref.hxx>

#include "localizedpropertynode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "type.hxx"

namespace configmgr {

LocalizedPropertyNode::LocalizedPropertyNode(
    int layer, Type staticType, bool nillable):
    Node(layer), staticType_(staticType), nillable_(nillable)
{}

rtl::Reference< Node > LocalizedPropertyNode::clone(bool) const {
    return new LocalizedPropertyNode(*this);
}

NodeMap & LocalizedPropertyNode::getMembers() {
    return members_;
}


LocalizedPropertyNode::LocalizedPropertyNode(
    LocalizedPropertyNode const & other):
    Node(other), staticType_(other.staticType_), nillable_(other.nillable_)
{
    other.members_.cloneInto(&members_);
}

LocalizedPropertyNode::~LocalizedPropertyNode() {}

Node::Kind LocalizedPropertyNode::kind() const {
    return KIND_LOCALIZED_PROPERTY;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
