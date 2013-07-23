/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <cassert>
#include <vector>

#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppu/unotype.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/weak.hxx"
#include "comphelper/servicehelper.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "data.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "path.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

namespace configmgr {

namespace
{
    class theChildAccessUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theChildAccessUnoTunnelId > {};
}

css::uno::Sequence< sal_Int8 > ChildAccess::getTunnelId()
{
    return theChildAccessUnoTunnelId::get().getSeq();
}

ChildAccess::ChildAccess(
    Components & components, rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, OUString const & name,
    rtl::Reference< Node > const & node):
    Access(components), root_(root), parent_(parent), name_(name), node_(node),
    inTransaction_(false)
{
    lock_ = lock();
    assert(root.is() && parent.is() && node.is());
}

ChildAccess::ChildAccess(
    Components & components, rtl::Reference< RootAccess > const & root,
    rtl::Reference< Node > const & node):
    Access(components), root_(root), node_(node), inTransaction_(false)
{
    lock_ = lock();
    assert(root.is() && node.is());
}

Path ChildAccess::getAbsolutePath() {
    assert(getParentAccess().is());
    Path path(getParentAccess()->getAbsolutePath());
    path.push_back(name_);
    return path;
}

Path ChildAccess::getRelativePath() {
    Path path;
    rtl::Reference< Access > parent(getParentAccess());
    if (parent.is()) {
        path = parent->getRelativePath();
    }
    path.push_back(name_);
    return path;
}

OUString ChildAccess::getRelativePathRepresentation() {
    OUStringBuffer path;
    rtl::Reference< Access > parent(getParentAccess());
    if (parent.is()) {
        path.append(parent->getRelativePathRepresentation());
        if (!path.isEmpty()) {
            path.append(sal_Unicode('/'));
        }
    }
    path.append(Data::createSegment(node_->getTemplateName(), name_));
    return path.makeStringAndClear();
}

rtl::Reference< Node > ChildAccess::getNode() {
    return node_;
}

bool ChildAccess::isFinalized() {
    return node_->getFinalized() != Data::NO_LAYER ||
        (parent_.is() && parent_->isFinalized());
}

OUString ChildAccess::getNameInternal() {
    return name_;
}

rtl::Reference< RootAccess > ChildAccess::getRootAccess() {
    return root_;
}

rtl::Reference< Access > ChildAccess::getParentAccess() {
    return parent_;
}

void ChildAccess::acquire() throw () {
    Access::acquire();
}

void ChildAccess::release() throw () {
    Access::release();
}

css::uno::Reference< css::uno::XInterface > ChildAccess::getParent()
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return static_cast< cppu::OWeakObject * >(parent_.get());
}

void ChildAccess::setParent(css::uno::Reference< css::uno::XInterface > const &)
    throw (css::lang::NoSupportException, css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    throw css::lang::NoSupportException(
        "setParent", static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 ChildAccess::getSomething(
    css::uno::Sequence< sal_Int8 > const & aIdentifier)
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return aIdentifier == getTunnelId()
        ? reinterpret_cast< sal_Int64 >(this) : 0;
}

void ChildAccess::bind(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Access > const & parent, OUString const & name)
    throw ()
{
    assert(!parent_.is() && root.is() && parent.is() && !name.isEmpty());
    root_ = root;
    parent_ = parent;
    name_ = name;
}

void ChildAccess::unbind() throw () {
    assert(parent_.is());
    parent_->releaseChild(name_);
    parent_.clear();
    inTransaction_ = true;
}

void ChildAccess::committed() {
    inTransaction_ = false;
}

void ChildAccess::setNode(rtl::Reference< Node > const & node) {
    node_ = node;
}

void ChildAccess::setProperty(
    css::uno::Any const & value, Modifications * localModifications)
{
    assert(localModifications != 0);
    Type type = TYPE_ERROR;
    bool nillable = false;
    switch (node_->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = dynamic_cast< PropertyNode * >(node_.get());
            type = prop->getStaticType();
            nillable = prop->isNillable();
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale)) {
                rtl::Reference< ChildAccess > child(getChild(locale));
                if (child.is()) {
                    child->setProperty(value, localModifications);
                } else {
                    insertLocalizedValueChild(
                        locale, value, localModifications);
                }
                return;
            }
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
            type = locprop->getStaticType();
            nillable = locprop->isNillable();
        }
        break;
    default:
        break;
    }
    checkValue(value, type, nillable);
    getParentAccess()->markChildAsModified(this);
    changedValue_.reset(new css::uno::Any(value));
    localModifications->add(getRelativePath());
}

css::uno::Any ChildAccess::asValue() {
    if (changedValue_.get() != 0) {
        return *changedValue_;
    }
    switch (node_->kind()) {
    case Node::KIND_PROPERTY:
        return dynamic_cast< PropertyNode * >(node_.get())->getValue(
            getComponents());
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            OUString locale(getRootAccess()->getLocale());
            if (!Components::allLocales(locale)) {
                rtl::Reference< ChildAccess > child(getChild("*" + locale));
                // As a last resort, return a nil value even though it may be
                // illegal for the given property:
                return child.is() ? child->asValue() : css::uno::Any();
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

void ChildAccess::commitChanges(bool valid, Modifications * globalModifications)
{
    assert(globalModifications != 0);
    commitChildChanges(valid, globalModifications);
    if (valid && changedValue_.get() != 0) {
        Path path(getAbsolutePath());
        getComponents().addModification(path);
        globalModifications->add(path);
        switch (node_->kind()) {
        case Node::KIND_PROPERTY:
            dynamic_cast< PropertyNode * >(node_.get())->setValue(
                Data::NO_LAYER, *changedValue_);
            break;
        case Node::KIND_LOCALIZED_VALUE:
            dynamic_cast< LocalizedValueNode * >(node_.get())->setValue(
                Data::NO_LAYER, *changedValue_);
            break;
        default:
            assert(false); // this cannot happen
            break;
        }
    }
    changedValue_.reset();
}

ChildAccess::~ChildAccess() {
    osl::MutexGuard g(*lock_);
    if (parent_.is()) {
        parent_->releaseChild(name_);
    }
}

void ChildAccess::addTypes(std::vector< css::uno::Type > * types) const {
    assert(types != 0);
    types->push_back(cppu::UnoType< css::container::XChild >::get());
    types->push_back(cppu::UnoType< css::lang::XUnoTunnel >::get());
}

void ChildAccess::addSupportedServiceNames(
    std::vector< OUString > * services)
{
    assert(services != 0);
    services->push_back(
        getParentNode()->kind() == Node::KIND_GROUP
        ? OUString("com.sun.star.configuration.GroupElement")
        : OUString("com.sun.star.configuration.SetElement"));
}

css::uno::Any ChildAccess::queryInterface(css::uno::Type const & aType)
    throw (css::uno::RuntimeException)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    css::uno::Any res(Access::queryInterface(aType));
    return res.hasValue()
        ? res
        : cppu::queryInterface(
            aType, static_cast< css::container::XChild * >(this),
            static_cast< css::lang::XUnoTunnel * >(this));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
