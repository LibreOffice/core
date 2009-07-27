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

#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "node.hxx"

namespace configmgr {

rtl::OUString Node::getTemplateName() const {
    return rtl::OUString();
}

void Node::setMandatory(bool mandatory) {
    (void) mandatory; // avoid warnings
    OSL_ASSERT(!mandatory);
}

bool Node::isMandatory() const {
    return false;
}

void Node::setLayer(int layer) {
    OSL_ASSERT(layer >= layer_);
    layer_ = layer;
}

int Node::getLayer() const {
    return layer_;
}

void Node::remove(int layer) {
    setLayer(layer);
    removed_ = true;
    clear();
}

bool Node::isRemoved() const {
    return removed_;
}

rtl::Reference< Node > Node::getMember(rtl::OUString const & name) {
    rtl::Reference< Node > node(findMember(name));
    return node.is() && !node->isRemoved() ? node : rtl::Reference< Node >();
}

Node::Node(int layer): layer_(layer), removed_(false) {}

Node::~Node() {}

void Node::clear() {}

rtl::Reference< Node > Node::findMember(rtl::OUString const &) {
    return rtl::Reference< Node >();
}

void Node::resurrect(int layer) {
    setLayer(layer);
    removed_ = false;
}

}
