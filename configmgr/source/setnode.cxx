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
    public std::unary_function< rtl::OUString const &, bool >
{
public:
    inline explicit EqualTemplateNames(rtl::OUString const & shortName):
        shortName_(shortName) {}

    inline bool operator ()(rtl::OUString const & longName) const
    { return Data::equalTemplateNames(shortName_, longName); }

private:
    rtl::OUString const & shortName_;
};

}

SetNode::SetNode(
    int layer, rtl::OUString const & defaultTemplateName,
    rtl::OUString const & templateName):
    Node(layer), defaultTemplateName_(defaultTemplateName),
    templateName_(templateName), mandatory_(Data::NO_LAYER)
{}

rtl::Reference< Node > SetNode::clone(bool keepTemplateName) const {
    return new SetNode(*this, keepTemplateName);
}

NodeMap & SetNode::getMembers() {
    return members_;
}

rtl::OUString SetNode::getTemplateName() const {
    return templateName_;
}

void SetNode::setMandatory(int layer) {
    mandatory_ = layer;
}

int SetNode::getMandatory() const {
    return mandatory_;
}

rtl::OUString const & SetNode::getDefaultTemplateName() const {
    return defaultTemplateName_;
}

std::vector< rtl::OUString > & SetNode::getAdditionalTemplateNames() {
    return additionalTemplateNames_;
}

bool SetNode::isValidTemplate(rtl::OUString const & templateName) const {
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
