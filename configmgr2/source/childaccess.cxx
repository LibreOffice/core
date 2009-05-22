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
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "rtl/uuid.h"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "localizedpropertynode.hxx"
#include "localizedpropertyvaluenode.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

css::uno::Sequence< sal_Int8 > ChildAccess::getTunnelId() {
    static css::uno::Sequence< sal_Int8 > id;
    if (id.getLength() == 0) {
        css::uno::Sequence< sal_Int8 > uuid(16);
        rtl_createUuid(
            reinterpret_cast< sal_uInt8 * >(uuid.getArray()), 0, false);
        id = uuid;
    }
    return id;
}

ChildAccess::ChildAccess(
    RootAccess * root, Access * parent, rtl::Reference< Node > const & node):
    root_(root), parent_(parent), node_(node)
{
    OSL_ASSERT(root != 0 && parent != 0 && node.is());
}

ChildAccess::ChildAccess(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Node > const & node):
    root_(root.get()), acquiredRoot_(root), parent_(0), node_(node)
{
    OSL_ASSERT(root.is() && node.is());
}

rtl::Reference< Node > ChildAccess::getNode() {
    return node_;
}

rtl::Reference< RootAccess > ChildAccess::getRoot() {
    return root_;
}

Access * ChildAccess::getParentAccess() const {
    return parent_;
}

void ChildAccess::bind(RootAccess * root, Access * parent) throw () {
    OSL_ASSERT(acquiredRoot_.is());
    acquiredRoot_.clear();
    root_ = root;
    parent_ = parent;
}

void ChildAccess::unbind() throw () {
    OSL_ASSERT(!acquiredRoot_.is());
    acquiredRoot_ = root_;
    parent_ = 0;
}

css::uno::Any ChildAccess::asValue() {
    rtl::Reference< Node > p(getNode());
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(p.get())) {
        return prop->getValue();
    }
    if (LocalizedPropertyValueNode * locval =
        dynamic_cast< LocalizedPropertyValueNode * >(p.get()))
    {
        return locval->getValue();
    }
    if (LocalizedPropertyNode * locprop =
        dynamic_cast< LocalizedPropertyNode * >(p.get()))
    {
        rtl::OUString locale(getRoot()->getLocale());
        if (!Components::allLocales(locale)) {
            rtl::Reference< LocalizedPropertyValueNode > value(
                locprop->getValue(locale));
            return value.is() ? value->getValue() : css::uno::Any();
        }
    }
    return css::uno::makeAny(
        css::uno::Reference< css::uno::XInterface >(
            static_cast< cppu::OWeakObject * >(this)));
}

ChildAccess::~ChildAccess() {}

sal_Int64 ChildAccess::getSomething(
    css::uno::Sequence< sal_Int8 > const & aIdentifier)
    throw (css::uno::RuntimeException)
{
    return aIdentifier == getTunnelId()
        ? reinterpret_cast< sal_Int64 >(this) : 0;
}

}
