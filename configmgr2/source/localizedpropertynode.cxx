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

#include "localizedpropertynode.hxx"
#include "localizedvalues.hxx"
#include "node.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

LocalizedPropertyNode::LocalizedPropertyNode(
    rtl::OUString const & name, Type type, bool nillable,
    LocalizedValues const & values):
    name_(name), type_(type), nillable_(nillable), values_(values) {}

LocalizedPropertyNode::~LocalizedPropertyNode() {}

Node * LocalizedPropertyNode::clone() const {
    return new LocalizedPropertyNode(*this);
}

Node * LocalizedPropertyNode::clone(rtl::OUString const &) const {
    OSL_ASSERT(false);
    throw css::uno::RuntimeException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
        0);
}

rtl::OUString LocalizedPropertyNode::getName() {
    return name_;
}

Node * LocalizedPropertyNode::getMember(rtl::OUString const &) {
    return 0; //TODO
}

Type LocalizedPropertyNode::getType() const {
    return type_;
}

bool LocalizedPropertyNode::isNillable() const {
    return nillable_;
}

LocalizedValues & LocalizedPropertyNode::getValues() {
    return values_;
}

void LocalizedPropertyNode::setValues(LocalizedValues const & values) {
    values_ = values;
}

css::uno::Any LocalizedPropertyNode::getValue(rtl::OUString const & locale)
    const
{
    //TODO
    LocalizedValues::const_iterator i(values_.find(locale));
    if (i != values_.end()) {
        return i->second;
    }
    i = values_.find(rtl::OUString());
    if (i != values_.end()) {
        return i->second;
    }
    i = values_.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en-US")));
    if (i != values_.end()) {
        return i->second;
    }
    i = values_.begin();
    if (i != values_.end()) {
        return i->second;
    }
    return css::uno::Any();
}

}
