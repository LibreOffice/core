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

#include "localizedvaluenode.hxx"
#include "node.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

LocalizedValueNode::LocalizedValueNode(int layer, css::uno::Any const & value):
    Node(layer), value_(value)
{}

rtl::Reference< Node > LocalizedValueNode::clone(bool) const {
    return new LocalizedValueNode(*this);
}

rtl::OUString LocalizedValueNode::getTemplateName() const {
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
}

css::uno::Any LocalizedValueNode::getValue() const {
    return value_;
}

void LocalizedValueNode::setValue(int layer, css::uno::Any const & value) {
    setLayer(layer);
    value_ = value;
}

LocalizedValueNode::LocalizedValueNode(LocalizedValueNode const & other):
    Node(other), value_(other.value_)
{}

LocalizedValueNode::~LocalizedValueNode() {}

Node::Kind LocalizedValueNode::kind() const {
    return KIND_LOCALIZED_VALUE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
