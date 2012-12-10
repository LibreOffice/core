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

#include <algorithm>
#include <functional>
#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "data.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

namespace {

// Work around some compilers' failure to accept
// std::binder1st(std::ptr_fun(&Data::equalTemplateNames), ...):
class EqualTemplateNames:
    public std::unary_function< OUString const &, bool >
{
public:
    inline explicit EqualTemplateNames(OUString const & shortName):
        shortName_(shortName) {}

    inline bool operator ()(OUString const & longName) const
    { return Data::equalTemplateNames(shortName_, longName); }

private:
    OUString const & shortName_;
};

}

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

OUString const & SetNode::getDefaultTemplateName() const {
    return defaultTemplateName_;
}

std::vector< OUString > & SetNode::getAdditionalTemplateNames() {
    return additionalTemplateNames_;
}

bool SetNode::isValidTemplate(OUString const & templateName) const {
    return Data::equalTemplateNames(templateName, defaultTemplateName_) ||
        (std::find_if(
            additionalTemplateNames_.begin(),
            additionalTemplateNames_.end(), EqualTemplateNames(templateName)) !=
         additionalTemplateNames_.end());
}

SetNode::SetNode(SetNode const & other, bool keepTemplateName):
    Node(other), defaultTemplateName_(other.defaultTemplateName_),
    additionalTemplateNames_(other.additionalTemplateNames_),
    mandatory_(other.mandatory_)
{
    cloneNodeMap(other.members_, &members_);
    if (keepTemplateName) {
        templateName_ = other.templateName_;
    }
}

SetNode::~SetNode() {}

Node::Kind SetNode::kind() const {
    return KIND_SET;
}

void SetNode::clear() {
    members_.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
