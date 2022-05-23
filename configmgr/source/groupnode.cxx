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
#include <rtl/ustring.hxx>
#include <utility>

#include "data.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

GroupNode::GroupNode(
    int layer, bool extensible, OUString templateName):
    Node(layer), extensible_(extensible), templateName_(std::move(templateName)),
    mandatory_(Data::NO_LAYER)
{}

rtl::Reference< Node > GroupNode::clone(bool keepTemplateName) const {
    return new GroupNode(*this, keepTemplateName);
}

NodeMap & GroupNode::getMembers() {
    return members_;
}

OUString GroupNode::getTemplateName() const {
    return templateName_;
}

void GroupNode::setMandatory(int layer) {
    mandatory_ = layer;
}

int GroupNode::getMandatory() const {
    return mandatory_;
}


GroupNode::GroupNode(GroupNode const & other, bool keepTemplateName):
    Node(other), extensible_(other.extensible_), mandatory_(other.mandatory_)
{
    other.members_.cloneInto(&members_);
    if (keepTemplateName) {
        templateName_ = other.templateName_;
    }
}

GroupNode::~GroupNode() {}

Node::Kind GroupNode::kind() const {
    return KIND_GROUP;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
