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

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "node.hxx"
#include "propertynode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

PropertyNode::PropertyNode(
    rtl::OUString const & name, Type type, bool nillable,
    css::uno::Any const & value, bool extension):
    name_(name), type_(type), nillable_(nillable), value_(value),
    extension_(extension)
{}

PropertyNode::~PropertyNode() {}

Node * PropertyNode::clone() const {
    return new PropertyNode(*this);
}

Node * PropertyNode::clone(rtl::OUString const &) const {
    OSL_ASSERT(false);
    throw css::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
        0);
}

rtl::OUString PropertyNode::getName() {
    return name_;
}

Node * PropertyNode::getMember(rtl::OUString const &) {
    return 0;
}

Type PropertyNode::getType() const {
    return type_;
}

bool PropertyNode::isNillable() const {
    return nillable_;
}

css::uno::Any PropertyNode::getValue() const {
    return value_;
}

void PropertyNode::setValue(css::uno::Any const & value) {
    value_ = value;
}

bool PropertyNode::isExtension() const {
    return extension_;
}

}
