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

#include <memory>

#include "rtl/ustring.hxx"

#include "groupnode.hxx"
#include "node.hxx"
#include "nodemap.hxx"

namespace configmgr {

GroupNode::GroupNode(rtl::OUString const & name, bool extensible):
    name_(name), extensible_(extensible) {}

GroupNode::~GroupNode() {}

GroupNode * GroupNode::clone() const {
    return new GroupNode(*this);
}

Node * GroupNode::clone(rtl::OUString const & name) const {
    std::auto_ptr< GroupNode > p(clone());
    p->name_ = name;
    return p.release();
}

rtl::OUString GroupNode::getName() {
    return name_;
}

Node * GroupNode::getMember(rtl::OUString const & name) {
    NodeMap::iterator i(members_.find(name));
    return i == members_.end() ? 0 : i->second;
}

bool GroupNode::isExtensible() const {
    return extensible_;
}

NodeMap & GroupNode::getMembers() {
    return members_;
}

}
