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

#include "com/sun/star/uno/Any.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "localizedpropertynode.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

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

Type LocalizedPropertyNode::getStaticType() const {
    return staticType_;
}

bool LocalizedPropertyNode::isNillable() const {
    return nillable_;
}

LocalizedPropertyNode::LocalizedPropertyNode(
    LocalizedPropertyNode const & other):
    Node(other), staticType_(other.staticType_), nillable_(other.nillable_)
{
    cloneNodeMap(other.members_, &members_);
}

LocalizedPropertyNode::~LocalizedPropertyNode() {}

Node::Kind LocalizedPropertyNode::kind() const {
    return KIND_LOCALIZED_PROPERTY;
}

void LocalizedPropertyNode::clear() {
    members_.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
