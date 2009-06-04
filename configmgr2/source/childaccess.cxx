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

#include <memory>
#include <vector>

#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/ElementChange.hpp"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "rtl/uuid.h"
#include "sal/types.h"

#include "access.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedpropertyvaluenode.hxx"
#include "lock.hxx"
#include "node.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "status.hxx"

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
    root_(root), parent_(parent), name_(name), node_(node),
    nodeGeneration_(node->getGeneration())
{
    OSL_ASSERT(root.is() && parent.is() && node.is());
}

ChildAccess::ChildAccess(
    rtl::Reference< RootAccess > const & root,
    rtl::Reference< Node > const & node):
    root_(root), node_(node), nodeGeneration_(node->getGeneration())
{
    OSL_ASSERT(root.is() && node.is());
}

rtl::Reference< Node > ChildAccess::getNode() {
    return node_;
}

rtl::Reference< RootAccess > ChildAccess::getRootAccess() {
    return root_;
}

rtl::Reference< Access > ChildAccess::getParentAccess() {
    return parent_;
}

rtl::OUString ChildAccess::getName() throw (css::uno::RuntimeException) {
    return name_;
}

css::uno::Reference< css::uno::XInterface > ChildAccess::getParent()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    return static_cast< cppu::OWeakObject * >(parent_.get());
}

void ChildAccess::setParent(css::uno::Reference< css::uno::XInterface > const &)
    throw (css::lang::NoSupportException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    throw css::lang::NoSupportException(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("setParent")),
        static_cast< cppu::OWeakObject * >(this));
}

sal_Int64 ChildAccess::getSomething(
    css::uno::Sequence< sal_Int8 > const & aIdentifier)
    throw (css::uno::RuntimeException)
{
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
    parent_.clear();
    //TODO: clear name_?
}

void ChildAccess::reportChanges(
    std::vector< css::util::ElementChange > * changes) const
{
    OSL_ASSERT(changes != 0);
    if (dynamic_cast< RemovedStatus * >(status_.get()) == 0) {
        for (HardChildMap::const_iterator i(modifiedChildren_.begin());
             i != modifiedChildren_.end(); ++i)
        {
            i->second->reportChanges(changes);
        }
    }
    if (dynamic_cast< ChangedStatus * >(status_.get()) != 0) {
        changes->push_back(css::util::ElementChange()); //TODO
    } else if (dynamic_cast< RemovedStatus * >(status_.get()) != 0) {
        changes->push_back(css::util::ElementChange()); //TODO
    } else if (dynamic_cast< InsertedStatus * >(status_.get()) != 0) {
        changes->push_back(css::util::ElementChange()); //TODO
    } else if (dynamic_cast< TransferedStatus * >(status_.get()) != 0) {
        changes->push_back(css::util::ElementChange()); //TODO
    }
}

void ChildAccess::commitChanges() {
    if (dynamic_cast< RemovedStatus * >(status_.get()) == 0) {
        while (!modifiedChildren_.empty()) {
            rtl::Reference< ChildAccess > child(
                modifiedChildren_.begin()->second);
            child->commitChanges();
            modifiedChildren_.erase(modifiedChildren_.begin());
        }
    }
    if (ChangedStatus * changed =
        dynamic_cast< ChangedStatus * >(status_.get()))
    {
        if (PropertyNode * prop = dynamic_cast< PropertyNode * >(node_.get())) {
            prop->setValue(changed->getValue());
        } else if (LocalizedPropertyValueNode * locval =
                   dynamic_cast< LocalizedPropertyValueNode * >(node_.get()))
        {
            locval->setValue(changed->getValue());
        } else if (LocalizedPropertyNode * locprop =
                   dynamic_cast< LocalizedPropertyNode * >(node_.get()))
        {
            locprop->setValue(
                getRootAccess()->getLocale(), changed->getValue());
        } else {
            OSL_ASSERT(false);
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                static_cast< cppu::OWeakObject * >(this));
        }
    } else if (dynamic_cast< RemovedStatus * >(status_.get()) != 0) {
        rtl::Reference< Node > parent(parent_->getNode());
        if (GroupNode * group = dynamic_cast< GroupNode * >(parent.get())) {
            //TODO: collision handling?
            group->getMembers().erase(name_);
            nodeGeneration_ = node_->nextGeneration();
        } else if (SetNode * set = dynamic_cast< SetNode * >(parent.get())) {
            //TODO: collision handling?
            set->getMembers().erase(name_);
            nodeGeneration_ = node_->nextGeneration();
        } else {
            OSL_ASSERT(false);
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                static_cast< cppu::OWeakObject * >(this));
        }
    } else if (dynamic_cast< InsertedStatus * >(status_.get()) != 0) {
        rtl::Reference< Node > parent(parent_->getNode());
        if (GroupNode * group = dynamic_cast< GroupNode * >(parent.get())) {
            //TODO: collision handling?
            group->getMembers()[name_] = node_;
            nodeGeneration_ = node_->nextGeneration();
        } else if (SetNode * set = dynamic_cast< SetNode * >(parent.get())) {
            //TODO: collision handling?
            set->getMembers()[name_] = node_;
            nodeGeneration_ = node_->nextGeneration();
        } else {
            OSL_ASSERT(false);
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
                static_cast< cppu::OWeakObject * >(this));
        }
    } else if (dynamic_cast< TransferedStatus * >(status_.get()) != 0) {
        //TODO
    }
    status_.release();
}

void ChildAccess::setStatus(std::auto_ptr< Status > status) {
    OSL_ASSERT(status.get() != 0);
    for (ChildAccess * p = this; p != 0 && p->parent_.is();
         p = dynamic_cast< ChildAccess * >(p->parent_.get()))
    {
        p->parent_->modifiedChildren_[p->name_] = p;
    }
    status_ = status;
}

bool ChildAccess::isCurrent() const {
    return dynamic_cast< InsertedStatus * >(status_.get()) != 0 ||
        dynamic_cast< TransferedStatus * >(status_.get()) != 0 ||
        (dynamic_cast< RemovedStatus * >(status_.get()) == 0 &&
         nodeGeneration_ == node_->getGeneration());
}

css::uno::Any ChildAccess::asValue() {
    if (ChangedStatus * changed =
        dynamic_cast< ChangedStatus * >(status_.get()))
    {
        return changed->getValue();
    }
    if (PropertyNode * prop = dynamic_cast< PropertyNode * >(node_.get())) {
        return prop->getValue();
    }
    if (LocalizedPropertyValueNode * locval =
        dynamic_cast< LocalizedPropertyValueNode * >(node_.get()))
    {
        return locval->getValue();
    }
    if (LocalizedPropertyNode * locprop =
        dynamic_cast< LocalizedPropertyNode * >(node_.get()))
    {
        rtl::OUString locale(getRootAccess()->getLocale());
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

ChildAccess::~ChildAccess() {
    osl::MutexGuard g(lock);
    if (parent_.is()) {
        parent_->releaseChild(name_);
    }
}

}
