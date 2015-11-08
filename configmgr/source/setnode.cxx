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

#include <algorithm>
#include <functional>
#include <vector>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

SetNode::SetNode(
    int layer, OUString const & defaultTemplateName,
    OUString const & templateName):
    Node(layer), defaultTemplateName_(defaultTemplateName),
    templateName_(templateName), mandatory_(Data::NO_LAYER)
{}

rtl::Reference< Node > SetNode::clone(bool keepTemplateName) const {
    return new SetNode(*this, keepTemplateName);
}

NodeMap & SetNode::getMembers() {
    return members_;
}

OUString SetNode::getTemplateName() const {
    return templateName_;
}

void SetNode::setMandatory(int layer) {
    mandatory_ = layer;
}

int SetNode::getMandatory() const {
    return mandatory_;
}



bool SetNode::isValidTemplate(OUString const & templateName) const {
    return Data::equalTemplateNames(templateName, defaultTemplateName_) ||
        (std::find_if(
            additionalTemplateNames_.begin(),
            additionalTemplateNames_.end(),
            [&templateName](OUString const & longName) { return Data::equalTemplateNames(templateName, longName); } ) !=
         additionalTemplateNames_.end());
}

SetNode::SetNode(SetNode const & other, bool keepTemplateName):
    Node(other), defaultTemplateName_(other.defaultTemplateName_),
    additionalTemplateNames_(other.additionalTemplateNames_),
    mandatory_(other.mandatory_)
{
    other.members_.cloneInto(&members_);
    if (keepTemplateName) {
        templateName_ = other.templateName_;
    }
}

SetNode::~SetNode() {}

Node::Kind SetNode::kind() const {
    return KIND_SET;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
