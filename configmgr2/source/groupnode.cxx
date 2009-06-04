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

#include "components.hxx"
#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

GroupNode::GroupNode(bool extensible, rtl::OUString const & templateName):
    extensible_(extensible), templateName_(templateName)
{}

rtl::Reference< Node > GroupNode::clone() const {
    rtl::Reference< GroupNode > fresh(
        new GroupNode(extensible_, templateName_));
    members_.clone(&fresh->members_);
    return fresh.get();
}

rtl::Reference< Node > GroupNode::getMember(rtl::OUString const & name) {
    NodeMap::iterator i(Components::resolveNode(name, &members_));
    return i == members_.end() ? rtl::Reference< Node >() : i->second;
}

bool GroupNode::isExtensible() const {
    return extensible_;
}

rtl::OUString GroupNode::getTemplateName() const {
    return templateName_;
}

NodeMap & GroupNode::getMembers() {
    return members_;
}

GroupNode::~GroupNode() {}

}
