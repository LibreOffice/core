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

#include <sal/config.h>

#include <cassert>
#include <cstdlib>
#include <vector>

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySetInfo.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <com/sun/star/util/ElementChange.hpp>
#include <comphelper/sequence.hxx>
#include <cppu/unotype.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <rtl/character.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include "access.hxx"
#include "broadcaster.hxx"
#include "childaccess.hxx"
#include "components.hxx"
#include "data.hxx"
#include "groupnode.hxx"
#include "localizedpropertynode.hxx"
#include "localizedvaluenode.hxx"
#include "lock.hxx"
#include "modifications.hxx"
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

// Conservatively forbid what is either not an XML Char (including lone
// surrogates, even though they should not appear in well-formed UNO OUString
// instances anyway), or is a slash (as it causes problems in path syntax):
bool isValidName(OUString const & name, bool setMember) {
    for (sal_Int32 i = 0; i != name.getLength();) {
        sal_uInt32 c = name.iterateCodePoints(&i);
        if ((c < 0x20 && !(c == 0x09 || c == 0x0A || c == 0x0D))
            || rtl::isSurrogate(c) || c == 0xFFFE || c == 0xFFFF
            || (!setMember && c == '/'))
        {
            return false;
        }
    }
    return !name.isEmpty();
}

}

oslInterlockedCount Access::acquireCounting() {
    return osl_atomic_increment(&m_refCount);
}

void Access::releaseNondeleting() {
    osl_atomic_decrement(&m_refCount);
}

bool Access::isValue() {
    rtl::Reference< Node > p(getNode());
    switch (p->kind()) {
    case Node::KIND_PROPERTY:
    case Node::KIND_LOCALIZED_VALUE:
        return true;
    case Node::KIND_LOCALIZED_PROPERTY:
        return !Components::allLocales(getRootAccess()->getLocale());
    default:
        return false;
    }
}

void Access::markChildAsModified(rtl::Reference< ChildAccess > const & child) {
    assert(child.is() && child->getParentAccess() == this);
    modifiedChildren_[child->getNameInternal()] = ModifiedChild(child, true);
    for (rtl::Reference< Access > p(this);;) {
        rtl::Reference< Access > parent(p->getParentAccess());
        if (!parent.is()) {
            break;
        }
        assert(dynamic_cast< ChildAccess * >(p.get()) != nullptr);
        parent->modifiedChildren_.emplace(
                p->getNameInternal(),
                ModifiedChild(static_cast< ChildAccess * >(p.get()), false));
        p = parent;
    }
}

void Access::releaseChild(OUString const & name) {
    cachedChildren_.erase(name);
}

void Access::initBroadcaster(
    Modifications::Node const & modifications, Broadcaster * broadcaster)
{
    initBroadcasterAndChanges(modifications, broadcaster, nullptr);
}

css::uno::Sequence< css::uno::Type > Access::getTypes()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    std::vector< css::uno::Type > types;
    types.push_back(cppu::UnoType< css::uno::XInterface >::get());
    types.push_back(cppu::UnoType< css::uno::XWeak >::get());
    types.push_back(cppu::UnoType< css::lang::XTypeProvider >::get());
    types.push_back(cppu::UnoType< css::lang::XServiceInfo >::get());
    types.push_back(cppu::UnoType< css::lang::XComponent >::get());
    types.push_back(cppu::UnoType< css::container::XContainer >::get());
    types.push_back(cppu::UnoType< css::beans::XExactName >::get());
    types.push_back(cppu::UnoType< css::container::XHierarchicalName >::get());
    types.push_back(cppu::UnoType< css::container::XNamed >::get());
    types.push_back(cppu::UnoType< css::beans::XProperty >::get());
    types.push_back(cppu::UnoType< css::container::XElementAccess >::get());
    types.push_back(cppu::UnoType< css::container::XNameAccess >::get());
    if (getNode()->kind() == Node::KIND_GROUP) {
        types.push_back(cppu::UnoType< css::beans::XPropertySetInfo >::get());
        types.push_back(cppu::UnoType< css::beans::XPropertySet >::get());
        types.push_back(cppu::UnoType< css::beans::XMultiPropertySet >::get());
        types.push_back(
            cppu::UnoType< css::beans::XHierarchicalPropertySet >::get());
        types.push_back(
            cppu::UnoType< css::beans::XMultiHierarchicalPropertySet >::get());
        types.push_back(
            cppu::UnoType< css::beans::XHierarchicalPropertySetInfo >::get());
    }
    if (getRootAccess()->isUpdate()) {
        types.push_back(cppu::UnoType< css::container::XNameReplace >::get());
        types.push_back(
            cppu::UnoType< css::container::XHierarchicalNameReplace >::get());
        if (getNode()->kind() != Node::KIND_GROUP ||
            static_cast< GroupNode * >(getNode().get())->isExtensible())
        {
            types.push_back(
                cppu::UnoType< css::container::XNameContainer >::get());
        }
        if (getNode()->kind() == Node::KIND_SET) {
            types.push_back(
                cppu::UnoType< css::lang::XSingleServiceFactory >::get());
        }
    } else {
        types.push_back(
            cppu::UnoType< css::container::XHierarchicalNameAccess >::get());
    }
    addTypes(&types);
    return comphelper::containerToSequence(types);
}

css::uno::Sequence< sal_Int8 > Access::getImplementationId()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return css::uno::Sequence< sal_Int8 >();
}

OUString Access::getImplementationName()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return OUString("org.openoffice-configmgr::Access");
}

sal_Bool Access::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > Access::getSupportedServiceNames()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    std::vector<OUString> services;
    services.emplace_back("com.sun.star.configuration.ConfigurationAccess");
    if (getRootAccess()->isUpdate()) {
        services.emplace_back("com.sun.star.configuration.ConfigurationUpdateAccess");
    }
    services.emplace_back("com.sun.star.configuration.HierarchyAccess");
    services.emplace_back("com.sun.star.configuration.HierarchyElement");
    if (getNode()->kind() == Node::KIND_GROUP) {
        services.emplace_back("com.sun.star.configuration.GroupAccess");
        services.emplace_back("com.sun.star.configuration.PropertyHierarchy");
        if (getRootAccess()->isUpdate()) {
            services.emplace_back("com.sun.star.configuration.GroupUpdate");
        }
    } else {
        services.emplace_back("com.sun.star.configuration.SetAccess");
        services.emplace_back("com.sun.star.configuration.SimpleSetAccess");
        if (getRootAccess()->isUpdate()) {
            services.emplace_back("com.sun.star.configuration.SetUpdate");
            services.emplace_back("com.sun.star.configuration.SimpleSetUpdate");
        }
    }
    addSupportedServiceNames(&services);
    return comphelper::containerToSequence(services);
}

void Access::dispose() {
    assert(thisIs(IS_ANY));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        if (getParentAccess().is()) {
            throw css::uno::RuntimeException(
                "configmgr dispose inappropriate Access",
                static_cast< cppu::OWeakObject * >(this));
        }
        if (disposed_) {
            return;
        }
        initDisposeBroadcaster(&bc);
        clearListeners();
        disposed_ = true;
    }
    bc.send();
}

void Access::addEventListener(
    css::uno::Reference< css::lang::XEventListener > const & xListener)
{
    assert(thisIs(IS_ANY));
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        if (!xListener.is()) {
            throw css::uno::RuntimeException(
                "null listener", static_cast< cppu::OWeakObject * >(this));
        }
        if (!disposed_) {
            disposeListeners_.insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing(
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    } catch (css::lang::DisposedException &) {}
}

void Access::removeEventListener(
    css::uno::Reference< css::lang::XEventListener > const & aListener)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    DisposeListeners::iterator i(disposeListeners_.find(aListener));
    if (i != disposeListeners_.end()) {
        disposeListeners_.erase(i);
    }
}

css::uno::Type Access::getElementType() {
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    rtl::Reference< Node > p(getNode());
    switch (p->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
        return mapType(
            static_cast< LocalizedPropertyNode * >(p.get())->getStaticType());
    case Node::KIND_GROUP:
        //TODO: Should a specific type be returned for a non-extensible group
        // with homogeneous members or for an extensible group that currently
        // has only homogeneous members?
        return cppu::UnoType<void>::get();
    case Node::KIND_SET:
        return cppu::UnoType<void>::get(); //TODO: correct?
    default:
        assert(false);
        throw css::uno::RuntimeException(
            "this cannot happen", static_cast< cppu::OWeakObject * >(this));
    }
}

sal_Bool Access::hasElements() {
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return !getAllChildren().empty(); //TODO: optimize
}

bool Access::getByNameFast(const OUString & name, css::uno::Any & value)
{
    bool bGotValue = false;
    rtl::Reference< ChildAccess > child;

    if (getNode()->kind() != Node::KIND_LOCALIZED_PROPERTY)
    { // try to get it directly
        ModifiedChildren::iterator i(modifiedChildren_.find(name));
        if (i != modifiedChildren_.end())
        {
            child = getModifiedChild(i);
            if (child.is())
            {
                value = child->asValue();
                bGotValue = true;
            }
        }
        else
        {
            rtl::Reference< Node > node(getNode()->getMember(name));
            if (!node.is())
                return false;
            bGotValue = ChildAccess::asSimpleValue(node, value, components_);
        }
    }

    if (!bGotValue)
    {
        child = getChild(name);
        if (!child.is())
            return false;
        value = child->asValue();
    }
    return true;
}

css::uno::Any Access::getByName(OUString const & aName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    css::uno::Any value;
    if (!getByNameFast(aName, value))
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    return value;
}

css::uno::Sequence< OUString > Access::getElementNames()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    std::vector<OUString> names;
    names.reserve(children.size());
    for (auto const& child : children)
    {
        names.push_back(child->getNameInternal());
    }
    return comphelper::containerToSequence(names);
}

sal_Bool Access::hasByName(OUString const & aName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return getChild(aName).is();
}

css::uno::Any Access::getByHierarchicalName(OUString const & aName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    rtl::Reference< ChildAccess > child(getSubChild(aName));
    if (!child.is()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

sal_Bool Access::hasByHierarchicalName(OUString const & aName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return getSubChild(aName).is();
}

void Access::replaceByHierarchicalName(
    OUString const & aName, css::uno::Any const & aElement)
{
    //TODO: Actually support sets and combine with replaceByName:
    assert(thisIs(IS_UPDATE));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        rtl::Reference< ChildAccess > child(getSubChild(aName));
        if (!child.is()) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        child->checkFinalized();
        rtl::Reference< Node > parent(child->getParentNode());
        assert(parent.is());
        Modifications localMods;
        switch (parent->kind()) {
        case Node::KIND_LOCALIZED_PROPERTY:
        case Node::KIND_GROUP:
            child->setProperty(aElement, &localMods);
            break;
        case Node::KIND_SET:
            throw css::lang::IllegalArgumentException(
                ("configmgr::Access::replaceByHierarchicalName does not"
                 " currently support set members"),
                static_cast< cppu::OWeakObject * >(this), 0);
        case Node::KIND_ROOT:
            throw css::lang::IllegalArgumentException(
                ("configmgr::Access::replaceByHierarchicalName does not allow"
                 " changing component " + aName),
                static_cast< cppu::OWeakObject * >(this), 0);
        default:
            assert(false); // this cannot happen
            break;
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

void Access::addContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
{
    assert(thisIs(IS_ANY));
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        if (!xListener.is()) {
            throw css::uno::RuntimeException(
                "null listener", static_cast< cppu::OWeakObject * >(this));
        }
        if (!disposed_) {
            containerListeners_.insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing(
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    } catch (css::lang::DisposedException &) {}
}

void Access::removeContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    ContainerListeners::iterator i(containerListeners_.find(xListener));
    if (i != containerListeners_.end()) {
        containerListeners_.erase(i);
    }
}

OUString Access::getExactName(OUString const & aApproximateName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return aApproximateName;
}

css::uno::Sequence< css::beans::Property > Access::getProperties()
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    std::vector< css::beans::Property > properties;
    properties.reserve(children.size());
    for (auto const& child : children)
    {
        properties.push_back(child->asProperty());
    }
    return comphelper::containerToSequence(properties);
}

css::beans::Property Access::getPropertyByName(OUString const & aName)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    rtl::Reference< ChildAccess > child(getChild(aName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asProperty();
}

sal_Bool Access::hasPropertyByName(OUString const & Name)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    return getChild(Name).is();
}

OUString Access::getHierarchicalName() {
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    // For backwards compatibility, return an absolute path representation where
    // available:
    OUString rootPath;
    rtl::Reference< RootAccess > root(getRootAccess());
    if (root.is()) {
        rootPath = root->getAbsolutePathRepresentation();
    }
    OUString rel(getRelativePathRepresentation());
    OUStringBuffer path(rootPath);
    if (!rootPath.isEmpty() && rootPath != "/" && !rel.isEmpty()) {
        path.append('/');
    }
    path.append(rel);
    return path.makeStringAndClear();
}

OUString Access::composeHierarchicalName(
    OUString const & aRelativeName)
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    if (aRelativeName.isEmpty() || aRelativeName[0] == '/') {
        throw css::lang::IllegalArgumentException(
            "configmgr composeHierarchicalName inappropriate relative name",
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    OUStringBuffer path(getRelativePathRepresentation());
    if (!path.isEmpty()) {
        path.append('/');
    }
    path.append(aRelativeName);
    return path.makeStringAndClear();
}

OUString Access::getName() {
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return getNameInternal();
}

void Access::setName(OUString const & aName)
{
    assert(thisIs(IS_ANY));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        checkFinalized();
        Modifications localMods;
        switch (getNode()->kind()) {
        case Node::KIND_GROUP:
        case Node::KIND_SET:
            {
                rtl::Reference< Access > parent(getParentAccess());
                if (parent.is()) {
                    rtl::Reference< Node > node(getNode());
                    if (! node->getTemplateName().isEmpty()) {
                        rtl::Reference< ChildAccess > other(
                            parent->getChild(aName));
                        if (other.get() == this) {
                            break;
                        }
                        if (node->getMandatory() == Data::NO_LAYER &&
                            !(other.is() && other->isFinalized()))
                        {
                            if (!isValidName(aName, true)) {
                                throw css::uno::RuntimeException(
                                    "invalid element name " + aName);
                            }
                            rtl::Reference< RootAccess > root(getRootAccess());
                            rtl::Reference< ChildAccess > childAccess(
                                static_cast< ChildAccess * >(this));
                            localMods.add(getRelativePath());
                            // unbind() modifies the parent chain that
                            // markChildAsModified() walks, so order is
                            // important:
                            parent->markChildAsModified(childAccess);
                                //TODO: must not throw
                            childAccess->unbind(); // must not throw
                            if (other.is()) {
                                other->unbind(); // must not throw
                            }
                            childAccess->bind(root, parent, aName);
                                // must not throw
                            parent->markChildAsModified(childAccess);
                                //TODO: must not throw
                            localMods.add(getRelativePath());
                            break;
                        }
                    }
                }
            }
            [[fallthrough]];
        case Node::KIND_LOCALIZED_PROPERTY:
            // renaming a property could only work for an extension property,
            // but a localized property is never an extension property
            throw css::uno::RuntimeException(
                "configmgr setName inappropriate node",
                static_cast< cppu::OWeakObject * >(this));
        default:
            assert(false); // this cannot happen
            break;
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::beans::Property Access::getAsProperty()
{
    assert(thisIs(IS_ANY));
    osl::MutexGuard g(*lock_);
    checkLocalizedPropertyAccess();
    return asProperty();
}

css::uno::Reference< css::beans::XPropertySetInfo > Access::getPropertySetInfo()
{
    assert(thisIs(IS_GROUP));
    return this;
}

void Access::setPropertyValue(
    OUString const & aPropertyName, css::uno::Any const & aValue)
{
    assert(thisIs(IS_GROUP));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        if (!getRootAccess()->isUpdate()) {
            throw css::uno::RuntimeException(
                "configmgr setPropertyValue on non-update access",
                static_cast< cppu::OWeakObject * >(this));
        }
        Modifications localMods;
        if (!setChildProperty(aPropertyName, aValue, &localMods)) {
            throw css::beans::UnknownPropertyException(
                aPropertyName, static_cast< cppu::OWeakObject * >(this));
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::uno::Any Access::getPropertyValue(OUString const & PropertyName)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);

    css::uno::Any value;
    if (!getByNameFast(PropertyName, value))
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    return value;
}

void Access::addPropertyChangeListener(
    OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        xListener)
{
    assert(thisIs(IS_GROUP));
    {
        osl::MutexGuard g(*lock_);
        if (!xListener.is()) {
            throw css::uno::RuntimeException(
                "null listener", static_cast< cppu::OWeakObject * >(this));
        }
        checkKnownProperty(aPropertyName);
        if (!disposed_) {
            propertyChangeListeners_[aPropertyName].insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing(
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    } catch (css::lang::DisposedException &) {}
}

void Access::removePropertyChangeListener(
    OUString const & aPropertyName,
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        aListener)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    checkKnownProperty(aPropertyName);
    PropertyChangeListeners::iterator i(
        propertyChangeListeners_.find(aPropertyName));
    if (i != propertyChangeListeners_.end()) {
        PropertyChangeListenersElement::iterator j(i->second.find(aListener));
        if (j != i->second.end()) {
            i->second.erase(j);
            if (i->second.empty()) {
                propertyChangeListeners_.erase(i);
            }
        }
    }
}

void Access::addVetoableChangeListener(
    OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
{
    assert(thisIs(IS_GROUP));
    {
        osl::MutexGuard g(*lock_);
        if (!aListener.is()) {
            throw css::uno::RuntimeException(
                "null listener", static_cast< cppu::OWeakObject * >(this));
        }
        checkKnownProperty(PropertyName);
        if (!disposed_) {
            vetoableChangeListeners_[PropertyName].insert(aListener);
            //TODO: actually call vetoableChangeListeners_
            return;
        }
    }
    try {
        aListener->disposing(
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    } catch (css::lang::DisposedException &) {}
}

void Access::removeVetoableChangeListener(
    OUString const & PropertyName,
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    checkKnownProperty(PropertyName);
    VetoableChangeListeners::iterator i(
        vetoableChangeListeners_.find(PropertyName));
    if (i != vetoableChangeListeners_.end()) {
        VetoableChangeListenersElement::iterator j(i->second.find(aListener));
        if (j != i->second.end()) {
            i->second.erase(j);
            if (i->second.empty()) {
                vetoableChangeListeners_.erase(i);
            }
        }
    }
}

void Access::setPropertyValues(
    css::uno::Sequence< OUString > const & aPropertyNames,
    css::uno::Sequence< css::uno::Any > const & aValues)
{
    assert(thisIs(IS_GROUP));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        if (!getRootAccess()->isUpdate()) {
            throw css::uno::RuntimeException(
                "configmgr setPropertyValues on non-update access",
                static_cast< cppu::OWeakObject * >(this));
        }
        if (aPropertyNames.getLength() != aValues.getLength()) {
            throw css::lang::IllegalArgumentException(
                ("configmgr setPropertyValues: aPropertyNames/aValues of"
                 " different length"),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        Modifications localMods;
        for (sal_Int32 i = 0; i < aPropertyNames.getLength(); ++i) {
            if (!setChildProperty(aPropertyNames[i], aValues[i], &localMods)) {
                throw css::lang::IllegalArgumentException(
                    "configmgr setPropertyValues inappropriate property name",
                    static_cast< cppu::OWeakObject * >(this), -1);
            }
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::uno::Sequence< css::uno::Any > Access::getPropertyValues(
    css::uno::Sequence< OUString > const & aPropertyNames)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    css::uno::Sequence< css::uno::Any > vals(aPropertyNames.getLength());

    for (sal_Int32 i = 0; i < aPropertyNames.getLength(); ++i)
    {
        if (!getByNameFast(aPropertyNames[i], vals[i]))
            throw css::uno::RuntimeException(
                "configmgr getPropertyValues inappropriate property name",
                static_cast< cppu::OWeakObject * >(this));
    }

    return vals;
}

void Access::addPropertiesChangeListener(
    css::uno::Sequence< OUString > const &,
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
{
    assert(thisIs(IS_GROUP));
    {
        osl::MutexGuard g(*lock_);
        if (!xListener.is()) {
            throw css::uno::RuntimeException(
                "null listener", static_cast< cppu::OWeakObject * >(this));
        }
        if (!disposed_) {
            propertiesChangeListeners_.insert(xListener);
            return;
        }
    }
    try {
        xListener->disposing(
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    } catch (css::lang::DisposedException &) {}
}

void Access::removePropertiesChangeListener(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    PropertiesChangeListeners::iterator i(
        propertiesChangeListeners_.find(xListener));
    if (i != propertiesChangeListeners_.end()) {
        propertiesChangeListeners_.erase(i);
    }
}

void Access::firePropertiesChangeEvent(
    css::uno::Sequence< OUString > const & aPropertyNames,
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
{
    assert(thisIs(IS_GROUP));
    css::uno::Sequence< css::beans::PropertyChangeEvent > events(
        aPropertyNames.getLength());
    for (sal_Int32 i = 0; i < events.getLength(); ++i) {
        events[i].Source = static_cast< cppu::OWeakObject * >(this);
        events[i].PropertyName = aPropertyNames[i];
        events[i].Further = false;
        events[i].PropertyHandle = -1;
    }
    xListener->propertiesChange(events);
}

css::uno::Reference< css::beans::XHierarchicalPropertySetInfo >
Access::getHierarchicalPropertySetInfo() {
    assert(thisIs(IS_GROUP));
    return this;
}

void Access::setHierarchicalPropertyValue(
    OUString const & aHierarchicalPropertyName,
    css::uno::Any const & aValue)
{
    assert(thisIs(IS_GROUP));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        if (!getRootAccess()->isUpdate()) {
            throw css::uno::RuntimeException(
                "configmgr setHierarchicalPropertyName on non-update access",
                static_cast< cppu::OWeakObject * >(this));
        }
        rtl::Reference< ChildAccess > child(
            getSubChild(aHierarchicalPropertyName));
        if (!child.is()) {
            throw css::beans::UnknownPropertyException(
                aHierarchicalPropertyName,
                static_cast< cppu::OWeakObject * >(this));
        }
        child->checkFinalized();
        Modifications localMods;
        child->setProperty(aValue, &localMods);
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::uno::Any Access::getHierarchicalPropertyValue(
    OUString const & aHierarchicalPropertyName)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    rtl::Reference< ChildAccess > child(getSubChild(aHierarchicalPropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aHierarchicalPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

void Access::setHierarchicalPropertyValues(
    css::uno::Sequence< OUString > const & aHierarchicalPropertyNames,
    css::uno::Sequence< css::uno::Any > const & Values)
{
    assert(thisIs(IS_GROUP));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        if (!getRootAccess()->isUpdate()) {
            throw css::uno::RuntimeException(
                "configmgr setPropertyValues on non-update access",
                static_cast< cppu::OWeakObject * >(this));
        }
        if (aHierarchicalPropertyNames.getLength() != Values.getLength()) {
            throw css::lang::IllegalArgumentException(
                ("configmgr setHierarchicalPropertyValues:"
                 " aHierarchicalPropertyNames/Values of different length"),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        Modifications localMods;
        for (sal_Int32 i = 0; i < aHierarchicalPropertyNames.getLength(); ++i) {
            rtl::Reference< ChildAccess > child(
                getSubChild(aHierarchicalPropertyNames[i]));
            if (!child.is()) {
                throw css::lang::IllegalArgumentException(
                    ("configmgr setHierarchicalPropertyValues inappropriate"
                     " property name"),
                    static_cast< cppu::OWeakObject * >(this), -1);
            }
            child->checkFinalized();
            child->setProperty(Values[i], &localMods);
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::uno::Sequence< css::uno::Any > Access::getHierarchicalPropertyValues(
    css::uno::Sequence< OUString > const & aHierarchicalPropertyNames)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    css::uno::Sequence< css::uno::Any > vals(
        aHierarchicalPropertyNames.getLength());
    for (sal_Int32 i = 0; i < aHierarchicalPropertyNames.getLength(); ++i) {
        rtl::Reference< ChildAccess > child(
            getSubChild(aHierarchicalPropertyNames[i]));
        if (!child.is()) {
            throw css::lang::IllegalArgumentException(
                ("configmgr getHierarchicalPropertyValues inappropriate"
                 " hierarchical property name"),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        vals[i] = child->asValue();
    }
    return vals;
}

css::beans::Property Access::getPropertyByHierarchicalName(
    OUString const & aHierarchicalName)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    rtl::Reference< ChildAccess > child(getSubChild(aHierarchicalName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aHierarchicalName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asProperty();
}

sal_Bool Access::hasPropertyByHierarchicalName(
    OUString const & aHierarchicalName)
{
    assert(thisIs(IS_GROUP));
    osl::MutexGuard g(*lock_);
    return getSubChild(aHierarchicalName).is();
}

void Access::replaceByName(
    OUString const & aName, css::uno::Any const & aElement)
{
    assert(thisIs(IS_UPDATE));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        rtl::Reference< ChildAccess > child(getChild(aName));
        if (!child.is()) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        child->checkFinalized();
        Modifications localMods;
        switch (getNode()->kind()) {
        case Node::KIND_LOCALIZED_PROPERTY:
        case Node::KIND_GROUP:
            child->setProperty(aElement, &localMods);
            break;
        case Node::KIND_SET:
            {
                rtl::Reference< ChildAccess > freeAcc(
                    getFreeSetMember(aElement));
                rtl::Reference< RootAccess > root(getRootAccess());
                localMods.add(child->getRelativePath());
                child->unbind(); // must not throw
                freeAcc->bind(root, this, aName); // must not throw
                markChildAsModified(freeAcc); //TODO: must not throw
            }
            break;
        default:
            assert(false); // this cannot happen
            break;
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

void Access::insertByName(
    OUString const & aName, css::uno::Any const & aElement)
{
    assert(thisIs(IS_EXTENSIBLE|IS_UPDATE));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        checkFinalized();
        if (getChild(aName).is()) {
            throw css::container::ElementExistException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        Modifications localMods;
        switch (getNode()->kind()) {
        case Node::KIND_LOCALIZED_PROPERTY:
            if (!isValidName(aName, false)) {
                throw css::lang::IllegalArgumentException(
                    aName, static_cast<cppu::OWeakObject *>(this), 0);
            }
            insertLocalizedValueChild(aName, aElement, &localMods);
            break;
        case Node::KIND_GROUP:
            {
                if (!isValidName(aName, false)) {
                    throw css::lang::IllegalArgumentException(
                        aName, static_cast<cppu::OWeakObject *>(this), 0);
                }
                checkValue(aElement, TYPE_ANY, true);
                rtl::Reference child(
                    new ChildAccess(
                        components_, getRootAccess(), this, aName,
                        new PropertyNode(
                            Data::NO_LAYER, TYPE_ANY, true, aElement, true)));
                markChildAsModified(child);
                localMods.add(child->getRelativePath());
            }
            break;
        case Node::KIND_SET:
            {
                if (!isValidName(aName, true)) {
                    throw css::lang::IllegalArgumentException(
                        aName, static_cast<cppu::OWeakObject *>(this), 0);
                }
                rtl::Reference< ChildAccess > freeAcc(
                    getFreeSetMember(aElement));
                freeAcc->bind(getRootAccess(), this, aName); // must not throw
                markChildAsModified(freeAcc); //TODO: must not throw
                localMods.add(freeAcc->getRelativePath());
            }
            break;
        default:
            assert(false); // this cannot happen
            break;
        }
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

void Access::removeByName(OUString const & aName)
{
    assert(thisIs(IS_EXTENSIBLE|IS_UPDATE));
    Broadcaster bc;
    {
        osl::MutexGuard g(*lock_);
        checkLocalizedPropertyAccess();
        rtl::Reference< ChildAccess > child(getChild(aName));
        if (!child.is() || child->isFinalized() ||
            child->getNode()->getMandatory() != Data::NO_LAYER)
        {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        if (getNode()->kind() == Node::KIND_GROUP) {
            rtl::Reference< Node > p(child->getNode());
            if (p->kind() != Node::KIND_PROPERTY ||
                !static_cast< PropertyNode * >(p.get())->isExtension())
            {
                throw css::container::NoSuchElementException(
                    aName, static_cast< cppu::OWeakObject * >(this));
            }
        }
        Modifications localMods;
        localMods.add(child->getRelativePath());
        // unbind() modifies the parent chain that markChildAsModified() walks,
        // so order is important:
        markChildAsModified(child); //TODO: must not throw
        child->unbind();
        getNotificationRoot()->initBroadcaster(localMods.getRoot(), &bc);
    }
    bc.send();
}

css::uno::Reference< css::uno::XInterface > Access::createInstance()
{
    assert(thisIs(IS_SET|IS_UPDATE));
    OUString tmplName(
        static_cast< SetNode * >(getNode().get())->getDefaultTemplateName());
    rtl::Reference< Node > tmpl(
        components_.getTemplate(tmplName));
    if (!tmpl.is()) {
        throw css::uno::Exception(
            "unknown template " + tmplName,
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< Node > node(tmpl->clone(true));
    node->setLayer(Data::NO_LAYER);
    return static_cast< cppu::OWeakObject * >(
        new ChildAccess(components_, getRootAccess(), node));
}

css::uno::Reference< css::uno::XInterface > Access::createInstanceWithArguments(
    css::uno::Sequence< css::uno::Any > const & aArguments)
{
    assert(thisIs(IS_SET|IS_UPDATE));
    if (aArguments.getLength() != 0) {
        throw css::uno::Exception(
            ("configuration SimpleSetUpdate createInstanceWithArguments"
             " must not specify any arguments"),
            static_cast< cppu::OWeakObject * >(this));
    }
    return createInstance();
}

Access::Access(Components & components):
    components_(components), disposed_(false), lock_( lock() )
{
}

Access::~Access() {}

void Access::initDisposeBroadcaster(Broadcaster * broadcaster) {
    assert(broadcaster != nullptr);
    for (auto const& disposeListener : disposeListeners_)
    {
        broadcaster->addDisposeNotification(
            disposeListener,
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    }
    for (auto const& containerListener : containerListeners_)
    {
        broadcaster->addDisposeNotification(
            containerListener.get(),
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    }
    for (auto const& propertyChangeListener : propertyChangeListeners_)
    {
        for (auto const& propertyChangeListenerElement : propertyChangeListener.second)
        {
            broadcaster->addDisposeNotification(
                propertyChangeListenerElement.get(),
                css::lang::EventObject(
                    static_cast< cppu::OWeakObject * >(this)));
        }
    }
    for (auto const& vetoableChangeListener : vetoableChangeListeners_)
    {
        for (auto const& vetoableChangeListenerElement : vetoableChangeListener.second)
        {
            broadcaster->addDisposeNotification(
                vetoableChangeListenerElement.get(),
                css::lang::EventObject(
                    static_cast< cppu::OWeakObject * >(this)));
        }
    }
    for (auto const& propertiesChangeListener : propertiesChangeListeners_)
    {
        broadcaster->addDisposeNotification(
            propertiesChangeListener.get(),
            css::lang::EventObject(static_cast< cppu::OWeakObject * >(this)));
    }
    //TODO: iterate over children w/ listeners (incl. unmodified ones):
    for (ModifiedChildren::iterator i(modifiedChildren_.begin());
         i != modifiedChildren_.end(); ++i)
    {
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            child->initDisposeBroadcaster(broadcaster);
        }
    }
}

void Access::clearListeners() throw() {
    disposeListeners_.clear();
    containerListeners_.clear();
    propertyChangeListeners_.clear();
    vetoableChangeListeners_.clear();
    propertiesChangeListeners_.clear();
    //TODO: iterate over children w/ listeners (incl. unmodified ones):
    for (ModifiedChildren::iterator i(modifiedChildren_.begin());
         i != modifiedChildren_.end(); ++i)
    {
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            child->clearListeners();
        }
    }
}

css::uno::Any Access::queryInterface(css::uno::Type const & aType)
{
    css::uno::Any res(OWeakObject::queryInterface(aType));
    if (res.hasValue()) {
        return res;
    }
    res = cppu::queryInterface(
        aType, static_cast< css::lang::XTypeProvider * >(this),
        static_cast< css::lang::XServiceInfo * >(this),
        static_cast< css::lang::XComponent * >(this),
        static_cast< css::container::XHierarchicalNameAccess * >(this),
        static_cast< css::container::XContainer * >(this),
        static_cast< css::beans::XExactName * >(this),
        static_cast< css::container::XHierarchicalName * >(this),
        static_cast< css::container::XNamed * >(this),
        static_cast< css::beans::XProperty * >(this),
        static_cast< css::container::XElementAccess * >(this),
        static_cast< css::container::XNameAccess * >(this));
    if (res.hasValue()) {
        return res;
    }
    if (getNode()->kind() == Node::KIND_GROUP) {
        res = cppu::queryInterface(
            aType, static_cast< css::beans::XPropertySetInfo * >(this),
            static_cast< css::beans::XPropertySet * >(this),
            static_cast< css::beans::XMultiPropertySet * >(this),
            static_cast< css::beans::XHierarchicalPropertySet * >(this),
            static_cast< css::beans::XMultiHierarchicalPropertySet * >(this),
            static_cast< css::beans::XHierarchicalPropertySetInfo * >(this));
        if (res.hasValue()) {
            return res;
        }
    }
    if (getRootAccess()->isUpdate()) {
        res = cppu::queryInterface(
            aType, static_cast< css::container::XNameReplace * >(this),
            static_cast< css::container::XHierarchicalNameReplace * >(this));
        if (res.hasValue()) {
            return res;
        }
        if (getNode()->kind() != Node::KIND_GROUP ||
            static_cast< GroupNode * >(getNode().get())->isExtensible())
        {
            res = cppu::queryInterface(
                aType, static_cast< css::container::XNameContainer * >(this));
            if (res.hasValue()) {
                return res;
            }
        }
        if (getNode()->kind() == Node::KIND_SET) {
            res = cppu::queryInterface(
                aType, static_cast< css::lang::XSingleServiceFactory * >(this));
        }
    }
    return res;
}


void Access::checkLocalizedPropertyAccess() {
    if (getNode()->kind() == Node::KIND_LOCALIZED_PROPERTY &&
        !Components::allLocales(getRootAccess()->getLocale()))
    {
        throw css::uno::RuntimeException(
            "configmgr Access to specialized LocalizedPropertyNode",
            static_cast< cppu::OWeakObject * >(this));
    }
}

rtl::Reference< Node > Access::getParentNode() {
    rtl::Reference< Access > parent(getParentAccess());
    return parent.is() ? parent->getNode() : rtl::Reference< Node >();
}

rtl::Reference< ChildAccess > Access::getChild(OUString const & name) {
    OUString locale;
    if (getNode()->kind() == Node::KIND_LOCALIZED_PROPERTY
        && name.startsWith("*", &locale))
    {
        if (locale.startsWith("*")) {
            SAL_WARN(
                "configmgr",
                ("access best-matching localized property value via"
                 " \"*<locale>\" with <locale> \"")
                    << locale << "\" recursively starting with \"*\"");
            return getChild(locale);
        }
        SAL_WARN_IF(
            locale.isEmpty(), "configmgr",
            ("access best-matching localized property value via \"*<locale>\""
             " with empty <locale>; falling back to defaults"));
        if (!locale.isEmpty()) {
            // Find best match using an adaptation of RFC 4647 lookup matching
            // rules, removing "-" or "_" delimited segments from the end:
            for (;;) {
                rtl::Reference< ChildAccess > child(getChild(locale));
                if (child.is()) {
                    return child;
                }
                sal_Int32 i = locale.getLength() - 1;
                while (i > 0 && locale[i] != '-' && locale[i] != '_') {
                    --i;
                }
                if (i <= 0) {
                    break;
                }
                locale = locale.copy(0, i);
            }
            // As a workaround for broken xcu data that does not use shortest
            // xml:lang attributes, look for the first entry with the same first
            // segment as the requested language tag before falling back to
            // defaults (see fdo#33638):
            assert(
                !locale.isEmpty() && locale.indexOf('-') == -1 &&
                locale.indexOf('_') == -1);
            std::vector< rtl::Reference< ChildAccess > > children(
                getAllChildren());
            for (auto const& child : children)
            {
                OUString name2(child->getNameInternal());
                if (name2.startsWith(locale) &&
                    (name2.getLength() == locale.getLength() ||
                     name2[locale.getLength()] == '-' ||
                     name2[locale.getLength()] == '_'))
                {
                    return child;
                }
            }
        }
        // Defaults are the "en-US" locale, the "en" locale, the empty string locale, the first child (if
        // any, and if the property is non-nillable), or a null ChildAccess, in that order:
        rtl::Reference< ChildAccess > child(getChild("en-US"));
        if (child.is()) {
            return child;
        }
        child = getChild("en");
        if (child.is()) {
            return child;
        }
        child = getChild("");
        if (child.is()) {
            return child;
        }
        if (!static_cast<LocalizedPropertyNode *>(getNode().get())->isNillable()) {
            std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
            if (!children.empty()) {
                return children.front();
            }
        }
        return rtl::Reference< ChildAccess >();
    }
    ModifiedChildren::iterator i(modifiedChildren_.find(name));
    return i == modifiedChildren_.end()
        ? getUnmodifiedChild(name) : getModifiedChild(i);
}

std::vector< rtl::Reference< ChildAccess > > Access::getAllChildren() {
    std::vector< rtl::Reference< ChildAccess > > vec;
    NodeMap const & members = getNode()->getMembers();
    for (auto const& member : members)
    {
        if (modifiedChildren_.find(member.first) == modifiedChildren_.end()) {
            vec.push_back(getUnmodifiedChild(member.first));
            assert(vec.back().is());
        }
    }
    for (ModifiedChildren::iterator i(modifiedChildren_.begin());
         i != modifiedChildren_.end(); ++i)
    {
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            vec.push_back(child);
        }
    }
    return vec;
}

void Access::checkValue(css::uno::Any const & value, Type type, bool nillable) {
    bool ok;
    switch (type) {
    case TYPE_ERROR:
        ok = false;
        break;
    case TYPE_ANY:
        switch (getDynamicType(value)) {
        case TYPE_ERROR:
            ok = false;
            break;
        case TYPE_NIL:
            ok = nillable;
            break;
        default:
            ok = true;
            break;
        case TYPE_ANY:
            for (;;) std::abort(); // cannot happen
        }
        break;
    default:
        ok = value.hasValue() ? value.isExtractableTo(mapType(type)) : nillable;
        break;
    case TYPE_NIL:
        for (;;) std::abort(); // cannot happen
    }
    if (!ok) {
        throw css::lang::IllegalArgumentException(
            "configmgr inappropriate property value",
            static_cast< cppu::OWeakObject * >(this), -1);
    }
}

void Access::insertLocalizedValueChild(
    OUString const & name, css::uno::Any const & value,
    Modifications * localModifications)
{
    assert(localModifications != nullptr);
    LocalizedPropertyNode * locprop = static_cast< LocalizedPropertyNode * >(
        getNode().get());
    checkValue(value, locprop->getStaticType(), locprop->isNillable());
    rtl::Reference child(
        new ChildAccess(
            components_, getRootAccess(), this, name,
            new LocalizedValueNode(Data::NO_LAYER, value)));
    markChildAsModified(child);
    localModifications->add(child->getRelativePath());
}

void Access::reportChildChanges(
    std::vector< css::util::ElementChange > * changes)
{
    assert(changes != nullptr);
    for (ModifiedChildren::iterator i(modifiedChildren_.begin());
         i != modifiedChildren_.end(); ++i)
    {
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            child->reportChildChanges(changes);
            changes->push_back(css::util::ElementChange());
                //TODO: changed value and/or inserted node
        } else {
            changes->push_back(css::util::ElementChange()); //TODO: removed node
        }
    }
}

void Access::commitChildChanges(
    bool valid, Modifications * globalModifications)
{
    assert(globalModifications != nullptr);
    while (!modifiedChildren_.empty()) {
        bool childValid = valid;
        ModifiedChildren::iterator i(modifiedChildren_.begin());
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            childValid = childValid && !child->isFinalized();
            child->commitChanges(childValid, globalModifications);
                //TODO: currently, this is called here for directly inserted
                // children as well as for children whose sub-children were
                // modified (and should never be called for directly removed
                // children); clarify what exactly should happen here for
                // directly inserted children
        }
        NodeMap & members = getNode()->getMembers();
        NodeMap::iterator j(members.find(i->first));
        if (child.is()) {
            // Inserted:
            if (j != members.end()) {
                childValid = childValid &&
                    j->second->getFinalized() == Data::NO_LAYER;
                if (childValid) {
                    child->getNode()->setMandatory(j->second->getMandatory());
                }
            }
            if (childValid) {
                members[i->first] = child->getNode();
            }
        } else {
            // Removed:
            childValid = childValid && j != members.end() &&
                j->second->getFinalized() == Data::NO_LAYER &&
                j->second->getMandatory() == Data::NO_LAYER;
            if (childValid) {
                members.erase(j);
            }
        }
        if (childValid && i->second.directlyModified) {
            std::vector<OUString> path(getAbsolutePath());
            path.push_back(i->first);
            components_.addModification(path);
            globalModifications->add(path);
        }
        i->second.child->committed();
        modifiedChildren_.erase(i);
    }
}

void Access::initBroadcasterAndChanges(
    Modifications::Node const & modifications, Broadcaster * broadcaster,
    std::vector< css::util::ElementChange > * allChanges)
{
    assert(broadcaster != nullptr);
    std::vector< css::beans::PropertyChangeEvent > propChanges;
    bool collectPropChanges = !propertiesChangeListeners_.empty();
    for (const auto & i : modifications.children)
    {
        rtl::Reference< ChildAccess > child(getChild(i.first));
        if (child.is()) {
            switch (child->getNode()->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                if (!i.second.children.empty()) {
                    if (Components::allLocales(getRootAccess()->getLocale())) {
                        child->initBroadcasterAndChanges(
                            i.second, broadcaster, allChanges);
                            //TODO: if allChanges==0, recurse only into children
                            // w/ listeners
                    } else {
                        //TODO: filter child mods that are irrelevant for
                        // locale:
                        for (auto const& containerListener : containerListeners_)
                        {
                            broadcaster->
                                addContainerElementReplacedNotification(
                                    containerListener,
                                    css::container::ContainerEvent(
                                        static_cast< cppu::OWeakObject * >(
                                            this),
                                        css::uno::Any(i.first),
                                        css::uno::Any(), css::uno::Any()));
                                //TODO: non-void Element, ReplacedElement
                        }
                        PropertyChangeListeners::iterator j(
                            propertyChangeListeners_.find(i.first));
                        if (j != propertyChangeListeners_.end()) {
                            for (auto const& propertyChangeListenerElement : j->second)
                            {
                                broadcaster->addPropertyChangeNotification(
                                    propertyChangeListenerElement,
                                    css::beans::PropertyChangeEvent(
                                        static_cast< cppu::OWeakObject * >(
                                            this),
                                        i.first, false, -1, css::uno::Any(),
                                        css::uno::Any()));
                            }
                        }
                        j = propertyChangeListeners_.find("");
                        if (j != propertyChangeListeners_.end()) {
                            for (auto const& propertyChangeListenerElement : j->second)
                            {
                                broadcaster->addPropertyChangeNotification(
                                    propertyChangeListenerElement,
                                    css::beans::PropertyChangeEvent(
                                        static_cast< cppu::OWeakObject * >(
                                            this),
                                        i.first, false, -1, css::uno::Any(),
                                        css::uno::Any()));
                            }
                        }
                        if (allChanges != nullptr) {
                            allChanges->push_back(
                                css::util::ElementChange(
                                    css::uno::Any(
                                        child->getRelativePathRepresentation()),
                                    css::uno::Any(), css::uno::Any()));
                                //TODO: non-void Element, ReplacedElement
                        }
                        if (collectPropChanges) {
                            propChanges.emplace_back(
                                    static_cast< cppu::OWeakObject * >(this),
                                    i.first, false, -1, css::uno::Any(),
                                    css::uno::Any());
                        }
                    }
                }
                // else: spurious Modifications::Node not representing a change
                break;
            case Node::KIND_LOCALIZED_VALUE:
                assert(Components::allLocales(getRootAccess()->getLocale()));
                for (auto const& containerListener : containerListeners_)
                {
                    broadcaster->addContainerElementReplacedNotification(
                        containerListener,
                        css::container::ContainerEvent(
                            static_cast< cppu::OWeakObject * >(this),
                            css::uno::Any(i.first), child->asValue(),
                            css::uno::Any()));
                        //TODO: distinguish add/modify; non-void ReplacedElement
                }
                if (allChanges != nullptr) {
                    allChanges->push_back(
                        css::util::ElementChange(
                            css::uno::Any(
                                child->getRelativePathRepresentation()),
                            child->asValue(), css::uno::Any()));
                        //TODO: non-void ReplacedElement
                }
                assert(!collectPropChanges);
                break;
            case Node::KIND_PROPERTY:
                {
                    for (auto const& containerListener : containerListeners_)
                    {
                        broadcaster->addContainerElementReplacedNotification(
                            containerListener,
                            css::container::ContainerEvent(
                                static_cast< cppu::OWeakObject * >(this),
                                css::uno::Any(i.first), child->asValue(),
                                css::uno::Any()));
                            //TODO: distinguish add/remove/modify; non-void
                            // ReplacedElement
                    }
                    PropertyChangeListeners::iterator j(
                        propertyChangeListeners_.find(i.first));
                    if (j != propertyChangeListeners_.end()) {
                        for (auto const& propertyChangeListenerElement : j->second)
                        {
                            broadcaster->addPropertyChangeNotification(
                                propertyChangeListenerElement,
                                css::beans::PropertyChangeEvent(
                                    static_cast< cppu::OWeakObject * >(this),
                                    i.first, false, -1, css::uno::Any(),
                                    css::uno::Any()));
                        }
                    }
                    j = propertyChangeListeners_.find("");
                    if (j != propertyChangeListeners_.end()) {
                        for (auto const& propertyChangeListenerElement : j->second)
                        {
                            broadcaster->addPropertyChangeNotification(
                                propertyChangeListenerElement,
                                css::beans::PropertyChangeEvent(
                                    static_cast< cppu::OWeakObject * >(this),
                                    i.first, false, -1, css::uno::Any(),
                                    css::uno::Any()));
                        }
                    }
                    if (allChanges != nullptr) {
                        allChanges->push_back(
                            css::util::ElementChange(
                                css::uno::Any(
                                    child->getRelativePathRepresentation()),
                                child->asValue(), css::uno::Any()));
                            //TODO: non-void ReplacedElement
                    }
                    if (collectPropChanges) {
                        propChanges.emplace_back(
                                static_cast< cppu::OWeakObject * >(this),
                                i.first, false, -1, css::uno::Any(),
                                css::uno::Any());
                    }
                }
                break;
            case Node::KIND_GROUP:
            case Node::KIND_SET:
                if (i.second.children.empty()) {
                    if (!child->getNode()->getTemplateName().isEmpty()) {
                        for (auto const& containerListener : containerListeners_)
                        {
                            broadcaster->
                                addContainerElementInsertedNotification(
                                    containerListener,
                                    css::container::ContainerEvent(
                                        static_cast< cppu::OWeakObject * >(
                                            this),
                                        css::uno::Any(i.first),
                                        child->asValue(), css::uno::Any()));
                        }
                        if (allChanges != nullptr) {
                            allChanges->push_back(
                                css::util::ElementChange(
                                    css::uno::Any(
                                        child->getRelativePathRepresentation()),
                                    css::uno::Any(), css::uno::Any()));
                                //TODO: non-void Element, ReplacedElement
                        }
                    }
                    // else: spurious Modifications::Node not representing a
                    // change
                } else {
                    child->initBroadcasterAndChanges(
                        i.second, broadcaster, allChanges);
                        //TODO: if allChanges==0, recurse only into children w/
                        // listeners
                }
                break;
            case Node::KIND_ROOT:
                assert(false); // this cannot happen
                break;
            }
        } else {
            switch (getNode()->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                // Removed localized property value:
                assert(Components::allLocales(getRootAccess()->getLocale()));
                for (auto const& containerListener : containerListeners_)
                {
                    broadcaster->addContainerElementRemovedNotification(
                        containerListener,
                        css::container::ContainerEvent(
                            static_cast< cppu::OWeakObject * >(this),
                            css::uno::Any(i.first), css::uno::Any(),
                            css::uno::Any()));
                        //TODO: non-void ReplacedElement
                }
                if (allChanges != nullptr) {
                    OUStringBuffer path(getRelativePathRepresentation());
                    if (!path.isEmpty()) {
                        path.append('/');
                    }
                    path.append(Data::createSegment("*", i.first));
                    allChanges->push_back(
                        css::util::ElementChange(
                            css::uno::Any(path.makeStringAndClear()),
                            css::uno::Any(), css::uno::Any()));
                        //TODO: non-void ReplacedElement
                }
                assert(!collectPropChanges);
                break;
            case Node::KIND_GROUP:
                {
                    // Removed (non-localized) extension property:
                    for (auto const& containerListener : containerListeners_)
                    {
                        broadcaster->addContainerElementRemovedNotification(
                            containerListener,
                            css::container::ContainerEvent(
                                static_cast< cppu::OWeakObject * >(this),
                                css::uno::Any(i.first), css::uno::Any(),
                                css::uno::Any()));
                            //TODO: non-void ReplacedElement
                    }
                    PropertyChangeListeners::iterator j(
                        propertyChangeListeners_.find(i.first));
                    if (j != propertyChangeListeners_.end()) {
                        for (auto const& propertyChangeListenerElement : j->second)
                        {
                            broadcaster->addPropertyChangeNotification(
                                propertyChangeListenerElement,
                                css::beans::PropertyChangeEvent(
                                    static_cast< cppu::OWeakObject * >(this),
                                    i.first, false, -1, css::uno::Any(),
                                    css::uno::Any()));
                        }
                    }
                    j = propertyChangeListeners_.find("");
                    if (j != propertyChangeListeners_.end()) {
                        for (auto const& propertyChangeListenerElement : j->second)
                        {
                            broadcaster->addPropertyChangeNotification(
                                propertyChangeListenerElement,
                                css::beans::PropertyChangeEvent(
                                    static_cast< cppu::OWeakObject * >(this),
                                    i.first, false, -1, css::uno::Any(),
                                    css::uno::Any()));
                        }
                    }
                    if (allChanges != nullptr) {
                        OUStringBuffer path(
                            getRelativePathRepresentation());
                        if (!path.isEmpty()) {
                            path.append('/');
                        }
                        path.append(i.first);
                        allChanges->push_back(
                            css::util::ElementChange(
                                css::uno::Any(path.makeStringAndClear()),
                                css::uno::Any(), css::uno::Any()));
                            //TODO: non-void ReplacedElement
                    }
                    if (collectPropChanges) {
                        propChanges.emplace_back(
                                static_cast< cppu::OWeakObject * >(this),
                                i.first, false, -1, css::uno::Any(),
                                css::uno::Any());
                    }
                }
                break;
            case Node::KIND_SET:
                // Removed set member:
                if (i.second.children.empty()) {
                    for (auto const& containerListener : containerListeners_)
                    {
                        broadcaster->addContainerElementRemovedNotification(
                            containerListener,
                            css::container::ContainerEvent(
                                static_cast< cppu::OWeakObject * >(this),
                                css::uno::Any(i.first),
                                css::uno::Any(), css::uno::Any()));
                            //TODO: non-void ReplacedElement
                    }
                    if (allChanges != nullptr) {
                        OUStringBuffer path(
                            getRelativePathRepresentation());
                        if (!path.isEmpty()) {
                            path.append('/');
                        }
                        path.append(Data::createSegment("*", i.first));
                        allChanges->push_back(
                            css::util::ElementChange(
                                css::uno::Any(path.makeStringAndClear()),
                                css::uno::Any(), css::uno::Any()));
                            //TODO: non-void ReplacedElement
                    }
                }
                // else: spurious Modifications::Node not representing a change
                break;
            default:
                assert(false); // this cannot happen
                break;
            }
        }
    }
    if (!propChanges.empty()) {
        css::uno::Sequence< css::beans::PropertyChangeEvent > seq(
            comphelper::containerToSequence(propChanges));
        for (auto const& propertyChangeListener : propertiesChangeListeners_)
        {
            broadcaster->addPropertiesChangeNotification(propertyChangeListener, seq);
        }
    }
}


Access::ModifiedChild::ModifiedChild():
    directlyModified(false)
{}

Access::ModifiedChild::ModifiedChild(
    rtl::Reference< ChildAccess > const & theChild, bool theDirectlyModified):
    child(theChild), directlyModified(theDirectlyModified)
{}

rtl::Reference< ChildAccess > Access::getModifiedChild(
    ModifiedChildren::iterator const & childIterator)
{
    return (childIterator->second.child->getParentAccess() == this &&
            (childIterator->second.child->getNameInternal() ==
             childIterator->first))
        ? childIterator->second.child : rtl::Reference< ChildAccess >();
}

rtl::Reference< ChildAccess > Access::createUnmodifiedChild(
                const OUString &name, const rtl::Reference< Node > &node)
{
    rtl::Reference child(
        new ChildAccess(components_, getRootAccess(), this, name, node));
    cachedChildren_[name] = child.get();
    return child;
}

rtl::Reference< ChildAccess > Access::getUnmodifiedChild(
    OUString const & name)
{
    assert(modifiedChildren_.find(name) == modifiedChildren_.end());
    rtl::Reference< Node > node(getNode()->getMember(name));
    if (!node.is()) {
        return rtl::Reference< ChildAccess >();
    }
    WeakChildMap::iterator i(cachedChildren_.find(name));
    if (i != cachedChildren_.end()) {
        rtl::Reference< ChildAccess > child;
        if (i->second->acquireCounting() > 1) {
            child.set(i->second); // must not throw
        }
        i->second->releaseNondeleting();
        if (child.is()) {
            child->setNode(node);
            return child;
        }
    }
    return createUnmodifiedChild(name,node);
}

rtl::Reference< ChildAccess > Access::getSubChild(OUString const & path) {
    sal_Int32 i = 0;
    // For backwards compatibility, allow absolute paths where meaningful:
    if( path.startsWith("/") ) {
        ++i;
        if (!getRootAccess().is()) {
            return rtl::Reference< ChildAccess >();
        }
        std::vector<OUString> abs(getAbsolutePath());
        for (auto const& elem : abs)
        {
            OUString name1;
            bool setElement1;
            OUString templateName1;
            i = Data::parseSegment(
                path, i, &name1, &setElement1, &templateName1);
            if (i == -1 || (i != path.getLength() && path[i] != '/')) {
                return rtl::Reference< ChildAccess >();
            }
            OUString name2;
            bool setElement2;
            OUString templateName2;
            Data::parseSegment(elem, 0, &name2, &setElement2, &templateName2);
            if (name1 != name2 || setElement1 != setElement2 ||
                (setElement1 &&
                 !Data::equalTemplateNames(templateName1, templateName2)))
            {
                return rtl::Reference< ChildAccess >();
            }
            if (i != path.getLength()) {
                ++i;
            }
        }
    }
    for (rtl::Reference< Access > parent(this);;) {
        OUString name;
        bool setElement;
        OUString templateName;
        i = Data::parseSegment(path, i, &name, &setElement, &templateName);
        if (i == -1 || (i != path.getLength() && path[i] != '/')) {
            return rtl::Reference< ChildAccess >();
        }
        rtl::Reference< ChildAccess > child(parent->getChild(name));
        if (!child.is()) {
            return rtl::Reference< ChildAccess >();
        }
        if (setElement) {
            rtl::Reference< Node > p(parent->getNode());
            switch (p->kind()) {
            case Node::KIND_LOCALIZED_PROPERTY:
                if (!Components::allLocales(getRootAccess()->getLocale()) ||
                    !templateName.isEmpty())
                {
                    return rtl::Reference< ChildAccess >();
                }
                break;
            case Node::KIND_SET:
                if (!templateName.isEmpty() &&
                    !static_cast< SetNode * >(p.get())->isValidTemplate(
                        templateName))
                {
                    return rtl::Reference< ChildAccess >();
                }
                break;
            default:
                return rtl::Reference< ChildAccess >();
            }
        }
        // For backwards compatibility, ignore a final slash after non-value
        // nodes:
        if (child->isValue()) {
            return i == path.getLength()
                ? child : rtl::Reference< ChildAccess >();
        } else if (i >= path.getLength() - 1) {
            return child;
        }
        ++i;
        parent = child.get();
    }
}

bool Access::setChildProperty(
    OUString const & name, css::uno::Any const & value,
    Modifications * localModifications)
{
    assert(localModifications != nullptr);
    rtl::Reference< ChildAccess > child(getChild(name));
    if (!child.is()) {
        return false;
    }
    child->checkFinalized();
    child->setProperty(value, localModifications);
    return true;
}

css::beans::Property Access::asProperty() {
    css::uno::Type type;
    bool nillable;
    bool removable;
    rtl::Reference< Node > p(getNode());
    switch (p->kind()) {
    case Node::KIND_PROPERTY:
        {
            PropertyNode * prop = static_cast< PropertyNode * >(p.get());
            type = mapType(prop->getStaticType());
            nillable = prop->isNillable();
            removable = prop->isExtension();
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            LocalizedPropertyNode * locprop =
                static_cast< LocalizedPropertyNode *>(p.get());
            if (Components::allLocales(getRootAccess()->getLocale())) {
                type = cppu::UnoType< css::uno::XInterface >::get();
                    //TODO: correct?
                removable = false;
            } else {
                type = mapType(locprop->getStaticType());
                removable = false; //TODO ???
            }
            nillable = locprop->isNillable();
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyNode * locprop =
                static_cast< LocalizedPropertyNode * >(getParentNode().get());
            type = mapType(locprop->getStaticType());
            nillable = locprop->isNillable();
            removable = false; //TODO ???
        }
        break;
    default:
        type = cppu::UnoType< css::uno::XInterface >::get(); //TODO: correct?
        nillable = false;
        rtl::Reference< Node > parent(getParentNode());
        removable = parent.is() && parent->kind() == Node::KIND_SET;
        break;
    }
    return css::beans::Property(
        getNameInternal(), -1, type,
        (css::beans::PropertyAttribute::BOUND | //TODO: correct for group/set?
         css::beans::PropertyAttribute::CONSTRAINED |
         (nillable ? css::beans::PropertyAttribute::MAYBEVOID : 0) |
         (getRootAccess()->isUpdate() && removable
          ? css::beans::PropertyAttribute::REMOVABLE : 0) |
         (!getRootAccess()->isUpdate() || p->getFinalized() != Data::NO_LAYER
          ? css::beans::PropertyAttribute::READONLY : 0))); //TODO: MAYBEDEFAULT
}

void Access::checkFinalized() {
    if (isFinalized()) {
        throw css::lang::IllegalArgumentException(
            "configmgr modification of finalized item",
            static_cast< cppu::OWeakObject * >(this), -1);
    }
}

void Access::checkKnownProperty(OUString const & descriptor) {
    if (descriptor.isEmpty()) {
        return;
    }
    rtl::Reference< ChildAccess > child(getChild(descriptor));
    if (child.is()) {
        switch (child->getNode()->kind()) {
        case Node::KIND_PROPERTY:
            return;
        case Node::KIND_LOCALIZED_PROPERTY:
            if (!Components::allLocales(getRootAccess()->getLocale())) {
                return;
            }
            break;
        case Node::KIND_LOCALIZED_VALUE:
            if (Components::allLocales(getRootAccess()->getLocale())) {
                return;
            }
            break;
        default:
            break;
        }
    }
    throw css::beans::UnknownPropertyException(
        descriptor, static_cast< cppu::OWeakObject * >(this));
}

rtl::Reference< ChildAccess > Access::getFreeSetMember(
    css::uno::Any const & value)
{
    rtl::Reference< ChildAccess > freeAcc;
    css::uno::Reference< css::lang::XUnoTunnel > tunnel;
    value >>= tunnel;
    if (tunnel.is()) {
        freeAcc.set(
            reinterpret_cast< ChildAccess * >(
                tunnel->getSomething(ChildAccess::getTunnelId())));
    }
    if (!freeAcc.is() || freeAcc->getParentAccess().is() ||
        (freeAcc->isInTransaction() &&
         freeAcc->getRootAccess() != getRootAccess()))
    {
        throw css::lang::IllegalArgumentException(
            "configmgr inappropriate set element",
            static_cast< cppu::OWeakObject * >(this), 1);
    }
    assert(dynamic_cast< SetNode * >(getNode().get()) != nullptr);
    if (!static_cast< SetNode * >(getNode().get())->isValidTemplate(
            freeAcc->getNode()->getTemplateName()))
    {
        throw css::lang::IllegalArgumentException(
            "configmgr inappropriate set element",
            static_cast< cppu::OWeakObject * >(this), 1);
    }
    return freeAcc;
}

rtl::Reference< Access > Access::getNotificationRoot() {
    for (rtl::Reference< Access > p(this);;) {
        rtl::Reference< Access > parent(p->getParentAccess());
        if (!parent.is()) {
            return p;
        }
        p = parent;
    }
}

#if !defined NDEBUG
bool Access::thisIs(int what) {
    osl::MutexGuard g(*lock_);
    rtl::Reference< Node > p(getNode());
    Node::Kind k(p->kind());
    return (k != Node::KIND_PROPERTY && k != Node::KIND_LOCALIZED_VALUE &&
        ((what & IS_GROUP) == 0 || k == Node::KIND_GROUP) &&
        ((what & IS_SET) == 0 || k == Node::KIND_SET) &&
        ((what & IS_EXTENSIBLE) == 0 || k != Node::KIND_GROUP ||
         static_cast< GroupNode * >(p.get())->isExtensible()) &&
        ((what & IS_GROUP_MEMBER) == 0 ||
         getParentNode()->kind() == Node::KIND_GROUP)) ||
        ((what & IS_SET_MEMBER) == 0 ||
         getParentNode()->kind() == Node::KIND_SET) ||
        ((what & IS_UPDATE) == 0 || getRootAccess()->isUpdate());
}
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
