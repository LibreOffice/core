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

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

GroupNode::GroupNode(
    int layer, bool extensible, rtl::OUString const & templateName):
    Node(layer), extensible_(extensible), templateName_(templateName)
{}

rtl::Reference< Node > GroupNode::clone() const {
    rtl::Reference< GroupNode > fresh(
        new GroupNode(getLayer(), extensible_, templateName_));
    cloneNodeMap(members_, &fresh->members_);
    return fresh.get();
}

rtl::OUString GroupNode::getTemplateName() const {
    return templateName_;
}

bool GroupNode::isExtensible() const {
    return extensible_;
}

NodeMap & GroupNode::getMembers() {
    return members_;
}

GroupNode::~GroupNode() {}

void GroupNode::clear() {
    members_.clear();
}

rtl::Reference< Node > GroupNode::findMember(rtl::OUString const & name) {
    NodeMap::iterator i(members_.find(name));
    return i == members_.end() ? rtl::Reference< Node >() : i->second;
}

}
