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

#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertiesChangeListener.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/container/XContainerListener.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/ElementChange.hpp"
#include "comphelper/sequenceasvector.hxx"
#include "cppu/unotype.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
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
#include "node.hxx"
#include "nodemap.hxx"
#include "propertynode.hxx"
#include "rootaccess.hxx"
#include "setnode.hxx"
#include "type.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

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

void Access::releaseChild(rtl::OUString const & name) {
    cachedChildren_.erase(name);
}

Access::Access(): AccessBase(lock) {}

Access::~Access() {}

css::uno::Any Access::queryInterface(css::uno::Type const & aType)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    css::uno::Any res(AccessBase::queryInterface(aType));
    if (res.hasValue()) {
        if (aType.getTypeName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.beans.XPropertySetInfo")) ||
            aType.getTypeName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.beans.XPropertySet")) ||
            aType.getTypeName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.beans.XMultiPropertySet")) ||
            aType.getTypeName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.beans.XHierarchicalPropertySet")) ||
            aType.getTypeName().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "com.sun.beans.XMultiHierarchicalPropertySet")))
        {
            if (getNode()->kind() != Node::KIND_GROUP) {
                res.clear();
            }
        } else if (aType.getTypeName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(
                           "com.sun.star.container.XNameReplace")))
        {
            if (!getRootAccess()->isUpdate()) {
                res.clear();
            }
        } else if (aType.getTypeName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(
                           "com.sun.star.container.XNameContainer")))
        {
            if ((getNode()->kind() == Node::KIND_GROUP &&
                 !dynamic_cast< GroupNode * >(
                     getNode().get())->isExtensible()) ||
                !getRootAccess()->isUpdate())
            {
                res.clear();
            }
        } else if (aType.getTypeName().equalsAsciiL(
                       RTL_CONSTASCII_STRINGPARAM(
                           "com.sun.star.lang.XSingleServiceFactory")))
        {
            if (getNode()->kind() != Node::KIND_SET ||
                !getRootAccess()->isUpdate())
            {
                res.clear();
            }
        }
    }
    return res;
}

void Access::checkLocalizedPropertyAccess() {
    if (getNode()->kind() == Node::KIND_LOCALIZED_PROPERTY &&
        !Components::allLocales(getRootAccess()->getLocale()))
    {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr Access to specialized LocalizedPropertyNode")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

rtl::Reference< Node > Access::getParentNode() {
    rtl::Reference< Access > parent(getParentAccess());
    return parent.is() ? parent->getNode() : rtl::Reference< Node >();
}

rtl::Reference< ChildAccess > Access::getChild(rtl::OUString const & name) {
    HardChildMap::iterator i(modifiedChildren_.find(name));
    return i == modifiedChildren_.end()
        ? getUnmodifiedChild(name) : getModifiedChild(i);
}

std::vector< rtl::Reference< ChildAccess > > Access::getAllChildren() {
    std::vector< rtl::Reference< ChildAccess > > vec;
    NodeMap & members = getNode()->getMembers();
    for (NodeMap::iterator i(members.begin()); i != members.end(); ++i) {
        if (!i->second->isRemoved() &&
            modifiedChildren_.find(i->first) == modifiedChildren_.end())
        {
            vec.push_back(getUnmodifiedChild(i->first));
            OSL_ASSERT(vec.back().is());
        }
    }
    for (HardChildMap::iterator i(modifiedChildren_.begin());
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
    case TYPE_NIL:
        OSL_ASSERT(false);
        // fall through (cannot happen)
    case TYPE_ERROR:
        ok = false;
        break;
    case TYPE_ANY:
        switch (mapType(value)) {
        case TYPE_ANY:
            OSL_ASSERT(false);
            // fall through (cannot happen)
        case TYPE_ERROR:
            ok = false;
            break;
        case TYPE_NIL:
            ok = nillable;
            break;
        default:
            ok = true;
            break;
        }
        break;
    default:
        ok = value.hasValue() ? value.isExtractableTo(mapType(type)) : nillable;
        break;
    }
    if (!ok) {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr inappropriate property value")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
}

void Access::insertLocalizedValueChild(
    rtl::OUString const & name, css::uno::Any const & value)
{
    LocalizedPropertyNode * locprop = dynamic_cast< LocalizedPropertyNode * >(
        getNode().get());
    checkValue(value, locprop->getType(), locprop->isNillable());
    rtl::Reference< ChildAccess > child(
        new ChildAccess(
            getRootAccess(), this, name,
            new LocalizedValueNode(Data::NO_LAYER, value)));
    child->markAsModified();
    //TODO notify change
}

void Access::reportChildChanges(
    std::vector< css::util::ElementChange > * changes)
{
    OSL_ASSERT(changes != 0);
    for (HardChildMap::iterator i(modifiedChildren_.begin());
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

void Access::commitChildChanges(bool valid) {
    while (!modifiedChildren_.empty()) {
        bool childValid = valid;
        HardChildMap::iterator i(modifiedChildren_.begin());
        rtl::Reference< ChildAccess > child(getModifiedChild(i));
        if (child.is()) {
            childValid = childValid && !child->isFinalized();
            child->commitChanges(childValid);
                //TODO: currently, this is called here for directly inserted
                // children as well as for children whose sub-children were
                // modified (and should never be called for directly removed
                // children); clarify what exactly should happen here for
                // directly inserted children
        }
        if (i->second->isModified()) {
            NodeMap & members = getNode()->getMembers();
            NodeMap::iterator j(members.find(i->first));
            if (child.is()) {
                // Inserted:
                if (j != members.end()) {
                    childValid = childValid &&
                        j->second->getFinalized() == Data::NO_LAYER;
                    if (childValid) {
                        child->getNode()->setMandatory(
                            j->second->getMandatory());
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
                    j->second->remove(Data::NO_LAYER);
                }
            }
            if (childValid) {
                Components::singleton().addModification(
                    getAbsolutePath() +
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                    Data::createSegment(
                        i->second->getNode()->getTemplateName(), i->first));
            }
            i->second->committed();
        }
        modifiedChildren_.erase(i);
    }
}

rtl::OUString Access::getImplementationName() throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    throw css::uno::RuntimeException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "configmgr Access has no service implementation name")),
        static_cast< cppu::OWeakObject * >(this));
}

sal_Bool Access::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    css::uno::Sequence< rtl::OUString > names(getSupportedServiceNames());
    for (sal_Int32 i = 0; i < names.getLength(); ++i) {
        if (names[i] == ServiceName) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< rtl::OUString > Access::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    comphelper::SequenceAsVector< rtl::OUString > services;
    services.push_back(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.ConfigurationAccess")));
    if (getRootAccess()->isUpdate()) {
        services.push_back(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationUpdateAccess")));
    }
    services.push_back(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.HierarchyAccess")));
    services.push_back(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.HierarchyElement")));
    if (getNode()->kind() == Node::KIND_GROUP) {
        services.push_back(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.GroupAccess")));
        services.push_back(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.PropertyHierarchy")));
        if (getRootAccess()->isUpdate()) {
            services.push_back(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.configuration.GroupUpdate")));
        }
    } else {
        services.push_back(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.SetAccess")));
        services.push_back(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.SimpleSetAccess")));
        if (getRootAccess()->isUpdate()) {
            services.push_back(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.configuration.SetUpdate")));
            services.push_back(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.configuration.SimpleSetUpdate")));
        }
    }
    addSupportedServiceNames(&services);
    return services.getAsConstList();
}

css::uno::Type Access::getElementType() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    rtl::Reference< Node > p(getNode());
    switch (p->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
        return mapType(
            dynamic_cast< LocalizedPropertyNode * >(p.get())->getType());
    case Node::KIND_GROUP:
        //TODO: Should a specific type be returned for a non-extensible group
        // with homogeneous members or for an extensible group that currently
        // has only homegeneous members?
        return cppu::UnoType< cppu::UnoVoidType >::get();
    case Node::KIND_SET:
        return cppu::UnoType< cppu::UnoVoidType >::get(); //TODO: correct?
    default:
        OSL_ASSERT(false);
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("this cannot happen")),
            static_cast< cppu::OWeakObject * >(this));
    }
}

sal_Bool Access::hasElements() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return !getAllChildren().empty(); //TODO: optimize
}

css::uno::Any Access::getByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    rtl::Reference< ChildAccess > child(getChild(aName));
    if (!child.is()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

css::uno::Sequence< rtl::OUString > Access::getElementNames()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    comphelper::SequenceAsVector< rtl::OUString > names;
    for (std::vector< rtl::Reference< ChildAccess > >::iterator i(
             children.begin());
         i != children.end(); ++i)
    {
        names.push_back((*i)->getNameInternal());
    }
    return names.getAsConstList();
}

sal_Bool Access::hasByName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return getChild(aName).is();
}

css::uno::Any Access::getByHierarchicalName(rtl::OUString const & aName)
    throw (css::container::NoSuchElementException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    rtl::Reference< ChildAccess > child(getSubChild(aName));
    if (!child.is()) {
        throw css::container::NoSuchElementException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

sal_Bool Access::hasByHierarchicalName(rtl::OUString const & aName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return getSubChild(aName).is();
}

void Access::addContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    rBHelper.addListener(
        cppu::UnoType< css::container::XContainerListener >::get(), xListener);
}

void Access::removeContainerListener(
    css::uno::Reference< css::container::XContainerListener > const & xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    rBHelper.removeListener(
        cppu::UnoType< css::container::XContainerListener >::get(), xListener);
}

rtl::OUString Access::getExactName(rtl::OUString const & aApproximateName)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return aApproximateName;
}

css::uno::Sequence< css::beans::Property > Access::getProperties()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    std::vector< rtl::Reference< ChildAccess > > children(getAllChildren());
    comphelper::SequenceAsVector< css::beans::Property > properties;
    for (std::vector< rtl::Reference< ChildAccess > >::iterator i(
             children.begin());
         i != children.end(); ++i)
    {
        properties.push_back((*i)->asProperty());
    }
    return properties.getAsConstList();
}

css::beans::Property Access::getPropertyByName(rtl::OUString const & aName)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getChild(aName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asProperty();
}

sal_Bool Access::hasPropertyByName(rtl::OUString const & Name)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    return getChild(Name).is();
}

rtl::OUString Access::getHierarchicalName() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return getRelativePath();
}

rtl::OUString Access::composeHierarchicalName(
    rtl::OUString const & aRelativeName)
    throw (
        css::lang::IllegalArgumentException, css::lang::NoSupportException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    if (aRelativeName.getLength() == 0 || aRelativeName[0] == '/') {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr composeHierarchicalName inappropriate relative"
                    " name")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    rtl::OUStringBuffer path(getRelativePath());
    if (path.getLength() != 0) {
        path.append(sal_Unicode('/'));
    }
    path.append(aRelativeName);
    return path.makeStringAndClear();
}

rtl::OUString Access::getName() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return getNameInternal();
}

void Access::setName(rtl::OUString const & /*aName*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::beans::Property Access::getAsProperty() throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    return asProperty();
}

css::uno::Reference< css::beans::XPropertySetInfo > Access::getPropertySetInfo()
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    return this;
}

void Access::setPropertyValue(
    rtl::OUString const & aPropertyName, css::uno::Any const & aValue)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setPropertyValue on non-update access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    if (!setChildProperty(aPropertyName, aValue)) {
        throw css::beans::UnknownPropertyException(
            aPropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

css::uno::Any Access::getPropertyValue(rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getChild(PropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

void Access::addPropertyChangeListener(
    rtl::OUString const & /*aPropertyName*/, //TODO
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        xListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XPropertyChangeListener >::get(), xListener);
}

void Access::removePropertyChangeListener(
    rtl::OUString const & /*aPropertyName*/, //TODO
    css::uno::Reference< css::beans::XPropertyChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XPropertyChangeListener >::get(), aListener);
}

void Access::addVetoableChangeListener(
    rtl::OUString const & /*PropertyName*/, //TODO
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XVetoableChangeListener >::get(), aListener);
}

void Access::removeVetoableChangeListener(
    rtl::OUString const & /*PropertyName*/, //TODO
    css::uno::Reference< css::beans::XVetoableChangeListener > const &
        aListener)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XVetoableChangeListener >::get(), aListener);
}

void Access::setPropertyValues(
    css::uno::Sequence< rtl::OUString > const & aPropertyNames,
    css::uno::Sequence< css::uno::Any > const & aValues)
    throw (
        css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setPropertyValues on non-update access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    if (aPropertyNames.getLength() != aValues.getLength()) {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setPropertyValues: aPropertyNames/aValues of"
                    " different length")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    for (sal_Int32 i = 0; i < aPropertyNames.getLength(); ++i) {
        if (!setChildProperty(aPropertyNames[i], aValues[i])) {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr setPropertyValues inappropriate property"
                        " name")),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
    }
}

css::uno::Sequence< css::uno::Any > Access::getPropertyValues(
    css::uno::Sequence< rtl::OUString > const & aPropertyNames)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    css::uno::Sequence< css::uno::Any > vals(aPropertyNames.getLength());
    for (sal_Int32 i = 0; i < aPropertyNames.getLength(); ++i) {
        rtl::Reference< ChildAccess > child(getChild(aPropertyNames[i]));
        if (!child.is()) {
            throw css::uno::RuntimeException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr getPropertyValues inappropriate property"
                        " name")),
                static_cast< cppu::OWeakObject * >(this));
        }
        vals[i] = child->asValue();
    }
    return vals;
}

void Access::addPropertiesChangeListener(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/, //TODO
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.addListener(
        cppu::UnoType< css::beans::XPropertiesChangeListener >::get(),
        xListener);
}

void Access::removePropertiesChangeListener(
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        xListener)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    rBHelper.removeListener(
        cppu::UnoType< css::beans::XPropertiesChangeListener >::get(),
        xListener);
}

void Access::firePropertiesChangeEvent(
    css::uno::Sequence< rtl::OUString > const & /*aPropertyNames*/,
    css::uno::Reference< css::beans::XPropertiesChangeListener > const &
        /*xListener*/)
    throw (css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

css::uno::Reference< css::beans::XHierarchicalPropertySetInfo >
Access::getHierarchicalPropertySetInfo() throw (css::uno::RuntimeException) {
    OSL_ASSERT(thisIs(IS_GROUP));
    if(true)abort();*(char*)0=0;throw 0;//TODO
}

void Access::setHierarchicalPropertyValue(
    rtl::OUString const & aHierarchicalPropertyName,
    css::uno::Any const & aValue)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setHierarchicalPropertyName on non-update"
                    " access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< ChildAccess > child(getSubChild(aHierarchicalPropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aHierarchicalPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }
    child->checkFinalized();
    child->setProperty(aValue);
}

css::uno::Any Access::getHierarchicalPropertyValue(
    rtl::OUString const & aHierarchicalPropertyName)
    throw (
        css::beans::UnknownPropertyException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    rtl::Reference< ChildAccess > child(getSubChild(aHierarchicalPropertyName));
    if (!child.is()) {
        throw css::beans::UnknownPropertyException(
            aHierarchicalPropertyName,
            static_cast< cppu::OWeakObject * >(this));
    }
    return child->asValue();
}

void Access::setHierarchicalPropertyValues(
    css::uno::Sequence< rtl::OUString > const & aHierarchicalPropertyNames,
    css::uno::Sequence< css::uno::Any > const & Values)
    throw (
        css::beans::PropertyVetoException, css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    if (!getRootAccess()->isUpdate()) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setPropertyValues on non-update access")),
            static_cast< cppu::OWeakObject * >(this));
    }
    if (aHierarchicalPropertyNames.getLength() != Values.getLength()) {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr setHierarchicalPropertyValues:"
                    " aHierarchicalPropertyNames/Values of different length")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
    for (sal_Int32 i = 0; i < aHierarchicalPropertyNames.getLength(); ++i) {
        rtl::Reference< ChildAccess > child(
            getSubChild(aHierarchicalPropertyNames[i]));
        if (!child.is()) {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr setHierarchicalPropertyValues inappropriate"
                        " property name")),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        child->checkFinalized();
        child->setProperty(Values[i]);
    }
}

css::uno::Sequence< css::uno::Any > Access::getHierarchicalPropertyValues(
    css::uno::Sequence< rtl::OUString > const & aHierarchicalPropertyNames)
    throw (
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_GROUP));
    osl::MutexGuard g(lock);
    css::uno::Sequence< css::uno::Any > vals(
        aHierarchicalPropertyNames.getLength());
    for (sal_Int32 i = 0; i < aHierarchicalPropertyNames.getLength(); ++i) {
        rtl::Reference< ChildAccess > child(
            getSubChild(aHierarchicalPropertyNames[i]));
        if (!child.is()) {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "configmgr getHierarchicalPropertyValues inappropriate"
                        " hierarchical property name")),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        vals[i] = child->asValue();
    }
    return vals;
}

void Access::replaceByName(
    rtl::OUString const & aName, css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_ANY|IS_UPDATE));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    switch (getNode()->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
    case Node::KIND_GROUP:
        if (!setChildProperty(aName, aElement)) {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
        break;
    case Node::KIND_SET:
        if(true)abort();*(char*)0=0;throw 0;//TODO
    default:
        OSL_ASSERT(false); // this cannot happen
        break;
    }
}

void Access::insertByName(
    rtl::OUString const & aName, css::uno::Any const & aElement)
    throw (
        css::lang::IllegalArgumentException,
        css::container::ElementExistException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_EXTENSIBLE|IS_UPDATE));
    osl::MutexGuard g(lock);
    checkLocalizedPropertyAccess();
    checkFinalized();
    if (getChild(aName).is()) {
        throw css::container::ElementExistException(
            aName, static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< Node > p(getNode());
    switch (p->kind()) {
    case Node::KIND_LOCALIZED_PROPERTY:
        insertLocalizedValueChild(aName, aElement);
        break;
    case Node::KIND_GROUP:
        checkValue(aElement, TYPE_ANY, true);
        rtl::Reference< ChildAccess >(
            new ChildAccess(
                getRootAccess(), this, aName,
                new PropertyNode(
                    Data::NO_LAYER, TYPE_ANY, true, aElement, true)))->
            markAsModified();
        //TODO notify change
        break;
    case Node::KIND_SET:
        {
            rtl::Reference< ChildAccess > freeAcc;
            css::uno::Reference< css::lang::XUnoTunnel > tunnel;
            aElement >>= tunnel;
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
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "configmgr insertByName inappropriate set"
                            " element")),
                    static_cast< cppu::OWeakObject * >(this), 1);
            }
            if (!dynamic_cast< SetNode * >(p.get())->isValidTemplate(
                    freeAcc->getNode()->getTemplateName()))
            {
                throw css::lang::IllegalArgumentException(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "configmgr insertByName inappropriate set"
                            " element")),
                    static_cast< cppu::OWeakObject * >(this), 1);
            }
            rtl::Reference< RootAccess > root(getRootAccess());
            freeAcc->bind(root, this, aName); // must not throw
            freeAcc->markAsModified(); //TODO: must not throw
            //TODO notify change
        }
        break;
    default:
        OSL_ASSERT(false); // this cannot happen
        break;
    }
}

void Access::removeByName(rtl::OUString const & aName)
    throw (
        css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_EXTENSIBLE|IS_UPDATE));
    osl::MutexGuard g(lock);
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
            !dynamic_cast< PropertyNode * >(p.get())->isExtension())
        {
            throw css::container::NoSuchElementException(
                aName, static_cast< cppu::OWeakObject * >(this));
        }
    }
    // unbind() modifies the parent chain that markAsModified() walks, so order
    // is important:
    child->markAsModified(); //TODO: must not throw
    child->unbind();
    //TODO notify change
}

css::uno::Reference< css::uno::XInterface > Access::createInstance()
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    rtl::OUString tmplName(
        dynamic_cast< SetNode * >(getNode().get())->getDefaultTemplateName());
    rtl::Reference< Node > tmpl(
        Components::singleton().getTemplate(Data::NO_LAYER, tmplName));
    if (!tmpl.is()) {
        throw css::uno::Exception(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("unknown template ")) +
             tmplName),
            static_cast< cppu::OWeakObject * >(this));
    }
    rtl::Reference< Node > node(tmpl->clone());
    node->setLayer(Data::NO_LAYER);
    return static_cast< cppu::OWeakObject * >(
        new ChildAccess(getRootAccess(), node));
}

css::uno::Reference< css::uno::XInterface > Access::createInstanceWithArguments(
    css::uno::Sequence< css::uno::Any > const & aArguments)
    throw (css::uno::Exception, css::uno::RuntimeException)
{
    OSL_ASSERT(thisIs(IS_SET|IS_UPDATE));
    if (aArguments.getLength() != 0) {
        throw css::uno::Exception(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configuration SimpleSetUpdate createInstanceWithArguments"
                    " must not specify any arguments")),
            static_cast< cppu::OWeakObject * >(this));
    }
    return createInstance();
}

rtl::Reference< ChildAccess > Access::getModifiedChild(
    HardChildMap::iterator const & childIterator)
{
    return (childIterator->second->getParentAccess() == this &&
            childIterator->second->getNameInternal() == childIterator->first)
        ? childIterator->second : rtl::Reference< ChildAccess >();
}

rtl::Reference< ChildAccess > Access::getUnmodifiedChild(
    rtl::OUString const & name)
{
    OSL_ASSERT(modifiedChildren_.find(name) == modifiedChildren_.end());
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
    rtl::Reference< ChildAccess > child(
        new ChildAccess(getRootAccess(), this, name, node));
    cachedChildren_[name] = child.get();
    return child;
}

rtl::Reference< ChildAccess > Access::getSubChild(rtl::OUString const & path) {
    rtl::OUString name;
    bool setElement;
    rtl::OUString templateName;
    sal_Int32 i = Data::parseSegment(
        path, 0, &name, &setElement, &templateName);
    if (i == -1 || (i != path.getLength() && path[i] != '/')) {
        return rtl::Reference< ChildAccess >();
    }
    rtl::Reference< ChildAccess > child(getChild(name));
    if (!child.is()) {
        return rtl::Reference< ChildAccess >();
    }
    if (setElement) {
        rtl::Reference< Node > p(getNode());
        switch (p->kind()) {
        case Node::KIND_LOCALIZED_PROPERTY:
            if (!Components::allLocales(getRootAccess()->getLocale()) ||
                templateName.getLength() != 0)
            {
                return rtl::Reference< ChildAccess >();
            }
            break;
        case Node::KIND_SET:
            if (templateName.getLength() != 0 &&
                !dynamic_cast< SetNode * >(p.get())->isValidTemplate(
                    templateName))
            {
                return rtl::Reference< ChildAccess >();
            }
            break;
        default:
            return rtl::Reference< ChildAccess >();
        }
    }
    // For backwards compatibility, ignore a final slash after non-value nodes:
    return child->isValue()
        ? (i == path.getLength() ? child : rtl::Reference< ChildAccess >())
        : (i >= path.getLength() - 1
           ? child : child->getSubChild(path.copy(i + 1)));
}

bool Access::setChildProperty(
    rtl::OUString const & name, com::sun::star::uno::Any const & value)
{
    rtl::Reference< ChildAccess > child(getChild(name));
    if (!child.is()) {
        return false;
    }
    child->checkFinalized();
    child->setProperty(value);
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
            PropertyNode * prop = dynamic_cast< PropertyNode * >(p.get());
            type = mapType(prop->getType());
            nillable = prop->isNillable();
            removable = prop->isExtension();
        }
        break;
    case Node::KIND_LOCALIZED_PROPERTY:
        {
            LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode *>(p.get());
            if (Components::allLocales(getRootAccess()->getLocale())) {
                type = cppu::UnoType< css::uno::XInterface >::get();
                    //TODO: correct?
                removable = false;
            } else {
                type = mapType(locprop->getType());
                removable = false; //TODO ???
            }
            nillable = locprop->isNillable();
        }
        break;
    case Node::KIND_LOCALIZED_VALUE:
        {
            LocalizedPropertyNode * locprop =
                dynamic_cast< LocalizedPropertyNode * >(getParentNode().get());
            type = mapType(locprop->getType());
            nillable = locprop->isNillable();
            removable = false; //TODO ???
        }
        break;
    default:
        type = cppu::UnoType< css::uno::XInterface >::get(); //TODO: correct?
        nillable = false;
        removable = getParentNode()->kind() == Node::KIND_SET;
        break;
    }
    return css::beans::Property(
        getNameInternal(), -1, type,
        (css::beans::PropertyAttribute::BOUND | //TODO: correct for group/set?
         css::beans::PropertyAttribute::CONSTRAINED |
         (nillable ? css::beans::PropertyAttribute::MAYBEVOID : 0) |
         (getRootAccess()->isUpdate()
          ? (removable ? css::beans::PropertyAttribute::REMOVEABLE : 0)
          : css::beans::PropertyAttribute::READONLY))); //TODO: MAYBEDEFAULT
}

void Access::checkFinalized() {
    if (isFinalized()) {
        throw css::lang::IllegalArgumentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "configmgr modification of finalized item")),
            static_cast< cppu::OWeakObject * >(this), -1);
    }
}

#if OSL_DEBUG_LEVEL > 0
bool Access::thisIs(int what) {
    osl::MutexGuard g(lock);
    rtl::Reference< Node > p(getNode());
    Node::Kind k(p->kind());
    return k != Node::KIND_PROPERTY && k != Node::KIND_LOCALIZED_VALUE &&
        ((what & IS_GROUP) == 0 || k == Node::KIND_GROUP) &&
        ((what & IS_SET) == 0 || k == Node::KIND_SET) &&
        ((what & IS_EXTENSIBLE) == 0 || k != Node::KIND_GROUP ||
         dynamic_cast< GroupNode * >(p.get())->isExtensible()) &&
        ((what & IS_GROUP_MEMBER) == 0 ||
         getParentNode()->kind() == Node::KIND_GROUP) ||
        ((what & IS_SET_MEMBER) == 0 ||
         getParentNode()->kind() == Node::KIND_SET) ||
        ((what & IS_UPDATE) == 0 || getRootAccess()->isUpdate());
}
#endif

}
