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
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "localizedpropertyvaluenode.hxx"
#include "node.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

LocalizedPropertyValueNode::LocalizedPropertyValueNode(
    css::uno::Any const & value):
    value_(value)
{}

rtl::Reference< Node > LocalizedPropertyValueNode::clone() const {
    return new LocalizedPropertyValueNode(value_);
}

rtl::Reference< Node > LocalizedPropertyValueNode::getMember(
    rtl::OUString const &)
{
    return rtl::Reference< Node >();
}

css::uno::Any LocalizedPropertyValueNode::getValue() const {
    return value_;
}

void LocalizedPropertyValueNode::setValue(css::uno::Any const & value) {
    value_ = value;
}

LocalizedPropertyValueNode::~LocalizedPropertyValueNode() {}

}
