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

#include <vector>

#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "rtl/uuid.h"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "data.hxx"
#include "groupnode.hxx"
#include "layer.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

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
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, rtl::OUString const & name,
    rtl::Reference< Node > const & node):
    root_(root), parent_(parent), name_(name), node_(node), modified_(false),
    inTransaction_(false)
{
    OSL_ASSERT(root.is() && parent.is() && node.is());
}

ChildAccess::ChildAccess(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Node > const & node):
    root_(root), node_(node), modified_(false), inTransaction_(false)
{
    OSL_ASSERT(root.is() && node.is());
}

rtl::OUString ChildAccess::getPath() {
    rtl::OUString path;
    rtl::Reference< Access > parent(getParentAccess());
    if (parent.is()) {
        path = parent->getPath() +
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    }
    return path + Data::createSegment(node_->getTemplateName(), name_);
}

rtl::Reference< Node > ChildAccess::getNode() {
    return node_;
}

bool ChildAccess::isFinalized() {
    return node_->getFinalized() != NO_LAYER ||
        (parent_.is() && parent_->isFinalized());
}

rtl::Reference< RootAccess > ChildAccess::getRootAccess() {
    return root_;
}

rtl::Reference< Access > ChildAccess::getParentAccess() {
    return parent_;
}

rtl::OUString ChildAccess::getName() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return name_;
}

css::uno::Reference< css::uno::XInterface > ChildAccess::getParent()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return static_cast< cppu::OWeakObject * >(parent_.get());
}

void ChildAccess::setParent(css::uno::Reference< css::uno::XInterface > const &)
    throw (css::lang::NoSupportException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    throw css::lang::NoSupportException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("setParent")),
        static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 ChildAccess::getSomething(
    css::uno::Sequence< sal_Int8 > const & aIdentifier)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return aIdentifier == getTunnelId()
        ? reinterpret_cast< sal_Int64 >(this) : 0;
}

oslInterlockedCount ChildAccess::acquireCounting() {
    return osl_incrementInterlockedCount(&m_refCount);
}

void ChildAccess::releaseNondeleting() {
    osl_decrementInterlockedCount(&m_refCount);
}

void ChildAccess::bind(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, rtl::OUString const & name)
    throw ()
{
    OSL_ASSERT(
        !parent_.is() && root.is() && parent.is() && name.getLength() != 0);
    root_ = root;
    parent_ = parent;
    name_ = name;
}

void ChildAccess::unbind() throw () {
    OSL_ASSERT(parent_.is());
    parent_->releaseChild(name_);
    parent_.clear();
    //TODO: clear name_?
    inTransaction_ = true;
}

void ChildAccess::markAsModified() {
    modified_ = true;
    for (ChildAccess * p = this; p != 0 && p->parent_.is();
         p = dynamic_cast< ChildAccess * >(p->parent_.get()))
    {
        p->parent_->modifiedChildren_[p->name_] = p;
    }
}

void ChildAccess::committed() {
    modified_ = false;
    inTransaction_ = false;
}

void ChildAccess::setNode(rtl::Reference< Node > const & node) {
    node_ = node;
}

void ChildAccess::setProperty(css::uno::Any const & value) {
    Type type = TYPE_ERROR;
    bool nillable = false;
    switch (node_->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(node_.get());
            type = prop->getType();
            nillable = prop->isNillable();
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            rtl::OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale)) {
                rtl::Reference< ChildAccess > child(getChild(locale));
                if (child.is()) {
                    child->setProperty(value);
                } else {
                    insertLocalizedValueChild(locale, value);
                }
                return;
            }
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
            type = locprop->getType();
            nillable = locprop->isNillable();
        }
        break;
    default:
        break;
    }
    checkValue(value, type, nillable);
    markAsModified();
    changedValue_.reset(new css::uno::Any(value));
    //TODO notify change
}

css::uno::Any ChildAccess::asValue() {
    if (changedValue_.get() != 0) {
        return *changedValue_;
    }
    switch (node_->kind()) {
    case Node::KIND_PROPERTY:
        return dynamic_cast< PropertyNode * >(node_.get())->getValue();
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            rtl::OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale)) {
                rtl::Reference< ChildAccess > child(getChild(locale));
                if (!child.is()) {
                    //TODO: find best match
                    child = getChild(rtl::OUString());
                    if (!child.is()) {
                        child = getChild(
                            rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("en-US")));
                        if (!child.is()) {
                            std::vector< rtl::Reference< ChildAccess > > all(
                                getAllChildren());
                            if (all.empty()) {
                                return css::uno::Any();
                            }
                            child = all.front();
                        }
                    }
                }
                return child->asValue();
            }
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        return dynamic_cast< LocalizedValueNode * >(node_.get())->getValue();
    default:
        break;
    }
    return css::uno::makeAny(
        css::uno::Reference< css::uno::XInterface >(
            static_cast< cppu::OWeakObject * >(this)));
}

void ChildAccess::commitChanges(bool valid) {
    commitChildChanges(valid);
    if (valid && changedValue_.get() != 0) {
        Components::singleton().addModification(getPath());
        switch (node_->kind()) {
        case Node::KIND_PROPERTY:
            dynamic_cast< PropertyNode * >(node_.get())->setValue(
                NO_LAYER, *changedValue_);
            break;
        case Node::KIND_LOCALIZED_VALUE:
            dynamic_cast< LocalizedValueNode * >(node_.get())->setValue(
                NO_LAYER, *changedValue_);
            break;
        default:
            OSL_ASSERT(false); // this cannot happen
            break;
        }
    }
    changedValue_.reset();
}

ChildAccess::~ChildAccess() {
    osl::MutexGuard g(lock);
    if (parent_.is()) {
        parent_->releaseChild(name_);
    }
}

void ChildAccess::addSupportedServiceNames(
    std::vector< rtl::OUString > * services)
{
    OSL_ASSERT(services != 0);
    services->push_back(
        getParentNode()->kind() == Node::KIND_GROUP
        ? rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.GroupElement"))
        : rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.SetElement")));
}

}
