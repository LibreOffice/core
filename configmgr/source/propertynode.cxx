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

#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "components.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

PropertyNode::PropertyNode(
    int layer, Type staticType, bool nillable, css::uno::Any const & value,
    bool extension):
    Node(layer), staticType_(staticType), nillable_(nillable),
    extension_(extension), value_(value)
{}

rtl::Reference< Node > PropertyNode::clone(bool) const {
    return new PropertyNode(*this);
}

Type PropertyNode::getStaticType() const {
    return staticType_;
}

bool PropertyNode::isNillable() const {
    return nillable_;
}

css::uno::Any PropertyNode::getValue(Components & components) {
    if (externalDescriptor_.getLength() != 0) {
        css::beans::Optional< css::uno::Any > val(
            components.getExternalValue(externalDescriptor_));
        if (val.IsPresent) {
            value_ = val.Value; //TODO: check value type
        }
        externalDescriptor_ = rtl::OUString(); // must not throw
    }
    return value_;
}

void PropertyNode::setValue(int layer, css::uno::Any const & value) {
    setLayer(layer);
    value_ = value;
    externalDescriptor_ = rtl::OUString();
}

void PropertyNode::setExternal(int layer, rtl::OUString const & descriptor) {
    OSL_ASSERT(descriptor.getLength() != 0);
    setLayer(layer);
    externalDescriptor_ = descriptor;
}

bool PropertyNode::isExtension() const {
    return extension_;
}

PropertyNode::PropertyNode(PropertyNode const & other):
    Node(other), staticType_(other.staticType_), nillable_(other.nillable_),
    extension_(other.extension_), externalDescriptor_(other.externalDescriptor_),
    value_(other.value_)
{}

PropertyNode::~PropertyNode() {}

Node::Kind PropertyNode::kind() const {
    return KIND_PROPERTY;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
