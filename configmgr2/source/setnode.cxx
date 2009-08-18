/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <algorithm>
#include <vector>

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "layer.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "setnode.hxx"

namespace configmgr {

SetNode::SetNode(
    int layer, rtl::OUString const & defaultTemplateName,
    rtl::OUString const & templateName):
    Node(layer), defaultTemplateName_(defaultTemplateName),
    templateName_(templateName), mandatory_(NO_LAYER)
{}

rtl::Reference< Node > SetNode::clone() const {
    return new SetNode(*this);
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
    return templateName == defaultTemplateName_ ||
        (std::find(
            additionalTemplateNames_.begin(), additionalTemplateNames_.end(),
            templateName) !=
         additionalTemplateNames_.end());
}

NodeMap & SetNode::getMembers() {
    return members_;
}

SetNode::SetNode(SetNode const & other):
    Node(other), defaultTemplateName_(other.defaultTemplateName_),
    additionalTemplateNames_(other.additionalTemplateNames_),
    templateName_(other.templateName_), mandatory_(other.mandatory_)
{
    cloneNodeMap(other.members_, &members_);
}

SetNode::~SetNode() {}

void SetNode::clear() {
    members_.clear();
}

rtl::Reference< Node > SetNode::findMember(rtl::OUString const & name) {
    NodeMap::iterator i(members_.find(name));
    return i == members_.end() ? rtl::Reference< Node >() : i->second;
}

}
